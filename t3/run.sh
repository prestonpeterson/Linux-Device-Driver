#!/bin/bash
make
./test_write
sudo dmesg --read-clear