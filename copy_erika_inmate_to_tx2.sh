#!/bin/bash
sshpass -p nvidia scp -v out/erika_inmate.bin nvidia@TX2:/home/nvidia/jailhouse-master_dprem/benchmarking/erika_inmate.bin 2>&1 | grep -v debug1
cp out/erika_inmate.bin jailhouse-master_dprem/benchmarking/
