
CURDIR := $(GET_LOCAL_DIR)


SUPPORT_SD_DISP := true
ifeq ($(SUPPORT_LVGL_GUI),true)
SUPPORT_LODE_PNG := true
SUPPORT_LODE_SJPG := true
endif
MODULE_CFLAGS += -Wno-unused-variable -Wno-discarded-qualifiers

ifeq ($(SUPPORT_SD_DISP),true)
MODULE_SRCS += \
	$(CURDIR)/display/sd_disp_drv.c \
	$(CURDIR)/gpu/lv_gpu_sdrv_dma2d.c \

GLOBAL_DEFINES += \
	SUPPORT_SD_DISP=1
else
MODULE_SRCS += \
	$(CURDIR)/display/gfx_disp_drv.c
endif

# fs
MODULE_SRCS += \
	$(CURDIR)/fs/fatfs_semidrive.c
# input
MODULE_SRCS += \
  $(CURDIR)/input/lv_port_indev.c

MODULE_INCLUDES += $(CURDIR)/display
GLOBAL_INCLUDES += $(CURDIR)/gpu
MODULE_INCLUDES += $(CURDIR)/input
MODULE_INCLUDES += $(CURDIR)/fs


ifeq ($(SUPPORT_LODE_PNG),true)
MODULE_SRCS += 	\
	$(CURDIR)/decoder/lodepng/lodepng.c \
	$(CURDIR)/decoder/lodepng/lv_png.c
GLOBAL_INCLUDES += $(CURDIR)/decoder/lodepng
GLOBAL_DEFINES += SUPPORT_LODE_PNG
endif

ifeq ($(SUPPORT_LODE_SJPG),true)
MODULE_SRCS += 	\
	$(CURDIR)/decoder/sjpg/tjpgd.c \
	$(CURDIR)/decoder/sjpg/lv_sjpg.c
GLOBAL_INCLUDES += $(CURDIR)/decoder/sjpg
GLOBAL_DEFINES += SUPPORT_LODE_SJPG
endif