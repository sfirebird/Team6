obj-m += i2c_client_sh1106.o
obj-m += logmsg.o

KDIR += /lib/modules/$(shell uname -r)/build 

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean