LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/sms_storage.c

MODULE_DEPS += \
    lib/partition \
    lib/storage_device \
    lib/libavb \

GLOBAL_INCLUDES += \
    $(LOCAL_DIR)/include \


include make/module.mk
