LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
		linenoise/linenoise.c \
		sdshell.c \

LOCAL_CFLAGS += -O2 -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-mismatched-new-delete

LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/linenoise/

LOCAL_C_INCLUDES += bionic

LOCAL_MODULE:= sdshell

include $(BUILD_EXECUTABLE)
