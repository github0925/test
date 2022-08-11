LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/include/ $(GLOBAL_INCLUDES) \

MODULE_SRCS += \
    $(LOCAL_DIR)/dw_dmac1.c \
    $(LOCAL_DIR)/dw_dmac1_mux.c \
    $(LOCAL_DIR)/dw_dma1.c \

include make/module.mk