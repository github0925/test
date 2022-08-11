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

#ifndef _OMX_VPUDEC_COMPONENT_H_
#define _OMX_VPUDEC_COMPONENT_H_

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <omx_base_filter.h>
#include <omx_base_video_port.h>

#include "OMX_VPU_Video.h"
#ifdef SUPPORT_CM_OMX_12
#else
#include <OMX_VideoExt.h>
#endif
#include "vpuconfig.h"
#include "vpuapi.h"
#include "vpuapifunc.h"
#include "omx_dump.h"
#include "omx_utils.h"
#include "omx_hdr_info_wrapper.h"
#include "omx_color_aspect_wrapper.h"
// #define USE_IFRAME_SEARCH_FOR_1STFRAME

#define SUPPORT_USE_RV_TR_AS_TIMESTAMP

//#define SUPPORT_ONE_BUFFER_CONTAIN_MULTIPLE_FRAMES

#define OMX_INPUT_BUFFER_FEEDED_BY_FFMPEG_AVPACKET

#ifdef SUPPORT_CM_OMX_12
#define SUPPORT_NALU_FORMAT_BY_OMX
#endif

#define MAX_DEC_BITSTREAM_BUFFER_COUNT      (16)
#define MAX_PPU_SRC_NUM 2
#define MAX_DECODER_COMPONENTS 16

#define IS_STATE_EMPTYTHISBUFFER(pInputBuffer)      (pInputBuffer != NULL)
#define IS_STATE_FILLTHISBUFFER(pOutBuffer)      (pOutBuffer != NULL)
typedef struct vpu_frame_t
{
    int format;
    int index;
    int stride;
    vpu_buffer_t vbY;
    vpu_buffer_t vbCb;
    vpu_buffer_t vbCr;
    vpu_buffer_t vbMvCol;
} vpu_frame_t;


typedef enum {
    OMX_BUFFER_OWNER_NOBODY = 0,
    OMX_BUFFER_OWNER_CLIENT,
    OMX_BUFERR_OWNER_COMPONENT,
} omx_buffer_owner;

typedef struct {
    omx_buffer_owner owner;
    int picType;
    OMX_TICKS nInputTimeStamp;
#ifdef SUPPORT_USE_RV_TR_AS_TIMESTAMP
    int rvTrB;
#endif
} omx_display_flag;


typedef struct {
    DecOutputInfo* buffer;
    int  size;
    int  count;
    int  front;
    int  rear;
} omx_outinfo_queue_item_t;


typedef struct {
    OMX_BUFFERHEADERTYPE* buffer;
    int  size;
    int  count;
    int  front;
    int  rear;
    pthread_mutex_t mutex;
} omx_bufferheader_queue_item_t;


typedef struct omx_timestamp_correction_smooth_t
{
    OMX_TICKS LeftTimeUs;
    OMX_TICKS AnchorTimeUs;
    OMX_TICKS TimePerFrameUs;
} omx_timestamp_correction_smooth_t;

typedef struct omx_timestamp_correction_t
{
    OMX_BOOL mTimeStampCalcuteMode;
    OMX_BOOL mTimeStampDirectMode;
    OMX_TICKS mDurationPerFrameUs;
    OMX_TICKS mFrameNumber;
    OMX_TICKS mPreviousTimeStampUs;
    OMX_TICKS mAnchorTimeUs;

    omx_timestamp_correction_smooth_t mSmoothTime;
} omx_timestamp_correction_t;

typedef struct omx_usebuffer_display_info_t
{
    OMX_BOOL bToBeDisplay;
    OMX_U32 dispFlagIdx;
    FrameBuffer dispFrame;
} omx_usebuffer_display_info_t;

typedef struct vpu_dec_context_t
{
    DecHandle handle;
    DecOpenParam decOP;
    ScalerInfo scalerInfo;
    DecInitialInfo initialInfo;
    DecOutputInfo dispOutputInfo;
    DecParam decParam;
    SecAxiUse secAxiUse;
    MaverickCacheConfig decCacheConfig;
    DRAMConfig dramCfg;
    FrameBufferAllocInfo fbAllocInfo;
    FrameBuffer fbPPU[MAX_PPU_SRC_NUM];
    FrameBuffer fbUser[MAX_REG_FRAME];
    vpu_buffer_t vbUserFb[MAX_REG_FRAME];   // consist of all buffers context information and not for freeing
    vpu_buffer_t vbAllocFb[MAX_REG_FRAME];  // buffer context that is allocated by OMX_AllocateBuffer and OMX_UseBuffer
    vpu_buffer_t vbDPB[MAX_REG_FRAME];      // buffer context that is allocated by OmxAllocateBuffer for DPB
#ifdef SUPPORT_PARALLEL
    vpu_buffer_t vbMvCol;
#endif
    vpu_buffer_t vbStream[MAX_DEC_BITSTREAM_BUFFER_COUNT];
    int fbWidth;
    int fbHeight;
    int fbStride;
    int fbFormat;
    int rotStride;
    int bsBufferCount;
    int bsQueueIndex;
    int regFbCount;
    int decFbCount; // DPB or Compress
    int dispFbCount; // can be scaler or WTL
    int seqInited;
    VpuRect rcPrevDisp;
    int ppu;
    int ppuIdx;
    int frameIdx;
    int dispOutIdx;
    int decodefinish;
    int int_reason;
    int totalNumofErrMbs;
    int needFrameBufCount;
    int instIdx;
    int coreIdx;
    int mapType;
    int productId;
    TiledMapConfig mapCfg;
    int chunkReuseRequired; //reuse the vbStream input Buffer, so don't re-write it
    int prevConsumeBytes;
    int curConsumedByte;
    int frameRate;
    int inputDumpIdx;
    int outputDumpIdx;
    int interResChanged;    // for VP9 inter-resolution change
    int fbcCurFrameIdx;     // for VP9 inter-resolution change
    int supportComamandQueue;
    int retPicRunCmd;
    int tryCount;
    double last_return_ts;
#ifdef FILE_DEC_DUMP
    char dumpInputFn[MAX_FN_LEN];
    char dumpOutputFn[MAX_FN_LEN];
#endif
} vpu_dec_context_t;

typedef struct dec_config_t
{
    int bitFormat;
    int rotAngle;
    int mirDir;
    int outNum;
    int checkeos;
    int mp4DeblkEnable;
    int iframeSearchEnable;
    int dynamicAllocEnable;
    int reorder;
    int mpeg4Class;
    int mapType;
    int seqInitMode;
    int dispMixerDirect;
    int wtlEnable;
    int wtlMode;    // 1: frame 2: field

    int FrameCacheBypass   ;
    int FrameCacheBurst    ;
    int FrameCacheMerge    ;
    int FrameCacheWayShape ;
} dec_config_t;

typedef union VideoParam
{
    OMX_VIDEO_PARAM_H263TYPE h263;
    OMX_VIDEO_PARAM_MPEG4TYPE mpeg4;
    OMX_VIDEO_PARAM_MPEG2TYPE mpeg2;
    OMX_VIDEO_PARAM_AVCTYPE avc;
    OMX_VIDEO_PARAM_RVTYPE rv;
    OMX_VIDEO_PARAM_WMVTYPE wmv;
    OMX_VIDEO_PARAM_MSMPEGTYPE msmpeg;
    OMX_VIDEO_PARAM_VP8TYPE vp8;
} VideoParam;



DERIVEDCLASS(omx_vpudec_component_PortType, omx_base_video_PortType)
#define omx_vpudec_component_PortType_FIELDS omx_base_video_PortType_FIELDS \
    OMX_CONFIG_RECTTYPE omxConfigCrop; \
    OMX_CONFIG_ROTATIONTYPE omxConfigRotate; \
    OMX_CONFIG_MIRRORTYPE omxConfigMirror; \
    OMX_CONFIG_SCALEFACTORTYPE omxConfigScale; \
    OMX_CONFIG_POINTTYPE omxConfigOutputPosition; \
    OMX_NALSTREAMFORMATTYPE nalUFormatSelect; \
    OMX_U32 nTempBufferCountActual; \
    OMX_BOOL bAllocateBuffer;
ENDCLASS(omx_vpudec_component_PortType)

/** Video Decoder component private structure.
 */
DERIVEDCLASS(omx_vpudec_component_PrivateType, omx_base_filter_PrivateType)
#define omx_vpudec_component_PrivateType_FIELDS omx_base_filter_PrivateType_FIELDS \
    VideoParam codParam; \
    /** @param vpuReady boolean flag that is true when the video coded has been initialized */ \
    OMX_BOOL vpuReady;  \
    /** @param video_coding_type Field that indicate the supported video format of video decoder */ \
    OMX_U32 video_coding_type;   \
    /** @param extradata pointer to extradata*/ \
    OMX_U8* picHeader; \
    OMX_U8* seqHeader; \
    /** @param extradata_size extradata size*/ \
    OMX_S32 seqHeaderSize; \
    OMX_S32 picHeaderSize; \
    vpu_dec_context_t vpu; \
    /** @param useNativeBuffer that indicate ANDROID native buffer use */ \
    OMX_BOOL useNativeBuffer; \
    OMX_BOOL bThumbnailMode; \
    /** @param count the native buffer number  */ \
    OMX_BOOL bUseOmxInputBufferAsDecBsBuffer; \
    OMX_BOOL bSeqChangeDetected; \
    OMX_BOOL portSettingChangeRequest; \
    OMX_BOOL bIsOutputEOSReached; \
    OMX_BOOL bIsTimeStampReorder; \
    OMX_BOOL bSkipNoneKeyframeDisplay; \
    OMX_BOOL bSupportNaluFormat; \
    OMX_BOOL bSizeLengthDetected; \
    OMX_U32 portSettingCount;   \
    tsem_t port_setting_change_tsem; \
    tsem_t disp_Buf_full_tsem; \
    omx_bufferheader_queue_item_t *in_bufferheader_queue; \
    omx_timestamp_correction_t omx_ts_correction; \
    omx_display_flag omx_display_flags[MAX_REG_FRAME]; \
    pthread_mutex_t display_flag_mutex; \
    pthread_mutex_t vpu_flush_mutex; \
    OMX_TICKS lastTimeStamp; \
    OMX_TICKS lastInputStamp; \
    /** @param adaptivePlayback */  \
    OMX_BOOL    bAdaptiveEnable; \
    OMX_U32     nMaxFrameWidth; \
    OMX_U32     nMaxFrameHeight; \
    /* @param color aspect*/ \
    /* color aspects passed from the framework*/ \
    ColorAspectsWrapper mDefaultColorAspects; \
    /* color aspects parsed from the bitstream */ \
    ColorAspectsWrapper mBitstreamColorAspects; \
    /* final color aspects after combining the above two aspects*/ \
    ColorAspectsWrapper mFinalColorAspects; \
    /* @param hdr static info*/ \
    /* hdr static info for current stream*/ \
    HdrInfoWrapper mHdrStaticInfo; \
    OMX_U32     nUsage;
ENDCLASS(omx_vpudec_component_PrivateType)


/* Component private entry points declaration */
OMX_ERRORTYPE omx_vpudec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName);
OMX_ERRORTYPE omx_vpudec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_vpudec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_vpudec_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_vpudec_component_MessageHandler(OMX_COMPONENTTYPE*, internalRequestMessageType*);
void* omx_vpudec_component_BufferMgmtFunction(void* param);

void omx_vpudec_component_BufferMgmtCallback(
    OMX_COMPONENTTYPE *openmaxStandComp,
    OMX_BUFFERHEADERTYPE* inputbuffer,
    OMX_BUFFERHEADERTYPE* outputbuffer);

OMX_ERRORTYPE omx_vpudec_component_GetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure);
OMX_ERRORTYPE omx_vpudec_component_SetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure);
OMX_ERRORTYPE omx_vpudec_component_ComponentRoleEnum(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_U8 *cRole,
    OMX_IN OMX_U32 nIndex);
OMX_ERRORTYPE omx_vpudec_component_SetConfig(
    OMX_HANDLETYPE hComponent,
    OMX_INDEXTYPE nIndex,
    OMX_PTR pComponentConfigStructure);
OMX_ERRORTYPE omx_vpudec_component_GetExtensionIndex(
    OMX_HANDLETYPE hComponent,
    OMX_STRING cParameterName,
    OMX_INDEXTYPE* pIndexType);
OMX_ERRORTYPE omx_vpudec_component_GetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure);
OMX_ERRORTYPE omx_vpudec_component_SendBufferFunction(
    omx_base_PortType *openmaxStandPort,
    OMX_BUFFERHEADERTYPE* pBuffer);
OMX_ERRORTYPE omx_vpudec_component_OutPort_ReturnBufferFunction(
    omx_base_PortType* openmaxStandPort,
    OMX_BUFFERHEADERTYPE* pBuffer);
OMX_ERRORTYPE omx_videodec_component_AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes);
OMX_ERRORTYPE omx_videodec_component_UseBuffer(
    OMX_HANDLETYPE hComponent,
    OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_U32 nPortIndex,
    OMX_PTR pAppPrivate,
    OMX_U32 nSizeBytes,
    OMX_U8* pBuffer);
OMX_ERRORTYPE omx_videodec_component_FreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);
#endif
