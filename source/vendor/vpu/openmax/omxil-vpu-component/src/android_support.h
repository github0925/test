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


#ifndef _OMX_VPU_ANDROID_SUPPORT_H
#define _OMX_VPU_ANDROID_SUPPORT_H

#undef LOG_TAG
#define LOG_TAG "VPUOMX"

#ifdef __cplusplus
extern "C" {
#endif
#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Index.h>
#include <OMX_VPU_Index.h>

#define VPU_CODA988_IDX 0
#define VPU_WAVE412_IDX 1
#define PROP_DISABLE_TILE "debug.vpu.disable.tile"
#define PROP_DISABLE_FBDC "debug.vpu.disable.ifbc"

typedef enum OMX_VPU_ANDORID_INDEXTYPE {
#define STR_INDEX_PARAM_ENABLE_ANDROID_NATIVE_BUFFER "OMX.google.android.index.enableAndroidNativeBuffers"
        OMX_IndexParamEnableAndroidBuffers = OMX_IndexParamVideoAVS + 1,  /**< reference: Android Native Window */
#define STR_INDEX_PARAM_GET_ANDROID_NATIVE_BUFFER "OMX.google.android.index.getAndroidNativeBufferUsage"
        OMX_IndexParamGetAndroidNativeBuffer,
#define STR_INDEX_PARAM_ANDROID_NATIVE_BUFFER_USAGE "OMX.google.android.index.AndroidNativeBufferConsumerUsage"
        OMX_IndexParamAndroidNativeBufferUsage,
#define STR_INDEX_PARAM_USE_ANDROID_NATIVE_BUFFER "OMX.google.android.index.useAndroidNativeBuffer"
        OMX_IndexParamUseAndroidNativeBuffer,
        /* for Android Store Metadata Inbuffer */
#define STR_INDEX_PARAM_STORE_METADATA_BUFFER "OMX.google.android.index.storeMetaDataInBuffers"
        OMX_IndexParamStoreMetaDataBuffer,	/**< reference: Android Store Metadata Inbuffer */
#define STR_INDEX_PARAM_STORE_ANWBUFFER_IN_METADATA "OMX.google.android.index.storeANWBufferInMetadata"
        OMX_IndexParamStoreANWBufferInMetadata,
#define STR_INDEX_PARAM_THUMBNAIL_MODE "OMX.vpu.index.ThumbnailMode"
        OMX_IndexConfigThumbnailMode,
#define STR_INDEX_PARAM_ADAPTIVE_PLAYBACK "OMX.google.android.index.prepareForAdaptivePlayback"
        OMX_IndexParamUseAdaptivePlayback,
#define STR_INDEX_PARAM_DESCRIBE_COLOR_ASPECT "OMX.google.android.index.describeColorAspects"
        OMX_IndexParamDescribeColorAspects,
#define STR_INDEX_PARAM_DESCRIBE_HDR_STATIC_INFO "OMX.google.android.index.describeHDRStaticInfo"
        OMX_IndexParamDescribeHDRStaticInfo,
} OMX_VPU_ANDORID_INDEXTYPE;



#ifdef ANDROID

#define PVR_ANDROID_HAS_SET_BUFFERS_DATASPACE
#define PVR_ANDROID_HAS_SET_BUFFERS_DATASPACE_2
#define SUPPORT_ADAPTIVE_PLAY

#include <powervr/img_gralloc_public.h>
#include <cutils/native_handle.h>

typedef struct {
    void *YPhyAddr;                     // [IN/OUT] physical address of Y
    void *CPhyAddr;                     // [IN/OUT] physical address of CbCr
    void *YVirAddr;                     // [IN/OUT] virtual address of Y
    void *CVirAddr;                     // [IN/OUT] virtual address of CbCr
    int YSize;                          // [IN/OUT] input size of Y data
    int CSize;                          // [IN/OUT] input size of CbCr data
} BUFFER_ADDRESS_INFO;

enum {
    kInputPortIndex  = 0,
    kOutputPortIndex = 1,
    kMaxPortIndex    = 1,
};

#define LOCK_MODE_TO_GET_VIRTUAL_ADDRESS 0
#define LOCK_MODE_TO_GET_PHYSICAL_ADDRESS 1
OMX_ERRORTYPE checkEnableAndroidBuffersHeader(OMX_PTR ComponentParameterStructure);
OMX_ERRORTYPE checkEnableAndroidBuffersPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex);
OMX_BOOL enableAndroidBuffer(OMX_PTR ComponentParameterStructure);


OMX_ERRORTYPE checkUseAndroidNativeBufferHeader(OMX_PTR ComponentParameterStructure);
OMX_ERRORTYPE checkUseAndroidNativeBufferPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex);
OMX_ERRORTYPE useAndroidNativeBuffer(OMX_PTR ComponentParameterStructure, OMX_BUFFERHEADERTYPE **pNativeBufHeaderType, OMX_COLOR_FORMATTYPE* eColorFormat, OMX_U32 size);


OMX_ERRORTYPE checkGetAndroidNativeBufferHeader(OMX_PTR ComponentParameterStructure);
OMX_ERRORTYPE checkGetAndroidNativeBufferPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex);
OMX_ERRORTYPE getAndroidNativeBufferUsage(OMX_PTR ComponentParameterStructure, OMX_U32 nUsage);
OMX_ERRORTYPE setAndroidNativeBufferUsage(OMX_PTR ComponentParameterStructure, OMX_U32* nUsage);
OMX_U32 getAndroidNativeHandle(OMX_BUFFERHEADERTYPE *pNativeBufHeaderType, OMX_U32* fd);
OMX_U32 mapAndroidPixelFormat(OMX_COLOR_FORMATTYPE index);
OMX_COLOR_FORMATTYPE mapOMXColorFormat(OMX_U32 index);

OMX_BOOL getAndroidNativeBufferInfo(OMX_BUFFERHEADERTYPE *pNativeBufHeaderType, int *pFormat, int *pStride, int *pWidth, int *pHeight);
int getNativeBufferSize(OMX_COLOR_FORMATTYPE colorFormat, int native_buffer_format, int stride, int height);

OMX_BOOL getAndroidNativeBufferHandleInfo(buffer_handle_t handle, int *pFormat, int *pWidth, int *pHeight, int *pStride, int *pSize);
OMX_ERRORTYPE checkAndroidParamHeader(OMX_PTR header, OMX_U32 size);
OMX_ERRORTYPE checkStoreMetaDataBufferPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex);
OMX_ERRORTYPE checkStoreMetaDataBufferHeader(OMX_PTR ComponentParameterStructure);
OMX_ERRORTYPE storeMetaDataBuffer(OMX_PTR ComponentParameterStructure, OMX_BOOL *pbEnable);
OMX_U32 lockAndroidBufferHandle(buffer_handle_t handle, int width, int height, OMX_U32 mode, void **pAddr);
OMX_U32 unLockAndroidBufferHandle(buffer_handle_t handle);
OMX_U32 lockAndroidNativeBuffer(OMX_BUFFERHEADERTYPE *pNativeBufHeaderType, int stride, int height, OMX_U32 mode, void **pAddr);
OMX_U32 unlockAndroidNativeBuffer(OMX_BUFFERHEADERTYPE *pNativeBufHeaderType);
#ifdef SUPPORT_ADAPTIVE_PLAY
OMX_ERRORTYPE checkUseAdaptivePlaybackHeader(OMX_PTR ComponentParameterStructure);
OMX_ERRORTYPE checkUseAdaptivePlaybackPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex);
OMX_ERRORTYPE getAdaptivePlayParams(OMX_PTR ComponentParameterStructure, OMX_BOOL *enable, OMX_U32 *maxWidth, OMX_U32 *maxHeight);
#endif
OMX_BOOL convertRgbToYuvbySW(OMX_BYTE pYuvData, OMX_BYTE pRgbData, OMX_U32 grallocFormat, OMX_U32 grallocWidth,  OMX_U32 grallocHeight);
OMX_BOOL checkTileIFBCDisabled(OMX_STRING prop, OMX_BOOL defvalue);
#endif //ANDROID

#ifdef __cplusplus
}
#endif

#endif

