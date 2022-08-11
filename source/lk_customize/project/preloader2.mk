##########################
# preloader build
##########################
define add_defines_cond_true
$(if $(filter true, $($(strip $(1)))),$(eval GLOBAL_DEFINES += $(strip $(1))=1))
endef
#LOCAL_DIR ?= $(GET_LOCAL_DIR)

DEBUG ?=0
WITH_KERNEL_VM ?=1

#begin fk
#make CHIPVERSION=d9 SD_PROJECT=default  preloader PROJ_POSTFIX=_d9_ref
LOCAL_DIR := $(GET_LOCAL_DIR)
$(info "----------1------------- $(LOCAL_DIR)")
TARGET := reference_d9
PROJECT :=$(SD_PROJECT)
include $(LOCAL_DIR)/../chipcfg/generate/$(CHIPVERSION)/projects/$(SD_PROJECT)/preloader2_cfg.mk
##end

include $(LOCAL_DIR)/../chipcfg/rules.mk

MEMBASE ?= $(AP1_PRELOADER_MEMBASE)
MEMSIZE ?= $(AP1_PRELOADER_MEMSIZE)

MEMDISK_BASE ?= 0x80000000
MEMDISK_SIZE ?= 0x4000000

ifeq ($(BOOTDEVICE), MEMDISK)
GLOBAL_DEFINES += \
	BACKDOOR_DDR=1
endif

ifeq ($(WITH_KERNEL_VM), 1)
PERIPHERAL_BASE_VIRT := 0xFFFF000000000000
GLOBAL_DEFINES += \
        PERIPHERAL_BASE_VIRT=$(PERIPHERAL_BASE_VIRT)
endif
GLOBAL_LDFLAGS += \
        --entry=$(MEMBASE)

#flag fo loading two bootloaders
LOAD_TWO_BOOTLOADER ?= false
ifeq ($(LOAD_TWO_BOOTLOADER), true)
GLOBAL_DEFINES += \
	LOAD_TWO_BOOTLOADER=1
endif

# log buf
ENABLE_LOG_BUF ?= 0
GLOBAL_DEFINES += \
       ENABLE_LOG_BUF=$(ENABLE_LOG_BUF) \
       MAX_LOG_BUF_LEN=4096

##########################
#driver modules define
##########################
ENABLE_IP_TEST ?= false
SUPPORT_ARM_GIC_SDDRV ?= true
SUPPORT_WDG_SDDRV ?= true
SUPPORT_TIMER_SDDRV ?= true
SUPPORT_RSTGEN_SDDRV ?= false
SUPPORT_UART_DWDRV ?= true
SUPPORT_PLL_SDDRV ?= false
SUPPORT_SCR_SDDRV ?= false
SUPPORT_CLKGEN_SDDRV ?= true
SUPPORT_MMC_SDDRV ?= true
SYSTEM_TIMER ?= sdrv_timer
SUPPORT_PORT_SDDRV ?= false
SUPPORT_DIO_SDDRV ?= false
SUPPORT_CPU_SDDRV ?= true
SUPPORT_BOARDINFO ?= true
SUPPORT_CE_SDDRV ?= true
ENABLE_NFORE ?= false
SUPPORT_DCF ?= false
##########################
#application modules define
##########################
ifeq ($(ENABLE_NFORE), true)
GLOBAL_DEFINES +=ENABLE_NFORE=1
endif
ifeq ($(VERIFIED_BOOT), true)
GLOBAL_DEFINES += \
	VERIFIED_BOOT=1
endif

ifeq ($(SUPPORT_CE_SDDRV), true)
SUPPORT_CRYPTO_TEST := false
SUPPORT_CRYPTO_SAMPLE := false
GLOBAL_DEFINES += \
	WITH_SIMULATION_PLATFORM=0 \
	CE_IN_SAFETY_DOMAIN=0
endif

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
SUPPORT_CLKGEN_NEED_INIT := true
SUPPORT_CLKGEN_IP_TEST := $(ENABLE_IP_TEST)
GLOBAL_DEFINES += ECO_SYSTEM_CFG=1
SUPPORT_CLKGEN_SAMPLE_CODE := false
endif

ifeq ($(SUPPORT_PORT_SDDRV), true)
SUPPORT_PORT_SYSTEM_INIT := false
SUPPORT_PORT_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_PORT_SAMPLE_CODE := false
GLOBAL_DEFINES += ECO_SYSTEM_CFG=1
endif

ifeq ($(SUPPORT_DIO_SDDRV), true)
SUPPORT_DIO_SYSTEM_INIT := false
SUPPORT_DIO_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_DIO_SAMPLE_CODE := false
endif

ifeq ($(SUPPORT_DMA_SDDRV), true)
SUPPORT_DMA_SYSTEM_INIT := false
SUPPORT_DMA_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_DMA_SAMPLE_CODE := true
endif

ifeq ($(SUPPORT_BOARDINFO), true)
BOARDINFO_HWID_USR ?=true
GLOBAL_DEFINES += \
		SUPPORT_BOARDINFO=1

MODULES += \
	lib/version
endif

##########################
#all module load
##########################
include $(LOCAL_DIR)/../chipdev/rules.mk
include $(LOCAL_DIR)/../hal/rules.mk
include $(LOCAL_DIR)/../framework/rules.mk
include $(LOCAL_DIR)/../application/rules.mk

MODULES += \
	app/shell \

#debug
ifneq ($(DEBUG), 0)
MODULES += lib/debugcommands
endif

ifeq ($(ENABLE_UNIFY_UART), true)
SUPPORT_VIRT_UART := true
endif

SUPPORT_VIRT_UART ?= false
ifeq ($(SUPPORT_VIRT_UART), true)
GLOBAL_DEFINES += \
	VIRT_UART_MEMBASE=$(shell echo $$(($(SEC_MEMBASE) + 0x10000000))) \
	VIRT_UART_MEMSIZE=0x200000

GLOBAL_DEFINES += SUPPORT_VIRT_UART=1
endif
