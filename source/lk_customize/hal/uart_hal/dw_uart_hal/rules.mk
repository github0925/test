LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_UART_DWDRV), true)
GLOBAL_DEFINES += ENABLE_DW_UART=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/uart_hal.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/uart_hal_weak.c
endif

include make/module.mk