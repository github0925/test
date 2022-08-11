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

target := jputest
target_multi := multitest

curdir := $(shell pwd)

HELPER_DIR := $(curdir)/../../helper
FFMPEG_DIR := $(curdir)/../../../ffmpeg
JPUAPI_DIR := $(curdir)/../../jpuapi
JPUJDI_DIR := $(curdir)/../../jdi

VPATH := \
	$(curdir): \
	$(HELPER_DIR)/src: \

src := \
	$(curdir)/jpg_dec_test.c  \
	$(curdir)/jpg_enc_test.c \
	$(curdir)/jputest.c  \
	$(HELPER_DIR)/src/jpuhelper.c   \
	$(HELPER_DIR)/src/bitstreamfeeder.c    \
	$(HELPER_DIR)/src/bitstreamwriter.c  \
	$(HELPER_DIR)/src/bsfeeder_fixedsize_impl.c  \
	$(HELPER_DIR)/src/bsfeeder_framesize_impl.c  \
	$(HELPER_DIR)/src/datastructure.c  \
	$(HELPER_DIR)/src/platform.c  \
	$(HELPER_DIR)/src/yuv_feeder.c

src_multi:= \
	$(curdir)/jpg_dec_test.c  \
	$(curdir)/jpg_enc_test.c \
	$(curdir)/multi_instance_test.c  \
	$(HELPER_DIR)/src/jpuhelper.c   \
	$(HELPER_DIR)/src/bitstreamfeeder.c    \
	$(HELPER_DIR)/src/bitstreamwriter.c  \
	$(HELPER_DIR)/src/bsfeeder_fixedsize_impl.c  \
	$(HELPER_DIR)/src/bsfeeder_framesize_impl.c  \
	$(HELPER_DIR)/src/datastructure.c  \
	$(HELPER_DIR)/src/platform.c  \
	$(HELPER_DIR)/src/yuv_feeder.c

CFLAGS += \
	-I$(HELPER_DIR)/include \
	-I$(FFMPEG_DIR)/include \
	-I$(JPUAPI_DIR)/src  \
	-I$(JPUAPI_DIR)/include \
	-I$(JPUJDI_DIR)/include

CFLAGS += -Dlinux

obj := $(patsubst %.c, %.o, $(notdir ${src}))
obj_multi := $(patsubst %.c, %.o, $(notdir ${src_multi}))

all:multitest jputest
	$(CC) -fPIC $(CFLAGES) -o $(target_multi) $(obj_multi) $(LDFLAGS) -ljpu -lpthread
	$(CC) -fPIC $(CFLAGES) -o $(target) $(obj) $(LDFLAGS) -ljpu -lpthread

jputest:
	$(CC) -c $(CFLAGS) -fPIC $(src)

multitest:
	$(CC) -c $(CFLAGS) -fPIC $(src_multi)

clean:
	rm *.o $(target) $(target_multi)

.PHONY:
	all clean
