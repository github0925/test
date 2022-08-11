LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/lvds_hsd156_serdes_1920x1080_lcd.c

include make/module.mk
