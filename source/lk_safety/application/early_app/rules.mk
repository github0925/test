LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	$(LOCAL_DIR)/main \
	lib/container \
	lib/res_loader \
	lib/elf

ifeq ($(ENABLE_CLUSTER),true)
MODULE_DEPS += $(LOCAL_DIR)/cluster
endif

ifeq ($(ENABLE_CONTROLPANEL),true)
MODULE_DEPS += $(LOCAL_DIR)/controlpanel
endif

ifeq ($(ENABLE_BOOT_ANIMATION),true)
MODULE_DEPS += $(LOCAL_DIR)/BootAnimation
endif

ifeq ($(ENABLE_FASTAVM),true)
MODULE_DEPS += $(LOCAL_DIR)/fastavm
MODULE_DEPS += $(LOCAL_DIR)/sms_monitor
MODULE_DEPS += $(LOCAL_DIR)/storage_ospi
endif

ifeq ($(ENABLE_SLIMAI),true)
ifneq ($(SUPPORT_SLIMAI_SAMPLE_CODE),true)
MODULE_DEPS += $(LOCAL_DIR)/slimAI
endif
endif

ifeq ($(ENABLE_TLVGL),true)
MODULE_DEPS += $(LOCAL_DIR)/tlvgl
endif

ifeq ($(ENABLE_QT_APP),true)
MODULE_DEPS += $(LOCAL_DIR)/qt_app
endif

include make/module.mk
