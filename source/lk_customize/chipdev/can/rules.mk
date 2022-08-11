LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/flexcan/include

ifeq ($(SUPPORT_RPC), true)
# Generate CAN driver data structures
ERPCGEN := $(LOCAL_DIR)/../../3rd/erpc/erpcgen/erpcgen
VCAN_CFG := $(LOCAL_DIR)/vcan/script/vcan.erpc
VCAN_GEN_DIR := $(LOCAL_DIR)/vcan/gen/vcan
VCANCBK_CFG := $(LOCAL_DIR)/vcan/script/vcan_cb.erpc
VCANCBK_GEN_DIR := $(LOCAL_DIR)/vcan/gen/vcan_cb
FLEXCAN_CFG := $(LOCAL_DIR)/vcan/script/flexcan_autogen.erpc
FLEXCAN_GEN_DIR := $(LOCAL_DIR)/vcan/gen

ifeq ($(filter clean, $(MAKECMDGOALS)), )
# Generate erpc files before compiling to avoid error when they are dpeneded by other modules compiled before can.
$(info ###### Generate CAN erpc files ######)
dummy := $(shell mkdir -p $(VCAN_GEN_DIR) $(VCANCBK_GEN_DIR))
dummy := $(shell $(ERPCGEN) --output $(VCAN_GEN_DIR) $(VCAN_CFG))
dummy := $(shell $(ERPCGEN) --output $(VCANCBK_GEN_DIR) $(VCANCBK_CFG))
dummy := $(shell $(ERPCGEN) --output $(FLEXCAN_GEN_DIR) $(FLEXCAN_CFG))
dummy := $(shell rm -f $(FLEXCAN_GEN_DIR)/flexcan_autogen_client.* $(FLEXCAN_GEN_DIR)/flexcan_autogen_server.*)
endif

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/vcan/gen/vcan \
	$(LOCAL_DIR)/vcan/gen/vcan_cb
endif # SUPPORT_RPC = true

ifeq ($(SUPPORT_CAN_SDDRV), true)
#Without SDPE
MODULE_SRCS += \
	$(LOCAL_DIR)/flexcan/Can.c \
	$(LOCAL_DIR)/flexcan/flexcan.c \
	$(LOCAL_DIR)/flexcan/can_cfg.c

GLOBAL_DEFINES += \
	__critical_code__= \
	__critical_data__=

MODULE_DEFINES := CORE_LITTLE_ENDIAN=1

else ifeq ($(SUPPORT_VIRTCAN_SERVER), true)
#Compile sdpe image
GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/vcan/gen \
	$(LOCAL_DIR)/vcan/sdpe

MODULE_SRCS += \
	$(LOCAL_DIR)/flexcan/Can.c \
	$(LOCAL_DIR)/flexcan/flexcan.c \
	$(LOCAL_DIR)/vcan/sdpe/vcan_server.c

ifeq ($(SUPPORT_RPC), true)
MODULE_SRCS += \
	$(VCAN_GEN_DIR)/vcan_server.cpp \
	$(VCANCBK_GEN_DIR)/vcan_cb_client.cpp
endif

MODULE_DEFINES := CORE_LITTLE_ENDIAN=1

else ifeq ($(SUPPORT_VIRTCAN_CLIENT), true)
#Compile safety image
GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/vcan/sdpe

MODULE_SRCS += \
	$(LOCAL_DIR)/flexcan/can_cfg.c \
	$(LOCAL_DIR)/vcan/autosar/vcan_client.c

ifeq ($(SUPPORT_RPC), true)
MODULE_SRCS += \
	$(VCAN_GEN_DIR)/vcan_client.cpp \
	$(VCANCBK_GEN_DIR)/vcan_cb_server.cpp
endif

GLOBAL_DEFINES += \
	__critical_code__= \
	__critical_data__=

GLOBAL_DEFINES += SDPE
endif

ifneq ($(filter g9%, $(CHIPVERSION)), )
GLOBAL_DEFINES += MAX_FLEXCAN_CH=20
else ifeq ($(CHIPVERSION), v9t)
GLOBAL_DEFINES += MAX_FLEXCAN_CH=8
else
GLOBAL_DEFINES += MAX_FLEXCAN_CH=4
endif

include make/module.mk
