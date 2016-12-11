#!/bin/bash
make
./test_read.o
sudo dmesg --read-clear