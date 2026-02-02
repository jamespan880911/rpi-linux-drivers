import smbus

bus = smbus.SMBus(1)
address = 0x3c

commands = [
    0x8d, 0x14, # 開啟電荷泵
    0xaf,       # 開啟顯示
    0x20, 0x00, # 設定為「水平定址模式」
]

for cmd in commands:
    bus.write_byte_data(address, 0x00, cmd)

for i in range(1024):
        bus.write_byte_data(address, 0x40, 0x00)


print("螢幕已清空為全黑！")
