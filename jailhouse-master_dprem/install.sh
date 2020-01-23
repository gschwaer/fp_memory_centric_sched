#!/bin/bash
sudo jailhouse cell list
sudo jailhouse disable
sudo rmmod jailhouse
sudo make KDIR=/usr/src/kernel/kernel-4.4/ install
