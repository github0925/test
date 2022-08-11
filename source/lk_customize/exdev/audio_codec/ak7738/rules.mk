LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc/ \
	$(LOCAL_DIR)/../../../../hal/audio_hal/common \

MODULE_SRCS += $(LOCAL_DIR)/akm_ak7738.c
include make/module.mk
