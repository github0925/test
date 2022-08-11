##########################
#safety image build
##########################
LOCAL_DIR := $(GET_LOCAL_DIR)
$(info "----------1------------- $(LOCAL_DIR)")
PROJECT :=$(SD_PROJECT)
include $(LOCAL_DIR)/../chipcfg/generate/$(CHIPVERSION)/projects/$(SD_PROJECT)/dil2_cfg.mk

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
GLOBAL_DEFINES += WITH_NO_PHYS_RELOCATION=0

#======================#
#Safety Image run @ DDR or IRAM1 or `NOR` flash
#DDR start at $(SAF_MEMBASE), size is $(SAF_MEMSIZE)
#IRAM1 start at 0x100000,256K size
#NOR definition TBD
#======================#
include chipcfg/rules.mk

OSPI_DIRECT_ACCESS ?= 0
RELOCATED_MODULE :=
GLOBAL_DEFINES += XIP=0

# DIL2 may be loaded to IRAM2 by DIL,
# and reloacated to IRAM1 on reset.
# WITH_NO_PHYS_RELOCATION must be 0 for DIL2
MEMBASE := 0x00100000
MEMSIZE := 0x00080000

ifeq ($(VERIFIED_BOOT), true)
SUPPORT_FUSE_CTRL := true
GLOBAL_DEFINES += VERIFIED_BOOT=1
MODULES += lib/verified_boot
endif

GLOBAL_DEFINES += OSPI_DIRECT_ACCESS=$(OSPI_DIRECT_ACCESS)

KERNEL_BASE = $(MEMBASE)

GLOBAL_LDFLAGS += \
        --entry=$(MEMBASE)

ifeq ($(WITH_KERNEL_VM),1)
PERIPHERAL_BASE_VIRT ?= 0xFFFF000000000000
GLOBAL_DEFINES += \
        PERIPHERAL_BASE_VIRT=$(PERIPHERAL_BASE_VIRT)
endif

ifeq ($(ENABLE_UNIFY_UART), true)
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
SUPPORT_SPINOR_SDDRV ?= true
SUPPORT_I2C_SDDRV ?= false
SUPPORT_I2S_SDDRV ?= false
SUPPORT_PLL_SDDRV ?= true
SUPPORT_LIN_SDDRV ?= false
SUPPORT_RTC_SDDRV ?= false
SUPPORT_PMU_SDDRV ?= false
SUPPORT_CAN_SDDRV ?= false
ENABLE_PIN_DELTA_CONFIG ?= false
SUPPORT_CE_SDDRV  ?= true
SUPPORT_VPU_CODAJ12_DRV ?=false
SUPPORT_DISP_SDDRV ?= true
$(call add_defines_if_true,SUPPORT_DISP_SDDRV)

SUPPORT_CSI_SDDRV ?= false
SUPPORT_PVT_SDDRV ?= false
SUPPORT_BOARDINFO ?= true

SUPPORT_ETHERNET1 ?= false
SUPPORT_CPU_SDDRV ?= true

SUPPORT_AUDIO_MANAGER ?= false
SUPPORT_AUDIO_SVC ?= false
SUPPORT_FAST_BOOT ?= true

SUPPORT_NEXT_OS ?= true
$(call add_defines_if_true,SUPPORT_NEXT_OS)

LD_SUFFIX := -dil2
SUPPORT_DIL2_INIT := true
$(call add_defines_if_true,SUPPORT_DIL2_INIT)

ifeq ($(SUPPORT_AUDIO_MANAGER),true)
SUPPORT_AUDIO_SVC := true
GLOBAL_DEFINES += \
        ENABLE_AUDIO_MANAGER=1
else
SUPPORT_AM_TEST := false
SUPPORT_AUDIO_SVC := false
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

MODULE_HELPER_CKGEN_SEC ?= false
MODULE_HELPER_CKGEN_DISP ?= true
MODULE_HELPER_CKGEN_SAF ?= false
MODULE_HELPER_CKGEN_SOC ?= true
MODULE_HELPER_CKGEN_UUU ?= false
MODULE_HELPER_RSTGEN_ISO ?= false
MODULE_HELPER_RSTGEN_MODULE ?= true
MODULE_HELPER_RSTGEN_CORE ?= false

endif

$(call add_defines_if_true,MODULE_HELPER_CKGEN_SEC)
$(call add_defines_if_true,MODULE_HELPER_CKGEN_DISP)
$(call add_defines_if_true,MODULE_HELPER_CKGEN_SAF)
$(call add_defines_if_true,MODULE_HELPER_CKGEN_SOC)
$(call add_defines_if_true,MODULE_HELPER_CKGEN_UUU)
$(call add_defines_if_true,MODULE_HELPER_RSTGEN_ISO)
$(call add_defines_if_true,MODULE_HELPER_RSTGEN_MODULE)
$(call add_defines_if_true,MODULE_HELPER_RSTGEN_CORE)

ifeq ($(SUPPORT_SPINOR_SDDRV),true)
SUPPORT_SPINOR_IP_TEST := false
SUPPORT_SPINOR_SAMPLE_CODE := false
NORFLASH_DEVICE_TYPE ?= mt35x
endif

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

SUPPORT_VIRTCAN_CLIENT ?= false
SUPPORT_VIRTLIN_CLIENT ?= false
##########################
#application module define
##########################

ifeq ($(SUPPORT_WDG_SDDRV),true)
SUPPORT_WDG_NEED_INIT := true
SUPPORT_WDG_SYSTEM_INIT := false
SUPPORT_WDG_IP_TEST := false
SUPPORT_WDG_SAMPLE_CODE := false
endif
ifeq ($(SUPPORT_DCF),true)
SUPPORT_MBOX_SAMPLE_CODE ?= false
SUPPORT_RPMSG_SAMPLE_CODE ?= true
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

ifeq ($(ENABLE_PIN_DELTA_CONFIG), true)
GLOBAL_DEFINES +=ENABLE_PIN_DELTA_CONFIG=1
endif

ifeq ($(SUPPORT_FAST_BOOT), true)
GLOBAL_DEFINES += \
        SUPPORT_FAST_BOOT=1
endif

ifeq ($(SUPPORT_PMIC_LP875XX), true)
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
	application/system/soc-init \
	lib/apploader \
	lib/libc \
	lib/heap \

