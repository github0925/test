/*************************************************************************/ /*!
@File
@Title          Intrinsics definitions
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _PVR_INTRINSICS_H_
#define _PVR_INTRINSICS_H_

/* PVR_CTZLL:
 * Count the number of trailing zeroes in a long long integer
 */

#if defined(__GNUC__)
#if defined(__x86_64__)

	#define PVR_CTZLL __builtin_ctzll
#endif
#endif

/* PVR_CLZLL:
 * Count the number of leading zeroes in a long long integer
 */

#if defined(__GNUC__)
#if defined(__x86_64__) || defined(__i386__) || defined(__aarch64__) || \
					defined(__arm__) || defined(__mips)

#define PVR_CLZLL __builtin_clzll

#endif
#endif

#endif /* _PVR_INTRINSICS_H_ */
