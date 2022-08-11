LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/dcf_sample.c

MODULE_DEPS += \
	framework/communication \
	framework/service/property

ifeq ($(SUPPORT_3RD_RPMSG_LITE),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/test_rpmsg_linux.c

MODULE_DEPS += framework/service/rpmsg
endif

ifeq ($(SUPPORT_POSIX),true)
MODULE_SRCS += \
        $(LOCAL_DIR)/test_posix.c
endif

ifeq ($(TARGET),fpga_ecockpit)
MODULE_COMPILEFLAGS += -DCONFIG_FPGA_ECOCKPIT=1
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format

include make/module.mk
