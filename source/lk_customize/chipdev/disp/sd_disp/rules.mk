LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/inc
GLOBAL_INCLUDES += $(LOCAL_DIR)/dsi/inc

MODULE_CFLAGS += -DDISP_VERSION=\"$(shell date +%Y-%m-%d-%H:%M:%S)\"


include $(LOCAL_DIR)/libs/rules.mk

ifeq ($(SUPPORT_DISP_SDDRV),true)
#GLOBAL_DEFINES += DISP_PERF_DEBUG
MODULE_SRCS += \
	$(LOCAL_DIR)/dsi/dwc_mipi_dsi_host.c \
	$(LOCAL_DIR)/dsi/disp_dsi_api.c \
	$(LOCAL_DIR)/dsi/disp_dsi.c \
	$(LOCAL_DIR)/core/disp_drv.c \
	$(LOCAL_DIR)/core/disp_interface.c \
	$(LOCAL_DIR)/core/disp_panel.c

endif

#include $(LOCAL_DIR)/libs/rules.mk

include make/module.mk
