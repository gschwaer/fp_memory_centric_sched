#!/bin/bash
sudo bash -c "echo 1 > /sys/kernel/debug/bpmp/debug/clk/uartc/state" && \
sudo stty -F /dev/ttyTHS2 speed 115200 1>&/dev/null && \
echo "UART-C enabled." || \
echo "Enabling UART-C failed."
