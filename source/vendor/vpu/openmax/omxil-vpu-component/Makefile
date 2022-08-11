LOCAL_MODULE_NAME := libomxvpu.so

CUR_DIR := $(shell pwd)
SRC_DIR := $(CUR_DIR)/src
VPU_DIR := $(CUR_DIR)/../../libvpu
HELP_DIR := $(CUR_DIR)/../../libvpu/helper
OMXIL_DIR := $(CUR_DIR)/../libomxil-bellagio-0.9.3
BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CUR_DIR)
endif

OBJ_DIR = $(BUILD_DIR)/libomxvpu
# create obj dir
$(shell if [ ! -e $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi)

VPATH := \
	$(SRC_DIR): \
	$(HELP_DIR): \
	$(VPU_DIR): \
	$(OMXIL_DIR)

SOURCES := \
	$(SRC_DIR)/library_entry_point.c \
	$(SRC_DIR)/omx_vpudec_component.c \
	$(SRC_DIR)/omx_vpuenc_component.c \
	$(SRC_DIR)/omx_utils.c \
	$(SRC_DIR)/fake_helper.c \
	$(HELP_DIR)/main_helper.c

CFLAGS += \
	-I$(SRC_DIR) \
	-I$(OMXIL_DIR) \
	-I$(OMXIL_DIR)/src \
	-I$(OMXIL_DIR)/src/base \
	-I$(OMXIL_DIR)/include \
	-I$(VPU_DIR)/vpuapi \
	-I$(HELP_DIR)

CFLAGS += \
	-fPIC \
	-Dlinux -DSUPPORT_ENCODER \
	-DCONFIG_DEBUG_LEVEL=15

LDFLAGS += \
	-shared \
	-lpthread \
	-lvpu \
	-lomxil-bellagio \
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
