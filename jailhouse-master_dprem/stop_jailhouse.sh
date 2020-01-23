#!/bin/bash
sudo jailhouse cell list
sudo jailhouse disable
sudo rmmod jailhouse
echo "Jailhouse disabled"
