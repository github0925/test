/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

#ifndef _OMX_UTILS_H_
#define _OMX_UTILS_H_

#include <OMX_Types.h>
#include <OMX_Video.h>

/* decoder names and roles */
#define VIDEO_DEC_BASE_NAME "OMX.vpu.video_decoder"
#define VIDEO_DEC_MPEG2_NAME  "OMX.vpu.video_decoder.mpeg2"
#define VIDEO_DEC_MPEG2_ROLE  "video_decoder.mpeg2"
#define VIDEO_DEC_MPEG4_NAME "OMX.vpu.video_decoder.mpeg4"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_DEC_MPEG4_ROLE OMX_ROLE_VIDEO_DECODER_MPEG4
#else
#define VIDEO_DEC_MPEG4_ROLE "video_decoder.mpeg4"
#endif
#define VIDEO_DEC_H264_NAME "OMX.vpu.video_decoder.avc"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_DEC_H264_ROLE OMX_ROLE_VIDEO_DECODER_AVC
#else
#define VIDEO_DEC_H264_ROLE "video_decoder.avc"
#endif
#define VIDEO_DEC_RV_NAME   "OMX.vpu.video_decoder.rv"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_DEC_RV_ROLE   OMX_ROLE_VIDEO_DECODER_RV
#else
#define VIDEO_DEC_RV_ROLE   "video_decoder.rv"
#endif
#define VIDEO_DEC_WMV_NAME    "OMX.vpu.video_decoder.wmv"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_DEC_WMV_ROLE    OMX_ROLE_VIDEO_DECODER_WMV
#else
#define VIDEO_DEC_WMV_ROLE    "video_decoder.wmv"
#endif
#define VIDEO_DEC_H263_NAME   "OMX.vpu.video_decoder.h263"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_DEC_H263_ROLE   OMX_ROLE_VIDEO_DECODER_H263
#else
#define VIDEO_DEC_H263_ROLE   "video_decoder.h263"
#endif
#define VIDEO_DEC_MSMPEG_NAME "OMX.vpu.video_decoder.msmpeg"
#define VIDEO_DEC_MSMPEG_ROLE "video_decoder.msmpeg"
#define VIDEO_DEC_AVS_NAME "OMX.vpu.video_decoder.avs"
#define VIDEO_DEC_AVS_ROLE "video_decoder.avs"
#define VIDEO_DEC_VP8_NAME "OMX.vpu.video_decoder.vp8"
#define VIDEO_DEC_VP8_ROLE "video_decoder.vp8"
#define VIDEO_DEC_THO_NAME "OMX.vpu.video_decoder.tho"
#define VIDEO_DEC_THO_ROLE "video_decoder.tho"
#define VIDEO_DEC_JPG_NAME "OMX.vpu.image_decoder.jpg"
#define VIDEO_DEC_JPG_ROLE "image_decoder.jpg"
#define VIDEO_DEC_VC1_NAME  "OMX.vpu.video_decoder.vc1"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_DEC_VC1_ROLE  OMX_ROLE_VIDEO_DECODER_VC1
#else
#define VIDEO_DEC_VC1_ROLE  "video_decoder.vc1"
#endif
#define VIDEO_DEC_HEVC_NAME "OMX.vpu.video_decoder.hevc"
#define VIDEO_DEC_HEVC_ROLE "video_decoder.hevc"
#define VIDEO_DEC_VP9_NAME "OMX.vpu.video_decoder.vp9"
#define VIDEO_DEC_VP9_ROLE "video_decoder.vp9"
#define VIDEO_DEC_DIVX_NAME "OMX.vpu.video_decoder.divx"
#define VIDEO_DEC_DIVX_ROLE "video_decoder.divx"

/* encoder names and roles */
#define VIDEO_ENC_BASE_NAME "OMX.vpu.video_encoder"

#define VIDEO_ENC_H263_NAME "OMX.vpu.video_encoder.h263"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_ENC_H263_ROLE OMX_ROLE_VIDEO_ENCODER_H263
#else
#define VIDEO_ENC_H263_ROLE "video_encoder.h263"
#endif

#define VIDEO_ENC_MPEG4_NAME "OMX.vpu.video_encoder.mpeg4"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_ENC_MPEG4_ROLE OMX_ROLE_VIDEO_ENCODER_MPEG4
#else
#define VIDEO_ENC_MPEG4_ROLE "video_encoder.mpeg4"
#endif

#define VIDEO_ENC_AVC_NAME "OMX.vpu.video_encoder.avc"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_ENC_AVC_ROLE OMX_ROLE_VIDEO_ENCODER_AVC
#else
#define VIDEO_ENC_AVC_ROLE "video_encoder.avc"
#endif

#define VIDEO_ENC_HEVC_NAME "OMX.vpu.video_encoder.hevc"
#ifdef SUPPORT_CM_OMX_12
#define VIDEO_ENC_HEVC_ROLE OMX_ROLE_VIDEO_ENCODER_HEVC
#else
#define VIDEO_ENC_HEVC_ROLE "video_encoder.hevc"
#endif

#define VIDEO_ENC_JPG_NAME "OMX.vpu.image_encoder.jpg"
#define VIDEO_ENC_JPG_ROLE "image_encoder.jpg"


typedef struct ComponentMapEntry {
    OMX_STRING cComponentName;
    OMX_VIDEO_CODINGTYPE cVideoCodingType;
    OMX_STRING cRole;
} ComponentMapEntry;

OMX_ERRORTYPE GetVideoCodingTypeByName(
    const OMX_STRING cComponentName, OMX_VIDEO_CODINGTYPE *codingType, OMX_BOOL isEncoder);

OMX_ERRORTYPE GetRoleByVideoCodingType(
    OMX_VIDEO_CODINGTYPE codingType, OMX_U8* cRole, OMX_BOOL isEncoder);

OMX_ERRORTYPE GetComponentRoleByIndex(
    OMX_U8 index, OMX_STRING cRole, OMX_BOOL isEncoder);

OMX_ERRORTYPE GetComponentNameByIndex(
    OMX_U8 index, OMX_STRING cComponentName, OMX_BOOL isEncoder);

OMX_U8 GetComponentCount(
    OMX_BOOL isEncoder);

double GetNowMs();

#endif // _OMX_UTILS_H_
