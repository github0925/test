LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/mipi_hsd123_serdes_1920x720_v9_lcd.c

include make/module.mk
