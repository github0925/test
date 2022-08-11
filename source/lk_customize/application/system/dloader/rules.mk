LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/dloader.c \
	$(LOCAL_DIR)/sd_boot_img.c \
	$(LOCAL_DIR)/semidrive_parser.c \

MODULE_DEPS += \
	lib/fastboot_common \
	lib/partition \
	lib/storage_device \
	lib/md5 \
	lib/libavb \
	external/lib/cksum \

include make/module.mk
