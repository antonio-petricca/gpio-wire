scripts-dir   = ../scripts

base          = gpiowire
obj-m        += $(base).o
obj-ko        = $(base).ko

mock-base     = gpio-mockup
mock-driver   = /lib/modules/$(shell uname -r)/kernel/drivers/gpio/$(mock-base).ko

dev-number    = 0
dev-settings  = /sys/class/$(base)s/$(base)$(dev-number)/settings

pin-number-1  = 139
pin-number-2  = 1013

dev-file      = /dev/$(base)$(dev-number)

ifneq ($(TARGET),release)
	ccflags-y += -g -DDEBUG
endif

all:
	reset; make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
build:
	make all
chip-build:
	cd $(scripts-dir); ./chip-build-lkm $(PWD) $(TARGET)
chip-install-sources:
	cd $(scripts-dir); ./chip-install-sources
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
gdb:
	@echo "********************************************************************************"
	@echo "Look at https://wiki.ubuntu.com/Kernel/KernelDebuggingTricks"
	@echo "Paragraph 'Using GDB to find the location where your kernel panicked or oopsed.'"
	@echo "EG: list *(sd_remove+0x20)"
	@echo "********************************************************************************"
	gdb $(base).o
get-pin-number:
	cat $(dev-settings)/pinNumber
install:
	sync
	sudo insmod $(obj-ko)
	sudo chmod 0666 $(dev-file)*
install-mockup:
	sync
	sudo insmod $(mock-driver) gpio_mockup_ranges=34,1023
	sudo insmod $(obj-ko) devicesNumber=3
	sudo chmod 0666 $(dev-file)*
ls-mod:
	lsmod | grep gpio
log-show:
	dmesg
log-tail:
	dmesg -w
mod-info:
	modinfo $(obj-ko)
mod-probe:
	sudo modprobe $(base)
mod-rm-mockup:
	sudo rmmod $(mock-base)
set-perf-debug-off:
	sudo bash -c "echo 0 > $(dev-settings)/perfDebug"
set-perf-debug-on:
	sudo bash -c "echo 1 > $(dev-settings)/perfDebug"
set-pin-number-1:
	sudo bash -c "echo 0 > $(dev-settings)/canSleep"
	sudo bash -c "echo $(pin-number-1) > $(dev-settings)/pinNumber"
set-pin-number-2:
	sudo bash -c "echo 1 > $(dev-settings)/canSleep"
	sudo bash -c "echo $(pin-number-2) > $(dev-settings)/pinNumber"
set-pin-swap-off:
	sudo bash -c "echo 0 > $(dev-settings)/swapOutput"
set-pin-swap-on:
	sudo bash -c "echo 1 > $(dev-settings)/swapOutput"
set-log-debug-level:
	sudo dmesg -n 8
setup-default: set-log-debug-level set-pin-number-1
	sudo bash -c "echo    2 > $(dev-settings)/bitSyncCount"
	sudo bash -c "echo  500 > $(dev-settings)/highStateEdge"
	sudo bash -c "echo 1000 > $(dev-settings)/bitZeroDuration"
	sudo bash -c "echo 2000 > $(dev-settings)/bitOneDuration"
	sudo bash -c "echo 5000 > $(dev-settings)/bitSyncDuration"
setup-fast: set-log-debug-level set-pin-number-1
	sudo bash -c "echo   2 > $(dev-settings)/bitSyncCount"
	sudo bash -c "echo 100 > $(dev-settings)/highStateEdge"
	sudo bash -c "echo 200 > $(dev-settings)/bitZeroDuration"
	sudo bash -c "echo 300 > $(dev-settings)/bitOneDuration"
	sudo bash -c "echo 500 > $(dev-settings)/bitSyncDuration"
setup-medium: set-log-debug-level set-pin-number-1
	sudo bash -c "echo    2 > $(dev-settings)/bitSyncCount"
	sudo bash -c "echo  500 > $(dev-settings)/highStateEdge"
	sudo bash -c "echo 1000 > $(dev-settings)/bitZeroDuration"
	sudo bash -c "echo 1500 > $(dev-settings)/bitOneDuration"
	sudo bash -c "echo 2500 > $(dev-settings)/bitSyncDuration"
setup-slow: set-log-debug-level set-pin-number-1
	sudo bash -c "echo    2 > $(dev-settings)/bitSyncCount"
	sudo bash -c "echo 1000 > $(dev-settings)/highStateEdge"
	sudo bash -c "echo 2000 > $(dev-settings)/bitZeroDuration"
	sudo bash -c "echo 3000 > $(dev-settings)/bitOneDuration"
	sudo bash -c "echo 5000 > $(dev-settings)/bitSyncDuration"
uninstall:
	sudo rmmod $(obj-ko)
uninstall-mockup:
	sudo rmmod $(obj-ko)
	sudo rmmod $(mock-base)
write-no-crc:
	bash -c 'read -p "Buffer: " user_text; echo -e -n "\x02$$user_text\x03" > $(dev-file)'
