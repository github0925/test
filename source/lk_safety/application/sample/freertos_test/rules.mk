LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc/ \

MODULE_SRCS += \
	$(LOCAL_DIR)/src/EventTest.c \
	$(LOCAL_DIR)/src/MutexTest.c \
	$(LOCAL_DIR)/src/QueueTest.c \

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
