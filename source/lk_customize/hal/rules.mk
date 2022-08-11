LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include/ \

MODULE_DEPS += hal/wdg_hal/sd_wdg_hal
MODULE_DEPS += hal/rstgen_hal/sd_rstgen_hal
MODULE_DEPS += hal/mbox_hal/sd_mbox_hal
MODULE_DEPS += hal/res/

MODULE_DEPS += hal/i2c_hal/sd_i2c_hal
MODULE_DEPS += hal/crypto_hal/
MODULE_DEPS += hal/firewall_hal/
MODULE_DEPS += hal/net

ifeq ($(SUPPORT_VPU_CODAJ12_DRV), true)
MODULE_DEPS += hal/vpu_hal/codaj12
MJPEG_IRQ_NUM ?= 23
GLOBAL_DEFINES += \
	MJPEG_IRQ_NUM=$(MJPEG_IRQ_NUM)

endif
MODULE_DEPS += hal/uart_hal/dw_uart_hal
MODULE_DEPS += hal/timer_hal/sd_timer_hal
MODULE_DEPS += hal/pwm_hal/sd_pwm_hal
MODULE_DEPS += hal/interrupt_hal/arm_gic_hal
MODULE_DEPS += hal/spi_nor
MODULE_DEPS += hal/mmc
MODULE_DEPS += hal/pvt_hal/
MODULE_DEPS += hal/spi_hal/sd_spi_hal
MODULE_DEPS += hal/gpioirq_hal

ifeq ($(SUPPORT_DISP_SDDRV),true)
MODULE_DEPS += hal/disp_hal/sd_disp_hal
endif

ifeq ($(SUPPORT_G2DLITE_SDDRV),true)
MODULE_DEPS += hal/g2dlite_hal/sd_g2dlite_hal
endif

ifeq ($(SUPPORT_USB_SDDRV),true)
MODULE_DEPS += hal/usb_hal/sd_usb_hal
endif

ifeq ($(SUPPORT_SCR_SDDRV), true)
MODULE_DEPS += hal/scr/
endif

ifeq ($(SUPPORT_PLL_SDDRV), true)
MODULE_DEPS += hal/pll/
endif

ifeq ($(SUPPORT_PORT_SDDRV), true)
MODULE_DEPS += hal/port_hal/sd_port_hal
endif

ifeq ($(SUPPORT_DIO_SDDRV), true)
MODULE_DEPS += hal/dio_hal/sd_dio_hal
endif

ifeq ($(SUPPORT_PMU_SDDRV),true)
MODULE_DEPS += hal/pmu_hal/sd_pmu_hal
endif

MODULE_DEPS += hal/clkgen_hal/sd_clkgen_hal
MODULE_DEPS += hal/dma_hal/sd_dma_hal
MODULE_DEPS += hal/i2s_hal/sd_i2s_hal

ifeq ($(SUPPORT_CSI_SDDRV), true)
MODULE_DEPS += hal/csi_hal/sd_csi_hal
endif

ifeq ($(SUPPORT_MODULE_HELPER_SDDRV), true)
MODULE_DEPS += hal/module_helper_hal
endif

ifeq ($(SUPPORT_CPU_SDDRV), true)
MODULE_DEPS += hal/cpu_hal/sd_cpu_hal
endif

MODULE_DEPS += hal/spdif_hal/sd_spdif_hal

ifeq ($(SUPPORT_BOARDINFO), true)
MODULE_DEPS += hal/boardinfo
endif

