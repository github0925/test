#ifndef __build_dil2_d9_ref_config_h_H
#define __build_dil2_d9_ref_config_h_H
#define LK 1
#define __TRUSTY__ 1
#define DISABLE_GPU 0
#define DISABLE_VPU 0
#define ENABLE_SERDES 1
#define SAF_SYSTEM_CFG 1
#define IN_SAFETY_DOMAIN 1
#define WITH_NO_PHYS_RELOCATION 0
#define XIP 0
#define OSPI_DIRECT_ACCESS 0
#define SUPPORT_DISP_SDDRV 1
#define SUPPORT_NEXT_OS 1
#define SUPPORT_DIL2_INIT 1
#define MODULE_HELPER_CKGEN_DISP 1
#define MODULE_HELPER_CKGEN_SOC 1
#define MODULE_HELPER_RSTGEN_MODULE 1
#define SUPPORT_FAST_BOOT 1
#define GIC_400 1
#define ENABLE_RTC
#define ENABLE_ETHERNET1 1
#define WITH_SIMULATION_PLATFORM 1
#define CE_IN_SAFETY_DOMAIN 1
#define SUPPORT_FAST_BOOT 1
#define SDRV_TIMER 1
#define MJPEG_IRQ_NUM 23
#define SUPPORT_DISP_SDDRV 1
#define SDRV_TIMER 1
#define CSI_BOARD_507_A02 1
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
#define KERNEL_BASE 0X00100000
#define KERNEL_LOAD_OFFSET 0
#define SMP_MAX_CPUS 1
#define MEMBASE 0X00100000
#define MEMSIZE 0X00080000
#define WAIT_PK_WITH_REGISTER_POLLING 0
#define RSA_PERFORMANCE_TEST 1
#define VPU_CODAJ12_DRV_MEM_SIZE 0X01400000
#define VMEM_PAGE_SIZE 0X100000
#define ENABLE_SD_CLKGEN 1
#define ENABLE_SDRV_DIO
#define ENABLE_SD_DISP 1
#define ENABLE_SD_I2C 1
#define ENABLE_ARM_GIC 1
#define ENABLE_SD_MBOX 1
#define MODULE_HELPER_PER_DISP 1
#define ENABLE_SD_PLL 1
#define ENABLE_SD_PMU 1
#define ENABLE_SDRV_PORT
#define ENABLE_SD_RSTGEN 1
#define ENABLE_SD_SCR 1
#define ENABLE_SD_TIMER 1
#define ENABLE_DW_UART 1
#define SYS_CFG_VALID 1
#define MIPI_LT9611_TO_HDMI_1920X1080_LCD
#define V_DEFAULT
#define PROJECT_D9_REF 1
#define PROJECT "D9_REF"
#define TARGET_REFERENCE_D9 1
#define TARGET "REFERENCE_D9"
#define PLATFORM_KUNLUN 1
#define PLATFORM "KUNLUN"
#define ARCH_ARM 1
#define ARCH "ARM"
#define WITH_APPLICATION_SYSTEM_OSPI_HANDOVER 1
#define WITH_APPLICATION_SYSTEM_SOC_INIT 1
#define WITH_APPLICATION_TEST_DISP 1
#define WITH_APPLICATION_TOOLS_PORT_HELPER 1
#define WITH_CHIPDEV_CLKGEN_SD_CLKGEN 1
#define WITH_CHIPDEV_CRYPTO_SILEX 1
#define WITH_CHIPDEV_DIO_SD_DIO 1
#define WITH_CHIPDEV_DISP_SD_DISP 1
#define WITH_CHIPDEV_FUNC_SAFETY 1
#define WITH_CHIPDEV_I2C_DW_I2C 1
#define WITH_CHIPDEV_INTERRUPT_ARM_GIC 1
#define WITH_CHIPDEV_MAILBOX_SD_MBOX 1
#define WITH_CHIPDEV_PLL 1
#define WITH_CHIPDEV_PMU_SD_PMU 1
#define WITH_CHIPDEV_PORT_SD_PORT 1
#define WITH_CHIPDEV_RSTGEN_SD_RSTGEN 1
#define WITH_CHIPDEV_RTC 1
#define WITH_CHIPDEV_SCR 1
#define WITH_CHIPDEV_TIMER_SD_TIMER 1
#define WITH_CHIPDEV_UART_DW_UART 1
#define WITH_CHIPDEV_VPU_CODAJ12 1
#define WITH_EXDEV_DISPLAY 1
#define WITH_EXDEV_GPIO 1
#define WITH_EXDEV_LCM 1
#define WITH_EXDEV_LCM_MIPI_LT9611_TO_HDMI_1920X1080_LCD_ 1
#define WITH_FRAMEWORK_AUDIO_CORE 1
#define WITH_FRAMEWORK_LIB_MEM_IMAGE 1
#define WITH_FRAMEWORK_LIB_SYSTEM 1
#define WITH_FRAMEWORK_SERVICE_BASE 1
#define WITH_FRAMEWORK_SERVICE_SYS_DIAGNOSIS 1
#define WITH_FRAMEWORK_SERVICE_WORKER 1
#define WITH_HAL_CLKGEN_HAL_SD_CLKGEN_HAL 1
#define WITH_HAL_CPU_HAL_SD_CPU_HAL 1
#define WITH_HAL_CRYPTO_HAL_ 1
#define WITH_HAL_DIO_HAL_SD_DIO_HAL 1
#define WITH_HAL_DISP_HAL_SD_DISP_HAL 1
#define WITH_HAL_DMA_HAL_SD_DMA_HAL 1
#define WITH_HAL_FIREWALL_HAL_ 1
#define WITH_HAL_GPIOIRQ_HAL 1
#define WITH_HAL_I2C_HAL_SD_I2C_HAL 1
#define WITH_HAL_I2S_HAL_SD_I2S_HAL 1
#define WITH_HAL_I2S_HAL_SD_I2S_HAL_1_0 1
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
#define WITH_LIB_DEBUG 1
#define WITH_LIB_HEAP 1
#define WITH_LIB_IO 1
#define WITH_LIB_LIBC 1
#define WITH_LIB_REBOOT 1
#define WITH_LIB_SYSTEM_CONFIG 1
#define WITH_PLATFORM_KUNLUN_COMMON 1
#define LK_DEBUGLEVEL 0
#define GLOBAL_INCLUDES "_I__BUILD_DIL2_D9_REF__I__INCLUDE__I__INCLUDE_KERNEL__I__INCLUDE_SHARED__I__INCLUDE_SHARED_LK__I__INCLUDE_UAPI__I__INCLUDE_UAPI_UAPI__IAPPLICATION_SYSTEM_OSPI_HANDOVER_INCLUDE__IAPPLICATION_SYSTEM_SOC_INIT_INCLUDE__IAPPLICATION_TEST_DISP_INCLUDE__IAPPLICATION_TOOLS_PORT_HELPER_INCLUDE__IARCH_ARM_ARM_INCLUDE__IARCH_ARM_INCLUDE__ICHIPCFG_GENERATE_D9_CHIP___ICHIPCFG_GENERATE_D9_PROJECTS_D9_REF___ICHIPCFG_GENERATE_D9_PROJECTS_D9_REF__SAFETY__ICHIPCFG_GENERATE_D9_TARGETS_REFERENCE_D9___ICHIPCFG_GENERATE_D9_TARGETS_REFERENCE_D9__SAFETY__ICHIPDEV_CLKGEN_SD_CLKGEN_INC___ICHIPDEV_CLKGEN_SD_CLKGEN_INCLUDE__ICHIPDEV_CRYPTO_SILEX_INCLUDE__ICHIPDEV_CRYPTO_SILEX_INCLUDE___ICHIPDEV_DIO_SD_DIO_INC___ICHIPDEV_DIO_SD_DIO_INCLUDE__ICHIPDEV_DISP_SD_DISP_DSI_INC__ICHIPDEV_DISP_SD_DISP_INC__ICHIPDEV_DISP_SD_DISP_INCLUDE__ICHIPDEV_FUNC_SAFETY_INC___ICHIPDEV_FUNC_SAFETY_INCLUDE__ICHIPDEV_I2C_DW_I2C_INCLUDE__ICHIPDEV_I2C_DW_I2C_INCLUDE___ICHIPDEV_INTERRUPT_ARM_GIC_INC___ICHIPDEV_INTERRUPT_ARM_GIC_INCLUDE__ICHIPDEV_MAILBOX_SD_MBOX__ICHIPDEV_MAILBOX_SD_MBOX_INCLUDE__ICHIPDEV_PLL_INC__ICHIPDEV_PLL_INCLUDE__ICHIPDEV_PMU_SD_PMU_INC___ICHIPDEV_PMU_SD_PMU_INCLUDE__ICHIPDEV_PORT_SD_PORT_INC___ICHIPDEV_PORT_SD_PORT_INCLUDE__ICHIPDEV_RSTGEN_SD_RSTGEN_INC___ICHIPDEV_RSTGEN_SD_RSTGEN_INCLUDE__ICHIPDEV_RTC_INC___ICHIPDEV_RTC_INCLUDE__ICHIPDEV_SCR_INCLUDE__ICHIPDEV_TIMER_SD_TIMER_INC___ICHIPDEV_TIMER_SD_TIMER_INCLUDE__ICHIPDEV_UART_DW_UART_INC___ICHIPDEV_UART_DW_UART_INCLUDE__ICHIPDEV_VPU_CODAJ12_INC__ICHIPDEV_VPU_CODAJ12_INC_JDI__ICHIPDEV_VPU_CODAJ12_INC_JPUAPI__ICHIPDEV_VPU_CODAJ12_INCLUDE__IEXDEV_DISPLAY_INCLUDE__IEXDEV_GPIO_INCLUDE__IEXDEV_LCM__IEXDEV_LCM_DS90UB9XX_INCLUDE__IEXDEV_LCM_INCLUDE__IEXDEV_LCM_LT9611_INCLUDE__IEXDEV_LCM_MIPI_LT9611_TO_HDMI_1920X1080_LCD__INCLUDE__IEXDEV_LCM_SN65DSI85_INCLUDE__IEXTERNAL_INCLUDE__IEXTERNAL_INCLUDE_SHARED__IEXTERNAL_INCLUDE_SHARED_LK__IEXTERNAL_INCLUDE_UAPI__IEXTERNAL_INCLUDE_UAPI_UAPI__IFRAMEWORK_AUDIO_CORE_INC___IFRAMEWORK_AUDIO_CORE_INCLUDE__IFRAMEWORK_LIB_MEM_IMAGE__IFRAMEWORK_LIB_MEM_IMAGE_INCLUDE__IFRAMEWORK_LIB_SYSTEM__IFRAMEWORK_LIB_SYSTEM_INCLUDE__IFRAMEWORK_SERVICE_BASE__IFRAMEWORK_SERVICE_BASE_INCLUDE__IFRAMEWORK_SERVICE_SYS_DIAGNOSIS__IFRAMEWORK_SERVICE_SYS_DIAGNOSIS_INCLUDE__IFRAMEWORK_SERVICE_WORKER__IFRAMEWORK_SERVICE_WORKER_INCLUDE__IHAL_CLKGEN_HAL_SD_CLKGEN_HAL_INC___IHAL_CLKGEN_HAL_SD_CLKGEN_HAL_INCLUDE__IHAL_CPU_HAL_SD_CPU_HAL_INC__IHAL_CPU_HAL_SD_CPU_HAL_INCLUDE__IHAL_CRYPTO_HAL__INC___IHAL_CRYPTO_HAL__INCLUDE__IHAL_DIO_HAL_SD_DIO_HAL_INC___IHAL_DIO_HAL_SD_DIO_HAL_INCLUDE__IHAL_DISP_HAL_SD_DISP_HAL_INC__IHAL_DISP_HAL_SD_DISP_HAL_INCLUDE__IHAL_DMA_HAL_SD_DMA_HAL_INC___IHAL_DMA_HAL_SD_DMA_HAL_INCLUDE__IHAL_FIREWALL_HAL__INC___IHAL_FIREWALL_HAL__INCLUDE__IHAL_GPIOIRQ_HAL_INCLUDE__IHAL_GPIOIRQ_HAL_INCLUDE___IHAL_I2C_HAL_SD_I2C_HAL_INC___IHAL_I2C_HAL_SD_I2C_HAL_INCLUDE__IHAL_I2S_HAL_SD_I2S_HAL_1_0_INC___IHAL_I2S_HAL_SD_I2S_HAL_1_0_INCLUDE__IHAL_I2S_HAL_SD_I2S_HAL_INC___IHAL_INCLUDE___IHAL_INTERRUPT_HAL_ARM_GIC_HAL_INC___IHAL_INTERRUPT_HAL_ARM_GIC_HAL_INCLUDE__IHAL_MBOX_HAL_SD_MBOX_HAL_INC___IHAL_MBOX_HAL_SD_MBOX_HAL_INCLUDE__IHAL_MMC_INC___IHAL_MMC_INCLUDE__IHAL_MODULE_HELPER_HAL_INC__IHAL_MODULE_HELPER_HAL_INCLUDE__IHAL_NET_INCLUDE__IHAL_PLL__INC__IHAL_PLL__INCLUDE__IHAL_PMU_HAL_SD_PMU_HAL_INC___IHAL_PMU_HAL_SD_PMU_HAL_INCLUDE__IHAL_PORT_HAL_SD_PORT_HAL_INC___IHAL_PORT_HAL_SD_PORT_HAL_INCLUDE__IHAL_PVT_HAL__INC___IHAL_PVT_HAL__INCLUDE__IHAL_PWM_HAL_SD_PWM_HAL_INC___IHAL_PWM_HAL_SD_PWM_HAL_INCLUDE__IHAL_RES__INC__IHAL_RES__INCLUDE__IHAL_RSTGEN_HAL_SD_RSTGEN_HAL_INC___IHAL_RSTGEN_HAL_SD_RSTGEN_HAL_INCLUDE__IHAL_SCR__INC__IHAL_SCR__INCLUDE__IHAL_SPDIF_HAL_SD_SPDIF_HAL_INC___IHAL_SPDIF_HAL_SD_SPDIF_HAL_INCLUDE__IHAL_SPI_HAL_SD_SPI_HAL_INC___IHAL_SPI_HAL_SD_SPI_HAL_INCLUDE__IHAL_SPI_NOR_INC___IHAL_SPI_NOR_INCLUDE__IHAL_TIMER_HAL_SD_TIMER_HAL_INC___IHAL_TIMER_HAL_SD_TIMER_HAL_INCLUDE__IHAL_UART_HAL_DW_UART_HAL_INC___IHAL_UART_HAL_DW_UART_HAL_INCLUDE__IHAL_VPU_HAL_CODAJ12_INC___IHAL_VPU_HAL_CODAJ12_INCLUDE__IHAL_WDG_HAL_SD_WDG_HAL_INC___IHAL_WDG_HAL_SD_WDG_HAL_INCLUDE__IINCLUDE_INCLUDE__IINCLUDE_INCLUDE_SHARED__IINCLUDE_INCLUDE_SHARED_LK__IINCLUDE_INCLUDE_UAPI__IINCLUDE_INCLUDE_UAPI_UAPI__IKERNEL_INCLUDE__IKERNEL_LK_WRAPPER_INCLUDE__ILIB_APPLOADER_INCLUDE__ILIB_BOOT_INCLUDE__ILIB_CBUF_INCLUDE__ILIB_CBUF_INCLUDE_LIB__ILIB_DEBUG_INCLUDE__ILIB_HEAP___ILIB_HEAP_INCLUDE__ILIB_IO_INCLUDE__ILIB_LIBC_INCLUDE__ILIB_REBOOT__ILIB_REBOOT_INCLUDE__ILIB_SYSTEM_CONFIG___ILIB_SYSTEM_CONFIG_INCLUDE__IPLATFORM_KUNLUN__IPLATFORM_KUNLUN_______CHIPCFG_GENERATE_D9___IPLATFORM_KUNLUN_COMMON__IPLATFORM_KUNLUN_COMMON_INCLUDE__IPLATFORM_KUNLUN_SAFETY___IPLATFORM_KUNLUN_SAFETY_INCLUDE__ITARGET_REFERENCE_D9_SAFETY___ITARGET_REFERENCE_D9_SAFETY_INCLUDE__ITOP_INCLUDE__ILIB_BOOT_INCLUDE"
#define GLOBAL_COMPILEFLAGS "_G__FINLINE__INCLUDE___BUILD_DIL2_D9_REF_CONFIG_H__WERROR__WALL__WSIGN_COMPARE__WNO_MULTICHAR__WNO_UNUSED_FUNCTION__WNO_UNUSED_LABEL__WNO_TAUTOLOGICAL_COMPARE__FNO_SHORT_ENUMS__FNO_COMMON__WNO_NONNULL_COMPARE__FFUNCTION_SECTIONS__FDATA_SECTIONS"
#define GLOBAL_OPTFLAGS "_O2"
#define GLOBAL_CFLAGS "__STD C11__WSTRICT_PROTOTYPES__WWRITE_STRINGS"
#define GLOBAL_CPPFLAGS "__STD CPP11__FNO_EXCEPTIONS__FNO_RTTI__FNO_THREADSAFE_STATICS"
#define GLOBAL_ASMFLAGS "_DASSEMBLY"
#define GLOBAL_LDFLAGS "__LINCLUDE__L___LEXTERNAL___ENTRY 0X00100000__Z_MAX_PAGE_SIZE 4096___GC_SECTIONS"
#define ARCH_COMPILEFLAGS "__MCPU CORTEX_R5__MFPU VFPV3_D16__MFLOAT_ABI HARD"
#define ARCH_CFLAGS ""
#define ARCH_CPPFLAGS ""
#define ARCH_ASMFLAGS ""
#endif
