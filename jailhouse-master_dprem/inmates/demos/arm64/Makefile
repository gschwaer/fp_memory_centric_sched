#
# Jailhouse AArch64 support
#
# Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
#
# Authors:
#  Antonios Motakis <antonios.motakis@huawei.com>
#
# This work is licensed under the terms of the GNU GPL, version 2.  See
# the COPYING file in the top-level directory.
#

include $(INMATES_LIB)/Makefile.lib

INMATES := gic-demo.bin uart-demo.bin ivshmem-demo.bin stress_4M.bin stress_4M_GPREM.bin stress_4M_MG.bin test.bin my-gic-demo.bin my-gic-demo-2.bin empty.bin my-gic-demo-0.bin hc_overhead.bin arb_overhead_1.bin arb_overhead_2.bin cache-demo.bin

gic-demo-y	:= ../arm/gic-demo.o
cache-demo-y	:= ../arm/cache-demo.o
my-gic-demo-y	:= ../arm/my-gic-demo.o
my-gic-demo-0-y	:= ../arm/my-gic-demo-0.o
my-gic-demo-2-y	:= ../arm/my-gic-demo-2.o
uart-demo-y	:= ../arm/uart-demo.o
ivshmem-demo-y	:= ../arm/ivshmem-demo.o
test-y		:= ../arm/test.o
empty-y		:= ../arm/empty.o
hc_overhead-y	:= ../arm/hc_overhead.o
arb_overhead_1-y:= ../arm/arb_overhead_1.o
arb_overhead_2-y:= ../arm/arb_overhead_2.o

stress_4M-y := hercules/stress_4M.o
stress_4M_GPREM-y := hercules/stress_4M_GPREM.o hercules/caches.o
stress_4M_MG-y := hercules/stress_4M_MG.o

$(eval $(call DECLARE_TARGETS,$(INMATES)))
