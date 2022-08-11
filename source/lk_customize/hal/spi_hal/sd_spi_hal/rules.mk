LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) \

ifeq ($(SUPPORT_SPI_MASTER_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/spi_hal_master.c \

else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/spi_hal_master_weak.c \

endif

ifeq ($(SUPPORT_SPI_SLAVE_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/spi_hal_slave.c \

else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/spi_hal_slave_weak.c \

endif

ifeq ($(SUPPORT_SPI1_SLAVE_SET), true)
GLOBAL_DEFINES += SPI1_CTRL_SLAVE_MODE=1
endif
ifeq ($(SUPPORT_SPI2_SLAVE_SET), true)
GLOBAL_DEFINES += SPI2_CTRL_SLAVE_MODE=1
endif
ifeq ($(SUPPORT_SPI3_SLAVE_SET), true)
GLOBAL_DEFINES += SPI3_CTRL_SLAVE_MODE=1
endif
ifeq ($(SUPPORT_SPI4_SLAVE_SET), true)
GLOBAL_DEFINES += SPI4_CTRL_SLAVE_MODE=1
endif
ifeq ($(SUPPORT_SPI5_SLAVE_SET), true)
GLOBAL_DEFINES += SPI5_CTRL_SLAVE_MODE=1
endif
ifeq ($(SUPPORT_SPI6_SLAVE_SET), true)
GLOBAL_DEFINES += SPI6_CTRL_SLAVE_MODE=1
endif
ifeq ($(SUPPORT_SPI7_SLAVE_SET), true)
GLOBAL_DEFINES += SPI7_CTRL_SLAVE_MODE=1
endif
ifeq ($(SUPPORT_SPI8_SLAVE_SET), true)
GLOBAL_DEFINES += SPI8_CTRL_SLAVE_MODE=1
endif

ifeq ($(SUPPORT_SPI_MASTER_TEST_SDDRV), true)
ifeq ($(SUPPORT_SPI_MASTER_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/spi_hal_test.c \
	$(LOCAL_DIR)/src/spi_hal_master_test.c \

endif
endif

ifeq ($(SUPPORT_SPI_SLAVE_TEST_SDDRV), true)
ifeq ($(SUPPORT_SPI_SLAVE_SDDRV), true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/spi_hal_test.c \
	$(LOCAL_DIR)/src/spi_hal_slave_test.c \

endif
endif

include make/module.mk
