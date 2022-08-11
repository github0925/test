/*************************************************************************/ /*!
@File           pvrversion.h
@Title          PowerVR version numbers and strings.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Version numbers and strings for PowerVR components.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRVERSION_H
#define PVRVERSION_H

#define PVRVERSION_MAJ               1U
#define PVRVERSION_MIN               13U

#define PVRVERSION_FAMILY           "rogueddk"
#define PVRVERSION_BRANCHNAME       "1.13"
#define PVRVERSION_BUILD             5824814
#define PVRVERSION_BSCONTROL        "Rogue_DDK_Android"

#define PVRVERSION_STRING           "Rogue_DDK_Android rogueddk 1.13@5824814"
#define PVRVERSION_STRING_SHORT     "1.13@5824814"

#define COPYRIGHT_TXT               "Copyright (c) Imagination Technologies Ltd. All Rights Reserved."

#define PVRVERSION_BUILD_HI          582
#define PVRVERSION_BUILD_LO          4814
#define PVRVERSION_STRING_NUMERIC   "1.13.582.4814"

#define PVRVERSION_PACK(MAJOR,MINOR) (((IMG_UINT32)((IMG_UINT32)(MAJOR) & 0xFFFFU) << 16U) | (((MINOR) & 0xFFFFU) << 0U))
#define PVRVERSION_UNPACK_MAJ(VERSION) (((VERSION) >> 16U) & 0xFFFFU)
#define PVRVERSION_UNPACK_MIN(VERSION) (((VERSION) >> 0U) & 0xFFFFU)

#endif /* PVRVERSION_H */
