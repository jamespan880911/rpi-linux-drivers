# Raspberry Pi Linux Driver Development

這是一個專注於 Linux Kernel Driver 開發的實作專案。
透過在 Raspberry Pi (ARM64) 平台上從零構建驅動程式，深入理解 Linux 核心子系統、記憶體管理與軟硬體解耦機制。

## 🛠 開發環境 (Environment)

* **Hardware**: Raspberry Pi 4 Model B / Pi 5
* **OS**: Raspberry Pi OS (64-bit, Bookworm)
* **Kernel Version**: Linux 6.6 LTS
* **Toolchain**: Native GCC on ARM64
* **Language**: C (ISO C99), GNU Make, Device Tree Source (DTS)

## 📂 專案結構 (Project Structure)

本專案採用模組化結構，目前包含以下驅動實作：

| Module | Description | Key Concepts |
| :--- | :--- | :--- |
| **[gpio_led_driver](./gpio_led_driver)** | 平台驅動 GPIO 控制 | `platform_driver`, `device_tree_overlay`, `devm_gpiod`, `sysfs`, `cdev` |
| *button_irq (Planning)* | 中斷處理驅動 | Interrupt Handling (`request_irq`), Top/Bottom Half, Workqueue |
| *i2c_sensor (Planning)* | I2C 感測器驅動 | I2C Subsystem, `regmap`, Industrial I/O (IIO) |

