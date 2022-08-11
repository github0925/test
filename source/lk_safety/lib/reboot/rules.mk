LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/reboot_service.c \

MODULE_DEPS += \
	framework/lib/mem_image \
	lib/boot

GLOBAL_INCLUDES += \
		$(LOCAL_DIR) \

include make/module.mk
