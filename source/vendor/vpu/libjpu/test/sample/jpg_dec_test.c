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
 *
 * Revision Histrory:
 * ----------------------
 * Version 1.1, 20/11/2019  chentianming<tianming.chen@semidrive.com> updata & modify this file
 */

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <strings.h>
#include "jpuapi.h"
#include "jpulog.h"
#include "jpuapifunc.h"
#include "main_helper.h"

#define NUM_FRAME_BUF               MAX_FRAME

BOOL TestDecoder(DecConfigParam *param)
{
	JpgDecHandle        handle        = {0};
	JpgDecOpenParam     decOP        = {0};
	JpgDecInitialInfo   initialInfo = {0};
	JpgDecOutputInfo    outputInfo    = {0};
	JpgDecParam         decParam    = {0};
	JpgRet              ret = JPG_RET_SUCCESS;
	FrameBuffer         frameBuf[NUM_FRAME_BUF];
	jpu_buffer_t        vbStream    = {0};
	FRAME_BUF          *pFrame[NUM_FRAME_BUF];
	Uint32              framebufWidth = 0, framebufHeight = 0, framebufStride = 0;
	Int32               i = 0, frameIdx = 0, saveIdx = 0, totalNumofErrMbs = 0;
	BOOL                suc = FALSE;
	Uint8              *pYuv     =    NULL;
	FILE               *fpYuv     =    NULL;
	Int32               needFrameBufCount = 0;
	Int32               int_reason = 0;
	Int32               instIdx = 0;
	Uint32              outbufSize = 0;
	DecConfigParam      decConfig;
	Uint32              decodingWidth, decodingHeight;
	Uint32              displayWidth, displayHeight;
	FrameFormat         subsample;
	Uint32              bitDepth = 0;
	Uint32              temp;
	BOOL                scalerOn = FALSE;
	BSFeeder            feeder;
	Uint32              apiVersion;
	Uint32              hwRevision;
	Uint32              hwProductId;
	Int32 feedSize = 1;
	memcpy(&decConfig, param, sizeof(DecConfigParam));
	memset(pFrame, 0x00, sizeof(pFrame));
	memset(frameBuf, 0x00, sizeof(frameBuf));
	ret = JPU_Init();

	if (ret != JPG_RET_SUCCESS && ret != JPG_RET_CALLED_BEFORE) {
		suc = 0;
		JLOG(ERR, "JPU_Init failed Error code is 0x%x \n", ret );
		goto ERR_DEC_INIT;
	}

	JPU_GetVersionInfo(&apiVersion, &hwRevision, &hwProductId);
	JLOG(INFO, "JPU Version API_VERSION=0x%x, HW_REVISION=%#x, HW_PRODUCT_ID=0x%x\n", apiVersion, hwRevision, hwProductId);

	if ((feeder = BitstreamFeeder_Create(decConfig.bitstreamFileName, decConfig.feedingMode, (EndianMode)decConfig.StreamEndian)) == NULL) {
		goto ERR_DEC_INIT;
	}

	if (strlen(decConfig.yuvFileName)) {
		if ((fpYuv = fopen(decConfig.yuvFileName, "wb")) == NULL) {
			JLOG(ERR, "Can't open %s \n", decConfig.yuvFileName );
			goto ERR_DEC_INIT;
		}
	}

	// Open an instance and get initial information for decoding.
	vbStream.size = (decConfig.bsSize == 0) ? STREAM_BUF_SIZE : decConfig.bsSize;
	vbStream.size = (vbStream.size + 1023) & ~1023; // ceil128(size)

	if (jdi_allocate_dma_memory(&vbStream) < 0) {
		JLOG(ERR, "fail to allocate bitstream buffer\n" );
		goto ERR_DEC_INIT;
	}

	JLOG(INFO, "%s, size %#x, jpu_buffer_t length %d\n", __func__, vbStream.size, sizeof(jpu_buffer_t));
	decOP.streamEndian          = decConfig.StreamEndian;
	decOP.frameEndian           = decConfig.FrameEndian;
	decOP.bitstreamBuffer       = vbStream.phys_addr;
	decOP.bitstreamBufferSize   = vbStream.size;
	//set virtual address mapped of physical address
	decOP.pBitStream            = (BYTE *)vbStream.virt_addr; //lint !e511
	decOP.chromaInterleave      = decConfig.cbcrInterleave;
	decOP.packedFormat          = decConfig.packedFormat;
	decOP.roiEnable             = decConfig.roiEnable;
	decOP.roiOffsetX            = decConfig.roiOffsetX;
	decOP.roiOffsetY            = decConfig.roiOffsetY;
	decOP.roiWidth              = decConfig.roiWidth;
	decOP.roiHeight             = decConfig.roiHeight;
	decOP.rotation              = decConfig.rotation;
	decOP.mirror                = decConfig.mirror;
	decOP.pixelJustification    = decConfig.pixelJustification;
	decOP.outputFormat          = decConfig.subsample;
	decOP.intrEnableBit         = ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_BIT_BUF_EMPTY));
	JLOG(INFO, "------------------------------ DECODER OPTIONS ------------------------------\n");
	JLOG(INFO, "[streamEndian           ]: %d\n", decOP.streamEndian);
	JLOG(INFO, "[frameEndian            ]: %d\n", decOP.frameEndian);
	JLOG(INFO, "[chromaInterleave       ]: %d\n", decOP.chromaInterleave);
	JLOG(INFO, "[chromaInterleave       ]: %d\n", decOP.chromaInterleave);
	JLOG(INFO, "[packedFormat           ]: %d\n", decOP.packedFormat);
	JLOG(INFO, "[roiEnable              ]: %d\n", decOP.roiEnable);
	JLOG(INFO, "[roiOffsetX             ]: %d\n", decOP.roiOffsetX);
	JLOG(INFO, "[roiOffsetY             ]: %d\n", decOP.roiOffsetY);
	JLOG(INFO, "[bitstreamBuffer        ]: 0x%08x\n", decOP.pBitStream);
	JLOG(INFO, "[bitstreamBufferSize    ]: %#x\n", decOP.bitstreamBufferSize);
	JLOG(INFO, "[roiWidth               ]: %d\n", decOP.roiWidth);
	JLOG(INFO, "[roiHeight              ]: %d\n", decOP.roiHeight);
	JLOG(INFO, "[rotation               ]: %d\n", decOP.rotation);
	JLOG(INFO, "[mirror                 ]: %d\n", decOP.mirror);
	JLOG(INFO, "[pixelJustification     ]: %d\n", decOP.pixelJustification);
	JLOG(INFO, "[outputFormat           ]: %d\n", decOP.outputFormat);
	JLOG(INFO, "[intrEnableBit          ]: %d\n", decOP.intrEnableBit);
	JLOG(INFO, "[bsmode                 ]: %d\n", decConfig.feedingMode);
	JLOG(INFO, "-----------------------------------------------------------------------------\n");
	ret = JPU_DecOpen(&handle, &decOP);

	if ( ret != JPG_RET_SUCCESS ) {
		JLOG(ERR, "JPU_DecOpen failed Error code is 0x%x \n", ret );
		goto ERR_DEC_INIT;
	}

	instIdx = handle->instIndex;
	//JPU_DecGiveCommand(handle, ENABLE_LOGGING, NULL);

	do {
		/* Fill jpeg data in the bitstream buffer */
		BitstreamFeeder_Act(feeder, handle, &vbStream);

		if ((ret = JPU_DecGetInitialInfo(handle, &initialInfo)) != JPG_RET_SUCCESS) {
			if (JPG_RET_BIT_EMPTY == ret) {
				JLOG(INFO, "<%s:%d> BITSTREAM EMPTY\n", __FUNCTION__, __LINE__);
				continue;
			} else {
				JLOG(ERR, "JPU_DecGetInitialInfo failed Error code is 0x%x, inst=%d \n", ret, instIdx);
				goto ERR_DEC_OPEN;
			}
		}
	} while (JPG_RET_SUCCESS != ret);

	if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_422)
		framebufWidth = JPU_CEIL(16, initialInfo.picWidth);
	else
		framebufWidth  = JPU_CEIL(8, initialInfo.picWidth);

	if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_440)
		framebufHeight = JPU_CEIL(16, initialInfo.picHeight);
	else
		framebufHeight = JPU_CEIL(8, initialInfo.picHeight);

	decodingWidth  = framebufWidth  >> decConfig.iHorScaleMode;
	decodingHeight = framebufHeight >> decConfig.iVerScaleMode;

	if (decOP.packedFormat != PACKED_FORMAT_NONE && decOP.packedFormat != PACKED_FORMAT_444) {
		// When packed format, scale-down resolution should be multiple of 2.
		decodingWidth  = JPU_CEIL(2, decodingWidth);
	}

	subsample = (decConfig.subsample == FORMAT_MAX) ? initialInfo.sourceFormat : decConfig.subsample;
	temp           = decodingWidth;
	decodingWidth  = (decConfig.rotation == 90 || decConfig.rotation == 270) ? decodingHeight : decodingWidth;
	decodingHeight = (decConfig.rotation == 90 || decConfig.rotation == 270) ? temp           : decodingHeight;

	if (decConfig.roiEnable == TRUE) {
		decodingWidth  = framebufWidth  = initialInfo.roiFrameWidth ;
		decodingHeight = framebufHeight = initialInfo.roiFrameHeight;
	}

	if (0 != decConfig.iHorScaleMode || 0 != decConfig.iVerScaleMode) {
		displayWidth  = JPU_FLOOR(2, (framebufWidth >> decConfig.iHorScaleMode));
		displayHeight = JPU_FLOOR(2, (framebufHeight >> decConfig.iVerScaleMode));
	} else {
		displayWidth  = decodingWidth;
		displayHeight = decodingHeight;
	}

	// Check restrictions
	if (decOP.rotation != 0 || decOP.mirror != MIRDIR_NONE) {
		if (decOP.outputFormat != FORMAT_MAX && decOP.outputFormat != initialInfo.sourceFormat) {
			JLOG(ERR, "The rotator cannot work with the format converter together.\n");
			goto ERR_DEC_OPEN;
		}
	}

	needFrameBufCount = initialInfo.minFrameBufferCount;
	bitDepth          = initialInfo.bitDepth;
	scalerOn          = (BOOL)(decConfig.iHorScaleMode || decConfig.iVerScaleMode);

	if (AllocateFrameBuffer(instIdx, subsample, decOP.chromaInterleave, decOP.packedFormat, decConfig.rotation, scalerOn, decodingWidth, decodingHeight, bitDepth, needFrameBufCount) == FALSE) {
		JLOG(ERR, "Failed to AllocateFrameBuffer()\n");
		goto ERR_DEC_OPEN;
	}

	JLOG(INFO, "[instIdx              ]: %d\n", instIdx);
	JLOG(INFO, "[source picture size  ]: (%d X %d)\n", initialInfo.picWidth, initialInfo.picHeight);
	JLOG(INFO, "[decode picture size  ]: (%d X %d)\n", decodingWidth, decodingHeight);
	JLOG(INFO, "[display width size   ]: (%d X %d)\n", displayWidth);
	JLOG(INFO, "[display height size  ]: (%d X %d)\n", displayHeight);
	JLOG(INFO, "[subsample            ]: %d\n", subsample);
	JLOG(INFO, "[needFrameBufCount    ]: %d\n", needFrameBufCount);
	JLOG(INFO, "[bitDepth             ]: %d\n", bitDepth);
	JLOG(INFO, "[packedFormat         ]: %d\n", decOP.packedFormat);
	JLOG(INFO, "[frameNum             ]: %d\n", param->outNum);

	for ( i = 0; i < needFrameBufCount; ++i ) {
		pFrame[i] = GetFrameBuffer(instIdx, i);
		frameBuf[i].bufY  = pFrame[i]->vbY.phys_addr;
		frameBuf[i].bufCb = pFrame[i]->vbCb.phys_addr;

		if (decOP.chromaInterleave == CBCR_SEPARATED)
			frameBuf[i].bufCr = pFrame[i]->vbCr.phys_addr;

		frameBuf[i].stride  = pFrame[i]->strideY;
		frameBuf[i].strideC = pFrame[i]->strideC;
		frameBuf[i].endian  = decOP.frameEndian;
		frameBuf[i].format  = (FrameFormat)pFrame[i]->Format;
	}

	framebufStride = frameBuf[0].stride;
	outbufSize = decodingWidth * decodingHeight * 3 * ((bitDepth + 7) / 8);

	if ((pYuv = malloc(outbufSize)) == NULL) {
		JLOG(ERR, "Fail to allocation memory for display buffer\n");
		goto ERR_DEC_OPEN;
	}

	// Register frame buffers requested by the decoder.
	if ((ret = JPU_DecRegisterFrameBuffer(handle, frameBuf, needFrameBufCount, framebufStride)) != JPG_RET_SUCCESS) {
		JLOG(ERR, "JPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
		goto ERR_DEC_OPEN;
	}

	JPU_DecGiveCommand(handle, SET_JPG_SCALE_HOR,  &(decConfig.iHorScaleMode));
	JPU_DecGiveCommand(handle, SET_JPG_SCALE_VER,  &(decConfig.iVerScaleMode));
	/* LOG HEADER */
	JLOG(INFO, "I   F    FB_INDEX  FRAME_START  ECS_START  CONSUME   RD_PTR   WR_PTR      CYCLE\n");
	JLOG(INFO, "-------------------------------------------------------------------------------\n");

	while (1) {
		// Start decoding a frame.
		ret = JPU_DecStartOneFrame(handle, &decParam);

		if (ret != JPG_RET_SUCCESS && ret != JPG_RET_EOS) {
			if (ret == JPG_RET_BIT_EMPTY) {
				JLOG(INFO, "BITSTREAM NOT ENOUGH.............\n");
				BitstreamFeeder_Act(feeder, handle, &vbStream);
				continue;
			}

			JLOG(ERR, "JPU_DecStartOneFrame failed Error code is 0x%x \n", ret );
			goto ERR_DEC_OPEN;
		}

		if (ret == JPG_RET_EOS) {
			JLOG(INFO, "DECODE FINISH\n");
			JPU_DecGetOutputInfo(handle, &outputInfo);
			break;
		}

		while (1) {
			if ((int_reason = JPU_WaitInterrupt(handle, JPU_INTERRUPT_TIMEOUT_MS)) == -1) {
				JLOG(ERR, "Error : timeout happened\n");
				JPU_SWReset(handle);
				break;
			}

			if (int_reason == -2) {
				JLOG(ERR, "Interrupt occurred. but this interrupt is not for my instance \n");
				break;
			}

			if (int_reason & ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_SLICE_DONE))) {
				JLOG(INFO, "INSTANCE #%d int_reason: %08x\n", handle->instIndex, int_reason);
				break;
			}

			if (int_reason & (1 << INT_JPU_BIT_BUF_EMPTY)) {
				if (decConfig.feedingMode != FEEDING_METHOD_FRAME_SIZE) {
					JLOG(INFO, " %s interrupt reason %#x INT_JPU_BIT_BUF_EMPTY now \n", __func__, int_reason);
					BitstreamFeeder_Act(feeder, handle, &vbStream);
				}

				JPU_ClrStatus(handle, (1 << INT_JPU_BIT_BUF_EMPTY));
				JPU_EnableInterrput(handle, decOP.intrEnableBit);
			}
		}

		JLOG(INFO, "\t<->INSTANCE #%d JPU_WaitInterrupt\n", handle->instIndex);

		if ((ret = JPU_DecGetOutputInfo(handle, &outputInfo)) != JPG_RET_SUCCESS) {
			JLOG(ERR, "JPU_DecGetOutputInfo failed Error code is 0x%x \n", ret );
			goto ERR_DEC_OPEN;
		}

		if (outputInfo.decodingSuccess == 0)
			JLOG(ERR, "JPU_DecGetOutputInfo decode fail framdIdx %d \n", frameIdx);

		JLOG(INFO, "%02d %04d  %8d     %8x %8x %10d  %8x  %8x %10d\n",
		     instIdx, frameIdx, outputInfo.indexFrameDisplay, outputInfo.bytePosFrameStart, outputInfo.ecsPtr, outputInfo.consumedByte,
		     outputInfo.rdPtr, outputInfo.wrPtr, outputInfo.frameCycle);

		if (outputInfo.indexFrameDisplay == -1)
			break;

		saveIdx = outputInfo.indexFrameDisplay;
		JLOG(INFO, "FRAMEBUFER[saveId], saveIdx %d, addr %p, pYuv1 %p outbufSize %d\n", saveIdx, &frameBuf[saveIdx], pYuv, outbufSize);

		if (!SaveYuvImageHelperFormat_V20(fpYuv, pYuv, &frameBuf[saveIdx], decOP.chromaInterleave, decOP.packedFormat, decodingWidth, decodingHeight, bitDepth, FALSE)) {
			goto ERR_DEC_OPEN;
		}

		if (outputInfo.numOfErrMBs) {
			Int32 errRstIdx, errPosX, errPosY;
			errRstIdx = (outputInfo.numOfErrMBs & 0x0F000000) >> 24;
			errPosX = (outputInfo.numOfErrMBs & 0x00FFF000) >> 12;
			errPosY = (outputInfo.numOfErrMBs & 0x00000FFF);
			JLOG(ERR, "Error restart Idx : %d, MCU x:%d, y:%d, in Frame : %d \n", errRstIdx, errPosX, errPosY, frameIdx);
		}

		frameIdx++;
		JLOG(ERR, "frame num   %d \n", frameIdx);

		if (decConfig.outNum && (frameIdx == decConfig.outNum)) {
			suc = TRUE;
			break;
		}

		if (decConfig.feedingMode == FEEDING_METHOD_FRAME_SIZE && feedSize != 0) {
			JPU_DecSetRdPtrEx(handle, vbStream.phys_addr, TRUE);
			feedSize = BitstreamFeeder_Act(feeder, handle, &vbStream);
		}
	}

	if (totalNumofErrMbs) {
		suc = 0;
		JLOG(ERR, "Total Num of Error MBs : %d\n", totalNumofErrMbs);
	}

ERR_DEC_OPEN:
	FreeFrameBuffer(instIdx);
	ret = JPU_DecClose(handle);

	if (ret != JPG_RET_SUCCESS)
		suc = 0;

	JLOG(INFO, "\nDec End. Tot Frame %d\n", frameIdx);
	BitstreamFeeder_Destroy(feeder);
ERR_DEC_INIT:
	jdi_free_dma_memory(&vbStream);

	if (pYuv)
		free(pYuv);

	if (fpYuv)
		fclose(fpYuv);

	JPU_DeInit();
	return suc;
}

#if 0  // for CM test
static void Help(
    const char *programName
)
{
	JLOG(INFO, "------------------------------------------------------------------------------\n");
	JLOG(INFO, " CODAJ12 Decoder/Encoder\n");
	JLOG(INFO, "------------------------------------------------------------------------------\n");
	JLOG(INFO, "%s [options] --input=jpg_file_path\n", programName);
	JLOG(INFO, "-h                      help\n");
	JLOG(INFO, "--input=FILE            jpeg filepath\n");
	JLOG(INFO, "--output=FILE           output file path\n");
	JLOG(INFO, "--stream-endian=ENDIAN  bitstream endianness. refer to datasheet Chapter 4.\n");
	JLOG(INFO, "--frame-endian=ENDIAN   pixel endianness of 16bit input source. refer to datasheet Chapter 4.\n");
	JLOG(INFO, "--pixelj=JUSTIFICATION  16bit-pixel justification. 0(default) - msb justified, 1 - lsb justified in little-endianness\n");
	JLOG(INFO, "--bs-size=SIZE          bitstream buffer size in byte\n");
	JLOG(INFO, "--roi=x,y,w,h           ROI region\n");
	JLOG(INFO, "--subsample             conversion sub-sample(ignore case): NONE, 420, 422, 444\n");
	JLOG(INFO, "--ordering              conversion ordering(ingore-case): NONE, NV12, NV21, YUYV, YVYU, UYVY, VYUY, AYUV\n");
	JLOG(INFO, "                        NONE - planar format\n");
	JLOG(INFO, "                        NV12, NV21 - semi-planar format for all the subsamples.\n");
	JLOG(INFO, "                                     If subsample isn't defined or is none, the sub-sample depends on jpeg information\n");
	JLOG(INFO, "                                     The subsample 440 can be converted to the semi-planar format. It means that the encoded sub-sample should be 440.\n");
	JLOG(INFO, "                        YUVV..VYUY - packed format. subsample be ignored.\n");
	JLOG(INFO, "                        AYUV       - packed format. subsample be ignored.\n");
	JLOG(INFO, "--rotation              0, 90, 180, 270\n");
	JLOG(INFO, "--mirror                0(none), 1(V), 2(H), 3(VH)\n");
	JLOG(INFO, "--scaleH                Horizontal downscale: 0(none), 1(1/2), 2(1/4), 3(1/8)\n");
	JLOG(INFO, "--scaleV                Vertical downscale  : 0(none), 1(1/2), 2(1/4), 3(1/8)\n");
	JLOG(INFO, "--bsmode                default 0  frame; 0! fix_size \n");
	exit(1);
}

Int32 main(Int32 argc, char **argv)
{
	Int32   ret = 1;
	struct option longOpt[] = {
		{ "stream-endian",      required_argument, NULL, 0 },
		{ "frame-endian",       required_argument, NULL, 0 },
		{ "output",             required_argument, NULL, 0 },
		{ "input",              required_argument, NULL, 0 },
		{ "pixelj",             required_argument, NULL, 0 },
		{ "bs-size",            required_argument, NULL, 0 },
		{ "roi",                required_argument, NULL, 0 },
		{ "subsample",          required_argument, NULL, 0 },
		{ "ordering",           required_argument, NULL, 0 },
		{ "rotation",           required_argument, NULL, 0 },
		{ "mirror",             required_argument, NULL, 0 },
		{ "scaleH",             required_argument, NULL, 0 },
		{ "scaleV",             required_argument, NULL, 0 },
		{ "slice-height",       required_argument, NULL, 0 },
		{ "enable-slice-intr",  required_argument, NULL, 0 },
		{ "bsmode",             required_argument, NULL, 0 },
		{ NULL,                 no_argument,       NULL, 0 },
	};
	Int32           c, l;
	char           *shortOpt    = (char *)"fhn:";
	DecConfigParam  config;
	char           *ext         = NULL;
	memset((void *)&config, 0x00, sizeof(DecConfigParam));
	config.subsample = FORMAT_MAX;

	while ((c = getopt_long(argc, argv, shortOpt, longOpt, &l)) != -1) {
		switch (c) {
			case 'h':
				Help(argv[0]);
				break;

			case 'n':
				config.outNum = atoi(optarg);
				JLOG(INFO, "forceNum %d\n", config.outNum);
				break;

			case 0:
				if (ParseDecTestLongArgs((void *)&config, longOpt[l].name, optarg) == FALSE) {
					Help(argv[0]);
				}

				break;

			default:
				Help(argv[0]);
				break;
		}
	}

	/* CHECK PARAMETERS */
	if ((config.iHorScaleMode || config.iVerScaleMode) && config.roiEnable) {
		JLOG(ERR, "Invalid operation mode : ROI cannot work with the scaler\n");
		return 1;
	}

	if (config.packedFormat && config.roiEnable) {
		JLOG(ERR, "Invalid operation mode : ROI cannot work with the packed format conversion\n");
		return 1;
	}

	if (config.roiEnable && (config.rotation || config.mirror)) {
		JLOG(ERR, "Invalid operation mode : ROI cannot work with the PPU.\n");
	}

	ext = GetFileExtension(config.bitstreamFileName);

//    if (strcasecmp("avi", ext) == 0 || strcasecmp("mkv", ext) == 0) {
//        config.feedingMode = FEEDING_METHOD_FRAME_SIZE;
//   }
	if (config.feedingMode == 0)
		config.feedingMode = FEEDING_METHOD_FRAME_SIZE;
	else
		config.feedingMode = FEEDING_METHOD_FIXED_SIZE;

	InitLog("ErrorLog.txt");
	ret = TestDecoder(&config);
	DeInitLog();
	return ret == TRUE ? 0 : 1;
}
#endif

