#!/bin/bash
./stop_jailhouse.sh
./make.sh
if [ $? -ne 0 ]; then
	exit $?
fi
./install.sh
./start_jailhouse.sh
#./add_test_cell.sh
