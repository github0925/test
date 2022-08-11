LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/../../early_app/slimAI/inc \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-host \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-host/standalone \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-common \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-kernel \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-host/thread-freertos \
        $(LOCAL_DIR)/../../../lib/heap/ \


MODULE_SRCS += \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-host/xrp_host_common.c \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-host/xrp_sync_queue.c \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-host/standalone/xrp_host.c \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-host/standalone/xrp_freertos.c \
	$(LOCAL_DIR)/../../early_app/slimAI/src/xrp-kernel/xrp_alloc.c \
	$(LOCAL_DIR)/slimAI.c \

ifneq ($(ENABLE_FASTAVM),true)
MODULE_SRCS += $(LOCAL_DIR)/../../early_app/fastavm/src/avm_vdsp_utility.c
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
