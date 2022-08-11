/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : omx_color_aspect_wrapper.h
 * Version  : 1.0.0
 * Purpose  : wrap for android ColorAspects
 * Authors  : jun.jin
 * Date     : 2020-12-25
 * Notes    :
 *
 ******************************************************************************/


/*---------------------------- include head file -----------------------------*/


#ifndef _OMX_HDR_STATIC_INFO_WRAPPER_H
#define _OMX_HDR_STATIC_INFO_WRAPPER_H

#undef LOG_TAG
#define LOG_TAG "VPUOMX"

#ifdef __cplusplus
extern "C" {
#endif
#include <OMX_Types.h>

//convert form frameworks/native/headers/media_plugin/media/hardware/VideoAPI.h
//maintain consistency with the framework definition.
typedef struct {
    OMX_U32 display_primaries_x[3];
    OMX_U32 display_primaries_y[3];
    OMX_U32 white_point_x                   : 16;
    OMX_U32 white_point_y                   : 16;
    OMX_U32 max_display_mastering_luminance : 32;
    OMX_U32 min_display_mastering_luminance : 32;
    OMX_U32 max_content_light_level;
    OMX_U32 max_frame_ave_light_level;
} HdrInfoWrapper;

#ifdef ANDROID
OMX_ERRORTYPE getHdrStaticInfoFromParams(HdrInfoWrapper *hdrInfoWrapper, OMX_PTR params);
OMX_ERRORTYPE setHdrStaticInfoToParams(HdrInfoWrapper *hdrInfoWrapper, OMX_PTR params);
OMX_BOOL supportDescribeHdrStaticInfo(OMX_VIDEO_CODINGTYPE codingType);
void convertVendorHdrToCodecHdrInfo(HdrInfoWrapper *hdrInfoWrapper);
#endif //ANDROID
#ifdef __cplusplus
}
#endif

#endif