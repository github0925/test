LOCAL_MODULE_NAME := libvpu.so

CUR_DIR := $(shell pwd)
BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CUR_DIR)
endif

OBJ_DIR = $(BUILD_DIR)/libvpu

# create obj dir
$(shell if [ ! -e $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi)

VPATH := \
	$(CUR_DIR)/vdi/linux: \
	$(CUR_DIR)/vpuapi: \
	$(CUR_DIR)/vpuapi/coda9: \
	$(CUR_DIR)/vpuapi/wave/common: \
	$(CUR_DIR)/vpuapi/wave/wave4: \
	$(CUR_DIR)/vpuapi/wave/wave5: \
	$(CUR_DIR)/vpuapi/wave/coda7q:

SOURCES := \
	$(CUR_DIR)/vdi/linux/vdi.c \
	$(CUR_DIR)/vdi/linux/vdi_osal.c \
	$(CUR_DIR)/vpuapi/vpuapi.c \
	$(CUR_DIR)/vpuapi/product.c \
	$(CUR_DIR)/vpuapi/coda9/coda9.c \
	$(CUR_DIR)/vpuapi/wave/common/common.c \
	$(CUR_DIR)/vpuapi/wave/coda7q/coda7q.c \
	$(CUR_DIR)/vpuapi/wave/wave4/wave4.c \
	$(CUR_DIR)/vpuapi/vpuapifunc.c \
	$(CUR_DIR)/vpuapi/wave/wave5/wave5.c

CFLAGS += \
	-I$(CUR_DIR)/vdi \
	-I$(CUR_DIR)/vpuapi

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
