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

// #include "../../../libvpu/config.h"
#include <st_static_component_loader.h>
#include <omx_vpudec_component.h>
#include <omx_vpuenc_component.h>
#include "vpuconfig.h"
#include "omx_utils.h"

#ifdef ANDROID
#include "android_support.h"
#endif

/** @brief The library entry point. It must have the same name for each
* library of the components loaded by the ST static component loader.
*
* This function fills the version, the component name and if existing also the roles
* and the specific names for each role. This base function is only an explanation.
* For each library it must be implemented, and it must fill data of any component
* in the library
*
* @param stComponents pointer to an array of components descriptors.If NULL, the
* function will return only the number of components contained in the library
*
* @return number of components contained in the library
*/

#ifdef SUPPORT_ENCODER
#define NUM_OF_COMPONENT 2
#else
#define NUM_OF_COMPONENT 1
#endif

#if defined(WIN32)
__declspec(dllexport)
#endif
int omx_component_library_Setup(stLoaderComponentType **stComponents)
{
    OMX_U32 i = 0, index = 0;
    OMX_BOOL isEncoder = OMX_FALSE;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s \n",__func__);

    if (stComponents == NULL) {
        DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s \n",__func__);
        return NUM_OF_COMPONENT; // Return Number of Components - one for audio, two for video
    }

    /** index 0 - video decoder */
    /** index 1 - video encoder */
    for (index = 0; index < NUM_OF_COMPONENT; index++) {
        stComponents[index]->componentVersion.s.nVersionMajor = 1;
        stComponents[index]->componentVersion.s.nVersionMinor = 1;
        stComponents[index]->componentVersion.s.nRevision = 1;
        stComponents[index]->componentVersion.s.nStep = 1;
        stComponents[index]->name = calloc(OMX_MAX_STRINGNAME_SIZE, 1);

        if (stComponents[index]->name == NULL) {
            return OMX_ErrorInsufficientResources;
        }

        if (0 == index) { // 0 is for video dec
            isEncoder = OMX_FALSE;
        } else if(1 == index) { // 1 is for video enc
            isEncoder = OMX_TRUE;
        }

        strcpy(stComponents[index]->name, isEncoder ? VIDEO_ENC_BASE_NAME : VIDEO_DEC_BASE_NAME);
        stComponents[index]->constructor = isEncoder ? omx_vpuenc_component_Constructor : omx_vpudec_component_Constructor;

        stComponents[index]->name_specific_length = GetComponentCount(isEncoder);
        stComponents[index]->name_specific = calloc(stComponents[index]->name_specific_length, sizeof(char *));
        if (stComponents[index]->name_specific == NULL) {
            return OMX_ErrorInsufficientResources;
        }
        stComponents[index]->role_specific = calloc(stComponents[index]->name_specific_length, sizeof(char *));
        if (stComponents[index]->role_specific == NULL) {
            return OMX_ErrorInsufficientResources;
        }

        for (i = 0; i < stComponents[index]->name_specific_length; i++) {
            stComponents[index]->name_specific[i] = calloc(OMX_MAX_STRINGNAME_SIZE, 1);
            if (stComponents[index]->name_specific[i] == NULL) {
                return OMX_ErrorInsufficientResources;
            }

            GetComponentNameByIndex(i, stComponents[index]->name_specific[i], isEncoder);

            stComponents[index]->role_specific[i] = calloc(OMX_MAX_STRINGNAME_SIZE, 1);
            if (stComponents[index]->role_specific[i] == NULL) {
                return OMX_ErrorInsufficientResources;
            }

            GetComponentRoleByIndex(i, stComponents[index]->role_specific[i], isEncoder);
        }
    }


    DEBUG(DEB_LEV_SIMPLE_SEQ, "Out of %s \n",__func__);

    return index;
}
