LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ifeq ($(SUPPORT_PLL_SDDRV),true)
GLOBAL_INCLUDES += $(LOCAL_DIR)/inc
MODULE_SRCS += \
	$(LOCAL_DIR)/src/pll_hal.c
GLOBAL_DEFINES += ENABLE_SD_PLL=1

SUPPORT_PLL_SDDRV_DEBUG ?= false
ifeq ($(SUPPORT_PLL_SDDRV_DEBUG),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/pll_debugcmd.c
endif

endif

include make/module.mk
