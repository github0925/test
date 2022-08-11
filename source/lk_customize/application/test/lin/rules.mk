LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/app_lin.c \
	$(LOCAL_DIR)/app_lin_cfg.c \


GLOBAL_DEFINES += \
	SCI1_USED=1 \
	SCI2_USED=1 \
	SCI3_USED=1 \
	SCI4_USED=1

include make/module.mk

