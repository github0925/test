LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ifeq ($(SUPPORT_WDG_IP_TEST),true)
MODULE_DEPS += application/test/wdg
endif

ifeq ($(SUPPORT_WDG_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/wdg
endif

ifeq ($(SUPPORT_RSTGEN_IP_TEST),true)
MODULE_DEPS += application/test/rstgen
endif

ifeq ($(SUPPORT_RSTGEN_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/rstgen
endif

ifeq ($(SUPPORT_SCR_SYSTEM_APP),true)
MODULE_DEPS += application/system/scr
endif

ifeq ($(SUPPORT_FIREWALL_SYSTEM_APP),true)
MODULE_DEPS += application/system/firewall
endif

ifeq ($(SUPPORT_FIREWALL_TEST),true)
MODULE_DEPS += application/test/firewall/verification
MODULE_DEPS += application/test/firewall/debugcommands
endif

ifeq ($(SUPPORT_CRYPTO_TEST),true)
MODULE_DEPS += application/test/crypto
endif

ifeq ($(SUPPORT_CRYPTO_SAMPLE),true)
MODULE_DEPS += application/sample/crypto
endif

ifeq ($(SUPPORT_MBOX_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/mailbox
endif

ifeq ($(SUPPORT_TEST_SLT), true)
MODULE_DEPS += application/test/slt_slave
endif

ifeq ($(SUPPORT_I2C_IP_TEST),true)
MODULE_DEPS += application/test/i2c
MODULES += lib/sdunittest
endif

ifeq ($(SUPPORT_I2C_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/i2c
GLOBAL_DEFINES += ENABLE_SD_I2C_SAMPLE=1
endif

ifeq ($(SUPPORT_USB_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/usb
endif

ifeq ($(SUPPORT_PORT_SYSTEM_INIT),true)
MODULE_DEPS += application/system/port
endif

ifeq ($(SUPPORT_PORT_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/port
endif

ifeq ($(SUPPORT_DIO_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/dio
endif

ifeq ($(SUPPORT_DIO_IP_TEST),true)
MODULE_DEPS += application/test/dio
endif

ifeq ($(SUPPORT_SCR_TEST),true)
MODULE_DEPS += application/test/scr
endif

ifeq ($(SUPPORT_CLKGEN_IP_TEST),true)
MODULE_DEPS += application/test/clkgen
endif

ifeq ($(SUPPORT_CLKGEN_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/clkgen
endif

ifeq ($(SUPPORT_DMA_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/dma
MODULES += lib/unittest
endif

ifeq ($(SUPPORT_DMA_IP_TEST),true)
MODULE_DEPS += application/test/dma
MODULES += lib/unittest
endif

ifeq ($(SUPPORT_PVT_SYSTEM_APP),true)
MODULE_DEPS += application/system/pvt
endif

ifeq ($(SUPPORT_HTOL_TEST),true)
MODULE_DEPS += application/test/htol
endif

ifeq ($(SUPPORT_ADC_TEST),true)
MODULE_DEPS += application/test/adc
endif

ifeq ($(SUPPORT_PCIE_TEST),true)
MODULE_DEPS += application/test/pcie
endif

ifeq ($(SUPPORT_PFM_SDDRV), true)
MODULES += application/tools/pfmon
endif

ifeq ($(SUPPORT_REBOOT_CMD), true)
MODULES += application/system/reboot
endif

ifeq ($(SUPPORT_SPDIF_SDDRV), true)
MODULES += application/test/spdif
endif

ifeq ($(SUPPORT_I2S_IP_TEST),true)
MODULE_DEPS += application/test/i2s
endif

ifeq ($(SUPPORT_I2S_SAMPLE_CODE),true)
ifeq ($(SUPPORT_I2S_SDDRV_2_0), true)
MODULE_DEPS += application/sample/i2s
endif
endif

ifeq ($(SUPPORT_SPI_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/spi
endif

ifeq ($(SUPPORT_PCIE_EP_MODE),true)
MODULE_DEPS += application/system/pcie
endif