LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

ifeq ($(SUPPORT_PWM_SDDRV),true)
MODULE_SRCS += \
        $(LOCAL_DIR)/backlight.c
else
MODULE_SRCS += \
        $(LOCAL_DIR)/backlight_weak.c
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk

