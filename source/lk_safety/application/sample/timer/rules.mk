LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/pwm_cpt/ $(GLOBAL_INCLUDES) \

MODULE_SRCS += \
        $(LOCAL_DIR)/pwm_cpt/cpt.c \

include make/module.mk

