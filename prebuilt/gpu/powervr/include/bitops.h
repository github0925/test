/*************************************************************************/ /*!
@File
@Title          Utility functions/macros for bitwise operations
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined (__BITOPS_H__)
#define __BITOPS_H__

#include "img_defs.h"

/********************************************************************************
Function Name      : GetRange
Inputs             : auArr[], uTop, uBottom
Outputs            : uBitData
Returns            : the bits between those offsets into auArr, uTop and uBottom,
					 inclusive. uTop and uBottom are either offsets into 2
					 consecutive elements of auArr or in the same element
Description        : -
**********************************************************************************/
FORCE_INLINE IMG_UINT32 GetRange(const IMG_UINT32 auArr[],
								 const IMG_UINT32 uTop,
								 const IMG_UINT32 uBottom)
{
	IMG_UINT32 uBitData;
	const IMG_UINT32 uTopLong	= uTop >> 5;
	const IMG_UINT32 uBottomLong	= uBottom >> 5;
	const IMG_UINT32 uBottomShift = (IMG_UINT32)uBottom & 0x1FL;
	const IMG_UINT32 uRange = (uTop - uBottom) + 1;
	const IMG_UINT32 uDataMask = ((uRange == 32) ?
								  0xFFFFFFFF :
								  ((1U << uRange) - 1));

	if (uTopLong == uBottomLong)
	{
		/* data fits within one of our 32-bit chunks */
		uBitData = ((auArr[uBottomLong] >> uBottomShift) & uDataMask);
	}
	else
	{
		uBitData = (((auArr[uBottomLong] >> uBottomShift) |
					  (auArr[uTopLong] << (32 - uBottomShift))) &
				   uDataMask);
	}

	return uBitData;
}

FORCE_INLINE IMG_UINT32 GetBit(const IMG_UINT32 auArr[], const IMG_UINT32 uPos)
{
	return auArr[uPos >> 5u] & (1u << (uPos % 32)) ? 1u : 0u;
}

/*
	Write the given bits into the given index locations of auArr
*/
FORCE_INLINE void SetRange(IMG_UINT32 auArr[],
						   const IMG_UINT32 uTop,
						   const IMG_UINT32 uBottom,
						   const IMG_UINT32 uBitData)
{
	const IMG_UINT32 uTopLong = uTop >> 5;
	const IMG_UINT32 uBottomLong	= uBottom >> 5;
	const IMG_UINT32 uBottomShift = (IMG_UINT32)uBottom & 0x1FU;
	const IMG_UINT32 uRange = (uTop - uBottom) + 1;
	const IMG_UINT32 uDataMask = ((uRange == 32) ?
								  0xFFFFFFFF :
								  ((1U << uRange) - 1));

	if (uTopLong == uBottomLong)
	{
		/*
			data fits within one of our 32-bit chunks
		*/
		auArr[uBottomLong] = (auArr[uBottomLong] &
								 (~(uDataMask << uBottomShift))) |
							 ((uBitData & uDataMask) << uBottomShift);
	}
	else
	{
		const IMG_UINT32 uTopShift = 32 - uBottomShift;

		/*
			data is split across two of our 32-bit chunks
		*/
		auArr[uTopLong] = (auArr[uTopLong] & (~(uDataMask >> uTopShift))) |
						  ((uBitData & uDataMask) >> uTopShift);
		auArr[uBottomLong] = (auArr[uBottomLong] &
							  (~(uDataMask << uBottomShift))) |
							 ((uBitData & uDataMask) << uBottomShift);
	}
}

FORCE_INLINE void SetBit(IMG_UINT32 auArr[],
						 const IMG_UINT32 uBit,
						 const IMG_UINT32 uBitData)
{
	if (uBitData)
	{
		auArr[uBit >> 5u] |= (1u << (uBit % 32u));
	}
	else
	{
		auArr[uBit >> 5u] &= ~(1u << (uBit % 32u));
	}
}

FORCE_INLINE IMG_UINT32 CountBits(const IMG_UINT32 auArr[],
								  const IMG_UINT32 uEnd,
								  const IMG_UINT32 uStart)
{
	IMG_UINT32 uCount, i;

	uCount = 0;
	for (i = uStart; i <= uEnd; i++)
	{
		uCount += GetBit(auArr, i);
	}
	return uCount;
}

FORCE_INLINE
IMG_UINT32 FirstSetBit(IMG_UINT32 v)
/*****************************************************************************
 FUNCTION	: FirstSetBit

 PURPOSE	: Returns the first set bit in a dword

 PARAMETERS	: v		- Find the first set bit in this dword (must be non-zero).

 RETURNS	: The (zero-based) index of the first set bit.
*****************************************************************************/
{
#if defined(__GNUC__)

	return __builtin_ctz(v);

#elif defined(LINUX)

	return ffs(v) - 1;

#else /* defined(LINUX) */

#if (_MSC_VER >= 1400)

	IMG_UINT32	uBitPos;

	_BitScanForward((unsigned long *)&uBitPos, v);
	return uBitPos;

#else /* (_MSC_VER >= 1400) */

#if defined(_MSC_VER) && defined(_M_IX86)

	IMG_UINT32 uBitPos;

	__asm
	{
		bsf eax, v
		mov uBitPos, eax
	}
	return uBitPos;

#else /* defined(_MSC_VER) && defined(_M_IX86) */

	/*
		"Using de Bruijn Sequences to Index a 1 in a Computer Word"
		(http://graphics.stanford.edu/~seander/bithacks.html)
	*/

	static const int MultiplyDeBruijnBitPosition[32] =
	{
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return MultiplyDeBruijnBitPosition[(((v & -(IMG_INT32)v) * 0x077CB531U)) >> 27];

#endif /* defined(_MSC_VER) && defined(_M_IX86) */
#endif /* (_MSC_VER >= 1400)  */
#endif /* defined(LINUX) */
}

#endif /* if !defined (__BITOPS_H__) */
