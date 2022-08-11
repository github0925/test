//--=========================================================================--
//  This file is a part of JPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2017  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#ifndef __DATA_STRUCTURE_H__
#define __DATA_STRUCTURE_H__

#include "jputypes.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/************************************************************************/
/* Queue                                                                */
/************************************************************************/
typedef struct {
	void   *data;
} QueueData;

typedef struct {
	Uint8      *buffer;
	Uint32      size;
	Uint32      itemSize;
	Uint32      count;
	Uint32      front;
	Uint32      rear;
	JpuMutex    lock;
} Queue;

Queue *Queue_Create(
    Uint32    itemCount,
    Uint32    itemSize);

Queue *Queue_Create_With_Lock(
    Uint32    itemCount,
    Uint32    itemSize);

void Queue_Destroy(
    Queue      *queue);

/**
 * \brief       Enqueue with deep copy
 */
BOOL Queue_Enqueue(
    Queue      *queue,
    void       *data);

/**
 * \brief       Caller has responsibility for releasing the returned data
 */
void *Queue_Dequeue(
    Queue      *queue);

void Queue_Flush(
    Queue      *queue);

void *Queue_Peek(
    Queue      *queue);

Uint32 Queue_Get_Cnt(
    Queue      *queue);

/**
 * \brief       @dstQ is NULL, it allocates Queue structure and then copy from @srcQ.
 */
Queue *Queue_Copy(
    Queue  *dstQ,
    Queue  *srcQ);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DATA_STRUCTURE_H__ */


