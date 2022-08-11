LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_INCLUDES += \
	$(LOCAL_DIR)/include \

MODULE_SRCS += \
	$(LOCAL_DIR)/device.c \
	$(LOCAL_DIR)/dma.c \
	$(LOCAL_DIR)/init.c \
	$(LOCAL_DIR)/io.c \
	$(LOCAL_DIR)/irq.c \
	$(LOCAL_DIR)/log.c \
	$(LOCAL_DIR)/shmem.c \
	$(LOCAL_DIR)/softirq.c \
	$(LOCAL_DIR)/version.c

include $(LOCAL_DIR)/system/lk/rules.mk

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -w -DMETAL_INTERNAL

include make/module.mk
