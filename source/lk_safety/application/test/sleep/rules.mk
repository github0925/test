LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/g9_ref_sleep.c

MODULE_DEPS += hal/pmu_hal/sd_pmu_hal

SUPPORT_PMU_SDDRV := true

include make/module.mk
