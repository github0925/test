LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/parallel_gm7123_to_vga_1920x1080_lcd.c

include make/module.mk
