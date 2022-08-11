LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

ifeq ($(NORFLASH_DEVICE_TYPE),mt35x)
MODULE_SRCS += \
	$(LOCAL_DIR)/mt35x.c
endif

ifeq ($(NORFLASH_DEVICE_TYPE),is25x)
MODULE_SRCS += \
	$(LOCAL_DIR)/is25x.c
endif

ifeq ($(NORFLASH_DEVICE_TYPE),gd25x)
MODULE_SRCS += \
	$(LOCAL_DIR)/gd25x.c
endif

ifeq ($(NORFLASH_DEVICE_TYPE),s25x)
MODULE_SRCS += \
	$(LOCAL_DIR)/s25x.c
endif

ifeq ($(NORFLASH_DEVICE_TYPE),w25q)
MODULE_SRCS += \
	$(LOCAL_DIR)/w25q.c
endif

ifeq ($(NORFLASH_DEVICE_TYPE),spi_nor)
MODULE_SRCS += \
	$(LOCAL_DIR)/spi_nor.c
endif

include make/module.mk
