#
# Jailhouse AArch64 support for HERCULES inmate
#
# Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
#
# Authors:
#  Antonios Motakis <antonios.motakis@huawei.com>
#  Luca Miccio <lucmiccio@gmail.com>
#
# This work is licensed under the terms of the GNU GPL, version 2.  See
# the COPYING file in the top-level directory.
#

$(info COMPILING HERCULES INMATES)
# PREM support
POLY_LIB_OUT_FOLDER=/home/luca/hipert/hercules/jailhouse-polybench/out_libs/
ifneq ("$(wildcard $(POLY_LIB_OUT_FOLDER))","")
$(info $(POLY_LIB_OUT_FOLDER) Found)
POLY_LIB_OUT = $(POLY_LIB_OUT_FOLDER)/no_prem/
POLY_LIB = $(POLY_LIB_OUT)/poly_2mm.a \
		$(POLY_LIB_OUT)/poly_3mm.a \
		$(POLY_LIB_OUT)/poly_adi.a \
		$(POLY_LIB_OUT)/poly_atax.a \
		$(POLY_LIB_OUT)/poly_bicg.a \
		$(POLY_LIB_OUT)/poly_covariance.a \
		$(POLY_LIB_OUT)/poly_gemm.a \
		$(POLY_LIB_OUT)/poly_gemver.a \
		$(POLY_LIB_OUT)/poly_jacobi_2d.a \
		$(POLY_LIB_OUT)/poly_mvt.a \
		$(POLY_LIB_OUT)/poly_seidel.a
LINUXINCLUDE += -I$(POLY_LIB_OUT_FOLDER)/include
# Uncomment this line only if you are using PREM-ized libraries
#POLY_LIB += $(POLY_LIB_OUT)/poly_libprem.a
# Comment all the above lines if you don't want the PREM support

LINKER_SCRIPT = $(INMATES_LIB)/inmate_hercules.lds
include $(INMATES_LIB)/Makefile.lib
$(info )

INMATES := hercules_test.bin irq_test.bin polybench_tests.bin test_GPREM.bin

hercules_test-y	:= hercules_test.o
irq_test-y := irq_test.o
polybench_tests-y := polybench_tests.o
test_GPREM-y := test_GPREM.o caches.o


$(eval $(call DECLARE_TARGETS,$(INMATES)))
else
$(info $(POLY_LIB_OUT_FOLDER) Not Found)
$(info Change POLY_LIB_OUT_FOLDER variable)
endif
