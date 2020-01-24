#!/bin/bash
sudo modprobe jailhouse && \
sudo jailhouse enable ../configs/arm64/jetson-tx2-colored.cell && \
echo "Jailhouse started in configuration \"configs/arm64/jetson-tx2-colored.cell\"." || \
echo "Starting Jailhouse in configuration \"configs/arm64/jetson-tx2-colored.cell\" failed."
