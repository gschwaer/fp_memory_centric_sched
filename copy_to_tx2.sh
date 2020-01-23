#!/bin/bash
sshpass -p nvidia rsync -rltPEch ./jailhouse-master_dprem nvidia@TX2:/home/nvidia/
