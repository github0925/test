LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/fs.c \
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/shell.c

EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/fs.ld

MODULE_COMPILEFLAGS += -Wno-pointer-bool-conversion -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unknown-attributes -Wno-nonnull-compare -Wno-unknown-warning-option

include make/module.mk
