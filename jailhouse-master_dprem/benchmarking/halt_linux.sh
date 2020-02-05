#!/bin/bash
sudo /usr/src/kernel/kernel-4.4/Documentation/watchdog/src/watchdog-test -t 600
#sudo bash -c "/sbin/init 1"
sudo insmod stop-machine/stop-machine.ko
