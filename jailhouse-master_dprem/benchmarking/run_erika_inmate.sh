#!/bin/bash
re_num='^[0-9]+$'
if [[ "$#" -ne 2 ]] || ! [[ "$2" =~ $re_num ]] || [[ "$2" -lt 0 ]] || [[ "$2" -gt 2 ]] || ! [[ -r "$1" ]]; then
	echo "Usage: $(basename "$0") erika_inmate.bin 0|1|2"
	exit 1
fi
sudo jailhouse cell shutdown "gschwaer-testing-col-p$2" 2>/dev/null
sudo jailhouse cell load "gschwaer-testing-col-p$2" "$1" && \
sudo jailhouse cell start "gschwaer-testing-col-p$2" && \
echo "Cell \"gschwaer-testing-col-p$2\" started." || \
echo "Starting cell \"gschwaer-testing-col-p$2\" failed."
