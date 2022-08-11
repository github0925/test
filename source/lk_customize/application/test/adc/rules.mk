LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)


MODULE_SRCS += \
	$(LOCAL_DIR)/cmd_adc_test.c \

MODULE_DEPS += \
    lib/libc \

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast \
		-D MODULE_NAME=$(MODULE)

include make/module.mk
