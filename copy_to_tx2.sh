#!/bin/bash
#copy all data from host to TX2 and deleting files which are not on the host
sshpass -p nvidia rsync -rltPEch --delete ./jailhouse-master_dprem nvidia@TX2:/home/nvidia/
