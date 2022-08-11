/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

#include <string.h>
#include <time.h>
#include "omx_utils.h"
#include "OMX_VPU_Video.h"

static const struct ComponentMapEntry kDecComponentMapEntry[] = {
    {VIDEO_DEC_H264_NAME, OMX_VIDEO_CodingAVC, VIDEO_DEC_H264_ROLE},
    {VIDEO_DEC_MPEG4_NAME, OMX_VIDEO_CodingMPEG4, VIDEO_DEC_MPEG4_ROLE},
    {VIDEO_DEC_MPEG2_NAME, OMX_VIDEO_CodingMPEG2, VIDEO_DEC_MPEG2_ROLE},
    {VIDEO_DEC_H263_NAME, OMX_VIDEO_CodingH263, VIDEO_DEC_H263_ROLE},
    {VIDEO_DEC_VP8_NAME, OMX_VIDEO_CodingVP8, VIDEO_DEC_VP8_ROLE},
    {VIDEO_DEC_HEVC_NAME, OMX_VIDEO_CodingHEVC, VIDEO_DEC_HEVC_ROLE},
    {VIDEO_DEC_VP9_NAME, OMX_VIDEO_CodingVP9, VIDEO_DEC_VP9_ROLE},
    {VIDEO_DEC_RV_NAME, OMX_VIDEO_CodingRV, VIDEO_DEC_RV_ROLE},
    {VIDEO_DEC_WMV_NAME, OMX_VIDEO_CodingWMV, VIDEO_DEC_WMV_ROLE},
    {VIDEO_DEC_MSMPEG_NAME, OMX_VIDEO_CodingMSMPEG, VIDEO_DEC_MSMPEG_ROLE},
};

static const struct ComponentMapEntry kEncComponentMapEntry[] = {
    {VIDEO_ENC_AVC_NAME, OMX_VIDEO_CodingAVC, VIDEO_ENC_AVC_ROLE},
    {VIDEO_ENC_MPEG4_NAME, OMX_VIDEO_CodingMPEG4, VIDEO_ENC_MPEG4_ROLE},
    {VIDEO_ENC_H263_NAME, OMX_VIDEO_CodingH263, VIDEO_ENC_H263_ROLE},
};

OMX_ERRORTYPE GetVideoCodingTypeByName(
    const OMX_STRING cComponentName, OMX_VIDEO_CODINGTYPE *codingType, OMX_BOOL isEncoder) {
    ComponentMapEntry *compEntry = NULL;

    compEntry = isEncoder ? (ComponentMapEntry *)kEncComponentMapEntry : (ComponentMapEntry *)kDecComponentMapEntry;
    for (size_t i = 0; i < GetComponentCount(isEncoder); ++i) {
        if (!strcmp(cComponentName, compEntry[i].cComponentName)) {
            *codingType = compEntry[i].cVideoCodingType;
            return OMX_ErrorNone;
        }
    }

    *codingType = OMX_VIDEO_CodingUnused;

    return OMX_ErrorInvalidComponentName;
}

OMX_ERRORTYPE GetRoleByVideoCodingType(
    OMX_VIDEO_CODINGTYPE codingType, OMX_U8* cRole, OMX_BOOL isEncoder) {
    ComponentMapEntry *compEntry = NULL;

    compEntry = isEncoder ? (ComponentMapEntry *)kEncComponentMapEntry : (ComponentMapEntry *)kDecComponentMapEntry;
    for (size_t i = 0; i < GetComponentCount(isEncoder); ++i) {
        if (codingType == compEntry[i].cVideoCodingType) {
            strcpy((char *)cRole, (char *)compEntry[i].cRole);
            return OMX_ErrorNone;
        }
    }

    *cRole = '\0';

    return OMX_ErrorUndefined;
}

OMX_ERRORTYPE GetComponentRoleByIndex(
    OMX_U8 index, OMX_STRING cRole, OMX_BOOL isEncoder) {
    ComponentMapEntry *compEntry = NULL;

    compEntry = isEncoder ? (ComponentMapEntry *)kEncComponentMapEntry : (ComponentMapEntry *)kDecComponentMapEntry;
    if (index < GetComponentCount(isEncoder)) {
        strcpy((char *)cRole, (char *)compEntry[index].cRole);
        return OMX_ErrorNone;
    }

    *cRole = '\0';
    return OMX_ErrorComponentNotFound;
}

OMX_ERRORTYPE GetComponentNameByIndex(
    OMX_U8 index, OMX_STRING cComponentName, OMX_BOOL isEncoder) {
    ComponentMapEntry *compEntry = NULL;

    compEntry = isEncoder ? (ComponentMapEntry *)kEncComponentMapEntry : (ComponentMapEntry *)kDecComponentMapEntry;
    if (index < GetComponentCount(isEncoder)) {
        strcpy((char *)cComponentName, (char *)compEntry[index].cComponentName);
        return OMX_ErrorNone;
    }

    *cComponentName = '\0';
    return OMX_ErrorComponentNotFound;
}

OMX_U8 GetComponentCount(
    OMX_BOOL isEncoder) {
    return isEncoder
               ? sizeof(kEncComponentMapEntry) / sizeof(kEncComponentMapEntry[0])
               : sizeof(kDecComponentMapEntry) / sizeof(kDecComponentMapEntry[0]);
}

double GetNowMs()
{
    double curr = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    curr = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000.0;
    curr /= 1000.0;

    return curr;
}
