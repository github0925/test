LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/config \
			$(LOCAL_DIR)/infra \
			$(LOCAL_DIR)/port \
			$(LOCAL_DIR)/setup \
			$(LOCAL_DIR)/transports

ifeq ($(SUPPORT_3RD_RPMSG_LITE),true)
MODULE_DEPS += 3rd/rpmsg-lite
endif

MODULE_SRCS += 	$(LOCAL_DIR)/infra/erpc_arbitrated_client_manager.cpp \
			$(LOCAL_DIR)/infra/erpc_basic_codec.cpp \
			$(LOCAL_DIR)/infra/erpc_client_manager.cpp \
			$(LOCAL_DIR)/infra/erpc_crc16.cpp \
			$(LOCAL_DIR)/infra/erpc_message_buffer.cpp \
			$(LOCAL_DIR)/infra/erpc_message_loggers.cpp \
			$(LOCAL_DIR)/infra/erpc_server.cpp \
			$(LOCAL_DIR)/infra/erpc_simple_server.cpp \
			$(LOCAL_DIR)/infra/erpc_transport_arbitrator.cpp \
			$(LOCAL_DIR)/setup/erpc_arbitrated_client_setup.cpp \
			$(LOCAL_DIR)/setup/erpc_client_setup.cpp \
			$(LOCAL_DIR)/setup/erpc_server_setup.cpp \
			$(LOCAL_DIR)/setup/erpc_setup_rpmsg_lite_remote.cpp \
			$(LOCAL_DIR)/setup/erpc_setup_rpmsg_lite_master.cpp \
			$(LOCAL_DIR)/setup/erpc_setup_mbf_rpmsg.cpp \
			$(LOCAL_DIR)/transports/erpc_rpmsg_lite_transport.cpp

ifeq ($(PORTED_KERNEL),FreeRTOS)
MODULE_SRCS += 	$(LOCAL_DIR)/port/erpc_port_freertos.cpp
MODULE_SRCS += 	$(LOCAL_DIR)/port/erpc_threading_freertos.cpp
else
MODULE_SRCS += 	$(LOCAL_DIR)/port/erpc_port_lk.cpp
MODULE_SRCS += 	$(LOCAL_DIR)/port/erpc_threading_lk.cpp
endif

MODULE_COMPILEFLAGS += -lstdc++ -lc

include make/module.mk
