LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(SUPPORT_PVT_APP_MONITOR),true)
MODULE_SRCS += \
        $(LOCAL_DIR)/pvt_monitor.c
endif

ifeq ($(SUPPORT_PVT_APP_PRINT),true)
MODULE_SRCS += \
        $(LOCAL_DIR)/pvt_print.c
endif

include make/module.mk

