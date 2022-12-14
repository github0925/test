//--=========================================================================--
//  This file is a part of QC Tool project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2004 - 2011   CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--
#include "main_helper.h"
//#define SUPPORT_FFMPEG_DEMUX
#ifdef SUPPORT_FFMPEG_DEMUX
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#define RCV_V2
#define IS_VP9_SUPERFRAME(__header)     ((__header & 0xe0) == 0xc0)
#define MAX_VP9_SUPERFRAME_INDEX_SIZE   34
#define VP9_MAX_SUBFRAMES               8

typedef struct VP9Superframe {
    Uint32  nframes;
    Uint32  frameSize[VP9_MAX_SUBFRAMES];
    void*   frames[VP9_MAX_SUBFRAMES];   /* A superframe has multiple frames up to 8 frames. */
    Uint32  currentIndex;
} VP9Superframe;

typedef struct ReaderContext {
    AVFormatContext*    avContext;
    BOOL                isFirstPacket;
    Uint32              videoIndex;
    Uint32              standard;
    Uint32              mp4ClassId;
    Uint8*              tempBuffer;
    Uint32              tempRdPtr;
    Uint32              tempWrPtr;
    Uint32              seqWidth;
    Uint32              seqHeight;
    VP9Superframe       superframe;
} ReaderContext;


static Uint32 u_bytes(
    Uint8*  data,
    Uint32  len
    )
{
    Uint32  i;
    Uint32  val=0;

    for (i=0; i<len; i++) {
        val |= data[i] << (i*8);
    }

    return val;
}

static BOOL VP9ParseSuperframe(
    void*           chunk,
    Uint32          size,
    VP9Superframe*  superframe
    )
{
    Uint32  startPos;
    Uint32  frameSizeLength = 0;
    Uint32  numFrames, totalSubframeSize = 0;
    Uint32   i;
    Uint8*  pData = NULL, superframeMarker;

    startPos = size - 1;

    pData = (Uint8*)chunk;
    pData = &pData[startPos];
    superframeMarker = *pData;
    /* Find first SUPERFRAME_MARKER(0b110x_xxxx) */
    for (i=1; i<MAX_VP9_SUPERFRAME_INDEX_SIZE; i++) {
        if (superframeMarker == *(pData-i)) {
            break;
        }
    }

    if (i == MAX_VP9_SUPERFRAME_INDEX_SIZE) {
        return FALSE;
    }

    pData = pData - i;
    frameSizeLength = ((*pData>>3) & 0x03) + 1;
    numFrames       = (*pData&0x07) + 1;
    pData++;
    for (i=0; i<numFrames; i++) {
        superframe->frameSize[i] = u_bytes(pData, frameSizeLength);
        pData += frameSizeLength;
    }

    /* Check post SUPERFRAME_MARKER */
    if (IS_VP9_SUPERFRAME(*pData) == FALSE) {
        VLOG(ERR, "INVALID POST SUPERFRAME_MARKER\n");
        return FALSE;
    }

    /* Check size */
    for (i=0; i<numFrames; i++) {
        totalSubframeSize += superframe->frameSize[i];
    }
    if (totalSubframeSize >= size) {
        VLOG(ERR, "TOTAL SIZE OF SUBFRAMES IS BIGGER THAN CHUNK SIZE\n");
        return FALSE;
    }


    pData = (Uint8*)chunk;
    for (i=0; i<numFrames; i++) {
        superframe->frames[i] = (void*)osal_malloc(superframe->frameSize[i]);
        memcpy(superframe->frames[i], (void*)pData, superframe->frameSize[i]);
        pData += superframe->frameSize[i];
    }
    superframe->currentIndex = 0;
    superframe->nframes      = numFrames;

    return TRUE;
}

static Int32 BuildSeqHeader(
    Uint8*        pbHeader,
    const CodStd    codStd,
    const AVStream* st,
    Int32*        sizelength
    )
{
    AVCodecParameters *codec_par = st->codecpar;
    Uint8*        pbMetaData = codec_par->extradata;
    Int32         nMetaData = codec_par->extradata_size;
    Uint8* p =    pbMetaData;
    Uint8 *a =    p + 4 - ((long) p & 3);
    Uint8* t =    pbHeader;
    Int32         size;
    Int32         fourcc;
    Int32         sps, pps, i, nal;
    Int32         frameRate = 0;

    fourcc = codec_par->codec_tag;
    if (!fourcc)
        fourcc = ConvCodecIdToFourcc(codec_par->codec_id);

    if (st->avg_frame_rate.den && st->avg_frame_rate.num)
        frameRate = (Int32)((double)st->avg_frame_rate.num/(double)st->avg_frame_rate.den);

    VLOG(INFO, "In %s : %d nMetaData: %d, pbMetaData %p , codStd %d\n",
            __FUNCTION__,__LINE__,
            nMetaData,
            pbMetaData,
            codStd);

    size = 0;
    *sizelength = 4; // default size length(in bytes) = 4
    if (codStd == STD_AVC || codStd == STD_AVS) {
        if (nMetaData > 1 && pbMetaData && pbMetaData[0] == 0x01) {
            // check mov/mo4 file format stream
            p += 4;
            *sizelength = (*p++ & 0x3) + 1;
            sps = (*p & 0x1f); // Number of sps
            p++;
            for (i = 0; i < sps; i++) {
                nal = (*p << 8) + *(p + 1) + 2;
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x01);
                PUT_BUFFER(t, p+2, nal-2);
                p += nal;
                size += (nal - 2 + 4); // 4 => length of start code to be inserted
            }

            pps = *(p++); // number of pps
            for (i = 0; i < pps; i++)
            {
                nal = (*p << 8) + *(p + 1) + 2;
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x01);
                PUT_BUFFER(t, p+2, nal-2);
                p += nal;
                size += (nal - 2 + 4); // 4 => length of start code to be inserted
            }
        }
        else if(nMetaData > 3) {
            size = -1;// return to meaning of invalid stream data;
            for (; p < a; p++) {
                if (p[0] == 0 && p[1] == 0 && p[2] == 1)  {
                    // find startcode
                    size = codec_par->extradata_size;
                    PUT_BUFFER(pbHeader, pbMetaData, size);
                    break;
                }
            }
        }
    }
    else if (codStd == STD_HEVC) {
        if (nMetaData > 1 && pbMetaData && pbMetaData[0] == 0x01) {
            static const Uint8 nalu_header[4] = { 0, 0, 0, 1 };
            Int32 numOfArrays = 0;
            Uint16 numNalus = 0;
            Uint16 nalUnitLength = 0;
            Uint32 offset = 0;

            p += 21;
            *sizelength = (*p++ & 0x3) + 1;
            numOfArrays = *p++;

            while(numOfArrays--) {
                p++;   // NAL type
                numNalus = (*p << 8) + *(p + 1);
                p+=2;
                for(i = 0;i < numNalus;i++)
                {
                    nalUnitLength = (*p << 8) + *(p + 1);
                    p+=2;
                    //if(i == 0)
                    {
                        memcpy(pbHeader + offset, nalu_header, 4);
                        offset += 4;
                        memcpy(pbHeader + offset, p, nalUnitLength);
                        offset += nalUnitLength;
                    }
                    p += nalUnitLength;
                }
            }

            size = offset;
        }
        else if(nMetaData > 3)
        {
            size = -1;// return to meaning of invalid stream data;

            for (; p < a; p++)
            {
                if (p[0] == 0 && p[1] == 0 && p[2] == 1) // find startcode
                {
                    size = codec_par->extradata_size;
                    PUT_BUFFER(pbHeader, pbMetaData, size);
                    break;
                }
            }
        }
    }
    else if (codStd == STD_VC1)
    {
        if (!fourcc)
            return -1;
        if (fourcc == MKTAG('W', 'V', 'C', '1') || fourcc == MKTAG('W', 'M', 'V', 'A'))	//VC AP
        {
            size = nMetaData;
            PUT_BUFFER(pbHeader, pbMetaData, size);
            //if there is no seq startcode in pbMetatData. VPU will be failed at seq_init stage.
        }
        else
        {
#ifdef RCV_V2
            PUT_LE32(pbHeader, ((0xC5 << 24)|0));
            size += 4; //version
            PUT_LE32(pbHeader, nMetaData);
            size += 4;
            PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
            size += nMetaData;
            PUT_LE32(pbHeader, codec_par->height);
            size += 4;
            PUT_LE32(pbHeader, codec_par->width);
            size += 4;
            PUT_LE32(pbHeader, 12);
            size += 4;
            PUT_LE32(pbHeader, 2 << 29 | 1 << 28 | 0x80 << 24 | 1 << 0);
            size += 4; // STRUCT_B_FRIST (LEVEL:3|CBR:1:RESERVE:4:HRD_BUFFER|24)
            PUT_LE32(pbHeader, codec_par->bit_rate);
            size += 4; // hrd_rate
            PUT_LE32(pbHeader, frameRate);
            size += 4; // frameRate
#else	//RCV_V1
            PUT_LE32(pbHeader, (0x85 << 24) | 0x00);
            size += 4; //frames count will be here
            PUT_LE32(pbHeader, nMetaData);
            size += 4;
            PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
            size += nMetaData;
            PUT_LE32(pbHeader, codec_par->height);
            size += 4;
            PUT_LE32(pbHeader, codec_par->width);
            size += 4;
#endif
        }
    }
    else if (codStd == STD_RV)
    {
        Int32 st_size =0;

        if (!fourcc)
            return -1;
        if (fourcc != MKTAG('R','V','3','0') && fourcc != MKTAG('R','V','4','0'))
            return -1;

        size = 26 + nMetaData;
        PUT_BE32(pbHeader, size); //Length
        PUT_LE32(pbHeader, MKTAG('V', 'I', 'D', 'O')); //MOFTag
        PUT_LE32(pbHeader, fourcc); //SubMOFTagl
        PUT_BE16(pbHeader, codec_par->width);
        PUT_BE16(pbHeader, codec_par->height);
        PUT_BE16(pbHeader, 0x0c); //BitCount;
        PUT_BE16(pbHeader, 0x00); //PadWidth;
        PUT_BE16(pbHeader, 0x00); //PadHeight;

        PUT_LE32(pbHeader, frameRate);
        PUT_BUFFER(pbHeader, pbMetaData, nMetaData); //OpaqueDatata
        size += st_size; //add for startcode pattern.
    }
    else if (codStd == STD_DIV3) {
        // not implemented yet
        if (!nMetaData) {
            PUT_LE32(pbHeader, MKTAG('C', 'N', 'M', 'V')); //signature 'CNMV'
            PUT_LE16(pbHeader, 0x00);                      //version
            PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
            PUT_LE32(pbHeader, MKTAG('D', 'I', 'V', '3')); //codec FourCC
            PUT_LE16(pbHeader, codec_par->width);                //width
            PUT_LE16(pbHeader, codec_par->height);               //height
            PUT_LE32(pbHeader, st->avg_frame_rate.num);      //frame rate
            PUT_LE32(pbHeader, st->avg_frame_rate.den);      //time scale(?)
            PUT_LE32(pbHeader, st->nb_index_entries);      //number of frames in file
            PUT_LE32(pbHeader, 0); //unused
            size += 32;
            return size;
        }


        PUT_BE32(pbHeader, nMetaData);
        size += 4;

        PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
        size += nMetaData;
    }

    else if (codStd == STD_VP8) {
        PUT_LE32(pbHeader, MKTAG('D', 'K', 'I', 'F')); //signature 'DKIF'
        PUT_LE16(pbHeader, 0x00);                      //version
        PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
        PUT_LE32(pbHeader, MKTAG('V', 'P', '8', '0')); //codec FourCC
        PUT_LE16(pbHeader, codec_par->width);                //width
        PUT_LE16(pbHeader, codec_par->height);               //height
        PUT_LE32(pbHeader, st->avg_frame_rate.num);      //frame rate
        PUT_LE32(pbHeader, st->avg_frame_rate.den);      //time scale(?)
        PUT_LE32(pbHeader, st->nb_index_entries);      //number of frames in file
        PUT_LE32(pbHeader, 0); //unused
        size += 32;
    }
    else if (codStd == STD_VP9) {
        PUT_LE32(pbHeader, MKTAG('D', 'K', 'I', 'F')); //signature 'DKIF'
        PUT_LE16(pbHeader, 0x00);                      //version
        PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
        PUT_LE32(pbHeader, MKTAG('V', 'P', '9', '0')); //codec FourCC
        PUT_LE16(pbHeader, codec_par->width);                //width
        PUT_LE16(pbHeader, codec_par->height);               //height
        PUT_LE32(pbHeader, st->avg_frame_rate.num);      //frame rate
        PUT_LE32(pbHeader, st->avg_frame_rate.den);      //time scale(?)
        PUT_LE32(pbHeader, st->nb_index_entries);      //number of frames in file
        PUT_LE32(pbHeader, 0); //unused
        size += 32;
    }
    else {
        PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
        size = nMetaData;
    }

    return size;
    /*lint -restore */
}

static Int32 BuildPicHeader(
    Uint8*        pbHeader,
    const CodStd    codStd,
    const AVStream* st,
    const AVPacket* pkt,
    Int32         sizelength
    )
{
    AVCodecParameters *codec_par = st->codecpar;
    Uint8*        pbChunk = pkt->data;
    Int32         size = 0;
    Int32         fourcc;
    Int32         cSlice, nSlice;
    Int32         i, val, offset;
    BOOL            hasStartCode = 0;

    size = 0;
    offset = 0;
    fourcc = codec_par->codec_tag;
    if (!fourcc)
        fourcc = ConvCodecIdToFourcc(codec_par->codec_id);

    if (codStd == STD_VC1) {
        if (!fourcc)
            return -1;

        if (fourcc == MKTAG('W', 'V', 'C', '1') || fourcc == MKTAG('W', 'M', 'V', 'A')) {
            if (pbChunk[0] != 0 || pbChunk[1] != 0 || pbChunk[2] != 1) {
                // check start code as prefix (0x00, 0x00, 0x01)
                pbHeader[0] = 0x00;
                pbHeader[1] = 0x00;
                pbHeader[2] = 0x01;
                pbHeader[3] = 0x0D;	// replace to the correct picture header to indicate as frame

                size += 4;
            }
        }
        else {
            PUT_LE32(pbHeader, pkt->size | ((pkt->flags & AV_PKT_FLAG_KEY) ? 0x80000000 : 0));
            size += 4;
#ifdef RCV_V2
            if (AV_NOPTS_VALUE == pkt->pts) {
                PUT_LE32(pbHeader, 0);
            }
            else {
                PUT_LE32(pbHeader, (int)((double)(pkt->pts/st->time_base.den))); // milli_sec
            }
            size += 4;
#endif
        }
    }
    else if (codStd == STD_HEVC) {
        if(pkt->size < 5)
            return 0;

        VLOG(INFO, "build h265 picHeader line %d  \n", __LINE__);
        if (!(codec_par->extradata_size > 1 && codec_par->extradata && codec_par->extradata[0] == 0x01))
        {
            const Uint8 *pbEnd = pbChunk + 4 - ((intptr_t)pbChunk & 3);

            for (; pbChunk < pbEnd ; pbChunk++) {
                if (pbChunk[0] == 0 && pbChunk[1] == 0 && pbChunk[2] == 1) {
                    hasStartCode = TRUE;
                    break;
                }
            }
        }

        if ((!hasStartCode && codec_par->extradata[0] == 0x01) ||
            (codec_par->extradata_size > 1 && codec_par->extradata && codec_par->extradata[0] == 0x01)) {
            // check sequence metadata if the stream is mov/mo4 file format.
            pbChunk = pkt->data;
            while (offset < pkt->size) {
                if(sizelength == 3) {
                    nSlice = pbChunk[offset] << 16 | pbChunk[offset+1] << 8 | pbChunk[offset+2];

                    pbChunk[offset] = 0x00;
                    pbChunk[offset+1] = 0x00;
                    pbChunk[offset+2] = 0x01;

                    offset += 3;
                }
                else {// sizeLength = 4
                    nSlice = pbChunk[offset] << 24 | pbChunk[offset+1] << 16 | pbChunk[offset+2] << 8 | pbChunk[offset+3];

                    pbChunk[offset] = 0x00;
                    pbChunk[offset+1] = 0x00;
                    pbChunk[offset+2] = 0x00;
                    pbChunk[offset+3] = 0x01;		//replace size to startcode

                    offset += 4;
                }

                switch ((pbChunk[offset]&0x7E)>>1) { /* NAL unit */
                case 39: /* PREFIX SEI */
                case 40: /* SUFFIX SEI */
                case 32: /* VPS */
                case 33: /* SPS */
                case 34: /* PPS */
                    /* check next */
                    break;
                }

                offset += nSlice;
            }
        }
    }
    else if (codStd == STD_RV) {
        int st_size = 0;

        if (!fourcc)
            return -1;
        if (fourcc != MKTAG('R','V','3','0') && fourcc != MKTAG('R','V','4','0')) // RV version 8, 9 , 10
            return -1;


        cSlice = pbChunk[0] + 1;
        nSlice =  pkt->size - 1 - (cSlice * 8);
        size = 20 + (cSlice*8);

        PUT_BE32(pbHeader, nSlice);
        if (AV_NOPTS_VALUE == pkt->pts) {
            PUT_LE32(pbHeader, 0);
        }
        else {
            PUT_LE32(pbHeader, (int)((double)(pkt->pts/st->time_base.den))); // milli_sec
        }

        PUT_BE16(pbHeader, st->nb_index_entries)
        PUT_BE16(pbHeader, 0x02); //Flags
        PUT_BE32(pbHeader, 0x00); //LastPacket
        PUT_BE32(pbHeader, cSlice); //NumSegments
        offset = 1;
        for (i = 0; i < (int) cSlice; i++)
        {
            val = (pbChunk[offset+3] << 24) | (pbChunk[offset+2] << 16) | (pbChunk[offset+1] << 8) | pbChunk[offset];
            PUT_BE32(pbHeader, val); //isValid
            offset += 4;
            val = (pbChunk[offset+3] << 24) | (pbChunk[offset+2] << 16) | (pbChunk[offset+1] << 8) | pbChunk[offset];
            PUT_BE32(pbHeader, val); //Offset
            offset += 4;
        }

        size += st_size;
    }
    else if (codStd == STD_AVC) {
        if(pkt->size < 5)
            return 0;

        VLOG(INFO, "build h264 picHeader line %d  \n", __LINE__);
        if (!(codec_par->extradata_size > 1 && codec_par->extradata && codec_par->extradata[0] == 0x01)) {
            const Uint8 *pbEnd = pbChunk + 4 - ((intptr_t)pbChunk & 3);

            for (; pbChunk < pbEnd ; pbChunk++) {
                if (pbChunk[0] == 0 && pbChunk[1] == 0 && pbChunk[2] == 1) {
                    hasStartCode = 1;
                    break;
                }
            }
        }

        if ((!hasStartCode && codec_par->extradata[0] == 0x01) ||
            (codec_par->extradata_size > 1 && codec_par->extradata && codec_par->extradata[0] == 0x01)) {
            // check sequence metadata if the stream is mov/mo4 file format.
            pbChunk = pkt->data;

            while (offset < pkt->size) {
                if(sizelength == 3) {
                    nSlice = pbChunk[offset] << 16 | pbChunk[offset+1] << 8 | pbChunk[offset+2];
                    pbChunk[offset] = 0x00;
                    pbChunk[offset+1] = 0x00;
                    pbChunk[offset+2] = 0x01;
                    offset += 3;
                }
                else {  // size length = 4
                    nSlice = pbChunk[offset] << 24 | pbChunk[offset+1] << 16 | pbChunk[offset+2] << 8 | pbChunk[offset+3];
                    pbChunk[offset] = 0x00;
                    pbChunk[offset+1] = 0x00;
                    pbChunk[offset+2] = 0x00;
                    pbChunk[offset+3] = 0x01;		//replace size to startcode
                    offset += 4;
                }

                switch (pbChunk[offset]&0x1f) { /* NAL unit */
                case 6: /* SEI */
                case 7: /* SPS */
                case 8: /* PPS */
                case 9: /* AU */
                    /* check next */
                    break;
                }

                offset += nSlice;
            }
        }
    }
    else if(codStd == STD_AVS) {
        const Uint8* pbEnd;

        if(pkt->size < 5)
            return 0;

        pbEnd = pbChunk + 4 - ((intptr_t)pbChunk & 3);

        for (; pbChunk < pbEnd ; pbChunk++) {
            if (pbChunk[0] == 0 && pbChunk[1] == 0 && pbChunk[2] == 1) {
                hasStartCode = 1;
                break;
            }
        }

        if(hasStartCode == 0) {
            pbChunk = pkt->data;

            while (offset < pkt->size) {
                nSlice = pbChunk[offset] << 24 | pbChunk[offset+1] << 16 | pbChunk[offset+2] << 8 | pbChunk[offset+3];

                pbChunk[offset]   = 0x00;
                pbChunk[offset+1] = 0x00;
                pbChunk[offset+2] = 0x00;
                pbChunk[offset+3] = 0x00;		//replace size to startcode
                pbChunk[offset+4] = 0x01;

                offset += 4;

                switch (pbChunk[offset]&0x1f) /* NAL unit */
                {
                case 6: /* SEI */
                case 7: /* SPS */
                case 8: /* PPS */
                case 9: /* AU */
                    /* check next */
                    break;
                }

                offset += nSlice;
            }
        }
    }
    else if (codStd == STD_DIV3 || codStd == STD_VP8 || codStd == STD_VP9 ) {
        PUT_LE32(pbHeader,pkt->size);
        PUT_LE32(pbHeader,0);
        PUT_LE32(pbHeader,0);
        size += 12;
    }
    return size;
}

/*
 *Brief: set ffmpeg lib log-output function
 */
static void av_ffmpeg_log(__attribute__((unused))void *c, int lvl, const char *fmt, va_list vlist)
{

    char logbuf[256] = {0};
    if (lvl > AV_LOG_TRACE)
        return;
    vsnprintf(logbuf, 255, fmt, vlist);
    VLOG(INFO, "[ffmpeg] %s", logbuf);
}

void* BSFeederFrameSize_Create(
    const char* path,
    __attribute__((unused)) CodStd      codecId,
    CodStd*     retCodecId,
    Uint32*   retMp4ClassId,
    Uint32*   retSeqWidth,
    Uint32*   retSeqHeight
    )
{
    /*lint -esym(438, avContext) */
    ReaderContext*      ffmpegReader = NULL;
    AVFormatContext*    avContext   = NULL;
    AVInputFormat*      fmt         = NULL;
    Int32             error;
    Int32             videoIndex;
    Uint32            mp4ClassId;
    Int32             standard;
    AVCodecParameters  *codec_par = NULL;

    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_callback(av_ffmpeg_log);

    if ((avContext=avformat_alloc_context()) == NULL) {
        return NULL;
    }

    VLOG(ERR, "%s:%d file path %s\n",__FILE__, __LINE__,path);

    avContext->flags |= AV_CODEC_FLAG_TRUNCATED;
    avContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if ((error=avformat_open_input(&avContext, path, fmt, NULL))) {
        VLOG(ERR, "%s:%d failed to av_open_input_file error(%d), %s\n",
             __FILE__, __LINE__, error, path);
        goto __failed_to_end;
    }

    if ((error=avformat_find_stream_info(avContext, NULL)) < 0) {
        VLOG(ERR, "%s:%d failed to avformat_find_stream_info. error(%d)\n",
            __FUNCTION__, __LINE__, error);
        goto __failed_to_end;
    }

    av_dump_format(avContext, 0, path, 0);
    videoIndex = av_find_best_stream(avContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    if (videoIndex < 0) {
        VLOG(ERR, "%s:%d failed to av_find_best_stream.\n", __FUNCTION__, __LINE__);
        goto __failed_to_end;
    }

    codec_par = avContext->streams[videoIndex]->codecpar;
    VLOG(INFO, "videoIndex %d , codec_tag  %d  codec_id %d  codec_type %d, extradata-size %d extradata %p, height %d  width %d \n",
        videoIndex ,
        codec_par->codec_tag,
        codec_par->codec_id,
        codec_par->codec_type,
        codec_par->extradata_size,
        codec_par->extradata,
        codec_par->width,
        codec_par->height);

    standard = ConvFOURCCToCodStd(codec_par->codec_tag);
    if (standard == -1)
        standard = ConvCodecIdToCodStd(codec_par->codec_id);

    mp4ClassId = ConvFOURCCToMp4Class(codec_par->codec_tag);
    if (mp4ClassId == -1)
        mp4ClassId = ConvCodecIdToMp4Class(codec_par->codec_id);

    if (standard != STD_MPEG4) {
        mp4ClassId = 0;
    }

    if (retCodecId != NULL) {
        *retCodecId = (CodStd)standard;
    }
    if (retMp4ClassId != NULL) {
        *retMp4ClassId = mp4ClassId;
    }

    if (retSeqWidth != NULL && retSeqHeight != NULL) {
        *retSeqWidth  = codec_par->width;
        *retSeqHeight = codec_par->height;
    }

    if ((ffmpegReader=(ReaderContext*)osal_malloc(sizeof(ReaderContext))) == NULL)
        goto __failed_to_end;

    ffmpegReader->standard      = standard;
    ffmpegReader->mp4ClassId    = mp4ClassId;
    ffmpegReader->avContext     = avContext;
    ffmpegReader->videoIndex    = videoIndex;
    ffmpegReader->isFirstPacket = TRUE;
    ffmpegReader->tempBuffer    = NULL;
    ffmpegReader->tempRdPtr     = 0;
    ffmpegReader->tempWrPtr     = 0;
    memset((void*)&ffmpegReader->superframe, 0x00, sizeof(VP9Superframe));

    VLOG(INFO, "ffmpegReader : standard %d\n", ffmpegReader->standard);
    VLOG(INFO, "ffmpegReader : mp4ClassId %d\n", ffmpegReader->mp4ClassId);
    VLOG(INFO, "ffmpegReader : videoIndex %d\n", ffmpegReader->videoIndex);

    return (void*)ffmpegReader;

__failed_to_end:
    if (avContext) {
        avformat_free_context(avContext);
        avContext = NULL;
    }

    if (ffmpegReader) {
        osal_free(ffmpegReader);
        ffmpegReader = NULL;
    }

    return NULL;
    /*lint +esym(438, avContext) */
}

BOOL BSFeederFrameSize_Destroy(
    void*   feeder
    )
{
    ReaderContext*  ctx = (ReaderContext*)feeder;
    Uint32          i;

    if (ctx == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (ctx->avContext)
        avformat_close_input(&ctx->avContext);

    for (i=0; i<ctx->superframe.nframes; i++) {
        if (ctx->superframe.frames[i] != NULL) {
            free(ctx->superframe.frames[i]);
        }
    }

    osal_free(ctx);

    return TRUE;
}

Int32 BSFeederFrameSize_Act(
    void*       feeder,
    BSChunk*    packet
    )
{
    ReaderContext*      ffmpegReader = (ReaderContext*)feeder;
    AVFormatContext*    avFormatContext = ffmpegReader->avContext;
    AVPacket            avpacket;
    Int32               error;
    Uint8*              seqHeader = NULL;
    Uint8*              picHeader = NULL;
    Uint8*              ptr;
    Int32               seqHeaderSize;
    Int32               picHeaderSize;
    Uint32              vindex, size, thoSeqSize;
    Int32               retSize = -1;
    Int32               packetSize = -1;
    static int          frames = 0;

    if (ffmpegReader->tempBuffer) {
        goto __consume_tempBuffer;
    }

    if (ffmpegReader->standard == STD_VP9) {
        VP9Superframe* superframe = &ffmpegReader->superframe;
        if (superframe->nframes > 0) {
            VLOG(INFO, "get superframe now frames %d\n", superframe->nframes);
            Uint32 index = superframe->currentIndex;
            if (index < superframe->nframes) {
                osal_memcpy(packet->data, superframe->frames[index], superframe->frameSize[index]);
                packet->size = superframe->frameSize[index];
                superframe->currentIndex++;
                return packet->size;
            }
            else {
                Uint32 i;
                for (i=0; i<VP9_MAX_SUBFRAMES; i++) {
                    if (superframe->frames[i] != NULL) {
                        osal_free(superframe->frames[i]);
                    }
                }
                osal_memset((void*)superframe, 0x00, sizeof(VP9Superframe));
            }
        }
    }

    seqHeaderSize = 0;
    picHeaderSize = 0;
    thoSeqSize    = 0;
    av_init_packet(&avpacket);
    while (TRUE) {
        error = av_read_frame(avFormatContext, &avpacket);
        if (error < 0) {
            if (error == AVERROR_EOF || avFormatContext->pb->eof_reached == TRUE) {
                packet->eos = TRUE;
                VLOG(ERR, "%s:%d failed to av_read_frame error(0x%08x)\n",__FUNCTION__, __LINE__, error);
                return 0;
            }
            else {
                VLOG(ERR, "%s:%d failed to av_read_frame error(0x%08x)\n",
                    __FUNCTION__, __LINE__, error);
                goto __end_read;
            }
        }

        if (avpacket.stream_index != ffmpegReader->videoIndex) {
            /*unref audio data*/
            av_packet_unref(&avpacket);
            continue;
        }

        VLOG(INFO, "ffmepg read video frame size %d:  data[0] %u: data[1] %u: data[2] %u: data[3] %u :data[4] %u : data[5] %u  frame %d \n",
                avpacket.size,avpacket.data[0],avpacket.data[1],
                avpacket.data[2],avpacket.data[3],avpacket.data[4],
                avpacket.data[5],++frames);
        break;
    }

    if (avpacket.size == 0)
        return 0;

    if (avpacket.size >= (signed)packet->size )
    {
        VLOG(ERR, "one packet size(%d) is bigger than STREAM_BUF_SIZE(%d)\n", avpacket.size, packet->size);
        return -1;
    }

    osal_memset(packet->data, 0x00, packet->size);

    vindex = ffmpegReader->videoIndex;

    if (ffmpegReader->isFirstPacket) {
        AVCodecParameters  *codec_par = NULL;

        codec_par = ffmpegReader->avContext->streams[vindex]->codecpar;
        seqHeader = (Uint8*)osal_malloc(codec_par->extradata_size + 1024);
        VLOG(INFO, "feeder, vps+sps+pps+sei size %d \n", codec_par->extradata_size);

        if (seqHeader == NULL) {
            goto __end_read;
        }
        osal_memset((void*)seqHeader, 0x00, codec_par->extradata_size + 1024);

        seqHeaderSize = BuildSeqHeader(seqHeader, (CodStd)ffmpegReader->standard,
                                       ffmpegReader->avContext->streams[vindex], &retSize);
        if (seqHeaderSize < 0) {
            VLOG(ERR, "%s:%d Can't make sequence header!\n", __FUNCTION__, __LINE__);
            packetSize = -1;
            goto __end_read;
        }

        ffmpegReader->isFirstPacket = FALSE;
    }

    picHeader     = (Uint8*)osal_malloc(1024);
    picHeaderSize = BuildPicHeader(picHeader, (CodStd)ffmpegReader->standard,
                                   ffmpegReader->avContext->streams[vindex], &avpacket, 0);
    if (picHeaderSize < 0) {
        VLOG(ERR, "%s:%d failed to build picture header\n", __FUNCTION__, __LINE__);
        goto __end_read;
    }

    VLOG(INFO, "feeder picHeaderSize %d, seqHeaderSize %d \n", picHeaderSize, seqHeaderSize);
    ptr  = avpacket.data;
    size = avpacket.size;
    switch (ffmpegReader->standard) {
    case STD_RV:
        if (seqHeaderSize)
            osal_memcpy((char*)packet->data, seqHeader, seqHeaderSize);

        if (picHeaderSize)
            osal_memcpy((char*)packet->data+seqHeaderSize, picHeader, picHeaderSize);

        if (ffmpegReader->standard == STD_RV) {
            int cSlice = ptr[0] + 1;
            int nSlice = avpacket.size - 1 - (cSlice*8);
            ptr += (1+(cSlice*8));
            size = nSlice;
        }

        osal_memcpy((char*)packet->data+seqHeaderSize+picHeaderSize, ptr, size);
        packetSize = seqHeaderSize + picHeaderSize + size;
        break;
    case STD_THO:
    case STD_VP3:
        VLOG(INFO, "No support now ...\n");
        break;
    case STD_VP9:
        packet->size    = size;
        osal_memcpy((char *)packet->data, ptr, size);
        packetSize      = size;
        break;
    default:
        if (picHeaderSize)
            osal_memcpy((char*)packet->data, picHeader, picHeaderSize);


        osal_memcpy((char*)packet->data+picHeaderSize, ptr, size);
        packetSize = picHeaderSize + size;
        VLOG(INFO, "feeder size packetSize %d\n", packetSize);
        break;
    }

    if (avFormatContext->pb->eof_reached && avFormatContext->streams[vindex]->last_in_packet_buffer == NULL) {
        packet->eos = TRUE;
    }

    // Sequence header data should be only one chunk data unit.
    // In case of RV, 1st chunk should be Sequence header + 1st frame.
    if (ffmpegReader->standard != STD_VP9 && ffmpegReader->standard != STD_RV) {
        if (seqHeaderSize > 0) {
            ffmpegReader->tempBuffer = (Uint8*)osal_malloc(packetSize);
            ffmpegReader->tempWrPtr  = packetSize;
            osal_memcpy(ffmpegReader->tempBuffer, (Uint8*)(packet->data), packetSize);
            osal_memcpy(packet->data, seqHeader, seqHeaderSize);
            packetSize = seqHeaderSize;
            goto __end_read;
        }
    }
    else if (ffmpegReader->standard == STD_VP9) {
        Uint8*  pData = (Uint8*)packet->data;
        Uint32  lastIndex = packet->size - 1;

        if (IS_VP9_SUPERFRAME(pData[lastIndex]) == TRUE) {
            VP9Superframe*  superframe = &ffmpegReader->superframe;
            if (VP9ParseSuperframe(pData, packet->size, superframe) == TRUE) {
                osal_memcpy(packet->data, superframe->frames[0], superframe->frameSize[0]);
                packet->size = superframe->frameSize[0];
                packetSize   = packet->size;
                superframe->currentIndex++;
            }
        }
    }

__end_read:
    av_packet_unref(&avpacket);

    if (picHeader)
        osal_free(picHeader);
    if (seqHeader)
        osal_free(seqHeader);

    return packetSize;

__consume_tempBuffer:
    if (ffmpegReader->tempBuffer != NULL) {
        VLOG(INFO, "%s:%d __consume_tempBuffer \n", __FUNCTION__, __LINE__);
        osal_memcpy(packet->data, ffmpegReader->tempBuffer, ffmpegReader->tempWrPtr);
        packetSize = ffmpegReader->tempWrPtr;
        osal_free(ffmpegReader->tempBuffer);
        ffmpegReader->tempBuffer = NULL;
        ffmpegReader->tempWrPtr  = 0;
        ffmpegReader->tempRdPtr  = 0;
    }

    return packetSize;
}

BOOL BSFeederFrameSize_Rewind(
    void* feeder
    )
{
    ReaderContext*      ffmpegReader = (ReaderContext*)feeder;
    AVFormatContext*    avFormatContext = ffmpegReader->avContext;
    Int32               ret;

    if ((ret=av_seek_frame(avFormatContext, ffmpegReader->videoIndex, 0, AVSEEK_FLAG_BYTE /*AVSEEK_FLAG_BACKWARD*/)) < 0) {
        VLOG(ERR, "%s:%d Failed to av_seek_frame:(ret:%d)\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    return TRUE;
}

int BSFeederFrameSize_Seek(
    void* feeder,
    uint64_t ts_time
    )
{
    ReaderContext*      ffmpegReader = (ReaderContext*)feeder;
    AVFormatContext*    avFormatContext = ffmpegReader->avContext;
    Int32               ret = 0;

    if ((ret = av_seek_frame(avFormatContext, ffmpegReader->videoIndex, ts_time, AVSEEK_FLAG_ANY)) < 0) {
        VLOG(ERR, "%s:%d Failed to av_seek_frame:(ret:%d)\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    return ret;
}

#else
void* BSFeederFrameSize_Create(
    const char* path,
    CodStd  codecId,
    CodStd* retCodecId,
    Uint32* retMp4ClassId,
    Uint32* retSeqWidth,
    Uint32* retSeqHeight
    )
{
    UNREFERENCED_PARAMETER(path);
    UNREFERENCED_PARAMETER(codecId);
    UNREFERENCED_PARAMETER(retCodecId);
    UNREFERENCED_PARAMETER(retMp4ClassId);
    UNREFERENCED_PARAMETER(retSeqWidth);
    UNREFERENCED_PARAMETER(retSeqHeight);

    return NULL;
}

BOOL BSFeederFrameSize_Destroy(
    void*   feeder
    )
{
    UNREFERENCED_PARAMETER(feeder);
    return TRUE;
}

Int32 BSFeederFrameSize_Act(
    void*       feeder,
    BSChunk*    packet
    )
{
    UNREFERENCED_PARAMETER(feeder);
    UNREFERENCED_PARAMETER(packet);

    return 0;
}

BOOL BSFeederFrameSize_Rewind(
    void* feeder
    )
{
    UNREFERENCED_PARAMETER(feeder);
    VLOG(ERR, "PLEASE PORT THIS %s ON YOUR ANDROID SYSTEM\n", __FUNCTION__);

    return FALSE;

}

int BSFeederFrameSize_Seek(
    void* feeder,
    uint64_t ts_time
    )
{

    UNREFERENCED_PARAMETER(feeder);
    UNREFERENCED_PARAMETER(ts_time);
    return 0;
}

#endif /* SUPPORT_FFMPEG_DEMUX */

