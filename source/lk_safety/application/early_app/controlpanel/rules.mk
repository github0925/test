LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc \
	$(LOCAL_DIR)/../../../include/dev/

MODULE_SRCS += \
	$(LOCAL_DIR)/src/controlpanel.c \
	$(LOCAL_DIR)/src/lv_controlpanel.c

GLOBAL_DEFINES += ENABLE_CONTROLPANEL
GLOBAL_DEFINES += LV_EX_CONF_INCLUDE_SIMPLE=1 LV_LVGL_H_INCLUDE_SIMPLE=1

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable

include make/module.mk
