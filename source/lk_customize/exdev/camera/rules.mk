LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/src/ov5640.c \
	$(LOCAL_DIR)/src/mipi_bridge.c \
	$(LOCAL_DIR)/src/max9286.c \
	$(LOCAL_DIR)/src/ov10640.c \
	$(LOCAL_DIR)/src/max20086.c \
	$(LOCAL_DIR)/src/n4_init_ch.c

include make/module.mk


