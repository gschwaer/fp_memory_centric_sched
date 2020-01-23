#!/bin/bash
sudo jailhouse cell destroy gschwaer-testing-col-p0
sudo jailhouse cell create ../../configs/arm64/jetson-tx2-gschwaer-testing-p0.cell
sudo jailhouse cell list
