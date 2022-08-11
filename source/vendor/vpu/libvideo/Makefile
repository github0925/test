LOCAL_MODULE_NAME := libvideo

CUR_DIR := $(shell pwd)
SRC_DIR := $(CUR_DIR)/src
VPU_DIR := $(CUR_DIR)/../libvpu
HELP_DIR := $(CUR_DIR)/../libvpu/helper
BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CUR_DIR)
endif

OBJ_DIR = $(BUILD_DIR)/libvideo
# create obj dir
$(shell if [ ! -e $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi)

VPATH := \
	$(SRC_DIR): \
	$(HELP_DIR): \
	$(VPU_DIR): \

SOURCES := \
	$(SRC_DIR)/encoder.c

CFLAGS += \
	-I$(SRC_DIR) \
	-I$(CUR_DIR)/include \
	-I$(VPU_DIR)/vpuapi \
	-I$(HELP_DIR)

CFLAGS += \
	-fPIC \

LDFLAGS += \
	-shared \
	-lvpu \
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