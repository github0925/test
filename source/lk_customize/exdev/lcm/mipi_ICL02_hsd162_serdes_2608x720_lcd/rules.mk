LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/mipi_ICL02_hsd162_serdes_2608x720_lcd.c

include make/module.mk
