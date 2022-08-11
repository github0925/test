LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#Building vpurun binary which will be placed in the /system/bin folder


#LOCAL_CFLAGS += -DPLATFORM_LINUX

HELPER_DIR := ../../helper

LOCAL_SRC_FILES := \
	cmd_vpu.c \
	main_coda980_dec_test.c \
	main_coda980_enc_test.c \
	main_w4_dec_test.c \
	../sdk_test/src/test_case.c \
	../sdk_test/src/test_sdk.c \
	$(HELPER_DIR)/main_helper.c \
	$(HELPER_DIR)/vpuhelper.c                         \
	$(HELPER_DIR)/bitstream/bitstreamfeeder.c         \
	$(HELPER_DIR)/bitstream/bitstreamreader.c         \
	$(HELPER_DIR)/bitstream/bsfeeder_fixedsize_impl.c \
	$(HELPER_DIR)/bitstream/bsfeeder_framesize_impl.c \
	$(HELPER_DIR)/bitstream/bsfeeder_size_plus_es_impl.c \
	$(HELPER_DIR)/comparator/bin_comparator_impl.c    \
	$(HELPER_DIR)/comparator/comparator.c             \
	$(HELPER_DIR)/comparator/md5_comparator_impl.c    \
	$(HELPER_DIR)/comparator/yuv_comparator_impl.c    \
	$(HELPER_DIR)/display/fbdev_impl.c                \
	$(HELPER_DIR)/display/simplerenderer.c            \
	$(HELPER_DIR)/misc/cnm_fpga.c                     \
	$(HELPER_DIR)/misc/cnm_video_helper.c             \
	$(HELPER_DIR)/misc/container.c                    \
	$(HELPER_DIR)/misc/datastructure.c                \
	$(HELPER_DIR)/misc/platform.c                     \
	$(HELPER_DIR)/misc/cfgParser.c                    \
	$(HELPER_DIR)/yuv/yuvfeeder.c                     \
	$(HELPER_DIR)/yuv/yuvLoaderfeeder.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../../helper \
	$(LOCAL_PATH)/../../vpuapi \
	$(LOCAL_PATH)/../sdk_test/inc

LOCAL_SHARED_LIBRARIES :=  \
	libvpu \
	libutils \
	liblog

LOCAL_CFLAGS += -DSUPPORT_FFMPEG_DEMUX
LOCAL_SHARED_LIBRARIES += \
	libavcodec \
	libavformat \
	libavutil

LOCAL_MODULE := vputest
LOCAL_PROPRIETARY_MODULE := true

LOCAL_MULTILIB := 64

include $(BUILD_EXECUTABLE)


### compile multi-instance
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DDEFIND_MULTI_INSTANCE
HELPER_DIR := ../../helper

LOCAL_SRC_FILES := \
	main_coda980_dec_test.c \
	main_coda980_enc_test.c \
	main_w4_dec_test.c \
	main_multi_instance_test.c \
	$(HELPER_DIR)/main_helper.c \
	$(HELPER_DIR)/vpuhelper.c                         \
	$(HELPER_DIR)/bitstream/bitstreamfeeder.c         \
	$(HELPER_DIR)/bitstream/bitstreamreader.c         \
	$(HELPER_DIR)/bitstream/bsfeeder_fixedsize_impl.c \
	$(HELPER_DIR)/bitstream/bsfeeder_framesize_impl.c \
	$(HELPER_DIR)/bitstream/bsfeeder_size_plus_es_impl.c \
	$(HELPER_DIR)/comparator/bin_comparator_impl.c    \
	$(HELPER_DIR)/comparator/comparator.c             \
	$(HELPER_DIR)/comparator/md5_comparator_impl.c    \
	$(HELPER_DIR)/comparator/yuv_comparator_impl.c    \
	$(HELPER_DIR)/display/fbdev_impl.c                \
	$(HELPER_DIR)/display/simplerenderer.c            \
	$(HELPER_DIR)/misc/cnm_fpga.c                     \
	$(HELPER_DIR)/misc/cnm_video_helper.c             \
	$(HELPER_DIR)/misc/container.c                    \
	$(HELPER_DIR)/misc/datastructure.c                \
	$(HELPER_DIR)/misc/platform.c                     \
	$(HELPER_DIR)/misc/cfgParser.c                    \
	$(HELPER_DIR)/yuv/yuvfeeder.c                     \
	$(HELPER_DIR)/yuv/yuvLoaderfeeder.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../../helper \
	$(LOCAL_PATH)/../../vpuapi \
	$(LOCAL_PATH)/../../vdi

LOCAL_SHARED_LIBRARIES :=  \
	libvpu \
	libutils \
	liblog

LOCAL_CFLAGS += -DSUPPORT_FFMPEG_DEMUX
LOCAL_SHARED_LIBRARIES += \
	libavcodec \
	libavformat \
	libavutil

LOCAL_MODULE := vpumultitest
LOCAL_PROPRIETARY_MODULE := true

LOCAL_MULTILIB := 64

include $(BUILD_EXECUTABLE)

