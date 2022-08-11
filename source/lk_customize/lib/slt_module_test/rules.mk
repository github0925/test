LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS := \
	$(LOCAL_DIR)/slt_module_test.c

EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/slt_module_test.ld

include make/module.mk
