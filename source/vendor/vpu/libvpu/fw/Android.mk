LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := coda980.out
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/vendor/firmware
LOCAL_SRC_FILES := coda980.out
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := pissarro.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/vendor/firmware
LOCAL_SRC_FILES := pissarro.bin
include $(BUILD_PREBUILT)
