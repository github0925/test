LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/include/ $(GLOBAL_INCLUDES)

MODULE_SRCS += \
	$(LOCAL_DIR)/src/v4l2.c \
	$(LOCAL_DIR)/src/sd_csi.c

ifeq ($(SUPPORT_MIPICSI_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/sd_mipi_csi2.c
endif

include make/module.mk
