LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ifneq ($(SUPPORT_SDPE_RPC), true)
# Generate LIN driver data structures
ERPCGEN := $(LOCAL_DIR)/../../3rd/erpc/erpcgen/erpcgen
SDPE_CTRL_ERPC_CFG := $(LOCAL_DIR)/sdpe_ctrl.erpc
SDPE_CTRL_GEN_DIR := $(LOCAL_DIR)/gen

ifeq ($(filter clean, $(MAKECMDGOALS)), )
# Generate erpc files before compiling to avoid error when they are dpeneded by other modules compiled before sdpe_ctrl.
$(info ###### Generate SDPE erpc files ######)
dummy := $(shell mkdir -p $(SDPE_CTRL_GEN_DIR))
dummy := $(shell $(ERPCGEN) --output $(SDPE_CTRL_GEN_DIR) $(SDPE_CTRL_ERPC_CFG))
endif

ifeq ($(SUPPORT_SDPE_CTRL_SERVER), true)
MODULE_SRCS += $(SDPE_CTRL_GEN_DIR)/sdpe_ctrl_server.cpp \
			   $(SDPE_CTRL_GEN_DIR)/sdpe_cb_client.cpp
else
MODULE_SRCS += $(SDPE_CTRL_GEN_DIR)/sdpe_ctrl_client.cpp \
               $(SDPE_CTRL_GEN_DIR)/sdpe_cb_server.cpp
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/gen
endif

ifeq ($(SUPPORT_SDPE_CTRL_SERVER), true)
MODULE_SRCS += $(LOCAL_DIR)/sdpe_ctrl_server.c
else
MODULE_SRCS += $(LOCAL_DIR)/sdpe_ctrl_client.c
endif

include make/module.mk
