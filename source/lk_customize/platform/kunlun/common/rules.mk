LOCAL_PATH := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_PATH)

GLOBAL_INCLUDES += \
	$(LOCAL_PATH)

MODULE_SRCS += \
	$(LOCAL_PATH)/$(ARCH)/com_platform.c

ifeq ($(SUPPORT_VIRT_UART), true)
GLOBAL_INCLUDES += $(LOCAL_PATH)/vuart_buf
MODULE_SRCS += $(LOCAL_PATH)/vuart_buf/vuart_buf.c
endif

include make/module.mk
