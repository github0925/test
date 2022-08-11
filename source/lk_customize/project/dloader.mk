##########################
#dloader build
##########################
DEBUG ?=0
WITH_KERNEL_VM ?=0

#begin fk
#make CHIPVERSION=d9 SD_PROJECT=default  dloader PROJ_POSTFIX=_d9_ref
LOCAL_DIR := $(GET_LOCAL_DIR)
$(info "----------1------------- $(LOCAL_DIR)")
TARGET := reference_d9
PROJECT :=$(SD_PROJECT)
include $(LOCAL_DIR)/../chipcfg/generate/$(CHIPVERSION)/projects/$(SD_PROJECT)/dloader_cfg.mk
##end

include $(LOCAL_DIR)/../chipcfg/rules.mk

ifeq ($(NO_DDR), true)
MEMBASE ?= 0x1a0000
MEMSIZE ?= 0x60000

DL_BUF_BASE ?= 0x140000
DL_BUF_SIZE ?= 0x40000

SPARSE_DATA_ALIGNED_BASE ?= 0x180000
SPARSE_DATA_ALIGNED_SIZE ?= 0x20000

GLOBAL_DEFINES += \
	NO_DDR=1
else
MEMBASE ?= $(SEC_MEMBASE)
MEMSIZE ?= $(SEC_MEMSIZE)

DL_BUF_BASE ?= $(shell echo $$(( $(MEMBASE) + $(MEMSIZE) + 0x100000)))
DL_BUF_SIZE ?= 0x2000000  #32M

SPARSE_DATA_ALIGNED_BASE ?= $(shell echo $$(( $(DL_BUF_BASE) + $(DL_BUF_SIZE) + 0x100000)))
SPARSE_DATA_ALIGNED_SIZE ?= 0x800000 #8M
endif

KERNEL_BASE ?= $(MEMBASE)
GLOBAL_DEFINES += \
	SPARSE_DATA_ALIGNED_BASE=$(SPARSE_DATA_ALIGNED_BASE) \
	SPARSE_DATA_ALIGNED_SIZE=$(SPARSE_DATA_ALIGNED_SIZE) \

ARM_WITH_VFP ?= 1
ARM_VFP_D16 ?= 1

GLOBAL_DEFINES += \
	WITH_NO_PHYS_RELOCATION=1 \
	DL_BUF_BASE=$(DL_BUF_BASE) \
	DL_BUF_SIZE=$(DL_BUF_SIZE) \

ifeq ($(WITH_KERNEL_VM), 1)
PERIPHERAL_BASE_VIRT := 0xFFFF000000000000
GLOBAL_DEFINES += \
        PERIPHERAL_BASE_VIRT=$(PERIPHERAL_BASE_VIRT)
endif

# log buf
ENABLE_LOG_BUF ?= 1
GLOBAL_DEFINES += \
       ENABLE_LOG_BUF=$(ENABLE_LOG_BUF) \
       MAX_LOG_BUF_LEN=4096

##########################
#driver modules define
##########################
ENABLE_IP_TEST ?= false
SUPPORT_SCR_SDDRV ?= true
SUPPORT_ARM_GIC_SDDRV ?= true
SUPPORT_WDG_SDDRV ?= true
SYSTEM_TIMER ?= sdrv_timer
SUPPORT_TIMER_SDDRV ?= true
SUPPORT_UART_DWDRV ?= true
SUPPORT_PLL_SDDRV ?= true
SUPPORT_DMA_SDDRV ?= false
SUPPORT_PORT_SDDRV ?= true
SUPPORT_USB_SDDRV ?= true
SUPPORT_SPINOR_SDDRV ?= true
SUPPORT_MMC_SDDRV ?= true
SUPPORT_RSTGEN_SDDRV ?= true
SUPPORT_CLKGEN_SDDRV ?= true
SUPPORT_FUSE_CTRL ?= true
SUPPORT_MODULE_HELPER_SDDRV ?= false
NORFLASH_DEVICE_TYPE ?= spi_nor
TOGGLE_OSPI_RESET_ENABLE ?= false
SUPPORT_SPI_MASTER_SDDRV ?= true
SUPPORT_DIO_SDDRV ?= true

SUPPORT_HTOL_TEST ?= false
SUPPORT_BOARDINFO ?= true
##########################
#application modules define
##########################
ifeq ($(SUPPORT_WDG_SDDRV), true)
SUPPORT_WDG_NEED_INIT := true
SUPPORT_WDG_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_WDG_SAMPLE_CODE := false
endif

ifeq ($(SUPPORT_RSTGEN_SDDRV), true)
SUPPORT_RSTGEN_NEED_INIT := false
SUPPORT_RSTGEN_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_RSTGEN_SAMPLE_CODE := false
endif

ifeq ($(SUPPORT_CLKGEN_SDDRV), true)
SUPPORT_CLKGEN_NEED_INIT := false
SUPPORT_CLKGEN_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_CLKGEN_SAMPLE_CODE := false
GLOBAL_DEFINES += SEC_SYSTEM_CFG=1
endif

ifeq ($(SUPPORT_SPINOR_SDDRV), true)
SUPPORT_SPINOR_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_SPINOR_SAMPLE_CODE := false

ifeq ($(TOGGLE_OSPI_RESET_ENABLE), true)
GLOBAL_DEFINES += TOGGLE_OSPI_RESET_ENABLE=1
endif
endif

ifeq ($(SUPPORT_DMA_SDDRV), true)
SUPPORT_DMA_SYSTEM_INIT := false
SUPPORT_DMA_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_DMA_SAMPLE_CODE := false
endif

ifeq ($(SYSTEM_TIMER), sdrv_timer)
GLOBAL_DEFINES += SDRV_TIMER=1
endif

ifeq ($(SUPPORT_BOARDINFO), true)
BOARDINFO_HWID_USR ?=true
GLOBAL_DEFINES += \
		SUPPORT_BOARDINFO=1

MODULES += \
	lib/version

endif

ifeq ($(BOOT_TYPE), emmc)
GLOBAL_DEFINES += BOOT_TYPE_EMMC=1
endif

# Platform modules.
include $(LOCAL_DIR)/../chipdev/rules.mk
include $(LOCAL_DIR)/../hal/rules.mk
include $(LOCAL_DIR)/../framework/rules.mk
include $(LOCAL_DIR)/../application/rules.mk

MODULES += \
	app/shell \
	application/system/dloader

#debug
ifneq ($(DEBUG), 0)
MODULES += lib/debugcommands
endif

ifeq ($(SUPPORT_HTOL_TEST), true)
MODULES += \
	app/tests
endif

GLOBAL_DEFINES += \
	NOT_USE_SYS_CFG=1

# spi 
DLOADER_USE_SPI2 ?= false
ifeq ($(DLOADER_USE_SPI2), true)
GLOBAL_DEFINES += DLOADER_USE_SPI=1
endif

    
