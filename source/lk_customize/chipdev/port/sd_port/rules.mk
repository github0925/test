LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

MODULE_SRCS += \
	$(LOCAL_DIR)/src/sd_port.c \
	$(LOCAL_DIR)/src/Port_PBcfg.c

include make/module.mk
