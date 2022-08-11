LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/cluster_preloader.c \

MODULE_DEPS += \
    lib/partition \
    lib/libavb \
    lib/verified_boot \
    framework/lib/mem_image \

include make/module.mk
