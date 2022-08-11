LOCAL_MODULE_NAME := libomxjpu.so

CUR_DIR := $(shell pwd)
SRC_DIR := $(CUR_DIR)
JPU_DIR := $(CUR_DIR)/../../libjpu
OMXIL_DIR := $(CUR_DIR)/../libomxil-bellagio-0.9.3
BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CUR_DIR)
endif

OBJ_DIR = $(BUILD_DIR)
# create obj dir
$(shell if [ ! -e $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi)

SOURCES := \
	$(SRC_DIR)/library_entry_point.c \
	$(SRC_DIR)/omx_mjpegdec_component.c

CFLAGS += \
	-I$(SRC_DIR) \
	-I$(OMXIL_DIR) \
	-I$(OMXIL_DIR)/src \
	-I$(OMXIL_DIR)/src/base \
	-I$(OMXIL_DIR)/include \
	-I$(JPU_DIR)/jpuapi/include \
	-I$(JPU_DIR)/jpuapi/middleware \
	-I$(JPU_DIR)/jdi/include

CFLAGS += \
	-fPIC

LDFLAGS += \
	-shared \
	-lpthread \
	-lmiddlewarejpu \
	-lomxil-bellagio \
	$(CFLAGES)

OBJS := $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(notdir $(SOURCES))))
TARGET := $(BUILD_DIR)/$(LOCAL_MODULE_NAME)

$(TARGET):$(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	-rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: clean
