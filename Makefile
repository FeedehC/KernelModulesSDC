obj-m +=  drv.o #mimodulo.o drv1.o #drv2.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc read_data.c -o read_data 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm *.csv *.o read_data

