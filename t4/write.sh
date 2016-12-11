#!/bin/bash
make clean
make
./test_write.o
sudo dmesg --read-clear