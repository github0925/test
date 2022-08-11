LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/mipi_lt9611_to_hdmi_1920x1080_2_lcd.c

include make/module.mk
