/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : omx_color_aspect_wrapper.cpp
 * Version  : 1.0.0
 * Purpose  : wrap for android ColorAspects
 * Authors  : jun.jin
 * Date     : 2020-12-25
 * Notes    :
 *
 ******************************************************************************/


/*---------------------------- include head file -----------------------------*/

#include <omxcore.h>
#include <HardwareAPI.h>
#include <hardware/hardware.h>
#include <media/stagefright/foundation/ColorUtils.h>

#include "omx_color_aspect_wrapper.h"
#include "android_support.h"

using namespace android;
static ColorAspectsPreference getColorAspectPreference(OMX_VIDEO_CODINGTYPE codingType);
static OMX_BOOL colorAspectsDiffer(const ColorAspectsWrapper *a, const ColorAspectsWrapper *b);

ColorAspectsPreference getColorAspectPreference(OMX_VIDEO_CODINGTYPE codingType) {
    switch(codingType) {
        case OMX_VIDEO_CodingAVC:
        case OMX_VIDEO_CodingHEVC:
        case OMX_VIDEO_CodingMPEG2:
            return kPreferBitstream;
        default:
            return kNotSupported;
    }
}


OMX_BOOL colorAspectsDiffer(const ColorAspectsWrapper *a, const ColorAspectsWrapper *b) {
    if (a->mRange != b->mRange
        || a->mPrimaries != b->mPrimaries
        || a->mTransfer != b->mTransfer
        || a->mMatrixCoeffs != b->mMatrixCoeffs) {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}


void convertIsoColorAspectsToCodecAspects(ColorAspectsWrapper *bitstreamColorAspect,
    OMX_S32 primaries, OMX_S32 transfer, OMX_S32 coeffs, OMX_BOOL fullRange) {
    ColorAspects colorAspects;
    ColorUtils::convertIsoColorAspectsToCodecAspects(
            primaries, transfer, coeffs, fullRange, colorAspects);

    DEBUG(DEB_LEV_FULL_SEQ, "%s : convert fullRange %d->%d", __func__, fullRange, colorAspects.mRange);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : convert primaries %d->%d", __func__, primaries, colorAspects.mPrimaries);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : convert transfer %d->%d", __func__, transfer, colorAspects.mTransfer);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : convert mMatrixCoeffs %d->%d", __func__, coeffs, colorAspects.mMatrixCoeffs);

    bitstreamColorAspect->mRange = colorAspects.mRange;
    bitstreamColorAspect->mTransfer = colorAspects.mTransfer;
    bitstreamColorAspect->mPrimaries = colorAspects.mPrimaries;
    bitstreamColorAspect->mMatrixCoeffs = colorAspects.mMatrixCoeffs;
}


OMX_ERRORTYPE getColorAspectFromParams(ColorAspectsWrapper *defaultColorAspect, OMX_PTR params) {
    ColorAspectsWrapper colorAspect;

    if (OMX_ErrorNone != checkAndroidParamHeader(params, sizeof(DescribeColorAspectsParams))) {
        return OMX_ErrorBadParameter;
    }

    const DescribeColorAspectsParams* colorAspectsParams = (const DescribeColorAspectsParams *)params;

    // make sure the port index is output port
    if (colorAspectsParams->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorBadPortIndex;
    }

    colorAspect.mRange = colorAspectsParams->sAspects.mRange;
    colorAspect.mTransfer = colorAspectsParams->sAspects.mTransfer;
    colorAspect.mPrimaries = colorAspectsParams->sAspects.mPrimaries;
    colorAspect.mMatrixCoeffs = colorAspectsParams->sAspects.mMatrixCoeffs;
    // update the default color aspects if necessary
    if (colorAspectsDiffer(&colorAspect, defaultColorAspect)) {
        *defaultColorAspect = colorAspect;
    }

    return OMX_ErrorNone;
}


OMX_ERRORTYPE setColorAspectToParams(ColorAspectsWrapper *finalColorAspect, OMX_PTR params) {
    if (OMX_ErrorNone != checkAndroidParamHeader(params, sizeof(DescribeColorAspectsParams))) {
        return OMX_ErrorBadParameter;
    }

    DescribeColorAspectsParams* colorAspectsParams = (DescribeColorAspectsParams *)params;
    if (colorAspectsParams->nPortIndex != kOutputPortIndex) {
        return OMX_ErrorBadPortIndex;
    }

    colorAspectsParams->sAspects.mRange = (ColorAspects::Range)finalColorAspect->mRange;
    colorAspectsParams->sAspects.mTransfer = (ColorAspects::Transfer)finalColorAspect->mTransfer;
    colorAspectsParams->sAspects.mPrimaries = (ColorAspects::Primaries)finalColorAspect->mPrimaries;
    colorAspectsParams->sAspects.mMatrixCoeffs = (ColorAspects::MatrixCoeffs)finalColorAspect->mMatrixCoeffs;

    DEBUG(DEB_LEV_FULL_SEQ, "%s : final fullRange %d", __func__, finalColorAspect->mRange);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : final primaries %d", __func__, finalColorAspect->mPrimaries);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : final transfer %d", __func__, finalColorAspect->mTransfer);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : final mMatrixCoeffs %d", __func__, finalColorAspect->mMatrixCoeffs);
    return OMX_ErrorNone;
}


OMX_BOOL supportsDescribeColorAspects(OMX_VIDEO_CODINGTYPE codingType) {
    if (getColorAspectPreference(codingType) != kNotSupported)
        return OMX_TRUE;
    return OMX_FALSE;
}


OMX_BOOL handleColorAspectsChange(
    OMX_VIDEO_CODINGTYPE codingType, ColorAspectsWrapper *defaultColorAspect,
    ColorAspectsWrapper *bitstreamColorAspect, ColorAspectsWrapper *finalColorAspect) {

    OMX_BOOL colorAspectChanged = OMX_FALSE;
    ColorAspectsPreference perference = getColorAspectPreference(codingType);

    DEBUG(DEB_LEV_FULL_SEQ, "color aspects preference: %d ", perference);

    if (perference == kPreferBitstream) {
        colorAspectChanged = updateFinalColorAspects(defaultColorAspect, bitstreamColorAspect, finalColorAspect);
    } else if (perference == kPreferContainer) {
        colorAspectChanged = updateFinalColorAspects(bitstreamColorAspect, defaultColorAspect, finalColorAspect);
    }

    return colorAspectChanged;
}


OMX_BOOL updateFinalColorAspects(
    const ColorAspectsWrapper *otherAspects, const ColorAspectsWrapper *preferredAspects, ColorAspectsWrapper *resultAspects) {

    OMX_BOOL updated = OMX_FALSE;
    ColorAspectsWrapper newAspects;
    newAspects.mRange = preferredAspects->mRange != ColorAspects::RangeUnspecified ?
        preferredAspects->mRange : otherAspects->mRange;
    newAspects.mPrimaries = preferredAspects->mPrimaries != ColorAspects::PrimariesUnspecified ?
        preferredAspects->mPrimaries : otherAspects->mPrimaries;
    newAspects.mTransfer = preferredAspects->mTransfer != ColorAspects::TransferUnspecified ?
        preferredAspects->mTransfer : otherAspects->mTransfer;
    newAspects.mMatrixCoeffs = preferredAspects->mMatrixCoeffs != ColorAspects::MatrixUnspecified ?
        preferredAspects->mMatrixCoeffs : otherAspects->mMatrixCoeffs;

    DEBUG(DEB_LEV_FULL_SEQ, "%s : newAspects fullRange %d", __func__, newAspects.mRange);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : newAspects primaries %d", __func__, newAspects.mPrimaries);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : newAspects transfer %d", __func__, newAspects.mTransfer);
    DEBUG(DEB_LEV_FULL_SEQ, "%s : newAspects mMatrixCoeffs %d", __func__, newAspects.mMatrixCoeffs);
    // Check to see if need update mFinalColorAspects.
    if (colorAspectsDiffer(&newAspects, resultAspects)) {
        *resultAspects = newAspects;
        updated = OMX_TRUE;
    }

    DEBUG(DEB_LEV_FULL_SEQ, "%s : updated is %d", __func__, updated);

    return updated;
}

