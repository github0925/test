LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_ARM_GIC_SDDRV),true)
GLOBAL_DEFINES += ENABLE_ARM_GIC=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/arm_gic_hal.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/arm_gic_hal_weak.c
endif

include make/module.mk
