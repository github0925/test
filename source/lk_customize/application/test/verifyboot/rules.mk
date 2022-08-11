LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/cmd_vb.c \


ifeq ($(SUPPORT_CE_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/ce_verify.c
GLOBAL_DEFINES += \
	USE_CE_VERIFY=1
else
MODULE_SRCS += \
	$(LOCAL_DIR)/boringssl_verify.c
endif

ifeq ($(VERIFIED_BOOT), true)
MODULE_DEPS += \
	lib/libavb \
	lib/sd_x509 \
	lib/verified_boot \
	lib/boringssl
endif

include make/module.mk
