LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/memtester.c \
	$(LOCAL_DIR)/tests.c \
	$(LOCAL_DIR)/cmd_memtester.c \

include make/module.mk
