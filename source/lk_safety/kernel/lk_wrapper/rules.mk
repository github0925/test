LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)


LK_WRAPPER_DEBUG ?= 0



GLOBAL_INCLUDES += \
	$(LKROOT)/include/kernel

MODULE_SRCS += \
	$(LOCAL_DIR)/lk_event.c  \
	$(LOCAL_DIR)/lk_mutex.c \
	$(LOCAL_DIR)/lk_semaphore.c \
	$(LOCAL_DIR)/lk_timers.c \
	$(LOCAL_DIR)/lk_thread.c


MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-pointer-to-int-cast

include make/module.mk
