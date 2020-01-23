#!/bin/bash
sudo ./start_uart_c.sh
sudo modprobe jailhouse
sudo jailhouse enable configs/arm64/jetson-tx2-colored.cell
sudo jailhouse cell list
