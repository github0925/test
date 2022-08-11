LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/dcf_common.c \
	$(LOCAL_DIR)/dcf_file.c   \
	$(LOCAL_DIR)/dcf_select.c \
	$(LOCAL_DIR)/dcf_notify.c \
	$(LOCAL_DIR)/dcf_service.c

ifeq ($(SUPPORT_DCF), true)
GLOBAL_DEFINES += CONFIG_SUPPORT_DCF=1

ifeq ($(SUPPORT_POSIX),true)
GLOBAL_DEFINES += CONFIG_SUPPORT_POSIX=1
endif

MODULE_DEPS += \
	framework/communication \
	framework/service/property \
	framework/service/rpmsg

endif

MODULE_COMPILEFLAGS += -Wall

include make/module.mk
