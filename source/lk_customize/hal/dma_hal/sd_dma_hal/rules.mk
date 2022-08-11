LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \
	
ifeq ($(SUPPORT_DMA_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SD_DMA=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/dma_hal.c 
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/dma_hal_weak.c 
endif

include make/module.mk
