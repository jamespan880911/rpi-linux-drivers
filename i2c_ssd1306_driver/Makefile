obj-m += ssd1306.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

TOOLS_DIR = tools

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	dtc -@ -I dts -O dtb -o ssd1306_overlay.dtbo ssd1306_overlay.dts
	$(MAKE) -C $(TOOLS_DIR)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f ssd1306_overlay.dtbo
	$(MAKE) -C $(TOOLS_DIR) clean
load:
	sudo insmod ssd1306.ko
	sudo dtoverlay ssd1306_overlay.dtbo
	@echo "驅動已載入，請檢查 /dev/ssd1306"

unload:
	-sudo dtoverlay -r ssd1306_overlay
	-sudo rmmod ssd1306
	@echo "驅動已卸載"

