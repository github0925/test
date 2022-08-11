LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR) $(GLOBAL_INCLUDES)

MODULE_SRCS += \
	$(LOCAL_DIR)/dw_adc.c \

include make/module.mk
