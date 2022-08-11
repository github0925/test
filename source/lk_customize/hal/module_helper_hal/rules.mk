LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/inc
MODULE_SRCS += \
	$(LOCAL_DIR)/src/module_helper_hal.c \
	$(LOCAL_DIR)/src/res/res_clk.c	\
	$(LOCAL_DIR)/src/res/res_rstgen.c	\

ifeq ($(MODULE_HELPER_PER_DDR), true)
GLOBAL_DEFINES += MODULE_HELPER_PER_DDR=1
MODULE_SRCS +=	$(LOCAL_DIR)/src/per/per_ddr.c
endif

ifeq ($(MODULE_HELPER_PER_SYS), true)
GLOBAL_DEFINES += MODULE_HELPER_PER_SYS=1
MODULE_SRCS +=	$(LOCAL_DIR)/src/per/per_sys.c
endif

ifeq ($(MODULE_HELPER_PER_TEST), true)
GLOBAL_DEFINES += MODULE_HELPER_PER_TEST=1
MODULE_SRCS +=	$(LOCAL_DIR)/src/per/per_test.c
endif

ifeq ($(MODULE_HELPER_PER_DISP), true)
GLOBAL_DEFINES += MODULE_HELPER_PER_DISP=1
MODULE_SRCS +=  $(LOCAL_DIR)/src/per/per_disp.c
endif

include make/module.mk
