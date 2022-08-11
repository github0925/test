LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_CLKGEN_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SD_CLKGEN=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/clkgen_hal.c \
	$(LOCAL_DIR)/src/clkgen_hal_ip_test.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/clkgen_hal_weak.c \
	$(LOCAL_DIR)/src/clkgen_hal_ip_test_weak.c
endif

include make/module.mk
