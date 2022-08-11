#####################################################################
# File name: rules.mk
# Author: chentianming
# Mail:  tianming.chen@semidrive.com
# Created Time:  23/10/2019
# Copyright (C) 2019 Semidrive Technology Co.Ltd.
#####################################################################

LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	lib/storage_device \
	lib/partition

GLOBAL_INCLUDES += \
	$(GLOBAL_INCLUDES)  \
	$(LOCAL_DIR)/inc  \
	$(LOCAL_DIR)/../../../../hal/vpu_hal/codaj12/inc

MODULE_SRCS += \
	$(LOCAL_DIR)/src/codaj12_dec_sample.c \
	$(LOCAL_DIR)/src/getopt_long.c \
	$(LOCAL_DIR)/src/cmd_vpu.c

include make/module.mk



