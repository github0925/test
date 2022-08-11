LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

PFM_POOL_SIZE ?= 0x400000

GLOBAL_DEFINES += \
	PFM_POOL_SIZE=$(PFM_POOL_SIZE)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_ddr_pfmon.c \

include make/module.mk