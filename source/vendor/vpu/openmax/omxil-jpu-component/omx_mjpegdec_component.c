#include "omx_mjpegdec_component.h"
#include <OMX_Core.h>
#include <omx_base_video_port.h>
#include <time.h>
#include <ctype.h> /* to declare isprint() */

#define MAX_COMPONENT_JPEGDEC 4
#define DEFAULT_STREAMBUFFER_SIZE 0xA00000 // case of 4K, 10M recommended
#define DEFAULT_VIDEO_INPUT_BUF_SIZE DEFAULT_STREAMBUFFER_SIZE
#define DEFAULT_MIN_VIDEO_INPUT_BUFFER_NUM 4  //omx defaults set to 2
#define DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM 6 //the output buffer must be more than 2

#define OMX_BufferSupplyVendorDRM (OMX_BufferSupplyVendorStartUnused + 1)

//private func for omx jpu
int jpu_decompress_16_bit_align(jpu_hw_instance *jpu, int decode_to_fd, int yuv_fd);

/** Number of Component Instance*/
static OMX_U32 numjpegdecInstance = 0;

double GetNowMs()
{
    double curr = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    curr = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000.0;
    curr /= 1000.0;

    return curr;
}

/** The Constructor
  *
  * @param openmaxStandComp the component handle to be constructed
  * @param cComponentName name of the component to be constructed
  */
OMX_ERRORTYPE omx_mjpegdec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName)
{

    OMX_ERRORTYPE err = OMX_ErrorNone;
    omx_mjpegdec_component_PrivateType *omx_mjpegdec_component_Private;
    omx_base_video_PortType *pInPort, *pOutPort;
    OMX_U32 i;

    GetDebugLevelFromProperty(DEBUG_LOG_COMP, DEB_LEV_ERR);

    if (!openmaxStandComp->pComponentPrivate)
    {
        //calloc will memset to 0
        openmaxStandComp->pComponentPrivate = calloc(1, sizeof(omx_mjpegdec_component_PrivateType));
        if (openmaxStandComp->pComponentPrivate == NULL)
        {
            DEBUG(DEB_LEV_ERR, "[%s][%d] calloc failed\n", __func__, __LINE__);
            return OMX_ErrorInsufficientResources;
        }
    }
    else
    {
        DEBUG(DEB_LEV_FUNCTION_NAME, "In %s, Error Component %x Already Allocated\n",
              __func__, openmaxStandComp->pComponentPrivate);
    }

    omx_mjpegdec_component_Private = openmaxStandComp->pComponentPrivate;
    omx_mjpegdec_component_Private->ports = NULL;

    /** we could create our own port structures here
    * fixme maybe the base class could use a "port factory" function pointer?
    */
    err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

    DEBUG(DEB_LEV_SIMPLE_SEQ, "constructor of mad decoder component is called\n");

    /** Domain specific section for the ports. */
    omx_mjpegdec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber = 0;
    omx_mjpegdec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts = 2;

    /** Allocate Ports and call port constructor. */
    if (omx_mjpegdec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts && !omx_mjpegdec_component_Private->ports)
    {
        omx_mjpegdec_component_Private->ports =
            calloc(omx_mjpegdec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts, sizeof(omx_base_PortType *));
        if (!omx_mjpegdec_component_Private->ports)
        {
            DEBUG(DEB_LEV_ERR, "[%s][%d] calloc failed\n", __func__, __LINE__);
            return OMX_ErrorInsufficientResources;
        }
        for (i = 0; i < omx_mjpegdec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++)
        {
            omx_mjpegdec_component_Private->ports[i] = calloc(1, sizeof(omx_base_video_PortType));
            if (!omx_mjpegdec_component_Private->ports[i])
            {
                DEBUG(DEB_LEV_ERR, "[%s][%d] calloc failed\n", __func__, __LINE__);
                return OMX_ErrorInsufficientResources;
            }
        }
    }

    base_video_port_Constructor(openmaxStandComp, &omx_mjpegdec_component_Private->ports[0], 0, OMX_TRUE);
    base_video_port_Constructor(openmaxStandComp, &omx_mjpegdec_component_Private->ports[1], 1, OMX_FALSE);

    /** parameters related to input port */
    pInPort = (omx_base_video_PortType *)omx_mjpegdec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    pInPort->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMJPEG;
    pInPort->sPortParam.nBufferCountActual = DEFAULT_MIN_VIDEO_INPUT_BUFFER_NUM;
    pInPort->sPortParam.nBufferCountMin = DEFAULT_MIN_VIDEO_INPUT_BUFFER_NUM;
    pInPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingMJPEG;
    pInPort->sPortParam.nBufferSize = DEFAULT_VIDEO_INPUT_BUF_SIZE;

    /** parameters related to output port */
    pOutPort = (omx_base_video_PortType *)omx_mjpegdec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    pOutPort->sPortParam.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar; //I420
    pOutPort->sPortParam.nBufferCountActual = DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM;
    pOutPort->sPortParam.nBufferCountMin = DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM;
    pOutPort->sVideoParam.eColorFormat = OMX_COLOR_FormatYUV420Planar;

    /** general configuration irrespective of any formats
    *  setting values of other fields of omx_mjpegdec_component_Private structure
    */
    omx_mjpegdec_component_Private->hMarkTargetComponent = NULL;
    omx_mjpegdec_component_Private->nFlags = 0x0;
    omx_mjpegdec_component_Private->BufferMgmtFunction = omx_mjpegdec_component_BufferMgmtFunction;
    omx_mjpegdec_component_Private->messageHandler = omx_mjpegdec_decoder_MessageHandler;
    omx_mjpegdec_component_Private->destructor = omx_mjpegdec_component_Destructor;
    openmaxStandComp->SetParameter = omx_mjpegdec_component_SetParameter;
    openmaxStandComp->GetParameter = omx_mjpegdec_component_GetParameter;
    openmaxStandComp->SetConfig = omx_mjpegdec_component_SetConfig;
    openmaxStandComp->GetConfig = omx_mjpegdec_component_GetConfig;

    numjpegdecInstance++;

    if (numjpegdecInstance > MAX_COMPONENT_JPEGDEC)
    {
        DEBUG(DEB_LEV_ERR, "[%s][%d] numjpegdecInstance:%d > MAX_COMPONENT_JPEGDEC:%d\n",
              __func__, __LINE__, numjpegdecInstance, MAX_COMPONENT_JPEGDEC);
        return OMX_ErrorInsufficientResources;
    }

    /** initialising mad structures */

    return err;
}

/** The destructor */
OMX_ERRORTYPE omx_mjpegdec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp)
{

    omx_mjpegdec_component_PrivateType *omx_mjpegdec_component_Private = openmaxStandComp->pComponentPrivate;
    OMX_U32 i;

    /* frees port/s */
    if (omx_mjpegdec_component_Private->ports)
    {
        for (i = 0; i < omx_mjpegdec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++)
        {
            if (omx_mjpegdec_component_Private->ports[i])
            {
                omx_mjpegdec_component_Private->ports[i]->PortDestructor(omx_mjpegdec_component_Private->ports[i]);
            }
        }
        free(omx_mjpegdec_component_Private->ports);
        omx_mjpegdec_component_Private->ports = NULL;
    }

    DEBUG(DEB_LEV_FUNCTION_NAME, "Destructor of mad decoder component is called\n");

    omx_base_filter_Destructor(openmaxStandComp);
    numjpegdecInstance--;

    return OMX_ErrorNone;
}

/** The Initialization function  */
OMX_ERRORTYPE omx_mjpegdec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp)
{

    omx_mjpegdec_component_PrivateType *omx_mjpegdec_component_Private = openmaxStandComp->pComponentPrivate;
    OMX_ERRORTYPE err = OMX_ErrorNone;

    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s \n", __func__);

    return err;
}

/** The Deinitialization function  */
OMX_ERRORTYPE omx_mjpegdec_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp)
{

    //omx_mjpegdec_component_PrivateType* omx_mjpegdec_component_Private = openmaxStandComp->pComponentPrivate;
    OMX_ERRORTYPE err = OMX_ErrorNone;

    return err;
}

/** this function sets the parameter values regarding format & index */
OMX_ERRORTYPE omx_mjpegdec_component_SetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR ComponentParameterStructure)
{

    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_PARAM_COMPONENTROLETYPE *pComponentRole;
    OMX_U32 portIndex;

    /* Check which structure we are being fed and make control its header */
    OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
    omx_mjpegdec_component_PrivateType *omx_mjpegdec_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *port;
    if (ComponentParameterStructure == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nParamIndex);
    switch (nParamIndex)
    {
    case OMX_IndexParamPortDefinition:
    {
        err = omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
        if (err == OMX_ErrorNone)
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
            portIndex = pPortDef->nPortIndex;
            port = (omx_base_video_PortType *)omx_mjpegdec_component_Private->ports[portIndex];

            if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
            {
                omx_base_video_PortType *outPort =
                    (omx_base_video_PortType *)omx_mjpegdec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
                outPort->sPortParam.format.video.nFrameWidth = JPU_CEIL(16, port->sPortParam.format.video.nFrameWidth); // 16-bit align
                outPort->sPortParam.format.video.nFrameHeight = JPU_CEIL(16, port->sPortParam.format.video.nFrameHeight);
                outPort->sPortParam.format.video.nStride = outPort->sPortParam.format.video.nFrameWidth;
                outPort->sPortParam.format.video.nSliceHeight = outPort->sPortParam.format.video.nFrameHeight;

                omx_mjpegdec_component_Private->omxOutputPortCrop.nLeft = 0;
                omx_mjpegdec_component_Private->omxOutputPortCrop.nTop = 0;
                omx_mjpegdec_component_Private->omxOutputPortCrop.nLeft = port->sPortParam.format.video.nFrameWidth;
                omx_mjpegdec_component_Private->omxOutputPortCrop.nTop = port->sPortParam.format.video.nFrameHeight;

                outPort->sPortParam.nBufferSize = outPort->sPortParam.format.video.nFrameWidth *
                                                  outPort->sPortParam.format.video.nFrameHeight * 3 / 2; // defalut output format is I420
            }

            DEBUG(DEB_LEV_SIMPLE_SEQ, "Setting parameter OMX_IndexParamPortDefinition nPortIndex=%d, %dx%d, stride %d, "
                                      "eColorFormat=%d, sVideoParam.eColorFormat=%d, eCompressionFormat=0x%x, xFrameRate=%d\n",
                  (int)portIndex, (int)port->sPortParam.format.video.nFrameWidth, (int)port->sPortParam.format.video.nFrameHeight,
                  (int)port->sPortParam.format.video.nStride, (int)pPortDef->format.video.eColorFormat,
                  (int)port->sVideoParam.eColorFormat, (int)pPortDef->format.video.eCompressionFormat,
                  (int)pPortDef->format.video.xFramerate);
        }
        break;
    }
    case OMX_IndexParamVideoPortFormat:
    {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat;
        pVideoPortFormat = ComponentParameterStructure;
        portIndex = pVideoPortFormat->nPortIndex;
        err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoPortFormat,
                                                      sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
        if (err != OMX_ErrorNone)
        {
            DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, err);
            break;
        }

        port = (omx_base_video_PortType *)omx_mjpegdec_component_Private->ports[portIndex];
        memcpy(&port->sVideoParam, pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
        omx_mjpegdec_component_Private->ports[portIndex]->sPortParam.format.video.eColorFormat =
            pVideoPortFormat->eColorFormat;
        omx_mjpegdec_component_Private->ports[portIndex]->sPortParam.format.video.eCompressionFormat =
            pVideoPortFormat->eCompressionFormat;

        DEBUG(DEB_LEV_SIMPLE_SEQ, "Setting parameter OMX_IndexParamVideoPortFormat portIndex=%d, nIndex=%d, "
                                  "eColorFormat=0x%x, eCompressionFormat=0x%x\n",
              (int)portIndex, (int)pVideoPortFormat->nIndex,
              pVideoPortFormat->eColorFormat, (int)pVideoPortFormat->eCompressionFormat);
        break;
    }
    case OMX_IndexParamStandardComponentRole:
        pComponentRole = (OMX_PARAM_COMPONENTROLETYPE *)ComponentParameterStructure;
        if (!strcmp((char *)pComponentRole->cRole, JPU_DECODER_ROLE))
        {
        }
        else
        {
            DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, err);
            return OMX_ErrorBadParameter;
        }
        //omx_mjpegdec_component_SetInternalParameters(openmaxStandComp);
        break;
    case OMX_IndexParamCompBufferSupplier:
    {
        OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplierType = (OMX_PARAM_BUFFERSUPPLIERTYPE *)ComponentParameterStructure;
        err = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
        if (err != OMX_ErrorNone)
        {
            DEBUG(DEB_LEV_ERR, "In %s Parameter nParamIndex:0x%x Check Error=%x\n",
                  __func__, nParamIndex, (int)err);
            break;
        }

        if (bufferSupplierType->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX &&
            bufferSupplierType->eBufferSupplier == OMX_BufferSupplyVendorDRM)
        {
            omx_mjpegdec_component_Private->useNativeBuffer = OMX_TRUE;
            DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_IndexParamCompBufferSupplier useNativeBuffer=%d\n",
                  (int)omx_mjpegdec_component_Private->useNativeBuffer);
        }
        break;
    }
    default: /*Call the base component function*/
        return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    }
    return err;
}

/** this function gets the parameters regarding formats and index */
OMX_ERRORTYPE omx_mjpegdec_component_GetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{

    OMX_PARAM_COMPONENTROLETYPE *pComponentRole;
    omx_base_video_PortType *port;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_U32 portIndex;
    OMX_U32 nIndex;
    OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat;

    OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
    omx_mjpegdec_component_PrivateType *omx_mjpegdec_component_Private = openmaxStandComp->pComponentPrivate;
    if (ComponentParameterStructure == NULL)
    {
        return OMX_ErrorBadParameter;
    }
    DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting parameter %i\n", nParamIndex);
    /* Check which structure we are being fed and fill its header */
    switch (nParamIndex)
    {
    case OMX_IndexParamVideoInit:
        if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone)
        {
            break;
        }
        memcpy(ComponentParameterStructure,
               &omx_mjpegdec_component_Private->sPortTypesParam[OMX_PortDomainVideo],
               sizeof(OMX_PORT_PARAM_TYPE));
        break;

    case OMX_IndexParamVideoPortFormat:
        pVideoPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
        if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone)
        {
            break;
        }

        portIndex = pVideoPortFormat->nPortIndex;
        nIndex = pVideoPortFormat->nIndex;
        port = (omx_base_video_PortType *)omx_mjpegdec_component_Private->ports[pVideoPortFormat->nPortIndex];

        if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
        {
            pVideoPortFormat->eColorFormat = OMX_COLOR_FormatUnused;
            pVideoPortFormat->eCompressionFormat = OMX_VIDEO_CodingMJPEG;
        }
        else if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
        {
            pVideoPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
            if (nIndex == 0)
                pVideoPortFormat->eColorFormat = OMX_COLOR_FormatYUV420Planar;
            else
                err = OMX_ErrorNoMore;
        }
        else
        {
            DEBUG(DEB_LEV_ERR, "Getting parameter: OMX_IndexParamVideoPortFormat OMX_ErrorBadPortIndex\n");
            return OMX_ErrorBadPortIndex;
        }
        break;

    case OMX_IndexParamStandardComponentRole:
        pComponentRole = (OMX_PARAM_COMPONENTROLETYPE *)ComponentParameterStructure;
        if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone)
        {
            break;
        }
        strcpy((char *)pComponentRole->cRole, JPU_DECODER_ROLE);
        break;
    default: /*Call the base component function*/
        return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    }
    return err;
}

OMX_ERRORTYPE omx_mjpegdec_component_SetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure)
{
    OMX_U32 portIndex;
    /* Check which structure we are being fed and make control its header */
    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_mjpegdec_component_PrivateType *omx_mjpegdec_component_Private = openmaxStandComp->pComponentPrivate;
    // Possible configs to set
    OMX_CONFIG_RECTTYPE *omxConfigCrop;
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (pComponentConfigStructure == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting configuration 0x%08x\n", nParamIndex);

    switch (nParamIndex)
    {
    case OMX_IndexConfigCommonOutputCrop:
        omxConfigCrop = (OMX_CONFIG_RECTTYPE *)pComponentConfigStructure;
        portIndex = omxConfigCrop->nPortIndex;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_RECTTYPE))) != OMX_ErrorNone)
        {
            break;
        }
        if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
        {
            omx_mjpegdec_component_Private->omxOutputPortCrop.nLeft = omxConfigCrop->nLeft;
            omx_mjpegdec_component_Private->omxOutputPortCrop.nTop = omxConfigCrop->nTop;
            omx_mjpegdec_component_Private->omxOutputPortCrop.nWidth = omxConfigCrop->nWidth;
            omx_mjpegdec_component_Private->omxOutputPortCrop.nHeight = omxConfigCrop->nHeight;

            DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting configuration OMX_IndexConfigCommonOutputCrop nLeft=%d, "
                                      "nTop=%d, nWidth=%d, nHeight=%d\n",
                  (int)omxConfigCrop->nLeft, (int)omxConfigCrop->nTop,
                  (int)omxConfigCrop->nWidth, (int)omxConfigCrop->nHeight);
        }
        else
        {
            return OMX_ErrorBadPortIndex;
        }
        break;

    default:
        return omx_base_component_SetConfig(hComponent, nParamIndex, pComponentConfigStructure);
        break;
    }
    return err;
}

OMX_ERRORTYPE omx_mjpegdec_component_GetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure)
{
    // Possible configs to ask for
    OMX_CONFIG_RECTTYPE *omxConfigCrop;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
    omx_mjpegdec_component_PrivateType *omx_mjpegdec_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *pPort;
    if (pComponentConfigStructure == NULL)
    {
        return OMX_ErrorBadParameter;
    }
    DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting configuration 0x%x\n", nParamIndex);
    /* Check which structure we are being fed and fill its header */
    switch (nParamIndex)
    {
    case OMX_IndexConfigCommonOutputCrop:
        omxConfigCrop = (OMX_CONFIG_RECTTYPE *)pComponentConfigStructure;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_RECTTYPE))) != OMX_ErrorNone)
        {
            break;
        }
        if (omxConfigCrop->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
        {
            memcpy(omxConfigCrop, &omx_mjpegdec_component_Private->omxOutputPortCrop, sizeof(OMX_CONFIG_RECTTYPE));
        }
        else if (omxConfigCrop->nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
        {
            return OMX_ErrorUnsupportedIndex;
        }
        else
        {
            return OMX_ErrorBadPortIndex;
        }

        DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting configuration OMX_IndexConfigCommonOutputCrop "
                                  "nLeft=%d, nTop=%d, nWidth=%d, nHeight=%d\n",
              (int)omxConfigCrop->nLeft, (int)omxConfigCrop->nTop,
              (int)omxConfigCrop->nWidth, (int)omxConfigCrop->nHeight);
        break;
    default:
        DEBUG(DEB_LEV_SIMPLE_SEQ, "unknown index\n");
        return OMX_ErrorUnsupportedIndex;
    }
    return err;
}

void dec_fill_input_buffer(void *private_ptr, int expect_size, void *input_buffer, int *fill_input_size)
{
    int feed_size = 0;
    OMX_BUFFERHEADERTYPE *pInputBuffer = (OMX_BUFFERHEADERTYPE *)private_ptr;

    feed_size = expect_size < pInputBuffer->nFilledLen ? expect_size : pInputBuffer->nFilledLen;
    memcpy(input_buffer, pInputBuffer->pBuffer + pInputBuffer->nOffset, feed_size);

    pInputBuffer->nOffset += feed_size;
    pInputBuffer->nFilledLen -= feed_size;
    *fill_input_size = feed_size;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "pInputBuffer:%x, expect_size:%d, feed_size:%d, "
                              "pInputBuffer->nFilledLen:%d, pInputBuffer->nOffset:%d\n",
          pInputBuffer, expect_size, feed_size, pInputBuffer->nFilledLen, pInputBuffer->nOffset);

    // dump input jpeg file
    // {
    //   static int i = 0;
    //   i++;
    //   char input_name[20] = {0};
    //   sprintf(input_name, "jpeg_%d.jpg", i);
    //   FILE * fp = fopen(input_name, "wb");
    //   fwrite(pInputBuffer->pBuffer, 1, pInputBuffer->nOffset , fp);
    //   fclose(fp);
    // }
    return;
}

OMX_ERRORTYPE omx_jpudec_component_flush_port_buffer(
    omx_base_PortType *pPort,
    OMX_BUFFERHEADERTYPE **ppBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    tsem_t *pSem = pPort->pBufferSem;
    queue_t *pQueue = pPort->pBufferQueue;
    OMX_BUFFERHEADERTYPE *pBuffer = *ppBuffer;

    do
    {
        DEBUG(DEB_LEV_FULL_SEQ, "Ports are flushing, so returning buffer = %p\n", pBuffer);
        if (pBuffer)
        {
            pBuffer->nFilledLen = 0;
            pBuffer->nTimeStamp = 0;
            pPort->ReturnBufferFunction((omx_base_PortType *)pPort, pBuffer);
            pBuffer = NULL;
        }

        if (pSem->semval > 0)
        {
            tsem_down(pSem);
            if (pQueue->nelem > 0)
            {
                pBuffer = dequeue(pQueue);
            }
        }
    } while (pBuffer);

    *ppBuffer = NULL;

    return err;
}

void *omx_mjpegdec_component_BufferMgmtFunction(void *param)
{
    OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)param;
    omx_mjpegdec_component_PrivateType *omx_mjpegdec_component_Private = openmaxStandComp->pComponentPrivate;

    omx_base_PortType *pInPort = (omx_base_PortType *)omx_mjpegdec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_base_PortType *pOutPort = (omx_base_PortType *)omx_mjpegdec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    tsem_t *pInputSem = pInPort->pBufferSem;
    tsem_t *pOutputSem = pOutPort->pBufferSem;
    queue_t *pInputQueue = pInPort->pBufferQueue;
    queue_t *pOutputQueue = pOutPort->pBufferQueue;
    OMX_BUFFERHEADERTYPE *pOutputBuffer = NULL;
    OMX_BUFFERHEADERTYPE *pInputBuffer = NULL;
    OMX_BOOL isInputBufferNeeded = OMX_TRUE, isOutputBufferNeeded = OMX_TRUE;
    int inBufExchanged = 0, outBufExchanged = 0;
    int ret = 0;
    double dec_start_ts = 0.0;
    double dec_end_ts = 0.0;
    double dec_return_ts = 0.0;
    OMX_ERRORTYPE err = OMX_ErrorHardware;
    OMX_S32 width, height;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, thread id:%i\n", __func__, (long int)syscall(__NR_gettid));
    while (!omx_mjpegdec_component_Private->bIsBufMgThreadExit &&
           (omx_mjpegdec_component_Private->state == OMX_StateIdle ||
            omx_mjpegdec_component_Private->state == OMX_StateExecuting ||
            omx_mjpegdec_component_Private->state == OMX_StatePause ||
            omx_mjpegdec_component_Private->transientState == OMX_TransStateLoadedToIdle))
    {

        pthread_mutex_lock(&omx_mjpegdec_component_Private->flush_mutex);
        while (PORT_IS_BEING_FLUSHED(pInPort) ||
               PORT_IS_BEING_FLUSHED(pOutPort))
        {
            pthread_mutex_unlock(&omx_mjpegdec_component_Private->flush_mutex);

            DEBUG(DEB_LEV_FULL_SEQ, "In %s 1 signalling flush all cond iE=%d,iF=%d,oE=%d,oF=%d "
                                    "iSemVal=%d,oSemval=%d\n",
                  __func__, inBufExchanged, isInputBufferNeeded, outBufExchanged,
                  isOutputBufferNeeded, pInputSem->semval, pOutputSem->semval);

            if (PORT_IS_BEING_FLUSHED(pOutPort))
            {
                omx_jpudec_component_flush_port_buffer(pOutPort, &pOutputBuffer);
                isOutputBufferNeeded = OMX_TRUE;
            }

            if (PORT_IS_BEING_FLUSHED(pInPort))
            {
                omx_jpudec_component_flush_port_buffer(pInPort, &pInputBuffer);
                isInputBufferNeeded = OMX_TRUE;
            }

            DEBUG(DEB_LEV_FULL_SEQ, "In %s 2 signalling flush all cond iE=%d,iF=%d,oE=%d,oF=%d "
                                    "iSemVal=%d,oSemval=%d\n",
                  __func__, inBufExchanged, isInputBufferNeeded, outBufExchanged,
                  isOutputBufferNeeded, pInputSem->semval, pOutputSem->semval);

            tsem_up(omx_mjpegdec_component_Private->flush_all_condition);
            tsem_down(omx_mjpegdec_component_Private->flush_condition);
            pthread_mutex_lock(&omx_mjpegdec_component_Private->flush_mutex);
        }
        pthread_mutex_unlock(&omx_mjpegdec_component_Private->flush_mutex);

        if ((isInputBufferNeeded == OMX_TRUE && pInputSem->semval == 0) &&
            (omx_mjpegdec_component_Private->state != OMX_StateLoaded &&
             omx_mjpegdec_component_Private->state != OMX_StateInvalid) &&
            !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
        {
            DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next input buffer omx_mjpegdec_component_Private->state=0x%x\n",
                  (int)omx_mjpegdec_component_Private->state);
            tsem_timed_down(omx_mjpegdec_component_Private->bMgmtSem, 30);
        }

        if (omx_mjpegdec_component_Private->state == OMX_StateLoaded ||
            omx_mjpegdec_component_Private->state == OMX_StateInvalid)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "In %s Input Buffer Management Thread is exiting\n", __func__);
            break;
        }

        if ((isOutputBufferNeeded == OMX_TRUE && pOutputSem->semval == 0) &&
            (omx_mjpegdec_component_Private->state != OMX_StateLoaded &&
             omx_mjpegdec_component_Private->state != OMX_StateInvalid) &&
            !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
        {
            //Signalled from EmptyThisBuffer or FillThisBuffer or some thing else
            DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next output buffer\n");
            tsem_timed_down(omx_mjpegdec_component_Private->bMgmtSem, 30);
        }

        if (omx_mjpegdec_component_Private->state == OMX_StateLoaded ||
            omx_mjpegdec_component_Private->state == OMX_StateInvalid)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "In %s Output Buffer Management Thread is exiting\n", __func__);
            break;
        }

        DEBUG(DEB_LEV_FULL_SEQ, "Waiting for input/output buffer in queue, input semval=%d output semval=%d in %s\n",
              (int)pInputSem->semval, (int)pOutputSem->semval, __func__);

        /*When we have input buffer to process then get one output buffer*/
        if (pOutputSem->semval > 0 && isOutputBufferNeeded == OMX_TRUE)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "in isOutputBufferNeeded pOutputQueue->nelem %d\n", pOutputQueue->nelem);
            tsem_down(pOutputSem);
            if (pOutputQueue->nelem > 0)
            {
                outBufExchanged++;
                isOutputBufferNeeded = OMX_FALSE;
                pOutputBuffer = dequeue(pOutputQueue);
                if (pOutputBuffer == NULL)
                {
                    DEBUG(DEB_LEV_ERR, "Had NULL output buffer!! op is=%d,iq=%d\n",
                          pOutputSem->semval, pOutputQueue->nelem);
                    break;
                }
            }
        }

        if (pInputSem->semval > 0 && isInputBufferNeeded == OMX_TRUE)
        {
            tsem_down(pInputSem);
            if (pInputQueue->nelem > 0)
            {
                inBufExchanged++;
                isInputBufferNeeded = OMX_FALSE;
                pInputBuffer = dequeue(pInputQueue);
                if (pInputBuffer == NULL)
                {
                    DEBUG(DEB_LEV_ERR, "Had NULL input buffer!!\n");
                    break;
                }
            }
        }

        if (isInputBufferNeeded == OMX_FALSE && pInputBuffer->hMarkTargetComponent != NULL)
        {
            if ((OMX_COMPONENTTYPE *)pInputBuffer->hMarkTargetComponent == (OMX_COMPONENTTYPE *)openmaxStandComp)
            {
                /*Clear the mark and generate an event*/
                (*(omx_mjpegdec_component_Private->callbacks->EventHandler))(openmaxStandComp,
                                                                             omx_mjpegdec_component_Private->callbackData,
                                                                             OMX_EventMark, /* The command was completed */
                                                                             1,             /* The commands was a OMX_CommandStateSet */
                                                                             0,             /* The state has been changed in message->messageParam2 */
                                                                             pInputBuffer->pMarkData);
            }
            else
            {
                omx_mjpegdec_component_Private->pMark.hMarkTargetComponent = pInputBuffer->hMarkTargetComponent;
                omx_mjpegdec_component_Private->pMark.pMarkData = pInputBuffer->pMarkData;
            }

            pInputBuffer->hMarkTargetComponent = NULL;
        }

        if (isInputBufferNeeded == OMX_FALSE && isOutputBufferNeeded == OMX_FALSE)
        {
            if (omx_mjpegdec_component_Private->pMark.hMarkTargetComponent != NULL)
            {
                pOutputBuffer->hMarkTargetComponent = omx_mjpegdec_component_Private->pMark.hMarkTargetComponent;
                pOutputBuffer->pMarkData = omx_mjpegdec_component_Private->pMark.pMarkData;
                omx_mjpegdec_component_Private->pMark.hMarkTargetComponent = NULL;
                omx_mjpegdec_component_Private->pMark.pMarkData = NULL;
            }

            if (omx_mjpegdec_component_Private->state == OMX_StateExecuting)
            {
                if (pInputBuffer->nFilledLen > 0)
                {
                    if (!omx_mjpegdec_component_Private->jpu)
                    {
                        omx_mjpegdec_component_Private->jpu = jpu_hw_init();
                        omx_mjpegdec_component_Private->jpu->fill_buffer_callback = dec_fill_input_buffer;

                        //set output format to OMX_COLOR_FormatYUV420Planar
                        omx_mjpegdec_component_Private->jpu->dec_feedong_method = FEED_METHOD_FRAME_SIZE;
                        omx_mjpegdec_component_Private->jpu->dec_output.output_cbcrInterleave = CBCR_SEPARATED;
                        omx_mjpegdec_component_Private->jpu->dec_output.output_packed_format = PACKED_FORMAT_NONE;
                        omx_mjpegdec_component_Private->jpu->dec_output.output_format = FORMAT_420;
                    }

                    if (!dec_return_ts)
                        dec_return_ts = GetNowMs();
                    dec_start_ts = GetNowMs();
                    omx_mjpegdec_component_Private->jpu->user_private_ptr = pInputBuffer;
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "pInputBuffer:%x pInputBuffer->nFilledLen:%d, "
                                              "pInputBuffer->nTimeStamp:%lld pOutputBuffer:%x\n",
                          pInputBuffer, pInputBuffer->nFilledLen,
                          (long long)pInputBuffer->nTimeStamp, pOutputBuffer);

                    if (omx_mjpegdec_component_Private->useNativeBuffer)
                    {
                        DEBUG(DEB_LEV_SIMPLE_SEQ, "*pOutputBuffer->pBuffer:%d\n",
                              *(OMX_U32 *)pOutputBuffer->pBuffer);

                        ret = jpu_decompress_16_bit_align(omx_mjpegdec_component_Private->jpu,
                                                    OMX_TRUE, *(OMX_U32 *)pOutputBuffer->pBuffer);
                        if (ret < 0)
                        {
                            DEBUG(DEB_LEV_ERR, "jpu_decompress_16_bit_align failed\n");
                            pOutputBuffer->nFilledLen = 0;
                            pInputBuffer->nFilledLen = 0;
                            if (omx_mjpegdec_component_Private->callbacks->EventHandler)
                                (*(omx_mjpegdec_component_Private->callbacks->EventHandler))(openmaxStandComp,
                                                                                             omx_mjpegdec_component_Private->callbackData,
                                                                                             OMX_EventError,
                                                                                             err,
                                                                                             0,
                                                                                             NULL);
                        }

                        if (omx_mjpegdec_component_Private->jpu->dec_output.current_output_index >= 0)
                        {
                            DEBUG(DEB_LEV_FULL_SEQ, "[%s][%d] pic_width:%d pic_height:%d pic_format:%d "
                                                    "output_width:%d output_height:%d output_cbcrInterleave:%d "
                                                    "output_packed_format:%d output_format_size:%d "
                                                    "jpu->dec_output.bit_depth:%d y_size:%d u_size:%d v_size:%d\n",
                                  __func__, __LINE__, omx_mjpegdec_component_Private->jpu->dec_output.pic_width,
                                  omx_mjpegdec_component_Private->jpu->dec_output.pic_height,
                                  omx_mjpegdec_component_Private->jpu->dec_output.pic_format,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_width,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_height,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_cbcrInterleave,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_packed_format,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_frame_size,
                                  omx_mjpegdec_component_Private->jpu->dec_output.bit_depth,
                                  omx_mjpegdec_component_Private->jpu->dec_output.y_size,
                                  omx_mjpegdec_component_Private->jpu->dec_output.u_size,
                                  omx_mjpegdec_component_Private->jpu->dec_output.v_size);

                            pOutputBuffer->nFilledLen = omx_mjpegdec_component_Private->jpu->dec_output.output_frame_size;
                        }
                        else
                        {
                            pOutputBuffer->nFilledLen = 0;
                        }
                    }
                    else
                    {

                        ret = jpu_decompress_16_bit_align(omx_mjpegdec_component_Private->jpu, OMX_FALSE, 0);
                        if (ret < 0)
                        {
                            DEBUG(DEB_LEV_ERR, "jpu_decompress failed\n");
                            pOutputBuffer->nFilledLen = 0;
                            pInputBuffer->nFilledLen = 0;
                            if (omx_mjpegdec_component_Private->callbacks->EventHandler)
                                (*(omx_mjpegdec_component_Private->callbacks->EventHandler))(openmaxStandComp,
                                                                                             omx_mjpegdec_component_Private->callbackData,
                                                                                             OMX_EventError,
                                                                                             err,
                                                                                             0,
                                                                                             NULL);
                        }

                        if (omx_mjpegdec_component_Private->jpu->dec_output.current_output_index >= 0)
                        {
                            DEBUG(DEB_LEV_FULL_SEQ, "[%s][%d] pic_width:%d pic_height:%d pic_format:%d "
                                                    "output_width:%d output_height:%d output_cbcrInterleave:%d "
                                                    "output_packed_format:%d output_format_size:%d "
                                                    "jpu->dec_output.bit_depth:%d y_size:%d u_size:%d v_size:%d\n",
                                  __func__, __LINE__, omx_mjpegdec_component_Private->jpu->dec_output.pic_width,
                                  omx_mjpegdec_component_Private->jpu->dec_output.pic_height,
                                  omx_mjpegdec_component_Private->jpu->dec_output.pic_format,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_width,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_height,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_cbcrInterleave,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_packed_format,
                                  omx_mjpegdec_component_Private->jpu->dec_output.output_frame_size,
                                  omx_mjpegdec_component_Private->jpu->dec_output.bit_depth,
                                  omx_mjpegdec_component_Private->jpu->dec_output.y_size,
                                  omx_mjpegdec_component_Private->jpu->dec_output.u_size,
                                  omx_mjpegdec_component_Private->jpu->dec_output.v_size);

                            pOutputBuffer->nFilledLen = 0;
                            if (omx_mjpegdec_component_Private->jpu->dec_output.y_ptr)
                            {
                                memcpy(pOutputBuffer->pBuffer + pOutputBuffer->nFilledLen,
                                       omx_mjpegdec_component_Private->jpu->dec_output.y_ptr,
                                       omx_mjpegdec_component_Private->jpu->dec_output.y_size);
                                pOutputBuffer->nFilledLen += omx_mjpegdec_component_Private->jpu->dec_output.y_size;
                            }
                            if (omx_mjpegdec_component_Private->jpu->dec_output.u_ptr)
                            {
                                memcpy(pOutputBuffer->pBuffer + pOutputBuffer->nFilledLen,
                                       omx_mjpegdec_component_Private->jpu->dec_output.u_ptr,
                                       omx_mjpegdec_component_Private->jpu->dec_output.u_size);
                                pOutputBuffer->nFilledLen += omx_mjpegdec_component_Private->jpu->dec_output.u_size;
                            }
                            if (omx_mjpegdec_component_Private->jpu->dec_output.v_ptr)
                            {
                                memcpy(pOutputBuffer->pBuffer + pOutputBuffer->nFilledLen,
                                       omx_mjpegdec_component_Private->jpu->dec_output.v_ptr,
                                       omx_mjpegdec_component_Private->jpu->dec_output.v_size);
                                pOutputBuffer->nFilledLen += omx_mjpegdec_component_Private->jpu->dec_output.v_size;
                            }
                        }
                        else
                        {
                            DEBUG(DEB_LEV_ERR, "jpu dec output index is an invalid value:%d\n",
                                  omx_mjpegdec_component_Private->jpu->dec_output.current_output_index);
                            pOutputBuffer->nFilledLen = 0;
                        }
                    }

                    dec_end_ts = GetNowMs();
                    pOutputBuffer->nTimeStamp = pInputBuffer->nTimeStamp;
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "decode time=%.1fms, pts %lld\n",
                          dec_end_ts - dec_start_ts, pOutputBuffer->nTimeStamp);
                }

                if (pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS)
                {
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "EOS\n");
                    pOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
                }
            }
            else if (!(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
            {
                DEBUG(DEB_LEV_ERR, "In %s Received Buffer in non-Executing State(%x)\n",
                      __func__, (int)omx_mjpegdec_component_Private->state);
            }
            else
            {
                pInputBuffer->nFilledLen = 0;
            }

            if (omx_mjpegdec_component_Private->state == OMX_StatePause &&
                !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
                tsem_wait(omx_mjpegdec_component_Private->bStateSem); //paused state

            if (pOutputBuffer &&
                (pOutputBuffer->nFilledLen != 0 ||
                 ((pOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS)))
            {
                double cur_ms = GetNowMs();
                DEBUG(DEB_LEV_FULL_SEQ, "one frame consumption:%.1fms\n", cur_ms - dec_return_ts);
                dec_return_ts = cur_ms;

                pOutPort->ReturnBufferFunction((omx_base_PortType *)pOutPort, pOutputBuffer);

                if (pOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS)
                {
                    (*(omx_mjpegdec_component_Private->callbacks->EventHandler))(openmaxStandComp,
                                                                                 omx_mjpegdec_component_Private->callbackData,
                                                                                 OMX_EventBufferFlag,
                                                                                 1, /* port index should be output port index */
                                                                                 pOutputBuffer->nFlags,
                                                                                 NULL);
                }

                outBufExchanged--;
                pOutputBuffer = NULL;
                isOutputBufferNeeded = OMX_TRUE;
                DEBUG(DEB_LEV_FULL_SEQ, "after ReturnBufferFunction outBufExchanged %d\n", outBufExchanged);
            }

            if (omx_mjpegdec_component_Private->state == OMX_StatePause &&
                !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
                tsem_wait(omx_mjpegdec_component_Private->bStateSem); //paused state

            if (isInputBufferNeeded == OMX_FALSE && pInputBuffer->nFilledLen == 0)
            {
                pInPort->ReturnBufferFunction((omx_base_PortType *)pInPort, pInputBuffer);
                inBufExchanged--;
                pInputBuffer = NULL;
                isInputBufferNeeded = OMX_TRUE;
                DEBUG(DEB_LEV_FULL_SEQ, "after ReturnBufferFunction inBufExchanged %d\n", inBufExchanged);
            }
        }
    }
    omx_mjpegdec_component_Private->bIsBufMgThreadExit = OMX_TRUE;
    if (omx_mjpegdec_component_Private->jpu)
    {
        jpu_hw_release(omx_mjpegdec_component_Private->jpu);
        omx_mjpegdec_component_Private->jpu = NULL;
    }
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s of component %p\n", __func__, openmaxStandComp);
    return NULL;
}

OMX_ERRORTYPE omx_mjpegdec_decoder_MessageHandler(OMX_COMPONENTTYPE *openmaxStandComp, internalRequestMessageType *message)
{
    omx_mjpegdec_component_PrivateType *omx_mjpegdec_component_Private =
        (omx_mjpegdec_component_PrivateType *)openmaxStandComp->pComponentPrivate;
    OMX_ERRORTYPE err;
    OMX_STATETYPE eCurrentState = omx_mjpegdec_component_Private->state;
    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s\n", __func__);

    if (message->messageType == OMX_CommandStateSet)
    {
        if ((message->messageParam == OMX_StateIdle) && (omx_mjpegdec_component_Private->state == OMX_StateLoaded))
        {
            err = omx_mjpegdec_component_Init(openmaxStandComp);
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s MAD Decoder Init Failed Error=%x\n", __func__, err);
                return err;
            }
        }
    }
    /** Execute the base message handling */
    err = omx_base_component_MessageHandler(openmaxStandComp, message);

    if (message->messageType == OMX_CommandStateSet)
    {
        if ((message->messageParam == OMX_StateLoaded) && (eCurrentState == OMX_StateIdle))
        {
            err = omx_mjpegdec_component_Deinit(openmaxStandComp);
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s MAD Decoder Deinit Failed Error=%x\n", __func__, err);
                return err;
            }
        }
    }
    return err;
}
