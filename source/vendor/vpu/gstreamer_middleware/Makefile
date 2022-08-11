TEST_MAIN_TARGET_NAME := gstreamer_codec_test_main
LIB_GSTREAMER_MIDDLEWARE_NAME = libgstreamermiddleware.so

CUR_DIR := $(shell pwd)
BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CUR_DIR)
endif

$(warning $(BUILD_DIR))

TEST_MAIN_DIR := $(CUR_DIR)/test/
LIB_COMMON_DIR := $(CUR_DIR)/common/
LIB_PLAYER_DIR := $(CUR_DIR)/player/
LIB_RECORD_DIR := $(CUR_DIR)/recorder/

TEST_MAIN_SRC := \
	$(TEST_MAIN_DIR)/gstreamer_codec_test.cc \
	$(TEST_MAIN_DIR)/test_codec_sync_source.cc \
	$(TEST_MAIN_DIR)/test_encode_async_dma_source.cc

LIB_GSTREAMER_MIDDLEWARE_SRC_C_FILES := \
	$(LIB_COMMON_DIR)/gstreamer_middleware_api.c \
	$(LIB_COMMON_DIR)/gstreamer_middleware_pool.c \
	$(LIB_RECORD_DIR)/gstreamer_middleware_encode.c \
	$(LIB_PLAYER_DIR)/gstreamer_middleware_decode.c \

LIB_GSTREAMER_MIDDLEWARE_SRC_CC_FILES := \
	$(LIB_COMMON_DIR)/hw_buffer_utils.cc \

CXXFLAGS += \
	-std=c++11 -fPIC -fno-rtti -rdynamic

CFLAGS += \
	-shared -fPIC \
	-I$(TEST_MAIN_DIR) \
	-I$(LIB_COMMON_DIR) \
	-I$(LIB_PLAYER_DIR) \
	-I$(LIB_RECORD_DIR)

LDFLAGS += \
	-Wl,-Bdynamic \
	-ldl \
	-lpthread \
	-lglib-2.0 \
	-lgobject-2.0 \
	-lgstreamer-1.0 \
	-lgstvideo-1.0 \
	-lgstallocators-1.0 \
	-ldrm \
	-lminigbm \
	-lhwconverter \
	-L$(BUILD_DIR)

TEST_MAIN_OBJ := $(addprefix $(BUILD_DIR)/, $(patsubst %.cc, %.o, $(notdir $(TEST_MAIN_SRC))))
TEST_MAIN_RAW_OBJ = $(patsubst %.cc, %.o, $(TEST_MAIN_SRC))
TEST_MAIN_TARGET = $(BUILD_DIR)/$(TEST_MAIN_TARGET_NAME)

LIB_GSTREAMER_MIDDLEWARE_OBJ := $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o, $(notdir $(LIB_GSTREAMER_MIDDLEWARE_SRC_C_FILES))))
LIB_GSTREAMER_MIDDLEWARE_OBJ += $(addprefix $(BUILD_DIR)/, $(patsubst %.cc, %.o, $(notdir $(LIB_GSTREAMER_MIDDLEWARE_SRC_CC_FILES))))
LIB_GSTREAMER_MIDDLEWARE_RAW_OBJ := $(patsubst %.c, %.o, $(LIB_GSTREAMER_MIDDLEWARE_SRC_C_FILES))
LIB_GSTREAMER_MIDDLEWARE_RAW_OBJ += $(patsubst %.cc, %.o, $(LIB_GSTREAMER_MIDDLEWARE_SRC_CC_FILES))
LIB_GSTREAMER_MIDDLEWARE = $(BUILD_DIR)/$(LIB_GSTREAMER_MIDDLEWARE_NAME)

all:$(TEST_MAIN_TARGET)

$(TEST_MAIN_TARGET):$(LIB_GSTREAMER_MIDDLEWARE) $(TEST_MAIN_OBJ)
	$(CXX) $(CFLAGES) $^ $(LDFLAGS) -lgstreamermiddleware -o $@

$(LIB_GSTREAMER_MIDDLEWARE):$(LIB_GSTREAMER_MIDDLEWARE_OBJ)
	$(CC) -shared -fPIC $(CFLAGES)  $^ $(LDFLAGS) -o $@

$(LIB_GSTREAMER_MIDDLEWARE_OBJ):$(LIB_GSTREAMER_MIDDLEWARE_RAW_OBJ)

$(TEST_MAIN_OBJ):$(TEST_MAIN_RAW_OBJ)

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $(BUILD_DIR)/$(notdir $@)

%.o:%.cc
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $< -o $(BUILD_DIR)/$(notdir $@)

clean:
	-rm -rf $(TEST_MAIN_OBJ) $(TEST_MAIN_TARGET) $(LIB_GSTREAMER_MIDDLEWARE_OBJ) $(LIB_GSTREAMER_MIDDLEWARE)

.PHONY: all clean
