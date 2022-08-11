LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_CFLAGS += $(CFLAGS)
CFLAGS :=

MODULE_SRCS += $(LOCAL_DIR)/eeprom_test.c \
				$(LOCAL_DIR)/eeprom.c

include make/module.mk
