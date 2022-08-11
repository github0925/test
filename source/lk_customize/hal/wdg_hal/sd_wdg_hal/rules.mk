LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \
	
ifeq ($(SUPPORT_WDG_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SD_WDG=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/wdg_hal.c \
	$(LOCAL_DIR)/src/wdg_hal_ip_test.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/wdg_hal_weak.c \
	$(LOCAL_DIR)/src/wdg_hal_weak_ip_test.c
endif

ifeq ($(SUPPORT_WDG_NEED_INIT),true)
GLOBAL_DEFINES += ENABLE_SD_WDG_INIT=1
endif

include make/module.mk
