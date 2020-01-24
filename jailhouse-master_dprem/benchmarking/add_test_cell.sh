#!/bin/bash
re='^[0-9]+$'
if [[ "$#" -ne 1 ]] || ! [[ "$1" =~ $re ]] || [[ "$1" -lt 0 ]] || [[ "$1" -gt 2 ]]; then
	echo "Usage: $(basename "$0") 0|1|2"
	exit 1
fi
sudo jailhouse cell destroy "gschwaer-testing-col-p$1" 2>/dev/null
sudo jailhouse cell create "../configs/arm64/jetson-tx2-gschwaer-testing-p$1.cell" && \
echo "Cell \"gschwaer-testing-col-p$1\" added." || \
echo "Adding cell \"gschwaer-testing-col-p$1\" failed."
