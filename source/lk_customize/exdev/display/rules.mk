LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_DEPS +=	exdev/lcm
#MODULE_DEPS +=  exdev/display/tests

ifeq ($(SUPPORT_LVGL_GUI),true)
MODULE_DEPS +=  3rd/littlevgl
include  $(LOCAL_DIR)/lv_drivers/rules.mk
MODULE_SRCS += $(LOCAL_DIR)/lvgl_gui.c

GLOBAL_DEFINES += LV_EX_CONF_INCLUDE_SIMPLE=1 LV_LVGL_H_INCLUDE_SIMPLE=1
endif

ifeq ($(SUPPORT_BACKLIGHT_SVC),true)
MODULE_DEPS += framework/service/backlight/backlight_rpmsg
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/sdm_display.c \
	$(LOCAL_DIR)/backlight.c

include make/module.mk
