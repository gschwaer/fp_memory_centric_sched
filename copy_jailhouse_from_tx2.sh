#!/bin/bash
# copy files from TX2 to host
sshpass -p nvidia rsync -rltPEch --exclude='*.o' nvidia@TX2:/home/nvidia/jailhouse-master_dprem ./
