LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/benchmarks.c \
    $(LOCAL_DIR)/cache_tests.c \
    $(LOCAL_DIR)/cbuf_tests.c \
    $(LOCAL_DIR)/clock_tests.c \
    $(LOCAL_DIR)/fibo.c \
    $(LOCAL_DIR)/float.c \
    $(LOCAL_DIR)/float_instructions.S \
    $(LOCAL_DIR)/float_test_vec.c \
    $(LOCAL_DIR)/mem_tests.c \
    $(LOCAL_DIR)/printf_tests.c \
    $(LOCAL_DIR)/tests.c \
    $(LOCAL_DIR)/thread_tests.c \
    $(LOCAL_DIR)/port_tests.c \

MODULE_ARM_OVERRIDE_SRCS := \

MODULE_DEPS += \
    lib/cbuf \
    lib/sdunittest

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable
MODULE_COMPILEFLAGS += -Wno-pointer-bool-conversion -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unknown-attributes -Wno-nonnull-compare -Wno-unknown-warning-option

include make/module.mk
