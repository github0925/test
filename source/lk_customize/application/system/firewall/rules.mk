LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
        $(LOCAL_DIR)/firewall.c

GLOBAL_DEFINES += \
	HW_NOT_MEET=1

include make/module.mk

