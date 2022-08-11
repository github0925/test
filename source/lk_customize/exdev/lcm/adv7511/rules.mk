LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

#MODULE_DEPS += \
#	lib/bytes

MODULE_SRCS += \
	$(LOCAL_DIR)/src/sd_wrapper.c \
	$(LOCAL_DIR)/src/tx_cec.c \
	$(LOCAL_DIR)/src/tx_isr.c \
	$(LOCAL_DIR)/src/tx_lib.c \
	$(LOCAL_DIR)/src/tx_multi.c \
	$(LOCAL_DIR)/src/7511_hal.c \
	$(LOCAL_DIR)/src/tx_hal.c \
	$(LOCAL_DIR)/src/wrd_hal.c \
