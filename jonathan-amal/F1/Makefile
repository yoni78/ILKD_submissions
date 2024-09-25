obj-m := rubiks_cube_module.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod rubiks_cube_module.ko

unload:
	sudo rmmod rubiks_cube_module
