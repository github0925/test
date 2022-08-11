LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/safety_init.c \
	$(LOCAL_DIR)/soc_init.c \
	$(LOCAL_DIR)/hpi_qos_init.c \
	$(LOCAL_DIR)/scr_init.c

ifeq ($(SUPPORT_NEXT_OS), true)
MODULE_DEPS += lib/reboot
endif

MODULE_DEPS += hal/pmu_hal/sd_pmu_hal

include make/module.mk
