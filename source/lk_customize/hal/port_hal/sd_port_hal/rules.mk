LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_PORT_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SDRV_PORT
MODULE_SRCS += \
	$(LOCAL_DIR)/src/hal_port.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/hal_port_weak.c
endif

MODULE_DEPS += \
        lib/system_config \

include make/module.mk
