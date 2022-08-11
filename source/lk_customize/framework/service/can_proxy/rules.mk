LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include \
	$(LOCAL_DIR)/utils \
	$(LOCAL_DIR)/bundle \
	$(LOCAL_DIR)/interface \
	$(LOCAL_DIR)/interface/rpmsg

MODULE_SRCS += \
	$(LOCAL_DIR)/can_proxy.c \
	$(LOCAL_DIR)/bundle/can_proxy_bundle.c \
	$(LOCAL_DIR)/interface/can_proxy_if.c \
	$(LOCAL_DIR)/interface/can/can_port.c \
	$(LOCAL_DIR)/interface/rpmsg/rpmsg_port.c \
	$(LOCAL_DIR)/utils/can_proxy_buffer.c \
	$(LOCAL_DIR)/utils/can_proxy_os_port.c

MODULE_DEPS += \
	framework/communication \
	framework/service/rpmsg \
	chipdev/can

GLOBAL_DEFINES += SUPPORT_CAN_PROXY=1

MODULE_COMPILEFLAGS += -Wall

include make/module.mk
