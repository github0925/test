# Makefile
#
# Copyright (C) 2020 Semidrive Technology Co., Ltd.
#
# Description:
#
# Revision Histrory:
# -----------------
# 1.1, 20/12/2020  chentianming <tianming.chen@semidrive.com> create this file
#

LIB_JPU_MODULE_NAME := libjpu.so
LIB_JPU_MIDDLEWARE_NAME := libmiddlewarejpu.so
CURDIR := $(shell pwd)

BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CURDIR)
endif

src := \
	../jdi/src/jdi.c \
	src/jpuapi.c \
	src/jpuapifunc.c \
	src/jpulog.c

middleware_src := \
	../jdi/src/jdi.c \
	src/jpuapi.c \
	src/jpuapifunc.c \
	src/jpulog.c \
	middleware/jpu_middleware_api.c \
	middleware/drmutils.c

CFLAGS += \
	-I$(CURDIR)/../jdi/include \
	-I$(CURDIR)/src  \
	-I$(CURDIR)/include \
	-I$(CURDIR)/middleware

CFLAGS += -Dlinux

OBJ := $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o, $(notdir $(src))))
RAW_OBJ := $(patsubst %.c, %.o, $(src))
TARGET := $(BUILD_DIR)/$(LIB_JPU_MODULE_NAME)

MIDDLEWARE_OBJ := $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o, $(notdir $(middleware_src))))
MIDDLEWARE_RAW_OBJ := $(patsubst %.c, %.o, $(middleware_src))
MIDDLEWARE_TARGET := $(BUILD_DIR)/$(LIB_JPU_MIDDLEWARE_NAME)

all:$(TARGET) $(MIDDLEWARE_TARGET)

$(TARGET):$(OBJ)
	$(CC) -fPIC $(CFLAGES) -shared $^ -o $@ -lpthread

$(MIDDLEWARE_TARGET):$(MIDDLEWARE_OBJ)
	$(CC) -fPIC $(CFLAGES) -shared $^ -o $@ -lpthread -ldrm

$(OBJ):$(RAW_OBJ)

$(MIDDLEWARE_OBJ):$(MIDDLEWARE_RAW_OBJ)

%.o:%.c
	$(CC) -c $(CFLAGS) -fPIC $< -o $(BUILD_DIR)/$(notdir $@)

clean:
	rm -f $(OBJ) $(TARGET) $(MIDDLEWARE_OBJ) $(MIDDLEWARE_TARGET)

.PHONY: all clean
