LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/mipi_hsd101_serdes_1280x800_lcd.c

include make/module.mk
