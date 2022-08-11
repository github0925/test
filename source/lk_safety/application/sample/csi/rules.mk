LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/app_csi.c \

MODULES += exdev/camera
MODULES += exdev/gpio

GLOBAL_DEFINES += CSI_BOARD_507=1
GLOBAL_DEFINES += CSI_BOARD_508=0

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
