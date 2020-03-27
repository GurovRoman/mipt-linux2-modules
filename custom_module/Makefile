obj-m += phonebook.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
src:
	make -C ../linux-source-5.4 M=$(PWD) modules