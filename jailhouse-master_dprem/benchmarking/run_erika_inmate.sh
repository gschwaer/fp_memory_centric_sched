#!/bin/bash
re_num='^[0-9]+$'
if [[ "$#" -ne 2 ]] || ! [[ "$1" =~ $re_num ]] || [[ "$1" -lt 0 ]] || [[ "$1" -gt 2 ]] || ! [[ -r "$2" ]]; then
	echo "Usage: $(basename "$0") 0|1|2 erika_inmate.bin"
	exit 1
fi
sudo jailhouse cell shutdown "gschwaer-testing-col-p$1" 2>/dev/null
sudo jailhouse cell load "gschwaer-testing-col-p$1" "$2" && \
sudo jailhouse cell start "gschwaer-testing-col-p$1" && \
echo "Cell \"gschwaer-testing-col-p$1\" started." || \
echo "Starting cell \"gschwaer-testing-col-p$1\" failed."
