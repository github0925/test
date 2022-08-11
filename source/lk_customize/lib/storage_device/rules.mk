LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += lib/bio

MEMDISK_BASE ?= 0x80000000
MEMDISK_SIZE ?= 0x4000000

GLOBAL_DEFINES += \
	MEMDISK_BASE=$(MEMDISK_BASE) \
	MEMDISK_SIZE=$(MEMDISK_SIZE)

MODULE_SRCS += \
	$(LOCAL_DIR)/storage_device.c \
	$(LOCAL_DIR)/storage_dev_memdisk.c \
	$(LOCAL_DIR)/storage_dev_mmc.c \
	$(LOCAL_DIR)/storage_dev_ospi.c

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include \

include make/module.mk
