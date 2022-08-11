LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/mipi_mtf101_serdes_1920x1200_2_lcd.c

include make/module.mk
