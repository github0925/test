#######################################################
# Building the test app
# encoderapis_test -> vpuencoder
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

HELPER_DIR := ../libvpu/helper

LOCAL_SRC_FILES := \
        src/encoder.c

LOCAL_SRC_FILES += \
        sample/encoderapis_test/encoderapis_test.c \
        sample/encoderapis_test/tqueue.c \
        $(HELPER_DIR)/misc/platform.c \
        $(HELPER_DIR)/main_helper.c

LOCAL_SHARED_LIBRARIES := \
        libvpu \
        libutils \
        liblog

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
        $(LOCAL_PATH)/include \
        $(LOCAL_PATH)/../libvpu/vpuapi \
        $(LOCAL_PATH)/$(HELPER_DIR) \
        $(LOCAL_PATH)/sample/encoderapis_test

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := vpuencoder
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

#######################################################
# Building the test app
# SubStreamDemo
include $(CLEAR_VARS)

HELPER_DIR := ../libvpu/helper

LOCAL_SRC_FILES := \
        src/encoder.c

LOCAL_SRC_FILES += \
        sample/subStreamDemo/ion_memorymanager.c \
        sample/subStreamDemo/subStreamDemo.c \
        sample/subStreamDemo/g2dapi.c \
        $(HELPER_DIR)/misc/platform.c \
        $(HELPER_DIR)/main_helper.c

LOCAL_SHARED_LIBRARIES := \
        libvpu \
        libutils \
        liblog \
        libion

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
        $(LOCAL_PATH)/include \
        $(LOCAL_PATH)/../libvpu/vpuapi \
        $(LOCAL_PATH)/$(HELPER_DIR) \
        $(LOCAL_PATH)/sample/subStreamDemo \
        system/core/libion/include/ion \
        system/core/libion \
        kernel/include/uapi/drm

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := subStreamDemo
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

########################################################
# Building the library
include $(CLEAR_VARS)

HELPER_DIR := ../libvpu/helper

LOCAL_SRC_FILES := \
        src/encoder.c \
        $(HELPER_DIR)/main_helper.c \
        src/fake_helper.c \

LOCAL_SHARED_LIBRARIES := \
        libvpu \
        libutils \
        liblog

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
        $(LOCAL_PATH)/include \
        $(LOCAL_PATH)/../libvpu/vpuapi \
        $(LOCAL_PATH)/$(HELPER_DIR) \

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libvideo
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
