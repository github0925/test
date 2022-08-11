LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

#GLOBAL_DEFINES += VRING_SIZE=0x0400
#GLOBAL_DEFINES += VRING_ALIGN=0x40
#GLOBAL_DEFINES += RL_BUFFER_PAYLOAD_SIZE=496

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/  \
	$(LOCAL_DIR)/client/  \
	$(LOCAL_DIR)/server/  \

MODULE_SRCS += \
	$(LOCAL_DIR)/virt_com.cpp  \
	$(LOCAL_DIR)/vcan_if.c  \
	$(LOCAL_DIR)/client/virCom_client.cpp  \
	$(LOCAL_DIR)/server/virComCbk_server.cpp  \

MODULE_DEPS += application/services/rpmsg

include make/module.mk

