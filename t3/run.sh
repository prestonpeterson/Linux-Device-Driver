#!/bin/bash
make clean
make
./test_sim_dev
sudo dmesg --read-clear