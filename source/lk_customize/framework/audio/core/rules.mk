LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)


GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

MODULE_SRCS += 	$(LOCAL_DIR)/src/sd_audio.c \

ifeq ($(SUPPORT_AUDIO_AGENT), true)
MODULE_SRCS += 	$(LOCAL_DIR)/src/au_agent.c \

MODULE_DEPS += \
	lib/container \

endif


MODULE_CFLAGS += -Wno-strict-prototypes -Wno-unused-variable

include make/module.mk
