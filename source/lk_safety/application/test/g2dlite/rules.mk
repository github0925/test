LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += exdev/display

MODULE_CFLAGS += $(CFLAGS)
CFLAGS :=

MODULE_SRCS += $(LOCAL_DIR)/g2dlite_example.c \
	$(LOCAL_DIR)/buffer.c

include make/module.mk
