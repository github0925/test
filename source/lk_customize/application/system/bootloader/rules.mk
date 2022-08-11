LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	lib/partition \
	lib/storage_device \
	lib/fdt \
	lib/libavb \
	lib/verified_boot \
	lib/boot \
	lib/libufdt

MODULE_SRCS += \
	$(LOCAL_DIR)/bootloader.c \
	$(LOCAL_DIR)/bootloader_qnx.c \
	$(LOCAL_DIR)/image_scan.c \
	$(LOCAL_DIR)/image_setup.c

include make/module.mk
