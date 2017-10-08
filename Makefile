MODULE = s3600251Device
DEBUG = -g
SOURCE = driver.c
PROG = S3600251UserApplication

# Running by kernel builduing system
ifneq ($(KERNELRELEASE),)
	obj-m := $(MODULE).o

# Runing without kernel build system
else
	KDIR ?= /lib/modules/`uname -r`/build 
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean 
exec:
	$(CC) $(SOURCE) -o $(PROG)  
load:
	insmod ./$(MODULE).ko
unload: 
	rmmod ./$(MODULE).ko
endif
