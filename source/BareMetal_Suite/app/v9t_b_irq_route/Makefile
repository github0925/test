include $(MKDIR)/build.mk

# specify SUBDIRS here
SUBDIRS +=

MODULE_CFLAGS := -DGICD_BASE=\(GIC5_BASE+0x1000U\) \
				 -DGICC_BASE=\(GIC5_BASE+0x2000U\)

include $(MKDIR)/objs_subdir.mk
include $(MKDIR)/rules.mk
