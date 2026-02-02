#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

int main() {
    int fd = open("/dev/ssd1306", O_RDWR);
    if (fd < 0) return -1;

    uint8_t black[1024] = {0}; // 全填 0
    write(fd, black, 1024);    // 透過驅動執行 Burst Write 清空螢幕

    close(fd);
    return 0;
}
