/*************************************************************************/ /*!
@File
@Title          RGX hw definitions
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _RGXFORMATCONVERT_H_
#define	_RGXFORMATCONVERT_H_

#ifdef INLINE_IS_PRAGMA
#pragma inline(ConvertF16ToF32)
#pragma inline(ConvertF32ToF16)
#pragma inline(ConvertF10ToF32)
#pragma inline(ConvertF11ToF32)
#endif

#define NAN_VALUE 0x7fc00000
#define INF_VALUE 0x7f800000

FORCE_INLINE
IMG_FLOAT ConvertF16ToF32(IMG_UINT32 ui32T1)
/*****************************************************************************
 FUNCTION	: ConvertF16ToF32

 PURPOSE	: Convert from FLOAT16 to FLOAT32.

 PARAMETERS	: ui32T1		- F16 value to convert.

 RETURNS	: Equivalent F32 value.
*****************************************************************************/
{
	IMG_UINT16 ui16Exponent;
	IMG_UINT16 ui16Mantissa;

	IMG_UINT32 ui32Sig;
	IMG_UINT32 ui32Exp;
	IMG_UINT32 ui32Man;

	IMG_UINT32 ui32Value;

	/* Extract the exponent and mantissa of the 16-bit float */
	ui16Exponent = (IMG_UINT32)((ui32T1 & 0x7C00) >> 10);
	ui16Mantissa = (IMG_UINT32)((ui32T1 & 0x03FF) >> 0);

	/* Set ui32Sig */
	ui32Sig = (ui32T1 & 0x8000) >> 15;

	/* Inf value */
	if ((ui16Exponent == 0x1F) && (ui16Mantissa == 0))
	{
		ui32Exp = 0xFF;
		ui32Man = 0;

		ui32Value = (IMG_UINT32)((ui32Sig << 31) | (ui32Exp << 23) | ui32Man);

	}
	else if ((ui16Exponent == 0x1F) && (ui16Mantissa != 0)) /* NaN value: Quiet NaN or Signalling NaN */
	{
		ui32Exp = 0xFF;
		ui32Man = (IMG_UINT32)(ui16Mantissa << (23 - 10));

		ui32Value = (IMG_UINT32)((ui32Sig << 31) | (ui32Exp << 23) | ui32Man);

	}
	else if ((ui16Exponent == 0) && (ui16Mantissa == 0)) /* Zero value: +0 or -0 */
	{
		ui32Value = (IMG_UINT32)(ui32Sig << 31);

	}
	else if ((ui16Exponent == 0) && (ui16Mantissa != 0)) /* 16-bit denormalised to 32-bit normalised */
	{
		IMG_INT16 i16ExpNewVal = ui16Exponent - 0xE; /* 0xE = 14 */
		IMG_UINT16 ui16ManNewVal = 0;

		IMG_UINT16 ui16Temp = ui16Mantissa & 0x3FF;
		IMG_UINT16 ui16Shift = 0;

		while ((ui16Temp & 0x400) == 0) /* 0x400 = 1024 */
		{
			ui16Temp = (IMG_UINT16)(ui16Temp << 1);
			ui16Shift++;
		}

		i16ExpNewVal -= ui16Shift;
		ui16ManNewVal = ui16Temp - 0x400;

		ui32Exp = (i16ExpNewVal + 0x7F) & 0xFF; /* 0x7F = 127 */
		ui32Man = (IMG_UINT32)(ui16ManNewVal << (23 - 10)) & 0x7FFFFF;

		ui32Value = (IMG_UINT32)((ui32Sig << 31) | (ui32Exp << 23) | ui32Man);

	}
	else if ((ui16Exponent > 0) && (ui16Exponent < 0x1F)) /* 16-bit normalised to 32-bit normalised */
	{
		IMG_INT16 i16ExpVal = ui16Exponent - 0xF; // 0xF = 15

		ui32Exp = (i16ExpVal + 127) & 0xFF;
		ui32Man = ((IMG_UINT32)(ui16Mantissa << (23 - 10))) & 0x7FFFFF;

		ui32Value = (IMG_UINT32)((ui32Sig << 31) | (ui32Exp << 23) | ui32Man);

	}
	else
	{
		/*
		 * Build fix for uninitialised return value. If this is a valid case
		 * to be hit, then the logic needs fixing.
		 */
		ui32Value = 0x0;
	}

	return *(IMG_FLOAT *)&ui32Value;

}

#ifdef INLINE_IS_PRAGMA
#pragma inline(ConvertF32ToF16)
#endif

FORCE_INLINE
IMG_UINT16 ConvertF32ToF16(IMG_FLOAT fV)
/*****************************************************************************
 FUNCTION	: ConvertF32ToF16

 PURPOSE	: Convert from FLOAT32 to FLOAT16.

 PARAMETERS	: fV		- F32 value to convert.

 RETURNS	: Equivalent F16 value.
*****************************************************************************/
{
	IMG_UINT16 ui16Value;

	IMG_UINT32 ui32Exponent;
	IMG_UINT32 ui32Mantissa;

	IMG_UINT16 ui16Sig;
	IMG_UINT16 ui16Exp;
	IMG_UINT16 ui16Man;

	/* Extract the exponent and mantissa of the float */
	ui32Exponent = ((*((IMG_UINT32 *)&fV)) & 0x7F800000) >> 23;
	ui32Mantissa = (*((IMG_UINT32 *)&fV)) & 0x7FFFFF;

	/* Set ui16Sig */
	ui16Sig = (IMG_UINT16)((*((IMG_UINT32 *)&fV)) >> 31);

	/* Inf value */
	if ((ui32Exponent == 0xFF) && (ui32Mantissa == 0))
	{
		ui16Exp = 0x1F;
		ui16Man = 0;

		ui16Value = (IMG_UINT16)((ui16Sig << 15) | (ui16Exp << 10) | ui16Man);

	}
	else if ((ui32Exponent == 0xFF) && (ui32Mantissa != 0)) /* NaN value: Quiet NaN or Signalling NaN */
	{
		ui16Exp = 0x1F;
		ui16Man = (IMG_UINT16)(ui32Mantissa >> (23 - 10));

		ui16Value = (IMG_UINT16)((ui16Sig << 15) | (ui16Exp << 10) | ui16Man);

	}
	else if ((ui32Exponent == 0) && (ui32Mantissa == 0)) /* Zero value: +0 or -0 */
	{
		ui16Value = (IMG_UINT16)(ui16Sig << 15);

	}
	else if ((ui32Exponent == 0) && (ui32Mantissa != 0)) /* 2^(-149) -- ((1-2^(-23)) * 2^(-126)), forced into zero */
	{
		ui16Value = (IMG_UINT16)(ui16Sig << 15);

	}
	else if ((ui32Exponent > 0) && (ui32Exponent < 0xFF))  /* 0xFF = 255 */
	{
		IMG_INT32 i32ExpVal = ui32Exponent - 0x7F; /* 0x7F = 127 */

		/* Denormalised value range for 16-bit is: 2^(-24) -- (1-2^(-10)) * 2^(-14)
		   Normalised value range for 16-bit is: 2^(-14) -- (2-2^(-10)) * 2^15 */

		/* underflow value set to 0 */
		if (i32ExpVal < -24)
		{
			ui16Exp = 0;
			ui16Man = 0;
		}
		else if ( ((i32ExpVal >= -24) && (i32ExpVal < -15)) ||
				  ((i32ExpVal == -15) && (ui32Mantissa <= ((1 << 23) - (1 << 14))))
			) /* 16-bit denormalised value needs to be converted from 32-bit normalised value */
		{
			IMG_UINT16 ui16ExpShift = (IMG_UINT16)(i32ExpVal + 24);

			// 16-bit mantissa represents 32-bit exponent part
			ui16Man = (IMG_UINT16)(1 << ui16ExpShift);

			// 16-bit mantissa also depicts 32-bit mantissa value
			ui16Man |= (IMG_UINT16)(ui32Mantissa >> (23 - ui16ExpShift));

			ui16Exp = 0;

		}
		else if ((i32ExpVal == -15) && (ui32Mantissa > ((1 << 23) - (1 << 14)))) /* set to 2^(-14) */
		{
			ui16Exp = 1;
			ui16Man = 0;

		}
		else if ( (i32ExpVal > 15) ||
				  ((i32ExpVal == 15) && (ui32Mantissa > (0x3FF * (1 << (23-10)))))
			) /* overflow value set to the representative max */
		{
			ui16Exp = 0xF;
			ui16Man = 0x3F;
		}
		else /* 16-bit normalised value needs to be converted from 32-bit normalised value */
		{
			ui16Exp = (IMG_UINT16)(i32ExpVal + 0xF);
			ui16Man = (IMG_UINT16)(ui32Mantissa >> (23 - 10));
		}

		ui16Value = (IMG_UINT16)((ui16Sig << 15) | (ui16Exp << 10) | ui16Man);
	}
	else
	{
		/*
		 * Build fix for uninitialised return value. If this is a valid case
		 * to be hit, then the logic needs fixing.
		 */
		ui16Value = 0x0;
	}


	return(ui16Value);

}

FORCE_INLINE
/***********************************************************************//**
 * Converts a 10bit float value to 32bit float.
 *
 * @param ui32F10			10bit float value
 * @return Float value
 **************************************************************************/
IMG_FLOAT ConvertF10ToF32(IMG_UINT16 ui16F10)
{
	IMG_UINT16 ui10Exponent;
	IMG_UINT16 ui10Mantissa;

	IMG_UINT32 ui32Exp;
	IMG_UINT32 ui32Man;

	IMG_UINT32 ui32Value=0;

	/* Extract the exponent and mantissa of the 16-bit float */
	ui10Exponent = (ui16F10 & 0x03E0) >> 5;
	ui10Mantissa = (ui16F10 & 0x001F) >> 0;

	/* no sign bit needs to be set, since 10-bit is unsigned */


	/* Inf value */
	if ((ui10Exponent == 0x1F) && (ui10Mantissa == 0)) /* 0x1F = 31 */
	{
		ui32Exp = 0xFF;
		ui32Man = 0;

		ui32Value = (IMG_UINT32)((ui32Exp << 23) | ui32Man);

	}
	else if ((ui10Exponent == 0x1F) && (ui10Mantissa != 0)) /* NaN value: Quiet NaN or Signalling NaN */
	{
		ui32Exp = 0xFF;
		ui32Man = (IMG_UINT32)(ui10Mantissa << (23 - 5));

		ui32Value = (IMG_UINT32)((ui32Exp << 23) | ui32Man);
	}
	else if ((ui10Exponent == 0) && (ui10Mantissa == 0)) /* Zero value */
	{
		ui32Value = 0;
	}
	else if ((ui10Exponent == 0) && (ui10Mantissa != 0)) /* 10-bit denormalised to 32-bit normalised */
	{
		IMG_INT16 i10ExpNewVal = ui10Exponent - 0xE; /* 0xE = 14 */
		IMG_UINT16 ui10ManNewVal = 0;

		IMG_UINT16 ui16Temp = ui10Mantissa & 0x1F;
		IMG_UINT16 ui16Shift = 0;

		while ((ui16Temp & 0x20) == 0) /* 0x20 = 32 */
		{
			ui16Temp = (IMG_UINT16)(ui16Temp << 1);
			ui16Shift++;
		}

		i10ExpNewVal -= ui16Shift;
		ui10ManNewVal = ui16Temp - 0x20;

		ui32Exp = (IMG_UINT32)((i10ExpNewVal + 0x7F) & 0xFF);
		ui32Man = (IMG_UINT32)((ui10ManNewVal << (23 - 5)) & 0x7FFFFF);

		ui32Value = (IMG_UINT32)((ui32Exp << 23) | ui32Man);
	}
	else if ((ui10Exponent > 0) && (ui10Exponent < 0x1F)) /* 10-bit normalised to 32-bit normalised */
	{
		IMG_INT16 i10ExpVal = ui10Exponent - 0xF; /* 0xF = 15 */

		ui32Exp = (IMG_UINT32)((i10ExpVal + 127) & 0xFF);
		ui32Man = (IMG_UINT32)(ui10Mantissa << (23 - 5)) & 0x7FFFFF;

		ui32Value = (IMG_UINT32)((ui32Exp << 23) | ui32Man);

	}

	return *((IMG_PFLOAT)(&ui32Value));

}

FORCE_INLINE
/***********************************************************************//**
 * Converts a 11bit float value to 32bit float.
 *
 * @param ui32F11			11bit float value
 * @return Float value
 **************************************************************************/
IMG_FLOAT ConvertF11ToF32(IMG_UINT16 ui16F11)
{
	IMG_UINT16 ui11Exponent;
	IMG_UINT16 ui11Mantissa;

	IMG_UINT32 ui32Exp;
	IMG_UINT32 ui32Man;

	IMG_UINT32 ui32Value=0;

	/* Extract the exponent and mantissa of the 16-bit float */
	ui11Exponent = (ui16F11 & 0x07C0) >> 6;
	ui11Mantissa = (ui16F11 & 0x003F) >> 0;

	/* no sign bit needs to be set, since 11-bit is unsigned */


	/* Inf value */
	if ((ui11Exponent == 0x1F) && (ui11Mantissa == 0)) /* 0x1F = 31 */
	{
		ui32Exp = 0xFF;
		ui32Man = 0;

		ui32Value = (IMG_UINT32)((ui32Exp << 23) | ui32Man);

	}
	else if ((ui11Exponent == 0x1F) && (ui11Mantissa != 0)) /* NaN value: Quiet NaN or Signalling NaN */
	{
		ui32Exp = 0xFF;
		ui32Man = (IMG_UINT32)(ui11Mantissa << (23 - 6));

		ui32Value = (IMG_UINT32)((ui32Exp << 23) | ui32Man);
	}
	else if ((ui11Exponent == 0) && (ui11Mantissa == 0)) /* Zero value */
	{
		ui32Value = 0;
	}
	else if ((ui11Exponent == 0) && (ui11Mantissa != 0)) /* 11-bit denormalised to 32-bit normalised */
	{
		IMG_INT16 i11ExpNewVal = ui11Exponent - 0xE; /* 0xE = 14 */
		IMG_UINT16 ui11ManNewVal = 0;

		IMG_UINT16 ui16Temp = ui11Mantissa & 0x3F;
		IMG_UINT16 ui16Shift = 0;

		while ((ui16Temp & 0x40) == 0) /* 0x40 = 64 */
		{
			ui16Temp = (IMG_UINT16)(ui16Temp << 1);
			ui16Shift++;
		}

		i11ExpNewVal -= ui16Shift;
		ui11ManNewVal = ui16Temp - 0x40;

		ui32Exp = (IMG_UINT32)((i11ExpNewVal + 0x7F) & 0xFF); /* 0x7F = 127 */
		ui32Man = (IMG_UINT32)(ui11ManNewVal << (23 - 6)) & 0x7FFFFF;

		ui32Value = (IMG_UINT32)((ui32Exp << 23) | ui32Man);

	}
	else if ((ui11Exponent > 0) && (ui11Exponent < 0x1F)) /* 11-bit normalised to 32-bit normalised */
	{
		IMG_INT16 i11ExpVal = ui11Exponent - 0xF; /* 0xF = 15 */

		ui32Exp = (IMG_UINT32)((i11ExpVal + 127) & 0xFF);
		ui32Man = (IMG_UINT32)(ui11Mantissa << (23 - 6)) & 0x7FFFFF;

		ui32Value = (IMG_UINT32)((ui32Exp << 23) | ui32Man);

	}

	return *((IMG_PFLOAT)(&ui32Value));

}

#endif	/* _RGXFORMATCONVERT_H_ */
