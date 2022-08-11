#####################################################################
# File name: rules.mk
# Author: chentianming
# Mail:  tianming.chen@semidrive.com
# Created Time:  23/10/2019
# Copyright (C) 2019 Semidrive Technology Co.Ltd.
#####################################################################

LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

VPU_CODAJ12_DRV_MEM_SIZE ?= 0x01400000
VMEM_PAGE_SIZE ?= 0x100000

GLOBAL_DEFINES += \
	VPU_CODAJ12_DRV_MEM_SIZE=$(VPU_CODAJ12_DRV_MEM_SIZE) \
	VMEM_PAGE_SIZE=$(VMEM_PAGE_SIZE)


GLOBAL_INCLUDES += \
    $(GLOBAL_INCLUDES)  \
    $(LOCAL_DIR)/inc  \
    $(LOCAL_DIR)/inc/jpuapi   \
    $(LOCAL_DIR)/inc/jdi

MODULE_SRCS += \
	$(LOCAL_DIR)/src/jpuapi/jpulog.c \
	$(LOCAL_DIR)/src/jdi/jdi.c  \
	$(LOCAL_DIR)/src/jpuapi/jpuapi.c \
	$(LOCAL_DIR)/src/jpuapi/jpuapifunc.c \
    $(LOCAL_DIR)/src/jdi/mm.c

MODULE_DEFS += \
    lib/cbuf

include make/module.mk
