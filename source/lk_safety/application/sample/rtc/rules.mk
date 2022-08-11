LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/rtc_demo.c

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk