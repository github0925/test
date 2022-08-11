LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_INCLUDES += \
	$(LOCAL_DIR)/include \

MODULE_SRCS += \
	$(LOCAL_DIR)/llist.c \
	$(LOCAL_DIR)/rpmsg_lite.c \
	$(LOCAL_DIR)/rpmsg_ns.c \
	$(LOCAL_DIR)/rpmsg_queue.c \
	$(LOCAL_DIR)/virtqueue.c \
	$(LOCAL_DIR)/porting/rpmsg_env_lk.c \
	$(LOCAL_DIR)/porting/rpmsg_platform.c


MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare

include make/module.mk
