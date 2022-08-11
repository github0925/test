LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/arbitrator.c

MODULE_DEPS += exdev/display
endif

MODULE_COMPILEFLAGS += -DCONFIG_RPMSG_SERVICE=1 -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format

include make/module.mk
