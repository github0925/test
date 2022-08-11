LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/tmp411_sample.c \

MODULE_DEPS += \
	exdev/tmp411

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin  -Wno-sign-compare -Wno-format

include make/module.mk
