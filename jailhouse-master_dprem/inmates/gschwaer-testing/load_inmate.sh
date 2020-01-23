#!/bin/bash
if [ "$#" -ne 1 ]; then
	echo "Usage: $(basename "$0") inmate.bin"
	exit 1
fi
sudo jailhouse cell shutdown gschwaer-testing-col-p0
sudo jailhouse cell load gschwaer-testing-col-p0 $1
sudo jailhouse cell list
