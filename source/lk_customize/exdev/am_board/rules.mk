LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/refa04/inc/ \
	$(LOCAL_DIR)/refa03/inc/ \
	$(LOCAL_DIR)/../../../../hal/audio_hal/common \

MODULE_SRCS += $(LOCAL_DIR)/refa04/src/refa04_path.c
MODULE_SRCS += $(LOCAL_DIR)/refa03/src/refa03_path.c
include make/module.mk