LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/surface_receiver.c

MODULE_CFLAGS += -Wno-strict-prototypes -Wno-unused-variable

include make/module.mk