
obj-m := v4l2-loop.o

ifeq ($(KERNELRELEASE),)

KERNELRELEASE := `uname -r`
KDIR := /lib/modules/$(KERNELRELEASE)/build

.PHONY: all install clean distclean
.PHONY: v4l2-loop.ko

all: v4l2-loop.ko

v4l2-loop.ko:
	@echo "Building $@ driver..."
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

clean:
	rm -f *~
	rm -f Module.symvers Module.markers modules.order
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif # !KERNELRELEASE
