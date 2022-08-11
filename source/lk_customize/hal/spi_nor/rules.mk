LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_SPINOR_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SDRV_SPINOR
MODULE_SRCS += \
	$(LOCAL_DIR)/src/spi_nor_hal.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/spi_nor_hal_weak.c
endif

include make/module.mk
