/*
 * Copyright (c) 2018, Chips&Media
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "jpuapi.h"
#include "main_helper.h"

typedef struct {
	FeedingMethod   method;
	Uint8          *remainData;
	Uint32          remainDataSize;
	Uint32          remainOffset;
	void           *actualFeeder;
	Uint32          room;
	BOOL            eos;
	EndianMode      endian;
} BitstreamFeeder;

extern void *BSFeederFixedSize_Create(const char *path);
extern BOOL BSFeederFixedSize_Destroy(void *feeder);
extern Int32 BSFeederFixedSize_Act(void *feeder, BSChunk *chunk);
extern void BSFeederFixedSize_SetFeedingSize(void *feeder, Uint32  feedingSize);

extern void *BSFeederFrameSize_Create(const char *path);
extern BOOL BSFeederFrameSize_Destroy(void *feeder);
extern Int32 BSFeederFrameSize_Act(void *feeder, BSChunk *packet);

/**
* Abstract Bitstream Feeader Functions
*/
BSFeeder BitstreamFeeder_Create(const char *path, FeedingMethod method, EndianMode endian)
{
	BitstreamFeeder    *handle = NULL;
	void               *feeder = NULL;

	switch (method) {
	case FEEDING_METHOD_FIXED_SIZE:
		feeder = BSFeederFixedSize_Create(path);
		break;

	case FEEDING_METHOD_FRAME_SIZE:
		feeder = BSFeederFrameSize_Create(path);
		break;

	default:
		feeder = NULL;
		break;
	}

	if (feeder != NULL) {
		if ((handle = (BitstreamFeeder *)malloc(sizeof(BitstreamFeeder))) == NULL) {
			JLOG(ERR, "%s:%d Failed to allocate memory\n", __FUNCTION__, __LINE__);
			return NULL;
		}

		handle->actualFeeder = feeder;
		handle->method       = method;
		handle->remainData   = NULL;
		handle->remainDataSize = 0;
		handle->remainOffset = 0;
		handle->eos          = FALSE;
		handle->endian       = endian;
	}

	return (BSFeeder)handle;
}

Uint32 BitstreamFeeder_Act(BSFeeder feeder, JpgDecHandle handle, jpu_buffer_t *bsBuffer)
{
	BitstreamFeeder *bsf = (BitstreamFeeder *)feeder;
	Int32            feedingSize = 0;
	Int32            room;
	BSChunk          chunk = {0};
	EndianMode       endian;
	PhysicalAddress  wrPtr;

	if (bsf == NULL) {
		JLOG(ERR, "%s Null handle\n", __func__);
		return 0;
	}

	JPU_DecGetBitstreamBuffer(handle, NULL/* rdPtr */, &wrPtr, &room);
	endian = bsf->endian;

	if (bsf->remainData == NULL) {
		chunk.size = bsBuffer->size;
		chunk.data = malloc(chunk.size);
		chunk.eos  = FALSE;

		if (chunk.data == NULL) {
			JLOG(ERR, "%s failed to allocate memory\n", __func__);
			return 0;
		}

		switch (bsf->method) {
		case FEEDING_METHOD_FIXED_SIZE:
			feedingSize = BSFeederFixedSize_Act(bsf->actualFeeder, &chunk);
			break;

		case FEEDING_METHOD_FRAME_SIZE:
			feedingSize = BSFeederFrameSize_Act(bsf->actualFeeder, &chunk);
			break;

		default:
			JLOG(ERR, "%s:%d Invalid method(%d)\n", __func__, bsf->method);
			free(chunk.data);
			return 0;
		}
	} else {
		chunk.data  = bsf->remainData;
		feedingSize = bsf->remainDataSize;
	}

	if (feedingSize < 0) {
		JLOG(ERR, "feeding size is negative value: %d\n", feedingSize);
		free(chunk.data);
		return 0;
	}

	if (feedingSize > 0) {
		Uint32          rightSize = 0, leftSize = 0;
		PhysicalAddress base  = bsBuffer->phys_addr;
		Uint32          size  = bsBuffer->size;
		Uint32          wSize = feedingSize;
		Uint8          *ptr   = chunk.data;;
		ptr                 += bsf->remainOffset;

		if ((Int32)room < feedingSize) {
			wSize               = room;
			bsf->remainData     = chunk.data;
			bsf->remainDataSize = feedingSize - wSize;
			bsf->remainOffset   += wSize;
		} else {
			bsf->remainData     = NULL;
			bsf->remainDataSize = 0;
			bsf->remainOffset   = 0;
		}

		leftSize = wSize;

		if ((wrPtr + wSize) >= (base + size)) {
			PhysicalAddress endAddr = base + size;
			rightSize = endAddr - wrPtr;
			leftSize  = (wrPtr + wSize) - endAddr;

			if (rightSize > 0) {
				JpuWriteMem(wrPtr, ptr, rightSize, (int)endian);
			}

			wrPtr = base;
			JLOG(INFO, " %s: %d, wrPtr %p\n", __func__, __LINE__, wrPtr);
		}

		JpuWriteMem(wrPtr, ptr + rightSize, leftSize, (int)endian);
		JPU_DecUpdateBitstreamBuffer(handle, wSize);
	}

	if ((TRUE == chunk.eos) && (bsf->remainDataSize == 0 /* there is no remain data to be fed more*/)) {
		JPU_DecUpdateBitstreamBuffer(handle, 0);
	}

	/* now feedingSize = 0, */
	if ((TRUE == chunk.eos) && (bsf->remainDataSize == 0)) {
		JLOG(INFO, "%s: file end now\n", __func__);
		JPU_DecUpdateBitstreamBuffer(handle, 0);
		bsf->eos = TRUE;

		if (bsf->method == FEEDING_METHOD_FRAME_SIZE)
			return 0;
	} else {
		bsf->eos = FALSE;
	}

	if (NULL == bsf->remainData)
		free(chunk.data);

	return feedingSize;
}

BOOL BitstreamFeeder_IsEos(BSFeeder feeder)
{
	BitstreamFeeder *bsf = (BitstreamFeeder *)feeder;

	if (bsf == NULL) {
		JLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	return bsf->eos;
}

BOOL BitstreamFeeder_Destroy(BSFeeder feeder)
{
	BitstreamFeeder *bsf = (BitstreamFeeder *)feeder;

	if (bsf == NULL) {
		return FALSE;
	}

	switch (bsf->method) {
	case FEEDING_METHOD_FIXED_SIZE:
		BSFeederFixedSize_Destroy(bsf->actualFeeder);
		break;

	case FEEDING_METHOD_FRAME_SIZE:
		BSFeederFrameSize_Destroy(bsf->actualFeeder);
		break;

	default:
		JLOG(ERR, "%s:%d Invalid method(%d)\n", __FUNCTION__, __LINE__, bsf->method);
		break;
	}

	if (bsf->remainData) {
		free(bsf->remainData);
	}

	free(bsf);
	return TRUE;
}

extern BOOL BSFeederFixedSize_Rewind(void *feeder);

BOOL BitstreamFeeder_Rewind(BSFeeder feeder)
{
	BitstreamFeeder    *bsf = (BitstreamFeeder *)feeder;
	BOOL                success = FALSE;

	if (bsf == NULL) {
		JLOG(ERR, "%s:%d handle is NULL\n", __FUNCTION__, __LINE__);
		return success;
	}

	switch (bsf->method) {
	case FEEDING_METHOD_FIXED_SIZE:
		success = BSFeederFixedSize_Rewind(bsf->actualFeeder);
		break;

	default:
		JLOG(ERR, "%s:%d Invalid method(%d)\n", __FUNCTION__, __LINE__, bsf->method);
		break;
	}

	return success;
}

