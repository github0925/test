include $(MKDIR)/build.mk

# specify SUBDIRS here
SUBDIRS += class
MODULE_CFLAGS += -I$(TOPDIR)/bt_dev/usb/inc
MODULE_CFLAGS += -I$(TOPDIR)/ip/dw_usb/include
MODULE_CFLAGS += -I$(TOPDIR)/bt_dev/usb/class/ezusb/

MODULE_FOLDER := $(shell basename $(CURDIR))
ifeq ($(TEST),all)
SUBDIRS += test
else
ifneq (,$(findstring $(MODULE_FOLDER),$(TEST)))
SUBDIRS += test
endif
endif

include $(MKDIR)/objs_subdir.mk
include $(MKDIR)/rules.mk
