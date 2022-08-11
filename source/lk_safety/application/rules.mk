LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ifeq ($(SUPPORT_WDG_SYSTEM_INIT),true)
MODULE_DEPS += application/system/wdg
endif

ifeq ($(SUPPORT_WDG_IP_TEST),true)
MODULE_DEPS += application/test/wdg
endif

ifeq ($(SUPPORT_WDG_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/wdg
endif

ifeq ($(SUPPORT_MBOX_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/mailbox
endif

ifeq ($(SUPPORT_EPU_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/epu
endif

ifeq ($(SUPPORT_VPU_CODAJ12_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/vpu/codaj12
endif

ifeq ($(SUPPORT_SLIMAI_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/slimAI
endif

ifeq ($(SUPPORT_PMU_TEST),true)
MODULE_DEPS += application/test/pmu
endif

ifeq ($(SUPPORT_MCAL_TEST),true)
MODULE_DEPS += application/test/mcal
endif

ifeq ($(SUPPORT_CSI_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/csi
endif

ifeq ($(SUPPORT_GETTING_START_DEMO),true)
MODULE_DEPS += application/sample/getting_started
endif

ifeq ($(SUPPORT_SPI_MASTER_DEMO),true)
MODULE_DEPS += application/sample/spi
endif

ifeq ($(SUPPORT_I2S_IP_TEST),true)
MODULE_DEPS += application/test/i2s
endif

ifeq ($(SUPPORT_I2S_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/i2s
endif

ifeq ($(SUPPORT_RPC_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/erpc_svc
endif

ifeq ($(SUPPORT_PWM_IP_TEST),true)
MODULE_DEPS += application/test/pwm
endif

ifeq ($(SUPPORT_AUDIO_PWM_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/audio_pwm
endif

ifeq ($(SUPPORT_BACKLIGHT_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/backlight
endif

ifeq ($(SUPPORT_CAN_SAMPLE_CODE), true)
MODULE_DEPS += application/sample/can
endif

ifeq ($(SUPPORT_SD_PFM), true)
MODULES += application/services/pfmon
endif

ifeq ($(SUPPORT_PVT_APP),true)
MODULE_DEPS += application/system/pvt
endif

ifeq ($(SUPPORT_PVT_TEST),true)
MODULE_DEPS += application/test/pvt
endif

ifeq ($(SUPPORT_MJPEG_TEST),true)
MODULE_DEPS += application/test/mjpeg
endif

ifeq ($(SUPPORT_TIMER_APP),true)
MODULE_DEPS += application/sample/timer
endif

ifeq ($(SUPPORT_ADC_SAMPLE_CODE), true)
MODULE_DEPS += application/sample/adc
endif

ifeq ($(SUPPORT_OPT3001_SAMPLE_CODE), true)
MODULE_DEPS += application/sample/opt3001
endif

ifeq ($(SUPPORT_SPINOR_SAMPLE_CODE),true)
MODULE_DEPS += application/sample/spi_nor
endif

ifeq ($(SUPPORT_SPINOR_IP_TEST),true)
MODULE_DEPS += application/test/spi_nor
endif

ifeq ($(SUPPORT_SEM_MONITOR),true)
MODULE_DEPS += application/tools/sem_monitor
endif

ifeq ($(SUPPORT_TEST_SLT), true)
MODULE_DEPS += application/test/slt
endif

ifeq ($(SUPPORT_EARLY_APP),true)
MODULE_DEPS += application/early_app
endif


ifeq ($(SUPPORT_SDPE_TEST),true)
MODULE_DEPS += application/test/sdpe
endif

ifeq ($(SUPPORT_CAN_STR),true)
MODULE_DEPS += application/test/can_str
endif

ifeq ($(SUPPORT_COM_SAMPLE), true)
MODULE_DEPS += application/sample/sig_route
endif

ifeq ($(SUPPORT_RTC_DEMO), true)
MODULE_DEPS += application/sample/rtc
endif

ifeq ($(SUPPORT_CAN_TEST),true)
MODULE_DEPS += application/test/can
GLOBAL_DEFINES += SUPPORT_CAN_TEST=1
endif

ifeq ($(SUPPORT_LIN_TEST),true)
MODULE_DEPS += application/test/lin
GLOBAL_DEFINES += SUPPORT_LIN_TEST=1
endif

ifeq ($(SUPPORT_DISP_SDDRV), true)
MODULE_DEPS += application/test/disp
endif

ifeq ($(SUPPORT_G2DLITE_SDDRV), true)
MODULE_DEPS += application/test/g2dlite
endif

ifeq ($(SUPPORT_KERNEL_MONITOR),true)
MODULE_DEPS += application/system/kernel_monitor
GLOBAL_DEFINES += SDRV_KERNEL_MONITOR=1
endif

ifeq ($(SUPPORT_CRYPTO_TEST),true)
MODULE_DEPS += application/test/crypto
endif

ifeq ($(SUPPORT_SLEEP_TEST), true)
MODULE_DEPS += application/test/sleep
endif

ifeq ($(SUPPORT_SSYSTEM_TEST), true)
MODULE_DEPS += application/test/ssystem
endif

ifeq ($(SUPPORT_PCIE_EP_MODE),true)
MODULE_DEPS += application/system/pcie
endif

ifeq ($(SUPPORT_PORT_HELPER),true)
MODULE_DEPS += application/tools/port_helper
endif

ifeq ($(SUPPORT_PIN_INFO),true)
MODULE_DEPS += application/tools/pin
endif

ifeq ($(SUPPORT_AM_TEST),true)
MODULE_DEPS += application/test/am_test
endif

ifeq ($(SUPPORT_TOOL_POWEROFF),true)
MODULE_DEPS += application/tools/poweroff
endif

ifeq ($(SUPPORT_AUDIO_TEST),true)
MODULE_DEPS += application/test/audio_test
endif

ifeq ($(SUPPORT_EEPROM_TEST),true)
MODULE_DEPS += application/test/eeprom
endif

ifeq ($(SUPPORT_PRINT_BUILDCONFIG),true)
MODULE_DEPS += application/test/buildconfig
endif

MODULE_DEPS += application/system/ospi_handover
