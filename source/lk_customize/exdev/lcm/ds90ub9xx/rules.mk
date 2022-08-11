LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/src/ds90ub941as.c \
	$(LOCAL_DIR)/src/ds90ub947.c \
	$(LOCAL_DIR)/src/ds90ub927.c \
	$(LOCAL_DIR)/src/ds90ub9xx_i2c_rw.c