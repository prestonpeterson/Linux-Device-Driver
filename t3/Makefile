UNAME	:= $(shell uname -r)
KDIR	:= /lib/modules/$(UNAME)/build
PWD	:= $(shell pwd)

obj-m	+= sim_dev.o
simdev-m	:= sim_dev.o sim_dev.ko

all::	test_sim_dev
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	
test_sim_dev: test_sim_dev.c
	gcc -o $@ $<

clean::
	$(RM) test_sim_dev 
	$(MAKE) -C $(KDIR) M=$(PWD) clean
