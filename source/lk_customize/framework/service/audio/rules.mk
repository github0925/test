LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/audio_rpc_svc.c

MODULE_DEPS += framework/communication
##MODULE_DEPS += framework/rpbuf
##MODULE_DEPS += framework/protocol
##MODULE_DEPS += framework/service/property

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
