include $(MKDIR)/build.mk

# Specify SUBDIRS here.

CFG_CFLAGS += -DDYNAMIC_SD_CRC_TABLE=1
CINCLUDE += \
        -I$(TOPDIR)/lib/libavb/ \
        -I$(TOPDIR)/lib/sd_x509/ \
        -I$(TOPDIR)/lib/verified_boot/include

include $(MKDIR)/objs_subdir.mk
include $(MKDIR)/rules.mk
