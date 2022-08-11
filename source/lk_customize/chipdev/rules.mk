LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	chipdev/i2c/dw_i2c \

SYSTEM_TIMER ?= generic
ifeq ($(SYSTEM_TIMER),generic)
	MODULE_DEPS += dev/timer/arm_generic
	GLOBAL_DEFINES += GENERIC_TIMER=1
else ifeq ($(SYSTEM_TIMER),sdrv_timer)
	GLOBAL_DEFINES += SDRV_TIMER=1
endif

ifeq ($(SUPPORT_WDG_SDDRV),true)
MODULE_DEPS += chipdev/wdg/sd_wdg
endif
ifeq ($(SUPPORT_RSTGEN_SDDRV),true)
MODULE_DEPS += chipdev/rstgen/sd_rstgen
endif

ifeq ($(SUPPORT_MBOX_SDDRV),true)
MODULE_DEPS += chipdev/mailbox/sd_mbox
endif

ifeq ($(SUPPORT_PORT_SDDRV),true)
MODULE_DEPS += chipdev/port/sd_port
endif
ifeq ($(SUPPORT_DIO_SDDRV),true)
MODULE_DEPS += chipdev/dio/sd_dio
endif

ifeq ($(SUPPORT_SCR_SDDRV),true)
MODULE_DEPS += chipdev/scr
endif

ifeq ($(SUPPORT_VPU_CODAJ12_DRV),true)
MODULE_DEPS += chipdev/vpu/codaj12
endif

ifeq ($(SUPPORT_CE_SDDRV),true)
MODULE_DEPS += chipdev/crypto/silex
endif

ifeq ($(SUPPORT_FIREWALL_SDDRV),true)
MODULE_DEPS += chipdev/firewall/semidrive
endif

ifeq ($(SUPPORT_UART_DWDRV),true)
MODULE_DEPS += chipdev/uart/dw_uart
endif

ifeq ($(SUPPORT_TIMER_SDDRV),true)
MODULE_DEPS += chipdev/timer/sd_timer
endif

ifeq ($(SUPPORT_PWM_SDDRV),true)
MODULE_DEPS += chipdev/pwm/sd_pwm
endif

ifeq ($(SUPPORT_ARM_GIC_SDDRV),true)
MODULE_DEPS += chipdev/interrupt/arm_gic
endif

ifeq ($(SUPPORT_USB_SDDRV),true)
MODULE_DEPS += chipdev/usb/dw_usb
endif

ifeq ($(SUPPORT_PLL_SDDRV),true)
MODULE_DEPS += chipdev/pll
endif

ifeq ($(SUPPORT_DISP_SDDRV),true)
MODULE_DEPS += chipdev/disp/sd_disp
endif

ifneq ($(findstring $(SUPPORT_DISP_SDDRV)$(SUPPORT_DISP_LINK)$(SUPPORT_DISP_TEST), true),)
#MODULE_DEPS += chipdev/disp/disp/sd_disp
endif

ifeq ($(SUPPORT_G2DLITE_SDDRV), true)
#MODULE_DEPS += chipdev/g2dlite/sd_g2dlite
endif

ifeq ($(SUPPORT_USB_HOST_SDDRV),true)
MODULE_DEPS += chipdev/usb/host
endif

ifeq ($(SUPPORT_CLKGEN_SDDRV),true)
MODULE_DEPS += chipdev/clkgen/sd_clkgen
endif

ifeq ($(SUPPORT_DMA_SDDRV),true)
ifeq ($(DOMAIN), safety)
MODULE_DEPS += chipdev/dma/dw_dma1
else
MODULE_DEPS += chipdev/dma/dw_dma
endif
endif

ifeq ($(SUPPORT_PMU_SDDRV),true)
MODULE_DEPS += chipdev/pmu/sd_pmu
endif

ifeq ($(SUPPORT_I2S_SDDRV),true)
MODULE_DEPS += chipdev/i2s/cadence_i2s
endif

ifeq ($(SUPPORT_MMC_SDDRV),true)
MODULE_DEPS += chipdev/mmc
endif

ifeq ($(SUPPORT_SPINOR_SDDRV),true)
MODULE_DEPS += chipdev/spi_nor
endif

ifeq ($(SUPPORT_PCIE_SDDRV),true)
MODULE_DEPS += chipdev/pcie/sd_pcie
endif

ifeq ($(SUPPORT_DDR_INIT_n_TRAINING),true)
MODULE_DEPS += chipdev/ddr/dw_umctl2
GLOBAL_INCLUDES += $(LOCAL_DIR)/ddr/dw_umctl2
endif

ifeq ($(SUPPORT_CSI_SDDRV),true)
MODULE_DEPS += chipdev/csi/sd_csi
endif

ifeq ($(SUPPORT_FUSE_CTRL),true)
MODULE_DEPS += chipdev/fuse_ctrl
endif

ifeq ($(SUPPORT_RTC_SDDRV),true)
MODULE_DEPS += chipdev/rtc
endif

ifeq ($(SUPPORT_SDPE_RPC), true)
GLOBAL_DEFINES += SUPPORT_SDPE_RPC=1
MODULES += chipdev/sdpe_rpc
endif

ifeq ($(SUPPORT_SPDIF_SDDRV),true)
MODULE_DEPS += chipdev/spdif
endif

ifeq ($(SUPPORT_FUNC_SAFETY_SDDRV),true)
MODULE_DEPS += chipdev/func_safety
endif

ifeq ($(SUPPORT_SPI_MASTER_SDDRV),true)
MODULE_DEPS += chipdev/spi/dw_ssi_spi
else ifeq ($(SUPPORT_SPI_SLAVE_SDDRV),true)
MODULE_DEPS += chipdev/spi/dw_ssi_spi
endif

ifeq ($(SUPPORT_ADC_SDDRV),true)
MODULE_DEPS += chipdev/adc
endif

ifeq ($(SUPPORT_CAN_SDDRV), true)
MODULE_DEPS += chipdev/can
endif
