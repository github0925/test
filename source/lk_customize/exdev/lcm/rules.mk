LCM_DIR := $(GET_LOCAL_DIR)

MODULE := $(LCM_DIR)

GLOBAL_INCLUDES += $(LCM_DIR)

MODULE_SRCS += \
	$(LCM_DIR)/disp_panels.c

LCM_LISTS := $(subst ",,$(CONFIG_CUSTOM_LCM))
MODULE_DEPS += $(foreach LCM,$(LCM_LISTS),$(LCM_DIR)/$(LCM)/)

LCM_DEFINES := $(shell echo $(CONFIG_CUSTOM_LCM) | tr a-z A-Z)
GLOBAL_DEFINES += $(foreach LCM,$(LCM_DEFINES),$(LCM))
GLOBAL_DEFINES += $(shell echo $(CONFIG_PANELS) | tr a-z A-Z)

#include $(LCM_DIR)/adv7511/rules.mk
include $(LCM_DIR)/sn65dsi85/rules.mk
include $(LCM_DIR)/ds90ub9xx/rules.mk
include $(LCM_DIR)/lt9611/rules.mk

MODULE_DEPS += lib/system_config

include make/module.mk
