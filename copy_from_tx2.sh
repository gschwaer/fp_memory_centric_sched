#!/bin/bash
sshpass -p nvidia rsync -rltPEch nvidia@TX2:/home/nvidia/jailhouse-master_dprem ./
