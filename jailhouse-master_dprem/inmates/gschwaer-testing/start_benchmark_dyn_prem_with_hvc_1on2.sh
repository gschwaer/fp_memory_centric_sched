#!/bin/bash
./add_test_cell0.sh
./add_test_cells12.sh
sudo bash -c "echo 1 > /sys/kernel/debug/bpmp/debug/clk/uartc/state"
sudo stty -F /dev/ttyTHS2 speed 115200
sudo jailhouse cell shutdown gschwaer-testing-col-p0
sudo jailhouse cell shutdown gschwaer-testing-col-p1
sudo jailhouse cell shutdown gschwaer-testing-col-p2
sudo jailhouse cell load gschwaer-testing-col-p0 erika_inmate_benchmark_p0_with_hvc.bin
sudo jailhouse cell load gschwaer-testing-col-p1 erika_inmate_interfering_p1_with_hvc.bin
sudo jailhouse cell load gschwaer-testing-col-p2 erika_inmate_interfering_p2_with_hvc.bin
sudo jailhouse cell start gschwaer-testing-col-p0
sudo jailhouse cell start gschwaer-testing-col-p1
sudo jailhouse cell start gschwaer-testing-col-p2
sudo jailhouse cell list
