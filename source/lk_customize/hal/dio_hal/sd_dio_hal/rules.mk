LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_DIO_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SDRV_DIO
MODULE_SRCS += \
	$(LOCAL_DIR)/src/hal_dio.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/hal_dio_weak.c
endif

include make/module.mk
