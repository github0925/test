LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc \
	$(LOCAL_DIR)/../../../include/dev/

MODULE_SRCS += \
	$(LOCAL_DIR)/src/lv_demo_cluster.c

include $(LOCAL_DIR)/../../../lib/sd_graphics/rules.mk

MODULE_DEPS += \
	framework/service/rpmsg \
	lib/sdcast

CLUSTER_BS_SIZE ?= 0x800000

GLOBAL_DEFINES += \
	CLUSTER_BS_SIZE=$(CLUSTER_BS_SIZE)

GLOBAL_DEFINES += ENABLE_CLUSTER
GLOBAL_DEFINES += ENABLE_LVGL_CLUSTER=1
GLOBAL_DEFINES += LV_EX_CONF_INCLUDE_SIMPLE=1 LV_LVGL_H_INCLUDE_SIMPLE=1
ifeq ($(FUNC_SAFE_CLUSTER),true)
	GLOBAL_DEFINES += FUNC_SAFE_CLUSTER=1
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable

include make/module.mk
