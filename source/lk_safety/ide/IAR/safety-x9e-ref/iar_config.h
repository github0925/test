#ifndef ___FreeRTOS_FreeRTOS_Demo_CORTEX_R5_SEMIDRIVE_X9_ide_IAR_safety_x9e_ref_iar_config_h_H
#define ___FreeRTOS_FreeRTOS_Demo_CORTEX_R5_SEMIDRIVE_X9_ide_IAR_safety_x9e_ref_iar_config_h_H
#define LK 1
#define __TRUSTY__ 1
#define BL_RTOS_PWM_RES RES_PWM_PWM4
#define BL_RTOS_PWM_GRP HAL_PWM_CHN_A_B_C_D_WORK
#define BL_RTOS_PWM_CH HAL_PWM_CHN_D
#define BL_RTOS_REVERSE_CONTROL 1
#define BL_RTOS_PWM_DEFAULT_DUTY 70
#define SAF_SYSTEM_CFG 1
#define IN_SAFETY_DOMAIN 1
#define WITH_NO_PHYS_RELOCATION 1
#define XIP 0
#define DISKD 6
#define OSPI_DIRECT_ACCESS 1
#define MEMBASE 0X3B000000
#define ROMBASE 0X4007800
#define ENABLE_AUDIO_MANAGER 1
#define PVT_IN_SAFETY_DOMAIN 1
#define GIC_400 1
#define ENABLE_RTC
#define WITH_SIMULATION_PLATFORM 1
#define CE_IN_SAFETY_DOMAIN 1
#define VDSP_ENABLE 1
#define SUPPORT_BOARDINFO 1
#define RPMSG_MASTER_DEVICE 1
#define ENABLE_PIN_DELTA_CONFIG 1
#define SDRV_TIMER 1
#define MJPEG_IRQ_NUM 23
#define SDRV_KERNEL_MONITOR 1
#define ENABLE_SERDES 1
#define SUPPORT_INPUT_SVC
#define SERDES_TP_X9M 1
#define SERDES_947_948 1
#define SUPPORT_DISP_SDDRV 1
#define SUPPORT_LVGL_GUI 1
#define SDRV_TIMER 1
#define CSI_BOARD_508 0
#define LOCKSTEP_SCR_ADDR 0XFC297000
#define LOCKSTEP_SCR_BIT 0
#define ARM_CPU_CORTEX_R5 1
#define ARM_WITH_CP15 1
#define ARM_WITH_MMU 0
#define ARM_ISA_ARMV7 1
#define ARM_ISA_ARMV7R 1
#define ARM_WITH_CACHE 1
#define ARM_WITH_VFP 1
#define ARM_WITH_NEON 1
#define ARM_WITH_THUMB2 1
#define ARM_WITH_MPU 1
#define ARM_VFP_D16 1
#define ARCH_DEFAULT_STACK_SIZE 4096
#define ARCH_ABT_STACK_SIZE 128
#define ARCH_IRQ_STACK_SIZE 512
#define ARCH_FIQ_STACK_SIZE 256
#define ARCH_SYS_STACK_SIZE 2048
#define ARCH_UND_STACK_SIZE 128
#define ARCH_SVC_STACK_SIZE 2048
#define KERNEL_ASPACE_BASE 0X40000000
#define KERNEL_ASPACE_SIZE 0XC0000000
#define USER_ASPACE_BASE 0X00001000
#define USER_ASPACE_SIZE 0X3FFFE000
#define KERNEL_BASE 0X3B000000
#define KERNEL_LOAD_OFFSET 0
#define SMP_MAX_CPUS 1
#define MEMBASE 0X3B000000
#define MEMSIZE 0X10000000
#define WAIT_PK_WITH_REGISTER_POLLING 0
#define RSA_PERFORMANCE_TEST 1
#define VPU_CODAJ12_DRV_MEM_SIZE 0X01400000
#define VMEM_PAGE_SIZE 0X100000
#define SUPPORT_SD_DISP 1
#define SUPPORT_LODE_PNG
#define SUPPORT_LODE_SJPG
#define LV_EX_CONF_INCLUDE_SIMPLE 1
#define LV_LVGL_H_INCLUDE_SIMPLE 1
#define CONFIG_IPCC_RPMSG 1
#define CONFIG_ENABLE_RPBUF 1
#define CONFIG_SUPPORT_DCF 1
#define CONFIG_SUPPORT_POSIX 1
#define CONFIG_RPMSG_SERVICE 1
#define BOARDINFO_EEPROM 1
#define ENABLE_SD_CLKGEN 1
#define ENABLE_SDRV_DIO
#define ENABLE_SD_DISP 1
#define ENABLE_SD_DMA 1
#define ENABLE_SD_G2DLITE 1
#define ENABLE_SD_I2C 1
#define ENABLE_ARM_GIC 1
#define ENABLE_SD_MBOX 1
#define MODULE_HELPER_PER_DISP 1
#define ENABLE_SD_PLL 1
#define ENABLE_SD_PMU 1
#define ENABLE_SDRV_PORT
#define ENABLE_SD_PWM 1
#define ENABLE_SD_RSTGEN 1
#define ENABLE_SD_SCR 1
#define ENABLE_SDRV_SPINOR
#define ENABLE_SD_TIMER 1
#define ENABLE_DW_UART 1
#define MEMDISK_BASE 0X80000000
#define MEMDISK_SIZE 0X4000000
#define SYS_CFG_VALID 1
#define LV_CONF_INCLUDE_SIMPLE 1
#define ENABLE_BOOTANIMATION
#define USE_ARGB888
#define CLUSTER_BS_SIZE 0X800000
#define ENABLE_CLUSTER
#define ENABLE_LVGL_CLUSTER 1
#define LV_EX_CONF_INCLUDE_SIMPLE 1
#define LV_LVGL_H_INCLUDE_SIMPLE 1
#define LVDS_HSD123_SERDES_1920X720_LCD
#define E_SERDES
#define ENABLE_SD_I2S 1
#define PROJECT_SERDES 1
#define PROJECT "SERDES"
#define TARGET_REFERENCE_X9 1
#define TARGET "REFERENCE_X9"
#define PLATFORM_KUNLUN 1
#define PLATFORM "KUNLUN"
#define ARCH_ARM 1
#define ARCH "ARM"
#define WITH_3RD_FATFS_SRC 1
#define WITH_3RD_LITTLEVGL 1
#define WITH_3RD_RPMSG_LITE 1
#define WITH_APPLICATION_EARLY_APP 1
#define WITH_APPLICATION_EARLY_APP_BOOTANIMATION 1
#define WITH_APPLICATION_EARLY_APP_CLUSTER 1
#define WITH_APPLICATION_EARLY_APP_MAIN 1
#define WITH_APPLICATION_SERVICES_UPDATE_MONITOR 1
#define WITH_APPLICATION_SYSTEM_BOOT_SS 1
#define WITH_APPLICATION_SYSTEM_KERNEL_MONITOR 1
#define WITH_APPLICATION_SYSTEM_OSPI_HANDOVER 1
#define WITH_APPLICATION_SYSTEM_PVT 1
#define WITH_APPLICATION_TEST_G2DLITE 1
#define WITH_APPLICATION_TOOLS_PORT_HELPER 1
#define WITH_APPLICATION_TOOLS_SEM_MONITOR 1
#define WITH_CHIPDEV_CLKGEN_SD_CLKGEN 1
#define WITH_CHIPDEV_CRYPTO_SILEX 1
#define WITH_CHIPDEV_CSI_SD_CSI 1
#define WITH_CHIPDEV_DIO_SD_DIO 1
#define WITH_CHIPDEV_DMA_DW_DMA1 1
#define WITH_CHIPDEV_FUNC_SAFETY 1
#define WITH_CHIPDEV_FUSE_CTRL 1
#define WITH_CHIPDEV_I2C_DW_I2C 1
#define WITH_CHIPDEV_I2S_CADENCE_I2S 1
#define WITH_CHIPDEV_I2S_CADENCE_I2S_2_0 1
#define WITH_CHIPDEV_INTERRUPT_ARM_GIC 1
#define WITH_CHIPDEV_MAILBOX_SD_MBOX 1
#define WITH_CHIPDEV_PLL 1
#define WITH_CHIPDEV_PMU_SD_PMU 1
#define WITH_CHIPDEV_PORT_SD_PORT 1
#define WITH_CHIPDEV_PWM_SD_PWM 1
#define WITH_CHIPDEV_RSTGEN_SD_RSTGEN 1
#define WITH_CHIPDEV_RTC 1
#define WITH_CHIPDEV_SCR 1
#define WITH_CHIPDEV_SPI_NOR 1
#define WITH_CHIPDEV_TIMER_SD_TIMER 1
#define WITH_CHIPDEV_UART_DW_UART 1
#define WITH_CHIPDEV_VPU_CODAJ12 1
#define WITH_EXDEV_AM_BOARD 1
#define WITH_EXDEV_AM_CODEC 1
#define WITH_EXDEV_AUDIO_CODEC_AK7738 1
#define WITH_EXDEV_AUDIO_CODEC_TAS6424 1
#define WITH_EXDEV_CAMERA 1
#define WITH_EXDEV_DISPLAY 1
#define WITH_EXDEV_GPIO 1
#define WITH_EXDEV_LCM 1
#define WITH_EXDEV_LCM_LVDS_HSD123_SERDES_1920X720_LCD_ 1
#define WITH_EXDEV_NORFLASH 1
#define WITH_EXDEV_TOUCH 1
#define WITH_FRAMEWORK_COMMUNICATION 1
#define WITH_FRAMEWORK_LIB_MEM_IMAGE 1
#define WITH_FRAMEWORK_PROTOCOL 1
#define WITH_FRAMEWORK_RPBUF 1
#define WITH_FRAMEWORK_SERVICE_AUDIO 1
#define WITH_FRAMEWORK_SERVICE_BACKLIGHT_BACKLIGHT_RTOS 1
#define WITH_FRAMEWORK_SERVICE_BASE 1
#define WITH_FRAMEWORK_SERVICE_CAMERA 1
#define WITH_FRAMEWORK_SERVICE_INPUT 1
#define WITH_FRAMEWORK_SERVICE_PROPERTY 1
#define WITH_FRAMEWORK_SERVICE_RPMSG 1
#define WITH_FRAMEWORK_SERVICE_SYS_DIAGNOSIS 1
#define WITH_FRAMEWORK_SERVICE_WORKER 1
#define WITH_FRAMEWORK_TEST_DCF 1
#define WITH_HAL_AUDIO_HAL_AUDIO_MANAGER 1
#define WITH_HAL_BOARDINFO 1
#define WITH_HAL_CLKGEN_HAL_SD_CLKGEN_HAL 1
#define WITH_HAL_CPU_HAL_SD_CPU_HAL 1
#define WITH_HAL_CRYPTO_HAL_ 1
#define WITH_HAL_CSI_HAL_SD_CSI_HAL 1
#define WITH_HAL_DIO_HAL_SD_DIO_HAL 1
#define WITH_HAL_DISP_HAL_SD_DISP_HAL 1
#define WITH_HAL_DMA_HAL_SD_DMA_HAL 1
#define WITH_HAL_FIREWALL_HAL_ 1
#define WITH_HAL_G2DLITE_HAL_SD_G2DLITE_HAL 1
#define WITH_HAL_GPIOIRQ_HAL 1
#define WITH_HAL_I2C_HAL_SD_I2C_HAL 1
#define WITH_HAL_I2S_HAL_SD_I2S_HAL 1
#define WITH_HAL_I2S_HAL_SD_I2S_HAL_2_0 1
#define WITH_HAL_INTERRUPT_HAL_ARM_GIC_HAL 1
#define WITH_HAL_MBOX_HAL_SD_MBOX_HAL 1
#define WITH_HAL_MMC 1
#define WITH_HAL_MODULE_HELPER_HAL 1
#define WITH_HAL_NET 1
#define WITH_HAL_PLL_ 1
#define WITH_HAL_PMU_HAL_SD_PMU_HAL 1
#define WITH_HAL_PORT_HAL_SD_PORT_HAL 1
#define WITH_HAL_PVT_HAL_ 1
#define WITH_HAL_PWM_HAL_SD_PWM_HAL 1
#define WITH_HAL_RES_ 1
#define WITH_HAL_RSTGEN_HAL_SD_RSTGEN_HAL 1
#define WITH_HAL_SCR_ 1
#define WITH_HAL_SPDIF_HAL_SD_SPDIF_HAL 1
#define WITH_HAL_SPI_HAL_SD_SPI_HAL 1
#define WITH_HAL_SPI_NOR 1
#define WITH_HAL_TIMER_HAL_SD_TIMER_HAL 1
#define WITH_HAL_UART_HAL_DW_UART_HAL 1
#define WITH_HAL_VPU_HAL_CODAJ12 1
#define WITH_HAL_WDG_HAL_SD_WDG_HAL 1
#define WITH_KERNEL 1
#define WITH_KERNEL_LK_WRAPPER 1
#define WITH_LIB_APPLOADER 1
#define WITH_LIB_BOOT 1
#define WITH_LIB_CBUF 1
#define WITH_LIB_CONSOLE 1
#define WITH_LIB_CONTAINER 1
#define WITH_LIB_DEBUG 1
#define WITH_LIB_DEBUGCOMMANDS 1
#define WITH_LIB_ELF 1
#define WITH_LIB_HEAP 1
#define WITH_LIB_IO 1
#define WITH_LIB_LIBC 1
#define WITH_LIB_REBOOT 1
#define WITH_LIB_RES_LOADER 1
#define WITH_LIB_SDCAST 1
#define WITH_LIB_SDRPC 1
#define WITH_LIB_SDUNITTEST 1
#define WITH_LIB_SHELL 1
#define WITH_LIB_STORAGE_DEVICE 1
#define WITH_LIB_SYSTEM_CONFIG 1
#define WITH_PLATFORM_KUNLUN_COMMON 1
#define LK_DEBUGLEVEL 0
#define GLOBAL_INCLUDES "_I__BUILD_SAFETY_X9E_REF__I__INCLUDE__I__INCLUDE_KERNEL__I__INCLUDE_SHARED__I__INCLUDE_SHARED_LK__I__INCLUDE_UAPI__I__INCLUDE_UAPI_UAPI__I3RD_FATFS_SRC_INCLUDE__I3RD_LITTLEVGL__I3RD_LITTLEVGL_INCLUDE__I3RD_LITTLEVGL_LVGL_RELEASE_V7__I3RD_RPMSG_LITE_INCLUDE__IAPPLICATION_EARLY_APP_BOOTANIMATION_INC__IAPPLICATION_EARLY_APP_BOOTANIMATION_INCLUDE__IAPPLICATION_EARLY_APP_CLUSTER__IAPPLICATION_EARLY_APP_CLUSTER__________INCLUDE_DEV___IAPPLICATION_EARLY_APP_CLUSTER__________LIB_SD_GRAPHICS_INC__IAPPLICATION_EARLY_APP_CLUSTER_INC__IAPPLICATION_EARLY_APP_CLUSTER_INCLUDE__IAPPLICATION_EARLY_APP_INCLUDE__IAPPLICATION_EARLY_APP_MAIN_INC__IAPPLICATION_EARLY_APP_MAIN_INCLUDE__IAPPLICATION_SERVICES_UPDATE_MONITOR_INCLUDE__IAPPLICATION_SYSTEM_BOOT_SS_INCLUDE__IAPPLICATION_SYSTEM_KERNEL_MONITOR_INCLUDE__IAPPLICATION_SYSTEM_OSPI_HANDOVER_INCLUDE__IAPPLICATION_SYSTEM_PVT_INCLUDE__IAPPLICATION_TEST_G2DLITE_INCLUDE__IAPPLICATION_TOOLS_PORT_HELPER_INCLUDE__IAPPLICATION_TOOLS_SEM_MONITOR_INCLUDE__IARCH_ARM_ARM_INCLUDE__IARCH_ARM_INCLUDE__ICHIPCFG_GENERATE_X9_ECO_CHIP___ICHIPCFG_GENERATE_X9_ECO_PROJECTS_SERDES___ICHIPCFG_GENERATE_X9_ECO_PROJECTS_SERDES__SAFETY__ICHIPCFG_GENERATE_X9_ECO_TARGETS_REFERENCE_X9___ICHIPCFG_GENERATE_X9_ECO_TARGETS_REFERENCE_X9__SAFETY__ICHIPDEV_CLKGEN_SD_CLKGEN_INC___ICHIPDEV_CLKGEN_SD_CLKGEN_INCLUDE__ICHIPDEV_CRYPTO_SILEX_INCLUDE__ICHIPDEV_CRYPTO_SILEX_INCLUDE___ICHIPDEV_CSI_SD_CSI_INCLUDE__ICHIPDEV_CSI_SD_CSI_INCLUDE___ICHIPDEV_DIO_SD_DIO_INC___ICHIPDEV_DIO_SD_DIO_INCLUDE__ICHIPDEV_DMA_DW_DMA1_INCLUDE__ICHIPDEV_DMA_DW_DMA1_INCLUDE___ICHIPDEV_FUNC_SAFETY_INC___ICHIPDEV_FUNC_SAFETY_INCLUDE__ICHIPDEV_FUSE_CTRL_INC___ICHIPDEV_FUSE_CTRL_INCLUDE__ICHIPDEV_I2C_DW_I2C_INCLUDE__ICHIPDEV_I2C_DW_I2C_INCLUDE___ICHIPDEV_I2S_CADENCE_I2S__________HAL_AUDIO_HAL_COMMON__ICHIPDEV_I2S_CADENCE_I2S_2_0_INC___ICHIPDEV_I2S_CADENCE_I2S_2_0_INCLUDE__ICHIPDEV_INTERRUPT_ARM_GIC_INC___ICHIPDEV_INTERRUPT_ARM_GIC_INCLUDE__ICHIPDEV_MAILBOX_SD_MBOX__ICHIPDEV_MAILBOX_SD_MBOX_INCLUDE__ICHIPDEV_PLL_INC__ICHIPDEV_PLL_INCLUDE__ICHIPDEV_PMU_SD_PMU_INC___ICHIPDEV_PMU_SD_PMU_INCLUDE__ICHIPDEV_PORT_SD_PORT_INC___ICHIPDEV_PORT_SD_PORT_INCLUDE__ICHIPDEV_PWM_SD_PWM_INC___ICHIPDEV_PWM_SD_PWM_INCLUDE__ICHIPDEV_RSTGEN_SD_RSTGEN_INC___ICHIPDEV_RSTGEN_SD_RSTGEN_INCLUDE__ICHIPDEV_RTC_INC___ICHIPDEV_RTC_INCLUDE__ICHIPDEV_SCR_INCLUDE__ICHIPDEV_SPI_NOR__ICHIPDEV_SPI_NOR_DEVICE__ICHIPDEV_SPI_NOR_HOST__ICHIPDEV_SPI_NOR_INCLUDE__ICHIPDEV_TIMER_SD_TIMER_INC___ICHIPDEV_TIMER_SD_TIMER_INCLUDE__ICHIPDEV_UART_DW_UART_INC___ICHIPDEV_UART_DW_UART_INCLUDE__ICHIPDEV_VPU_CODAJ12_INC__ICHIPDEV_VPU_CODAJ12_INC_JDI__ICHIPDEV_VPU_CODAJ12_INC_JPUAPI__ICHIPDEV_VPU_CODAJ12_INCLUDE__IEXDEV_AM_BOARD_____________HAL_AUDIO_HAL_COMMON__IEXDEV_AM_BOARD_INCLUDE__IEXDEV_AM_BOARD_REFA04_INC___IEXDEV_AM_CODEC_____________HAL_AUDIO_HAL_COMMON__IEXDEV_AM_CODEC_AK7738_INC___IEXDEV_AM_CODEC_INCLUDE__IEXDEV_AM_CODEC_TAS5404_INC___IEXDEV_AM_CODEC_TAS6424_INC___IEXDEV_AM_CODEC_TCA9539_INC___IEXDEV_AM_CODEC_XF6020_INC___IEXDEV_AUDIO_CODEC_AK7738_____________HAL_AUDIO_HAL_COMMON__IEXDEV_AUDIO_CODEC_AK7738_INC___IEXDEV_AUDIO_CODEC_AK7738_INCLUDE__IEXDEV_AUDIO_CODEC_TAS6424_____________HAL_AUDIO_HAL_COMMON__IEXDEV_AUDIO_CODEC_TAS6424_INC___IEXDEV_AUDIO_CODEC_TAS6424_INCLUDE__IEXDEV_CAMERA_INCLUDE__IEXDEV_DISPLAY_INCLUDE__IEXDEV_DISPLAY_LV_DRIVERS_DECODER_LODEPNG__IEXDEV_DISPLAY_LV_DRIVERS_DECODER_SJPG__IEXDEV_DISPLAY_LV_DRIVERS_GPU__IEXDEV_GPIO_INCLUDE__IEXDEV_LCM__IEXDEV_LCM_DS90UB9XX_INCLUDE__IEXDEV_LCM_INCLUDE__IEXDEV_LCM_LVDS_HSD123_SERDES_1920X720_LCD__INCLUDE__IEXDEV_LCM_SN65DSI85_INCLUDE__IEXDEV_NORFLASH__IEXDEV_NORFLASH_INCLUDE__IEXDEV_TOUCH_INCLUDE__IEXTERNAL_INCLUDE__IEXTERNAL_INCLUDE_SHARED__IEXTERNAL_INCLUDE_SHARED_LK__IEXTERNAL_INCLUDE_UAPI__IEXTERNAL_INCLUDE_UAPI_UAPI__IFRAMEWORK_COMMUNICATION_INCLUDE__IFRAMEWORK_LIB_MEM_IMAGE__IFRAMEWORK_LIB_MEM_IMAGE_INCLUDE__IFRAMEWORK_PROTOCOL_INCLUDE__IFRAMEWORK_PROTOCOL_INCLUDE___IFRAMEWORK_RPBUF_INCLUDE__IFRAMEWORK_RPBUF_INCLUDE___IFRAMEWORK_SERVICE_AUDIO_INCLUDE__IFRAMEWORK_SERVICE_BACKLIGHT_BACKLIGHT_RTOS_INCLUDE__IFRAMEWORK_SERVICE_BASE__IFRAMEWORK_SERVICE_BASE_INCLUDE__IFRAMEWORK_SERVICE_CAMERA_INCLUDE__IFRAMEWORK_SERVICE_INPUT_INCLUDE__IFRAMEWORK_SERVICE_PROPERTY__IFRAMEWORK_SERVICE_PROPERTY_INCLUDE__IFRAMEWORK_SERVICE_RPMSG_INCLUDE__IFRAMEWORK_SERVICE_SYS_DIAGNOSIS__IFRAMEWORK_SERVICE_SYS_DIAGNOSIS_INCLUDE__IFRAMEWORK_SERVICE_WORKER__IFRAMEWORK_SERVICE_WORKER_INCLUDE__IFRAMEWORK_TEST_DCF_INCLUDE__IHAL_AUDIO_HAL_AUDIO_MANAGER_INC___IHAL_AUDIO_HAL_AUDIO_MANAGER_INCLUDE__IHAL_AUDIO_HAL_AUDIO_MANAGER_LIB_INC__IHAL_BOARDINFO___IHAL_BOARDINFO_INCLUDE__IHAL_CLKGEN_HAL_SD_CLKGEN_HAL_INC___IHAL_CLKGEN_HAL_SD_CLKGEN_HAL_INCLUDE__IHAL_CPU_HAL_SD_CPU_HAL_INC__IHAL_CPU_HAL_SD_CPU_HAL_INCLUDE__IHAL_CRYPTO_HAL__INC___IHAL_CRYPTO_HAL__INCLUDE__IHAL_CSI_HAL_SD_CSI_HAL_INC___IHAL_CSI_HAL_SD_CSI_HAL_INCLUDE__IHAL_DIO_HAL_SD_DIO_HAL_INC___IHAL_DIO_HAL_SD_DIO_HAL_INCLUDE__IHAL_DISP_HAL_SD_DISP_HAL_INC__IHAL_DISP_HAL_SD_DISP_HAL_INCLUDE__IHAL_DISP_HAL_SD_DISP_HAL_LIB_INC__IHAL_DMA_HAL_SD_DMA_HAL_INC___IHAL_DMA_HAL_SD_DMA_HAL_INCLUDE__IHAL_FIREWALL_HAL__INC___IHAL_FIREWALL_HAL__INCLUDE__IHAL_G2DLITE_HAL_SD_G2DLITE_HAL_INC___IHAL_G2DLITE_HAL_SD_G2DLITE_HAL_INCLUDE__IHAL_G2DLITE_HAL_SD_G2DLITE_HAL_LIB_INC__IHAL_GPIOIRQ_HAL_INCLUDE__IHAL_GPIOIRQ_HAL_INCLUDE___IHAL_I2C_HAL_SD_I2C_HAL_INC___IHAL_I2C_HAL_SD_I2C_HAL_INCLUDE__IHAL_I2S_HAL_SD_I2S_HAL_2_0_INC___IHAL_I2S_HAL_SD_I2S_HAL_2_0_INCLUDE__IHAL_I2S_HAL_SD_I2S_HAL_INC___IHAL_INCLUDE___IHAL_INTERRUPT_HAL_ARM_GIC_HAL_INC___IHAL_INTERRUPT_HAL_ARM_GIC_HAL_INCLUDE__IHAL_MBOX_HAL_SD_MBOX_HAL_INC___IHAL_MBOX_HAL_SD_MBOX_HAL_INCLUDE__IHAL_MMC_INC___IHAL_MMC_INCLUDE__IHAL_MODULE_HELPER_HAL_INC__IHAL_MODULE_HELPER_HAL_INCLUDE__IHAL_NET_INCLUDE__IHAL_PLL__INC__IHAL_PLL__INCLUDE__IHAL_PMU_HAL_SD_PMU_HAL_INC___IHAL_PMU_HAL_SD_PMU_HAL_INCLUDE__IHAL_PORT_HAL_SD_PORT_HAL_INC___IHAL_PORT_HAL_SD_PORT_HAL_INCLUDE__IHAL_PVT_HAL__INC___IHAL_PVT_HAL__INCLUDE__IHAL_PWM_HAL_SD_PWM_HAL_INC___IHAL_PWM_HAL_SD_PWM_HAL_INCLUDE__IHAL_RES__INC__IHAL_RES__INCLUDE__IHAL_RSTGEN_HAL_SD_RSTGEN_HAL_INC___IHAL_RSTGEN_HAL_SD_RSTGEN_HAL_INCLUDE__IHAL_SCR__INC__IHAL_SCR__INCLUDE__IHAL_SPDIF_HAL_SD_SPDIF_HAL_INC___IHAL_SPDIF_HAL_SD_SPDIF_HAL_INCLUDE__IHAL_SPI_HAL_SD_SPI_HAL_INC___IHAL_SPI_HAL_SD_SPI_HAL_INCLUDE__IHAL_SPI_NOR_INC___IHAL_SPI_NOR_INCLUDE__IHAL_TIMER_HAL_SD_TIMER_HAL_INC___IHAL_TIMER_HAL_SD_TIMER_HAL_INCLUDE__IHAL_UART_HAL_DW_UART_HAL_INC___IHAL_UART_HAL_DW_UART_HAL_INCLUDE__IHAL_VPU_HAL_CODAJ12_INC___IHAL_VPU_HAL_CODAJ12_INCLUDE__IHAL_WDG_HAL_SD_WDG_HAL_INC___IHAL_WDG_HAL_SD_WDG_HAL_INCLUDE__IINCLUDE_INCLUDE__IINCLUDE_INCLUDE_SHARED__IINCLUDE_INCLUDE_SHARED_LK__IINCLUDE_INCLUDE_UAPI__IINCLUDE_INCLUDE_UAPI_UAPI__IKERNEL_INCLUDE__IKERNEL_LK_WRAPPER_INCLUDE__ILIB_APPLOADER_INCLUDE__ILIB_BOOT_INCLUDE__ILIB_CBUF_INCLUDE__ILIB_CBUF_INCLUDE_LIB__ILIB_CONSOLE_INCLUDE__ILIB_CONTAINER_INC__ILIB_CONTAINER_INCLUDE__ILIB_DEBUG_INCLUDE__ILIB_DEBUGCOMMANDS_INCLUDE__ILIB_ELF_INCLUDE__ILIB_HEAP___ILIB_HEAP_INCLUDE__ILIB_IO_INCLUDE__ILIB_LIBC_INCLUDE__ILIB_REBOOT__ILIB_REBOOT_INCLUDE__ILIB_RES_LOADER__ILIB_RES_LOADER_INCLUDE__ILIB_SDCAST_INCLUDE__ILIB_SDRPC__ILIB_SDRPC_INCLUDE__ILIB_SDUNITTEST_INCLUDE__ILIB_SHELL_INCLUDE__ILIB_STORAGE_DEVICE_INCLUDE__ILIB_SYSTEM_CONFIG___ILIB_SYSTEM_CONFIG_INCLUDE__IPLATFORM_KUNLUN__IPLATFORM_KUNLUN_______CHIPCFG_GENERATE_X9_ECO___IPLATFORM_KUNLUN_COMMON__IPLATFORM_KUNLUN_COMMON_INCLUDE__IPLATFORM_KUNLUN_SAFETY___IPLATFORM_KUNLUN_SAFETY_INCLUDE__ITARGET_REFERENCE_X9_SAFETY___ITARGET_REFERENCE_X9_SAFETY_INCLUDE__ITOP_INCLUDE__ILIB_SDCAST_INCLUDE"
#define GLOBAL_COMPILEFLAGS "_G__FINLINE__INCLUDE___BUILD_SAFETY_X9E_REF_CONFIG_H__WERROR__WALL__WSIGN_COMPARE__WNO_MULTICHAR__WNO_UNUSED_FUNCTION__WNO_UNUSED_LABEL__WNO_TAUTOLOGICAL_COMPARE__FNO_SHORT_ENUMS__FNO_COMMON__WNO_NONNULL_COMPARE__FFUNCTION_SECTIONS__FDATA_SECTIONS"
#define GLOBAL_OPTFLAGS "_O2"
#define GLOBAL_CFLAGS "__STD C11__WSTRICT_PROTOTYPES__WWRITE_STRINGS"
#define GLOBAL_CPPFLAGS "__STD CPP11__FNO_EXCEPTIONS__FNO_RTTI__FNO_THREADSAFE_STATICS"
#define GLOBAL_ASMFLAGS "_DASSEMBLY"
#define GLOBAL_LDFLAGS "__LINCLUDE__L___LEXTERNAL___ENTRY 0X3B000000__Z_MAX_PAGE_SIZE 4096___GC_SECTIONS"
#define ARCH_COMPILEFLAGS "__MCPU CORTEX_R5__MFPU VFPV3_D16__MFLOAT_ABI HARD"
#define ARCH_CFLAGS ""
#define ARCH_CPPFLAGS ""
#define ARCH_ASMFLAGS ""
#endif