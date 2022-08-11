LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc \
	$(LOCAL_DIR)/src/xrp-host \
	$(LOCAL_DIR)/src/xrp-host/standalone \
	$(LOCAL_DIR)/src/xrp-common \
	$(LOCAL_DIR)/src/xrp-kernel \
	$(LOCAL_DIR)/src/xrp-host/thread-freertos \
        $(LOCAL_DIR)/../../../exdev/display/include \
        $(LOCAL_DIR)/../../../exdev/lcm \
        $(LOCAL_DIR)/../../../lib/heap/ \
        $(LOCAL_DIR)/../../../hal/disp_hal/sd_disp_hal/lib/inc \
        $(LOCAL_DIR)/../../../chipdev/disp/sd_disp/inc \


MODULE_SRCS += \
	$(LOCAL_DIR)/src/xrp-host/xrp_host_common.c \
	$(LOCAL_DIR)/src/xrp-host/xrp_sync_queue.c \
	$(LOCAL_DIR)/src/xrp-host/standalone/xrp_host.c \
	$(LOCAL_DIR)/src/xrp-host/standalone/xrp_freertos.c \
	$(LOCAL_DIR)/src/xrp-kernel/xrp_alloc.c \

ifneq ($(ENABLE_FASTAVM),true)
MODULE_SRCS += $(LOCAL_DIR)/../fastavm/src/avm_vdsp_utility.c
endif

MODULES += exdev/camera
MODULES += exdev/gpio


GLOBAL_DEFINES += ENABLE_CV


MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
