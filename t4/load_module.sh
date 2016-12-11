#!/bin/bash
# /**
# * Name: Preston Peterson
# * Lab/task: Project 2 Task 4
# * Date: 12/08/16
# **/
make
sudo rmmod sim_dev
sudo rm /dev/sim_dev
sudo insmod sim_dev.ko
sudo mknod /dev/sim_dev c 567 0
sudo chmod a+w /dev/sim_dev
