LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_DEPS += framework/lib/system
MODULE_DEPS += framework/service/base
MODULE_DEPS += framework/service/sys_diagnosis
MODULE_DEPS += framework/service/worker
MODULE_DEPS += framework/audio/core

ifeq ($(SUPPORT_DCF),true)
MODULE_DEPS += framework/rpbuf
MODULE_DEPS += framework/communication
MODULE_DEPS += framework/protocol
endif

ifeq ($(SUPPORT_3RD_RPMSG_LITE),true)
MODULE_DEPS += framework/service/rpmsg
endif

ifeq ($(SUPPORT_RPMSG_SAMPLE_CODE),true)
MODULE_DEPS += framework/test/dcf \
               framework/service/samples
endif

ifeq ($(SUPPORT_BACKLIGHT_ROTS_SVC),true)
MODULE_DEPS += framework/service/backlight/backlight_rtos
endif

ifeq ($(SUPPORT_INPUT_SVC),true)
MODULE_DEPS += framework/service/input
endif

ifeq ($(SUPPORT_CLUSTER_IVI),true)
MODULE_DEPS += framework/service/cluster_bridge
endif

ifeq ($(SUPPORT_CAN_PROXY),true)
MODULE_DEPS += framework/service/can_proxy
endif

ifeq ($(SUPPORT_DISPLAY_SVC),true)
MODULE_DEPS += framework/service/display
endif

ifeq ($(SUPPORT_CSI_SVC),true)
MODULE_DEPS += framework/service/camera
endif

ifeq ($(SUPPORT_VIRTUAL_I2C),true)
MODULE_DEPS += framework/service/i2c
endif


ifeq ($(SUPPORT_UPDATE_MONITOR), true)
MODULE_DEPS += framework/service/update_monitor
endif


ifeq ($(SUPPORT_AUDIO_MANAGER), true)
MODULE_DEPS +=  framework/audio/am
MODULE_DEPS += framework/service/audio
endif

ifeq ($(SUPPORT_VIRT_CONSOLE),true)
MODULE_DEPS += framework/service/sdshell
endif
