LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_MMC_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SDRV_MMC
MODULE_SRCS += \
	$(LOCAL_DIR)/src/mmc_hal.c \
	$(LOCAL_DIR)/src/mmc_dwcmshc.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/mmc_hal_weak.c
endif



include make/module.mk
