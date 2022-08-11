LOCAL_PATH := $(call my-dir)

#Building omxregister-bellagio binary which will be placed in the /system/bin folder
# include $(CLEAR_VARS)

# LOCAL_SRC_FILES := \
	src/common.c \
	src/omxregister.c \

# LOCAL_MODULE_TAGS := optional
# LOCAL_MODULE := omxregister-bellagio
# LOCAL_PROPRIETARY_MODULE := true
# LOCAL_MODULE_RELATIVE_PATH := hw


# LOCAL_CFLAGS := -DOMXILCOMPONENTSPATH=\"/system/lib\" -DCONFIG_DEBUG_LEVEL=255

# LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/src \
	$(LOCAL_PATH)/include \
	$(TOP)/frameworks/av/include/media/stagefright/ \

# LOCAL_SHARED_LIBRARIES := \
        libutils \
        libdl \
	liblog

# include $(BUILD_EXECUTABLE)

# Building the libomxil-bellagio
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/common.c \
	src/content_pipe_file.c \
	src/content_pipe_inet.c \
	src/omx_create_loaders_linux.c \
	src/omxcore.c \
	src/omx_reference_resource_manager.c \
	src/omx_resource_config.c \
	src/queue.c \
	src/st_static_component_loader.c \
	src/tsemaphore.c \
	src/utils.c \
	src/base/OMXComponentRMExt.c \
	src/base/omx_base_audio_port.c \
	src/base/omx_base_clock_port.c \
	src/base/omx_base_component.c \
	src/base/omx_base_filter.c \
	src/base/omx_base_image_port.c \
	src/base/omx_base_port.c \
	src/base/omx_base_sink.c \
	src/base/omx_base_source.c \
	src/base/omx_base_video_port.c \
	src/core_extensions/OMXCoreRMExt.c

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libomxil-bellagio
LOCAL_PROPRIETARY_MODULE := true
# LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_CFLAGS := -DCONFIG_DEBUG_LEVEL=32


LOCAL_STATIC_LIBRARIES := libperfapi
#check whether PLATFORM_SDK_VERSION is android5.0
ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \< 21)))
LOCAL_LDLIBS += -lpthread
endif

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	libdl \
	liblog

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/src \
	$(LOCAL_PATH)/src/base \
	$(TOP)/frameworks/av/include/media/stagefright \
	$(TOP)/frameworks/native/include/media/openmax \

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := .omxregister
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/vendor/etc
LOCAL_SRC_FILES := .omxregister
include $(BUILD_PREBUILT)
