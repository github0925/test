LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/ssystem.c \
	$(LOCAL_DIR)/auxiliary_funcs.c \
	$(LOCAL_DIR)/service.c \

MODULE_DEPS += \
	framework/service/property

ifneq ($(DEBUG), 0)
MODULE_SRCS += \
	$(LOCAL_DIR)/debugcommands.c
endif

MODULE_DEPS += \
	dev \
	lib/partition \
	lib/storage_device \
	lib/boot \
	lib/libavb \
	lib/system_config \
	framework/lib/mem_image

ifeq ($(VERIFIED_BOOT), true)
MODULE_DEPS += \
	lib/verified_boot
endif

ifeq ($(SUPPORT_SSYSTEM_SERVER), true)

MODULE_DEPS += \
	framework/service/rpmsg

GLOBAL_DEFINES += CONFIG_SSYSTEM_SERVER=1
endif

include make/module.mk
