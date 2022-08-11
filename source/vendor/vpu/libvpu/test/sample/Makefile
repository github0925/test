LOCAL_MODULE_NAME := vputest

CUR_DIR := $(shell pwd)
VPULIB_DIR := $(CUR_DIR)/../../
VPUAPI_DIR := $(CUR_DIR)/../../vpuapi
HELPER_DIR := $(CUR_DIR)/../../helper
FFMPEG := $(CUR_DIR)/../../../ffmpeg
SDKTEST_DIR := $(CUR_DIR)/../sdk_test

BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CUR_DIR)
endif

OBJ_DIR = $(BUILD_DIR)/vputest
# create obj dir
$(shell if [ ! -e $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi)

VPATH := \
	$(HELPER_DIR): \
	$(HELPER_DIR)/bitstream: \
	$(HELPER_DIR)/comparator: \
	$(HELPER_DIR)/display: \
	$(HELPER_DIR)/misc: \
	$(HELPER_DIR)/yuv: \
	$(SDKTEST_DIR)/src: \
	$(VPULIB_DIR)

SOURCES := \
	$(CUR_DIR)/cmd_vpu.c \
	$(CUR_DIR)/main_coda980_dec_test.c \
	$(CUR_DIR)/main_coda980_enc_test.c \
	$(CUR_DIR)/main_w4_dec_test.c \
	$(SDKTEST_DIR)/src/test_case.c \
	$(SDKTEST_DIR)/src/test_sdk.c \
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

CFLAGS += \
	-I$(VPUAPI_DIR) \
	-I$(HELPER_DIR) \
	-I$(FFMPEG)/include \
	-I$(SDKTEST_DIR)/inc

CFLAGS += \
	-fPIC \
	-DSUPPORT_FFMPEG_DEMUX \
	-Dlinux

LDFLAGS += \
	-lvpu \
	-lavcodec\
	-lavutil\
	-lavformat\
	-lpthread

OBJS := $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(notdir $(SOURCES))))
TARGET := $(LOCAL_MODULE_NAME)

$(TARGET):$(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	-rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: clean
