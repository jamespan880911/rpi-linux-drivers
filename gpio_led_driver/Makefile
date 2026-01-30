obj-m += led_driver.o

all:
	# 編譯 Driver (.ko)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	# 編譯 Device Tree (.dtbo)
	dtc -@ -I dts -O dtb -o led_overlay.dtbo led_overlay.dts

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f led_overlay.dtbo
