LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include/ $(GLOBAL_INCLUDES)

ifeq ($(SUPPORT_GPIOIRQ_SDDRV),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/gpioirq.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/gpioirq_weak.c
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
