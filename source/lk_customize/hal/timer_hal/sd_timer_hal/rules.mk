LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_TIMER_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SD_TIMER=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/timer_hal.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/timer_hal_weak.c
endif

include make/module.mk
