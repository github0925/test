LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/lvds_kd070_serdes_1024x600_lcd.c

include make/module.mk
