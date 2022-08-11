LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/../../../hal/audio_hal/common $(GLOBAL_INCLUDES)\
	$(LOCAL_DIR)/include/

MODULE_SRCS += \
	$(LOCAL_DIR)/i2s_test_on_board_new.c

MODULE_DEPS += lib/debugcommands

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
include $(LOCAL_DIR)/../../../hal/audio_hal/test/rules.mk
include $(LOCAL_DIR)/../../../exdev/audio_codec/tlv320aic23b_q1/rules.mk
