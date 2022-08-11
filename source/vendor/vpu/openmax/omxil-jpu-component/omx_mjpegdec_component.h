#ifndef _OMX_MJPEGDEC_COMPONENT_H_
#define _OMX_MJPEGDEC_COMPONENT_H_

#ifdef HAVE_STDLIB_H
#undef HAVE_STDLIB_H
#endif
#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <omx_base_filter.h>
#include <omx_mjpeg_utils.h>
#include "jpu_middleware_api.h"

/** Port private definition.
 * Contains component allocated buffer descriptions.
 * Only MAX_BUFFERS buffers can be allocated by the component
 * Buffers have an allocated state: The user has requested a buffer/has not
 */
#define BUFFER_ALLOCATED (1 << 0)
#define MAX_BUFFERS 2
#define DEFAULT_FRAME_WIDTH 640
#define DEFAULT_FRAME_HEIGHT 480
#define IN_BUFFER_SIZE 4096
#define OUT_BUFFER_SIZE DEFAULT_FRAME_WIDTH *DEFAULT_FRAME_HEIGHT * 3 + 54 /* 640 x 480 x 2bytes x(3/2) +54bytes header*/

/** Jpeg Decoder component private structure.
 */
DERIVEDCLASS(omx_mjpegdec_component_PrivateType, omx_base_filter_PrivateType)
#define omx_mjpegdec_component_PrivateType_FIELDS \
  omx_base_filter_PrivateType_FIELDS              \
  OMX_COMPONENTTYPE *hMarkTargetComponent;        \
  OMX_PTR pMarkData;                              \
  OMX_U32 nFlags;                                 \
  OMX_BOOL useNativeBuffer;                       \
  OMX_CONFIG_RECTTYPE omxOutputPortCrop;          \
  jpu_hw_instance *jpu;
ENDCLASS(omx_mjpegdec_component_PrivateType)

/* Component private entry points declaration */
OMX_ERRORTYPE omx_mjpegdec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName);
OMX_ERRORTYPE omx_mjpegdec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_mjpegdec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_mjpegdec_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_mjpegdec_decoder_MessageHandler(OMX_COMPONENTTYPE *, internalRequestMessageType *);
void *omx_mjpegdec_component_BufferMgmtFunction(void *param);

void omx_mjpegdec_component_BufferMgmtCallback(
    OMX_COMPONENTTYPE *openmaxStandComp,
    OMX_BUFFERHEADERTYPE *inputbuffer,
    OMX_BUFFERHEADERTYPE *outputbuffer);

OMX_ERRORTYPE omx_mjpegdec_component_GetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_mjpegdec_component_SetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_mjpegdec_component_SetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_mjpegdec_component_GetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure);
#endif
