LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ARCH    := arm64
ARM_CPU := cortex-a55
CPU     := generic

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/

MODULE_DEPS += platform/$(PLATFORM)/common
MODULE_SRCS += \
	$(LOCAL_DIR)/platform.c	\
	$(LOCAL_DIR)/demo.c	\
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/uart.c \
	$(LOCAL_DIR)/interrupt.c

ifeq ($(SUPPORT_VIRT_UART), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/vuart.c
endif

ifeq ($(SYSTEM_TIMER),sdrv_timer)
	MODULE_SRCS += $(LOCAL_DIR)/timer.c
endif

ifeq ($(SUPPORT_DCF),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/domain_config.c
endif

include make/module.mk
