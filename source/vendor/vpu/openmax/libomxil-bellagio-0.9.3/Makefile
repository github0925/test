LOCAL_MODULE_NAME := libomxil-bellagio.so

CUR_DIR := $(shell pwd)
SRC_DIR := $(CUR_DIR)/src
BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CUR_DIR)
endif

OBJ_DIR = $(BUILD_DIR)/libomxil
# create obj dir
$(shell if [ ! -e $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi)

VPATH := \
	$(SRC_DIR): \
	$(SRC_DIR)/base: \
	$(SRC_DIR)/core_extensions:

SOURCES := \
	$(SRC_DIR)/common.c \
	$(SRC_DIR)/content_pipe_file.c \
	$(SRC_DIR)/content_pipe_inet.c \
	$(SRC_DIR)/omx_create_loaders_linux.c \
	$(SRC_DIR)/omxcore.c \
	$(SRC_DIR)/omx_reference_resource_manager.c \
	$(SRC_DIR)/omx_resource_config.c \
	$(SRC_DIR)/queue.c \
	$(SRC_DIR)/st_static_component_loader.c \
	$(SRC_DIR)/tsemaphore.c \
	$(SRC_DIR)/utils.c \
	$(SRC_DIR)/base/OMXComponentRMExt.c \
	$(SRC_DIR)/base/omx_base_audio_port.c \
	$(SRC_DIR)/base/omx_base_clock_port.c \
	$(SRC_DIR)/base/omx_base_component.c \
	$(SRC_DIR)/base/omx_base_filter.c \
	$(SRC_DIR)/base/omx_base_image_port.c \
	$(SRC_DIR)/base/omx_base_port.c \
	$(SRC_DIR)/base/omx_base_sink.c \
	$(SRC_DIR)/base/omx_base_source.c \
	$(SRC_DIR)/base/omx_base_video_port.c \
	$(SRC_DIR)/core_extensions/OMXCoreRMExt.c

CFLAGS += \
	-I$(SRC_DIR) \
	-I$(SRC_DIR)/base \
	-I$(SRC_DIR)/../include \
	-I$(SRC_DIR)/core_extensions

CFLAGS += \
	-fPIC \
	-Dlinux

LDFLAGS += \
	-shared \
	-lpthread \
	$(CFLAGES)

OBJS := $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(notdir $(SOURCES))))
TARGET := $(LOCAL_MODULE_NAME)

$(TARGET):$(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	-rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: clean
