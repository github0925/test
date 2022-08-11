LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc/

MODULE_SRCS += \
	$(LOCAL_DIR)/src/sem.c \
	$(LOCAL_DIR)/src/eic.c \
	$(LOCAL_DIR)/src/bipc.c \

include make/module.mk
