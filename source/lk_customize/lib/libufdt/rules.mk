LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	lib/fdt

MODULE_SRCS += \
	$(LOCAL_DIR)/ufdt_overlay.c \
	$(LOCAL_DIR)/ufdt_convert.c \
	$(LOCAL_DIR)/ufdt_node.c \
	$(LOCAL_DIR)/ufdt_node_pool.c \
	$(LOCAL_DIR)/ufdt_prop_dict.c \
	$(LOCAL_DIR)/sysdeps/libufdt_sysdeps_vendor.c

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include \
	$(LOCAL_DIR)/sysdeps/include

MODULE_COMPILEFLAGS += -Wno-sign-compare 

include make/module.mk
