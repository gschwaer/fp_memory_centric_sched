#!/bin/bash
if [ "$#" -ne 1 ]; then
	echo "Usage: $(basename "$0") some_erika_inmate.bin"
	exit 1
fi
sudo bash -c "echo 1 > /sys/kernel/debug/bpmp/debug/clk/uartc/state"
sudo stty -F /dev/ttyTHS2 speed 115200
sudo jailhouse cell shutdown gschwaer-testing-col-p0
sudo jailhouse cell load gschwaer-testing-col-p0 $1
sudo jailhouse cell start gschwaer-testing-col-p0
sudo jailhouse cell list
