LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)


MODULE_SRCS += \
	$(LOCAL_DIR)/sms_monitor.c \


MODULE_DEPS += framework/service/rpmsg \

include make/module.mk
