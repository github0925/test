LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)


MODULE_DEPS += \
$(LOCAL_DIR)/lk_wrapper

FreeRTOS_SRC += \
	$(FreeRTOS_ROOT)/Source/portable/GCC/ARM_CR5/portASM.S \
	$(FreeRTOS_ROOT)/Source/portable/GCC/ARM_CR5/cr5_port.c \
	$(FreeRTOS_ROOT)/Source/portable/MemMang/heap_4.c \
	$(FreeRTOS_ROOT)/Source/croutine.c \
	$(FreeRTOS_ROOT)/Source/event_groups.c \
	$(FreeRTOS_ROOT)/Source/list.c \
	$(FreeRTOS_ROOT)/Source/queue.c \
	$(FreeRTOS_ROOT)/Source/stream_buffer.c \
	$(FreeRTOS_ROOT)/Source/tasks.c \
	$(FreeRTOS_ROOT)/Source/timers.c \

MODULE_SRCS += \
	$(FreeRTOS_SRC) \

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-pointer-to-int-cast

include make/module.mk
