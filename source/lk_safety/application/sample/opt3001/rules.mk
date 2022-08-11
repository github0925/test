LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/opt_sample.c \

MODULE_DEPS += \
	exdev/opt

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-sign-compare -Wno-format

include make/module.mk

