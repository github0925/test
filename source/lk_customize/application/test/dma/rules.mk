LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/cmd_dma.c \

MODULE_DEPS += \
	lib/cbuf

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

MODULES += \
	lib/unittest \
	lib/debugcommands \

include make/module.mk
