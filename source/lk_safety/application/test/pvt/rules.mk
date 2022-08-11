LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(SUPPORT_PVT_TEST),true)
MODULE_SRCS += \
        $(LOCAL_DIR)/pvt_test.c
endif

include make/module.mk

