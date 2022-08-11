/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : omx_hdr_inf_wrapper.cpp
 * Version  : 1.0.0
 * Purpose  : wrap for android HDR metadata
 * Authors  : jun.jin
 * Date     : 202-01-21
 * Notes    :
 *
 ******************************************************************************/


/*---------------------------- include head file -----------------------------*/

#include <omxcore.h>
#include <HardwareAPI.h>

#include "omx_hdr_info_wrapper.h"
#include "android_support.h"

using namespace android;


void convertVendorHdrToCodecHdrInfo(HdrInfoWrapper *hdrInfoWrapper) {
    // the fw report max_display_mastering_luminance with a denominator 10000 implicitly.
    // but the framework need the raw data, so divide 10000 here
    hdrInfoWrapper->max_display_mastering_luminance = hdrInfoWrapper->max_display_mastering_luminance / 10000;
}


OMX_ERRORTYPE getHdrStaticInfoFromParams(HdrInfoWrapper *hdrInfoWrapper, OMX_PTR params) {
    if (OMX_ErrorNone != checkAndroidParamHeader(params, sizeof(DescribeHDRStaticInfoParams))) {
        return OMX_ErrorBadParameter;
    }

    const DescribeHDRStaticInfoParams* hdrStaticInfoParams = (const DescribeHDRStaticInfoParams *)params;

    // make sure the port index is output port
    if (hdrStaticInfoParams->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorBadPortIndex;
    }

    hdrInfoWrapper->display_primaries_x[0] = hdrStaticInfoParams->sInfo.sType1.mG.x;
    hdrInfoWrapper->display_primaries_x[1] = hdrStaticInfoParams->sInfo.sType1.mB.x;
    hdrInfoWrapper->display_primaries_x[2] = hdrStaticInfoParams->sInfo.sType1.mR.x;

    hdrInfoWrapper->display_primaries_y[0] = hdrStaticInfoParams->sInfo.sType1.mG.y;
    hdrInfoWrapper->display_primaries_y[1] = hdrStaticInfoParams->sInfo.sType1.mB.y;
    hdrInfoWrapper->display_primaries_y[2] = hdrStaticInfoParams->sInfo.sType1.mR.y;

    hdrInfoWrapper->white_point_x = hdrStaticInfoParams->sInfo.sType1.mW.x;
    hdrInfoWrapper->white_point_y = hdrStaticInfoParams->sInfo.sType1.mW.y;

    hdrInfoWrapper->max_display_mastering_luminance = hdrStaticInfoParams->sInfo.sType1.mMaxDisplayLuminance;
    hdrInfoWrapper->min_display_mastering_luminance = hdrStaticInfoParams->sInfo.sType1.mMinDisplayLuminance;

    hdrInfoWrapper->max_content_light_level  = hdrStaticInfoParams->sInfo.sType1.mMaxContentLightLevel;
    hdrInfoWrapper->max_frame_ave_light_level= hdrStaticInfoParams->sInfo.sType1.mMaxFrameAverageLightLevel;

    return OMX_ErrorNone;
}


OMX_ERRORTYPE setHdrStaticInfoToParams(HdrInfoWrapper *hdrInfoWrapper, OMX_PTR params) {
    if (OMX_ErrorNone != checkAndroidParamHeader(params, sizeof(DescribeHDRStaticInfoParams))) {
        return OMX_ErrorBadParameter;
    }

    DescribeHDRStaticInfoParams* hdrStaticInfoParams = (DescribeHDRStaticInfoParams *)params;
    if (hdrStaticInfoParams->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorBadPortIndex;
    }

    // gbr to rgb
    hdrStaticInfoParams->sInfo.sType1.mG.x = hdrInfoWrapper->display_primaries_x[0];
    hdrStaticInfoParams->sInfo.sType1.mB.x = hdrInfoWrapper->display_primaries_x[1];
    hdrStaticInfoParams->sInfo.sType1.mR.x = hdrInfoWrapper->display_primaries_x[2];

    hdrStaticInfoParams->sInfo.sType1.mG.y = hdrInfoWrapper->display_primaries_y[0];
    hdrStaticInfoParams->sInfo.sType1.mB.y = hdrInfoWrapper->display_primaries_y[1];
    hdrStaticInfoParams->sInfo.sType1.mR.y = hdrInfoWrapper->display_primaries_y[2];

    hdrStaticInfoParams->sInfo.sType1.mW.x = hdrInfoWrapper->white_point_x;
    hdrStaticInfoParams->sInfo.sType1.mW.y = hdrInfoWrapper->white_point_y;

    hdrStaticInfoParams->sInfo.sType1.mMaxDisplayLuminance = hdrInfoWrapper->max_display_mastering_luminance;
    hdrStaticInfoParams->sInfo.sType1.mMinDisplayLuminance = hdrInfoWrapper->min_display_mastering_luminance;

    hdrStaticInfoParams->sInfo.sType1.mMaxContentLightLevel = hdrInfoWrapper->max_content_light_level;
    hdrStaticInfoParams->sInfo.sType1.mMaxFrameAverageLightLevel = hdrInfoWrapper->max_frame_ave_light_level;

    DEBUG(DEB_LEV_FULL_SEQ, "%s : mG.x %d, mG.y %d", __func__,  hdrStaticInfoParams->sInfo.sType1.mG.x,  hdrStaticInfoParams->sInfo.sType1.mG.y);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : mW.x %d, mW.y %d", __func__,  hdrStaticInfoParams->sInfo.sType1.mW.x,  hdrStaticInfoParams->sInfo.sType1.mW.y);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : mMaxDisplayLuminance %d, mMaxContentLightLevel %d", __func__,
                                hdrStaticInfoParams->sInfo.sType1.mMaxDisplayLuminance, hdrStaticInfoParams->sInfo.sType1.mMaxContentLightLevel);
    return OMX_ErrorNone;
}


OMX_BOOL supportDescribeHdrStaticInfo(OMX_VIDEO_CODINGTYPE codingType) {
    // for now, we only updata the hdr static info for hevc
    if (codingType != OMX_VIDEO_CodingHEVC)
        return OMX_FALSE;
    return OMX_TRUE;
}

