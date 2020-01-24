#!/bin/bash
# copy files from TX2 to host without deleting files that are not on the TX2
sshpass -p nvidia rsync -rltPEch nvidia@TX2:/home/nvidia/jailhouse-master_dprem ./
