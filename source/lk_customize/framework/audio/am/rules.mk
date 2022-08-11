LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)
MODULE_STATIC_LIB := false
include $(LOCAL_DIR)/lib/rules.mk
GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

MODULE_SRCS += 	$(LOCAL_DIR)/src/am_api.c \
				$(LOCAL_DIR)/src/am.c

MODULE_CFLAGS += -Wno-strict-prototypes -Wno-unused-variable

MODULE_DEPS +=  exdev/am_codec \
                exdev/am_board \
                exdev/gpio \

include make/module.mk
