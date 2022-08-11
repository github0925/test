LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)


GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc

MODULE_DEPS += \
	lib/libc \
	kernel/lk_wrapper

MODULE_SRCS += \
	$(LOCAL_DIR)/container.c

include make/module.mk
