#!/bin/bash
sudo jailhouse disable
./make.sh
if [ $? -ne 0 ]; then
	exit $?
fi
./install.sh
./benchmarking/start_jailhouse.sh
#./add_test_cell.sh
