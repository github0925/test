LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \
$(info global define $(GLOBAL_INCLUDES))

ifeq ($(SUPPORT_PMU_SDDRV),true)
GLOBAL_DEFINES += ENABLE_SD_PMU=1
MODULE_SRCS += \
	$(LOCAL_DIR)/src/pmu_hal.c
endif

ifeq ($(SUPPORT_PMU_TEST),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/pmu_hal_ip_test.c
endif

MODULE_DEPS += chipdev/pmu/sd_pmu

include make/module.mk
