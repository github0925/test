##########################
# ssystem build
##########################
define add_defines_if_true
$(if $(filter true, $($(strip $(1)))),$(eval GLOBAL_DEFINES += $(strip $(1))=1))
endef

LOCAL_DIR ?= $(GET_LOCAL_DIR)

DEBUG ?= 0
WITH_KERNEL_VM ?= 0
DDR_BOOT ?= false
VIRTUALIZATION_ENABLE ?= 0
VIRTUALIZATION_EXT ?= 0
#make CHIPVERSION=d9 SD_PROJECT=default  dloader PROJ_POSTFIX=_d9_ref
LOCAL_DIR := $(GET_LOCAL_DIR)
$(info "----------1------------- $(LOCAL_DIR)")
TARGET := reference_d9
PROJECT :=$(SD_PROJECT)
include $(LOCAL_DIR)/../chipcfg/generate/$(CHIPVERSION)/projects/$(SD_PROJECT)/ssystem_cfg.mk
##end

include $(LOCAL_DIR)/../chipcfg/rules.mk

ifeq ($(SUPPORT_FAST_BOOT), true)
DDR_BOOT := false
KEY_NODE_INFO_OFF := true
GLOBAL_DEFINES += \
        SUPPORT_FAST_BOOT=1
endif

ifeq ($(DDR_BOOT), true)
MEMBASE := $(SEC_MEMBASE)
MEMSIZE := $(SEC_MEMSIZE)
else
MEMBASE ?= 0x00140000 #iram2
MEMSIZE ?= 0x40000    #256k
endif

MEMDISK_BASE ?= 0x70000000
MEMDISK_SIZE ?= 0x4000000

KERNEL_BASE = $(MEMBASE)
ARM_WITH_VFP := 1
ARM_VFP_D16 := 1

USB2_POWER_EN ?= true
PLATFORM_G9X ?= false
PLATFORM_X9_PLUS ?= false
GLOBAL_DEFINES += \
	WITH_NO_PHYS_RELOCATION=1 \
	MEMBASE=$(MEMBASE) \
	VIRTUALIZATION_ENABLE=$(VIRTUALIZATION_ENABLE) \
	VIRTUALIZATION_EXT=$(VIRTUALIZATION_EXT)

ifeq ($(PLATFORM_X9_PLUS), true)
GLOBAL_DEFINES += PLATFORM_X9_PLUS=1
endif

ifeq ($(USB2_POWER_EN), true)
GLOBAL_DEFINES += USB2_POWER_EN=1
endif

GLOBAL_LDFLAGS += \
        --entry=$(MEMBASE)

#temp delay for dc, second
DELAY_TIME ?= 0
GLOBAL_DEFINES += \
        DELAY_TIME=$(DELAY_TIME)

AP_BACKDOOR_BASE ?= AP2_BOOTLOADER_MEMBASE
#BACKDOOR_DDR
ifeq ($(BOOTDEVICE), MEMDISK)
GLOBAL_DEFINES += \
	BACKDOOR_DDR=1 \
	AP_IMAGE_BASE=$(AP_BACKDOOR_BASE)
endif


HALF_SOURCE_CLOCK ?= false
ifeq ($(HALF_SOURCE_CLOCK),true)
GLOBAL_DEFINES += \
	HALF_SOURCE_CLOCK=1
endif

# log buf
ENABLE_LOG_BUF ?= 0
GLOBAL_DEFINES += \
       ENABLE_LOG_BUF=$(ENABLE_LOG_BUF) \
       MAX_LOG_BUF_LEN=4096

# backlight mode
BACKLIGHT_GPIO ?= false
ifeq ($(BACKLIGHT_GPIO), true)
GLOBAL_DEFINES += BACKLIGHT_GPIO=1
endif

##########################
#driver modules define
##########################
ENABLE_IP_TEST ?= false
SYSTEM_TIMER ?= sdrv_timer
SUPPORT_SCR_SDDRV ?= true
SUPPORT_ARM_GIC_SDDRV ?= true
SUPPORT_WDG_SDDRV ?= false
SUPPORT_RSTGEN_SDDRV ?= true
SUPPORT_UART_DWDRV ?= true
SUPPORT_CLKGEN_SDDRV ?= true
SUPPORT_PLL_SDDRV ?= true
SUPPORT_TIMER_SDDRV ?= true
SUPPORT_FIREWALL_SDDRV ?= true
SUPPORT_CE_SDDRV ?= true
SUPPORT_MMC_SDDRV ?= true
SUPPORT_PORT_SDDRV ?= true
SUPPORT_DIO_SDDRV ?= true
SUPPORT_DISP_LINK ?= false
SUPPORT_MBOX_SDDRV ?= true
SUPPORT_CPU_SDDRV ?= true
SUPPORT_VDSP_SDDRV ?= false
SUPPORT_PCIE_SDDRV ?= false
SUPPORT_FW_INIT_CMD ?= true

FIREWALL_ENABLE ?= false
SUPPORT_PVT_SYSTEM_APP ?= false
SUPPORT_MODULE_HELPER_SDDRV ?= false
SUPPORT_HTOL_TEST ?= false
SUPPORT_PFM_SDDRV ?= false
SUPPORT_REBOOT_CMD ?= false
SUPPORT_HSM ?= false
SUPPORT_BOARDINFO ?= true
ENABLE_WIFI ?= false
ENABLE_BT ?= false
SUPPORT_TEST_SLT ?= false
KEY_NODE_INFO_OFF ?= false

ifeq ($(SUPPORT_TEST_SLT), true)
GLOBAL_DEFINES += \
	SLT_RUN_IN_SEC_DOMAIN=1 \
	SUPPORT_TEST_SLT=1
endif

ifeq ($(SUPPORT_HSM), true)
SUPPORT_CE_SDDRV := true
SUPPORT_FIREWALL_SDDRV := true
GLOBAL_DEFINES += SUPPORT_HSM=1
endif

ifeq ($(SYSTEM_TIMER), sdrv_timer)
GLOBAL_DEFINES += SDRV_TIMER=1
endif

##########################
#application modules define
##########################
SUPPORT_SSYSTEM_SERVER := true

ifeq ($(VERIFIED_BOOT), true)
GLOBAL_DEFINES += \
	VERIFIED_BOOT=1
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

ifeq ($(SUPPORT_SCR_SDDRV),true)
SUPPORT_SCR_SYSTEM_APP := true
endif

ifeq ($(SUPPORT_FIREWALL_SDDRV), true)
SUPPORT_FIREWALL_TEST := $(ENABLE_IP_TEST)
SUPPORT_FIREWALL_SYSTEM_APP := true
ifeq ($(FIREWALL_ENABLE), true)
GLOBAL_DEFINES += \
        FIREWALL_ENABLE=1
endif
ifeq ($(SUPPORT_FW_INIT_CMD), true)
GLOBAL_DEFINES += \
        SUPPORT_FW_INIT_CMD=1
endif
endif

ifeq ($(SUPPORT_CE_SDDRV), true)
SUPPORT_CRYPTO_TEST ?= false
SUPPORT_CRYPTO_SAMPLE ?= false
GLOBAL_DEFINES += \
	CE_IN_SAFETY_DOMAIN=0
endif

ifeq ($(SUPPORT_MBOX_SDDRV), true)
SUPPORT_DCF := true
SUPPORT_3RD_RPMSG_LITE ?= true
SUPPORT_POSIX := true
SUPPORT_MBOX_SAMPLE_CODE ?= false
SUPPORT_RPMSG_SAMPLE_CODE ?= false
endif

ifeq ($(SUPPORT_REBOOT_CMD), true)
SUPPORT_WDG_SDDRV := true
endif

# Platform modules.
ifeq ($(SUPPORT_CLKGEN_SDDRV), true)
SUPPORT_CLKGEN_NEED_INIT := false
SUPPORT_CLKGEN_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_CLKGEN_SAMPLE_CODE := false
GLOBAL_DEFINES += SEC_SYSTEM_CFG=1
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

ifeq ($(SUPPORT_VDSP_SDDRV), true)
SUPPORT_VDSP_APP := false
GLOBAL_DEFINES += VDSP_ENABLE=1
endif

ifeq ($(SUPPORT_PCIE_SDDRV), true)
SUPPORT_PCIE_TEST := $(ENABLE_IP_TEST)
endif

ifeq ($(SUPPORT_PVT_SYSTEM_APP), true)
GLOBAL_DEFINES += \
	PVT_IN_SAFETY_DOMAIN=0
endif

ifeq ($(SUPPORT_DMA_SDDRV), true)
SUPPORT_DMA_SYSTEM_INIT := false
SUPPORT_DMA_IP_TEST := $(ENABLE_IP_TEST)
SUPPORT_DMA_SAMPLE_CODE := false
endif

ifeq ($(SUPPORT_PFM_SDDRV), true)
PFM_POOL_SIZE:=0x100000
ifeq ($(PFM_AUTO), true)
GLOBAL_DEFINES += \
       SDRV_DDR_PFM_AUTO_FLAG=0
else
GLOBAL_DEFINES += \
       SDRV_DDR_PFM_AUTO_FLAG=1
endif
endif

ifeq ($(SUPPORT_MODULE_HELPER_SDDRV), true)
MODULE_HELPER_PER_DDR ?= false
MODULE_HELPER_PER_SYS ?= false
MODULE_HELPER_PER_TEST ?= false
MODULE_HELPER_PER_DISP ?= true
endif

HALF_SOURCE_CLOCK ?= false
ifeq ($(DISABLE_UART_IRQ),true)
GLOBAL_DEFINES += \
       SSYSTEM_DISABLE_UART_IRQ=1
endif

ifeq ($(SUPPORT_BOARDINFO), true)
BOARDINFO_HWID_USR ?=true
GLOBAL_DEFINES += \
		SUPPORT_BOARDINFO=1

MODULES += \
	lib/version
endif

ifeq ($(ENABLE_WIFI), true)
GLOBAL_DEFINES +=ENABLE_WIFI=1
endif

ifeq ($(ENABLE_BT), true)
GLOBAL_DEFINES +=ENABLE_BT=1
endif

$(call add_defines_if_true,KEY_NODE_INFO_OFF)
$(call add_defines_if_true,NO_DDR)
$(call add_defines_if_true,PLATFORM_G9X)
$(call add_defines_if_true,PLATFORM_G9Q)
$(call add_defines_if_true,PLATFORM_G9S)

# Platform modules.
include $(LOCAL_DIR)/../chipdev/rules.mk
include $(LOCAL_DIR)/../hal/rules.mk
include $(LOCAL_DIR)/../framework/rules.mk
include $(LOCAL_DIR)/../application/rules.mk

MODULES += \
	app/shell \
	application/system/ssystem

#debug
#ifneq ($(DEBUG), 0)
MODULES += lib/debugcommands
#endif

SUPPORT_TEST_SLT ?= false
ifeq ($(SUPPORT_TEST_SLT), true)
MODULES += \
	lib/slt_module_test
GLOBAL_DEFINES += \
	SLT_RUN_IN_SEC_DOMAIN=1
endif

ifeq ($(SUPPORT_HTOL_TEST), true)
MODULES += \
	app/tests
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
