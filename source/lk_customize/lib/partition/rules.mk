LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	lib/storage_device \
	external/lib/cksum

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/partition.c  \
	$(LOCAL_DIR)/partition_parser.c  \
	$(LOCAL_DIR)/ab_partition_parser.c  \

include make/module.mk
