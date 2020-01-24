#!/bin/bash
sudo jailhouse disable && \
sudo rmmod jailhouse && \
echo "Jailhouse disabled." || \
echo "Disabling Jailhouse failed."
