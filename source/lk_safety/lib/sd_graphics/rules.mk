GRAPHICS_DIR := $(GET_LOCAL_DIR)

########

GLOBAL_MODULE_LDFLAGS += -L$(GRAPHICS_DIR) -lsd_graphics
GLOBAL_INCLUDES += $(GRAPHICS_DIR)/inc

########
# MODULE := $(LOCAL_DIR)
# MODULE_STATIC_LIB := true

# GLOBAL_INCLUDES += $(LOCAL_DIR)/inc

# MODULE_SRCS += \
# 	$(LOCAL_DIR)/sd_graphics.c

# MODULE_CFLAGS += -Wno-strict-prototypes -Wno-unused-variable

# include make/module.mk
