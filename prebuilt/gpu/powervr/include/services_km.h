/*************************************************************************/ /*!
@File
@Title          Services API Kernel mode Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Exported services API details
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef SERVICES_KM_H
#define SERVICES_KM_H

#if defined(SUPPORT_GPUVIRT_VALIDATION)
#include "virt_validation_defs.h"
#endif

/*! 4k page size definition */
#define PVRSRV_4K_PAGE_SIZE					4096UL      /*!< Size of a 4K Page */
#define PVRSRV_4K_PAGE_SIZE_ALIGNSHIFT		12          /*!< Amount to shift an address by so that
                                                          it is always page-aligned */
/*! 16k page size definition */
#define PVRSRV_16K_PAGE_SIZE					16384UL      /*!< Size of a 16K Page */
#define PVRSRV_16K_PAGE_SIZE_ALIGNSHIFT		14          /*!< Amount to shift an address by so that
                                                          it is always page-aligned */
/*! 64k page size definition */
#define PVRSRV_64K_PAGE_SIZE					65536UL      /*!< Size of a 64K Page */
#define PVRSRV_64K_PAGE_SIZE_ALIGNSHIFT		16          /*!< Amount to shift an address by so that
                                                          it is always page-aligned */
/*! 256k page size definition */
#define PVRSRV_256K_PAGE_SIZE					262144UL      /*!< Size of a 256K Page */
#define PVRSRV_256K_PAGE_SIZE_ALIGNSHIFT		18          /*!< Amount to shift an address by so that
                                                          it is always page-aligned */
/*! 1MB page size definition */
#define PVRSRV_1M_PAGE_SIZE					1048576UL      /*!< Size of a 1M Page */
#define PVRSRV_1M_PAGE_SIZE_ALIGNSHIFT		20          /*!< Amount to shift an address by so that
                                                          it is always page-aligned */
/*! 2MB page size definition */
#define PVRSRV_2M_PAGE_SIZE					2097152UL      /*!< Size of a 2M Page */
#define PVRSRV_2M_PAGE_SIZE_ALIGNSHIFT		21          /*!< Amount to shift an address by so that
                                                          it is always page-aligned */

#ifndef PVRSRV_DEV_CONNECTION_TYPEDEF
#define PVRSRV_DEV_CONNECTION_TYPEDEF
/*!
 * Forward declaration (look on connection.h)
 */
typedef struct PVRSRV_DEV_CONNECTION_TAG PVRSRV_DEV_CONNECTION;
#endif

/*!
	Flags for Services connection.
	Allows to define per-client policy for Services
*/
/*
 *   Use of the 32-bit connection flags mask
 *   ( X = taken/in use, - = available/unused )
 *
 *   31  27     20             6 4   0
 *    |   |      |             | |   |
 *    X---XXXXXXXX-------------XXX----
 */

#define SRV_NO_HWPERF_CLIENT_STREAM     (1U << 4)  /*!< Don't create HWPerf for this connection */
#define SRV_FLAGS_CLIENT_64BIT_COMPAT   (1U << 5)  /*!< This flags gets set if the client is 64 Bit compatible. */
#define SRV_FLAGS_CLIENT_SLR_DISABLED   (1U << 6)  /*!< This flag is set if the client does not want Sync Lockup Recovery (SLR) enabled. */
#define SRV_FLAGS_PDUMPCTRL             (1U << 31) /*!< PDump Ctrl client flag */

/*
 * Bits 20 - 27 are used to pass information needed for validation
 * of the GPU Virtualisation Validation mechanism. In particular:
 *
 * Bits:
 * [20 - 22]: OSid of the memory region that will be used for allocations
 * [23 - 25]: OSid that will be emitted by the Firmware for all memory accesses
 *            regarding that memory context.
 *      [26]: If the AXI Protection register will be set to secure for that OSid
 *      [27]: If the Emulator Wrapper Register checking for protection violation
 *            will be set to secure for that OSid
 */

#define VIRTVAL_FLAG_OSID_SHIFT        (20)
#define SRV_VIRTVAL_FLAG_OSID_MASK     (7U << VIRTVAL_FLAG_OSID_SHIFT)

#define VIRTVAL_FLAG_OSIDREG_SHIFT     (23)
#define SRV_VIRTVAL_FLAG_OSIDREG_MASK  (7U << VIRTVAL_FLAG_OSIDREG_SHIFT)

#define VIRTVAL_FLAG_AXIPREG_SHIFT     (26)
#define SRV_VIRTVAL_FLAG_AXIPREG_MASK  (1U << VIRTVAL_FLAG_AXIPREG_SHIFT)

#define VIRTVAL_FLAG_AXIPTD_SHIFT      (27)
#define SRV_VIRTVAL_FLAG_AXIPTD_MASK   (1U << VIRTVAL_FLAG_AXIPTD_SHIFT)


/* Size of pointer on a 64 bit machine */
#define	POINTER_SIZE_64BIT	(8)


/*
    Pdump flags which are accessible to Services clients
*/
#define PDUMP_NONE          0x00000000U /*<! No flags */

#define PDUMP_BLKDATA       0x10000000U /*<! This flag indicates block-mode PDump data to be recorded in
                                                          Block script stream in addition to Main script stream,
                                                          if capture mode is set to BLOCKED */

#define PDUMP_CONT          0x40000000U /*<! Output this entry always regardless of framed capture range,
                                                          used by client applications being dumped. */
#define PDUMP_PERSIST       0x80000000U /*<! Output this entry always regardless of app and range,
                                                          used by persistent resources created after
                                                          driver initialisation that must appear in
                                                          all PDump captures in that session. */

/* Valid range of values for pdump block length in Block mode of PDump */
#define PDUMP_BLOCKLEN_MIN          10
#define PDUMP_BLOCKLEN_MAX          1000

#define PDUMP_FRAME_MIN             0
#define PDUMP_FRAME_MAX             (IMG_UINT32_MAX - 1)
#define PDUMP_FRAME_UNSET           IMG_UINT32_MAX

/* Status of the device. */
typedef enum
{
	PVRSRV_DEVICE_STATUS_UNKNOWN,        /* status of the device is unknown */
	PVRSRV_DEVICE_STATUS_OK,             /* the device is operational */
	PVRSRV_DEVICE_STATUS_NOT_RESPONDING, /* the device is not responding */
	PVRSRV_DEVICE_STATUS_DEVICE_ERROR    /* the device is not operational */
} PVRSRV_DEVICE_STATUS;

#endif /* SERVICES_KM_H */
/**************************************************************************//**
End of file (services_km.h)
******************************************************************************/
