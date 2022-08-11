LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/inc

ifeq ($(SUPPORT_RPC), true)
# Generate LIN driver data structures
ERPCGEN := $(LOCAL_DIR)/../../3rd/erpc/erpcgen/erpcgen
LIN_DRV_ERPC_CFG := $(LOCAL_DIR)/lin_drv.erpc
LIN_DRV_GEN_DIR := $(LOCAL_DIR)/gen

ifeq ($(filter clean, $(MAKECMDGOALS)), )
# Generate erpc files before compiling to avoid error when they are dpeneded by other modules compiled before lin_hal.
$(info ###### Generate LIN erpc files ######)
dummy := $(shell mkdir -p $(LIN_DRV_GEN_DIR))
dummy := $(shell $(ERPCGEN) --output $(LIN_DRV_GEN_DIR) $(LIN_DRV_ERPC_CFG))
endif

GLOBAL_INCLUDES += $(LIN_DRV_GEN_DIR)
endif

ifeq ($(SUPPORT_LIN_SDDRV), true)
MODULE_SRCS += $(LOCAL_DIR)/Lin.c
endif

ifeq ($(SUPPORT_VIRTLIN_CLIENT), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/Lin_PBCfg.c \
	$(LOCAL_DIR)/vlin_client.c

ifeq ($(SUPPORT_RPC), true)
MODULE_SRCS += $(LIN_DRV_GEN_DIR)/lin_drv_client.cpp
endif
else ifeq ($(SUPPORT_VIRTLIN_SERVER), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/vlin_server.c

ifeq ($(SUPPORT_RPC), true)
MODULE_SRCS += $(LIN_DRV_GEN_DIR)/lin_drv_server.cpp
endif
else
MODULE_SRCS += $(LOCAL_DIR)/Lin_PBCfg.c
endif

include make/module.mk
