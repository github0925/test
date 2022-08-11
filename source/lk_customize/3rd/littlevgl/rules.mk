
LVGL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LVGL_DIR)
LVGL_DIR_NAME := lvgl-release-v7
# LVGL_DIR_NAME := lvgl
GLOBAL_INCLUDES += $(LVGL_DIR)
GLOBAL_INCLUDES += $(LVGL_DIR)/$(LVGL_DIR_NAME)
GLOBAL_DEFINES += LV_CONF_INCLUDE_SIMPLE=1

include $(LVGL_DIR)/$(LVGL_DIR_NAME)/lvgl.mk

ifeq ($(SUPPORT_LVGL_EXAMPLES),true)
MODULE_DEPS += 3rd/littlevgl/lv_examples-release-v7
endif
include make/module.mk

