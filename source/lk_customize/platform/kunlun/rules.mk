LOCAL_DIR := $(GET_LOCAL_DIR)
WITH_CPP_SUPPORT=true

GLOBAL_INCLUDES := \
	$(LOCAL_DIR) $(LOCAL_DIR)/../../chipcfg/generate/$(CHIPVERSION)/ $(GLOBAL_INCLUDES)\

#include $(LOCAL_DIR)/common/rules.mk
include $(LOCAL_DIR)/$(SUB_PLATFORM)/rules.mk
