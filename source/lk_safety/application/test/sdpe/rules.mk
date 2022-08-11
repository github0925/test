LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/sdpe_test.c

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable \
		       -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
