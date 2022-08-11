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


#ifndef _OMX_COLOR_ASPECT_WRAPPER_H
#define _OMX_COLOR_ASPECT_WRAPPER_H

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
//convert form frameworks/native/headers/media_plugin/media/hardware/VideoAPI.h
//maintain consistency with the framework definition.
typedef struct {
    OMX_U32 mRange;
    OMX_U32 mPrimaries;
    OMX_U32 mTransfer;
    OMX_U32 mMatrixCoeffs;
} ColorAspectsWrapper;

#ifdef ANDROID
typedef enum {
    kNotSupported,
    kPreferBitstream,
    kPreferContainer,
} ColorAspectsPreference;

void convertIsoColorAspectsToCodecAspects(ColorAspectsWrapper *bitstreamColorAspect,
    OMX_S32 primaries, OMX_S32 transfer, OMX_S32 coeffs, OMX_BOOL fullRange);
OMX_ERRORTYPE getColorAspectFromParams(ColorAspectsWrapper *colorAspect, OMX_PTR params);
OMX_ERRORTYPE setColorAspectToParams(ColorAspectsWrapper *finalColorAspect, OMX_PTR params);
OMX_BOOL supportsDescribeColorAspects(OMX_VIDEO_CODINGTYPE codingType);
OMX_BOOL updateFinalColorAspects(
    const ColorAspectsWrapper *otherAspects, const ColorAspectsWrapper *preferredAspects, ColorAspectsWrapper *resultAspects);
OMX_BOOL handleColorAspectsChange(
    OMX_VIDEO_CODINGTYPE codingType, ColorAspectsWrapper *defaultColorAspect,
    ColorAspectsWrapper *bitstreamColorAspect, ColorAspectsWrapper *finalColorAspect);
#endif //ANDROID
#ifdef __cplusplus
}
#endif

#endif