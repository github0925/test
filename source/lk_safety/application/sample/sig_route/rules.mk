LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/com.c \
	$(LOCAL_DIR)/com_cfg.c

GLOBAL_INCLUDES += $(LOCAL_DIR)

GLOBAL_DEFINES += COM

include make/module.mk
