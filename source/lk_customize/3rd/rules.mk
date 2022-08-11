LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ifeq ($(SUPPORT_3RD_LIBMETAL),true)
MODULE_DEPS += 3rd/libmetal
endif

ifeq ($(SUPPORT_3RD_RPMSG_LITE),true)
MODULE_DEPS += 3rd/rpmsg-lite
endif

ifeq ($(SUPPORT_RPC),true)
MODULE_DEPS += 3rd/erpc/erpc_c
GLOBAL_DEFINES += SUPPORT_3RD_ERPC=1
endif

ifeq ($(SUPPORT_FATFS),true)
MODULE_DEPS += 3rd/fatfs/src
endif
