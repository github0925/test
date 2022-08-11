LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/safety_disp.c

MODULE_SRCS += \
	$(LOCAL_DIR)/disp_share/disp_share.c

MODULE_DEPS += framework/communication

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-unused-but-set-variable

include make/module.mk
