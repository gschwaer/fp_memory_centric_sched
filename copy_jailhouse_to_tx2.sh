#!/bin/bash
#copy all data from host to TX2
sshpass -p nvidia rsync -rltPEch ./jailhouse-master_dprem nvidia@TX2:/home/nvidia/
