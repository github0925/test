LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/src/lt9611.c \
	$(LOCAL_DIR)/src/lt9611_i2c.c
