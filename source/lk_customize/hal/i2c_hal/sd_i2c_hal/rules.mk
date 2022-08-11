LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_I2C_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SD_I2C=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/i2c_hal.c \
	$(LOCAL_DIR)/src/i2c_hal_ip_test.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/i2c_hal_weak.c \
	$(LOCAL_DIR)/src/i2c_hal_ip_test_weak.c
endif

include make/module.mk
