LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/preloader.c \

MODULE_DEPS += \
	lib/partition \
	lib/libavb \
	lib/verified_boot \
	lib/boot \
    framework/lib/mem_image

ifneq ($(BOOT_DEVICE),)
GLOBAL_DEFINES += \
    BOOT_DEVICE=$(BOOT_DEVICE)
endif

ifneq ($(BOOT_DEVICE_GPT_START),)
GLOBAL_DEFINES += \
    BOOT_DEVICE_GPT_START=$(BOOT_DEVICE_GPT_START)
endif

include make/module.mk
