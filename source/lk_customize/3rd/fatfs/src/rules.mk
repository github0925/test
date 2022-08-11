LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_DEPS += lib/sdunittest

MODULE_SRCS += 	$(LOCAL_DIR)/diskio_sdrv.c \
			$(LOCAL_DIR)/ff.c \
			$(LOCAL_DIR)/ffsystem.c \
			$(LOCAL_DIR)/ffunicode.c \
#			$(LOCAL_DIR)/ff_unittest.c

include make/module.mk
