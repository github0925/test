LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_INCLUDES += \
	$(LOCAL_DIR)/include \

MODULE_SRCS += \
	$(LOCAL_DIR)/ipcc_hal_mb.c

SUPPORT_IPCC_RPMSG ?= true

ifeq ($(SUPPORT_IPCC_RPMSG), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/ipcc_rpmsg.c \
	$(LOCAL_DIR)/ipcc_rpmsg_ns.c \
	$(LOCAL_DIR)/ipcc_service.c \
	$(LOCAL_DIR)/ipcc_queue.c \
	$(LOCAL_DIR)/ipcc_device.c \
	$(LOCAL_DIR)/ipcc_rpc.c

GLOBAL_DEFINES += CONFIG_IPCC_RPMSG=1
endif

MODULE_COMPILEFLAGS += -Wall

include make/module.mk
