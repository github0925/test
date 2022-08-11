LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/

MODULE := $(LOCAL_DIR)

ifeq ($(BOARDINFO_HW), true)
BOARDINFO_EEPROM ?= true
MODULE_DEPS += lib/storage_device

MODULE_SRCS += \
	$(LOCAL_DIR)/boardinfo_common.c \
	$(LOCAL_DIR)/boardinfo_hwid_hw.c

ifeq ($(BOARDINFO_EEPROM), true)
GLOBAL_DEFINES += BOARDINFO_EEPROM=1
MODULE_SRCS += $(LOCAL_DIR)/eeprom.c
else ifeq ($(BOARDINFO_GPIO), true)
GLOBAL_DEFINES += BOARDINFO_GPIO=1
MODULE_SRCS += $(LOCAL_DIR)/board_info_gpio.c
endif

endif


ifeq ($(BOARDINFO_HWID_USR), true)
MODULE_SRCS += \
        $(LOCAL_DIR)/boardinfo_hwid_usr.c
endif

include make/module.mk
