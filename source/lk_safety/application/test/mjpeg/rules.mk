LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
        $(LOCAL_DIR)/mjpeg_test.c

MODULE_DEPS += \
	lib/res_loader \

$(shell cp $(LOCAL_DIR)/mjpeg_green_pic.bin $(LOCAL_DIR)/../../../res/early_app/BootAnimation/mjpeg_green_pic.bin)

include make/module.mk

