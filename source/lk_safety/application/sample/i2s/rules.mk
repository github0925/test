LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/include/ $(GLOBAL_INCLUDES) \

MODULE_SRCS += \
	$(LOCAL_DIR)/i2s_test_on_board.c \
#	$(LOCAL_DIR)/cs42888-i2c.c \

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
