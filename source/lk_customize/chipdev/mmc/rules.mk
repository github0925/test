LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += lib/bio

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/host/sdhci.c \
	$(LOCAL_DIR)/core/mmc_sdhci.c


include make/module.mk
