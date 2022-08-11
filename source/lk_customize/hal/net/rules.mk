LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/rpmsg_net_rtos.c

ifeq ($(SUPPORT_NETIF),true)
MODULE_COMPILEFLAGS += -DENABLE_RPMSG_NET=1
MODULE_DEPS := lib/lwip
endif

include make/module.mk
