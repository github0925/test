LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(SUPPORT_SPI_MASTER_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/app_spi.c \

endif

ifeq ($(SUPPORT_SPI_SLAVE_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/app_spi_slave.c \

endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
