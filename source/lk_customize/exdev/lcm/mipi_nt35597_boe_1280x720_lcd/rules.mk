LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/mipi_nt35597_boe_1280x720_lcd.c

include make/module.mk
