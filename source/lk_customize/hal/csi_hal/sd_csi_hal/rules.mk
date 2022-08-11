LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \
     
ifeq ($(SUPPORT_CSI_SDDRV),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/csi_hal.c

ifeq ($(SUPPORT_MIPICSI_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/mipi_csi_hal.c
endif
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/csi_hal_weak.c
endif

include make/module.mk
