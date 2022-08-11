LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

MODULE_SRCS := \
	$(LOCAL_DIR)/slt_main.c \
	$(LOCAL_DIR)/slt_config.c \
	$(LOCAL_DIR)/slt_message.c \
	$(LOCAL_DIR)/slt_test.c \
	$(LOCAL_DIR)/slt_com_host.c

ifeq ($(SUPPORT_SLT_TEST_DEMO),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/slt_module_test_sample.c
endif

MODULE_SRCS += $(wildcard $(LOCAL_DIR)/test_case/$(CHIPVERSION)/$(DOMAIN)/*.c)

MODULE_DEPS += \
    lib/libc \
	lib/slt_module_test

include make/module.mk
