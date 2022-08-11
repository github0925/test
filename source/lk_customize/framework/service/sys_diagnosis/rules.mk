LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)


MODULE_SRCS += \
	$(LOCAL_DIR)/sys_diagnosis.c

MODULE_DEPS += \
	chipdev/func_safety

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format

include make/module.mk
