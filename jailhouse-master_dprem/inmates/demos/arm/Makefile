#
# Jailhouse, a Linux-based partitioning hypervisor
#
# Copyright (c) ARM Limited, 2014
#
# Authors:
#  Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
#
# This work is licensed under the terms of the GNU GPL, version 2.  See
# the COPYING file in the top-level directory.
#

include $(INMATES_LIB)/Makefile.lib

INMATES := gic-demo.bin uart-demo.bin ivshmem-demo.bin test.bin my-gic-demo.bin my-gic-demo-2.bin empty.bin my-gic-demo-0.bin hc_overhead.bin arb_overhead_1.bin arb_overhead_2.bin

gic-demo-y	:= gic-demo.o
my-gic-demo-y	:= my-gic-demo.o
my-gic-demo-0-y	:= my-gic-demo-0.o
my-gic-demo-2-y	:= my-gic-demo-2.o
uart-demo-y	:= uart-demo.o
ivshmem-demo-y	:= ivshmem-demo.o
test-y		:= test.o
empty-y		:= empty.o
hc_overhead-y	:= hc_overhead.o
arb_overhead_1-y:= arb_overhead_1.o
arb_overhead_2-y:= arb_overhead_2.o

$(eval $(call DECLARE_TARGETS,$(INMATES)))
