LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/rsa_demo.c \

MODULE_DEPS += \
    lib/libc

include make/module.mk
