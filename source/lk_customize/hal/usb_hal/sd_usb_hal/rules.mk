LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/src/usb_hal.c \
	$(LOCAL_DIR)/src/class/class_fastboot.c \
	$(LOCAL_DIR)/src/class/winusb.c \

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc/

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format

include make/module.mk
