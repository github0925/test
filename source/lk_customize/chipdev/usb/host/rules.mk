LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	lib/libusb

MODULE_SRCS += \
		$(LOCAL_DIR)/xhci.c \
		$(LOCAL_DIR)/xhci-mem.c \
		$(LOCAL_DIR)/xhci-ring.c \
		$(LOCAL_DIR)/xhci-dwc3.c \
		$(LOCAL_DIR)/xhci-sdrv.c

GLOBAL_DEFINES += SUPPORT_USB_HOST=1

MODULE_CFLAGS += -std=gnu11

include make/module.mk
