LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)


GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/reboot.c \

MODULE_DEPS += \
	hal/wdg_hal/sd_wdg_hal \
	chipdev/wdg/sd_wdg \
	lib/boot

include make/module.mk
