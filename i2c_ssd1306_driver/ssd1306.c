#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>

#define DEVICE_NAME "ssd1306"
#define CLASS_NAME  "ssd_class"

/* 裝置結構體：封裝所有硬體與軟體資源，方便在函式間傳遞 */
struct ssd1306_data {
    struct i2c_client *client;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    dev_t dev_num;
};

/* --- 底層指令傳送 (Control Byte = 0x00) --- */
static int ssd1306_write_cmd(struct i2c_client *client, uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd}; 
    return i2c_master_send(client, buf, 2);
}

/* --- 底層數據傳送 (Control Byte = 0x40) --- */
static int ssd1306_write_data(struct i2c_client *client, uint8_t data) {
    uint8_t buf[2] = {0x40, data}; 
    return i2c_master_send(client, buf, 2);
}

static int ssd1306_init_display(struct i2c_client *client) {
    uint8_t *clear_buf;

    /* 1. 喚醒序列：照著你目前的設定 */
    ssd1306_write_cmd(client, 0xAE); // 顯示關閉 (Display OFF)
    ssd1306_write_cmd(client, 0xD5); // 設定時鐘分頻頻率
    ssd1306_write_cmd(client, 0x80);
    ssd1306_write_cmd(client, 0xA8); // 設定多路復用率
    ssd1306_write_cmd(client, 0x3F);
    ssd1306_write_cmd(client, 0xD3); // 設定顯示偏移
    ssd1306_write_cmd(client, 0x00);
    ssd1306_write_cmd(client, 0x40); // 設定起始行
    ssd1306_write_cmd(client, 0x8D); // 啟動充電泵
    ssd1306_write_cmd(client, 0x14); 
    ssd1306_write_cmd(client, 0x20); // 設定記憶體定址模式 (重要！)
    ssd1306_write_cmd(client, 0x00); // 水平模式 (Horizontal Mode)
    ssd1306_write_cmd(client, 0xA1); // 段重映射 (左右反轉)
    ssd1306_write_cmd(client, 0xC8); // 掃描方向 (上下反轉)
    ssd1306_write_cmd(client, 0xDA); // COM 硬體配置
    ssd1306_write_cmd(client, 0x12);
    ssd1306_write_cmd(client, 0x81); // 設定對比度
    ssd1306_write_cmd(client, 0xCF);
    ssd1306_write_cmd(client, 0xA4); // 恢復顯存內容顯示
    ssd1306_write_cmd(client, 0xA6); // 正常顯示 (非反相)

    /* 2. 確保載入驅動後螢幕是全黑的 */
    // 先把座標指針歸零
    ssd1306_write_cmd(client, 0x21); ssd1306_write_cmd(client, 0x00); ssd1306_write_cmd(client, 127);
    ssd1306_write_cmd(client, 0x22); ssd1306_write_cmd(client, 0x00); ssd1306_write_cmd(client, 7);

    // 準備 1025 Bytes 的 0x00 (包含一個 0x40 Data Control Byte)
    clear_buf = kzalloc(1025, GFP_KERNEL); 
    if (clear_buf) {
        clear_buf[0] = 0x40; // 數據模式
        // 一次性送出 (Burst Mode)，這會把開機時隨機的雪花雜訊蓋掉
        i2c_master_send(client, clear_buf, 1025);
        kfree(clear_buf);
    }

    /* 3. 正式點亮 */
    ssd1306_write_cmd(client, 0xAF); // 顯示開啟 (Display ON)

    return 0;
}

/* --- 檔案操作：當 User space 執行 write() 時呼叫 --- */
static ssize_t ssd1306_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos) {
    struct ssd1306_data *data = file->private_data;
    uint8_t *kbuf;
    int ret;

    /* 1. 配置內核空間暫存區 (多預留 1 Byte 給 Control Byte) */
    kbuf = kmalloc(count + 1, GFP_KERNEL);
    if (!kbuf) return -ENOMEM;

    /* 2. 第一個 Byte 放 Control Byte (0x40 代表接下來全是數據) */
    kbuf[0] = 0x40;

    /* 3. 從第二個 Byte 開始複製 User 數據 */
    if (copy_from_user(&kbuf[1], user_buf, count)) {
        kfree(kbuf);
        return -EFAULT;
    }

    /* 4. 重設顯示座標 (維持原樣) */
    ssd1306_write_cmd(data->client, 0x21); 
    ssd1306_write_cmd(data->client, 0x00);
    ssd1306_write_cmd(data->client, 127);
    ssd1306_write_cmd(data->client, 0x22); 
    ssd1306_write_cmd(data->client, 0x00);
    ssd1306_write_cmd(data->client, 7);

    dev_info(data->device, "ssd1306: Burst sending %zu bytes\n", count);

    /* 5. 一次性傳送 1025 Bytes (1 Control + 1024 Data) */
    ret = i2c_master_send(data->client, kbuf, count + 1);
    
    kfree(kbuf);
    
    if (ret < 0) {
        dev_err(data->device, "I2C burst transfer failed: %d\n", ret);
        return ret;
    }

    return count;
}

static int ssd1306_open(struct inode *inode, struct file *file) {
    /* 使用 container_of 從 cdev 反推資料結構起始位址 */
    struct ssd1306_data *data = container_of(inode->i_cdev, struct ssd1306_data, cdev);
    file->private_data = data;
    return 0;
}

static const struct file_operations ssd1306_fops = {
    .owner = THIS_MODULE,
    .open  = ssd1306_open,
    .write = ssd1306_write,
};

/* --- Probe：媒合成功後的硬體與裝置註冊 --- */
static int ssd1306_probe(struct i2c_client *client) {
    struct ssd1306_data *data;
    int ret;

    /* 使用受管制的分配器，在驅動移除時會自動釋放部分資源 */
    data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
    if (!data) return -ENOMEM;
    data->client = client;
    i2c_set_clientdata(client, data);

    /* 1. 分配字元裝置編號 */
    ret = alloc_chrdev_region(&data->dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;

    /* 2. 初始化並新增字元裝置 (cdev) */
    cdev_init(&data->cdev, &ssd1306_fops);
    ret = cdev_add(&data->cdev, data->dev_num, 1);
    if (ret < 0) goto unregister_region;

    /* 3. 在 /sys/class 下建立類別並自動生成 /dev/ssd1306 節點 */
    data->class = class_create(CLASS_NAME);
    data->device = device_create(data->class, NULL, data->dev_num, NULL, DEVICE_NAME);

    /* 4. 點亮螢幕 --- */
    ssd1306_init_display(client);

    dev_info(&client->dev, "SSD1306: Probe success on 0x%02x\n", client->addr);
    return 0;

unregister_region:
    unregister_chrdev_region(data->dev_num, 1);
    return ret;
}

/* --- Remove：卸載時的清理工作 --- */
static void ssd1306_remove(struct i2c_client *client) {
    struct ssd1306_data *data = i2c_get_clientdata(client);
    
    ssd1306_write_cmd(client, 0xAE); // 關閉螢幕顯示
    device_destroy(data->class, data->dev_num);
    class_destroy(data->class);
    cdev_del(&data->cdev);
    unregister_chrdev_region(data->dev_num, 1);
}

/* --- 媒合名單：與 Device Tree 對應 --- */
static const struct of_device_id ssd1306_ids[] = {
    { .compatible = "solomon,ssd1306", },
    { }
};
MODULE_DEVICE_TABLE(of, ssd1306_ids);

static struct i2c_driver ssd1306_driver = {
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
    .driver = {
        .name = "ssd1306",
        .of_match_table = ssd1306_ids,
    },
};

module_i2c_driver(ssd1306_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pan, Chin-Chih");
MODULE_DESCRIPTION("Professional Linux I2C OLED Driver");
