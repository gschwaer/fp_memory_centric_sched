#!/bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
sudo modprobe jailhouse && \
sudo jailhouse enable $parent_path/../configs/arm64/jetson-tx2-colored.cell && \
echo "Jailhouse started in configuration \"configs/arm64/jetson-tx2-colored.cell\"." || \
echo "Starting Jailhouse in configuration \"configs/arm64/jetson-tx2-colored.cell\" failed."
