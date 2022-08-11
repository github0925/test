# Building the omxil-vpu-component
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

HELPER_DIR := ../../libvpu/helper

LOCAL_SRC_FILES := \
		src/library_entry_point.c \
		src/omx_vpudec_component.c \
		src/omx_vpuenc_component.c \
		src/omx_utils.c \
		src/android_support.cpp \
		src/omx_color_aspect_wrapper.cpp \
		src/omx_hdr_info_wrapper.cpp \

LOCAL_SRC_FILES += \
		src/fake_helper.c \
		$(HELPER_DIR)/main_helper.c \

LOCAL_SHARED_LIBRARIES := \
		libvpu \
		libomxil-bellagio \
		libutils \
		libcutils \
		libui \
		libdl \
		liblog \
		libyuv \
		libstagefright_foundation \

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \> 28)))
LOCAL_SHARED_LIBRARIES += libprocessgroup
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
		$(LOCAL_PATH)/../libomxil-bellagio-0.9.3/include \
		$(LOCAL_PATH)/../libomxil-bellagio-0.9.3/src \
		$(LOCAL_PATH)/../libomxil-bellagio-0.9.3/src/base \
		$(LOCAL_PATH)/../../libvpu/vpuapi \
		$(LOCAL_PATH)/$(HELPER_DIR) \
		$(TOP)/frameworks/av/media/libstagefright \
		$(TOP)/frameworks/native/include/media/hardware \
		$(TOP)/hardware/libhardware/modules/gralloc \
		$(TOP)/external/libyuv/files/include \
		$(TOP)/system/core/libion/kernel-headers \

LOCAL_C_INCLUDES += \
	$(TOP)/vendor/semidrive/gpu/android/include/public/ \
	$(TOP)/vendor/semidrive/gpu/android/include/public/powervr \

LOCAL_HEADER_LIBRARIES := \
		libhardware_headers \
		media_plugin_headers \
		libstagefright_foundation_headers \

LOCAL_CFLAGS := -DCONFIG_DEBUG_LEVEL=1 -DANDROID_PLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION) -DSUPPORT_ENCODER -DFILE_DEC_DUMP -DFILE_ENC_DUMP
#check whether PLATFORM_SDK_VERSION is android5.0
ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \< 21)))
LOCAL_LDLIBS += -lpthread
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libomxvpu
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
