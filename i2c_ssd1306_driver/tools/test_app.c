#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define DEV_PATH "/dev/ssd1306"

int main() {
    int fd;
    // SSD1306 128x64 分辨率，每 8 像素佔 1 Byte，總共 1024 Bytes
    uint8_t buffer[1024]; 

    // 1. 開啟裝置檔案
    fd = open(DEV_PATH, O_RDWR);
    if (fd < 0) {
        perror("無法開啟裝置，請確認是否有 sudo 權限或驅動已載入");
        return -1;
    }

    // 2. 產生條紋圖樣 (Horizontal Stripes)
    // 0xAA = 10101010, 0x55 = 01010101
    for (int i = 0; i < 1024; i++) {
        buffer[i] = (i % 2 == 0) ? 0xAA : 0x55;
    }

    // 3. 透過 write 系統呼叫將資料送進驅動程式
    printf("正在將條紋資料送往 OLED...\n");
    if (write(fd, buffer, sizeof(buffer)) < 0) {
        perror("寫入失敗");
        close(fd);
        return -1;
    }

    printf("成功！請查看 OLED 螢幕是否出現黑白相間的條紋。\n");

    close(fd);
    return 0;
}
