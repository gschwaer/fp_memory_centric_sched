#!/bin/bash

sudo jailhouse enable configs/arm64/jetson-tx2.cell 

sudo jailhouse cell create configs/arm64/jetson-tx2-demo.cell 
sudo jailhouse cell create configs/arm64/jetson-tx2-demo-2.cell

sudo jailhouse cell load jetson-tx2-demo   inmates/demos/arm64/arb_overhead_1.bin
sudo jailhouse cell load jetson-tx2-demo-2 inmates/demos/arm64/arb_overhead_2.bin

sudo jailhouse cell start jetson-tx2-demo
sleep 0.5
sudo jailhouse cell start jetson-tx2-demo-2

