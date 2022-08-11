LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_PATH)

MODULE_SRCS += \
        $(LOCAL_DIR)/rpmsg_device.c

ifeq ($(SUPPORT_3RD_RPMSG_LITE),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/rpmsg_echo.c   \
	$(LOCAL_DIR)/rpmsg_tty.c   \
	$(LOCAL_DIR)/rpmsg_channel.c

GLOBAL_DEFINES += CONFIG_RPMSG_SERVICE=1

MODULE_DEPS += \
	3rd/rpmsg-lite \
	framework/communication
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format

include make/module.mk
