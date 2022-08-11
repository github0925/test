LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include
MODULE := $(LOCAL_DIR)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/tca9539.c \

ifeq ($(CSI_BOARD_VER), CSI_BOARD_507_A02)
MODULE_SRCS += \
        $(LOCAL_DIR)/src/tca6408.c
endif

ifeq ($(CSI_BOARD_VER), CSI_BOARD_507_A02P)
MODULE_SRCS += \
        $(LOCAL_DIR)/src/tca6408.c
endif

ifeq ($(CSI_BOARD_VER), CSI_BOARD_ICL02)
MODULE_SRCS += \
        $(LOCAL_DIR)/src/tca6408.c
endif

ifeq ($(CSI_BOARD_VER), CSI_BOARD_510)
MODULE_SRCS += \
        $(LOCAL_DIR)/src/tca6408.c
endif

include make/module.mk
