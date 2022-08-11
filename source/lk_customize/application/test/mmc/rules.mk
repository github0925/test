LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/cmd_mmc.c

GLOBAL_INCLUDES += \
        $(LOCAL_DIR)/ \

MODULE_DEPS += \
	lib/cbuf \
	lib/sdunittest \
#	chipdev/mmc

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
