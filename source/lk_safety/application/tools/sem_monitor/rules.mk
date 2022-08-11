LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/sem_monitor.c
MODULE_DEPS += chipdev/func_safety

include make/module.mk

