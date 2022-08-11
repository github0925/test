LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/sd_avb_ops.c \
	$(LOCAL_DIR)/sd_crc32.c \

ifeq ($(SUPPORT_CE_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/ce_verify.c
else
MODULE_DEPS += \
		lib/boringssl

MODULE_SRCS += \
	$(LOCAL_DIR)/boringssl_verify.c
endif

MODULE_DEPS += \
		lib/libavb \
		lib/partition \
		lib/sd_x509

GLOBAL_DEFINES += \
		DYNAMIC_SD_CRC_TABLE=1
include make/module.mk
