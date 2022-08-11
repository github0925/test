LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_DEPS += \
	3rd/fatfs/src

MODULE_SRCS += \
	$(LOCAL_DIR)/res_loader.c

ifeq ($(VERIFIED_BOOT),true)
MODULE_DEPS += lib/verified_boot
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
