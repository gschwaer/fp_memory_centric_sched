#!/bin/bash
sshpass -p nvidia scp -v out/erika_inmate.bin nvidia@TX2:/home/nvidia/jailhouse-master_dprem/inmates/gschwaer-testing/erika_inmate_dprem.bin 2>&1 | grep -v debug1
