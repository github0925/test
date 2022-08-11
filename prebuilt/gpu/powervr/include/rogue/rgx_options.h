/*************************************************************************/ /*!
@File
@Title          RGX build options
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

/* Each build option listed here is packed into a dword which provides up to
 *  log2(RGX_BUILD_OPTIONS_MASK_KM + 1) flags for KM and
 *  (32 - log2(RGX_BUILD_OPTIONS_MASK_KM + 1)) flags for UM.
 * The corresponding bit is set if the build option was enabled at compile
 * time.
 *
 * In order to extract the enabled build flags the INTERNAL_TEST switch should
 * be enabled in a client program which includes this header. Then the client
 * can test specific build flags by reading the bit value at
 *  ##OPTIONNAME##_SET_OFFSET
 * in RGX_BUILD_OPTIONS_KM or RGX_BUILD_OPTIONS.
 *
 * IMPORTANT: add new options to unused bits or define a new dword
 * (e.g. RGX_BUILD_OPTIONS_KM2 or RGX_BUILD_OPTIONS2) so that the bitfield
 * remains backwards compatible.
 */

#ifndef RGX_OPTIONS_H
#define RGX_OPTIONS_H

#define RGX_BUILD_OPTIONS_MASK_KM 0x0000FFFFUL

#if defined(NO_HARDWARE) || defined(INTERNAL_TEST)
	#define NO_HARDWARE_SET_OFFSET	OPTIONS_BIT0
	#define OPTIONS_BIT0		(0x1UL << 0)
	#if OPTIONS_BIT0 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT0		0x0UL
#endif /* NO_HARDWARE */


#if defined(PDUMP) || defined(INTERNAL_TEST)
	#define PDUMP_SET_OFFSET	OPTIONS_BIT1
	#define OPTIONS_BIT1		(0x1UL << 1)
	#if OPTIONS_BIT1 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT1		0x0UL
#endif /* PDUMP */


#if defined(INTERNAL_TEST)
	#define UNUSED_SET_OFFSET	OPTIONS_BIT2
	#define OPTIONS_BIT2		(0x1UL << 2)
	#if OPTIONS_BIT2 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT2		0x0UL
#endif

/* No longer used */
#if defined(INTERNAL_TEST)
	#define OPTIONS_BIT3		(0x1UL << 3)
	#if OPTIONS_BIT3 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT3		0x0UL
#endif


#if defined(SUPPORT_RGX) || defined(INTERNAL_TEST)
	#define SUPPORT_RGX_SET_OFFSET	OPTIONS_BIT4
	#define OPTIONS_BIT4		(0x1UL << 4)
	#if OPTIONS_BIT4 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT4		0x0UL
#endif /* SUPPORT_RGX */


#if defined(SUPPORT_SECURE_EXPORT) || defined(INTERNAL_TEST)
	#define SUPPORT_SECURE_EXPORT_SET_OFFSET	OPTIONS_BIT5
	#define OPTIONS_BIT5		(0x1UL << 5)
	#if OPTIONS_BIT5 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT5		0x0UL
#endif /* SUPPORT_SECURE_EXPORT */


#if defined(SUPPORT_INSECURE_EXPORT) || defined(INTERNAL_TEST)
	#define SUPPORT_INSECURE_EXPORT_SET_OFFSET	OPTIONS_BIT6
	#define OPTIONS_BIT6		(0x1UL << 6)
	#if OPTIONS_BIT6 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT6		0x0UL
#endif /* SUPPORT_INSECURE_EXPORT */


#if defined(SUPPORT_VFP) || defined(INTERNAL_TEST)
	#define SUPPORT_VFP_SET_OFFSET	OPTIONS_BIT7
	#define OPTIONS_BIT7		(0x1UL << 7)
	#if OPTIONS_BIT7 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT7		0x0UL
#endif /* SUPPORT_VFP */

#if defined(SUPPORT_WORKLOAD_ESTIMATION) || defined(INTERNAL_TEST)
	#define SUPPORT_WORKLOAD_ESTIMATION_OFFSET	OPTIONS_BIT8
	#define OPTIONS_BIT8		(0x1UL << 8)
	#if OPTIONS_BIT8 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT8		0x0UL
#endif /* SUPPORT_WORKLOAD_ESTIMATION */
#define OPTIONS_WORKLOAD_ESTIMATION_MASK	(0x1UL << 8)

#if defined(SUPPORT_PDVFS) || defined(INTERNAL_TEST)
	#define SUPPORT_PDVFS_OFFSET	OPTIONS_BIT9
	#define OPTIONS_BIT9		(0x1UL << 9)
	#if OPTIONS_BIT9 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT9		0x0UL
#endif /* SUPPORT_PDVFS */
#define OPTIONS_PDVFS_MASK	(0x1UL << 9)

#if defined(DEBUG) || defined(INTERNAL_TEST)
	#define DEBUG_SET_OFFSET	OPTIONS_BIT10
	#define OPTIONS_BIT10		(0x1UL << 10)
	#if OPTIONS_BIT10 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT10		0x0UL
#endif /* DEBUG */
/* The bit position of this should be the same as DEBUG_SET_OFFSET option
 * when defined.
 */
#define OPTIONS_DEBUG_MASK	(0x1UL << 10)

#if defined(SUPPORT_BUFFER_SYNC) || defined(INTERNAL_TEST)
	#define SUPPORT_BUFFER_SYNC_SET_OFFSET	OPTIONS_BIT11
	#define OPTIONS_BIT11		(0x1UL << 11)
	#if OPTIONS_BIT11 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT11		0x0UL
#endif /* SUPPORT_BUFFER_SYNC */

#if defined(RGX_FW_IRQ_OS_COUNTERS) || defined(INTERNAL_TEST)
	#define SUPPORT_FW_IRQ_REG_COUNTERS		OPTIONS_BIT12
	#define OPTIONS_BIT12		(0x1UL << 12)
	#if OPTIONS_BIT12 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT12		0x0UL
#endif /* RGX_FW_IRQ_OS_COUNTERS */

#if defined(SUPPORT_AUTOVZ)
	#define SUPPORT_AUTOVZ_OFFSET OPTIONS_BIT14
	#define OPTIONS_BIT14     (0x1UL << 14)
	#if OPTIONS_BIT14 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT14     0x0UL
#endif

#if defined(SUPPORT_AUTOVZ_HW_REGS)
	#define SUPPORT_AUTOVZ_HW_REGS_OFFSET OPTIONS_BIT15
	#define OPTIONS_BIT15     (0x1UL << 15)
	#if OPTIONS_BIT15 > RGX_BUILD_OPTIONS_MASK_KM
	#error "Bit exceeds reserved range"
	#endif
#else
	#define OPTIONS_BIT15     0x0UL
#endif


#define RGX_BUILD_OPTIONS_KM	\
	(OPTIONS_BIT0  |\
	 OPTIONS_BIT1  |\
	 OPTIONS_BIT2  |\
	 OPTIONS_BIT3  |\
	 OPTIONS_BIT4  |\
	 OPTIONS_BIT6  |\
	 OPTIONS_BIT7  |\
	 OPTIONS_BIT8  |\
	 OPTIONS_BIT9  |\
	 OPTIONS_BIT10 |\
	 OPTIONS_BIT11 |\
	 OPTIONS_BIT12 |\
	 OPTIONS_BIT14 |\
	 OPTIONS_BIT15)

#define RGX_BUILD_OPTIONS_MASK_FW \
	(RGX_BUILD_OPTIONS_MASK_KM & \
	 ~OPTIONS_BIT11)

#define OPTIONS_BIT31		(0x1UL << 31)
#if OPTIONS_BIT31 <= RGX_BUILD_OPTIONS_MASK_KM
#error "Bit exceeds reserved range"
#endif
#define SUPPORT_PERCONTEXT_FREELIST_SET_OFFSET	OPTIONS_BIT31

#define RGX_BUILD_OPTIONS (RGX_BUILD_OPTIONS_KM | OPTIONS_BIT31)

#define OPTIONS_STRICT (RGX_BUILD_OPTIONS &                  \
                        ~(OPTIONS_DEBUG_MASK               | \
                          OPTIONS_WORKLOAD_ESTIMATION_MASK | \
                          OPTIONS_PDVFS_MASK))

#endif /* RGX_OPTIONS_H */
