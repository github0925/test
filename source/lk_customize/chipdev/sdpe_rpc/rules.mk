LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/service/sdpe_rpc_service.c \
               $(LOCAL_DIR)/mbuf/sdpe_rpc_mbuf.c

ifeq ($(SUPPORT_SDPE_RPC_SERVER), true)
GLOBAL_DEFINES += SUPPORT_SDPE_RPC_SERVER=1
MODULE_SRCS += $(LOCAL_DIR)/service/sdpe_ctrl/sdpe_rpc_ctrl_server.c \
               $(LOCAL_DIR)/service/vcan/sdpe_rpc_vcan_server.c \
               $(LOCAL_DIR)/service/vlin/sdpe_rpc_vlin_server.c \
               $(LOCAL_DIR)/service/eth/sdpe_rpc_eth_server.c \
               $(LOCAL_DIR)/service/doip/sdpe_rpc_doip_server.c
else
MODULE_SRCS += $(LOCAL_DIR)/service/sdpe_ctrl/sdpe_rpc_ctrl_client.c \
               $(LOCAL_DIR)/service/vcan/sdpe_rpc_vcan_client.c \
               $(LOCAL_DIR)/service/vlin/sdpe_rpc_vlin_client.c
endif

MODULE_SRCS += $(LOCAL_DIR)/infra/sdpe_rpc_framework.c \
               $(LOCAL_DIR)/infra/sdpe_rpc_cbuf.c \
               $(LOCAL_DIR)/infra/sdpe_rpc_l2.c

ifeq ($(SUPPORT_3RD_RPMSG_LITE),true)
GLOBAL_DEFINES += SUPPORT_SDPE_RPC_RPMSG=1
MODULE_SRCS += $(LOCAL_DIR)/transport/sdpe_rpc_rpmsg_transport.c
endif
MODULE_SRCS += $(LOCAL_DIR)/transport/sdpe_rpc_mbox_transport.c

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

include make/module.mk
