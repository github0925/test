LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/sdpe_stat.c \
	$(LOCAL_DIR)/sdpe_perf_test.c

include make/module.mk
