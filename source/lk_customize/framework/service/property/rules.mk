LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/property.c \

MODULE_DEPS += \
	framework/communication

MODULE_COMPILEFLAGS += -Wall

include make/module.mk
