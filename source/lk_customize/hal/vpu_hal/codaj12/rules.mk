#####################################################################
# File name: rules.mk
# Author: chentianming
# Mail:  tianming.chen@semidrive.com
# Created Time:  29/10/2019
# Copyright (C) 2019 Semidrive Technology Co. Ltd
#####################################################################
LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
    $(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES)

MODULE_SRCS += \
    $(LOCAL_DIR)/src/vpu_hal.c

#MODULE_COMPILEFLAGS +=  \
    -Wno-int-conversion \
    -Wno-discarded-qualifiers \
    -Wno-format  \
    -fno-builtin  \
    -Wno-unused-variable \
    -Wno-sign-compare  \
    -Wno-format  \
    -Wno-int-to-void-pointer-cast $(INCLUDES)

include make/module.mk

MODULE_DEPS += \
    lib/cbuf
