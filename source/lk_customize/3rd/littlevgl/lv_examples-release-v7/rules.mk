LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += exdev/display

LVGL_DIR := $(LOCAL_DIR)/..

GLOBAL_INCLUDES += $(LVGL_DIR) $(LOCAL_DIR)
GLOBAL_DEFINES += LV_EX_CONF_INCLUDE_SIMPLE=1 LV_LVGL_H_INCLUDE_SIMPLE=1
MODULE_CFLAGS += -Wno-sign-compare

MODULE_SRCS := $(shell find -L $(LOCAL_DIR)/src/lv_ex_get_started -name \*.c)
MODULE_SRCS += $(shell find -L $(LOCAL_DIR)/src/lv_demo_stress -name \*.c)
MODULE_SRCS += $(shell find -L $(LOCAL_DIR)/src/lv_demo_widgets -name \*.c)

MODULE_SRCS += $(LOCAL_DIR)/app_main.c
CSRCS :=

include make/module.mk

