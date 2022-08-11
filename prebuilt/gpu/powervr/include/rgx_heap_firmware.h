/*************************************************************************/ /*!
@File
@Title          RGX FW heap definitions
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(RGX_HEAP_FIRMWARE_H)
#define RGX_HEAP_FIRMWARE_H

/* Start at 903GiB. Size of 32MB per OSID (see rgxheapconfig.h)
 * NOTE:
 *      The firmware heaps bases and sizes are defined here to
 *      simplify #include dependencies, see rgxheapconfig.h
 *      for the full RGX virtual address space layout.
 */

/*
 * The Config heap holds initialisation data shared between the
 * the driver and firmware (e.g. pointers to the KCCB and FWCCB).
 * The Main Firmware heap size is adjusted accordingly but most
 * of the map / unmap functions must take into consideration
 * the entire range (i.e. main and config heap).
 */

#if !defined(RGX_FW_HEAP_SHIFT)
#define RGX_FW_HEAP_SHIFT                            (25)
#endif

#define RGX_FIRMWARE_NUMBER_OF_FW_HEAPS              (2)
#define RGX_FIRMWARE_HEAP_SHIFT                      RGX_FW_HEAP_SHIFT
#define RGX_FIRMWARE_RAW_HEAP_BASE                   (0xE1C0000000ULL)
#define RGX_FIRMWARE_RAW_HEAP_SIZE                   (IMG_UINT32_C(1) << RGX_FIRMWARE_HEAP_SHIFT)

#if defined(SUPPORT_MIPS_64K_PAGE_SIZE)
#if defined(PDUMP)
/* PDUMP drivers allocate each structure from the Config heap in a different PMR.
 * Ensure the heap can hold 3 PMRs of 64KB */
#define RGX_FIRMWARE_CONFIG_HEAP_SIZE                (IMG_UINT32_C(0x30000)) /* 192KB */
#else
#define RGX_FIRMWARE_CONFIG_HEAP_SIZE                (IMG_UINT32_C(0x20000)) /* 128KB */
#endif
#else
/* regular 4KB page size system assumed */
#define RGX_FIRMWARE_CONFIG_HEAP_SIZE                (IMG_UINT32_C(0x10000)) /* 64KB */
#endif

#define RGX_FIRMWARE_META_MAIN_HEAP_SIZE             (RGX_FIRMWARE_RAW_HEAP_SIZE - RGX_FIRMWARE_CONFIG_HEAP_SIZE)
/*
 * MIPS FW needs space in the Main heap to map GPU memory.
 * This space is taken from the MAIN heap, to avoid creating a new heap.
 */
#define RGX_FIRMWARE_MIPS_GPU_MAP_RESERVED_SIZE_NORMAL       (IMG_UINT32_C(0x100000)) /* 1MB */
#define RGX_FIRMWARE_MIPS_GPU_MAP_RESERVED_SIZE_BRN65101     (IMG_UINT32_C(0x400000)) /* 4MB */

#define RGX_FIRMWARE_MIPS_MAIN_HEAP_SIZE_NORMAL      (RGX_FIRMWARE_RAW_HEAP_SIZE -  RGX_FIRMWARE_CONFIG_HEAP_SIZE - \
                                                      RGX_FIRMWARE_MIPS_GPU_MAP_RESERVED_SIZE_NORMAL)

#define RGX_FIRMWARE_MIPS_MAIN_HEAP_SIZE_BRN65101    (RGX_FIRMWARE_RAW_HEAP_SIZE -  RGX_FIRMWARE_CONFIG_HEAP_SIZE - \
                                                      RGX_FIRMWARE_MIPS_GPU_MAP_RESERVED_SIZE_BRN65101)

#if !defined(__KERNEL__)
#if defined(FIX_HW_BRN_65101)
#define RGX_FIRMWARE_MIPS_GPU_MAP_RESERVED_SIZE      RGX_FIRMWARE_MIPS_GPU_MAP_RESERVED_SIZE_BRN65101
#define RGX_FIRMWARE_MIPS_MAIN_HEAP_SIZE             RGX_FIRMWARE_MIPS_MAIN_HEAP_SIZE_BRN65101

#include "img_defs.h"
static_assert((RGX_FIRMWARE_RAW_HEAP_SIZE) >= IMG_UINT32_C(0x800000), "MIPS GPU map size cannot be increased due to BRN65101 with a small FW heap");

#else
#define RGX_FIRMWARE_MIPS_GPU_MAP_RESERVED_SIZE      RGX_FIRMWARE_MIPS_GPU_MAP_RESERVED_SIZE_NORMAL
#define RGX_FIRMWARE_MIPS_MAIN_HEAP_SIZE             RGX_FIRMWARE_MIPS_MAIN_HEAP_SIZE_NORMAL
#endif
#endif /* !defined(__KERNEL__) */

/* Host sub-heap order: MAIN + CONFIG */
#define RGX_FIRMWARE_HOST_MAIN_HEAP_BASE             RGX_FIRMWARE_RAW_HEAP_BASE
#define RGX_FIRMWARE_HOST_CONFIG_HEAP_BASE           (RGX_FIRMWARE_HOST_MAIN_HEAP_BASE + \
                                                      RGX_FIRMWARE_RAW_HEAP_SIZE - \
                                                      RGX_FIRMWARE_CONFIG_HEAP_SIZE)

/* Guest sub-heap order: CONFIG + MAIN */
#define RGX_FIRMWARE_GUEST_CONFIG_HEAP_BASE          RGX_FIRMWARE_RAW_HEAP_BASE
#define RGX_FIRMWARE_GUEST_MAIN_HEAP_BASE            (RGX_FIRMWARE_GUEST_CONFIG_HEAP_BASE + \
                                                      RGX_FIRMWARE_CONFIG_HEAP_SIZE)

/*
 * The maximum configurable size via RGX_FW_HEAP_SHIFT is 32MiB (1<<25) and
 * the minimum is 4MiB (1<<22); the default firmware heap size is set to
 * maximum 32MiB.
 */
#if defined(RGX_FW_HEAP_SHIFT) && (RGX_FW_HEAP_SHIFT < 22 || RGX_FW_HEAP_SHIFT > 25)
#error "RGX_FW_HEAP_SHIFT is outside valid range [22, 25]"
#endif

#endif /* RGX_HEAP_FIRMWARE_H */
