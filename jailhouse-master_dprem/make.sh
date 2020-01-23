#!/bin/bash
sudo make clean
make -j6 KDIR=/usr/src/kernel/kernel-4.4/
exit $?
