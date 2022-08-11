LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/ak7738/inc/ \
	$(LOCAL_DIR)/tca9539/inc/ \
	$(LOCAL_DIR)/tas6424/inc/ \
	$(LOCAL_DIR)/../../../../hal/audio_hal/common \
	$(LOCAL_DIR)/xf6020/inc/ \
	$(LOCAL_DIR)/tas5404/inc/ \

ifeq ($(SUPPORT_AUDIO_MANAGER), true)
MODULE_SRCS += $(LOCAL_DIR)/ak7738/src/ak7738.c
MODULE_SRCS += $(LOCAL_DIR)/tca9539/src/am_tca9539.c
MODULE_SRCS += $(LOCAL_DIR)/tas6424/src/tas6424.c
MODULE_SRCS += $(LOCAL_DIR)/xf6020/src/xf6020.c
MODULE_SRCS += $(LOCAL_DIR)/tas5404/src/am_tas5404.c
endif
include make/module.mk
