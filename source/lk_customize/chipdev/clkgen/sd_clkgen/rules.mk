LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

MODULE_SRCS += \
	$(LOCAL_DIR)/src/clkgen_drv.c \
	$(LOCAL_DIR)/src/clkgen_drv_test.c

include make/module.mk
