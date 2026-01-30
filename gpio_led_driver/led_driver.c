#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h> 
#include <linux/gpio/consumer.h>   
#include <linux/miscdevice.h>
#include <linux/of.h>              

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pan Chin-Chih");
MODULE_DESCRIPTION("Level 2: Platform GPIO LED Driver");

//操作GPIO指標（Decoupling）
struct gpio_desc *led_gpio;

//File Operations
static ssize_t led_write(struct file *file, const char __user *user_buf,
                         size_t count, loff_t *ppos) {
    char kbuf;
    if (copy_from_user(&kbuf, user_buf, 1)) return -EFAULT;

    if (kbuf == '1') {
        gpiod_set_value(led_gpio, 1); 
        printk(KERN_INFO "LED: Turn ON\n");
    } else if (kbuf == '0') {
        gpiod_set_value(led_gpio, 0); 
        printk(KERN_INFO "LED: Turn OFF\n");
    }
    return count;
}

//定義操作
static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = led_write,
};

//向上註冊 /dev/my_led 提供給User
static struct miscdevice led_miscdevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "my_led",
    .fops = &led_fops,
};

// Platform Driver Probe (Device Tree 配對成功呼叫)
static int led_probe(struct platform_device *pdev) {
    int ret;
    printk(KERN_INFO "LED: Match Found! Probing...\n");

    // 跟 Device Tree 拿 GPIO (對應 dts 裡的 "led-gpios")
    led_gpio = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(led_gpio)) {
        printk(KERN_ERR "LED: Failed to get GPIO from DTS\n");
        return PTR_ERR(led_gpio);
    }

    // 註冊雜項裝置(掛牌)
    ret = misc_register(&led_miscdevice);
    if (ret) return ret;

    printk(KERN_INFO "LED: Init Success!\n");
    return 0;
}

static void led_remove(struct platform_device *pdev) {
    misc_deregister(&led_miscdevice);
    printk(KERN_INFO "LED: Removed\n");
}

//Match table (和device tree配對)
static const struct of_device_id led_dt_ids[] = {
    { .compatible = "my-gpio-led" },
    { }
};
MODULE_DEVICE_TABLE(of, led_dt_ids);

//Driver structure
static struct platform_driver my_platform_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "my_platform_driver",
        .of_match_table = led_dt_ids,
    },
};

// 取代module_init和module_exit
module_platform_driver(my_platform_driver);
