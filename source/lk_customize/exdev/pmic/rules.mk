LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

ifeq ($(SUPPORT_PMIC_LP875XX), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/lp875xx.c
endif

include make/module.mk
