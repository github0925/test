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

#include <math.h>
#include <errno.h>
#include "omx_utils.h"
#include "omxcore.h"
#include "OMX_Video.h"
#include "main_helper.h"
#include "OMX_VPU_Video.h"
#include "omx_base_video_port.h"
#include "omx_vpuenc_component.h"
#include "coda9/coda9_vpuconfig.h"

#ifdef ANDROID
#include "android_support.h"
#include <cutils/sched_policy.h>
#endif

#include <sched.h>
#include <sys/resource.h>
#include "omx_utils.h"

#define MAKE_FOURCC(a,b,c,d) (OMX_U32)(((unsigned char)a) | ((unsigned char)b << 8) | ((unsigned char)c << 16) | ((unsigned char)d << 24))

#define DEFAULT_FRAME_WIDTH             352
#define DEFAULT_FRAME_HEIGHT            288
#define DEFAULT_FRAMERATE               30
#define DEFAULT_BITRATE_KBPS            0       //0 means Use for unknown, don't care or variable
#define DEFAULT_COLOR_FORMAT            OMX_COLOR_FormatYUV420Planar

#define DEFAULT_ACTUAL_VIDEO_OUTPUT_BUFFER_NUM  4
#define DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM     2
#define DEFAULT_ACTUAL_VIDEO_INPUT_BUFFER_NUM   2
#define DEFAULT_MIN_VIDEO_INPUT_BUFFER_NUM      2
#define DEFAULT_STREAM_BUF_SIZE                 0x400000
#define VPU_WAIT_TIME_OUT   10      //should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function
#define OMX_VPU_ENC_TIMEOUT                 1000
#define STREAM_READ_SIZE    (512 * 16)
#define COLOR_DEPTH_8_BIT       8
#define COLOR_DEPTH_10_BIT      10
#define ANDROID_PRIORITY_VIDEO  -10
#define OMX_IndexParamConsumerUsageBits 0x6F800004

// the following two defs are extended by vendor, please keep them consistent with
// the vendor extension of media framework
#define OMX_Extension_IndexPortParamBufferMode (OMX_IndexVendorStartUnused + 1)
#define OMX_VIDEO_CodingVendorDmaMode (OMX_VIDEO_CodingVendorStartUnused + 1)

static enc_config_t s_enc_config;
static CodStd codingTypeToCodStd(OMX_VIDEO_CODINGTYPE codingType);
static void OmxCheckVersion(int coreIdx);
static void SetInternalVideoEncParameters(OMX_COMPONENTTYPE *openmaxStandComp);
static int OmxGetVpuBsBufferByVirtualAddress(vpu_enc_context_t *pVpu, vpu_buffer_t *vb, unsigned long virt_addr);
static int OmxGetInputBufferIndex(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer);
static void omx_vpuenc_component_vpuLibDeInit(omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private);
static OMX_BOOL OmxConfigEncoderParameter(OMX_COMPONENTTYPE *openmaxStandComp);
static inline void OmxWriteYuvData(OMX_BYTE pYuv, vpu_enc_context_t *pVpu, FrameBuffer* fb, OMX_BOOL reAlign);
#ifdef ANDROID
static OMX_U32 OmxGetCorrespondColorFormat(OMX_U32 grallocFormat, omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private);
static OMX_ERRORTYPE OmxGetCorrespondPhysAddr(vpu_enc_context_t *pVpu, OMX_S32 bufHandle, PhysicalAddress *physAddr);
#endif
static long debug_enc_dump;

#ifdef FILE_ENC_DUMP
void OMX_DUMP_INPUT_YUV_TO_FILE(vpu_enc_context_t *pVpu, OMX_BYTE pYuv, OMX_U32 size)
{
    FILE *fp = NULL;
    int errNum = 0;

    if (debug_enc_dump != DUMP_INPUT && debug_enc_dump != DUMP_BOTH)
        return;
    if (pVpu->inputDumpIdx == 0)
    {
        if ((fp = fopen(pVpu->dumpInputFn, "w+b")))
        {
            fseek(fp, 0, SEEK_SET);
            fclose(fp);
        }
        else
        {
            errNum = errno;
            DEBUG(DEB_LEV_FULL_SEQ, "In %s, errNum %d reason %s \n", __func__, errNum, strerror(errNum));
        }
    }

    if ((fp = fopen(pVpu->dumpInputFn, "a+b")))
    {
        fwrite(pYuv, size, 1, fp);
        pVpu->inputDumpIdx++;
        fclose(fp);
    }
    else
    {
        errNum = errno;
        DEBUG(DEB_LEV_FULL_SEQ, "In %s, errNum %d reason %s \n", __func__, errNum,strerror(errNum));
    }
}

void OMX_ZEROCOPY_INPUT_BUF_DUMP(omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private)
{
    void *pFrame = NULL;
    int grallocFormat = HAL_PIXEL_FORMAT_RGBA_8888;   // default to HAL_PIXEL_FORMAT_RGBA_8888
    DEBUG(DEB_LEV_FULL_SEQ, "In %s,  \n", __func__);
    vpu_enc_context_t *pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;
    getAndroidNativeBufferHandleInfo(omx_vpuenc_component_Private->buffer_handle, &grallocFormat, NULL, NULL, NULL, NULL);
    lockAndroidBufferHandle(omx_vpuenc_component_Private->buffer_handle, pVpu->srcFrameStride, pVpu->srcFrameHeight, LOCK_MODE_TO_GET_VIRTUAL_ADDRESS, &pFrame);
    OMX_DUMP_INPUT_YUV_TO_FILE(pVpu, pFrame, pVpu->srcFrameBufSize);
    unLockAndroidBufferHandle(omx_vpuenc_component_Private->buffer_handle);
}

void OMX_DUMP_OUTPUT_TO_FILE(vpu_enc_context_t *pVpu, OMX_BUFFERHEADERTYPE* pOutputBuffer)
{
    FILE *fp = NULL;
    int errNum = 0;
    if (debug_enc_dump != DUMP_OUTPUT && debug_enc_dump != DUMP_BOTH)
      return;

    if (pVpu->outputDumpIdx == 0)
    {
        if ((fp = fopen(pVpu->dumpOutputFn, "w+b")))
        {
            fseek(fp, 0, SEEK_SET);
            fclose(fp);
        }
        else
        {
            errNum = errno;
            DEBUG(DEB_LEV_FULL_SEQ, "In %s, errNum %d reason %s \n", __func__, errNum, strerror(errNum));
        }
    }
    DEBUG(DEB_LEV_FULL_SEQ, "In %s, filled len is %d \n", __func__, pOutputBuffer->nFilledLen);
    if ((fp = fopen(pVpu->dumpOutputFn, "a+b")))
    {
        DEBUG(DEB_LEV_FULL_SEQ, "In %s, filled len is %d \n", __func__, pOutputBuffer->nFilledLen);
        fwrite(pOutputBuffer->pBuffer, pOutputBuffer->nFilledLen, 1, fp);
        pVpu->outputDumpIdx++;
        fclose(fp);
    }
    else
    {
        errNum = errno;
        DEBUG(DEB_LEV_FULL_SEQ, "In %s, errNum %d reason %s \n", __func__, errNum,strerror(errNum));
    }
}
#else
#define OMX_DUMP_INPUT_YUV_TO_FILE(pVpu, pYuv, size)
#define OMX_ZEROCOPY_INPUT_BUF_DUMP(omx_vpuenc_component_Private)
#define OMX_DUMP_OUTPUT_TO_FILE(pVpu, pOutputBuffer)
#define GENERATE_FILE_NAMES(pVpu, FILE_NAME)
#endif


/** The Constructor of the video encoder component
* @param openmaxStandComp the component handle to be constructed
* @param cComponentName is the name of the constructed component
*/
OMX_ERRORTYPE omx_vpuenc_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName)
{

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private;
    omx_base_video_PortType *inPort,*outPort;
    vpu_enc_context_t *pVpu;
    RetCode ret = RETCODE_SUCCESS;
    GetDebugLevelFromProperty(DEBUG_LOG_COMP, DEB_LEV_ERR);
    GetOmxDumpLevelFromProperty(DEBUG_ENC_DUMP, 0, &debug_enc_dump);

    if (!openmaxStandComp->pComponentPrivate) {
        DEBUG(DEB_LEV_FULL_SEQ, "In %s, allocating component\n", __func__);
        openmaxStandComp->pComponentPrivate = malloc(sizeof(omx_vpuenc_component_PrivateType));
        if(openmaxStandComp->pComponentPrivate == NULL) {
            DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
            eError = OMX_ErrorInsufficientResources;
            goto ERROR_OMX_VPU_ENC_COMPONENT_CONSTRUCTOR;
        }
        memset(openmaxStandComp->pComponentPrivate,0x00, sizeof(omx_vpuenc_component_PrivateType));
    } else {
        DEBUG(DEB_LEV_FULL_SEQ, "In %s, Error Component %p Already Allocated\n", __func__, openmaxStandComp->pComponentPrivate);
    }

    omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    omx_vpuenc_component_Private->ports = NULL;

    /** we could create our own port structures here
    * fixme maybe the base class could use a "port factory" function pointer?
    */
    eError = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

    omx_vpuenc_component_Private->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber = 0;
    omx_vpuenc_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts = 2;

    /** Allocate Ports and call port constructor. */
    if (omx_vpuenc_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts && !omx_vpuenc_component_Private->ports) {
        OMX_U32 i;
        omx_vpuenc_component_Private->ports = malloc(omx_vpuenc_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts*sizeof(omx_base_PortType *));
        if (!omx_vpuenc_component_Private->ports) {
            DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
            eError = OMX_ErrorInsufficientResources;
            goto ERROR_OMX_VPU_ENC_COMPONENT_CONSTRUCTOR;
        }
        memset(omx_vpuenc_component_Private->ports, 0x00, omx_vpuenc_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts*sizeof(omx_base_PortType *));

        for (i=0; i < omx_vpuenc_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++) {
            omx_vpuenc_component_Private->ports[i] = malloc(sizeof(omx_base_video_PortType));
            if (!omx_vpuenc_component_Private->ports[i]) {
                DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
                eError = OMX_ErrorInsufficientResources;
                goto ERROR_OMX_VPU_ENC_COMPONENT_CONSTRUCTOR;
            }
            memset(omx_vpuenc_component_Private->ports[i],0x00, sizeof(omx_base_video_PortType));
        }
    }

    base_video_port_Constructor(openmaxStandComp, &omx_vpuenc_component_Private->ports[0], 0, OMX_TRUE);
    base_video_port_Constructor(openmaxStandComp, &omx_vpuenc_component_Private->ports[1], 1, OMX_FALSE);


    /** Domain specific section for the ports.
    * first we set the parameter common to both formats
    */

    /* set default parameters to input/output port */
    inPort = (omx_base_video_PortType *) omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    inPort->sPortParam.eDomain = OMX_PortDomainVideo;
    inPort->sPortParam.format.video.nFrameWidth = DEFAULT_FRAME_WIDTH;
    inPort->sPortParam.format.video.nFrameHeight = DEFAULT_FRAME_HEIGHT;
    inPort->sPortParam.nBufferSize = DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT * 3 / 2;
    inPort->sPortParam.format.video.xFramerate = (DEFAULT_FRAMERATE<<16);
    inPort->sPortParam.format.video.eColorFormat = DEFAULT_COLOR_FORMAT;
    inPort->sPortParam.nBufferCountActual = DEFAULT_ACTUAL_VIDEO_INPUT_BUFFER_NUM;
    inPort->sPortParam.nBufferCountMin = DEFAULT_MIN_VIDEO_INPUT_BUFFER_NUM;

    inPort->sVideoParam.eColorFormat = DEFAULT_COLOR_FORMAT;
    inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingUnused;

    eError = GetVideoCodingTypeByName(cComponentName, &omx_vpuenc_component_Private->video_encoding_type, OMX_TRUE);
    if (OMX_ErrorNone != eError) {
        DEBUG(DEB_LEV_ERR, "%s:%d Invalid component name:%s\n", __func__, __LINE__, cComponentName);
        goto ERROR_OMX_VPU_ENC_COMPONENT_CONSTRUCTOR;
    }

    outPort = (omx_base_video_PortType *) omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    outPort->sPortParam.eDomain = OMX_PortDomainVideo;
    outPort->sPortParam.nBufferSize = DEFAULT_STREAM_BUF_SIZE;
    outPort->sPortParam.format.video.xFramerate = (DEFAULT_FRAMERATE<<16);
    outPort->sPortParam.format.video.nBitrate = (DEFAULT_BITRATE_KBPS*1024);
    outPort->sPortParam.format.video.nFrameWidth = DEFAULT_FRAME_WIDTH;
    outPort->sPortParam.format.video.nFrameHeight = DEFAULT_FRAME_HEIGHT;
    outPort->sPortParam.format.video.eCompressionFormat = omx_vpuenc_component_Private->video_encoding_type;
    outPort->sPortParam.nBufferCountActual = DEFAULT_ACTUAL_VIDEO_OUTPUT_BUFFER_NUM;
    outPort->sPortParam.nBufferCountMin = DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM;
    outPort->sVideoParam.eColorFormat = OMX_COLOR_FormatUnused;
    outPort->sVideoParam.eCompressionFormat = omx_vpuenc_component_Private->video_encoding_type;


    omx_vpuenc_component_Private->codParam.bitrate.eControlRate = OMX_Video_ControlRateDisable;
    omx_vpuenc_component_Private->codParam.bitrate.nTargetBitrate = (DEFAULT_BITRATE_KBPS*1024);

    omx_vpuenc_component_Private->codParam.intraRefresh.eRefreshMode = OMX_VIDEO_IntraRefreshMax;
    omx_vpuenc_component_Private->codParam.intraRefresh.nCirMBs = 0;
    omx_vpuenc_component_Private->codParam.intraRefresh.nAirMBs = 0;
    omx_vpuenc_component_Private->codParam.intraRefresh.nAirRef = 0;

    omx_vpuenc_component_Private->codParam.quantization.nQpI = 20;//default Qp

    SetInternalVideoEncParameters(openmaxStandComp);

    omx_vpuenc_component_Private->vpuReady = OMX_FALSE;
    omx_vpuenc_component_Private->bFormatConversion = OMX_FALSE;
#ifdef ANDROID
    omx_vpuenc_component_Private->metadata_buffer_type = kMetadataBufferTypeInvalid;
#endif
    omx_vpuenc_component_Private->BufferMgmtCallback = NULL;//omx_vpuenc_component_BufferMgmtCallback;
    omx_vpuenc_component_Private->BufferMgmtFunction = omx_vpuenc_component_BufferMgmtFunction;

    /** initializing the coenc context etc that was done earlier by vpulibinit function */
    omx_vpuenc_component_Private->messageHandler = omx_vpuenc_component_MessageHandler;
    omx_vpuenc_component_Private->destructor = omx_vpuenc_component_Destructor;
    openmaxStandComp->SetParameter = omx_vpuenc_component_SetParameter;
    openmaxStandComp->GetParameter = omx_vpuenc_component_GetParameter;
#ifdef ANDROID
    openmaxStandComp->GetExtensionIndex = omx_vpuenc_component_GetExtensionIndex;
#endif
    openmaxStandComp->ComponentRoleEnum = omx_vpuenc_component_ComponentRoleEnum;

    openmaxStandComp->AllocateBuffer = omx_videoenc_component_AllocateBuffer;
    openmaxStandComp->UseBuffer = omx_videoenc_component_UseBuffer;
    openmaxStandComp->FreeBuffer = omx_videoenc_component_FreeBuffer;

    openmaxStandComp->SetConfig = omx_vpuenc_component_SetConfig;
    openmaxStandComp->GetConfig = omx_vpuenc_component_GetConfig;


    pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;
    memset(pVpu, 0x00, sizeof (vpu_enc_context_t));
    GENERATE_FILE_NAMES(pVpu, FILE_ENC_NAME);

    omx_vpuenc_component_Private->portSettingChangeRequest = OMX_FALSE;

    pVpu->productId = VPU_GetProductId(pVpu->coreIdx);

    {
        char*   path;
        Uint32  sizeInWord;
        Uint16 *pusBitCode;

        if (pVpu->coreIdx == 0) {
            path = CORE_1_BIT_CODE_FILE_PATH;
        }
        else
        {
            DEBUG(DEB_LEV_ERR, "Invalid core index: %d\n", pVpu->coreIdx);
            eError = OMX_ErrorHardware;
            goto ERROR_OMX_VPU_ENC_COMPONENT_CONSTRUCTOR;
        }

        char bitfile[32];
        sprintf(bitfile, "%s%s", FW_PATH, path);
        DEBUG(DEB_LEV_SIMPLE_SEQ, "VPU_GetProductId %x, core %d, bitfile %s\n", pVpu->productId, pVpu->coreIdx, bitfile);
        if (LoadFirmware(pVpu->productId, (Uint8**)&pusBitCode, &sizeInWord, bitfile) < 0) {
            DEBUG(DEB_LEV_ERR, "failed open bit_firmware file path is %s\n", bitfile);
            eError = OMX_ErrorHardware;
            goto ERROR_OMX_VPU_ENC_COMPONENT_CONSTRUCTOR;
        }

        ret = VPU_InitWithBitcode(pVpu->coreIdx, pusBitCode, sizeInWord);

        if (pusBitCode)
            osal_free(pusBitCode);
    }

    if (ret != RETCODE_SUCCESS && ret != RETCODE_CALLED_BEFORE)
    {
        DEBUG(DEB_LEV_ERR, "VPU_Init failed Error code is 0x%x \n", (int)ret);
        eError = OMX_ErrorHardware;
        goto ERROR_OMX_VPU_ENC_COMPONENT_CONSTRUCTOR;
    }

    eError = RM_RegisterComponent(cComponentName, MAX_ENCODER_COMPONENTS);
    if (eError != OMX_ErrorNone) {
        DEBUG(DEB_LEV_ERR, "In %s, RM_RegisterComponent failed Error code is 0x%x\n", __func__, eError);
        goto ERROR_OMX_VPU_ENC_COMPONENT_CONSTRUCTOR;
    }
    DEBUG(DEB_LEV_SIMPLE_SEQ, "VPU_Init success\n");
    return OMX_ErrorNone;

ERROR_OMX_VPU_ENC_COMPONENT_CONSTRUCTOR:
    omx_vpuenc_component_Destructor(openmaxStandComp);
    return eError;
}


OMX_ERRORTYPE omx_vpuenc_component_GetExtensionIndex(
    OMX_IN OMX_HANDLETYPE  hComponent,
    OMX_IN OMX_STRING      cParameterName,
    OMX_OUT OMX_INDEXTYPE *pIndexType)
{

#ifdef ANDROID
    DEBUG(DEB_LEV_FUNCTION_NAME,"In  %s, cParameterName = %s \n",__func__, cParameterName);
    if (strcmp(cParameterName, STR_INDEX_PARAM_STORE_METADATA_BUFFER) == 0)
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexParamStoreMetaDataBuffer;
        return OMX_ErrorNone;
    }
    if (strcmp(cParameterName, STR_INDEX_PARAM_STORE_ANWBUFFER_IN_METADATA) == 0)
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexParamStoreANWBufferInMetadata;
        return OMX_ErrorNone;
    }
#endif
    return omx_base_component_GetExtensionIndex(hComponent, cParameterName, pIndexType);
}


/** The destructor of the video encoder component
*/
OMX_ERRORTYPE omx_vpuenc_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp) {
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_enc_context_t *pVpu= (vpu_enc_context_t *)&omx_vpuenc_component_Private->vpu;

    DEBUG(DEB_LEV_FULL_SEQ, "In %s,\n", __func__);

    if (!omx_vpuenc_component_Private->ports)
    {
        DEBUG(DEB_LEV_SIMPLE_SEQ, "Destructor of video encoder component is already called\n");
        return OMX_ErrorNone;
    }

    if (omx_vpuenc_component_Private->vpuReady) {
        omx_vpuenc_component_vpuLibDeInit(omx_vpuenc_component_Private);
        omx_vpuenc_component_Private->vpuReady = OMX_FALSE;
    }

    VPU_DeInit(pVpu->coreIdx);

    DEBUG(DEB_LEV_FULL_SEQ, "Destructor of video encoder component is called\n");
    omx_base_filter_Destructor(openmaxStandComp);
    /* frees port/s */
    if (omx_vpuenc_component_Private->ports) {
        OMX_U32 i;
        for (i=0; i < omx_vpuenc_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++) {
            if(omx_vpuenc_component_Private->ports[i])
                omx_vpuenc_component_Private->ports[i]->PortDestructor(omx_vpuenc_component_Private->ports[i]);
        }
        free(omx_vpuenc_component_Private->ports);
        omx_vpuenc_component_Private->ports=NULL;
    }

    DEBUG(DEB_LEV_FULL_SEQ, "Out %s,\n", __func__);
    return OMX_ErrorNone;
}


/** It initializates the VPU framework, and opens an VPU videoencoder of type specified by IL client
*/
OMX_ERRORTYPE omx_vpuenc_component_vpuLibInit(omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private) {

    omx_base_video_PortType *inPort = (omx_base_video_PortType *) omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_base_video_PortType *outPort = (omx_base_video_PortType *) omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];

    vpu_enc_context_t *pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;
    OMX_ERRORTYPE error = OMX_ErrorHardware;
    RetCode ret = RETCODE_SUCCESS;
    unsigned long maxAddr = 0;
    int i;
    OMX_U32 eColorFormat;
#ifdef ANDROID
    int grallocFormat = 0;
#endif

    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
    OmxCheckVersion(pVpu->coreIdx);

    pVpu->encOP.bitstreamFormat = codingTypeToCodStd(omx_vpuenc_component_Private->video_encoding_type);
    if (-1 == (int)pVpu->encOP.bitstreamFormat)
    {
        DEBUG(DEB_LEV_ERR, "can not found Codec codingType is 0x%x \n", (int)omx_vpuenc_component_Private->video_encoding_type);
        error = OMX_ErrorComponentNotFound;
        goto ERR_VPU_ENC_INIT;
    }
    if (pVpu->productId == PRODUCT_ID_420)
        pVpu->mapType = COMPRESSED_FRAME_MAP;
    else
        pVpu->mapType = (s_enc_config.mapType & 0x0f);
    pVpu->encOP.linear2TiledEnable = (s_enc_config.mapType>>4)&0x1;
    if (pVpu->encOP.linear2TiledEnable)
        pVpu->encOP.linear2TiledMode = FF_FIELD;    // coda980 only
    pVpu->encOP.picWidth        = outPort->sPortParam.format.video.nFrameWidth;
    pVpu->encOP.picHeight       = outPort->sPortParam.format.video.nFrameHeight;
#ifdef ANDROID
    //TODO: shall be outPort xFramerate?
    pVpu->encOP.frameRateInfo   = inPort->sPortParam.format.video.xFramerate>>16;
#else
    pVpu->encOP.frameRateInfo   = outPort->sPortParam.format.video.xFramerate>>16;
#endif
    pVpu->encOP.bitRate         = outPort->sPortParam.format.video.nBitrate/1024;

    eColorFormat = inPort->sPortParam.format.video.eColorFormat;
#ifdef ANDROID
    DEBUG(DEB_LEV_FULL_SEQ, "%s storeMetadata %d, frameRateInfo %d\n", __func__, omx_vpuenc_component_Private->storeMetaDataInInputBuffer, pVpu->encOP.frameRateInfo);
    if (OMX_TRUE == omx_vpuenc_component_Private->storeMetaDataInInputBuffer)
    {

        if (omx_vpuenc_component_Private->metadata_buffer_type == kMetadataBufferTypeGrallocSource
            || omx_vpuenc_component_Private->metadata_buffer_type == kMetadataBufferTypeANWBuffer)
        {
            // update gralloc format, stride and height from private_handle_t
            getAndroidNativeBufferHandleInfo(omx_vpuenc_component_Private->buffer_handle,
                                               &grallocFormat, NULL, &pVpu->srcFrameHeight, &pVpu->srcFrameStride, NULL);
        }

        if (inPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatAndroidOpaque) // need to consider gralloc_format
        {
            eColorFormat = OmxGetCorrespondColorFormat(grallocFormat, omx_vpuenc_component_Private);
        }
    }
#endif
    pVpu->encOP.cbcrOrder = CBCR_ORDER_NORMAL;
    switch(eColorFormat)
    {
    case MAKE_FOURCC('N', 'V', '1', '2'):
    case OMX_COLOR_FormatYUV420SemiPlanar:
    case OMX_COLOR_FormatYUV420PackedSemiPlanar:
        {
            pVpu->encOP.cbcrInterleave = 1;
            pVpu->encOP.nv21 = 0;
        }
        break;
    case MAKE_FOURCC('Y', 'V', '1', '2'):
        {
            pVpu->encOP.cbcrOrder = CBCR_ORDER_REVERSED;
            pVpu->encOP.cbcrInterleave = 0;
            pVpu->encOP.nv21 = 0;
        }
        break;
    case OMX_COLOR_FormatYUV420Planar:
    case OMX_COLOR_FormatYUV420PackedPlanar:
        {
            pVpu->encOP.cbcrInterleave = 0;
            pVpu->encOP.nv21 = 0;
        }
        break;

    case MAKE_FOURCC('N', 'V', '2', '1'):
        {
            pVpu->encOP.cbcrInterleave = 1;
            pVpu->encOP.nv21 = 1;
        }
        break;
    default:
        {
            DEBUG(DEB_LEV_ERR, "not supported video input format\n" );
            error = OMX_ErrorUnsupportedSetting;
            goto ERR_VPU_ENC_INIT;
        }
        break;
    }

    pVpu->encOP.maxIntraSize = 0;
    /* using minimum search range to match linebuffer size
     * TODO: auto-config seaching range as resolution changing
     */
    pVpu->encOP.MESearchRangeX = 3; // x range 16
    pVpu->encOP.MESearchRangeY = 2; // y range 16
    pVpu->encOP.rcGopIQpOffsetEn = 0;
    pVpu->encOP.rcGopIQpOffset = 0;
#define INITIAL_DELAY 500 //set initial decoder buffer delay 0 by default
#define VBV_BUFFER_SIZE 0       // max to optimize
    pVpu->encOP.initialDelay    = INITIAL_DELAY;
    pVpu->encOP.vbvBufferSize   = VBV_BUFFER_SIZE; // 0 = ignore
    pVpu->encOP.meBlkMode   = 0; // for compare with C-model ( C-model = only 0 )
    pVpu->encOP.MEUseZeroPmv = 0;
    pVpu->encOP.frameSkipDisable  = 1; // for compare with C-model ( C-model = only 1 )
    pVpu->encOP.gopSize         = 0; // only first picture is I
    pVpu->encOP.idrInterval     = 0;
    pVpu->encOP.sliceMode.sliceMode = 0; // 1 slice per picture
    pVpu->encOP.sliceMode.sliceSizeMode = 0;
    pVpu->encOP.sliceMode.sliceSize = 8192;
    pVpu->encOP.rcIntraQp       = -1; // disable : -1
    pVpu->encOP.userQpMax       = -1; // disable : -1, range : 3~31
    pVpu->encOP.userGamma       = -1; // default value.
    pVpu->encOP.rcIntervalMode  = 0; // 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level
    pVpu->encOP.mbInterval      = 0;
    pVpu->encOP.rcEnable    = 0;
    pVpu->encOP.intraCostWeight = 0;

    if (omx_vpuenc_component_Private->codParam.intraRefresh.eRefreshMode == OMX_VIDEO_IntraRefreshMax ||
        omx_vpuenc_component_Private->codParam.intraRefresh.nCirMBs == 0)
        pVpu->encOP.intraRefresh = 0;
    else
        pVpu->encOP.intraRefresh = omx_vpuenc_component_Private->codParam.intraRefresh.nCirMBs;
    pVpu->encOP.ConscIntraRefreshEnable = 0;
    pVpu->encOP.CountIntraMbEnable = 0;
    pVpu->encOP.FieldSeqIntraRefreshEnable = 0;

    // Standard specificbuffer_out_sizebi
    if (pVpu->encOP.bitstreamFormat == STD_MPEG4)
    {
        pVpu->encOP.EncStdParam.mp4Param.mp4DataPartitionEnable = omx_vpuenc_component_Private->codParam.errorrection.bEnableDataPartitioning;
        pVpu->encOP.EncStdParam.mp4Param.mp4ReversibleVlcEnable = omx_vpuenc_component_Private->codParam.errorrection.bEnableRVLC;
        pVpu->encOP.EncStdParam.mp4Param.mp4HecEnable = omx_vpuenc_component_Private->codParam.errorrection.bEnableHEC;
        pVpu->encOP.EncStdParam.mp4Param.mp4IntraDcVlcThr = 0;
        pVpu->encOP.EncStdParam.mp4Param.mp4Verid = 2;
        pVpu->encOP.MEUseZeroPmv = 0;
#ifdef ANDROID
        if ((omx_vpuenc_component_Private->codParam.mpeg4.nPFrames % pVpu->encOP.frameRateInfo) == 0) // to workaround setPFramesSpacing of Acodec.cpp ( do not -1 to make nPFrames )
            pVpu->encOP.gopSize = (omx_vpuenc_component_Private->codParam.mpeg4.nPFrames);
        else
            pVpu->encOP.gopSize = (omx_vpuenc_component_Private->codParam.mpeg4.nPFrames + 1);
#else
        pVpu->encOP.gopSize = (omx_vpuenc_component_Private->codParam.mpeg4.nPFrames + 1);
#endif
        {
            int indexProfile = 0;
            int tempVal = omx_vpuenc_component_Private->codParam.mpeg4.eProfile;
            static int profileTable[16] = {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

            while(1)
            {
                tempVal /= 2;

                if(tempVal == 0)
                    break;

                indexProfile++;
            }
            if(profileTable[indexProfile] == -1)
            {
                DEBUG(DEB_LEV_ERR, "CNM support simple profiles\n");
                goto ERR_VPU_ENC_INIT;
            }
        }

        if (omx_vpuenc_component_Private->codParam.bitrate.nTargetBitrate != 0)
            pVpu->encOP.bitRate = omx_vpuenc_component_Private->codParam.bitrate.nTargetBitrate/1024;

        if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateVariable)
            pVpu->encOP.rcEnable = 1;
        else if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateConstant)
            pVpu->encOP.rcEnable = 1;
        else if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateVariableSkipFrames)
        {
            pVpu->encOP.rcEnable = 1;
            pVpu->encOP.frameSkipDisable  = 0;
        }
        else if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateConstantSkipFrames)
        {
            pVpu->encOP.rcEnable = 1;
            pVpu->encOP.frameSkipDisable = 0;
        }
        else
        {
            pVpu->encOP.rcEnable = 0;
            pVpu->encOP.bitRate = 0;
        }
    }
    else if (pVpu->encOP.bitstreamFormat == STD_H263)
    {
        pVpu->encOP.EncStdParam.h263Param.h263AnnexIEnable = 0;
        pVpu->encOP.EncStdParam.h263Param.h263AnnexJEnable = 0;
        pVpu->encOP.EncStdParam.h263Param.h263AnnexKEnable = 0;
        pVpu->encOP.EncStdParam.h263Param.h263AnnexTEnable = 0;
        pVpu->encOP.MEUseZeroPmv = 0;

#ifdef ANDROID
        if ((omx_vpuenc_component_Private->codParam.h263.nPFrames % pVpu->encOP.frameRateInfo) == 0) // to consider setPFramesSpacing of Acodec.cpp
            pVpu->encOP.gopSize = (omx_vpuenc_component_Private->codParam.h263.nPFrames);
        else
            pVpu->encOP.gopSize = (omx_vpuenc_component_Private->codParam.h263.nPFrames + 1);
#else
        pVpu->encOP.gopSize = (omx_vpuenc_component_Private->codParam.h263.nPFrames + 1);
#endif
        {
            int indexProfile = 0;
            int tempVal = omx_vpuenc_component_Private->codParam.h263.eProfile;
            static int profileTable[] = {0, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

            while(1)
            {
                tempVal /= 2;

                if(tempVal == 0)
                    break;

                indexProfile++;
            }
            if(profileTable[indexProfile] == -1)
            {
                DEBUG(DEB_LEV_ERR, "CNM support simple profiles\n");
                goto ERR_VPU_ENC_INIT;
            }

            if(profileTable[indexProfile] == 1)
            {
                pVpu->encOP.EncStdParam.h263Param.h263AnnexJEnable = 1;
                pVpu->encOP.EncStdParam.h263Param.h263AnnexKEnable = 1;
                pVpu->encOP.EncStdParam.h263Param.h263AnnexTEnable = 1;
            }
        }

        if (omx_vpuenc_component_Private->codParam.bitrate.nTargetBitrate != 0)
            pVpu->encOP.bitRate = omx_vpuenc_component_Private->codParam.bitrate.nTargetBitrate/1024;

        if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateVariable)
            pVpu->encOP.rcEnable = 1;
        else if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateConstant)
            pVpu->encOP.rcEnable = 1;
        else if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateVariableSkipFrames)
        {
            pVpu->encOP.rcEnable = 1;
            pVpu->encOP.frameSkipDisable = 0;
        }
        else if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateConstantSkipFrames)
        {
            pVpu->encOP.rcEnable = 1;
            pVpu->encOP.frameSkipDisable = 0;
        }
        else
        {
            pVpu->encOP.rcEnable = 0;
            pVpu->encOP.bitRate = 0;
        }
    }
    else if (pVpu->encOP.bitstreamFormat == STD_AVC)
    {
        pVpu->encOP.EncStdParam.avcParam.constrainedIntraPredFlag = 0;
        pVpu->encOP.EncStdParam.avcParam.disableDeblk = 0;
        pVpu->encOP.EncStdParam.avcParam.deblkFilterOffsetAlpha = 0;
        pVpu->encOP.EncStdParam.avcParam.deblkFilterOffsetBeta = 0;
        pVpu->encOP.EncStdParam.avcParam.chromaQpOffset = 0;
        pVpu->encOP.EncStdParam.avcParam.audEnable = 0;
        pVpu->encOP.EncStdParam.avcParam.frameCroppingFlag = 0;
        pVpu->encOP.EncStdParam.avcParam.frameCropLeft = 0;
        pVpu->encOP.EncStdParam.avcParam.frameCropRight = 0;
        pVpu->encOP.EncStdParam.avcParam.frameCropTop = 0;
        pVpu->encOP.EncStdParam.avcParam.frameCropBottom = 0;
        pVpu->encOP.EncStdParam.avcParam.level = 0;
        pVpu->encOP.EncStdParam.avcParam.profile = 0;

        if ((pVpu->encOP.picHeight % 16) != 0 || (pVpu->encOP.picWidth % 16) != 0)
        {
            // In case of AVC encoder, when we want to use unaligned display width(For example, 1080),
            // frameCroppingFlag parameters should be adjusted to displayable rectangle
            if (s_enc_config.rotAngle != 90 && s_enc_config.rotAngle != 270) // except rotation
            {
                pVpu->encOP.EncStdParam.avcParam.frameCroppingFlag = 1;
                // frameCropBottomOffset = picHeight(MB-aligned) - displayable rectangle height
                pVpu->encOP.EncStdParam.avcParam.frameCropBottom = VPU_ALIGN16(pVpu->encOP.picHeight) - pVpu->encOP.picHeight;
                // frameCropRight = picWidth(MB-aligned) - displayable rectangle width
                pVpu->encOP.EncStdParam.avcParam.frameCropRight  = VPU_ALIGN16(pVpu->encOP.picWidth) - pVpu->encOP.picWidth;
            }
        }


        {
            int indexProfile = 0;
            int tempVal = omx_vpuenc_component_Private->codParam.avc.eProfile;
            int profileTable[7] = {0, 1, -1, 2, -1, -1, -1};

            while(1)
            {
                tempVal /= 2;

                if(tempVal == 0)
                    break;

                indexProfile++;
            }
            if(profileTable[indexProfile] == -1)
            {
                DEBUG(DEB_LEV_ERR, "CNM support simple profiles\n");
                error = OMX_ErrorUnsupportedSetting;
                goto ERR_VPU_ENC_INIT;
            }
            pVpu->encOP.EncStdParam.avcParam.profile = profileTable[indexProfile];
        }
#ifdef ANDROID
        if ((omx_vpuenc_component_Private->codParam.avc.nPFrames % pVpu->encOP.frameRateInfo) == 0) // to consider setPFramesSpacing of Acodec.cpp
            pVpu->encOP.gopSize = (omx_vpuenc_component_Private->codParam.avc.nPFrames);
        else
            pVpu->encOP.gopSize = (omx_vpuenc_component_Private->codParam.avc.nPFrames + 1);
#else
        pVpu->encOP.gopSize = (omx_vpuenc_component_Private->codParam.avc.nPFrames + 1);
#endif
        pVpu->encOP.EncStdParam.avcParam.level = omx_vpuenc_component_Private->codParam.avc.eLevel;
        {
            int indexLevel = 0;
            int tempVal = omx_vpuenc_component_Private->codParam.avc.eLevel;
            int levelTable[16] = {10, 9, 11, 12, 13, 20, 21, 22, 30, 31, 32, 40, 41, 42, 50, 51};

            while(1)
            {
                tempVal /= 2;

                if(tempVal == 0)
                    break;

                indexLevel++;
            }
            pVpu->encOP.EncStdParam.avcParam.level = levelTable[indexLevel];
        }

        if (omx_vpuenc_component_Private->codParam.bitrate.nTargetBitrate != 0)
            pVpu->encOP.bitRate = omx_vpuenc_component_Private->codParam.bitrate.nTargetBitrate/1024;

        if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateVariable)
            pVpu->encOP.rcEnable = 2;
        else if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateConstant)
            pVpu->encOP.rcEnable = 1;
        else if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateVariableSkipFrames)
        {
            pVpu->encOP.rcEnable = 2;
            pVpu->encOP.frameSkipDisable = 0;
        }
        else if (omx_vpuenc_component_Private->codParam.bitrate.eControlRate == OMX_Video_ControlRateConstantSkipFrames)
        {
            pVpu->encOP.rcEnable = 1;
            pVpu->encOP.frameSkipDisable = 0;
        }
        else
        {
            pVpu->encOP.rcEnable = 0;
            pVpu->encOP.bitRate = 0;
        }

        pVpu->encOP.maxIntraSize = 0;
        pVpu->encOP.userQpMin = -1; // disable : -1
        pVpu->encOP.userMinDeltaQp = -1; // disable : -1
        pVpu->encOP.userMaxDeltaQp  = -1; // disable : -1
        if (pVpu->encOP.rcEnable == 2)
            pVpu->encOP.userQpMax = 32; // for AVR

        if(pVpu->encOP.EncStdParam.avcParam.profile == 0)
        {
            pVpu->encOP.EncStdParam.avcParam.ppsParam[0].entropyCodingMode = 0;
            pVpu->encOP.EncStdParam.avcParam.ppsParam[0].transform8x8Mode  = 0;
        }
        else if(pVpu->encOP.EncStdParam.avcParam.profile == 1)
        {
            pVpu->encOP.EncStdParam.avcParam.ppsParam[0].entropyCodingMode = 1;
            pVpu->encOP.EncStdParam.avcParam.ppsParam[0].transform8x8Mode  = 0;
        }
        else if(pVpu->encOP.EncStdParam.avcParam.profile == 2)
        {
            pVpu->encOP.EncStdParam.avcParam.ppsParam[0].entropyCodingMode = 1;
            pVpu->encOP.EncStdParam.avcParam.ppsParam[0].transform8x8Mode  = 1;
        }
        else
            goto ERR_VPU_ENC_INIT;

        pVpu->encOP.EncStdParam.avcParam.ppsParam[0].ppsId             = 0;
        pVpu->encOP.EncStdParam.avcParam.ppsParam[0].cabacInitIdc      = 0;
        pVpu->encOP.EncStdParam.avcParam.ppsNum = 1;
        pVpu->encOP.EncStdParam.avcParam.chromaFormat400                = 0;
        pVpu->encOP.EncStdParam.avcParam.fieldFlag                      = 0;
        pVpu->encOP.EncStdParam.avcParam.fieldRefMode                   = 0;
    }
    else {
        DEBUG(DEB_LEV_ERR, "Invalid codec standard mode \n" );
        error = OMX_ErrorUnsupportedSetting;
        goto ERR_VPU_ENC_INIT;
    }

    if (pVpu->encOP.EncStdParam.avcParam.fieldFlag)
    {
        if (s_enc_config.rotAngle == 90 || s_enc_config.rotAngle == 270)
        {
            DEBUG(DEB_LEV_ERR, "Invalid Rotator Option in field mode\n");
            error = OMX_ErrorUnsupportedSetting;
            goto ERR_VPU_ENC_INIT;
        }
    }

    if (pVpu->encOP.bitstreamFormat == STD_AVC) {
        if (pVpu->srcFrameFormat == FORMAT_400)
            pVpu->encOP.EncStdParam.avcParam.chromaFormat400 = 1;
    }

    if (OMX_TRUE == omx_vpuenc_component_Private->bFormatConversion)
    {
        pVpu->privateYuv = (OMX_BYTE)malloc(pVpu->srcFrameBufSize);
        if(NULL == pVpu->privateYuv) {
            DEBUG(DEB_LEV_FULL_SEQ,"pVpu->privateYuv malloc failed\n");
            goto ERR_VPU_ENC_INIT;
        }
    }

    DEBUG(DEB_LEV_FULL_SEQ,"bUseOmxAllocateBufferOfOutPort %d\n", omx_vpuenc_component_Private->bUseOmxAllocateBufferOfOutPort);
    if (omx_vpuenc_component_Private->bUseOmxAllocateBufferOfOutPort == OMX_TRUE)  // OMX_AllocateBuffer case
    {
        pVpu->encOP.bitstreamBuffer = pVpu->vbStream[0].phys_addr;
        maxAddr = pVpu->vbStream[0].phys_addr;

        for (i=0; i < (int)outPort->sPortParam.nBufferCountActual; i++)
        {
            if (pVpu->vbStream[i].size > 0)
            {
                if (pVpu->vbStream[i].phys_addr < pVpu->encOP.bitstreamBuffer)
                    pVpu->encOP.bitstreamBuffer = pVpu->vbStream[i].phys_addr;

                if (pVpu->vbStream[i].phys_addr > maxAddr)
                    maxAddr = pVpu->vbStream[i].phys_addr;
            }
        }
        pVpu->encOP.bitstreamBufferSize = maxAddr - pVpu->encOP.bitstreamBuffer + pVpu->vbStream[0].size;
        DEBUG(DEB_LEV_SIMPLE_SEQ, "set bitstream buffer size = %d, addr=0x%x, count=%d\n", pVpu->encOP.bitstreamBufferSize, (int)pVpu->encOP.bitstreamBuffer, (int)outPort->sPortParam.nBufferCountActual);
    }
    else //OMX_UseBuffer case
    {
        pVpu->vbStream[0].size = DEFAULT_STREAM_BUF_SIZE;

        if (vdi_allocate_dma_memory(pVpu->coreIdx, &pVpu->vbStream[0]) < 0)
        {
            DEBUG(DEB_LEV_ERR, "fail to allocate bitstream buffer\n" );
            goto ERR_VPU_ENC_INIT;
        }
        pVpu->encOP.bitstreamBuffer = pVpu->vbStream[0].phys_addr;
        pVpu->encOP.bitstreamBufferSize = pVpu->vbStream[0].size;
        DEBUG(DEB_LEV_FULL_SEQ, "bitstreamBuffer 0x%x, size %d\n", pVpu->encOP.bitstreamBuffer, pVpu->encOP.bitstreamBufferSize);
    }

    pVpu->encOP.ringBufferEnable = 0;
    pVpu->encOP.lineBufIntEn = 0;
    pVpu->encOP.frameEndian    = VDI_LITTLE_ENDIAN;
    pVpu->encOP.streamEndian   = VDI_LITTLE_ENDIAN;
    pVpu->encOP.coreIdx = pVpu->coreIdx;

#ifdef SUPPORT_MULTIPLE_PPS // if SUPPORT_MULTIPLE_PPS is enabled. encoder can include multiple pps in bistream output.
    if (pVpu->encOP.bitstreamFormat == STD_AVC) {
        getAvcEncPPS(&pVpu->encOP); //add PPS before OPEN
    }
#endif
    DEBUG(DEB_LEV_SIMPLE_SEQ, "VPU_EncOpen Parameter : width : %d, height : %d, srcFrameStride : %d, srcFrameHeight : %d, srcFrameBufSize : %d, framebufStride : %d, frameRateInfo : %d, bitrate : %d\n", pVpu->encOP.picWidth, pVpu->encOP.picHeight, pVpu->srcFrameStride, pVpu->srcFrameHeight, pVpu->srcFrameBufSize, pVpu->srcFrameStride, pVpu->encOP.frameRateInfo, pVpu->encOP.bitRate);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "gopSize : %d, bitstreamFormat : %d, frameSkipDisable : %d, rcEnable : %d, intraRefresh : %d\n", pVpu->encOP.gopSize,  pVpu->encOP.bitstreamFormat, pVpu->encOP.frameSkipDisable, pVpu->encOP.rcEnable, pVpu->encOP.intraRefresh);
#ifdef ANDROID
    DEBUG(DEB_LEV_SIMPLE_SEQ, "eColorFormat : 0x%x, storeMetaDataInInputBuffer : %d, grallocFormat : 0x%x\n", (int)eColorFormat,  (int)omx_vpuenc_component_Private->storeMetaDataInInputBuffer, grallocFormat);
#endif

    // Open an instance and get initial information for encoding.
    ret = VPU_EncOpen(&pVpu->handle, &pVpu->encOP);
    if (ret != RETCODE_SUCCESS) {
        DEBUG(DEB_LEV_ERR, "VPU_EncOpen failed Error code is 0x%x \n", ret);
        goto ERR_VPU_ENC_INIT_PHASE;
    }

    if (VPU_GetOpenInstanceNum(pVpu->coreIdx) > MAX_NUM_INSTANCE)
    {
        DEBUG(DEB_LEV_ERR, "exceed the opened instance num than %d num\n", MAX_NUM_INSTANCE);
        error = OMX_ErrorNoMore;
        goto ERR_VPU_ENC_INIT_PHASE;
    }

    //VPU_EncGiveCommand(pVpu->handle, ENABLE_LOGGING, 0);

#define MAX_HEADER_SIZE 2048
    pVpu->pHeaderData = (BYTE *)malloc(MAX_HEADER_SIZE);
    if (!pVpu->pHeaderData) {
        DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
        goto ERR_VPU_ENC_INIT_PHASE;
    }

    omx_vpuenc_component_Private->idrPeriod = pVpu->encOP.gopSize; // set idrperid to gopsize in default to encoder all of I frame to IDR
    DEBUG(DEB_LEV_SIMPLE_SEQ, "VPU_EncOpen Success\n");
    return OMX_ErrorNone;


ERR_VPU_ENC_INIT_PHASE:
    if (omx_vpuenc_component_Private->bUseOmxAllocateBufferOfOutPort == OMX_FALSE)
        vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbStream[0]);
ERR_VPU_ENC_INIT:

    VPU_DeInit(pVpu->coreIdx);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "error =0x%x\n", error);

    return error;
}


/** It Deinitializates the vpuapi framework, and close the vpu video encoder of selected coding type
*/
void omx_vpuenc_component_vpuLibDeInit(omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private)
{
    vpu_enc_context_t *pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;
    RetCode ret = RETCODE_SUCCESS;

    ret = VPU_EncClose(pVpu->handle);

    if (ret == RETCODE_FRAME_NOT_COMPLETE)
    {
        VPU_EncGetOutputInfo(pVpu->handle, &pVpu->outputInfo);
        VPU_EncClose(pVpu->handle);
    }

    if (pVpu->pHeaderData)
    {
        free(pVpu->pHeaderData);
        pVpu->pHeaderData = NULL;
    }

    if (pVpu->privateYuv)
    {
        free(pVpu->privateYuv);
        pVpu->privateYuv = NULL;
    }

    if (omx_vpuenc_component_Private->bUseOmxAllocateBufferOfOutPort == OMX_FALSE)
        vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbStream[0]);

    for (int i = 0; i < pVpu->regFrameBufCount; i++) {
        if (pVpu->vbReconFrameBuf[i].size > 0) {
            vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbReconFrameBuf[i]);
        }
    }

    for (int i = 0; i < pVpu->fbAllocInfo.num; i++) {
        if (pVpu->vbSrcFb[i].size > 0) {
            vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbSrcFb[i]);
        }
    }
}


/** internal function to set coenc related parameters in the private type structure
*/
void SetInternalVideoEncParameters(OMX_COMPONENTTYPE *openmaxStandComp)
{

    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *outPort = (omx_base_video_PortType *)omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];

    if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingMPEG4) {
#ifdef SUPPORT_CM_OMX_12
#else
        strcpy(outPort->sPortParam.format.video.cMIMEType,"video/mpeg4");
#endif
        outPort->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG4;
        outPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingMPEG4;

        setHeader(&omx_vpuenc_component_Private->codParam.mpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
        omx_vpuenc_component_Private->codParam.mpeg4.nPortIndex = 1;
        omx_vpuenc_component_Private->codParam.mpeg4.nSliceHeaderSpacing = 0;
        omx_vpuenc_component_Private->codParam.mpeg4.bSVH = OMX_FALSE;
        omx_vpuenc_component_Private->codParam.mpeg4.bGov = OMX_FALSE;
        omx_vpuenc_component_Private->codParam.mpeg4.nPFrames = 0;
        omx_vpuenc_component_Private->codParam.mpeg4.nBFrames = 0;
        omx_vpuenc_component_Private->codParam.mpeg4.nIDCVLCThreshold = 0;
        omx_vpuenc_component_Private->codParam.mpeg4.bACPred = OMX_FALSE;
        omx_vpuenc_component_Private->codParam.mpeg4.nMaxPacketSize = 0;
        omx_vpuenc_component_Private->codParam.mpeg4.nTimeIncRes = 0;
        omx_vpuenc_component_Private->codParam.mpeg4.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
        omx_vpuenc_component_Private->codParam.mpeg4.eLevel = OMX_VIDEO_MPEG4Level0;
        omx_vpuenc_component_Private->codParam.mpeg4.nAllowedPictureTypes = 0;
        omx_vpuenc_component_Private->codParam.mpeg4.nHeaderExtension = 0;
        omx_vpuenc_component_Private->codParam.mpeg4.bReversibleVLC = OMX_FALSE;

    }
    else if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingH263)
    {
#ifdef SUPPORT_CM_OMX_12
#else
        strcpy(outPort->sPortParam.format.video.cMIMEType,"video/h263");
#endif
        outPort->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingH263;
        outPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingH263;

        setHeader(&omx_vpuenc_component_Private->codParam.h263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
        omx_vpuenc_component_Private->codParam.h263.nPortIndex = 1;
        omx_vpuenc_component_Private->codParam.h263.nPFrames = 0;
        omx_vpuenc_component_Private->codParam.h263.nBFrames = 0;
        omx_vpuenc_component_Private->codParam.h263.eProfile = OMX_VIDEO_H263ProfileBaseline ;
        omx_vpuenc_component_Private->codParam.h263.eLevel = OMX_VIDEO_H263Level10 ;
        omx_vpuenc_component_Private->codParam.h263.bPLUSPTYPEAllowed = 0;
        omx_vpuenc_component_Private->codParam.h263.nAllowedPictureTypes = 0;
        omx_vpuenc_component_Private->codParam.h263.bForceRoundingTypeToZero = 0;
        omx_vpuenc_component_Private->codParam.h263.nPictureHeaderRepetition = 0;
        omx_vpuenc_component_Private->codParam.h263.nGOBHeaderInterval = 0;
    }
    else if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingAVC) {
#ifdef SUPPORT_CM_OMX_12
#else
        strcpy(outPort->sPortParam.format.video.cMIMEType,"video/avc");
#endif
        outPort->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
        outPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingAVC;

        setHeader(&omx_vpuenc_component_Private->codParam.avc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
        omx_vpuenc_component_Private->codParam.avc.nPortIndex = 1;
        omx_vpuenc_component_Private->codParam.avc.nSliceHeaderSpacing = 0;
        omx_vpuenc_component_Private->codParam.avc.nPFrames = 30;
        omx_vpuenc_component_Private->codParam.avc.nBFrames = 0;
        omx_vpuenc_component_Private->codParam.avc.bUseHadamard = OMX_TRUE;
        omx_vpuenc_component_Private->codParam.avc.nRefFrames = 1;
        omx_vpuenc_component_Private->codParam.avc.nRefIdx10ActiveMinus1 = 0;
        omx_vpuenc_component_Private->codParam.avc.nRefIdx11ActiveMinus1 = 0;
        omx_vpuenc_component_Private->codParam.avc.bEnableUEP = OMX_TRUE;
        omx_vpuenc_component_Private->codParam.avc.bEnableFMO = OMX_TRUE;
        omx_vpuenc_component_Private->codParam.avc.bEnableASO = OMX_TRUE;
        omx_vpuenc_component_Private->codParam.avc.bEnableRS = OMX_TRUE;
        omx_vpuenc_component_Private->codParam.avc.eProfile = OMX_VIDEO_AVCProfileBaseline;
        omx_vpuenc_component_Private->codParam.avc.eLevel = OMX_VIDEO_AVCLevel4;
        omx_vpuenc_component_Private->codParam.avc.nAllowedPictureTypes = 0;
        omx_vpuenc_component_Private->codParam.avc.bFrameMBsOnly = OMX_FALSE;
        omx_vpuenc_component_Private->codParam.avc.bMBAFF = OMX_FALSE;
        omx_vpuenc_component_Private->codParam.avc.bEntropyCodingCABAC = OMX_TRUE;
        omx_vpuenc_component_Private->codParam.avc.bWeightedPPrediction = OMX_TRUE;
        omx_vpuenc_component_Private->codParam.avc.nWeightedBipredicitonMode = 0;
        omx_vpuenc_component_Private->codParam.avc.bconstIpred = OMX_TRUE;
        omx_vpuenc_component_Private->codParam.avc.bDirect8x8Inference = OMX_FALSE;
        omx_vpuenc_component_Private->codParam.avc.bDirectSpatialTemporal = OMX_FALSE;
        omx_vpuenc_component_Private->codParam.avc.nCabacInitIdc = 0;
        omx_vpuenc_component_Private->codParam.avc.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;
    }
}


/** The Initialization function of the video encoder
*/
OMX_ERRORTYPE omx_vpuenc_component_Init(OMX_COMPONENTTYPE *openmaxStandComp)
{
    UNREFERENCED_PARAMETER(openmaxStandComp);
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    s_enc_config.mapType = LINEAR_FRAME_MAP;

    s_enc_config.subFrameSyncEn = 0;
    s_enc_config.FrameCacheBypass   = 0;
    s_enc_config.FrameCacheBurst    = 0;
    s_enc_config.FrameCacheMerge    = 3;
    s_enc_config.FrameCacheWayShape = 15;

    return eError;
}


/** The Deinitialization function of the video encoder
*/
OMX_ERRORTYPE omx_vpuenc_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp) {
    UNREFERENCED_PARAMETER(openmaxStandComp);

    OMX_ERRORTYPE eError = OMX_ErrorNone;


    DEBUG(DEB_LEV_SIMPLE_SEQ, "Out %s : %d\n", __func__, eError);
    return eError;
}


/** Executes all the required steps after an output buffer frame-size has changed.
*/
static inline void UpdateFrameSize(OMX_COMPONENTTYPE *openmaxStandComp)
{
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *inPort = (omx_base_video_PortType *)omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_base_video_PortType *outPort = (omx_base_video_PortType *)omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    vpu_enc_context_t *pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;

    switch(inPort->sPortParam.format.video.eColorFormat) {
    case OMX_COLOR_FormatYUV420Planar:
    case OMX_COLOR_FormatYUV420PackedPlanar:
    case OMX_COLOR_FormatYUV420SemiPlanar:
    case OMX_COLOR_FormatYUV420PackedSemiPlanar:
    // case MAKE_FOURCC('N', 'V', '2', '1'):
#ifdef ANDROID
    case OMX_COLOR_FormatAndroidOpaque:
#endif
        pVpu->srcFrameFormat = FORMAT_420;
        break;

    case OMX_COLOR_FormatMonochrome:
        pVpu->srcFrameFormat = FORMAT_400;

    default:
        pVpu->srcFrameFormat = FORMAT_420;
        break;
    }

    inPort->sPortParam.format.video.nStride =
        CalcStride(inPort->sPortParam.format.video.nFrameWidth, inPort->sPortParam.format.video.nFrameHeight, pVpu->srcFrameFormat, pVpu->encOP.cbcrInterleave, pVpu->mapType, FALSE);

    inPort->sPortParam.nBufferSize =
        VPU_GetFrameBufSize(pVpu->coreIdx, inPort->sPortParam.format.video.nStride, inPort->sPortParam.format.video.nFrameHeight, pVpu->mapType, pVpu->srcFrameFormat, pVpu->encOP.cbcrInterleave, NULL);

    pVpu->srcFrameStride = inPort->sPortParam.format.video.nStride;
    pVpu->srcFrameHeight = inPort->sPortParam.format.video.nFrameHeight;
    pVpu->srcFrameBufSize = inPort->sPortParam.nBufferSize;

    outPort->sPortParam.format.video.nFrameWidth = inPort->sPortParam.format.video.nFrameWidth;
    outPort->sPortParam.format.video.nFrameHeight = inPort->sPortParam.format.video.nFrameHeight;
    outPort->sPortParam.format.video.nStride = inPort->sPortParam.format.video.nStride;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "Out %s Inport nBufferSize=%d, eColorFormat=0x%x, nFrameWidth=%d, nFrameHeight=%d, nStride=%d, nSliceHeight=%d\n", __func__,
        (int)inPort->sPortParam.nBufferSize, (int)inPort->sVideoParam.eColorFormat, (int)inPort->sPortParam.format.video.nFrameWidth, (int)inPort->sPortParam.format.video.nFrameHeight,
        (int)inPort->sPortParam.format.video.nStride, (int)inPort->sPortParam.format.video.nSliceHeight);
}


/** This function is used to process the input buffer and provide one output buffer
*/
void omx_vpuenc_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_BUFFERHEADERTYPE* pOutputBuffer)
{
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *inPort = (omx_base_video_PortType *) omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_base_video_PortType *outPort = (omx_base_video_PortType *) omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    vpu_enc_context_t *pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;
    RetCode ret = RETCODE_SUCCESS;
    int srcFrameIdx;
    OMX_S32 nHeaderLen = 0;
    int i;
    int sizeFb;
    int err = OMX_ErrorStreamCorrupt;
    vpu_buffer_t vbStream;
    SecAxiUse secAxiUse;
    OMX_BOOL portSettingChangeDetected = OMX_FALSE;

    double ts_start = 0.0;
    double ts_end = 0.0;

    UNREFERENCED_PARAMETER(outPort);

    if (pInputBuffer && (pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) && pInputBuffer->nFilledLen == 0)
    {
        DEBUG(DEB_LEV_FULL_SEQ, "%s : Receive EOS flag in inputBuffer, ignore the data in the buffer\n", __func__);
        if (pOutputBuffer)
        {
            pOutputBuffer->nFilledLen = 0;
            pOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
        }
        return;
    }
#ifdef ANDROID
    if (omx_vpuenc_component_Private->storeMetaDataInInputBuffer == OMX_TRUE) {
        DEBUG(DEB_LEV_FULL_SEQ, "%s storeMetaInInputBuffer\n", __func__);
        omx_vpuenc_component_Private->metadata_buffer_type = *(MetadataBufferType *)pInputBuffer->pBuffer;
        if (omx_vpuenc_component_Private->metadata_buffer_type == kMetadataBufferTypeANWBuffer)
        {
            encoder_video_native_metadata_t *meta_data = (encoder_video_native_metadata_t *)pInputBuffer->pBuffer;
            struct ANativeWindowBuffer *buffer = meta_data->pBuffer;
            omx_vpuenc_component_Private->buffer_handle = buffer->handle;

            DEBUG(DEB_LEV_FULL_SEQ, "%s storeMetaInInputBuffer with fence fd %d\n", __func__, meta_data->nFenceFd);
        }
        else if(omx_vpuenc_component_Private->metadata_buffer_type == kMetadataBufferTypeGrallocSource)
        {
            encoder_video_gralloc_metadata_t *meta_data = (encoder_video_gralloc_metadata_t *)pInputBuffer->pBuffer;
            omx_vpuenc_component_Private->buffer_handle = meta_data->meta_handle;
        }
        else
        {
            DEBUG(DEB_LEV_ERR, "%s Unsupported metadata type (%d) \n", __func__, omx_vpuenc_component_Private->metadata_buffer_type);
        }
    }
#else
    if (omx_vpuenc_component_Private->storeMetaDataInDrmBuffer == OMX_TRUE)
    {
        omx_vpuenc_component_Private->buffer_handle = *(OMX_U32 *)pInputBuffer->pBuffer;
        DEBUG(DEB_LEV_FULL_SEQ, "buffer_handle is (%u) \n", omx_vpuenc_component_Private->buffer_handle);
    }
#endif

    if (omx_vpuenc_component_Private->portSettingChangeRequest == OMX_TRUE) // this should not enter vpu_flush_mutex lock to reduce lock delay
    {
        if (pInputBuffer)
            pInputBuffer->nFilledLen = pInputBuffer->nFilledLen; // it means the inputBuffer should remain.
        if (pOutputBuffer)
            pOutputBuffer->nFilledLen = 0; // there is no data to output.

        return;
    }

    if (!omx_vpuenc_component_Private->vpuReady)
    {
        err = omx_vpuenc_component_vpuLibInit(omx_vpuenc_component_Private);
        if (err != OMX_ErrorNone)
        {
            DEBUG(DEB_LEV_ERR, "In %s omx_vpuenc_component_vpuLibInit Failed\n", __func__);
            goto ERR_ENC;
        }
        omx_vpuenc_component_Private->vpuReady = OMX_TRUE;
    }

    if (omx_vpuenc_component_Private->bUseOmxAllocateBufferOfOutPort == OMX_TRUE)
    {
        if (OmxGetVpuBsBufferByVirtualAddress(pVpu, &vbStream, (unsigned long)pOutputBuffer->pBuffer) == -1)
        {
            DEBUG(DEB_LEV_ERR, "Not found bistream buffer in OMX Buffer Header=%p\n", pInputBuffer->pBuffer);
            goto ERR_ENC;
        }
    }
    else
    {
        vbStream = pVpu->vbStream[0];
    }

    if(!pVpu->seqInited)
    {

        if( s_enc_config.useRot == 1 )
        {
            VPU_EncGiveCommand(pVpu->handle, ENABLE_ROTATION, 0);
            VPU_EncGiveCommand(pVpu->handle, ENABLE_MIRRORING, 0);
            VPU_EncGiveCommand(pVpu->handle, SET_ROTATION_ANGLE, &s_enc_config.rotAngle);
            VPU_EncGiveCommand(pVpu->handle, SET_MIRROR_DIRECTION, &s_enc_config.mirDir);
        }

        if (vdi_set_sram_cfg(pVpu->coreIdx, MODE_SRAM_LINEBUFFER) != RETCODE_SUCCESS)
        {
            DEBUG(DEB_LEV_ERR, "vdi_set_sram_cfg LINEBUFFER mode failed, coreIdx %d\n", pVpu->coreIdx);
            goto ERR_ENC;
        }
        else
        {
            pVpu->encParam.sramMode = MODE_SRAM_LINEBUFFER;
        }

        /* set sram buffer for sec AXI, coda enc using soc-sram*/
        memset(&secAxiUse, 0x0, sizeof(secAxiUse));
        secAxiUse.u.coda9.useBitEnable = USE_BIT_INTERNAL_BUF;
        secAxiUse.u.coda9.useIpEnable = USE_IP_INTERNAL_BUF;
        secAxiUse.u.coda9.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
        secAxiUse.u.coda9.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
        ret = VPU_EncGiveCommand(pVpu->handle, SET_SEC_AXI, &secAxiUse);
        if (ret != RETCODE_SUCCESS) {
            DEBUG(DEB_LEV_ERR, "VPU_DecGiveCommand ( SET_SEC_AXI ) failed Error code is 0x%x \n",
                  ret);
            goto ERR_ENC;
        }

        ret = VPU_EncGetInitialInfo(pVpu->handle, &pVpu->initialInfo);
        if (ret != RETCODE_SUCCESS)
        {
            DEBUG(DEB_LEV_ERR, "VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret);
            goto ERR_ENC;
        }

        pVpu->regFrameBufCount = pVpu->initialInfo.minFrameBufferCount;
        DEBUG(DEB_LEV_FULL_SEQ, "* Enc InitialInfo =>\n minframeBuffercount: %u\n minSrcBufferCount: %d\n", pVpu->initialInfo.minFrameBufferCount, pVpu->initialInfo.minSrcFrameCount);
        DEBUG(DEB_LEV_FULL_SEQ, " picWidth: %u\n picHeight: %u\n ",pVpu->encOP.picWidth, pVpu->encOP.picHeight);
        DEBUG(DEB_LEV_FULL_SEQ, "bUseOmxAllocateBufferOfInputPort %d, nBufferCountActual %d\n",
            omx_vpuenc_component_Private->bUseOmxAllocateBufferOfInputPort, inPort->sPortParam.nBufferCountActual);
        portSettingChangeDetected = OMX_FALSE;

        // Note: The below values of MaverickCache configuration are best values.
        MaverickCache2Config(
            &pVpu->encCacheConfig,
            0, //encoder
            pVpu->encOP.cbcrInterleave, // cb cr interleave
            0, /* bypass */
            0, /* burst  */
            3, /* merge mode */
            pVpu->mapType,
            15 /* shape */);
        VPU_EncGiveCommand(pVpu->handle, SET_CACHE_CONFIG, &pVpu->encCacheConfig);

        if (omx_vpuenc_component_Private->bUseOmxAllocateBufferOfInputPort == OMX_TRUE)
        {
            if (pVpu->initialInfo.minSrcFrameCount > (int)inPort->sPortParam.nBufferCountActual)
                portSettingChangeDetected = OMX_TRUE;
        }

        if (portSettingChangeDetected == OMX_TRUE)
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "---->pVpu->initialInfo.minSrcFrameCount=%d, nBufferCountActual=%d\n",
                (int)pVpu->initialInfo.minSrcFrameCount, (int)inPort->sPortParam.nBufferCountActual);

            omx_vpuenc_component_Private->nTempBufferCountActual = pVpu->initialInfo.minSrcFrameCount;

            if (omx_vpuenc_component_Private->callbacks->EventHandler)
            {
                OMX_ERRORTYPE   ret;

                DEBUG(DEB_LEV_SIMPLE_SEQ,"Call PortSettingsChange");
                ret = (*(omx_vpuenc_component_Private->callbacks->EventHandler))
                    (openmaxStandComp,
                    omx_vpuenc_component_Private->callbackData,
                    OMX_EventPortSettingsChanged,
                    0,  /* This is the input port index */
                    0,
                    NULL);

                if (ret == OMX_ErrorNone)
                {
                    omx_vpuenc_component_Private->portSettingChangeRequest = OMX_TRUE;
                }
                else
                {
                    DEBUG(DEB_LEV_ERR, "fail to OMX_EventPortSettingsChanged Event for output buffer count\n");
                    goto ERR_ENC;
                }

                DEBUG(DEB_LEV_SIMPLE_SEQ,"Call PortSettingsChange Event Done");
            }
            pVpu->seqInited = 0;
            goto SKIP_ENCODE;
        }

        pVpu->fbAllocInfo.format = pVpu->srcFrameFormat;
        pVpu->fbAllocInfo.cbcrInterleave = pVpu->encOP.cbcrInterleave;
        pVpu->fbAllocInfo.nv21           = pVpu->encOP.nv21;
        if (pVpu->encOP.linear2TiledEnable)
            pVpu->fbAllocInfo.mapType = LINEAR_FRAME_MAP;
        else
            pVpu->fbAllocInfo.mapType = pVpu->mapType;
        pVpu->fbAllocInfo.stride = pVpu->srcFrameStride;
        pVpu->fbAllocInfo.height = VPU_ALIGN32(pVpu->srcFrameHeight);  // To cover interlaced picture
        pVpu->fbAllocInfo.endian = pVpu->encOP.frameEndian;
        pVpu->fbAllocInfo.lumaBitDepth = COLOR_DEPTH_8_BIT;
        pVpu->fbAllocInfo.chromaBitDepth = COLOR_DEPTH_8_BIT;
        pVpu->fbAllocInfo.num = pVpu->regFrameBufCount;
        pVpu->fbAllocInfo.type = FB_TYPE_CODEC;

        sizeFb = VPU_GetFrameBufSize(pVpu->coreIdx, pVpu->fbAllocInfo.stride, pVpu->fbAllocInfo.height, pVpu->fbAllocInfo.mapType, pVpu->fbAllocInfo.format, pVpu->fbAllocInfo.cbcrInterleave, NULL);
        DEBUG(DEB_LEV_FULL_SEQ, "FB_TYPE_CODEC productId %d, regFrameBufCount %d, stride %d, w %d, h %d, mapType %d, fbsize %d\n",
            pVpu->productId, pVpu->regFrameBufCount, pVpu->fbAllocInfo.stride, pVpu->fbAllocInfo.stride, pVpu->fbAllocInfo.height, pVpu->mapType, sizeFb);

        for (i = 0; i < pVpu->regFrameBufCount; i++) {
            pVpu->vbReconFrameBuf[i].size = sizeFb;
            if (vdi_allocate_dma_memory(pVpu->coreIdx, &pVpu->vbReconFrameBuf[i]) < 0)
            {
                DEBUG(DEB_LEV_ERR, "fail to allocate recon buffer\n" );
                goto ERR_ENC;
            }
            pVpu->fbRecon[i].bufY  = pVpu->vbReconFrameBuf[i].phys_addr;
            pVpu->fbRecon[i].bufCb = -1;
            pVpu->fbRecon[i].bufCr = -1;
            pVpu->fbRecon[i].updateFbInfo = TRUE;
        }

        ret = VPU_EncAllocateFrameBuffer(pVpu->handle, pVpu->fbAllocInfo, pVpu->fbRecon);
        if (ret != RETCODE_SUCCESS) {
            DEBUG(DEB_LEV_ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret);
            goto ERR_ENC;
        }

        ret = VPU_EncRegisterFrameBuffer(pVpu->handle, pVpu->fbRecon, pVpu->regFrameBufCount, pVpu->fbAllocInfo.stride, pVpu->fbAllocInfo.height, pVpu->mapType);
        if( ret != RETCODE_SUCCESS )
        {
            DEBUG(DEB_LEV_ERR, "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret );
            goto ERR_ENC;
        }
        VPU_EncGiveCommand(pVpu->handle, GET_TILEDMAP_CONFIG, &pVpu->mapCfg);

        /* ALLOCATE SROUCE BUFFERS */
        pVpu->fbAllocInfo.stride = pVpu->srcFrameStride;
        pVpu->fbAllocInfo.height = pVpu->srcFrameHeight;
        pVpu->fbAllocInfo.endian = pVpu->encOP.frameEndian;
        pVpu->fbAllocInfo.type = FB_TYPE_PPU;

        sizeFb = inPort->sPortParam.nBufferSize;
        DEBUG(DEB_LEV_FULL_SEQ, "FB_TYPE_PPU storeMetaDataInInputBuffer %d, sride %d, height %d, sizeFb %d\n", omx_vpuenc_component_Private->storeMetaDataInInputBuffer, pVpu->fbAllocInfo.stride, pVpu->fbAllocInfo.height, sizeFb);

        pVpu->fbAllocInfo.num = 1; // default case need one frame buffer
        if (OMX_TRUE == omx_vpuenc_component_Private->bUseOmxAllocateBufferOfInputPort)  // OMX_AllocateBuffer case
        {
            pVpu->fbAllocInfo.num = (int)inPort->sPortParam.nBufferCountActual;

            for (i=0; i < (int)inPort->sPortParam.nBufferCountActual; i++)
            {
                pVpu->fbSrc[i].bufY = pVpu->vbSrcFb[i].phys_addr;
                pVpu->fbSrc[i].bufCb = -1;  // not allocation. assign this buffer into API
                pVpu->fbSrc[i].bufCr = -1;
                pVpu->fbSrc[i].updateFbInfo = TRUE;
            }
        }
        else
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "allocate vbSrcFb when storeMetaDataInInputBuffer or usebuffer case\n");
            for (i = 0; i < pVpu->fbAllocInfo.num; i++) {
                pVpu->vbSrcFb[i].size = sizeFb;
                if (vdi_allocate_dma_memory(pVpu->coreIdx, &pVpu->vbSrcFb[i]) < 0) {
                    VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                    goto ERR_ENC;
                }

                pVpu->fbSrc[i].bufY  = pVpu->vbSrcFb[i].phys_addr;
                pVpu->fbSrc[i].bufCb = (PhysicalAddress)-1;
                pVpu->fbSrc[i].bufCr = (PhysicalAddress)-1;
                pVpu->fbSrc[i].updateFbInfo = TRUE;
            }
        }

        DEBUG(DEB_LEV_FULL_SEQ, "fbAllocInfo.num %d, sizeFb %d\n", pVpu->fbAllocInfo.num, sizeFb);
        ret = VPU_EncAllocateFrameBuffer(pVpu->handle, pVpu->fbAllocInfo, pVpu->fbSrc);
        if( ret != RETCODE_SUCCESS )
        {
            DEBUG(DEB_LEV_ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
            goto ERR_ENC;
        }

        if (pVpu->encOP.bitstreamFormat == STD_MPEG4)
        {
            pVpu->encHeaderParam.headerType = VOS_HEADER;
            pVpu->encHeaderParam.buf = vbStream.phys_addr;
            pVpu->encHeaderParam.size = vbStream.size;
            VPU_EncGiveCommand(pVpu->handle, ENC_PUT_VIDEO_HEADER, &pVpu->encHeaderParam);

            //copy needed because the pVpu->encHeaderParam.buf address must be aligned by 8byte. but some size of encoder header can be arbitrary size.
            if(pVpu->encHeaderParam.size > 0) {
                vdi_read_memory(pVpu->coreIdx, pVpu->encHeaderParam.buf, pVpu->pHeaderData + nHeaderLen, pVpu->encHeaderParam.size, pVpu->encOP.streamEndian);
            }
            DEBUG(DEB_LEV_SIMPLE_SEQ, "ENC_PUT_VIDEO_HEADER VOS_HEADER header length: %zu, encHeaderParam.buf=0x%x, pHeaderData=%p\n", pVpu->encHeaderParam.size, (int)pVpu->encHeaderParam.buf, (pVpu->pHeaderData + nHeaderLen));
            nHeaderLen += pVpu->encHeaderParam.size;

            pVpu->encHeaderParam.headerType = VIS_HEADER;
            pVpu->encHeaderParam.buf = vbStream.phys_addr;
            pVpu->encHeaderParam.size = vbStream.size;
            VPU_EncGiveCommand(pVpu->handle, ENC_PUT_VIDEO_HEADER, &pVpu->encHeaderParam);

            //copy needed because the pVpu->encHeaderParam.buf address must be aligned by 8byte. but some size of encoder header can be arbitrary size.
            if (pVpu->encHeaderParam.size > 0)
            {
                vdi_read_memory(pVpu->coreIdx, pVpu->encHeaderParam.buf, pVpu->pHeaderData + nHeaderLen, pVpu->encHeaderParam.size, pVpu->encOP.streamEndian);
            }
            DEBUG(DEB_LEV_SIMPLE_SEQ, "ENC_PUT_VIDEO_HEADER VIS_HEADER header length: %zu, encHeaderParam.buf=0x%x, pHeaderData=%p\n", pVpu->encHeaderParam.size, (int)pVpu->encHeaderParam.buf, (pVpu->pHeaderData + nHeaderLen));
            nHeaderLen += pVpu->encHeaderParam.size;

            pVpu->encHeaderParam.headerType = VOL_HEADER;
            pVpu->encHeaderParam.buf = vbStream.phys_addr;
            pVpu->encHeaderParam.size = vbStream.size;
            ret = VPU_EncGiveCommand(pVpu->handle, ENC_PUT_VIDEO_HEADER, &pVpu->encHeaderParam);
            if(pVpu->encHeaderParam.size > 0) {
                vdi_read_memory(pVpu->coreIdx, pVpu->encHeaderParam.buf, pVpu->pHeaderData + nHeaderLen, pVpu->encHeaderParam.size, pVpu->encOP.streamEndian);
            }
            DEBUG(DEB_LEV_SIMPLE_SEQ, "ENC_PUT_VIDEO_HEADER VOL_HEADER header length: %zu, encHeaderParam.buf=0x%x, pHeaderData=%p\n", pVpu->encHeaderParam.size, (int)pVpu->encHeaderParam.buf, (pVpu->pHeaderData + nHeaderLen));
            nHeaderLen += pVpu->encHeaderParam.size;

            memcpy(pOutputBuffer->pBuffer, pVpu->pHeaderData, nHeaderLen);
        }
        else if (pVpu->encOP.bitstreamFormat == STD_AVC)
        {
            pVpu->encHeaderParam.headerType = SPS_RBSP;
            pVpu->encHeaderParam.buf = vbStream.phys_addr;
            pVpu->encHeaderParam.size = vbStream.size;

            VPU_EncGiveCommand(pVpu->handle, ENC_PUT_VIDEO_HEADER, &pVpu->encHeaderParam);

            //copy needed because the pVpu->encHeaderParam.buf address must be aligned by 8byte. but some size of encoder header can be arbitrary size.
            if(pVpu->encHeaderParam.size > 0) {
                vdi_read_memory(pVpu->coreIdx, pVpu->encHeaderParam.buf, pVpu->pHeaderData + nHeaderLen, pVpu->encHeaderParam.size, pVpu->encOP.streamEndian);
            }
            DEBUG(DEB_LEV_SIMPLE_SEQ, "ENC_PUT_VIDEO_HEADER SPS_RBSP header length: %zu, encHeaderParam.buf=0x%x, pHeaderData=%p\n", pVpu->encHeaderParam.size, (int)pVpu->encHeaderParam.buf, (pVpu->pHeaderData + nHeaderLen));

            nHeaderLen += pVpu->encHeaderParam.size;

            //backup current ActivePPSIdx
            VPU_EncGiveCommand(pVpu->handle, ENC_GET_ACTIVE_PPS, &pVpu->activePPSIdx);
            pVpu->encHeaderParam.headerType = PPS_RBSP;
            for (i=0; i<pVpu->encOP.EncStdParam.avcParam.ppsNum; i++) {
                pVpu->encHeaderParam.buf = vbStream.phys_addr;
                pVpu->encHeaderParam.pBuf = (BYTE *)vbStream.virt_addr;
                pVpu->encHeaderParam.size  = vbStream.size;
                VPU_EncGiveCommand(pVpu->handle, ENC_SET_ACTIVE_PPS, &i);
                VPU_EncGiveCommand(pVpu->handle, ENC_PUT_VIDEO_HEADER, &pVpu->encHeaderParam);
                if(pVpu->encHeaderParam.size > 0) {
                    vdi_read_memory(pVpu->coreIdx, pVpu->encHeaderParam.buf, pVpu->pHeaderData + nHeaderLen, pVpu->encHeaderParam.size, pVpu->encOP.streamEndian);
                }
                DEBUG(DEB_LEV_SIMPLE_SEQ, "ENC_PUT_VIDEO_HEADER ENC_GET_ACTIVE_PPS  header length: %zu, encHeaderParam.buf=0x%x, pHeaderData=%p\n", pVpu->encHeaderParam.size, (int)pVpu->encHeaderParam.buf, (pVpu->pHeaderData + nHeaderLen));

                nHeaderLen += pVpu->encHeaderParam.size;
            }
            //restore Active PPS Idx
            VPU_EncGiveCommand(pVpu->handle, ENC_SET_ACTIVE_PPS, &pVpu->activePPSIdx);
            memcpy(pOutputBuffer->pBuffer, pVpu->pHeaderData, nHeaderLen);
        }

        pOutputBuffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG;
        pOutputBuffer->nTimeStamp = 0;
        pOutputBuffer->nFilledLen = nHeaderLen;
        pVpu->seqInited = 1;

        OMX_DUMP_OUTPUT_TO_FILE(pVpu, pOutputBuffer);
        DEBUG(DEB_LEV_SIMPLE_SEQ, "%s : VPU_EncGetInitialInfo and ENC_PUT_VIDEO_HEADER success pOutputBuffer->nFilledLen : %d, pOutputBuffer->nAllocLen : %d, pOutputBuffer->nFlags=0x%x\n", __func__, (int)pOutputBuffer->nFilledLen, (int)pOutputBuffer->nAllocLen, (int)pOutputBuffer->nFlags);
        return;
    }


    if (omx_vpuenc_component_Private->bUseOmxAllocateBufferOfInputPort == OMX_TRUE)
        srcFrameIdx = OmxGetInputBufferIndex(openmaxStandComp, pInputBuffer);
    else
        srcFrameIdx = 0; //because we only have one wrapper buf, so the buf index is always 0

    if (srcFrameIdx < 0)
    {
        DEBUG(DEB_LEV_ERR, "fail to find input buffer index\n");
        goto ERR_ENC;
    }

    DEBUG(DEB_LEV_FULL_SEQ, "storeMetaDataInInputBuffer %d\n", omx_vpuenc_component_Private->storeMetaDataInInputBuffer);
    if (omx_vpuenc_component_Private->storeMetaDataInInputBuffer == OMX_TRUE
        || omx_vpuenc_component_Private->storeMetaDataInDrmBuffer == OMX_TRUE)
    {
#ifdef ANDROID
        // we can pass the buf to vpu directly
        if (OMX_FALSE == omx_vpuenc_component_Private->bFormatConversion)
        {
            PhysicalAddress physAddr = 0;
            buffer_handle_t native_handle = omx_vpuenc_component_Private->buffer_handle;

            if (OMX_ErrorNone == OmxGetCorrespondPhysAddr(pVpu, native_handle->data[0], &physAddr))
            {
                pVpu->fbSrc[srcFrameIdx].bufY = physAddr;
                pVpu->fbSrc[srcFrameIdx].bufCb = -1;
                pVpu->fbSrc[srcFrameIdx].bufCr = -1;

                // VPU_EncAllocateFrameBuffer is not to allocate another buffer. it just assign the other Framebuffer information.
                ret = VPU_EncAllocateFrameBuffer(pVpu->handle, pVpu->fbAllocInfo, pVpu->fbSrc);
                if( ret != RETCODE_SUCCESS )
                {
                    DEBUG(DEB_LEV_ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
                    goto ERR_ENC;
                }

                OMX_ZEROCOPY_INPUT_BUF_DUMP(omx_vpuenc_component_Private);
                DEBUG(DEB_LEV_FULL_SEQ, "%s : storeMetaDataInInputBuffer as kMetadataBufferTypeANWBuffer, pInputBuffer->pBuffer=%p, nBufferSize=%d\n",
                      __func__, pInputBuffer->pBuffer, (int)inPort->sPortParam.nBufferSize);
            }
            else
            {
                DEBUG(DEB_LEV_FULL_SEQ, "%s : storeMetaDataInInputBuffer, failed to get the phys addr for buf handle %d\n", __func__, native_handle->data[0]);
            }
        }
        else
        // we should get the buf from the handle, for current case, we support rgba8888 input only
        {
            void *pFrame = NULL;
            int grallocFormat = HAL_PIXEL_FORMAT_RGBA_8888;   // default to HAL_PIXEL_FORMAT_RGBA_8888

            getAndroidNativeBufferHandleInfo(omx_vpuenc_component_Private->buffer_handle, &grallocFormat, NULL, NULL, NULL, NULL);

            DEBUG(DEB_LEV_FULL_SEQ,
                "%s : metadata_buffer_type is kMetadataBufferTypeGrallocSource, "
                "eColorFormat=0x%x, grallocFormat=0x%x\n",
                __func__, inPort->sPortParam.format.video.eColorFormat, grallocFormat);

            lockAndroidBufferHandle(omx_vpuenc_component_Private->buffer_handle, pVpu->srcFrameStride, pVpu->srcFrameHeight, LOCK_MODE_TO_GET_VIRTUAL_ADDRESS, &pFrame);

            if (OMX_FALSE == convertRgbToYuvbySW(pVpu->privateYuv, (OMX_BYTE)pFrame, grallocFormat, pVpu->srcFrameStride, pVpu->srcFrameHeight))
            {
                DEBUG(DEB_LEV_ERR, "%s : fail to ConvertRgbToYuv\n", __func__);
                goto ERR_ENC;
            }

            OmxWriteYuvData(pVpu->privateYuv, pVpu, &pVpu->fbSrc[srcFrameIdx], false);
            DEBUG(DEB_LEV_FULL_SEQ,
                "%s : storeMetaDataInInputBuffer as kMetadataBufferTypeGrallocSource  to"
                "pAddr=0x%x, bufY=0x%x, pInputBuffer->pBuffer=%p, srcFrameStride=%d,"
                "srcFrameHeight=%d, srcFrameBufSize=%d, pInputBufferLen:%d\n",
                __func__, (int)pFrame, (int)pVpu->fbSrc[srcFrameIdx].bufY, pInputBuffer->pBuffer,
                (int)pVpu->srcFrameStride, (int)pVpu->srcFrameHeight, (int)pVpu->srcFrameBufSize,
                (int)pInputBuffer->nFilledLen);

            unLockAndroidBufferHandle(omx_vpuenc_component_Private->buffer_handle);
        }
#else//ANDROID
    if (omx_vpuenc_component_Private->storeMetaDataInDrmBuffer == OMX_TRUE)
    {
        PhysicalAddress physAddr = 0;
        OMX_S32 native_handle = omx_vpuenc_component_Private->buffer_handle;

        if (OMX_ErrorNone == OmxGetCorrespondPhysAddr(pVpu, native_handle, &physAddr))
        {
            pVpu->fbSrc[srcFrameIdx].bufY = physAddr;
            pVpu->fbSrc[srcFrameIdx].bufCb = -1;
            pVpu->fbSrc[srcFrameIdx].bufCr = -1;

            // VPU_EncAllocateFrameBuffer is not to allocate another buffer. it just assign the other Framebuffer information.
            ret = VPU_EncAllocateFrameBuffer(pVpu->handle, pVpu->fbAllocInfo, pVpu->fbSrc);
            if( ret != RETCODE_SUCCESS )
            {
                DEBUG(DEB_LEV_ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
                goto ERR_ENC;
            }

            DEBUG(DEB_LEV_FULL_SEQ, "%s : storeMetaDataInDrmBuffer , pInputBuffer->pBuffer=%p, nBufferSize=%d\n",
                  __func__, pInputBuffer->pBuffer, (int)inPort->sPortParam.nBufferSize);
        }
        else
        {
            DEBUG(DEB_LEV_FULL_SEQ, "%s : storeMetaDataInDrmBuffer, failed to get the phys addr for buf handle %d\n", __func__, native_handle);
        }
    }
#endif
    }
    else
    {
        if (omx_vpuenc_component_Private->bUseOmxAllocateBufferOfInputPort == OMX_TRUE)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "%s UseOmxAllocateBufferOfInputPort, noting to do\n", __func__);
        }
        else
        {
            DEBUG(DEB_LEV_FULL_SEQ,
                  "%s : before UseBuffer case LoadYuv srcFrameIdx=%d, bufY=0x%x, pInputBuffer->pBuffer=%p,"
                  "inPort->sPortParam.nBufferSize=%d, pInputBufferLen:%d\n",
                  __func__, (int)srcFrameIdx, (int)pVpu->fbSrc[srcFrameIdx].bufY, pInputBuffer->pBuffer,
                  (int)inPort->sPortParam.nBufferSize, (int)pInputBuffer->nFilledLen);
            OmxWriteYuvData(pInputBuffer->pBuffer, pVpu, &pVpu->fbSrc[srcFrameIdx],
                            pInputBuffer->nFilledLen < inPort->sPortParam.nBufferSize);
            DEBUG(DEB_LEV_FULL_SEQ,
                  "%s : UseBuffer case LoadYuv srcFrameIdx=%d, bufY=0x%x, pInputBuffer->pBuffer=%p,"
                  "inPort->sPortParam.nBufferSize=%d, pInputBufferLen:%d\n",
                  __func__, (int)srcFrameIdx, (int)pVpu->fbSrc[srcFrameIdx].bufY, pInputBuffer->pBuffer,
                  (int)inPort->sPortParam.nBufferSize, (int)pInputBuffer->nFilledLen);
        }
    }

    pVpu->encParam.quantParam = 10; //default value
    pVpu->encParam.forceIPicture = 0;
    pVpu->encParam.skipPicture = 0;
    if (!pVpu->encOP.rcEnable)
        pVpu->encParam.quantParam = omx_vpuenc_component_Private->codParam.quantization.nQpI;
    pVpu->encParam.sourceFrame = &pVpu->fbSrc[srcFrameIdx];

    pVpu->fieldDone = 0;
    pVpu->encParam.picStreamBufferAddr = vbStream.phys_addr;
    pVpu->encParam.picStreamBufferSize = vbStream.size;
    pVpu->encParam.srcIdx              = srcFrameIdx;

    pVpu->encParam.sourceFrame->endian = pVpu->encOP.frameEndian;
    pVpu->encParam.sourceFrame->cbcrInterleave = pVpu->encOP.cbcrInterleave;
    pVpu->encParam.sourceFrame->sourceLBurstEn = 0;
    if (pVpu->encOP.EncStdParam.avcParam.fieldFlag)
        pVpu->encParam.fieldRun = 1;

    if (!pVpu->frameIdx)
        pVpu->encParam.forceIPicture = 1;
    else if (omx_vpuenc_component_Private->idrPeriod &&((pVpu->frameIdx % omx_vpuenc_component_Private->idrPeriod) == 0))
        pVpu->encParam.forceIPicture = 1;
    else
        pVpu->encParam.forceIPicture = 0;

    if (OmxConfigEncoderParameter(openmaxStandComp) != OMX_TRUE)
    {
        DEBUG(DEB_LEV_ERR, "Fail to OmxConfigEncoderParameter \n");
        goto ERR_ENC;
    }

#ifdef REPORT_PERFORMANCE
    report_performance_ready(pVpu->coreIdx);
#endif

    DEBUG(DEB_LEV_FULL_SEQ, "%s : VPU_EncStartOneFrame quantParam : %d, RcEnable=%d, forceIPicture=%d, picStreamBufferAddr=0x%x, picStreamBufferSize=%d, sourceFrame index=%d, bufY=0x%x, bufCb=0x%x, bufCr=0x%x, timestamp=%lld\n",
        __func__, pVpu->encParam.quantParam, pVpu->encOP.rcEnable, (int)pVpu->encParam.forceIPicture, (int)pVpu->encParam.picStreamBufferAddr, (int)pVpu->encParam.picStreamBufferSize, (int)srcFrameIdx, (int)pVpu->encParam.sourceFrame->bufY, (int)pVpu->encParam.sourceFrame->bufCb, (int)pVpu->encParam.sourceFrame->bufCr, pInputBuffer->nTimeStamp);
    if (pVpu->productId == PRODUCT_ID_420) {
        VPU_EncSetWrPtr(pVpu->handle, pVpu->encParam.picStreamBufferAddr, 1);

        if (pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) {
            DEBUG(DEB_LEV_FULL_SEQ, "%s : Set Source End Flag = 1\n", __func__);
            pVpu->encParam.srcEndFlag = 1;
        }
    }

    // Start encoding a frame.
    ts_start = GetNowMs();
    ret = VPU_EncStartOneFrame(pVpu->handle, &pVpu->encParam);
    if (ret != RETCODE_SUCCESS)
    {
        DEBUG(DEB_LEV_ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret);
        goto ERR_ENC;
    }

    while(1)
    {
        OMX_S32 int_reason = 0;
        int_reason = VPU_WaitInterrupt(pVpu->coreIdx, OMX_VPU_ENC_TIMEOUT);
        if (int_reason == -1)
        {
            DEBUG(DEB_LEV_ERR, "Error : encoder timeout happened\n");
            VPU_SWReset(pVpu->coreIdx, SW_RESET_SAFETY, pVpu->handle);
            VPU_EncGetOutputInfo(pVpu->handle, &pVpu->outputInfo); // need to free some resources that is created VPU_DecStartOneFrame
            int_reason = 0;
            goto ERR_ENC;
        }

        if (int_reason)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "int_reason 0x%x\n", int_reason);
            VPU_ClearInterrupt(pVpu->coreIdx);

            if (int_reason & (1<<INT_BIT_PIC_RUN))
                break;
        }
    }

    ret = VPU_EncGetOutputInfo(pVpu->handle, &pVpu->outputInfo);
    if (ret != RETCODE_SUCCESS)
    {
        DEBUG(DEB_LEV_ERR, "VPU_EncGetOutputInfo failed Error code is 0x%x \n", ret);
        goto ERR_ENC;
    }
    DisplayEncodedInformation(pVpu->handle, pVpu->encOP.bitstreamFormat, pVpu->frameIdx, &pVpu->outputInfo);
    ts_end = GetNowMs();

    if (pVpu->productId == PRODUCT_ID_420)
    {
        if (!(pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS))
            pOutputBuffer->nFlags = pInputBuffer->nFlags;

        if (pVpu->outputInfo.reconFrameIndex == -1) {
            pOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
            pOutputBuffer->nFilledLen = 0;
        }
    }
    else
        pOutputBuffer->nFlags = pInputBuffer->nFlags;

    pOutputBuffer->nTimeStamp = pInputBuffer->nTimeStamp;
    if(pVpu->outputInfo.picType == PIC_TYPE_I || pVpu->outputInfo.picType == PIC_TYPE_IDR)//Picture Type I
        pOutputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;


    if (pVpu->outputInfo.bitstreamWrapAround == 1)
    {
        DEBUG(DEB_LEV_ERR, "Warnning!! BitStream buffer wrap arounded. prepare more large buffer \n");
    }

    if (pVpu->productId == PRODUCT_ID_420)
        DEBUG(DEB_LEV_FULL_SEQ, "Out %s : VPU_EncStartOneFrame Done index=%d, reconIdx=%d, enSrcIdx=%d, input->nFilledLen=%d, nAllocLen=%d, outputInfo.bitstreamSize=%d\n", __func__, pVpu->frameIdx, pVpu->outputInfo.reconFrameIndex, pVpu->outputInfo.encSrcIdx, (int)pOutputBuffer->nFilledLen, (int)pOutputBuffer->nAllocLen, (int)pVpu->outputInfo.bitstreamSize);
    else
        DEBUG(DEB_LEV_FULL_SEQ, "Out %s : VPU_EncStartOneFrame Done index=%d, picType=%d, nFilledLen=%d, nAllocLen=%d, outputInfo.bitstreamSize=%d, time=%.1fms\n", __func__, pVpu->frameIdx, pVpu->outputInfo.picType, (int)pOutputBuffer->nFilledLen, (int)pOutputBuffer->nAllocLen, (int)pVpu->outputInfo.bitstreamSize, (ts_end - ts_start));

    pOutputBuffer->nOffset = 0;
    pInputBuffer->nFilledLen = 0; //it means that encoder component consumes all yuv data.

    if (pVpu->productId == PRODUCT_ID_420)
    {
        if (pVpu->outputInfo.reconFrameIndex < 0)
            goto SKIP_ENCODE;
    }
    else
    {
        if (pVpu->outputInfo.bitstreamSize == 0)
        {
            DEBUG(DEB_LEV_ERR, "Error!! No Encoded bitstreamSize\n");
            goto ERR_ENC;
        }
    }

    if (omx_vpuenc_component_Private->bUseOmxAllocateBufferOfOutPort == OMX_TRUE)
    {
        pOutputBuffer->nFilledLen = pVpu->outputInfo.bitstreamSize;
    }
    else
    {
        vdi_read_memory(pVpu->coreIdx, pVpu->outputInfo.bitstreamBuffer, pOutputBuffer->pBuffer, pVpu->outputInfo.bitstreamSize, pVpu->encOP.streamEndian);
        pOutputBuffer->nFilledLen = pVpu->outputInfo.bitstreamSize;
    }

    pVpu->frameIdx++;
    OMX_DUMP_OUTPUT_TO_FILE(pVpu, pOutputBuffer);

    return;
SKIP_ENCODE:
    if (pOutputBuffer)
    {
        pOutputBuffer->nFilledLen = 0;
        DEBUG(DEB_LEV_FULL_SEQ, "Out %s skip encoding pOutputBuffer->nFilledLen=%d\n\n", __func__, (int)pOutputBuffer->nFilledLen);
    }
    return;

ERR_ENC:

    DEBUG(DEB_LEV_FULL_SEQ, "Out %s : Encode Error\n", __func__);
    if (pOutputBuffer)
        pOutputBuffer->nFilledLen = 0;
    if (pInputBuffer)
        pInputBuffer->nFilledLen = 0;

    if (omx_vpuenc_component_Private->callbacks->EventHandler)
    {
        (*(omx_vpuenc_component_Private->callbacks->EventHandler))
            (openmaxStandComp,
            omx_vpuenc_component_Private->callbackData,
            OMX_EventError,
            OMX_ErrorStreamCorrupt,
            0,
            NULL);
    }
    return;
}


OMX_ERRORTYPE omx_vpuenc_component_SetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure)
{
    OMX_U32 portIndex;
    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if (pComponentConfigStructure == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting configuration %i\n", nParamIndex);


    switch (nParamIndex)
    {
    case OMX_IndexConfigVideoBitrate:
        {
            OMX_VIDEO_CONFIG_BITRATETYPE *pVideoConfigBitrateType = (OMX_VIDEO_CONFIG_BITRATETYPE *)pComponentConfigStructure;
            portIndex = pVideoConfigBitrateType->nPortIndex;

            if ((eError = checkHeader(pComponentConfigStructure, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE))) != OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            DEBUG(DEB_LEV_FULL_SEQ, "[SetParameter]OMX_IndexConfigVideoBitrate : nEncodeBitrate=%dbps, portIndex=%d\n",
                (int)pVideoConfigBitrateType->nEncodeBitrate, (int)portIndex);

            if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                memcpy(&omx_vpuenc_component_Private->codParam.videoConfigBitrateType, pVideoConfigBitrateType, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));
                omx_vpuenc_component_Private->requestChangeBitrate = OMX_TRUE;
            }
            else
                return OMX_ErrorBadPortIndex;

        }
        break;
    case OMX_IndexConfigVideoIntraVOPRefresh:   // from requestIFrame() in ACodec.cpp
        {

            OMX_CONFIG_INTRAREFRESHVOPTYPE *pConfigIntraRefeshVopType = (OMX_CONFIG_INTRAREFRESHVOPTYPE *)pComponentConfigStructure;
            portIndex = pConfigIntraRefeshVopType->nPortIndex;

            DEBUG(DEB_LEV_FULL_SEQ, "[SetParameter]OMX_IndexConfigVideoIntraVOPRefresh : configIntraRefreshVop=%d, portIndex=%d\n",
                (int)pConfigIntraRefeshVopType->IntraRefreshVOP, (int)portIndex);


            if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                memcpy(&omx_vpuenc_component_Private->codParam.configIntraRefreshVopType, pConfigIntraRefeshVopType, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
                omx_vpuenc_component_Private->requestIFrame = OMX_TRUE;
            }
            else
                return OMX_ErrorBadPortIndex;
        }
        break;
    case OMX_IndexConfigVideoFramerate:
        {
            OMX_CONFIG_FRAMERATETYPE  *pConfigFramerateType = (OMX_CONFIG_FRAMERATETYPE  *)pComponentConfigStructure;

            portIndex = pConfigFramerateType->nPortIndex;

            if ((eError = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_FRAMERATETYPE ))) != OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            DEBUG(DEB_LEV_FULL_SEQ, "[SetParameter]OMX_IndexConfigVideoFramerate : xEncodeFramerate>>16=%d, portIndex=%d\n",
                (int)(pConfigFramerateType->xEncodeFramerate>>16), (int)portIndex);

            if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                memcpy(&omx_vpuenc_component_Private->codParam.configFramerateType, pConfigFramerateType, sizeof(OMX_CONFIG_FRAMERATETYPE));
                omx_vpuenc_component_Private->requestChangeFramerate = OMX_TRUE;
            }
            else
                return OMX_ErrorBadPortIndex;

        }
        break;
    case OMX_IndexConfigVideoAVCIntraPeriod:
        {
            OMX_VIDEO_CONFIG_AVCINTRAPERIOD *pVideoConfigAvcIntraPeriod = (OMX_VIDEO_CONFIG_AVCINTRAPERIOD  *)pComponentConfigStructure;

            portIndex = pVideoConfigAvcIntraPeriod->nPortIndex;

            if ((eError = checkHeader(pComponentConfigStructure, sizeof(OMX_VIDEO_CONFIG_AVCINTRAPERIOD ))) != OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if (omx_vpuenc_component_Private->video_encoding_type != OMX_VIDEO_CodingAVC) {
                DEBUG(DEB_LEV_ERR, "In %s Invalid coding type for OMX_IndexConfigVideoAVCIntraPeriod Parameter\n",__func__);
            }


            DEBUG(DEB_LEV_FULL_SEQ, "[SetParameter]OMX_IndexConfigVideoAVCIntraPeriod : nIDRPeriod=%d, nPFrames=%d, portIndex=%d\n",
                (int)pVideoConfigAvcIntraPeriod->nIDRPeriod, (int)pVideoConfigAvcIntraPeriod->nPFrames, (int)portIndex);

            memcpy(&omx_vpuenc_component_Private->codParam.videoConfigAvcIntraPeriod, pVideoConfigAvcIntraPeriod, sizeof(OMX_VIDEO_CONFIG_AVCINTRAPERIOD));
            omx_vpuenc_component_Private->requestChangeGopSize = OMX_TRUE;
            omx_vpuenc_component_Private->idrPeriod = pVideoConfigAvcIntraPeriod->nIDRPeriod;

        }
        break;

    default: // delegate to superclass
        return omx_base_component_SetConfig(hComponent, nParamIndex, pComponentConfigStructure);
    }

    return eError;
}


OMX_ERRORTYPE omx_vpuenc_component_GetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure)
{
    UNREFERENCED_PARAMETER(hComponent);

    if (pComponentConfigStructure == NULL)
        return OMX_ErrorBadParameter;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting configuration %i\n", nParamIndex);

    // return omx_base_component_GetConfig(hComponent, nParamIndex, pComponentConfigStructure);

    DEBUG(DEB_LEV_ERR, "unknown index 0x%08x\n", nParamIndex);
    return OMX_ErrorUnsupportedIndex;
}


OMX_ERRORTYPE omx_vpuenc_component_SetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_IN  OMX_PTR ComponentParameterStructure)
{
    /* Check which structure we are being fed and make control its header */
    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *port;

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 portIndex;
    OMX_U32 paramIndex;

    if (ComponentParameterStructure == NULL)
        return OMX_ErrorBadParameter;

    paramIndex = (OMX_U32)nParamIndex;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "Setting parameter 0x%x\n", (int)paramIndex);
    switch(paramIndex)
    {
    case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
            portIndex = pPortDef->nPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            eError = omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
            if(eError != OMX_ErrorNone) {
                break;
            }

            port = (omx_base_video_PortType *)omx_vpuenc_component_Private->ports[pPortDef->nPortIndex];
            memcpy(&port->sPortParam, pPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            port->sVideoParam.eColorFormat = port->sPortParam.format.video.eColorFormat;
            port->sVideoParam.eCompressionFormat = port->sPortParam.format.video.eCompressionFormat;

            if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX) {
                UpdateFrameSize(openmaxStandComp);
            }

            DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_IndexParamPortDefinition portIndex %u, xFramerate %u, nBitrate %u, nStride %u, nBufferSize %u\n",
                portIndex, port->sPortParam.format.video.xFramerate, port->sPortParam.format.video.nBitrate,
                port->sPortParam.format.video.nStride, port->sPortParam.nBufferSize);
            break;

        }
    case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
            portIndex = pVideoPortFormat->nPortIndex;
            /*Check Structure Header and verify component state*/
            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if (portIndex <= OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                port = (omx_base_video_PortType *)omx_vpuenc_component_Private->ports[portIndex];
                memcpy(&port->sVideoParam, pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
                port->sPortParam.format.video.eColorFormat = port->sVideoParam.eColorFormat;
                port->sPortParam.format.video.eCompressionFormat = port->sVideoParam.eCompressionFormat;
                if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                    UpdateFrameSize(openmaxStandComp);
            }
            else
                return OMX_ErrorBadPortIndex;

            break;
        }
    case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pComponentRole = (OMX_PARAM_COMPONENTROLETYPE *)ComponentParameterStructure;

            if (!strcmp((char *)pComponentRole->cRole, VIDEO_ENC_MPEG4_ROLE))
                omx_vpuenc_component_Private->video_encoding_type = OMX_VIDEO_CodingMPEG4;
            else if (!strcmp((char *)pComponentRole->cRole, VIDEO_ENC_AVC_ROLE))
                omx_vpuenc_component_Private->video_encoding_type = OMX_VIDEO_CodingAVC;
            else if (!strcmp((char *)pComponentRole->cRole, VIDEO_ENC_H263_ROLE))
                omx_vpuenc_component_Private->video_encoding_type = OMX_VIDEO_CodingH263;
            else if (!strcmp((char *)pComponentRole->cRole, VIDEO_ENC_HEVC_ROLE))
                omx_vpuenc_component_Private->video_encoding_type = OMX_VIDEO_CodingHEVC;
            else
                return OMX_ErrorUndefined;

            SetInternalVideoEncParameters(openmaxStandComp);
            break;
        }
    case OMX_IndexParamVideoH263:
        {
            OMX_VIDEO_PARAM_H263TYPE *pVideoH263 = (OMX_VIDEO_PARAM_H263TYPE *)ComponentParameterStructure;
            portIndex = pVideoH263->nPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if (pVideoH263->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                memcpy(&omx_vpuenc_component_Private->codParam.h263, pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
            else
                return OMX_ErrorBadPortIndex;


            break;
        }
    case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *pVideoMpeg4 = (OMX_VIDEO_PARAM_MPEG4TYPE *)ComponentParameterStructure;
            portIndex = pVideoMpeg4->nPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if (pVideoMpeg4->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                memcpy(&omx_vpuenc_component_Private->codParam.mpeg4, pVideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
            else
                return OMX_ErrorBadPortIndex;

            break;
        }
    case OMX_IndexParamVideoAvc:
        {
            OMX_VIDEO_PARAM_AVCTYPE *pVideoAvc = (OMX_VIDEO_PARAM_AVCTYPE *)ComponentParameterStructure;
            portIndex = pVideoAvc->nPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                memcpy(&omx_vpuenc_component_Private->codParam.avc, pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            else
                return OMX_ErrorBadPortIndex;

            break;
        }

    case OMX_IndexParamVideoQuantization:
        {
            OMX_VIDEO_PARAM_QUANTIZATIONTYPE *pQauntizationType = (OMX_VIDEO_PARAM_QUANTIZATIONTYPE *)ComponentParameterStructure;
            portIndex = pQauntizationType->nPortIndex;

            DEBUG(DEB_LEV_FULL_SEQ, "[SetParameter]OMX_IndexParamVideoQuantization : nQpI=%d, nQpP=%d, nQpB=%d, portIndex=%d\n",
                (int)pQauntizationType->nQpI, (int)pQauntizationType->nQpP, (int)pQauntizationType->nQpB, (int)portIndex);


            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pQauntizationType, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                memcpy(&omx_vpuenc_component_Private->codParam.quantization, pQauntizationType, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
            else
                return OMX_ErrorBadPortIndex;
            break;
        }
    case OMX_IndexParamVideoIntraRefresh:
        {
            OMX_VIDEO_PARAM_INTRAREFRESHTYPE *pIntraRefreshType = (OMX_VIDEO_PARAM_INTRAREFRESHTYPE *)ComponentParameterStructure;
            portIndex = pIntraRefreshType->nPortIndex;

            DEBUG(DEB_LEV_FULL_SEQ, "[SetParameter]OMX_IndexParamVideoIntraRefresh : eRefreshMode=0x%x, nCirMBs=%d, nAirMBs=%d, nAirRef=%d, portIndex=%d\n",
                (int)pIntraRefreshType->eRefreshMode, (int)pIntraRefreshType->nCirMBs, (int)pIntraRefreshType->nAirMBs, (int)pIntraRefreshType->nAirRef, (int)portIndex);

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pIntraRefreshType, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                if (pIntraRefreshType->eRefreshMode == OMX_VIDEO_IntraRefreshAdaptive || pIntraRefreshType->eRefreshMode == OMX_VIDEO_IntraRefreshBoth)
                {
                    eError = OMX_ErrorUnsupportedSetting;
                    DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                    break;
                }

                memcpy(&omx_vpuenc_component_Private->codParam.intraRefresh, pIntraRefreshType, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));

            }
            else
                return OMX_ErrorBadPortIndex;

            break;
        }
    case OMX_IndexParamVideoErrorCorrection:
        {
            //ANDROID Setting
            OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *pErrorCorrectionType = (OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *)ComponentParameterStructure;
            portIndex = pErrorCorrectionType->nPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pErrorCorrectionType, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if(portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                memcpy(&omx_vpuenc_component_Private->codParam.errorrection, pErrorCorrectionType, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
            else
                return OMX_ErrorBadPortIndex;

            break;
        }
    case OMX_IndexParamVideoBitrate:
        {
            OMX_VIDEO_PARAM_BITRATETYPE     *videoRateControl = (OMX_VIDEO_PARAM_BITRATETYPE *)ComponentParameterStructure;
            portIndex = videoRateControl->nPortIndex;

            DEBUG(DEB_LEV_SIMPLE_SEQ, "[SetParameter]OMX_IndexParamVideoBitrate : videoRateControl->eControlRate : %d, bitrate %d", (int)videoRateControl->eControlRate, (int)videoRateControl->nTargetBitrate);

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, videoRateControl, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if(portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                memcpy(&omx_vpuenc_component_Private->codParam.bitrate, videoRateControl, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
            else
                return OMX_ErrorBadPortIndex;


            break;
        }
    case OMX_Extension_IndexPortParamBufferMode:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
            portIndex = pVideoPortFormat->nPortIndex;
            /*Check Structure Header and verify component state*/
            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
            if (eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
            {
                if (pVideoPortFormat->eCompressionFormat == OMX_VIDEO_CodingVendorDmaMode)
                    omx_vpuenc_component_Private->storeMetaDataInDrmBuffer = OMX_TRUE;
            }
            else
                return OMX_ErrorBadPortIndex;

            break;
        }
#ifdef ANDROID
    case OMX_IndexParamStoreMetaDataBuffer:
    case OMX_IndexParamStoreANWBufferInMetadata:
        {
            eError = checkStoreMetaDataBufferHeader(ComponentParameterStructure);
            if (eError != OMX_ErrorNone)
                break;

            eError = checkStoreMetaDataBufferPort(ComponentParameterStructure, &portIndex);
            if (eError != OMX_ErrorNone)
                break;

            if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
                eError = storeMetaDataBuffer(ComponentParameterStructure, &omx_vpuenc_component_Private->storeMetaDataInInputBuffer);
            else
                eError = storeMetaDataBuffer(ComponentParameterStructure, &omx_vpuenc_component_Private->storeMetaDataInOutputBuffer);
            if (eError != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s storeMetaDataBuffer/ANWBufferInMetadata Check Error=%x\n", __func__, eError);
                break;
            }

            DEBUG(DEB_LEV_SIMPLE_SEQ, "[SetParameter]StoreMetaDataBuffer/ANWBufferInMetadata : portIndex=%d, storeMetaDataInInputBuffer=%d, storeMetaDataInOutputBuffer=%d\n",
                (int)portIndex, (int)omx_vpuenc_component_Private->storeMetaDataInInputBuffer, (int)omx_vpuenc_component_Private->storeMetaDataInOutputBuffer);

            break;
        }
#endif
    default: /*Call the base component function*/
        DEBUG(DEB_LEV_FULL_SEQ, "omx_base_component_SetParameter %x\n", paramIndex);
        return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    }
    return eError;
}


OMX_ERRORTYPE omx_vpuenc_component_GetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{

    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *port;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 portIndex;

    if (ComponentParameterStructure == NULL)
        return OMX_ErrorBadParameter;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "Getting parameter 0x%x\n", nParamIndex);
    /* Check which structure we are being fed and fill its header */
    switch((int)nParamIndex)
    {
    case OMX_IndexParamVideoInit:
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone) {
            break;
        }
        memcpy(ComponentParameterStructure, &omx_vpuenc_component_Private->sPortTypesParam[OMX_PortDomainVideo], sizeof(OMX_PORT_PARAM_TYPE));
        break;
    case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat  = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;

            if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone) {
                break;
            }
            port = (omx_base_video_PortType *)omx_vpuenc_component_Private->ports[pVideoPortFormat->nPortIndex];
            if (pVideoPortFormat->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                memcpy(pVideoPortFormat, &port->sVideoParam, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
            }
            else if (pVideoPortFormat->nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
            {
                int nSelectIndex;
                static OMX_COLOR_FORMATTYPE sVpuSupportedPrpColorFormat[] =
                {
                        OMX_COLOR_FormatYUV420Planar,
                        OMX_COLOR_FormatYUV420SemiPlanar
#ifdef ANDROID
                        ,
                        OMX_COLOR_FormatAndroidOpaque
#endif

                };

                nSelectIndex = pVideoPortFormat->nIndex;

                if (nSelectIndex > (int)((sizeof(sVpuSupportedPrpColorFormat)/sizeof(OMX_COLOR_FORMATTYPE))-1)) {
                    return OMX_ErrorNoMore;
                }

                memcpy(pVideoPortFormat, &port->sVideoParam, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
                pVideoPortFormat->nIndex = nSelectIndex;
                pVideoPortFormat->eColorFormat = sVpuSupportedPrpColorFormat[nSelectIndex];

                DEBUG(DEB_LEV_SIMPLE_SEQ, "Out %s OMX_IndexParamVideoPortFormat index=%d, pVideoPortFormat->eColorFormat=0x%x\n", __func__, nSelectIndex, (int)pVideoPortFormat->eColorFormat);
            }
            else
            {
                return OMX_ErrorBadPortIndex;
            }
            break;
        }
    case OMX_IndexParamVideoH263:
        {
            OMX_VIDEO_PARAM_H263TYPE *pVideoH263 = (OMX_VIDEO_PARAM_H263TYPE *)ComponentParameterStructure;
            portIndex = pVideoH263->nPortIndex;

            if (portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_H263TYPE))) != OMX_ErrorNone)
                break;

            memcpy(pVideoH263, &omx_vpuenc_component_Private->codParam.h263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
            break;
        }
    case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *pVideoMpeg4 = (OMX_VIDEO_PARAM_MPEG4TYPE *)ComponentParameterStructure;
            portIndex = pVideoMpeg4->nPortIndex;

            if (portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE))) != OMX_ErrorNone)
                break;

            memcpy(pVideoMpeg4, &omx_vpuenc_component_Private->codParam.mpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
            break;
        }
    case OMX_IndexParamVideoAvc:
        {
            OMX_VIDEO_PARAM_AVCTYPE *pVideoAvc = (OMX_VIDEO_PARAM_AVCTYPE *)ComponentParameterStructure;
            portIndex = pVideoAvc->nPortIndex;

            if (portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            if ((eError = checkHeader(pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE))) != OMX_ErrorNone)
                break;

            memcpy(pVideoAvc, &omx_vpuenc_component_Private->codParam.avc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            break;
        }
    case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pComponentRole = (OMX_PARAM_COMPONENTROLETYPE *)ComponentParameterStructure;

            if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone)
                break;

            if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingMPEG4)
                strcpy((char *)pComponentRole->cRole, VIDEO_ENC_MPEG4_ROLE);
            else if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingAVC)
                strcpy((char *)pComponentRole->cRole, VIDEO_ENC_AVC_ROLE);
            else if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingH263)
                strcpy((char *)pComponentRole->cRole, VIDEO_ENC_H263_ROLE);
            else if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingHEVC)
                strcpy((char *)pComponentRole->cRole, VIDEO_ENC_HEVC_ROLE);
            else
                strcpy((char *)pComponentRole->cRole,"\0");

            break;
        }

        case OMX_IndexParamVideoErrorCorrection:
        {
            //ANDROID Setting
            OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *pErrorCorrectionType = (OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *)ComponentParameterStructure;
            portIndex = pErrorCorrectionType->nPortIndex;

            if(portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pErrorCorrectionType, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            pErrorCorrectionType->bEnableHEC = OMX_FALSE;
            pErrorCorrectionType->bEnableResync = OMX_TRUE;
            pErrorCorrectionType->nResynchMarkerSpacing = 256;
            pErrorCorrectionType->bEnableDataPartitioning = OMX_FALSE;
            pErrorCorrectionType->bEnableRVLC = OMX_FALSE;

            break;
        }
    case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
            portIndex = pPortDefinition->nPortIndex;

            if(portIndex > OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pPortDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            port = (omx_base_video_PortType *)omx_vpuenc_component_Private->ports[portIndex];
            memcpy(pPortDefinition, &port->sPortParam,  sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

#ifdef ANDROID
            DEBUG(DEB_LEV_FULL_SEQ, "%s OMX_IndexParamPortDefinition storeMetadata %d\n", __func__, omx_vpuenc_component_Private->storeMetaDataInInputBuffer);
            if (omx_vpuenc_component_Private->storeMetaDataInInputBuffer) // only android feature
            {
                if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
                {
                    pPortDefinition->nBufferSize = MAX(sizeof(encoder_video_gralloc_metadata_t), sizeof(encoder_video_native_metadata_t));
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "[GetParameter]OMX_IndexParamPortDefinition return nBufferSize=%d \n", (int)pPortDefinition->nBufferSize);
                }
            }
#endif

            if (pPortDefinition->nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
            {
                if (omx_vpuenc_component_Private->portSettingChangeRequest && port->sPortParam.nBufferCountActual != omx_vpuenc_component_Private->nTempBufferCountActual)
                {
                    // it is time to update the new nBufferCountActual value into component. because ILCient does not call SetParameter[OMX_IndexParamPortDefinition] once portSettingChangeRequest.
                    pPortDefinition->nBufferCountActual = omx_vpuenc_component_Private->nTempBufferCountActual;
                }
            }

            break;
        }
    case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *pDstProfileLevel = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*)ComponentParameterStructure;

            portIndex = pDstProfileLevel->nPortIndex;
            DEBUG(DEB_LEV_SIMPLE_SEQ, "[GetParameter]OMX_IndexParamVideoProfileLevelQuerySupported portIndex=%d\n", (int)portIndex);

            if(portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pDstProfileLevel, sizeof(OMX_VIDEO_PARAM_PROFILELEVELTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingAVC)
            {
#ifdef SUPPORT_CM_OMX_12
                if (pDstProfileLevel->nIndex == 0)
#else
                if (pDstProfileLevel->nProfileIndex == 0)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_AVCProfileBaseline;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_AVCLevel4;
                }
#ifdef SUPPORT_CM_OMX_12
                else if (pDstProfileLevel->nIndex == 1)
#else
                else if (pDstProfileLevel->nProfileIndex == 1)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_AVCProfileMain;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_AVCLevel4;
                }
#ifdef SUPPORT_CM_OMX_12
                else if (pDstProfileLevel->nIndex == 2)
#else
                else if (pDstProfileLevel->nProfileIndex == 2)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_AVCProfileHigh;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_AVCLevel42;
                }
                else
                {
#ifdef SUPPORT_CM_OMX_12
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpuenc_component_Private->video_encoding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nProfileIndex, (int)omx_vpuenc_component_Private->video_encoding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
            else if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingMPEG4)
            {
#ifdef SUPPORT_CM_OMX_12
                if (pDstProfileLevel->nIndex == 0)
#else
                if (pDstProfileLevel->nProfileIndex == 0)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_MPEG4ProfileSimple;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_MPEG4Level5;
                }
                else
                {
#ifdef SUPPORT_CM_OMX_12
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpuenc_component_Private->video_encoding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nProfileIndex, (int)omx_vpuenc_component_Private->video_encoding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
            else if (omx_vpuenc_component_Private->video_encoding_type == OMX_VIDEO_CodingH263)
            {
#ifdef SUPPORT_CM_OMX_12
                if (pDstProfileLevel->nIndex == 0)
#else
                if (pDstProfileLevel->nProfileIndex == 0)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_H263ProfileBaseline;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_H263Level70;
                }
                else
                {
#ifdef SUPPORT_CM_OMX_12
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpuenc_component_Private->video_encoding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nProfileIndex, (int)omx_vpuenc_component_Private->video_encoding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
            else
            {
                DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore Unsupported coding type=0x%x\n",
                    (int)omx_vpuenc_component_Private->video_encoding_type);
                return OMX_ErrorNoMore;
            }
#ifdef SUPPORT_CM_OMX_12
            DEBUG(DEB_LEV_SIMPLE_SEQ, "[GetParameter]OMX_IndexParamVideoProfileLevelQuerySupported return ideo_encoding_type=0x%x, nIndex=%d, eProfile=0x%x, eLevel=0x%x\n", (int)omx_vpuenc_component_Private->video_encoding_type, (int)pDstProfileLevel->nIndex, (int)pDstProfileLevel->eProfile, (int)pDstProfileLevel->eLevel);
#else
            DEBUG(DEB_LEV_SIMPLE_SEQ, "[GetParameter]OMX_IndexParamVideoProfileLevelQuerySupported return ideo_encoding_type=0x%x, nProfileIndex=%d, eProfile=0x%x, eLevel=0x%x\n", (int)omx_vpuenc_component_Private->video_encoding_type, (int)pDstProfileLevel->nProfileIndex, (int)pDstProfileLevel->eProfile, (int)pDstProfileLevel->eLevel);
#endif
        }
        break;
    case OMX_IndexParamVideoBitrate:
        {
            OMX_VIDEO_PARAM_BITRATETYPE     *videoRateControl = (OMX_VIDEO_PARAM_BITRATETYPE *)ComponentParameterStructure;
            portIndex = videoRateControl->nPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, videoRateControl, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }

            if(portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                videoRateControl->eControlRate = omx_vpuenc_component_Private->codParam.bitrate.eControlRate;
                videoRateControl->nTargetBitrate = omx_vpuenc_component_Private->codParam.bitrate.nTargetBitrate;
            }
            else
                return OMX_ErrorBadPortIndex;
        }
        break;
    case OMX_IndexParamVideoIntraRefresh:
        {
            OMX_VIDEO_PARAM_INTRAREFRESHTYPE *pIntraRefreshType = (OMX_VIDEO_PARAM_INTRAREFRESHTYPE *)ComponentParameterStructure;
            portIndex = pIntraRefreshType->nPortIndex;

            eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pIntraRefreshType, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
            if(eError!=OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,eError);
                break;
            }
            if(portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                pIntraRefreshType->eRefreshMode = omx_vpuenc_component_Private->codParam.intraRefresh.eRefreshMode;
                pIntraRefreshType->nCirMBs = omx_vpuenc_component_Private->codParam.intraRefresh.nCirMBs;
                pIntraRefreshType->nAirMBs = omx_vpuenc_component_Private->codParam.intraRefresh.nAirMBs;
                pIntraRefreshType->nAirRef = omx_vpuenc_component_Private->codParam.intraRefresh.nAirRef;
            }
            else
                return OMX_ErrorBadPortIndex;
        }
        break;
        case OMX_IndexParamConsumerUsageBits:
        {
#ifdef ANDROID
             OMX_U32 *usageBits = (OMX_U32 *)ComponentParameterStructure;
             *usageBits = GRALLOC_USAGE_SW_READ_OFTEN;
#endif
         }
         break;
    default: /*Call the base component function*/
        return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    }
    return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_vpuenc_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp,internalRequestMessageType *message)
{
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    OMX_STATETYPE eCurrentState = omx_vpuenc_component_Private->state;
    OMX_ERRORTYPE eError;

    DEBUG(DEB_LEV_FULL_SEQ, "In %s messageType=%d, messageParam=%d, eCurrentState=%d\n", __func__, message->messageType, message->messageParam, (int)eCurrentState);

    if (message->messageType == OMX_CommandStateSet)
    {
        if ((message->messageParam == OMX_StateIdle) && (eCurrentState == OMX_StateLoaded))
        {
            eError = omx_vpuenc_component_Init(openmaxStandComp);
            if(eError!=OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Video Encoder Init Failed Error=%x\n",__func__,eError);
                return eError;
            }
        }
        else if ((message->messageParam == OMX_StateLoaded) && (eCurrentState == OMX_StateIdle))
        {

        }
    }

    if (message->messageType == OMX_CommandStateSet)
    {
        if ((message->messageParam == OMX_StateLoaded) && (eCurrentState == OMX_StateIdle))
        {
            eError = omx_vpuenc_component_Deinit(openmaxStandComp);
            if(eError!=OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Video Encoder Deinit Failed Error=%x\n",__func__,eError);
                return eError;
            }
        }
    }

    eError = omx_base_component_MessageHandler(openmaxStandComp,message);

        // Execute the base message handling
    return eError;
}


OMX_ERRORTYPE omx_vpuenc_component_ComponentRoleEnum( OMX_IN OMX_HANDLETYPE hComponent, OMX_OUT OMX_U8 *cRole, OMX_IN OMX_U32 nIndex)
{
    UNREFERENCED_PARAMETER(hComponent);
    OMX_ERRORTYPE error = OMX_ErrorNone;

    switch (nIndex) {
    case 0:
        strcpy((char*) cRole, VIDEO_ENC_MPEG4_ROLE);
        break;
    case 1:
        strcpy((char*) cRole, VIDEO_ENC_AVC_ROLE);
        break;
    case 2:
        strcpy((char*) cRole, VIDEO_ENC_H263_ROLE);
        break;
    default:
        error = OMX_ErrorUnsupportedIndex;
    }

    return error;
}


static CodStd codingTypeToCodStd(OMX_VIDEO_CODINGTYPE codingType)
{
    CodStd codStd = -1;

    switch (codingType) {
    case OMX_VIDEO_CodingAVC:
        codStd = STD_AVC;
        break;
    case OMX_VIDEO_CodingH263:
        codStd = STD_H263;
        break;
    case OMX_VIDEO_CodingMPEG4:
        codStd = STD_MPEG4;
        break;
    case OMX_VIDEO_CodingHEVC:
        codStd = STD_HEVC;
        break;
    default:
        codStd = -1;
    }

    return codStd;
}


void OmxCheckVersion(int coreIdx)
{
    unsigned int version;
    unsigned int revision;
    unsigned int productId;


    VPU_GetVersionInfo(coreIdx, &version, &revision, &productId);

    DEBUG(DEB_LEV_ERR, "VPU coreNum : [%d]\n", coreIdx);
    DEBUG(DEB_LEV_ERR, "Firmware Version => projectId : %x | version : %04x.%04x.%08x | revision : r%d\n",
        (int)(version>>16), (int)((version>>(12))&0x0f), (int)((version>>(8))&0x0f), (int)((version)&0xff), (int)revision);
    DEBUG(DEB_LEV_ERR, "Hardware Version => %04x\n", productId);
    DEBUG(DEB_LEV_ERR, "API Version => %04x\n\n", API_VERSION);
}


OMX_ERRORTYPE omx_videoenc_component_AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_base_component_PrivateType* omx_base_component_Private = (omx_base_component_PrivateType*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_enc_context_t *pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;
    omx_base_PortType *openmaxStandPort;


    if (nPortIndex >= (omx_base_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainImage].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainOther].nPorts)) {
            DEBUG(DEB_LEV_ERR, "In %s: wrong port index\n", __func__);
            return OMX_ErrorBadPortIndex;
    }

    openmaxStandPort = (omx_base_PortType *) omx_base_component_Private->ports[nPortIndex];
    if (openmaxStandPort)
    {
        unsigned int i;
        DEBUG(DEB_LEV_FUNCTION_NAME, "In %s for Component=%p, Port=%p\n", __func__, openmaxStandComp, openmaxStandPort);

        if (nPortIndex != openmaxStandPort->sPortParam.nPortIndex) {
            return OMX_ErrorBadPortIndex;
        }
        if (PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(openmaxStandPort)) {
            return OMX_ErrorBadPortIndex;
        }

        if (omx_base_component_Private->transientState != OMX_TransStateLoadedToIdle) {
            if (!openmaxStandPort->bIsTransientToEnabled) {
                DEBUG(DEB_LEV_ERR, "In %s: The port is not allowed to receive buffers\n", __func__);
                return OMX_ErrorIncorrectStateTransition;
            }
        }

        if (omx_vpuenc_component_Private->storeMetaDataInInputBuffer == OMX_TRUE)
        {
            // it is reasonable to skip
        }
        else
        {
            if(nSizeBytes < openmaxStandPort->sPortParam.nBufferSize) {
                DEBUG(DEB_LEV_ERR, "In %s: Requested Buffer Size %u is less than Minimum Buffer Size %u\n", __func__, nSizeBytes, openmaxStandPort->sPortParam.nBufferSize);
                return OMX_ErrorIncorrectStateTransition;
            }
        }


        if (nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
        {
            if (openmaxStandPort->sPortParam.nBufferCountActual >  MAX_ENC_BITSTREAM_BUFFER_COUNT)
            {
                DEBUG(DEB_LEV_ERR, "In %s: nBufferCountActual[%d] is more than MAX_ENC_BITSTREAM_BUFFER_COUNT[%d]\n", __func__, (int)openmaxStandPort->sPortParam.nBufferCountActual, (int)MAX_ENC_BITSTREAM_BUFFER_COUNT);
                return OMX_ErrorInsufficientResources;
            }
        }

        if (omx_vpuenc_component_Private->portSettingChangeRequest == OMX_TRUE &&
            openmaxStandPort->sPortParam.nBufferCountActual != omx_vpuenc_component_Private->nTempBufferCountActual &&
            nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
        {
            int j;

            if(openmaxStandPort->pInternalBufferStorage)
            {
                free(openmaxStandPort->pInternalBufferStorage);
                openmaxStandPort->pInternalBufferStorage = NULL;
            }

            if(openmaxStandPort->bBufferStateAllocated) {
                free(openmaxStandPort->bBufferStateAllocated);
                openmaxStandPort->bBufferStateAllocated = NULL;
            }

            openmaxStandPort->sPortParam.nBufferCountActual = omx_vpuenc_component_Private->nTempBufferCountActual;

            openmaxStandPort->pInternalBufferStorage = malloc(openmaxStandPort->sPortParam.nBufferCountActual*sizeof(OMX_BUFFERHEADERTYPE *));
            if (!openmaxStandPort->pInternalBufferStorage) {
                DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
                return OMX_ErrorInsufficientResources;
            }
            memset(openmaxStandPort->pInternalBufferStorage, 0x00, openmaxStandPort->sPortParam.nBufferCountActual*sizeof(OMX_BUFFERHEADERTYPE *));

            openmaxStandPort->bBufferStateAllocated = malloc(openmaxStandPort->sPortParam.nBufferCountActual*sizeof(BUFFER_STATUS_FLAG));
            if (!openmaxStandPort->bBufferStateAllocated) {
                DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
                return OMX_ErrorInsufficientResources;
            }
            memset(openmaxStandPort->bBufferStateAllocated, 0x00, openmaxStandPort->sPortParam.nBufferCountActual*sizeof(BUFFER_STATUS_FLAG));

            for(j=0; j < (int)openmaxStandPort->sPortParam.nBufferCountActual; j++) {
                openmaxStandPort->bBufferStateAllocated[j] = BUFFER_FREE;
            }
        }
        for(i=0; i < openmaxStandPort->sPortParam.nBufferCountActual; i++)
        {
            if (openmaxStandPort->bBufferStateAllocated[i] == BUFFER_FREE)
            {
                openmaxStandPort->pInternalBufferStorage[i] = malloc(sizeof(OMX_BUFFERHEADERTYPE));
                if (!openmaxStandPort->pInternalBufferStorage[i]) {
                    DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
                    return OMX_ErrorInsufficientResources;
                }
                memset(openmaxStandPort->pInternalBufferStorage[i], 0x00, sizeof(OMX_BUFFERHEADERTYPE));

                setHeader(openmaxStandPort->pInternalBufferStorage[i], sizeof(OMX_BUFFERHEADERTYPE));
                if (openmaxStandPort->sPortParam.eDir == OMX_DirOutput)
                {
                    /* allocate the buffer */
                    pVpu->vbStream[i].size = nSizeBytes;
                    pVpu->vbStream[i].size = ((pVpu->vbStream[i].size+1023)&~1023);

                    if (vdi_allocate_dma_memory(pVpu->coreIdx, &pVpu->vbStream[i]) < 0)
                    {
                        DEBUG(DEB_LEV_ERR, "fail to allocate bitstream buffer bufferIdx=%d\n", (int)i);
                        return OMX_ErrorInsufficientResources;
                    }
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_AllocateBuffer for output allocate bitstream phys_addr=0x%x, virt_addr=0x%x, size = %d\n", (int)pVpu->vbStream[i].phys_addr, (int)pVpu->vbStream[i].virt_addr, (int)pVpu->vbStream[i].size);
                    openmaxStandPort->pInternalBufferStorage[i]->pBuffer = (OMX_U8 *)pVpu->vbStream[i].virt_addr;
                    memset(openmaxStandPort->pInternalBufferStorage[i]->pBuffer, 0x00, nSizeBytes);

                    openmaxStandPort->pInternalBufferStorage[i]->nOutputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
                }
                else
                {
                    DEBUG(DEB_LEV_FULL_SEQ, "%s storeMetadata %d\n", __func__, omx_vpuenc_component_Private->storeMetaDataInInputBuffer);
                    if (omx_vpuenc_component_Private->storeMetaDataInInputBuffer)
                    {
                        // in case of storeMetaDataInInputBuffer on android, it is good to allocate input buffer on CPU buffer. because the buffer will have just metadata information
                        openmaxStandPort->pInternalBufferStorage[i]->pBuffer = malloc(nSizeBytes);
                        if(openmaxStandPort->pInternalBufferStorage[i]->pBuffer==NULL) {
                            DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
                            return OMX_ErrorInsufficientResources;
                        }
                        memset(openmaxStandPort->pInternalBufferStorage[i]->pBuffer, 0x00, nSizeBytes);
                        DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_AllocateBuffer for input allocate storeMetaDataInInputBuffer pBuffer=%p, size = %d\n", openmaxStandPort->pInternalBufferStorage[i]->pBuffer, (int)nSizeBytes);
                    }
                    else
                    {
                        /* allocate the buffer on CMA buffer*/
                        pVpu->vbSrcFb[i].size = nSizeBytes;
                        if (vdi_allocate_dma_memory(pVpu->coreIdx, &pVpu->vbSrcFb[i]) < 0)
                        {
                            DEBUG(DEB_LEV_ERR, "fail to allocate framebuffer buffer bufferIdx=%d\n", (int)i);
                            return OMX_ErrorInsufficientResources;
                        }
                        DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_AllocateBuffer for input allocate framebuffer physic addr = 0x%x, virt_addr = 0x%x, size = %d\n", (int)pVpu->vbSrcFb[i].phys_addr, (int)pVpu->vbSrcFb[i].virt_addr, (int)pVpu->vbSrcFb[i].size);
                        openmaxStandPort->pInternalBufferStorage[i]->pBuffer = (OMX_U8 *)pVpu->vbSrcFb[i].virt_addr;
                        memset(openmaxStandPort->pInternalBufferStorage[i]->pBuffer, 0x00, nSizeBytes);
                    }

                    openmaxStandPort->pInternalBufferStorage[i]->nInputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
                }
                openmaxStandPort->pInternalBufferStorage[i]->nAllocLen = nSizeBytes;
                openmaxStandPort->pInternalBufferStorage[i]->pPlatformPrivate = openmaxStandPort;
                openmaxStandPort->pInternalBufferStorage[i]->pAppPrivate = pAppPrivate;
                *ppBuffer = openmaxStandPort->pInternalBufferStorage[i];
                openmaxStandPort->bBufferStateAllocated[i] = BUFFER_ALLOCATED;
                openmaxStandPort->bBufferStateAllocated[i] |= HEADER_ALLOCATED;
                openmaxStandPort->nNumAssignedBuffers++;

                if (openmaxStandPort->sPortParam.nBufferCountActual == openmaxStandPort->nNumAssignedBuffers)
                {
                    openmaxStandPort->sPortParam.bPopulated = OMX_TRUE;
                    openmaxStandPort->bIsFullOfBuffers = OMX_TRUE;
                    if (openmaxStandPort->sPortParam.eDir == OMX_DirOutput)
                        omx_vpuenc_component_Private->bUseOmxAllocateBufferOfOutPort = OMX_TRUE;
                    else
                        omx_vpuenc_component_Private->bUseOmxAllocateBufferOfInputPort = OMX_TRUE;
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s nPortIndex=%d\n",__func__,(int)nPortIndex);
                    tsem_up(openmaxStandPort->pAllocSem);

                    if (omx_vpuenc_component_Private->portSettingChangeRequest == OMX_TRUE)
                    {
                        omx_vpuenc_component_Private->portSettingChangeRequest = OMX_FALSE;
                    }
                }
                DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for port %p\n", __func__, openmaxStandPort);
                return OMX_ErrorNone;
            }
        }

        DEBUG(DEB_LEV_ERR, "Out of %s for port %p. Error: no available buffers\n",__func__, openmaxStandPort);
        return OMX_ErrorInsufficientResources;
    }
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out %s for Component=%p, Port=%p, buffer %p\n", __func__, openmaxStandComp, openmaxStandPort, ppBuffer);

    return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_videoenc_component_UseBuffer(
    OMX_HANDLETYPE hComponent,
    OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_U32 nPortIndex,
    OMX_PTR pAppPrivate,
    OMX_U32 nSizeBytes,
    OMX_U8* pBuffer)
{
    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_base_component_PrivateType* omx_base_component_Private = (omx_base_component_PrivateType*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_PortType *openmaxStandPort;

    openmaxStandPort = (omx_base_PortType *) omx_base_component_Private->ports[nPortIndex];
    if (openmaxStandPort)
    {
        unsigned int i;
        DEBUG(DEB_LEV_FULL_SEQ, "In %s for Component=%p, port=%d, pBuffer=%p\n", __func__, openmaxStandComp, nPortIndex, pBuffer);

        if (nPortIndex != openmaxStandPort->sPortParam.nPortIndex) {
            return OMX_ErrorBadPortIndex;
        }

        if (PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(openmaxStandPort)) {
            return OMX_ErrorBadPortIndex;
        }

        if (omx_base_component_Private->transientState != OMX_TransStateLoadedToIdle) {
            if (!openmaxStandPort->bIsTransientToEnabled) {
                DEBUG(DEB_LEV_ERR, "In %s: The port is not allowed to receive buffers\n", __func__);
                return OMX_ErrorIncorrectStateTransition;
            }
        }

        if (omx_vpuenc_component_Private->storeMetaDataInInputBuffer == OMX_TRUE
            || omx_vpuenc_component_Private->storeMetaDataInDrmBuffer == OMX_TRUE)
        {
            // it is reasonable to skip
            DEBUG(DEB_LEV_FULL_SEQ, "In %s storeMetaDataInInputBuffer to skip\n", __func__);
        }
        else
        {
            if(nSizeBytes < openmaxStandPort->sPortParam.nBufferSize) {
                DEBUG(DEB_LEV_ERR, "In %s: Requested Buffer Size %u is less than Minimum Buffer Size %u\n", __func__, nSizeBytes, openmaxStandPort->sPortParam.nBufferSize);
                return OMX_ErrorIncorrectStateTransition;
            }
        }

        if (nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
        {
            if (openmaxStandPort->sPortParam.nBufferCountActual >  MAX_ENC_BITSTREAM_BUFFER_COUNT)
            {
                DEBUG(DEB_LEV_ERR, "In %s: nBufferCountActual[%d] is more than MAX_ENC_BITSTREAM_BUFFER_COUNT[%d]\n", __func__, (int)openmaxStandPort->sPortParam.nBufferCountActual, (int)MAX_ENC_BITSTREAM_BUFFER_COUNT);
                return OMX_ErrorInsufficientResources;
            }
        }

        i = openmaxStandPort->nNumAssignedBuffers;
        DEBUG(DEB_LEV_SIMPLE_SEQ, "%s nBufferCountActual %d, nNumAssignedBuffers %d, i %d\n", __func__,
            openmaxStandPort->sPortParam.nBufferCountActual, openmaxStandPort->nNumAssignedBuffers, i);
        if (openmaxStandPort->bBufferStateAllocated[i] == BUFFER_FREE)
        {
            openmaxStandPort->pInternalBufferStorage[i] = malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (!openmaxStandPort->pInternalBufferStorage[i]) {
                DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
                return OMX_ErrorInsufficientResources;
            }
            memset(openmaxStandPort->pInternalBufferStorage[i], 0x00, sizeof(OMX_BUFFERHEADERTYPE));
            openmaxStandPort->bIsEmptyOfBuffers = OMX_FALSE;
            setHeader(openmaxStandPort->pInternalBufferStorage[i], sizeof(OMX_BUFFERHEADERTYPE));

            openmaxStandPort->pInternalBufferStorage[i]->pBuffer = pBuffer;
            openmaxStandPort->pInternalBufferStorage[i]->nAllocLen = nSizeBytes;
            openmaxStandPort->pInternalBufferStorage[i]->pPlatformPrivate = openmaxStandPort;
            openmaxStandPort->pInternalBufferStorage[i]->pAppPrivate = pAppPrivate;
            openmaxStandPort->bBufferStateAllocated[i] = BUFFER_ASSIGNED;
            openmaxStandPort->bBufferStateAllocated[i] |= HEADER_ALLOCATED;

            if (openmaxStandPort->sPortParam.eDir == OMX_DirInput) {
                openmaxStandPort->pInternalBufferStorage[i]->nInputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
            } else {
                openmaxStandPort->pInternalBufferStorage[i]->nOutputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
            }

            *ppBufferHdr = openmaxStandPort->pInternalBufferStorage[i];
            openmaxStandPort->nNumAssignedBuffers++;
            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s [%u] ppBufferHdr=%p, (portIndex=%d), nNumAssignedBuffers %d\n", __func__,
                    i, ppBufferHdr, (int)openmaxStandPort->sPortParam.eDir, openmaxStandPort->nNumAssignedBuffers);

            if (openmaxStandPort->sPortParam.nBufferCountActual == openmaxStandPort->nNumAssignedBuffers)
            {
                openmaxStandPort->sPortParam.bPopulated = OMX_TRUE;
                openmaxStandPort->bIsFullOfBuffers = OMX_TRUE;
                if (openmaxStandPort->sPortParam.eDir == OMX_DirOutput)
                    omx_vpuenc_component_Private->bUseOmxAllocateBufferOfOutPort = OMX_FALSE;
                else
                    omx_vpuenc_component_Private->bUseOmxAllocateBufferOfInputPort = OMX_FALSE;
                DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s nPortIndex=%d populated\n",__func__,(int)nPortIndex);
                tsem_up(openmaxStandPort->pAllocSem);

                if (omx_vpuenc_component_Private->portSettingChangeRequest == OMX_TRUE)
                {
                    omx_vpuenc_component_Private->portSettingChangeRequest = OMX_FALSE;
                }
            }
            DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for port %p\n", __func__, openmaxStandPort);
            return OMX_ErrorNone;
        }

        DEBUG(DEB_LEV_ERR, "Out of %s for port %p. Error: no available buffers\n",__func__, openmaxStandPort);
        return OMX_ErrorInsufficientResources;
    }

    DEBUG(DEB_LEV_FUNCTION_NAME, "Out %s for Component=%p, Port=%p, buffer %p\n", __func__, openmaxStandComp, openmaxStandPort, ppBufferHdr);
    return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_videoenc_component_FreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{

    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_base_component_PrivateType* omx_base_component_Private = (omx_base_component_PrivateType*)openmaxStandComp->pComponentPrivate;
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_enc_context_t *pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;
    omx_base_PortType *openmaxStandPort;


    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s for component %p\n", __func__, hComponent);
    if (nPortIndex >= (omx_base_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainImage].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainOther].nPorts)) {
            DEBUG(DEB_LEV_ERR, "In %s: wrong port index\n", __func__);
            return OMX_ErrorBadPortIndex;
    }

    openmaxStandPort = omx_base_component_Private->ports[nPortIndex];
    if (openmaxStandPort)
    {
        unsigned int i;
        OMX_COMPONENTTYPE* omxComponent = openmaxStandPort->standCompContainer;
        omx_base_component_PrivateType* omx_base_component_Private = (omx_base_component_PrivateType*)omxComponent->pComponentPrivate;
        DEBUG(DEB_LEV_FUNCTION_NAME, "In %s for port %p, nNumAssignedBuffers=%d\n", __func__, openmaxStandPort, (int)openmaxStandPort->nNumAssignedBuffers);

        if (nPortIndex != openmaxStandPort->sPortParam.nPortIndex) {
            return OMX_ErrorBadPortIndex;
        }
        if (PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(openmaxStandPort)) {
            return OMX_ErrorBadPortIndex;
        }

        if (omx_base_component_Private->transientState != OMX_TransStateIdleToLoaded) {
            if (!openmaxStandPort->bIsTransientToDisabled) {
                DEBUG(DEB_LEV_FULL_SEQ, "In %s: The port is not allowed to free the buffers\n", __func__);
                (*(omx_base_component_Private->callbacks->EventHandler))
                    (omxComponent,
                    omx_base_component_Private->callbackData,
                    OMX_EventError, /* The command was completed */
                    OMX_ErrorPortUnpopulated, /* The commands was a OMX_CommandStateSet */
                    nPortIndex, /* The state has been changed in message->messageParam2 */
                    NULL);
            }
        }

        for(i=0; i < openmaxStandPort->sPortParam.nBufferCountActual; i++){
            if (openmaxStandPort->bBufferStateAllocated[i] & (BUFFER_ASSIGNED | BUFFER_ALLOCATED))  // OMX_AllocateBuffer or OMX_UseBuffer have been invoked.
            {
                openmaxStandPort->bIsFullOfBuffers = OMX_FALSE;
                if (openmaxStandPort->bBufferStateAllocated[i] & BUFFER_ALLOCATED) // OMX_AllocateBuffer has been invoked by IL Client.
                {
                    if(openmaxStandPort->pInternalBufferStorage[i] == pBuffer)
                    {
                        if (openmaxStandPort->sPortParam.eDir == OMX_DirOutput)
                        {

                            DEBUG(DEB_LEV_FULL_SEQ, "freeing vpu bitstream buffer for output OMX_AllocateBuffer  pBuffer = %p, virt_addr=0x%x, size=%d\n", openmaxStandPort->pInternalBufferStorage[i]->pBuffer, (int)pVpu->vbStream[i].virt_addr, (int)pVpu->vbStream[i].size);
                            if (pVpu->vbStream[i].size > 0)
                            {
                                vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbStream[i]);

                                openmaxStandPort->pInternalBufferStorage[i]->pBuffer=NULL;
                            }
                            else
                            {
                                DEBUG(DEB_LEV_ERR, "fail to find bitstream buffer that should free pBuffer=%p, virt_addr=0x%x, size=%d\n", openmaxStandPort->pInternalBufferStorage[i]->pBuffer, (int)pVpu->vbStream[i].virt_addr, (int)pVpu->vbStream[i].size);
                                return OMX_ErrorInsufficientResources;
                            }
                        }
                        else  // for input
                        {
                            if (omx_vpuenc_component_Private->storeMetaDataInInputBuffer)
                            {
                                DEBUG(DEB_LEV_SIMPLE_SEQ, "freeing  to storeMetaDataInInputBuffer for output OMX_AllocateBuffer pBuffer = %p\n", openmaxStandPort->pInternalBufferStorage[i]->pBuffer);
                                if (openmaxStandPort->pInternalBufferStorage[i]->pBuffer)
                                {
                                    free(openmaxStandPort->pInternalBufferStorage[i]->pBuffer);
                                    openmaxStandPort->pInternalBufferStorage[i]->pBuffer=NULL;
                                }
                            }
                            else
                            {
                                DEBUG(DEB_LEV_SIMPLE_SEQ, "freeing  vpu frame buffer  pBuffer = %p, virt_addr=0x%x, size=%d\n", openmaxStandPort->pInternalBufferStorage[i]->pBuffer, (int)pVpu->vbSrcFb[i].virt_addr, (int)pVpu->vbSrcFb[i].size);
                                if (pVpu->vbSrcFb[i].size > 0)
                                {
                                    vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbSrcFb[i]);
                                    openmaxStandPort->pInternalBufferStorage[i]->pBuffer=NULL;
                                }
                            }

                        }
                    }
                    else
                    {
                        DEBUG(DEB_LEV_FULL_SEQ, "!!!!!! buffer not match[%u] , port:%u\n", i, (int)nPortIndex);
                        continue;
                    }
                }
                else if (openmaxStandPort->bBufferStateAllocated[i] & BUFFER_ASSIGNED) // OMX_UseBuffer has been invoked by IL Client.
                {
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "freeing  pBuffer = %p, portIndex=%d for OMX_UseBuffer\n", pBuffer, (int)nPortIndex);
                    if (openmaxStandPort->pInternalBufferStorage[i]->pBuffer != pBuffer->pBuffer)
                    {
                        DEBUG(DEB_LEV_FULL_SEQ, "!!!!!! buffer not match[%u] , port:%u\n", i, (int)nPortIndex);
                        continue;
                    }
                }

                if(openmaxStandPort->bBufferStateAllocated[i] & HEADER_ALLOCATED) {
                    if (openmaxStandPort->pInternalBufferStorage[i])
                    {
                        free(openmaxStandPort->pInternalBufferStorage[i]);
                        openmaxStandPort->pInternalBufferStorage[i]=NULL;
                    }
                }

                openmaxStandPort->bBufferStateAllocated[i] = BUFFER_FREE;

                openmaxStandPort->nNumAssignedBuffers--;

                if (openmaxStandPort->nNumAssignedBuffers == 0) {
                    openmaxStandPort->sPortParam.bPopulated = OMX_FALSE;
                    openmaxStandPort->bIsEmptyOfBuffers = OMX_TRUE;
                    tsem_up(openmaxStandPort->pAllocSem);
                }
                DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for port %p\n", __func__, openmaxStandPort);
                return OMX_ErrorNone;
            }
        }
        DEBUG(DEB_LEV_ERR, "Out of %s for port %p with OMX_ErrorInsufficientResources\n", __func__, openmaxStandPort);
        return OMX_ErrorInsufficientResources;
    }
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for component %p\n", __func__, hComponent);
    return OMX_ErrorNone;

}


int OmxGetVpuBsBufferByVirtualAddress(vpu_enc_context_t *pVpu, vpu_buffer_t *vb, unsigned long virt_addr)
{
    int i;

    for (i=0; i < MAX_ENC_BITSTREAM_BUFFER_COUNT; i++)
    {
        if (pVpu->vbStream[i].virt_addr == virt_addr)
        {
            *vb = pVpu->vbStream[i];
            return i;
        }
    }

    return -1;
}


int OmxGetInputBufferIndex(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer)
{
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *inPort = (omx_base_video_PortType *) omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    int i;

    for (i=0; i < (int)inPort->sPortParam.nBufferCountActual; i++)
    {
        if (inPort->pInternalBufferStorage[i] == pInputBuffer)
            return i;
    }

    return -1;
}


static inline void OmxWriteYuvData(OMX_BYTE pYuv, vpu_enc_context_t *pVpu, FrameBuffer* fb, OMX_BOOL reAlign)
{
    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s srcFrameBufSize=%d reAlign:%d\n",
          __func__, pVpu->srcFrameBufSize, reAlign);

    OMX_BYTE pSrc = NULL;
    PhysicalAddress pDst;
    int nY, nCb, nCr, y;
    int lumaSize, lumaStride, chromaSize, chromaStride, chromaWidth;

    if (!reAlign)
    {
        vdi_write_memory(pVpu->coreIdx, fb->bufY, (unsigned char *)pYuv,
                         pVpu->srcFrameBufSize, pVpu->encOP.frameEndian);
        OMX_DUMP_INPUT_YUV_TO_FILE(pVpu, pYuv, pVpu->srcFrameBufSize);
    }
    else
    {
        DEBUG(DEB_LEV_ERR, "In %s srcFrameStride=%d, srcWidth=%d, srcHeight=%d\n",
              __func__, pVpu->srcFrameStride, pVpu->encOP.picWidth, pVpu->encOP.picHeight);
        nY = pVpu->encOP.picHeight;
        lumaStride = pVpu->srcFrameStride;
        lumaSize = pVpu->encOP.picWidth * pVpu->encOP.picHeight;

        nCb = nCr = nY / 2;
        chromaSize = lumaSize / 4;
        chromaStride = lumaStride / 2;
        chromaWidth = pVpu->encOP.picWidth / 2;

        pDst = fb->bufY;
        pSrc = pYuv;
        for (y = 0; y < nY; ++y)
        {
            vdi_write_memory(pVpu->coreIdx, pDst + lumaStride * y,
                             (unsigned char *)(pYuv + y * pVpu->encOP.picWidth),
                             pVpu->encOP.picWidth, pVpu->encOP.frameEndian);
            OMX_DUMP_INPUT_YUV_TO_FILE(pVpu, pYuv + y * pVpu->encOP.picWidth, lumaStride);
        }

        pSrc = pYuv + lumaSize;
        pDst = fb->bufCb;
        for (y = 0; y < nCb; ++y)
        {
            vdi_write_memory(pVpu->coreIdx, pDst + chromaStride * y,
                             (unsigned char *)(pSrc + y * chromaWidth),
                             chromaWidth, pVpu->encOP.frameEndian);
            OMX_DUMP_INPUT_YUV_TO_FILE(pVpu, pSrc + y * chromaWidth, chromaStride);
        }

        pSrc = pYuv + lumaSize + chromaSize;
        pDst = fb->bufCr;
        for (y = 0; y < nCr; ++y)
        {
            vdi_write_memory(pVpu->coreIdx, pDst + chromaStride * y,
                             (unsigned char *)(pSrc + y * chromaWidth),
                             chromaWidth, pVpu->encOP.frameEndian);
            OMX_DUMP_INPUT_YUV_TO_FILE(pVpu, pSrc + y * chromaWidth, chromaStride);
        }
    }
}


OMX_BOOL OmxConfigEncoderParameter(OMX_COMPONENTTYPE *openmaxStandComp)
{
    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_enc_context_t *pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;
    RetCode ret;

    if (omx_vpuenc_component_Private->requestIFrame == OMX_TRUE)
    {
        if (omx_vpuenc_component_Private->codParam.configIntraRefreshVopType.IntraRefreshVOP == OMX_TRUE)
            pVpu->encParam.forceIPicture = 1;
        else
            pVpu->encParam.forceIPicture = 0;
        omx_vpuenc_component_Private->requestIFrame = OMX_FALSE;
    }

    if (omx_vpuenc_component_Private->requestChangeBitrate == OMX_TRUE)
    {
        int newBitrateKbps = (omx_vpuenc_component_Private->codParam.videoConfigBitrateType.nEncodeBitrate/1024);

        ret = VPU_EncGiveCommand(pVpu->handle, ENC_SET_BITRATE, &newBitrateKbps);
        if (ret != RETCODE_SUCCESS)
        {
            DEBUG(DEB_LEV_ERR, "In %s Fail to VPU_EncGiveCommand[ENC_SET_BITRATE] retCode=0x%x\n",__func__, (int)ret);
            goto ERR_CONFIG_ENC_PARAMETER;
        }
        omx_vpuenc_component_Private->requestChangeBitrate = OMX_FALSE;
    }

    if (omx_vpuenc_component_Private->requestChangeFramerate == OMX_TRUE)
    {
        int newFramerate= (omx_vpuenc_component_Private->codParam.configFramerateType.xEncodeFramerate>>16);

        ret = VPU_EncGiveCommand(pVpu->handle, ENC_SET_FRAME_RATE, &newFramerate);
        if (ret != RETCODE_SUCCESS)
        {
            DEBUG(DEB_LEV_ERR, "In %s Fail to VPU_EncGiveCommand[ENC_SET_FRAME_RATE] retCode=0x%x\n",__func__, (int)ret);
            goto ERR_CONFIG_ENC_PARAMETER;
        }
        omx_vpuenc_component_Private->requestChangeFramerate = OMX_FALSE;
    }

    if (omx_vpuenc_component_Private->requestChangeGopSize == OMX_TRUE)
    {
        int newGopNum = (omx_vpuenc_component_Private->codParam.videoConfigAvcIntraPeriod.nPFrames + 1);

        ret = VPU_EncGiveCommand(pVpu->handle, ENC_SET_GOP_NUMBER, &newGopNum);
        if (ret != RETCODE_SUCCESS)
        {
            DEBUG(DEB_LEV_ERR, "In %s Fail to VPU_EncGiveCommand[ENC_SET_GOP_NUMBER] retCode=0x%x\n",__func__, (int)ret);
            goto ERR_CONFIG_ENC_PARAMETER;
        }
        omx_vpuenc_component_Private->requestChangeGopSize = OMX_FALSE;
    }


    return OMX_TRUE;

ERR_CONFIG_ENC_PARAMETER:
    return OMX_FALSE;
}


#ifdef ANDROID
OMX_U32 OmxGetCorrespondColorFormat(OMX_U32 grallocFormat, omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private)
{
    OMX_U32 colorFormat = OMX_COLOR_FormatYUV420Planar;

    DEBUG(DEB_LEV_FULL_SEQ, "%s grallocFormat %d\n", __func__, grallocFormat);

    // default to true, a format conversion is needed
    omx_vpuenc_component_Private->bFormatConversion = OMX_TRUE;

    //default camera settings, should be consistent with the camera settings
    switch (grallocFormat) {
    case HAL_PIXEL_FORMAT_YCRCB_420_SP:
        colorFormat = MAKE_FOURCC('N', 'V', '2', '1');
    case HAL_PIXEL_FORMAT_YCBCR_420_888:
        omx_vpuenc_component_Private->bFormatConversion = OMX_FALSE;
        break;
    default:
        break;
    }

    return colorFormat;
}
#endif

OMX_ERRORTYPE OmxGetCorrespondPhysAddr(vpu_enc_context_t *pVpu, OMX_S32 bufHandle, PhysicalAddress *physAddr)
{
    vpu_buffer_t vdb_temp = {0};

    DEBUG(DEB_LEV_FULL_SEQ, "map buf handle: %d to phys addr\n", bufHandle);
    vdb_temp.buf_handle = bufHandle;
    if (vdi_device_memory_map(pVpu->coreIdx, &vdb_temp) < 0) {
        DEBUG(DEB_LEV_ERR, "fail to map memory \n" );
        return OMX_ErrorBadParameter;
    }

    DEBUG(DEB_LEV_FULL_SEQ, "map buf handle: %d to phys addr: %"PRIu64" successfully \n", bufHandle, vdb_temp.phys_addr);

    *physAddr = vdb_temp.phys_addr;
    vdi_device_memory_unmap(pVpu->coreIdx, &vdb_temp);
    return OMX_ErrorNone;
}


void* omx_vpuenc_component_BufferMgmtFunction (void* param)
{
    OMX_COMPONENTTYPE* openmaxStandComp = (OMX_COMPONENTTYPE*)param;

    omx_vpuenc_component_PrivateType* omx_vpuenc_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *pInPort = (omx_base_video_PortType *) omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_base_video_PortType *pOutPort = (omx_base_video_PortType *) omx_vpuenc_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    vpu_enc_context_t *pVpu = (vpu_enc_context_t *) & omx_vpuenc_component_Private->vpu;

    tsem_t* pInputSem = pInPort->pBufferSem;
    tsem_t* pOutputSem = pOutPort->pBufferSem;
    queue_t* pInputQueue = pInPort->pBufferQueue;
    queue_t* pOutputQueue = pOutPort->pBufferQueue;
    OMX_BUFFERHEADERTYPE* pOutputBuffer=NULL;
    OMX_BUFFERHEADERTYPE* pInputBuffer=NULL;
    OMX_BOOL isInputBufferNeeded=OMX_TRUE,isOutputBufferNeeded=OMX_TRUE;
    int inBufExchanged=0,outBufExchanged=0;

    omx_vpuenc_component_Private->bIsOutputEOSReached = OMX_FALSE;
#if defined(WIN32)
    omx_vpuenc_component_Private->bellagioThreads->nThreadBufferMngtID = (long int)GetCurrentThreadId();
#else
    omx_vpuenc_component_Private->bellagioThreads->nThreadBufferMngtID = (long int)syscall(__NR_gettid);
#endif
    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s of component %p\n", __func__, openmaxStandComp);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s the thread ID is %i\n", __func__, (int)omx_vpuenc_component_Private->bellagioThreads->nThreadBufferMngtID);

    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_VIDEO);

    /* checks if the component is in a state able to receive buffers */
    while(!omx_vpuenc_component_Private->bIsBufMgThreadExit && (omx_vpuenc_component_Private->state == OMX_StateIdle || omx_vpuenc_component_Private->state == OMX_StateExecuting ||
        omx_vpuenc_component_Private->state == OMX_StatePause || omx_vpuenc_component_Private->transientState == OMX_TransStateLoadedToIdle))
    {

        /*Wait till the ports are being flushed*/
        pthread_mutex_lock(&omx_vpuenc_component_Private->flush_mutex);
        while( PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))
        {
            pthread_mutex_unlock(&omx_vpuenc_component_Private->flush_mutex);

            DEBUG(DEB_LEV_FULL_SEQ, "In %s 1 signaling flush all cond iE=%d,iF=%d,oE=%d,oF=%d iSemVal=%d,oSemval=%d\n",
                __func__,inBufExchanged, (int)isInputBufferNeeded, outBufExchanged, (int)isOutputBufferNeeded, (int)pInputSem->semval, (int)pOutputSem->semval);

            if(isOutputBufferNeeded==OMX_FALSE && PORT_IS_BEING_FLUSHED(pOutPort)) {
                pOutPort->ReturnBufferFunction((omx_base_PortType *)pOutPort, pOutputBuffer);
                DEBUG(DEB_LEV_FULL_SEQ, "Ports are flushing,so returning output buffer = %p\n", pOutputBuffer);
                outBufExchanged--;
                pOutputBuffer=NULL;
                isOutputBufferNeeded=OMX_TRUE;
            }

            if(isInputBufferNeeded==OMX_FALSE && PORT_IS_BEING_FLUSHED(pInPort)) {
                pInPort->ReturnBufferFunction((omx_base_PortType *)pInPort,pInputBuffer);
                DEBUG(DEB_LEV_FULL_SEQ, "Ports are flushing,so returning input buffer = %p\n", pInputBuffer);
                inBufExchanged--;
                pInputBuffer=NULL;
                isInputBufferNeeded=OMX_TRUE;
            }

            DEBUG(DEB_LEV_FULL_SEQ, "In %s 2 signaling flush all cond iE=%d,iF=%d,oE=%d,oF=%d iSemVal=%d,oSemval=%d\n",
                __func__,inBufExchanged, (int)isInputBufferNeeded, outBufExchanged, (int)isOutputBufferNeeded, (int)pInputSem->semval, (int)pOutputSem->semval);

            tsem_up(omx_vpuenc_component_Private->flush_all_condition); // this giving signal to resume base_port_FlushProcessingBuffers.
            tsem_down(omx_vpuenc_component_Private->flush_condition); // this waiting for all buffer flushed in queue from base_port_FlushProcessingBuffers
            pthread_mutex_lock(&omx_vpuenc_component_Private->flush_mutex);
        }
        pthread_mutex_unlock(&omx_vpuenc_component_Private->flush_mutex);
        /*No buffer to process. So wait here*/
        if((isInputBufferNeeded==OMX_TRUE && pInputSem->semval==0) &&
#ifdef SUPPORT_CM_OMX_12
            (omx_vpuenc_component_Private->state != OMX_StateLoaded)) {
#else
            (omx_vpuenc_component_Private->state != OMX_StateLoaded && omx_vpuenc_component_Private->state != OMX_StateInvalid)) {
#endif
                //Signaled from EmptyThisBuffer or FillThisBuffer or some thing else
                DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next input buffer omx_vpuenc_component_Private->state=0x%x\n", (int)omx_vpuenc_component_Private->state);
                tsem_down(omx_vpuenc_component_Private->bMgmtSem);

        }

#ifdef SUPPORT_CM_OMX_12
        if(omx_vpuenc_component_Private->state == OMX_StateLoaded) {
#else
        if(omx_vpuenc_component_Private->state == OMX_StateLoaded || omx_vpuenc_component_Private->state == OMX_StateInvalid) {
#endif
            DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Input Buffer Management Thread is exiting\n",__func__);
            break;
        }

        if((isOutputBufferNeeded==OMX_TRUE && pOutputSem->semval==0) &&
#ifdef SUPPORT_CM_OMX_12
            (omx_vpuenc_component_Private->state != OMX_StateLoaded) &&
#else
            (omx_vpuenc_component_Private->state != OMX_StateLoaded && omx_vpuenc_component_Private->state != OMX_StateInvalid) &&
#endif
            !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
                //Signaled from EmptyThisBuffer or FillThisBuffer or some thing else
                DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next output buffer omx_vpuenc_component_Private->state=0x%x\n", (int)omx_vpuenc_component_Private->state);
                tsem_down(omx_vpuenc_component_Private->bMgmtSem);

        }

#ifdef SUPPORT_CM_OMX_12
        if(omx_vpuenc_component_Private->state == OMX_StateLoaded) {
#else
        if(omx_vpuenc_component_Private->state == OMX_StateLoaded || omx_vpuenc_component_Private->state == OMX_StateInvalid) {
#endif
            DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Output Buffer Management Thread is exiting\n",__func__);
            break;
        }

        DEBUG(DEB_LEV_FULL_SEQ, "Waiting for input/output buffer in queue, input semval=%d output semval=%d in %s\n", (int)pInputSem->semval, (int)pOutputSem->semval, __func__);

        /*When we have input buffer to process then get one output buffer*/
        if(pOutputSem->semval>0 && isOutputBufferNeeded==OMX_TRUE)
        {
            tsem_down(pOutputSem);
            if(pOutputQueue->nelem>0)
            {
                outBufExchanged++;
                isOutputBufferNeeded=OMX_FALSE;
                pOutputBuffer = dequeue(pOutputQueue);
                if(pOutputBuffer == NULL){
                    DEBUG(DEB_LEV_ERR, "Had NULL output buffer!! op is=%d,iq=%d\n", (int)pOutputSem->semval, (int)pOutputQueue->nelem);
                    break;
                }
            }
        }

        if(pInputSem->semval>0 && isInputBufferNeeded==OMX_TRUE )
        {
            tsem_down(pInputSem);
            if(pInputQueue->nelem>0){
                inBufExchanged++;
                isInputBufferNeeded=OMX_FALSE;
                pInputBuffer = dequeue(pInputQueue);
                if(pInputBuffer == NULL){
                    DEBUG(DEB_LEV_ERR, "Had NULL input buffer!!\n");
                    break;
                }
            }
        }

        if(isInputBufferNeeded==OMX_FALSE)
        {
            if(pInputBuffer->hMarkTargetComponent != NULL){
                if((OMX_COMPONENTTYPE*)pInputBuffer->hMarkTargetComponent ==(OMX_COMPONENTTYPE *)openmaxStandComp) {
                    /*Clear the mark and generate an event*/
                    (*(omx_vpuenc_component_Private->callbacks->EventHandler))
                        (openmaxStandComp,
                        omx_vpuenc_component_Private->callbackData,
                        OMX_EventMark, /* The command was completed */
                        1, /* The commands was a OMX_CommandStateSet */
                        0, /* The state has been changed in message->messageParam2 */
                        pInputBuffer->pMarkData);
                } else {
                    /*If this is not the target component then pass the mark*/
                    omx_vpuenc_component_Private->pMark.hMarkTargetComponent = pInputBuffer->hMarkTargetComponent;
                    omx_vpuenc_component_Private->pMark.pMarkData            = pInputBuffer->pMarkData;
                }
                pInputBuffer->hMarkTargetComponent = NULL;
            }
        }

        if(isInputBufferNeeded==OMX_FALSE && isOutputBufferNeeded==OMX_FALSE)
        {
            if(omx_vpuenc_component_Private->pMark.hMarkTargetComponent != NULL){
                pOutputBuffer->hMarkTargetComponent = omx_vpuenc_component_Private->pMark.hMarkTargetComponent;
                pOutputBuffer->pMarkData            = omx_vpuenc_component_Private->pMark.pMarkData;
                omx_vpuenc_component_Private->pMark.hMarkTargetComponent = NULL;
                omx_vpuenc_component_Private->pMark.pMarkData            = NULL;
            }

            if(omx_vpuenc_component_Private->state == OMX_StateExecuting)  {
                if (pInputBuffer->nFilledLen > 0 || ((pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS)) {
                    if (!pVpu->last_return_ts)
                        pVpu->last_return_ts = GetNowMs();
                    omx_vpuenc_component_BufferMgmtCallback(openmaxStandComp, pInputBuffer, pOutputBuffer);
                } else {
                    /*It no buffer management call back the explicitly consume input buffer*/
                    pInputBuffer->nFilledLen = 0;
                }

            } else if(!(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
                DEBUG(DEB_LEV_ERR, "In %s Received Buffer in non-Executing State(%x)\n", __func__, (int)omx_vpuenc_component_Private->state);
            } else {
                pInputBuffer->nFilledLen = 0;
            }

            if(((pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS && pInputBuffer->nFilledLen==0) &&
                omx_vpuenc_component_Private->bIsEOSReached == OMX_FALSE) {
                    omx_vpuenc_component_Private->bIsEOSReached = OMX_TRUE;
            }

            if(omx_vpuenc_component_Private->state==OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
                /*Waiting at paused state*/
                tsem_wait(omx_vpuenc_component_Private->bStateSem);
            }

            /*If EOS and Input buffer Filled Len Zero then Return output buffer immediately*/
            if (pOutputBuffer) {
                if((pOutputBuffer->nFilledLen != 0) || ((pOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS)) {
                    if ((pOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS) {
                        omx_vpuenc_component_Private->bIsOutputEOSReached = OMX_TRUE;
                        DEBUG(DEB_LEV_FULL_SEQ, "Detected EOS flags in output buffer filled len=%d\n", (int)pOutputBuffer->nFilledLen);
                    }

                    double cur_ms = GetNowMs();
                    double elapsed_ts = cur_ms - pVpu->last_return_ts;
                    DEBUG(DEB_LEV_FULL_SEQ, "Enc instance[%d:%d] Return buf %p elapsed: %.1fms\n", pVpu->coreIdx, pVpu->handle->instIndex, pOutputBuffer->pBuffer, elapsed_ts);
                    pVpu->last_return_ts = cur_ms;
                    pOutPort->ReturnBufferFunction((omx_base_PortType *)pOutPort, pOutputBuffer);

                    if (omx_vpuenc_component_Private->bIsOutputEOSReached == OMX_TRUE) {
                        (*(omx_vpuenc_component_Private->callbacks->EventHandler))(openmaxStandComp,
                            omx_vpuenc_component_Private->callbackData,
                            OMX_EventBufferFlag,
                            1, /* port index should be output port index */
                            pOutputBuffer->nFlags,
                            NULL);
                    }

                    outBufExchanged--;
                    pOutputBuffer=NULL;
                    isOutputBufferNeeded=OMX_TRUE;
                }
            }


            if(omx_vpuenc_component_Private->state==OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
                /*Waiting at paused state*/
                tsem_wait(omx_vpuenc_component_Private->bStateSem);
            }

            /*Input Buffer has been completely consumed. So, return input buffer*/
            if((isInputBufferNeeded == OMX_FALSE) && (pInputBuffer->nFilledLen==0)) {
                if (omx_vpuenc_component_Private->bIsEOSReached == OMX_TRUE && omx_vpuenc_component_Private->bIsOutputEOSReached == OMX_FALSE) {
                    /* don't return input buffer until output buffer reach EOS */
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "need not pInPort->ReturnBufferFunction bIsEOSReached == OMX_TRUE isOutputEos == OMX_FALSE\n");
                } else {
                    pInPort->ReturnBufferFunction((omx_base_PortType *)pInPort,pInputBuffer);
                    inBufExchanged--;
                    pInputBuffer=NULL;
                    isInputBufferNeeded=OMX_TRUE;

                }
            }
        }
    }

    omx_vpuenc_component_Private->bIsBufMgThreadExit = TRUE;
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s of component %p\n", __func__, openmaxStandComp);
    return NULL;
}
