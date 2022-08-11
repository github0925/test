LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := $(GLOBAL_INCLUDES)
MODULE_SRCS += \
	$(LOCAL_DIR)/cmd_can.c

ifeq ($(SUPPORT_VIRTCAN_CLIENT), true)
MODULE_DEFINES := SDPE
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable \
		       -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
