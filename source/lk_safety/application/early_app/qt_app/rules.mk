LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc \
	$(LOCAL_DIR)/lib/inc

MODULE_SRCS := $(shell find -L $(LOCAL_DIR)/src -name \*.cpp)

QT_DIR := $(GET_LOCAL_DIR)

GLOBAL_MODULE_LDFLAGS += -L$(QT_DIR)/lib \
						-lQulQuickUltralite_X9-FREERTOS_Linux_armgcc_Release \
						-lQulQuickUltralitePNGDecoderNull_X9-FREERTOS_Linux_armgcc_Release \
						-lQulQuickUltralitePlatform_X9-FREERTOS_32bpp_Linux_armgcc_Release \
						-lQulQuickUltraliteTimeline_X9-FREERTOS_Linux_armgcc_Release \
						-lQulQuickUltraliteCharts_X9-FREERTOS_Linux_armgcc_Release \
						-lQulQuickUltraliteControlsStyleDefault_X9-FREERTOS_Linux_armgcc_Release \
						-lQulQuickUltraliteTemplates_X9-FREERTOS_Linux_armgcc_Release \
						-lQulQuickUltralite_X9-FREERTOS_Linux_armgcc_Release \
						-lm -lstdc++

GLOBAL_DEFINES += ENABLE_QT_APP

GLOBAL_DEFINES += QUL_STD_STRING_SUPPORT

#GLOBAL_DEFINES += QUL_INTERNAL_SPLIT_IMAGE_OPTIMIZATION=OFF

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable -Wno-maybe-uninitialized

include make/module.mk
