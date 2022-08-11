TARGET_TEST_MAIN := gstreamer_test_main
TARGET_TEST_MODULE := gstreamer_test_module

CUR_DIR := $(shell pwd)
BUILD_DIR = $(WORKDIR_CUR)

DIR_EXIST = $(shell if [ -d $(BUILD_DIR) ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(DIR_EXIST), exist)
    BUILD_DIR := $(CUR_DIR)
endif

OBJ_DIR = $(BUILD_DIR)/gstreamer_test
#create obj dir
$(shell if [ ! -e $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi)

OBJS_TEST_MAIN := $(OBJ_DIR)/gstreamer_test_main.o
OBJS_TEST_MODULE := $(OBJ_DIR)/gstreamer_test_module.o           \
	$(OBJ_DIR)/gstreamer_test_lib.o              \
	$(OBJ_DIR)/gstreamer_test_elements.o         \
	$(OBJ_DIR)/gstreamer_test_cmd_parser.o       \
	$(OBJ_DIR)/gstreamer_test_case_api.o         \
	$(OBJ_DIR)/gstreamer_test_player_utils.o     \
	$(OBJ_DIR)/gstreamer_test_player.o

CXXFLAGS += \
	-I$(CUR_DIR)
CXXFLAGS += -std=c++11 -fPIC -fno-rtti -rdynamic

EXE_LDFLAGS += \
	-Wl,-Bstatic \
	-ljsoncpp_static \
	-lgtest \
	-Wl,-Bdynamic \
	-ldl \
	-lpthread \
	-ljsoncpp \
	-lglib-2.0 \
	-lgobject-2.0 \
	-lgstreamer-1.0 \
	$(CXXFLAGES)


all:$(TARGET_TEST_MAIN) $(TARGET_TEST_MODULE)

$(TARGET_TEST_MAIN): $(OBJS_TEST_MAIN)
	$(CXX) $^ -o $(BUILD_DIR)/$@ $(EXE_LDFLAGS)

$(TARGET_TEST_MODULE): $(OBJS_TEST_MODULE)
	$(CXX) $^ -o $(BUILD_DIR)/$@ $(EXE_LDFLAGS)

$(OBJ_DIR)/%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $^ -o $@

clean:
	-rm -rf $(OBJ_DIR) $(BUILD_DIR)/$(TARGET_TEST_MAIN) $(BUILD_DIR)/$(TARGET_TEST_MODULE)
