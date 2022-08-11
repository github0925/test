##########################
#safety image build
##########################
#total cmd

#make CHIPVERSION=d9 SD_PROJECT=default  bootloader PROJ_POSTFIX=_d9_ref

LOCAL_DIR := $(GET_LOCAL_DIR)
$(info "----------1------------- $(LOCAL_DIR)")
PROJECT :=$(SD_PROJECT)
include $(LOCAL_DIR)/../chipcfg/generate/$(CHIPVERSION)/projects/$(SD_PROJECT)/safety_cfg.mk

define add_defines_if_true
$(if $(filter true, $($(strip $(1)))),$(eval GLOBAL_DEFINES += $(strip $(1))=1))
endef

#1. SOC definition
DOMAIN := safety
DEBUG ?= 0
SYSTEM_TIMER ?= sdrv_timer
GLOBAL_DEFINES += SAF_SYSTEM_CFG=1
GLOBAL_DEFINES += IN_SAFETY_DOMAIN=1

#2.Memory configuration
WITH_KERNEL_VM ?= 0
WITH_NO_PHYS_RELOCATION ?= 1
GLOBAL_DEFINES += \
	WITH_NO_PHYS_RELOCATION=$(WITH_NO_PHYS_RELOCATION)

#======================#
#Safety Image run @ DDR or IRAM1 or `NOR` flash
#DDR start at $(SAF_MEMBASE), size is $(SAF_MEMSIZE)
#IRAM1 start at 0x100000,256K size
#NOR definition TBD
#======================#
include chipcfg/rules.mk

DISKD ?= 6

ifeq ($(EXECUTION_PLACE),norflash)
GLOBAL_DEFINES += XIP=1
RELOCATED_MODULE := \
	chipdev/clkgen \
	chipdev/pll \
	chipdev/spi_nor \
	chipdev/rstgen \
	application/system/ospi_handover

MEMBASE ?= 0x00100000
MEMSIZE ?= 0x00040000
OSPI_DIRECT_ACCESS ?= 1
$(info Switch to norflash as execution target -- Module:$(RELOCATED_MODULE) will be relocated into RAM)
else ifeq ($(EXECUTION_PLACE),iram)
RELOCATED_MODULE :=
GLOBAL_DEFINES += XIP=0
DISKD := 4
OSPI_DIRECT_ACCESS ?= 0

MEMBASE ?= 0x00100000
MEMSIZE ?= 0x00040000
$(info Switch to iram as execution target)
else ifeq ($(EXECUTION_PLACE), ddr)
DISKD := 4
OSPI_DIRECT_ACCESS ?= 0
RELOCATED_MODULE :=
GLOBAL_DEFINES += XIP=0
MEMBASE ?= $(SAF_MEMBASE)
MEMSIZE ?= $(SAF_MEMSIZE)
$(info Switch to ddr as execution target)
else
$(error Wrong EXECUTION_PLACE:$(EXECUTION_PLACE). Must specify execution place. EXECUTION_PLACE:=norflash or EXECUTION_PLACE:=iram)
endif

ifneq ($(PRELOAD_RES_SIZE),)
	MEMDISK_BASE := $(shell echo $$(($(MEMBASE) + $(MEMSIZE) - $(PRELOAD_RES_SIZE))))
	MEMDISK_SIZE := $(PRELOAD_RES_SIZE)
	DISKD := 7
endif

ifeq ($(VERIFIED_BOOT), true)
SUPPORT_FUSE_CTRL := true
GLOBAL_DEFINES += VERIFIED_BOOT=1
MODULES += lib/verified_boot
endif

GLOBAL_DEFINES += DISKD=$(DISKD)
GLOBAL_DEFINES += OSPI_DIRECT_ACCESS=$(OSPI_DIRECT_ACCESS)

ROMBASE ?= 0x4007800
ROMSIZE ?= 0x200000
KERNEL_BASE = $(MEMBASE)

GLOBAL_DEFINES += \
	MEMBASE=$(MEMBASE) \
	ROMBASE=$(ROMBASE)
GLOBAL_LDFLAGS += \
        --entry=$(MEMBASE)

ifeq ($(WITH_KERNEL_VM),1)
PERIPHERAL_BASE_VIRT ?= 0xFFFF000000000000
GLOBAL_DEFINES += \
        PERIPHERAL_BASE_VIRT=$(PERIPHERAL_BASE_VIRT)
endif

ifeq ($(ENABLE_UNIFY_UART), true)
SUPPORT_VIRT_CONSOLE := true
endif

SUPPORT_VIRT_CONSOLE ?= false
ifeq ($(SUPPORT_VIRT_CONSOLE), true)
GLOBAL_DEFINES += SUPPORT_VIRT_CONSOLE=1
SUPPORT_VIRT_UART := true
endif

SUPPORT_VIRT_UART ?= false
ifeq ($(SUPPORT_VIRT_UART), true)
GLOBAL_DEFINES += \
	VIRT_UART_MEMBASE=$(SEC_MEMBASE) \
	VIRT_UART_MEMSIZE=0x200000

GLOBAL_DEFINES += SUPPORT_VIRT_UART=1
endif

#3.CPU arch & instruction configuration
ARM_WITH_VFP ?= 1
ARM_VFP_D16 ?= 1
WITH_SMP ?=0

ifeq ($(WITH_SMP),1)
$(error Unsupport SMP mode)
endif

##########################
#chipdev module define
##########################
SUPPORT_WDG_SDDRV ?= false
SUPPORT_UART_DWDRV ?= true
SUPPORT_MBOX_SDDRV ?= false
SUPPORT_TIMER_SDDRV ?= true
SUPPORT_ARM_GIC_SDDRV ?= true
SUPPORT_SCR_SDDRV ?= true
SUPPORT_RSTGEN_SDDRV ?= true
SUPPORT_PORT_SDDRV ?= true
SUPPORT_DIO_SDDRV ?= true
SUPPORT_CLKGEN_SDDRV ?= true
SUPPORT_DMA_SDDRV ?= false
SUPPORT_SPINOR_SDDRV ?= false
SUPPORT_I2C_SDDRV ?= false
SUPPORT_I2S_SDDRV ?= false
SUPPORT_PLL_SDDRV ?= true
SUPPORT_LIN_SDDRV ?= false
SUPPORT_RTC_SDDRV ?= false
SUPPORT_PMU_SDDRV ?= false
SUPPORT_CAN_SDDRV ?= false
SUPPORT_UNIFIED_BOOT ?= true
ENABLE_PIN_DELTA_CONFIG ?= false
SUPPORT_CE_SDDRV  ?= true
SUPPORT_VPU_CODAJ12_DRV ?=false
SUPPORT_DISP_SDDRV ?= false
SUPPORT_CSI_SDDRV ?= false

SUPPORT_LVGL_GUI ?=false
SUPPORT_LVGL_EXAMPLES ?=false

SUPPORT_PVT_SDDRV ?= true
SUPPORT_BOARDINFO ?= true
SUPPORT_FATFS ?= true

SUPPORT_ETHERNET1 ?= false
SUPPORT_FAST_BOOT ?= false
SUPPORT_CPU_SDDRV ?= true

SUPPORT_AUDIO_MANAGER ?= false
SUPPORT_AUDIO_AGENT ?= false
SUPPORT_AUDIO_TEST ?= false


SUPPORT_PRINT_BUILDCONFIG ?= true

ifeq ($(SUPPORT_AUDIO_MANAGER),true)
ifeq ($(SUPPORT_I2S_SDDRV_2_0),true)
SUPPORT_AUDIO_AGENT := true
endif
GLOBAL_DEFINES += \
        ENABLE_AUDIO_MANAGER=1

else
SUPPORT_AM_TEST := false
endif


ENABLE_BL_BY_GPIO ?= false
ifeq ($(ENABLE_BL_BY_GPIO),true)
GLOBAL_DEFINES += \
        ENABLE_BL_BY_GPIO=1
endif

SUPPORT_MODULE_HELPER_SDDRV ?= true
ifeq ($(SUPPORT_MODULE_HELPER_SDDRV),true)
MODULE_HELPER_PER_DDR ?= false
MODULE_HELPER_PER_SYS ?= false
MODULE_HELPER_PER_TEST ?= false
MODULE_HELPER_PER_DISP ?= true

MODULE_HELPER_CKGEN_SEC ?= true
MODULE_HELPER_CKGEN_DISP ?= true
MODULE_HELPER_CKGEN_SAF ?= true
MODULE_HELPER_CKGEN_SOC ?= true
MODULE_HELPER_CKGEN_UUU ?= true
MODULE_HELPER_RSTGEN_ISO ?= true
MODULE_HELPER_RSTGEN_MODULE ?= true
MODULE_HELPER_RSTGEN_CORE ?= true
endif

$(call add_defines_if_true,MODULE_HELPER_CKGEN_SEC)
$(call add_defines_if_true,MODULE_HELPER_CKGEN_DISP)
$(call add_defines_if_true,MODULE_HELPER_CKGEN_SAF)
$(call add_defines_if_true,MODULE_HELPER_CKGEN_SOC)
$(call add_defines_if_true,MODULE_HELPER_CKGEN_UUU)
$(call add_defines_if_true,MODULE_HELPER_RSTGEN_ISO)
$(call add_defines_if_true,MODULE_HELPER_RSTGEN_MODULE)
$(call add_defines_if_true,MODULE_HELPER_RSTGEN_CORE)

ifeq ($(SUPPORT_PVT_SDDRV), true)
SUPPORT_PVT_APP ?= true
SUPPORT_PVT_APP_MONITOR ?= false
SUPPORT_PVT_APP_PRINT ?= true

GLOBAL_DEFINES += \
	PVT_IN_SAFETY_DOMAIN=1

PVT_AUTO_OUT ?= false
ifeq ($(PVT_AUTO_OUT), true)
GLOBAL_DEFINES += \
        PVT_AUTO_OUT_PRT=1
endif
endif

ifeq ($(SUPPORT_DISP_SDDRV),true)
CONFIG_PANELS ?= "default"
CONFIG_CUSTOM_LCM ?= "lvds_youda_1920x720_lcd lvds_atk10_1_1280x800_lcd"
endif

ifeq ($(SUPPORT_CSI_SDDRV),true)
SUPPORT_MIPICSI_SDDRV := true
SUPPORT_CSI_SAMPLE_CODE := false
endif

NEED_CHANGE_VOLTAGE ?= false
ifeq ($(NEED_CHANGE_VOLTAGE), true)
GLOBAL_DEFINES += \
        NEED_CHANGE_VOLTAGE=1
endif

ifeq ($(SUPPORT_FAST_BOOT), true)
GLOBAL_DEFINES += \
        SUPPORT_FAST_BOOT=1
endif

##########################
#kernel module define
##########################
PORTED_KERNEL ?= FreeRTOS
##########################
#library define
##########################
SUPPORT_DCF ?= true
SUPPORT_RPC ?= true

SUPPORT_VIRTCAN_CLIENT ?= false
SUPPORT_VIRTLIN_CLIENT ?= false
##########################
#application module define
##########################
SUPPORT_TOOL_POWEROFF ?= true

ifeq ($(SUPPORT_WDG_SDDRV),true)
SUPPORT_WDG_NEED_INIT := true
SUPPORT_WDG_SYSTEM_INIT := false
SUPPORT_WDG_IP_TEST := false
SUPPORT_WDG_SAMPLE_CODE := false
endif
ifeq ($(SUPPORT_DCF),true)
SUPPORT_MBOX_SAMPLE_CODE ?= false
SUPPORT_RPMSG_SAMPLE_CODE ?= true
SUPPORT_3RD_RPMSG_LITE ?= false
SUPPORT_IPCC_RPMSG ?= true
endif

ifeq ($(SUPPORT_PMU_SDDRV),true)
SUPPORT_PMU_TEST := false
endif

ifeq ($(SUPPORT_TIMER_SDDRV),true)
SUPPORT_TIMER_APP := false
endif

MODULE_HELPER_PORT ?= true
ifeq ($(SUPPORT_PORT_SDDRV),true)
ifeq ($(MODULE_HELPER_PORT),true)
SUPPORT_PORT_HELPER := true
endif
SUPPORT_PIN_INFO := true
SUPPORT_PORT_SYSTEM_INIT := false
SUPPORT_PORT_IP_TEST := false
SUPPORT_PORT_SAMPLE_CODE := false
endif

ifeq ($(SUPPORT_CLKGEN_SDDRV),true)
SUPPORT_CLKGEN_NEED_INIT := true
SUPPORT_CLKGEN_IP_TEST := false
SUPPORT_CLKGEN_SAMPLE_CODE := false
endif

ifeq ($(SUPPORT_I2S_SDDRV),true)
SUPPORT_I2S_IP_TEST := false
SUPPORT_I2S_SAMPLE_CODE := false
endif

ifeq ($(SUPPORT_SPINOR_SDDRV),true)
SUPPORT_SPINOR_IP_TEST := false
SUPPORT_SPINOR_SAMPLE_CODE := false
NORFLASH_DEVICE_TYPE ?= mt35x
endif

ifeq ($(SUPPORT_ARM_GIC_SDDRV),true)
GLOBAL_DEFINES += \
	GIC_400=1
endif

ifeq ($(SUPPORT_RTC_SDDRV),true)
GLOBAL_DEFINES += \
	ENABLE_RTC
endif

ifeq ($(SUPPORT_ETHERNET1), true)
GLOBAL_DEFINES += ENABLE_ETHERNET1=1
endif

ifeq ($(SUPPORT_CE_SDDRV),true)
SUPPORT_CRYPTO_TEST ?= false

GLOBAL_DEFINES += \
	WITH_SIMULATION_PLATFORM=1 \
	CE_IN_SAFETY_DOMAIN=1
endif

ifeq ($(SUPPORT_VDSP_SDDRV), true)
SUPPORT_VDSP_APP := false
GLOBAL_DEFINES += VDSP_ENABLE=1
endif

ifeq ($(SUPPORT_BOARDINFO), true)
GLOBAL_DEFINES += \
	SUPPORT_BOARDINFO=1

SUPPORT_FUSE_CTRL ?= true
BOARDINFO_HW ?=true
BOARDINFO_HWID_USR ?=true
endif

ifeq ($(SUPPORT_3RD_RPMSG_LITE), true)
GLOBAL_DEFINES += \
	RPMSG_MASTER_DEVICE=1
SUPPORT_UPDATE_MONITOR ?= true
endif

ifeq ($(ENABLE_PIN_DELTA_CONFIG), true)
GLOBAL_DEFINES +=ENABLE_PIN_DELTA_CONFIG=1
endif

ifeq ($(SUPPORT_UNIFIED_BOOT), true)
MODULES += application/system/boot_ss
endif

ifeq ($(SUPPORT_FATFS), true)
MODULES += lib/storage_device
endif

ifeq ($(SUPPORT_PMIC_LP875XX), true)
SUPPORT_POWER ?= true
$(call add_defines_if_true,SUPPORT_POWER)

MODULE_DEPS += lib/power exdev/pmic

endif

$(call add_defines_if_true,PLATFORM_G9X)
$(call add_defines_if_true,PLATFORM_G9Q)
$(call add_defines_if_true,PLATFORM_V9F)
$(call add_defines_if_true,PLATFORM_V9TS_B)
$(call add_defines_if_true,PLATFORM_BF200)

##########################
#all module load
##########################
include chipdev/rules.mk
include hal/rules.mk
include framework/rules.mk
include application/rules.mk
include 3rd/rules.mk

#load lib module
MODULES += \
	lib/shell \
	lib/apploader \
	lib/debugcommands \
	lib/libc \
	lib/heap \
	lib/sdrpc

ifeq ($(SUPPORT_TEST_SLT), true)
MODULES += \
	lib/slt_module_test
endif
# backlight mode
BACKLIGHT_GPIO ?= false
ifeq ($(BACKLIGHT_GPIO), true)
GLOBAL_DEFINES += BACKLIGHT_GPIO=1
endif

# spi 
USE_SPI2_DEMO ?= false
ifeq ($(USE_SPI2_DEMO), true)
GLOBAL_DEFINES += OSPI_HANDOVER_SPI=1
endif