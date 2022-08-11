LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include/

MODULE_SRCS += \
        $(LOCAL_DIR)/firewall.c

GLOBAL_DEFINES += \
        HW_NOT_MEET=1\

ifeq ($(HANDOVER_ALL_SAF), true)
GLOBAL_DEFINES += \
        ENABLE_SAF_FIREWALL=1
endif

include make/module.mk
