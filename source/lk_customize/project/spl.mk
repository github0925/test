##########################
# spl build
##########################
define add_defines_cond_true
$(if $(filter true, $($(strip $(1)))),$(eval GLOBAL_DEFINES += $(strip $(1))=1))
endef

#begin fk
#make CHIPVERSION=d9 SD_PROJECT=default  spl PROJ_POSTFIX=_d9_ref
LOCAL_DIR := $(GET_LOCAL_DIR)
$(info "----------1------------- $(LOCAL_DIR)")
TARGET := reference_d9
PROJECT :=$(SD_PROJECT)
include $(LOCAL_DIR)/../chipcfg/generate/$(CHIPVERSION)/projects/$(SD_PROJECT)/spl_cfg.mk
##end
DEBUG ?= 0
WITH_KERNEL_VM ?=0

include $(LOCAL_DIR)/../chipcfg/rules.mk

MEMBASE ?= 0x00140000   # iram2
ifeq ($(NO_DDR), true)
MEMSIZE ?= 0x60000

DLOADER_BASE ?= 0x1a0000
DLOADER_MAX_SIZE ?= 0x60000

$(call add_defines_cond_true,NO_DDR)

SUPPORT_MODULE_HELPER_SDDRV := false
else
MEMSIZE ?= 0x80000      # 512 KB

DLOADER_BASE ?= $(SEC_MEMBASE)
DLOADER_MAX_SIZE ?= $(SEC_MEMSIZE)
endif

SSYSTEM_BASE ?= $(SEC_MEMBASE)
SSYSTEM_MAX_SIZE ?= $(SEC_MEMSIZE)

GLOBAL_DEFINES += EMU=1
GLOBAL_DEFINES += \
	WITH_NO_PHYS_RELOCATION=1 \
	SSYSTEM_BASE=$(SSYSTEM_BASE) \
	SSYSTEM_MAX_SIZE=$(SSYSTEM_MAX_SIZE) \
	DLOADER_BASE=$(DLOADER_BASE) \
	DLOADER_MAX_SIZE=$(DLOADER_MAX_SIZE)

MEMDISK_BASE ?= 0x70000000
MEMDISK_SIZE ?= 0x4000000

KERNEL_BASE = $(MEMBASE)
ARM_WITH_VFP := 1
ARM_VFP_D16 := 1

GLOBAL_LDFLAGS += \
	--entry=$(MEMBASE)

GLOBAL_DEFINES += \
	NOT_OUT_PUT_HWID=1

BOOTDEVICE ?= BOOTDEVICE_EMMC
ifeq ($(BOOTDEVICE), MEMDISK)
GLOBAL_DEFINES += \
	BACKDOOR_DDR=1
endif

ifeq ($(BOOTDEVICE), USB)
GLOBAL_DEFINES += \
	BOOTDEVICE_USB=1
endif

NEED_CHANGE_VOLTAGE ?= false
ifeq ($(NEED_CHANGE_VOLTAGE), true)
GLOBAL_DEFINES += \
	NEED_CHANGE_VOLTAGE=1
endif

# log buf
ENABLE_LOG_BUF ?= 1
GLOBAL_DEFINES += \
       ENABLE_LOG_BUF=$(ENABLE_LOG_BUF) \
       MAX_LOG_BUF_LEN=4096

SYSTEM_TIMER ?= sdrv_timer
ifeq ($(SYSTEM_TIMER), sdrv_timer)
GLOBAL_DEFINES += SDRV_TIMER=1
endif

##########################
#driver modules define
##########################
ENABLE_IP_TEST ?= false
SUPPORT_I2C_SDDRV ?= false
SUPPORT_SCR_SDDRV ?= true
SUPPORT_ARM_GIC_SDDRV ?= true
SUPPORT_WDG_SDDRV ?= true
SUPPORT_RSTGEN_SDDRV ?= true
SYSTEM_TIMER ?= sdrv_timer
SUPPORT_MBOX_SDDRV ?= true
SUPPORT_PLL_SDDRV ?= true
SUPPORT_TIMER_SDDRV ?= true
SUPPORT_UART_DWDRV ?= true
SUPPORT_PORT_SDDRV ?= true
SUPPORT_DIO_SDDRV ?= true
SUPPORT_USB_SDDRV ?= true
SUPPORT_MMC_SDDRV ?= true
SUPPORT_CLKGEN_SDDRV ?= true
SUPPORT_DDR_INIT_n_TRAINING ?=true
SUPPORT_CE_SDDRV ?= true
SUPPORT_CPU_SDDRV ?= true
#DDR_TYPE := lpddr4x
LPDDR4 ?= false
DDR4 ?= false
DDR3 ?= false
DDR_DEBUG ?= false

SUPPORT_MODULE_HELPER_SDDRV ?=true
SUPPORT_BOARDINFO ?= true

##########################
#application modules define
##########################

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

ifeq ($(SUPPORT_PORT_SDDRV), true)
SUPPORT_PORT_SYSTEM_INIT := false
SUPPORT_PORT_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_PORT_SAMPLE_CODE := false
GLOBAL_DEFINES += SEC_SYSTEM_CFG=1
endif
ifeq ($(SUPPORT_DIO_SDDRV), true)
SUPPORT_DIO_SYSTEM_INIT := false
SUPPORT_DIO_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_DIO_SAMPLE_CODE := false
endif

ifeq ($(SUPPORT_CLKGEN_SDDRV), true)
SUPPORT_CLKGEN_NEED_INIT := true
SUPPORT_CLKGEN_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_CLKGEN_SAMPLE_CODE := false
GLOBAL_DEFINES += SEC_SYSTEM_CFG=1
endif

ifeq ($(SUPPORT_DMA_SDDRV), true)
SUPPORT_DMA_SYSTEM_INIT := false
SUPPORT_DMA_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_DMA_SAMPLE_CODE := true
endif

ifeq ($(SUPPORT_MODULE_HELPER_SDDRV), true)
MODULE_HELPER_PER_DDR ?= true

MODULE_HELPER_CKGEN_SEC ?= true
MODULE_HELPER_CKGEN_DISP ?= true
MODULE_HELPER_CKGEN_SAF ?= true
MODULE_HELPER_CKGEN_SOC ?= true
MODULE_HELPER_CKGEN_UUU ?= true
MODULE_HELPER_RSTGEN_ISO ?= true
MODULE_HELPER_RSTGEN_MODULE ?= true
MODULE_HELPER_RSTGEN_CORE ?= true
endif
$(call add_defines_cond_true,MODULE_HELPER_CKGEN_SEC)
$(call add_defines_cond_true,MODULE_HELPER_CKGEN_DISP)
$(call add_defines_cond_true,MODULE_HELPER_CKGEN_SAF)
$(call add_defines_cond_true,MODULE_HELPER_CKGEN_SOC)
$(call add_defines_cond_true,MODULE_HELPER_CKGEN_UUU)
$(call add_defines_cond_true,MODULE_HELPER_RSTGEN_ISO)
$(call add_defines_cond_true,MODULE_HELPER_RSTGEN_MODULE)
$(call add_defines_cond_true,MODULE_HELPER_RSTGEN_CORE)

ifeq ($(SUPPORT_DDR_INIT_n_TRAINING), true)
GLOBAL_DEFINES += DDR_INIT_N_TRAINING=1

ifeq ($(LPDDR4), true)
GLOBAL_DEFINES += LPDDR4=1
DDR_TYPE := lpddr4
else ifeq ($(DDR4), true)
GLOBAL_DEFINES += DDR4=1
DDR_TYPE := ddr4
else ifeq ($(DDR3), true)
GLOBAL_DEFINES += DDR3=1
DDR_TYPE := ddr3
else
GLOBAL_DEFINES += LPDDR4X=1
DDR_TYPE := lpddr4x
endif

#DDR_SIZE £º8G 4G 1G
#DDR_FREQ £º4266 3200 2133 1600 800
DDR_SIZE ?= 8GB
DDR_FREQ ?= 3200
GLOBAL_DEFINES += DDR_$(DDR_SIZE)_$(DDR_FREQ)=1 \
				  DDR_FREQ="$(DDR_FREQ)" \
				  DDR_SIZE="$(DDR_SIZE)"		  			  


ifeq ($(DDR_DEBUG), true)
GLOBAL_DEFINES += DDR_DEBUG=1
endif
endif

ifeq ($(SUPPORT_BOARDINFO), true)
BOARDINFO_HWID_USR ?=true
GLOBAL_DEFINES += \
		SUPPORT_BOARDINFO=1

MODULES += \
	lib/version
endif

# Platform modules.
include $(LOCAL_DIR)/../chipdev/rules.mk
include $(LOCAL_DIR)/../hal/rules.mk
include $(LOCAL_DIR)/../framework/rules.mk
include $(LOCAL_DIR)/../application/rules.mk

MODULES += \
	app/shell \
	application/system/spl

#debug
ifneq ($(DEBUG), 0)
MODULES += lib/debugcommands
endif

