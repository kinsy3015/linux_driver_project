obj-m = timer.o mmap.o suspend.o

KDIR := /work/linux-3.0.22.fin

all:
	make -C $(KDIR) M=$(PWD) modules
	arm-none-linux-gnueabi-gcc mmap_app.c -o mmap_app
	arm-none-linux-gnueabi-gcc aa_app.c -o aa_app
	arm-none-linux-gnueabi-gcc suspend_app.c -o suspend_app
clean:
	make -C $(KDIR) M=$(PWD) clean

copy:
	cp timer.ko mmap.ko suspend.ko mmap_app aa_app suspend_app /nfs/rootfs/root/
