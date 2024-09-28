obj-m := rubiks_cube_module.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build modules M=$(PWD)

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build clean M=$(PWD)

load:
	sudo insmod rubiks_cube_module.ko

unload:
	sudo rmmod rubiks_cube_module
