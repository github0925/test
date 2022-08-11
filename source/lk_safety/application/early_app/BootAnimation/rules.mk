LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc

ifeq ($(TARGET),reference_x9u)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/ba_uart.c
endif

MODULE_SRCS += \
	$(LOCAL_DIR)/src/ba_config.c \
	$(LOCAL_DIR)/src/player_g2d.c \
	$(LOCAL_DIR)/src/decoder_task.c \
	$(LOCAL_DIR)/src/vpu2disp.c \
	$(LOCAL_DIR)/src/animation.c \
	$(LOCAL_DIR)/src/player_task.c \
	$(LOCAL_DIR)/src/audio_task.c \
	$(LOCAL_DIR)/src/ba_replay.c \


GLOBAL_DEFINES += ENABLE_BOOTANIMATION USE_ARGB888 ENABLE_AUDIO_AGENT=1

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

MODULE_DEPS += exdev/audio_codec/tas6424 \
               exdev/audio_codec/ak7738 \
               exdev/gpio \

include make/module.mk
