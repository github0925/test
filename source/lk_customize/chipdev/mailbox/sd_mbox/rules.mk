LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
        $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/mb_controller.c

MODULE_DEPS += \
	lib/cbuf

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format

include make/module.mk
