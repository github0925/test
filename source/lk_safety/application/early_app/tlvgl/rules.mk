LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc \


MODULE_SRCS += \
	$(LOCAL_DIR)/src/tlvgl.c \
	$(LOCAL_DIR)/res/LeftTurn.c \
	$(LOCAL_DIR)/res/rightTurn.c \
	$(LOCAL_DIR)/src/stb_decoder.c \
	$(LOCAL_DIR)/src/lv_two_flashing.c \
	$(LOCAL_DIR)/src/radio.c \
	$(LOCAL_DIR)/src/lv_demo_6btn.c \

$(info MODULE_OBJS = $(MODULE_OBJS))

GLOBAL_DEFINES += ENABLE_TLVGL

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
