LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/../../../hal/audio_hal/common \
	$(GLOBAL_INCLUDES)

ifeq ($(SUPPORT_I2S_SDDRV_2_0), true)
MODULE_DEPS += $(LOCAL_DIR)/2.0
else
MODULE_DEPS += $(LOCAL_DIR)/1.0
endif
