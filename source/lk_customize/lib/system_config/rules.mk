LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/

MODULE := $(LOCAL_DIR)

ifdef SYS_CFG_MEMBASE
GLOBAL_DEFINES += \
	SYS_CFG_VALID=1
endif

MODULE_SRCS += \
	$(LOCAL_DIR)/system_configs_parse.c \

include make/module.mk
