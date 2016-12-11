# /**
# * Name: Preston Peterson
# * Lab/task: Project 2 Task 4
# * Date: 12/08/16
# **/

UNAME	:= $(shell uname -r)
KDIR	:= /lib/modules/$(UNAME)/build
PWD	:= $(shell pwd)

obj-m	+= sim_dev.o
simdev-m	:= sim_dev.o sim_dev.ko

all:: test_read.o test_write.o sim_dev
	$(MAKE) -C $(KDIR) M=$(PWD) modules

test_read.o: test_sim_dev_read.c timer_posix.c
	gcc test_sim_dev_read.c timer_posix.c -lrt -Wall -o test_read.o

test_write.o: test_sim_dev_write.c timer_posix.c
	gcc test_sim_dev_write.c timer_posix.c -lrt -Wall -o test_write.o

sim_dev: sim_dev.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@rm test_read.o test_write.o
	@$(MAKE) -C $(KDIR) M=$(PWD) clean