LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/lvds_hsd123_serdes_1920x720_lcd.c

include make/module.mk
