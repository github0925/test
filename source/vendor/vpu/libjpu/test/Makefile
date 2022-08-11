# Makefile
#
# Copyright (C) 2020 Semidrive Technology Co., Ltd.
#
# Description:
#
# Revision Histrory:
# -----------------
# 1.1, 08/12/2020  chentianming <tianming.chen@semidrive.com> create this file
#

LOCAL_MODULE_TARGET_NAME := jputest
LOCAL_MODULE_TARGET_MULTI_NAME := multitest
JPU_MUDDLEWARE_TEST_MODLE_NAME := jpumiddlewaretest

CURDIR := $(shell pwd)
BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CURDIR)
endif

$(warning $(BUILD_DIR))

HELPER_DIR := $(CURDIR)/../helper
FFMPEG_DIR := $(CURDIR)/../../ffmpeg
JPUAPI_DIR := $(CURDIR)/../jpuapi
JPUJDI_DIR := $(CURDIR)/../jdi

src := \
	$(CURDIR)/sample/jpg_dec_test.c  \
	$(CURDIR)/sample/jpg_enc_test.c \
	$(CURDIR)/sample/jputest.c  \
	$(HELPER_DIR)/src/jpuhelper.c   \
	$(HELPER_DIR)/src/bitstreamfeeder.c    \
	$(HELPER_DIR)/src/bitstreamwriter.c  \
	$(HELPER_DIR)/src/bsfeeder_fixedsize_impl.c  \
	$(HELPER_DIR)/src/bsfeeder_framesize_impl.c  \
	$(HELPER_DIR)/src/datastructure.c  \
	$(HELPER_DIR)/src/platform.c  \
	$(HELPER_DIR)/src/yuv_feeder.c

src_multi:= \
	$(CURDIR)/sample/jpg_dec_test.c  \
	$(CURDIR)/sample/jpg_enc_test.c \
	$(CURDIR)/sample/multi_instance_test.c  \
	$(HELPER_DIR)/src/jpuhelper.c   \
	$(HELPER_DIR)/src/bitstreamfeeder.c    \
	$(HELPER_DIR)/src/bitstreamwriter.c  \
	$(HELPER_DIR)/src/bsfeeder_fixedsize_impl.c  \
	$(HELPER_DIR)/src/bsfeeder_framesize_impl.c  \
	$(HELPER_DIR)/src/datastructure.c  \
	$(HELPER_DIR)/src/platform.c  \
	$(HELPER_DIR)/src/yuv_feeder.c

middleware_test_src := \
	$(CURDIR)/test_middleware/jpg_dec_middleware_test.c  \
	$(CURDIR)/test_middleware/jpg_enc_middleware_test.c \
	$(CURDIR)/test_middleware/jpu_middleware_test.c \
	$(HELPER_DIR)/src/jpuhelper.c

CFLAGS += \
	-I$(HELPER_DIR)/include \
	-I$(FFMPEG_DIR)/include \
	-I$(JPUAPI_DIR)/src  \
	-I$(JPUAPI_DIR)/include \
	-I$(JPUJDI_DIR)/include

CFLAGS += -Dlinux

OBJ := $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o, $(notdir $(src))))
RAW_OBJ = $(patsubst %.c, %.o, $(src))
TARGET := $(BUILD_DIR)/$(LOCAL_MODULE_TARGET_NAME)

OBJ_MULTI := $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o, $(notdir $(src_multi))))
RAW_OBJ_MULTI = $(patsubst %.c, %.o, $(src_multi))
TARGET_MULTI := $(BUILD_DIR)/$(LOCAL_MODULE_TARGET_MULTI_NAME)

MIDDLEWARE_TEST_OBJ := $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o, $(notdir $(middleware_test_src))))
MIDDLEWARE_TEST_RAW_OBJ := $(patsubst %.c, %.o, $(middleware_test_src))
MIDDLEWARE_TEST_TARGET := $(BUILD_DIR)/$(JPU_MUDDLEWARE_TEST_MODLE_NAME)

all:$(TARGET) $(TARGET_MULTI) $(MIDDLEWARE_TEST_TARGET)

$(TARGET):$(OBJ)
	$(CC) -fPIC $(CFLAGES) $^ -o $@ $(LDFLAGS) -ljpu -lpthread

$(TARGET_MULTI):$(OBJ_MULTI)
	$(CC) -fPIC $(CFLAGES) $^ -o $@ $(LDFLAGS) -ljpu -lpthread

$(MIDDLEWARE_TEST_TARGET):$(MIDDLEWARE_TEST_OBJ)
	$(CC) -fPIC $(CFLAGES) $^ -o $@ $(LDFLAGS) -lmiddlewarejpu -lpthread -ldrm

$(OBJ):$(RAW_OBJ)

$(OBJ_MULTI):$(RAW_OBJ_MULTI)

$(MIDDLEWARE_TEST_OBJ):$(MIDDLEWARE_TEST_RAW_OBJ)

%.o:%.c
	$(CC) -c $(CFLAGS) -fPIC $< -o $(BUILD_DIR)/$(notdir $@)

clean:
	rm -f $(OBJ) $(OBJ_MULTI) $(MIDDLEWARE_TEST_OBJ) $(TARGET) $(TARGET_MULTI) $(MIDDLEWARE_TEST_TARGET)

.PHONY:
	all clean
