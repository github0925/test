include $(MKDIR)/build.mk

SUBDIRS += $(SOC) 

MODULE_FOLDER := $(shell basename $(CURDIR))
ifeq ($(TEST),all)
SUBDIRS += 
else
ifneq (,$(findstring $(MODULE_FOLDER),$(TEST)))
SUBDIRS += test
endif
endif

include $(MKDIR)/objs_subdir.mk
include $(MKDIR)/rules.mk
