LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/src/serdes_9xx.c \

ifeq ($(SUPPORT_TOUCH_GOODIX), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/goodix.c
endif

ifeq ($(SUPPORT_TOUCH_SIW), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/siw_touch.c
endif

ifeq ($(SUPPORT_TOUCH_EGALAX), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/egalax_i2c.c
endif
