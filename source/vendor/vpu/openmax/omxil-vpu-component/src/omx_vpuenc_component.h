//--=========================================================================--
//  This implements some useful common functionalities
//  for handling the register files used in Bellagio
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2015  CHIPS&MEDIA INC.
//            (C) CPPYRIGHT 2020 Semidrive Technology Ltd.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#ifndef _OMX_VPUENC_COMPONENT_H_
#define _OMX_VPUENC_COMPONENT_H_

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <stdlib.h>
#include <string.h>
#include <omx_base_filter.h>
#ifdef ANDROID
#include <MetadataBufferType.h>
#include <hardware/gralloc.h>
#include <nativebase/nativebase.h>
#endif

#include "vpuconfig.h"
#include "vpuapi.h"
#include "vpuapifunc.h"
#include "omx_dump.h"

#ifdef ANDROID
/*
 * frameworks/native/include/media/hardware

 * struct VideoGrallocMetadata {
      MetadataBufferType eType;               // must be kMetadataBufferTypeGrallocSource
 * #ifdef OMX_ANDROID_COMPILE_AS_32BIT_ON_64BIT_PLATFORMS
 *    OMX_PTR pHandle;
 * #else
 *    buffer_handle_t pHandle;
 * #endif
 * };


 * struct VideoNativeMetadata {
      MetadataBufferType eType;               // must be kMetadataBufferTypeANWBuffer
 * #ifdef OMX_ANDROID_COMPILE_AS_32BIT_ON_64BIT_PLATFORMS
 *    OMX_PTR pBuffer;
 * #else
 *    struct ANativeWindowBuffer* pBuffer;
 * #endif
 *    int nFenceFd;                           // -1 if unused
 * };
*/

typedef struct encoder_video_gralloc_metadata_t{
    MetadataBufferType metadata_buffer_type;
    buffer_handle_t meta_handle;
} encoder_video_gralloc_metadata_t;

typedef struct encoder_video_native_metadata_t{
    MetadataBufferType metadata_buffer_type;
    struct ANativeWindowBuffer* pBuffer;
    int nFenceFd;                           // -1 if unused
} encoder_video_native_metadata_t;

typedef MetadataBufferType META_DATA_BUFFER_TYPE;
typedef buffer_handle_t BUFFER_HANDLE;
#else
// for the env without MetadataBufferType and buffer_handle_t
typedef OMX_U8  META_DATA_BUFFER_TYPE;
typedef OMX_U32 BUFFER_HANDLE;
#endif

#define MAX_ENC_BITSTREAM_BUFFER_COUNT (16)
#define MAX_ENCODER_COMPONENTS 16

typedef struct vpu_enc_context_t {
    EncHandle   handle;
    EncOpenParam    encOP;
    EncInitialInfo  initialInfo;
    EncOutputInfo   outputInfo;
    EncParam    encParam;
    SecAxiUse   secAxiUse;
    EncHeaderParam  encHeaderParam;
    BYTE *pHeaderData;
    MaverickCacheConfig encCacheConfig;
    OMX_BYTE privateYuv;
    int     srcFrameHeight;
    int     srcFrameStride;
    int     srcFrameFormat;
    int     srcFrameBufSize;
    int     framebufStride;
    int     framebufWidth;
    int     framebufHeight;
    int     seqInited;
    int     regFrameBufCount;
    int     frameIdx;
    int     instIdx;
    int     coreIdx;
    int     mapType;
    int     frameBufSize;
    int     outputDumpIdx;
    int     inputDumpIdx;
    TiledMapConfig mapCfg;
    FrameBuffer     fbSrc[MAX_REG_FRAME];
    FrameBuffer     fbRecon[MAX_REG_FRAME];             // for WAVE420
    vpu_buffer_t    vbReconFrameBuf[MAX_REG_FRAME];     // for WAVE420
    FrameBufferAllocInfo fbAllocInfo;
    vpu_buffer_t vbSrcFb[MAX_REG_FRAME];
    int productId;
    int fieldDone;
    int activePPSIdx;
    vpu_buffer_t vbStream[MAX_ENC_BITSTREAM_BUFFER_COUNT];
    double last_return_ts;
#ifdef FILE_ENC_DUMP
    char dumpInputFn[MAX_FN_LEN];
    char dumpOutputFn[MAX_FN_LEN];
#endif
} vpu_enc_context_t;

typedef struct enc_config_t
{
    int stdMode;
    int picWidth;
    int picHeight;
    int kbps;
    int rotAngle;
    int mirDir;
    int useRot;
    int qpReport;
    int saveEncHeader;
    int dynamicAllocEnable;
    int ringBufferEnable;
    int rcIntraQp;
    int picQp;
    int outNum;
    int dispMixerDirect;
    int mapType;
    int linear2TiledEnable;
    // 2D cache option
    int FrameCacheBypass   ;
    int FrameCacheBurst    ;
    int FrameCacheMerge    ;
    int FrameCacheWayShape ;

    int lineBufIntEn;
    int subFrameSyncEn;

} enc_config_t;

typedef struct VideoEncParam {
    union{
    OMX_VIDEO_PARAM_MPEG4TYPE mpeg4;
    OMX_VIDEO_PARAM_AVCTYPE avc;
    OMX_VIDEO_PARAM_H263TYPE h263;
    };

    OMX_VIDEO_PARAM_BITRATETYPE bitrate;
    OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE errorrection;
    OMX_VIDEO_PARAM_QUANTIZATIONTYPE quantization;
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE intraRefresh;

    OMX_VIDEO_CONFIG_BITRATETYPE videoConfigBitrateType;
    OMX_CONFIG_FRAMERATETYPE configFramerateType;
    OMX_CONFIG_INTRAREFRESHVOPTYPE configIntraRefreshVopType;
    OMX_VIDEO_CONFIG_AVCINTRAPERIOD videoConfigAvcIntraPeriod;
} VideoEncParam;

#define VIDEO_ENC_NUM 3

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




/** Video Encoder component private structure.
  */
DERIVEDCLASS(omx_vpuenc_component_PrivateType, omx_base_filter_PrivateType)
#define omx_vpuenc_component_PrivateType_FIELDS omx_base_filter_PrivateType_FIELDS \
    vpu_enc_context_t vpu; \
    VideoEncParam codParam; \
    OMX_BOOL vpuReady;  \
        /** @param video_encoding_type Field that indicate the supported video format of video encoder */ \
    OMX_U32 video_encoding_type; \
    OMX_U32 idrPeriod; \
    OMX_BOOL requestChangeBitrate; \
    OMX_BOOL requestChangeFramerate; \
    OMX_BOOL requestChangeGopSize; \
    OMX_BOOL requestIFrame; \
    OMX_BOOL storeMetaDataInOutputBuffer; \
    OMX_BOOL storeMetaDataInInputBuffer; \
    OMX_BOOL storeMetaDataInDrmBuffer; \
    OMX_BOOL bUseOmxAllocateBufferOfInputPort; \
    OMX_BOOL bUseOmxAllocateBufferOfOutPort; \
    OMX_BOOL bFormatConversion; \
    OMX_BOOL portSettingChangeRequest; \
    OMX_BOOL bIsOutputEOSReached;\
    OMX_U32  nTempBufferCountActual; \
    BUFFER_HANDLE         buffer_handle; \
    META_DATA_BUFFER_TYPE metadata_buffer_type;
ENDCLASS(omx_vpuenc_component_PrivateType)



/* Component private entry points enclaration */
OMX_ERRORTYPE omx_vpuenc_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName);
OMX_ERRORTYPE omx_vpuenc_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_vpuenc_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_vpuenc_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_vpuenc_component_MessageHandler(OMX_COMPONENTTYPE*,internalRequestMessageType*);
void* omx_vpuenc_component_BufferMgmtFunction (
    void* param);

void omx_vpuenc_component_BufferMgmtCallback(
  OMX_COMPONENTTYPE *openmaxStandComp,
  OMX_BUFFERHEADERTYPE* inputbuffer,
  OMX_BUFFERHEADERTYPE* outputbuffer);

OMX_ERRORTYPE omx_vpuenc_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_vpuenc_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_vpuenc_component_ComponentRoleEnum(
  OMX_IN OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_U8 *cRole,
  OMX_IN OMX_U32 nIndex);

#ifdef ANDROID
OMX_ERRORTYPE omx_vpuenc_component_GetExtensionIndex(
    OMX_HANDLETYPE hComponent,
    OMX_STRING cParameterName,
    OMX_INDEXTYPE* pIndexType);
#endif

OMX_ERRORTYPE omx_vpuenc_component_SetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_vpuenc_component_GetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure);


OMX_ERRORTYPE omx_videoenc_component_AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes);

OMX_ERRORTYPE omx_videoenc_component_UseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_U8* pBuffer);

OMX_ERRORTYPE omx_videoenc_component_FreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);
#endif
