/*************************************************************************/ /*!
@File           pvrsrv_error.h
@Title          services error enumerant
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Defines error codes used by any/all services modules
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(PVRSRV_ERROR_H)
#define PVRSRV_ERROR_H

/*!
 *****************************************************************************
 * Error values
 *****************************************************************************/
typedef enum PVRSRV_ERROR
{
	PVRSRV_OK,
#define PVRE(x) x,
#include "pvrsrv_errors.h"
#undef PVRE
	PVRSRV_ERROR_FORCE_I32 = 0x7fffffff

} PVRSRV_ERROR;

#endif /* !defined(PVRSRV_ERROR_H) */
