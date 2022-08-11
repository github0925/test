LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc \
        $(LOCAL_DIR)/../../../exdev/display/include \
        $(LOCAL_DIR)/../../../exdev/lcm \
        $(LOCAL_DIR)/../../../lib/heap/ \
        $(LOCAL_DIR)/../../../hal/disp_hal/sd_disp_hal/lib/inc \
        $(LOCAL_DIR)/../../../chipdev/disp/sd_disp/inc \
	$(LOCAL_DIR)/../../../application/sample/fastAVM \


MODULE_SRCS += \
	$(LOCAL_DIR)/src/avm_fastavm.c \
	$(LOCAL_DIR)/src/avm_fastavm_task.c \
	$(LOCAL_DIR)/src/avm_vdsp2disp.c \
	$(LOCAL_DIR)/src/vstreamer.c \
	$(LOCAL_DIR)/src/avm_app_csi.c \
	$(LOCAL_DIR)/src/avm_player_utility.c \
	$(LOCAL_DIR)/src/avm_vdsp_utility.c \
	$(LOCAL_DIR)/src/avm_lvgl.c \
	# $(LOCAL_DIR)/src/fastavm_api.c
	# $(LOCAL_DIR)/../../../application/sample/fastAVM/sample_avm.c


MODULE_DEPS += exdev/camera
MODULE_DEPS += exdev/gpio



GLOBAL_DEFINES += ENABLE_FASMAVM
GLOBAL_DEFINES += ENABLE_AVM_FIRST=1


MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
