LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=         \
        yuvEncode.cpp    \

LOCAL_SHARED_LIBRARIES := \
        libstagefright liblog libutils libbinder libui libgui \
        libstagefright_foundation libmedia libcutils libmedia_omx

LOCAL_C_INCLUDES:= \
        frameworks/av/media/libstagefright \
        frameworks/native/include/media/openmax

LOCAL_CFLAGS += -Wno-multichar -Werror -Wall

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= yuvEncode

include $(BUILD_EXECUTABLE)