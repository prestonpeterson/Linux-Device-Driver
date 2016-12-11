#!/bin/bash
make
sudo rmmod sim_dev
sudo rm /dev/sim_dev
sudo insmod sim_dev.ko
sudo mknod /dev/sim_dev c 567 0
sudo chmod a+w /dev/sim_dev
