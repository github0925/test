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

#include <sys/syscall.h>
#include <stdbool.h>
#include <omxcore.h>
#include <omx_base_video_port.h>
#include <omx_vpudec_component.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include "OMX_VPU_Video.h"
#include "OMX_VPU_Index.h"

#include "vpuconfig.h"
#include "vpuapi.h"
#include "vpuapifunc.h"
#include "vpuerror.h"

#include "coda9/coda9_regdefine.h"
#include <coda9/coda9_vpuconfig.h>
#include <wave/common/common_vpuconfig.h>
#include <wave/common/common_regdefine.h>
#include <wave/wave4/wave4_regdefine.h>

#include "android_support.h"
#include "main_helper.h"

#if defined(ANDROID_INPUT_DUMP) || defined(ANDROID_OUTPUT_DUMP)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif


#ifdef ANDROID
#if ANDROID_PLATFORM_SDK_VERSION >= 19
#define NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS 1 // must set to the proper value as many as minUndequeuedBufs in mNativeWindow->query(mNativeWindow.get(),NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBufs);
#define VP9_MAX_FRAME_BUFFER_NUM             11 //  accord with framework, the acodec will add 5 extra buffers, so the max buf is (16 - 11)
#else
#define NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS 2 // must set to the proper value as many as minUndequeuedBufs in mNativeWindow->query(mNativeWindow.get(),NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBufs);
#endif
#define DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM   12
#define OMX_EXTRA_FRAME_BUFFER_NUM            3
#else
#define NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS  0
#define VP9_MAX_FRAME_BUFFER_NUM              14
#define OMX_EXTRA_FRAME_BUFFER_NUM            0
#define DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM   22
#endif

#define DEFAULT_STREAMBUFFER_SIZE       0xA00000       // case of 4K, 10M recommended
#define DEFAULT_SCREEN_WIDTH  1280
#define DEFAULT_SCREEN_HEIGHT 720
#define DEFAULT_WIDTH                   DEFAULT_SCREEN_WIDTH
#define DEFAULT_HEIGHT                  DEFAULT_SCREEN_HEIGHT
#define DEFAULT_VIDEO_INPUT_BUF_SIZE    DEFAULT_STREAMBUFFER_SIZE
#define DEFAULT_VIDEO_OUTPUT_BUF_SIZE   ((DEFAULT_WIDTH*DEFAULT_HEIGHT*3)/2)
#define DEFAULT_VIDEO_OUTPUT_FORMAT     OMX_COLOR_FormatYUV420SemiPlanar
#define DEFAULT_NATIVE_OUTPUT_FORMAT    OMX_COLOR_FormatYUV420SemiPlanar
#ifdef WORKAROUND_VP8_CTS_TEST
#define DEFAULT_VP8_VIDEO_OUTPUT_FORMAT OMX_COLOR_FormatYUV420SemiPlanar
#endif

#define MAX_FPS_XRATE                   (120 >> 16)
#define DEFAULT_BITRATE                 64000

#define DEFAULT_ACTUAL_VIDEO_OUTPUT_BUFFER_NUM  DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM
#define DEFAULT_ACTUAL_VIDEO_INPUT_BUFFER_NUM   4
#define DEFAULT_MIN_VIDEO_INPUT_BUFFER_NUM      4

#define DEFAULT_FRAMERATE               (0)     // 0 means that if input stream does not have framerate value. set it to framerate in sequence header from VPU
#define OMX_MAX_CHUNK_HEADER_SIZE           DEFAULT_STREAMBUFFER_SIZE
#define OMX_VPU_DEC_TIMEOUT                 1000
#define STREAM_END_SIZE                 0

#define SUPPORT_SCALE_TO_SCREEN_SIZE OMX_TRUE

#ifdef ANDROID
#if ANDROID_PLATFORM_SDK_VERSION >= 19
#define OMX_VIDEO_CODINGTYPE_VP8 OMX_VIDEO_CodingVP8
#else
#define OMX_VIDEO_CODINGTYPE_VP8 OMX_VIDEO_CodingVPX
#endif
#else
#define OMX_VIDEO_CODINGTYPE_VP8 OMX_VIDEO_CodingVP8
#endif

#define MINIPIPPEN_MINMUM_RESOLUTION   64
#define VPU_DEC_TIMEOUT_CNT 5
#define DEFAULT_BS_BUF_COUNT 1

#define OMX_BufferSupplyVendorDRM (OMX_BufferSupplyVendorStartUnused + 1)

#define MAKE_FOURCC(a,b,c,d) ( ((unsigned char)a) | ((unsigned char)b << 8) | ((unsigned char)c << 16) | ((unsigned char)d << 24) )
static long debug_dec_dump;

#ifdef FILE_DEC_DUMP
void OMX_DUMP_OUTPUT_YUV_TO_FILE(vpu_dec_context_t *pVpu, OMX_BUFFERHEADERTYPE* pBuffer, OMX_U32 stride, OMX_U32 height, OMX_BOOL useNativeBuffer)
{
    FILE *fp = NULL;
    int errNum = 0;
    OMX_PTR pVirAddrs = NULL;
    if (debug_dec_dump != DUMP_OUTPUT && debug_dec_dump != DUMP_BOTH)
        return;

    if(pVpu->outputDumpIdx == 0)
    {
        if ((fp = fopen(pVpu->dumpOutputFn, "w+b")))
        {
            fseek(fp, 0, SEEK_SET);
            fclose(fp);
        }
        else
        {
            errNum = errno;
            DEBUG(DEB_LEV_FULL_SEQ, "In %s,File: %s, errNum %d reason %s \n", __func__, pVpu->dumpOutputFn, errNum, strerror(errNum));
        }
    }

    if((fp = fopen(pVpu->dumpOutputFn, "a+b")))
    {
        if(useNativeBuffer)
        {
            if(lockAndroidNativeBuffer(pBuffer, stride, height, LOCK_MODE_TO_GET_VIRTUAL_ADDRESS, &pVirAddrs) == 0)
            {
                fwrite((OMX_BYTE)pVirAddrs, pBuffer->nFilledLen, 1, fp);
                pVpu->outputDumpIdx++;
                unlockAndroidNativeBuffer(pBuffer);
            }
        }
        else
        {
            fwrite(pBuffer->pBuffer, pBuffer->nFilledLen, 1, fp);
            pVpu->outputDumpIdx++;
        }
        fclose(fp);
    }
}

void OMX_DUMP_INTPUT_TO_FILE(vpu_dec_context_t *pVpu)
{
    OMX_U32 room;
    FILE *fp = NULL;
    int errNum = 0;
    OMX_U8* pbEs = NULL;
    PhysicalAddress rdPtr, wrPtr;
    if (debug_dec_dump != DUMP_INPUT && debug_dec_dump != DUMP_BOTH)
        return;

    if(pVpu->chunkReuseRequired)
    {
        return;
    }

    VPU_DecGetBitstreamBuffer(pVpu->handle, &rdPtr, &wrPtr, &room);
    if(pVpu->inputDumpIdx == 0)
    {
        if ((fp = fopen(pVpu->dumpInputFn, "w+b")))
        {
            fseek(fp, 0, SEEK_SET);
            fclose(fp);
        }
        else
        {
            errNum = errno;
            DEBUG(DEB_LEV_FULL_SEQ, "In %s,File: %s, errNum %d reason %s \n", __func__, pVpu->dumpInputFn, errNum, strerror(errNum));
        }
    }

    room = wrPtr - rdPtr;

    if((fp = fopen(pVpu->dumpInputFn, "a+b")))
    {
        pbEs = (OMX_BYTE)malloc(room);
        if(pbEs)
        {
            vdi_read_memory(pVpu->coreIdx, rdPtr, pbEs, room, pVpu->decOP.streamEndian);
            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s room size=%d\n",__func__, room);

#ifdef ANDROID_INPUT_DUMP_WITH_SIZE
            fwrite((unsigned char*)&room, 4, 1, fp);
#endif
            fwrite((unsigned char*)pbEs, room, 1, fp);
            free(pbEs);
            pVpu->inputDumpIdx++;
        }

        fclose(fp);
    }
    else
        DEBUG(DEB_LEV_ERR, "can't open a dump file(/data/dump.es)");
}
#else
#define OMX_DUMP_INTPUT_TO_FILE(pVpu)
#define OMX_DUMP_OUTPUT_YUV_TO_FILE(pVpu, pBuffer, stride, height, useNativeBuffer)
#define GENERATE_FILE_NAMES(pVpu,FILE_NAME)
#endif

static int codingTypeToMp4class(OMX_VIDEO_CODINGTYPE codingType, int fourCC);
static int codingTypeToCodStd(OMX_VIDEO_CODINGTYPE codingType);
static int BuildOmxSeqHeader(OMX_U8 *pbHeader, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_COMPONENTTYPE *openmaxStandComp, int* sizelength);
static int BuildOmxPicHeader(OMX_U8 *pbHeader, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_COMPONENTTYPE *openmaxStandComp, int sizelength);
static void initializeVideoPorts(omx_vpudec_component_PortType* inPort, omx_vpudec_component_PortType* outPort, OMX_VIDEO_CODINGTYPE codingType);
static void SetInternalVideoParameters(OMX_COMPONENTTYPE *openmaxStandComps);
static OMX_BOOL OmxVpuFlush(OMX_COMPONENTTYPE *openmaxStandComp);
static int OmxAllocateFrameBuffers(OMX_COMPONENTTYPE *openmaxStandComp);
static OMX_BOOL OmxUpdateOutputBufferHeaderToDisplayOrder(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE **ppOutputBuffer);
static OMX_TICKS OmxTimeStampCorrection(OMX_COMPONENTTYPE *openmaxStandComp, OMX_TICKS nInputTimeStamp);
static OMX_BOOL OmxClearDisplayFlag(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE *pBuffer, OMX_BOOL bFillThisBufer);
static void OmxCheckVersion(int coreIdx);
static OMX_U32 OmxGetVpuFrameRate(omx_vpudec_component_PrivateType* omx_vpudec_component_Private);
static void OmxConfigFrameBufferCount(omx_vpudec_component_PrivateType* omx_vpudec_component_Private);
#ifdef SUPPORT_NALU_FORMAT_BY_OMX
static OMX_BOOL OmxPackNaluToAU(omx_vpudec_component_PrivateType* omx_vpudec_component_Private, OMX_BUFFERHEADERTYPE* pInputBuffer);
#endif
// BUFFERHEADERTYPE queue for async decoding
static omx_bufferheader_queue_item_t* omx_bufferheader_queue_init(int count);
static void omx_bufferheader_queue_deinit(omx_bufferheader_queue_item_t* queue);
static int omx_bufferheader_queue_enqueue(omx_bufferheader_queue_item_t* queue, OMX_BUFFERHEADERTYPE* data);
static int omx_bufferheader_queue_dequeue(omx_bufferheader_queue_item_t* queue, OMX_BUFFERHEADERTYPE* data);
static int omx_bufferheader_queue_dequeue_all(omx_bufferheader_queue_item_t* queue);

static int OmxWriteBsBufFromBufHelper(OMX_COMPONENTTYPE *openmaxStandComp, vpu_dec_context_t *pVpu, vpu_buffer_t vbStream, BYTE *pChunk,  int chunkSize);
static OMX_BOOL OmxGetVpuBsBufferByVirtualAddress(OMX_COMPONENTTYPE *openmaxStandComp, vpu_buffer_t *vb, OMX_BUFFERHEADERTYPE *pInputBuffer);
static void OmxWaitUntilOutBuffersEmpty(omx_vpudec_component_PrivateType* omx_vpudec_component_Private);
static int OmxCopyVpuBufferToOmxBuffer(OMX_COMPONENTTYPE *openmaxStandComp, OMX_U8 *pBuffer, FrameBuffer fb);
static void omx_vpudec_component_vpuLibDeInit(omx_vpudec_component_PrivateType* omx_vpudec_component_Private);
static void calculate_fb_param(OMX_COMPONENTTYPE *openmaxStandComp, OMX_U32 width, OMX_U32 height);
static void updateOutSetting(omx_vpudec_component_PrivateType* omx_vpudec_component_Private);
static void initDefaultCoreSetting(omx_vpudec_component_PrivateType* omx_vpudec_component_Private);
static OMX_ERRORTYPE OmxCheckCropInfo(OMX_COMPONENTTYPE *openmaxStandComp, omx_vpudec_component_PrivateType* omx_vpudec_component_Private);
static OMX_U32 OmxCheckVUIParams(OMX_COMPONENTTYPE *openmaxStandComp, omx_vpudec_component_PrivateType* omx_vpudec_component_Private, OMX_BOOL dispOutputInfo);
static OMX_U32 OmxCheckHDRStaticMetadata(OMX_COMPONENTTYPE *openmaxStandComp, omx_vpudec_component_PrivateType* omx_vpudec_component_Private);
static dec_config_t s_dec_config;
long s_debug_level = DEB_LEV_ERR;

#ifdef GSTREAMER_LOG
GstDebugCategory * gst_omx_vpu_debug_category = NULL;
#endif

/** The Constructor of the video decoder component
* @param openmaxStandComp the component handle to be constructed
* @param cComponentName is the name of the constructed component
*/
OMX_ERRORTYPE omx_vpudec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private;
    omx_vpudec_component_PortType *inPort, *outPort;
    vpu_dec_context_t *pVpu;

    RetCode ret = RETCODE_SUCCESS;
    GetDebugLevelFromProperty(DEBUG_LOG_COMP, DEB_LEV_ERR);
    GetOmxDumpLevelFromProperty(DEBUG_DEC_DUMP, 0, &debug_dec_dump);

    if (!openmaxStandComp->pComponentPrivate)
    {
        DEBUG(DEB_LEV_FUNCTION_NAME, "In %s, allocating component\n", __func__);

        openmaxStandComp->pComponentPrivate = malloc(sizeof (omx_vpudec_component_PrivateType));
        if (openmaxStandComp->pComponentPrivate == NULL) {
            DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
            err = OMX_ErrorInsufficientResources;
            goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
        }
        memset(openmaxStandComp->pComponentPrivate,0x00,sizeof(omx_vpudec_component_PrivateType));
    }
    else
    {
        DEBUG(DEB_LEV_FUNCTION_NAME, "In %s, Error Component 0x%p Already Allocated\n", __func__, openmaxStandComp->pComponentPrivate);
    }

    omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    omx_vpudec_component_Private->ports = NULL;

    pVpu = (vpu_dec_context_t *)&omx_vpudec_component_Private->vpu;
    memset(pVpu, 0x00, sizeof (vpu_dec_context_t));

    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s, cComponentName %s\n", __func__, cComponentName);

    GENERATE_FILE_NAMES(pVpu,FILE_DEC_NAME);
    err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

    err = GetVideoCodingTypeByName(cComponentName, &omx_vpudec_component_Private->video_coding_type, OMX_FALSE);
    if (err != OMX_ErrorNone) {
        // IL client specified an invalid component name
        DEBUG(DEB_LEV_ERR, "In %s, invalid component name %s !\n", __func__, cComponentName);
        goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
    }

    initDefaultCoreSetting(omx_vpudec_component_Private);

    omx_vpudec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber = 0;
    omx_vpudec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts = 2;
    /** Allocate Ports and call port constructor. */
    if (omx_vpudec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts && !omx_vpudec_component_Private->ports)
    {
        int i;
        omx_vpudec_component_Private->ports = malloc(omx_vpudec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts*sizeof (omx_base_PortType *));
        if (!omx_vpudec_component_Private->ports) {
            DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
            err = OMX_ErrorInsufficientResources;
            goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
        }
        memset(omx_vpudec_component_Private->ports, 0x00, omx_vpudec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts*sizeof (omx_base_PortType *));

        for (i = 0; i < (int)omx_vpudec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++)
        {
            omx_vpudec_component_Private->ports[i] = malloc(sizeof (omx_vpudec_component_PortType));
            if (!omx_vpudec_component_Private->ports[i]) {
                DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
                err = OMX_ErrorInsufficientResources;
                goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
            }

            memset(omx_vpudec_component_Private->ports[i], 0x00, sizeof (omx_vpudec_component_PortType));
        }
    }

    base_video_port_Constructor(openmaxStandComp, &omx_vpudec_component_Private->ports[0], 0, OMX_TRUE);
    base_video_port_Constructor(openmaxStandComp, &omx_vpudec_component_Private->ports[1], 1, OMX_FALSE);

    omx_vpudec_component_Private->ports[0]->Port_SendBufferFunction = &omx_vpudec_component_SendBufferFunction;

    omx_vpudec_component_Private->ports[1]->Port_SendBufferFunction = &omx_vpudec_component_SendBufferFunction;
    omx_vpudec_component_Private->ports[1]->ReturnBufferFunction = &omx_vpudec_component_OutPort_ReturnBufferFunction;


    /** here we can override whatever defaults the base_component constructor set
    * e.g. we can override the function pointers in the private struct
    */
    /** Domain specific section for the ports.
    * first we set the parameter common to both formats
    */

    //common parameters related to input port
    inPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    //common parameters related to output port
    outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    initializeVideoPorts(inPort, outPort, omx_vpudec_component_Private->video_coding_type);
    SetInternalVideoParameters(openmaxStandComp);

    pVpu->productId = VPU_GetProductId(pVpu->coreIdx);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "constructor productId %d, coreIdx %d\n", pVpu->productId, pVpu->coreIdx);
    omx_vpudec_component_Private->seqHeader = NULL;
    omx_vpudec_component_Private->seqHeaderSize = 0;
    omx_vpudec_component_Private->picHeader = NULL;
    omx_vpudec_component_Private->picHeaderSize = 0;

    omx_vpudec_component_Private->BufferMgmtCallback = NULL; //omx_vpudec_component_BufferMgmtCallback;
    omx_vpudec_component_Private->BufferMgmtFunction = omx_vpudec_component_BufferMgmtFunction;

    /** initializing the codec context etc that was done earlier by libinit function */
    omx_vpudec_component_Private->messageHandler = omx_vpudec_component_MessageHandler;
    omx_vpudec_component_Private->destructor = omx_vpudec_component_Destructor;
    openmaxStandComp->SetParameter = omx_vpudec_component_SetParameter;
    openmaxStandComp->GetParameter = omx_vpudec_component_GetParameter;
    openmaxStandComp->ComponentRoleEnum = omx_vpudec_component_ComponentRoleEnum;
    openmaxStandComp->GetExtensionIndex = omx_vpudec_component_GetExtensionIndex;
    openmaxStandComp->SetConfig = omx_vpudec_component_SetConfig;
    openmaxStandComp->GetConfig = omx_vpudec_component_GetConfig;
    openmaxStandComp->AllocateBuffer = omx_videodec_component_AllocateBuffer;
    openmaxStandComp->UseBuffer = omx_videodec_component_UseBuffer;
    openmaxStandComp->FreeBuffer = omx_videodec_component_FreeBuffer;


    omx_vpudec_component_Private->seqHeader = malloc(OMX_MAX_CHUNK_HEADER_SIZE);
    if (!omx_vpudec_component_Private->seqHeader)
    {
        DEBUG(DEB_LEV_ERR, "fail to allocate the seqHeader buffer\n" );
        err = OMX_ErrorInsufficientResources;
        goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
    }

    omx_vpudec_component_Private->picHeader = malloc(OMX_MAX_CHUNK_HEADER_SIZE);
    if (!omx_vpudec_component_Private->picHeader)
    {
        DEBUG(DEB_LEV_ERR, "fail to allocate the picheader buffer\n" );
        err = OMX_ErrorInsufficientResources;
        goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
    }

    memset(&omx_vpudec_component_Private->port_setting_change_tsem,0x00,sizeof(tsem_t));
    tsem_init(&omx_vpudec_component_Private->port_setting_change_tsem, 0);

    memset(&omx_vpudec_component_Private->disp_Buf_full_tsem,0x00,sizeof(tsem_t));
    tsem_init(&omx_vpudec_component_Private->disp_Buf_full_tsem, 0);

    omx_vpudec_component_Private->vpuReady = OMX_FALSE;
    omx_vpudec_component_Private->portSettingChangeRequest = OMX_FALSE;
    omx_vpudec_component_Private->in_bufferheader_queue = omx_bufferheader_queue_init(MAX_REG_FRAME);
    pthread_mutex_init(&omx_vpudec_component_Private->display_flag_mutex, NULL);
    memset(&omx_vpudec_component_Private->omx_display_flags[0], 0x00, sizeof(omx_display_flag)*MAX_REG_FRAME);
    memset(&omx_vpudec_component_Private->omx_ts_correction, 0x00, sizeof(omx_timestamp_correction_t));

    omx_vpudec_component_Private->bIsTimeStampReorder = OMX_TRUE;
    pthread_mutex_init(&omx_vpudec_component_Private->vpu_flush_mutex, NULL);

    DEBUG(DEB_LEV_SIMPLE_SEQ, "begin to init vpu %d\n", pVpu->coreIdx);

    {
        // this code block comes from vpurun.c
        char*   path;
        Uint32  sizeInWord;
        Uint16 *pusBitCode;
        if (pVpu->coreIdx == 0) {
            switch(pVpu->productId) {
            case PRODUCT_ID_980:  path = CORE_1_BIT_CODE_FILE_PATH; break;
            case PRODUCT_ID_4102: path = CORE_3_BIT_CODE_FILE_PATH; break;
            case PRODUCT_ID_420:  path = CORE_0_BIT_CODE_FILE_PATH; break;
            case PRODUCT_ID_412:  path = CORE_2_BIT_CODE_FILE_PATH; break;
            case PRODUCT_ID_420L: path = CORE_5_BIT_CODE_FILE_PATH; break;
            case PRODUCT_ID_510:  path = CORE_6_BIT_CODE_FILE_PATH; break;
            default:
                DEBUG(DEB_LEV_ERR, "Unknown product id: %d\n", pVpu->productId);
                err = OMX_ErrorHardware;
                goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
            }
        }
        else if (pVpu->coreIdx == 1)  path = CORE_2_BIT_CODE_FILE_PATH;
        else
        {
            DEBUG(DEB_LEV_ERR, "Invalid core index: %d\n", pVpu->coreIdx);
            err = OMX_ErrorHardware;
            goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
        }

        char bitfile[32];
        sprintf(bitfile, "%s%s", FW_PATH, path);
        DEBUG(DEB_LEV_SIMPLE_SEQ, "VPU_GetProductId %x, core %d, bitfile %s\n", pVpu->productId, pVpu->coreIdx, bitfile);
        if (LoadFirmware(pVpu->productId, (Uint8**)&pusBitCode, &sizeInWord, bitfile) < 0) {
            DEBUG(DEB_LEV_ERR, "failed open bit_firmware file path is %s\n", bitfile);
            err = OMX_ErrorHardware;
            goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
        }

        DEBUG(DEB_LEV_SIMPLE_SEQ, "try to VPU_InitWithBitcode\n");
        ret = VPU_InitWithBitcode(pVpu->coreIdx, pusBitCode, sizeInWord);

        if (pusBitCode)
            osal_free(pusBitCode);
    }

    if (ret != RETCODE_SUCCESS && ret != RETCODE_CALLED_BEFORE)
    {
        DEBUG(DEB_LEV_ERR, "VPU_Init failed Error code is 0x%x \n", (int)ret);
        err = OMX_ErrorHardware;
        goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
    }

    err = RM_RegisterComponent(cComponentName, MAX_DECODER_COMPONENTS);
    if (err != OMX_ErrorNone) {
        DEBUG(DEB_LEV_ERR, "In %s, RM_RegisterComponent failed Error code is 0x%x\n", __func__, err);
        goto ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR;
    }
    DEBUG(DEB_LEV_SIMPLE_SEQ, "VPU_Init success\n");

    return OMX_ErrorNone;

ERROR_OMX_VPU_DEC_COMPONENT_CONSTRUCTOR:
    omx_vpudec_component_Destructor(openmaxStandComp);
    return err;
}

/** The destructor of the video decoder component
*/
OMX_ERRORTYPE omx_vpudec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    OMX_U32 i;
    vpu_dec_context_t *pVpu= (vpu_dec_context_t *)&omx_vpudec_component_Private->vpu;

    DEBUG(DEB_LEV_FULL_SEQ, "In %s,\n", __func__);
    if (!omx_vpudec_component_Private->ports)
    {
        DEBUG(DEB_LEV_SIMPLE_SEQ, "Destructor of video decoder component is already called\n");
        return OMX_ErrorNone;
    }

    if (omx_vpudec_component_Private->vpuReady)
    {
        omx_vpudec_component_vpuLibDeInit(omx_vpudec_component_Private);
        omx_vpudec_component_Private->vpuReady = OMX_FALSE;
    }

    if (omx_vpudec_component_Private->seqHeader)
    {
        free(omx_vpudec_component_Private->seqHeader);
        omx_vpudec_component_Private->seqHeader = NULL;
        omx_vpudec_component_Private->seqHeaderSize = 0;
    }

    if (omx_vpudec_component_Private->picHeader)
    {
        free(omx_vpudec_component_Private->picHeader);
        omx_vpudec_component_Private->picHeader = NULL;
        omx_vpudec_component_Private->picHeaderSize = 0;
    }

    if (omx_vpudec_component_Private->in_bufferheader_queue)
    {
        omx_bufferheader_queue_dequeue_all(omx_vpudec_component_Private->in_bufferheader_queue);
        omx_bufferheader_queue_deinit(omx_vpudec_component_Private->in_bufferheader_queue);
        omx_vpudec_component_Private->in_bufferheader_queue = NULL;
    }

    VPU_DeInit(pVpu->coreIdx);
    tsem_deinit(&omx_vpudec_component_Private->port_setting_change_tsem);
    tsem_deinit(&omx_vpudec_component_Private->disp_Buf_full_tsem);
    pthread_mutex_destroy(&omx_vpudec_component_Private->display_flag_mutex);
    pthread_mutex_destroy(&omx_vpudec_component_Private->vpu_flush_mutex);

    DEBUG(DEB_LEV_FULL_SEQ, "Destructor of video decoder component is called\n");

    /*
       kick the message handler thread, the thread may still hang on the AllocSem.
       make sure the message handler thread is activated, so the omx_base_component_Destructor
       can get out from the pthread_join
    */
    if (omx_vpudec_component_Private->ports)
    {
        for (i = 0; i < omx_vpudec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++)
        {
            if (omx_vpudec_component_Private->ports[i])
            {
                tsem_up(omx_vpudec_component_Private->ports[i]->pAllocSem);
            }
        }
    }

    omx_base_filter_Destructor(openmaxStandComp);

    /* frees port/s */
    if (omx_vpudec_component_Private->ports)
    {
        for (i = 0; i < omx_vpudec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++)
        {
            if (omx_vpudec_component_Private->ports[i])
            {
                omx_vpudec_component_Private->ports[i]->PortDestructor(omx_vpudec_component_Private->ports[i]);
                omx_vpudec_component_Private->ports[i] = NULL;
            }
        }
        free(omx_vpudec_component_Private->ports);
        omx_vpudec_component_Private->ports = NULL;
    }

    DEBUG(DEB_LEV_FULL_SEQ, "Out %s,\n", __func__);
    return OMX_ErrorNone;
}

/** It initializates the framework, and opens an videodecoder of type specified by IL client
*/
OMX_ERRORTYPE omx_vpudec_component_vpuLibInit(omx_vpudec_component_PrivateType* omx_vpudec_component_Private)
{
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    RetCode ret = RETCODE_SUCCESS;
    omx_vpudec_component_PortType *inPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    OMX_ERRORTYPE err = OMX_ErrorHardware;
    int i=0;
    unsigned long maxAddr;

    DEBUG(DEB_LEV_FULL_SEQ, "In %s, coreIdx %d\n", __func__, pVpu->coreIdx);

    if (codingTypeToCodStd(omx_vpudec_component_Private->video_coding_type) == -1)
    {
        DEBUG(DEB_LEV_ERR, "\nthe requested codec type not support video_coding_type=%d\n", (int)omx_vpudec_component_Private->video_coding_type);
        return OMX_ErrorComponentNotFound;
    }

    pVpu->dispOutIdx = 0;
    pVpu->frameIdx = 0;
    pVpu->decodefinish = 0;
    pVpu->int_reason = 0;
    pVpu->totalNumofErrMbs = 0;
    pVpu->bsBufferCount = DEFAULT_BS_BUF_COUNT;
    pVpu->bsQueueIndex = 0;
    pVpu->tryCount = 0;

    OmxCheckVersion(pVpu->coreIdx);
    memset(&(pVpu->decOP),0, sizeof(DecOpenParam));

    updateOutSetting(omx_vpudec_component_Private);

    pVpu->decOP.bitstreamFormat = (CodStd)codingTypeToCodStd(omx_vpudec_component_Private->video_coding_type);
    if ((-1) == (int)pVpu->decOP.bitstreamFormat)
    {
        DEBUG(DEB_LEV_ERR, "can not found Codec codingType is 0x%x \n", (int)omx_vpudec_component_Private->video_coding_type);
        goto ERR_VPU_DEC_INIT;
    }
    DEBUG(DEFAULT_MESSAGES, "codingTypeToCodStd video_coding_type=%d, decOP.bitstreamFormat =%d\n", (int)omx_vpudec_component_Private->video_coding_type, (int)pVpu->decOP.bitstreamFormat );
    pVpu->decOP.mp4Class = codingTypeToMp4class(omx_vpudec_component_Private->video_coding_type, 0 /*fourcc*/);

    if (inPort->bAllocateBuffer == OMX_TRUE && omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer == OMX_TRUE)
    {
        pVpu->decOP.bitstreamBuffer = pVpu->vbStream[0].phys_addr;
        maxAddr = pVpu->vbStream[0].phys_addr;
        for (i=0; i<MAX_DEC_BITSTREAM_BUFFER_COUNT; i++)
        {
            if (pVpu->vbStream[i].size > 0)
            {
                if (pVpu->vbStream[i].phys_addr < pVpu->decOP.bitstreamBuffer)
                    pVpu->decOP.bitstreamBuffer = pVpu->vbStream[i].phys_addr;

                if (pVpu->vbStream[i].phys_addr > maxAddr)
                    maxAddr = pVpu->vbStream[i].phys_addr;
            }
        }

        pVpu->decOP.bitstreamBufferSize = maxAddr - pVpu->decOP.bitstreamBuffer + pVpu->vbStream[0].size;

        DEBUG(DEB_LEV_SIMPLE_SEQ, "set bitstream buffer size = %d, addr=0x%x\n", pVpu->decOP.bitstreamBufferSize, (int)pVpu->decOP.bitstreamBuffer );
    }
    else
    {
        // wave BS_MODE_PIC_END need one more buffer
        if (pVpu->coreIdx == VPU_WAVE412_IDX) {
            pVpu->bsBufferCount = DEFAULT_BS_BUF_COUNT + 1;
        }

        for (i = 0; i < pVpu->bsBufferCount; i++) {
            pVpu->vbStream[i].size = DEFAULT_STREAMBUFFER_SIZE;
            if (vdi_allocate_dma_memory(pVpu->coreIdx, &pVpu->vbStream[i]) < 0)
            {
                DEBUG(DEB_LEV_ERR, "fail to allocate bitstream buffer\n" );
                goto ERR_VPU_DEC_INIT;
            }
            DEBUG(DEB_LEV_SIMPLE_SEQ, "allocate bitstream buffer 0x%llx, size %d\n", (long long)pVpu->vbStream[i].phys_addr, pVpu->vbStream[i].size);
        }

        pVpu->decOP.bitstreamBuffer = pVpu->vbStream[0].phys_addr;
        pVpu->decOP.bitstreamBufferSize = pVpu->vbStream[0].size; //bitstream buffer size must be aligned 1024
    }

    pVpu->decOP.mp4DeblkEnable = 0;

    if(pVpu->productId == PRODUCT_ID_410 || pVpu->productId == PRODUCT_ID_4102 || pVpu->productId == PRODUCT_ID_420 || pVpu->productId == PRODUCT_ID_412 || pVpu->productId == PRODUCT_ID_510)
        pVpu->decOP.tiled2LinearMode = FF_NONE;
    else
        pVpu->decOP.tiled2LinearMode = FF_FRAME; // coda980 only

    DEBUG(DEB_LEV_SIMPLE_SEQ, "setting video output format eColorFormat=0x%x, useNativeBuffer=%d, bThumbnailMode=%d\n",
        (int)outPort->sPortParam.format.video.eColorFormat, (int)omx_vpudec_component_Private->useNativeBuffer, (int)omx_vpudec_component_Private->bThumbnailMode);

    if(outPort->sPortParam.format.video.eColorFormat == MAKE_FOURCC('N', 'V', '1', '2') ||
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatYUV420SemiPlanar ||
#ifdef ANDROID
        outPort->sPortParam.format.video.eColorFormat == HAL_PIXEL_FORMAT_NV12 ||
#endif
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatYUV420PackedSemiPlanar)
    {
        DEBUG(DEB_LEV_SIMPLE_SEQ, "normal YUV420SemiPlanar/NV12\n");
        pVpu->decOP.cbcrInterleave = 1;
        pVpu->decOP.cbcrOrder = CBCR_ORDER_NORMAL;
    }
    else if(outPort->sPortParam.format.video.eColorFormat == MAKE_FOURCC('Y', 'V', '1', '2') ||
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatYUV420Planar ||
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatYUV420PackedPlanar)
    {
        DEBUG(DEB_LEV_SIMPLE_SEQ, "normal YUV420Planar/YV12");
        pVpu->decOP.cbcrInterleave = 0;
#ifdef ANDROID
        if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
            pVpu->decOP.cbcrOrder = CBCR_ORDER_REVERSED;
        else
            pVpu->decOP.cbcrOrder = CBCR_ORDER_NORMAL;
#else
        pVpu->decOP.cbcrOrder = CBCR_ORDER_NORMAL;
#endif
    }
    else if(outPort->sPortParam.format.video.eColorFormat == MAKE_FOURCC('I', '4', '2', '2') ||
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatYUV422Planar ||
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatYUV422PackedPlanar)
    {
        pVpu->decOP.cbcrInterleave = 0;
#ifdef ANDROID
        if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
            pVpu->decOP.cbcrOrder = CBCR_ORDER_REVERSED;
        else
            pVpu->decOP.cbcrOrder = CBCR_ORDER_NORMAL;
#else
        pVpu->decOP.cbcrOrder = CBCR_ORDER_NORMAL;
#endif
    }
    else if(outPort->sPortParam.format.video.eColorFormat == MAKE_FOURCC('N', 'V', '1', '6') ||
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatYUV422SemiPlanar ||
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatYUV422PackedSemiPlanar)
    {
        pVpu->decOP.cbcrInterleave = 1;
        pVpu->decOP.cbcrOrder = CBCR_ORDER_NORMAL;
    }
    else if(outPort->sPortParam.format.video.eColorFormat == MAKE_FOURCC('Y', 'U', 'Y', 'V') ||
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatYCbYCr )
    {
        pVpu->decOP.cbcrInterleave = 0;
        pVpu->decOP.cbcrOrder = CBCR_ORDER_NORMAL;
    }
    else if(outPort->sPortParam.format.video.eColorFormat == MAKE_FOURCC('U', 'Y', 'V', 'Y') ||
        outPort->sPortParam.format.video.eColorFormat == OMX_COLOR_FormatCbYCrY )
    {
        pVpu->decOP.cbcrInterleave = 0;
        pVpu->decOP.cbcrOrder = CBCR_ORDER_NORMAL;
    }
    else if (outPort->sPortParam.format.video.eColorFormat == OMX_SEMI_COLOR_FormatIFBC32x8Tiled)
    {
        if (pVpu->coreIdx == VPU_CODA988_IDX) {
            DEBUG(DEB_LEV_ERR, "coda: not support ifbc\n" );
            goto ERR_VPU_DEC_INIT;
        }

        DEBUG(DEB_LEV_SIMPLE_SEQ, "wave: 32x8tile ifbc\n");
        pVpu->scalerInfo.enScaler = TRUE;
        pVpu->decOP.pvricFbcEnable = TRUE;
        pVpu->decOP.cbcrInterleave = 1;
        pVpu->decOP.cbcrOrder = CBCR_ORDER_NORMAL;
    }
    else
    {
        DEBUG(DEB_LEV_ERR, "not supported video output format\n" );
        goto ERR_VPU_DEC_INIT;
    }
    pVpu->decOP.bwbEnable = VPU_ENABLE_BWB;
    pVpu->decOP.frameEndian  = VDI_LITTLE_ENDIAN;
    pVpu->decOP.streamEndian = VDI_LITTLE_ENDIAN;
    pVpu->decOP.bitstreamMode = BS_MODE_PIC_END;
    DEBUG(DEB_LEV_SIMPLE_SEQ, "frameEndian %d, streamEndian %d\n", VPU_FRAME_ENDIAN, VPU_STREAM_ENDIAN);
    pVpu->decOP.wtlEnable           = s_dec_config.wtlEnable;
    pVpu->decOP.wtlMode             = s_dec_config.wtlMode;
    if (pVpu->coreIdx == VPU_WAVE412_IDX) {
        pVpu->decOP.frameEndian  = VDI_128BIT_LITTLE_ENDIAN;
        pVpu->decOP.streamEndian = VDI_128BIT_LITTLE_ENDIAN;
        pVpu->decOP.fbc_mode = 0x0C; //normal: 0x0C - Best prediction: 0x00 - Basic prediction: 0x3C
        pVpu->decOP.bwOptimization = TRUE;
        pVpu->decOP.wtlEnable = TRUE;
    }

    pVpu->fbAllocInfo.mapType = pVpu->mapType;
    // pVpu->fbAllocInfo.cbcrInterleave = pVpu->decOP.cbcrInterleave;
    // pVpu->fbAllocInfo.nv21 = pVpu->decOP.nv21;
    // pVpu->fbAllocInfo.format = pVpu->fbFormat;
    // pVpu->fbAllocInfo.stride = outPort->sPortParam.format.video.nFrameWidth;
    // pVpu->fbAllocInfo.height = outPort->sPortParam.format.video.nFrameHeight;
    // pVpu->fbAllocInfo.size = outPort->sPortParam.nBufferSize;
    // pVpu->fbAllocInfo.lumaBitDepth = DEFAULT_COLOR_DEPTH;
    // pVpu->fbAllocInfo.chromaBitDepth = DEFAULT_COLOR_DEPTH;
    // pVpu->fbAllocInfo.endian = pVpu->decOP.frameEndian;
    // pVpu->fbAllocInfo.num = outPort->sPortParam.nBufferCountActual;
    // pVpu->fbAllocInfo.type = FB_TYPE_CODEC;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, VPU_DecOpen bitstreamFormat=%d, bitstreamBuffer=0x%x, bitstreamBufferSize=%d, mp4Class=%d, cbcrInterleave=%d, cbcrOrder=%d, bitstreamMode=%d\n", __func__,
        (int)pVpu->decOP.bitstreamFormat, (int)pVpu->decOP.bitstreamBuffer,  (int)pVpu->decOP.bitstreamBufferSize,  (int)pVpu->decOP.mp4Class,  (int)pVpu->decOP.cbcrInterleave,  (int)pVpu->decOP.cbcrOrder,  (int)pVpu->decOP.bitstreamMode);
#ifdef SUPPORT_SCALER_STRIDE
    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, VPU_DecOpen enScaler=%d, imageFormat=0x%x, scaleWidth=%d, scaleHeight=%d, scaleStride=%d\n", __func__,
        (int)pVpu->scalerInfo.enScaler, (int)pVpu->scalerInfo.imageFormat,  (int)pVpu->scalerInfo.scaleWidth,  (int)pVpu->scalerInfo.scaleHeight, (int)pVpu->scalerInfo.scaleStride);
#else
    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, VPU_DecOpen enScaler=%d, scaleWidth=%d, scaleHeight=%d\n", __func__,
        (int)pVpu->scalerInfo.enScaler, (int)pVpu->scalerInfo.scaleWidth,  (int)pVpu->scalerInfo.scaleHeight);
#endif
    pVpu->decOP.coreIdx = pVpu->coreIdx;
    VLOG(INFO, "------------------------------ DECODER OPTIONS ------------------------------\n");
    VLOG(INFO, "[coreIdx            ]: %d\n", pVpu->coreIdx                  );
    VLOG(INFO, "[mapType            ]: %d\n", pVpu->mapType                  );
    VLOG(INFO, "[tiled2linear       ]: %d\n", pVpu->decOP.tiled2LinearEnable );
    VLOG(INFO, "[wtlEnable          ]: %d\n", pVpu->decOP.wtlEnable          );
    VLOG(INFO, "[wtlMode            ]: %d\n", pVpu->decOP.wtlMode            );
    VLOG(INFO, "[bitstreamBuffer    ]: 0x%08x\n", pVpu->decOP.bitstreamBuffer);
    VLOG(INFO, "[bitstreamBufferSize]: %d\n", pVpu->decOP.bitstreamBufferSize);
    VLOG(INFO, "[bitstreamMode      ]: %d\n", pVpu->decOP.bitstreamMode      );
    VLOG(INFO, "[cbcrInterleave     ]: %d\n", pVpu->decOP.cbcrInterleave     );
    VLOG(INFO, "[nv21               ]: %d\n", pVpu->decOP.nv21               );
    VLOG(INFO, "[streamEndian       ]: %d\n", pVpu->decOP.streamEndian       );
    VLOG(INFO, "[frameEndian        ]: %d\n", pVpu->decOP.frameEndian        );
    VLOG(INFO, "[BWB                ]: %d\n", pVpu->decOP.bwbEnable          );
    VLOG(INFO, "[productid          ]: %d\n", pVpu->productId                );
    VLOG(INFO, "[bitstreamFormat    ]: %d\n", pVpu->decOP.bitstreamFormat    );
    VLOG(INFO, "[avcExtension       ]: %d\n", pVpu->decOP.avcExtension       );
    VLOG(INFO, "[tiled2LinearEnable ]: %d\n", pVpu->decOP.tiled2LinearEnable );
    VLOG(INFO, "[tiled2LinearMode   ]: %d\n", pVpu->decOP.tiled2LinearMode   );
    VLOG(INFO, "[mp4DeblkEnable     ]: %d\n", pVpu->decOP.mp4DeblkEnable     );
    VLOG(INFO, "[mp4Class           ]: %d\n", pVpu->decOP.mp4Class           );
    VLOG(INFO, "[FBC                ]: %x\n", pVpu->decOP.fbc_mode           );
    VLOG(INFO, "[BWOPT              ]: %d\n", pVpu->decOP.bwOptimization     );
    VLOG(INFO, "[PVRIC              ]: %d\n", pVpu->decOP.pvricFbcEnable     );
    VLOG(INFO, "[PVRIC31HW          ]: %d\n", pVpu->decOP.pvric31HwSupport   );
    VLOG(WARN, "-----------------------------------------------------------------------------\n");

    ret = VPU_DecOpen(&pVpu->handle, &pVpu->decOP);
    if (ret != RETCODE_SUCCESS)
    {
        DEBUG(DEB_LEV_ERR, "VPU_DecOpen failed Error code is 0x%x \n", (int)ret);
        goto ERR_VPU_DEC_INIT;
    }

    pVpu->instIdx = VPU_GetOpenInstanceNum(pVpu->coreIdx);
    if (pVpu->instIdx > MAX_NUM_INSTANCE)
    {
        DEBUG(DEB_LEV_ERR, "exceed the opened instance num than %d num\n", MAX_NUM_INSTANCE);
        err = OMX_ErrorNoMore;
        goto ERR_VPU_DEC_INIT;
    }

    DEBUG(DEB_LEV_SIMPLE_SEQ, "VPU_DecOpen success vpu->instIdx : %d", pVpu->instIdx);

    // VPU_DecGiveCommand(pVpu->handle, ENABLE_LOGGING, 0);

#ifdef USE_IFRAME_SEARCH_FOR_1STFRAME
    if (pVpu->decOP.bitstreamFormat != STD_VP9)     // skipmode for VP9 under developing at the moment
        pVpu->decParam.skipframeMode = 1;
#endif

    DEBUG(DEB_LEV_FULL_SEQ, "In %s vpu library/codec initialize done\n",__func__);
    return OMX_ErrorNone;

ERR_VPU_DEC_INIT:
    DEBUG(DEB_LEV_ERR, "In %s vpu library/codec initialize err =0 x%x\n",__func__, (int)err);
    return err;
}


void omx_vpudec_component_vpuLibDeInit(omx_vpudec_component_PrivateType* omx_vpudec_component_Private)
{
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *inPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    RetCode ret = RETCODE_SUCCESS;
    DecOutputInfo decOutputInfo;
    int i = 0;
    DEBUG(DEB_LEV_FULL_SEQ, "In %s \n", __func__);

    ret = VPU_DecClose(pVpu->handle);
    if (ret == RETCODE_FRAME_NOT_COMPLETE)
    {
        DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s VPU_DecClose need to call VPU_DecGetOutputInfo first \n", __func__);
        VPU_DecGetOutputInfo(pVpu->handle, &decOutputInfo);
        VPU_DecClose(pVpu->handle);
    }

    if (inPort && inPort->bAllocateBuffer == OMX_TRUE)
    {
        if (omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer == OMX_FALSE)
        {
            for (i = 0; i < pVpu->bsBufferCount; i++)
            {
                if (pVpu->vbStream[i].size)
                {
                    DEBUG(DEB_LEV_FULL_SEQ, "free pVpu->vbStream[%d].phy=0x%llx", i, (long long)pVpu->vbStream[i].phys_addr);
                    vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbStream[i]);
                    memset(&pVpu->vbStream[i], 0, sizeof(vpu_buffer_t));
                }
            }
        }
    }
    else
    {
        for (i = 0; i < pVpu->bsBufferCount; i++)
        {
            if (pVpu->vbStream[i].size)
            {
                DEBUG(DEB_LEV_FULL_SEQ, "free pVpu->vbStream[%d].phy=0x%llx", i, (long long)pVpu->vbStream[i].phys_addr);
                vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbStream[i]);
                memset(&pVpu->vbStream[i], 0, sizeof(vpu_buffer_t));
            }
        }
    }


    if (outPort && outPort->bAllocateBuffer == OMX_FALSE)
    {
        if (omx_vpudec_component_Private->useNativeBuffer == OMX_FALSE)
        {
            for (i=0; i<MAX_REG_FRAME; i++)
            {
                if (pVpu->vbAllocFb[i].size > 0)
                {
                    vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbAllocFb[i]);
                    memset(&pVpu->vbAllocFb[i], 0, sizeof(vpu_buffer_t));
                }
            }
        }
    }

    for (i=0; i<MAX_REG_FRAME; i++) // free framebufer for DPB
    {
        if (pVpu->vbDPB[i].size > 0)
        {
            vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbDPB[i]);
            memset(&pVpu->vbDPB[i], 0, sizeof(vpu_buffer_t));
        }
    }

#ifdef SUPPORT_PARALLEL
    if (pVpu->vbMvCol.size) {
        vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbMvCol);
        memset(&pVpu->vbMvCol, 0, sizeof(vpu_buffer_t));
    }
#endif

    DEBUG(DEB_LEV_FULL_SEQ, "Out %s \n", __func__);
}

/** internal function to set codec related parameters in the private type structure
*/
void SetInternalVideoParameters(OMX_COMPONENTTYPE *openmaxStandComp)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = (omx_vpudec_component_PrivateType* )openmaxStandComp->pComponentPrivate;
    omx_vpudec_component_PortType *inPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];

    DEBUG(DEB_LEV_SIMPLE_SEQ, "SetInternalVideoParameters codingType=%d, inPort->bAllocateBuffer=%d\n", (unsigned int)omx_vpudec_component_Private->video_coding_type, (int)inPort->bAllocateBuffer);
    inPort->sPortParam.format.video.eCompressionFormat = omx_vpudec_component_Private->video_coding_type;
    inPort->sVideoParam.eCompressionFormat = omx_vpudec_component_Private->video_coding_type;

    omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer = OMX_FALSE;
}

void initializeVideoPorts(omx_vpudec_component_PortType* inPort, omx_vpudec_component_PortType* outPort, OMX_VIDEO_CODINGTYPE codingType)
{
    /* set default parameters to input port */
    inPort->sPortParam.eDomain = OMX_PortDomainVideo;
    inPort->sPortParam.nBufferSize = DEFAULT_VIDEO_INPUT_BUF_SIZE;
    inPort->sPortParam.format.video.nBitrate = DEFAULT_BITRATE;
    inPort->sPortParam.format.video.xFramerate = DEFAULT_FRAMERATE;
    inPort->sPortParam.format.video.nFrameWidth = DEFAULT_WIDTH;
    inPort->sPortParam.format.video.nFrameHeight = DEFAULT_HEIGHT;
    inPort->sPortParam.format.video.nStride = DEFAULT_WIDTH;
    inPort->sPortParam.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    inPort->sPortParam.format.video.eCompressionFormat = codingType;
    inPort->sPortParam.nBufferCountActual = DEFAULT_ACTUAL_VIDEO_INPUT_BUFFER_NUM;
    inPort->sPortParam.nBufferCountMin = DEFAULT_MIN_VIDEO_INPUT_BUFFER_NUM;
    inPort->sVideoParam.eColorFormat = OMX_COLOR_FormatUnused;
    inPort->sVideoParam.eCompressionFormat = codingType;
    inPort->sVideoParam.xFramerate = DEFAULT_FRAMERATE;

    inPort->nTempBufferCountActual = inPort->sPortParam.nBufferCountActual;

    setHeader(&inPort->omxConfigCrop, sizeof(OMX_CONFIG_RECTTYPE));
    inPort->omxConfigCrop.nPortIndex = OMX_BASE_FILTER_INPUTPORT_INDEX;
    inPort->omxConfigCrop.nLeft = inPort->omxConfigCrop.nTop = 0;
    inPort->omxConfigCrop.nWidth = DEFAULT_WIDTH;
    inPort->omxConfigCrop.nHeight = DEFAULT_HEIGHT;

    setHeader(&inPort->omxConfigRotate, sizeof(OMX_CONFIG_ROTATIONTYPE));
    inPort->omxConfigRotate.nPortIndex = OMX_BASE_FILTER_INPUTPORT_INDEX;
    inPort->omxConfigRotate.nRotation = 0;

    setHeader(&inPort->omxConfigMirror, sizeof(OMX_CONFIG_MIRRORTYPE));
    inPort->omxConfigMirror.nPortIndex = OMX_BASE_FILTER_INPUTPORT_INDEX;
    inPort->omxConfigMirror.eMirror = OMX_MirrorNone;
    setHeader(&inPort->omxConfigScale, sizeof(OMX_CONFIG_SCALEFACTORTYPE));
    inPort->omxConfigScale.nPortIndex = OMX_BASE_FILTER_INPUTPORT_INDEX;
    inPort->omxConfigScale.xWidth = inPort->omxConfigScale.xHeight = 0x10000;
    setHeader(&inPort->omxConfigOutputPosition, sizeof(OMX_CONFIG_POINTTYPE));
    inPort->omxConfigOutputPosition.nPortIndex = OMX_BASE_FILTER_INPUTPORT_INDEX;
    inPort->omxConfigOutputPosition.nX = inPort->omxConfigOutputPosition.nY = 0;

    /* set default parameters to output port */
    outPort->sPortParam.nBufferSize = DEFAULT_VIDEO_OUTPUT_BUF_SIZE;
    outPort->sPortParam.format.video.eColorFormat = DEFAULT_VIDEO_OUTPUT_FORMAT;
    outPort->sPortParam.format.video.xFramerate = DEFAULT_FRAMERATE;
    outPort->sPortParam.nBufferCountActual =
        codingType == OMX_VIDEO_CodingVP9 ? VP9_MAX_FRAME_BUFFER_NUM : DEFAULT_ACTUAL_VIDEO_OUTPUT_BUFFER_NUM;
    outPort->sPortParam.nBufferCountMin =
        codingType == OMX_VIDEO_CodingVP9 ? VP9_MAX_FRAME_BUFFER_NUM : DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM;
    outPort->sPortParam.format.video.nFrameWidth = DEFAULT_WIDTH;
    outPort->sPortParam.format.video.nFrameHeight = DEFAULT_HEIGHT;
    outPort->sVideoParam.eColorFormat = DEFAULT_VIDEO_OUTPUT_FORMAT;
    outPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingUnused;
    outPort->sVideoParam.xFramerate = DEFAULT_FRAMERATE;

    outPort->nTempBufferCountActual = outPort->sPortParam.nBufferCountActual;

    setHeader(&outPort->omxConfigCrop, sizeof(OMX_CONFIG_RECTTYPE));
    outPort->omxConfigCrop.nPortIndex = OMX_BASE_FILTER_OUTPUTPORT_INDEX;
    outPort->omxConfigCrop.nLeft = outPort->omxConfigCrop.nTop = 0;
    outPort->omxConfigCrop.nWidth = DEFAULT_WIDTH;
    outPort->omxConfigCrop.nHeight = DEFAULT_HEIGHT;

    setHeader(&outPort->omxConfigRotate, sizeof(OMX_CONFIG_ROTATIONTYPE));
    outPort->omxConfigRotate.nPortIndex = OMX_BASE_FILTER_OUTPUTPORT_INDEX;
    outPort->omxConfigRotate.nRotation = 0;

    setHeader(&outPort->omxConfigMirror, sizeof(OMX_CONFIG_MIRRORTYPE));
    outPort->omxConfigMirror.nPortIndex = OMX_BASE_FILTER_OUTPUTPORT_INDEX;
    outPort->omxConfigMirror.eMirror = OMX_MirrorNone;

    setHeader(&outPort->omxConfigScale, sizeof(OMX_CONFIG_SCALEFACTORTYPE));
    outPort->omxConfigScale.nPortIndex = OMX_BASE_FILTER_OUTPUTPORT_INDEX;
    outPort->omxConfigScale.xWidth = outPort->omxConfigScale.xHeight = 0x10000;

    setHeader(&outPort->omxConfigOutputPosition, sizeof(OMX_CONFIG_POINTTYPE));
    outPort->omxConfigOutputPosition.nPortIndex = OMX_BASE_FILTER_OUTPUTPORT_INDEX;
    outPort->omxConfigOutputPosition.nX = outPort->omxConfigOutputPosition.nY = 0;

}

/** The Initialization function of the video decoder
*/
OMX_ERRORTYPE omx_vpudec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp)
{
    UNREFERENCED_PARAMETER(openmaxStandComp);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    s_dec_config.mp4DeblkEnable = 0;    // configure the vpu specified feature;

    s_dec_config.wtlEnable  = FALSE;
    s_dec_config.wtlMode    = FF_FRAME;
    s_dec_config.mapType    = LINEAR_FRAME_MAP;


    s_dec_config.FrameCacheBypass   = 0;
    s_dec_config.FrameCacheBurst    = 0;
    s_dec_config.FrameCacheMerge    = 3;
    s_dec_config.FrameCacheWayShape = 15;

    return err;
}

/** The Deinitialization function of the video decoder
*/
OMX_ERRORTYPE omx_vpudec_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = (omx_vpudec_component_PrivateType *)openmaxStandComp->pComponentPrivate;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    DEBUG(DEB_LEV_SIMPLE_SEQ, "omx_vpudec_component_Deinit - In");

    omx_bufferheader_queue_dequeue_all(omx_vpudec_component_Private->in_bufferheader_queue);
    omx_vpudec_component_Private->lastTimeStamp = (OMX_TICKS)-1;
    omx_vpudec_component_Private->lastInputStamp = (OMX_TICKS)-1;

    if(OmxVpuFlush(openmaxStandComp) == OMX_FALSE)
        return OMX_ErrorInsufficientResources;

    omx_vpudec_component_vpuLibDeInit(omx_vpudec_component_Private);
    omx_vpudec_component_Private->vpuReady = OMX_FALSE;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "omx_vpudec_component_Deinit - Out");
    return err;
}

static int ecolor2fbformat(OMX_COLOR_FORMATTYPE eColorFormat)
{
    DEBUG(DEB_LEV_FULL_SEQ, "%s eColorFormat %d\n", __func__, eColorFormat);
    switch (eColorFormat)
    {
        case OMX_COLOR_FormatYUV420Planar:
        case OMX_COLOR_FormatYUV420PackedPlanar:
        case OMX_COLOR_FormatYUV420SemiPlanar:
        case OMX_COLOR_FormatYUV420PackedSemiPlanar:
            return FORMAT_420;
        case OMX_COLOR_FormatYUV422Planar:
        case OMX_COLOR_FormatYUV422PackedPlanar:
        case OMX_COLOR_FormatYUV422SemiPlanar:
        case OMX_COLOR_FormatYUV422PackedSemiPlanar:
            return FORMAT_422;
        case OMX_COLOR_FormatYCbYCr:
            return FORMAT_YUYV;
        case OMX_COLOR_FormatYCrYCb:
            return FORMAT_YVYU;
        case OMX_COLOR_FormatCbYCrY:
            return FORMAT_UYVY;
        case OMX_COLOR_FormatCrYCbY:
            return FORMAT_VYUY;
        case OMX_COLOR_FormatYUV444Interleaved:
            return FORMAT_444;
        default:
            DEBUG(DEB_LEV_ERR, "%s unknown eColorFormat=0x%x", __func__, eColorFormat);
            return FORMAT_420;
    }
}

/** Executes all the required steps after an output buffer frame-size has changed.
*/
static inline void UpdateFrameSize(OMX_COMPONENTTYPE *openmaxStandComp)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    omx_vpudec_component_PortType *inPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];

    OMX_U32 width = omx_vpudec_component_Private->nMaxFrameWidth >
                    inPort->sPortParam.format.video.nFrameWidth ?
                    omx_vpudec_component_Private->nMaxFrameWidth :
                    inPort->sPortParam.format.video.nFrameWidth;
    OMX_U32 height = omx_vpudec_component_Private->nMaxFrameHeight >
                     inPort->sPortParam.format.video.nFrameHeight ?
                     omx_vpudec_component_Private->nMaxFrameHeight :
                     inPort->sPortParam.format.video.nFrameHeight;

    calculate_fb_param(openmaxStandComp, width, height);

    outPort->sPortParam.format.video.nFrameWidth = pVpu->fbWidth;
    outPort->sPortParam.format.video.nFrameHeight = pVpu->fbHeight;

    outPort->sPortParam.format.video.nStride = pVpu->fbStride;
    outPort->sPortParam.format.video.nSliceHeight = pVpu->fbHeight;

    outPort->omxConfigCrop.nLeft = inPort->omxConfigCrop.nLeft;
    outPort->omxConfigCrop.nTop = inPort->omxConfigCrop.nTop;
    outPort->omxConfigCrop.nWidth = inPort->omxConfigCrop.nWidth;
    outPort->omxConfigCrop.nHeight = inPort->omxConfigCrop.nHeight;

    TiledMapType mapType = pVpu->mapType;
    if (pVpu->coreIdx == VPU_WAVE412_IDX)
    {
        mapType = LINEAR_FRAME_MAP;
        if (pVpu->decOP.pvricFbcEnable)
        {
            mapType = PVRIC_COMPRESSED_FRAME_MAP;
        }
    }
    outPort->sPortParam.nBufferSize = VPU_GetFrameBufSize(pVpu->coreIdx, outPort->sPortParam.format.video.nStride, outPort->sPortParam.format.video.nFrameHeight, mapType, pVpu->fbFormat, pVpu->decOP.cbcrInterleave, &pVpu->dramCfg);

    DEBUG(DEB_LEV_SIMPLE_SEQ, "Out %s OutPort nBufferSize=%d, eColorFormat=0x%x, nFrameWidth=%d, nFrameHeight=%d, nStride=%d, nSliceHeight=%d, crop nWidth=%d, crop nHeight=%d, mapType %d\n", __func__,
        (int)outPort->sPortParam.nBufferSize, (int)outPort->sPortParam.format.video.eColorFormat, (int)outPort->sPortParam.format.video.nFrameWidth, (int)outPort->sPortParam.format.video.nFrameHeight,
        (int)outPort->sPortParam.format.video.nStride, (int)outPort->sPortParam.format.video.nSliceHeight, (int)outPort->omxConfigCrop.nWidth, (int)outPort->omxConfigCrop.nHeight, mapType);
}

#if 0
static int calculate_minipippen_scale_resolution(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BOOL bScaleToSreen)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;

    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    int surfaceWidth = DEFAULT_SCREEN_WIDTH;
    int surfaceHeight = DEFAULT_SCREEN_HEIGHT;

    DEBUG(DEB_LEV_FULL_SEQ, "In %s , xWidth = %d xHeight = %d  picWidth=%d picHeight=%d\n", __func__, (int)outPort->omxConfigScale.xWidth, (int)outPort->omxConfigScale.xHeight, pVpu->initialInfo.picWidth, pVpu->initialInfo.picHeight);

    if (bScaleToSreen)
    {
        if (pVpu->initialInfo.picWidth < surfaceWidth || pVpu->initialInfo.picHeight < surfaceHeight)
        {
            pVpu->scalerInfo.scaleWidth   = (pVpu->initialInfo.picWidth+15)&~15;

            pVpu->scalerInfo.scaleHeight = ((pVpu->initialInfo.picHeight + 7)&~7);
#ifdef SUPPORT_SCALER_STRIDE
            pVpu->scalerInfo.scaleStride = (pVpu->initialInfo.picWidth+15)&~15;
#endif
        }
        else
        {
            pVpu->scalerInfo.scaleWidth   = surfaceWidth;
            pVpu->scalerInfo.scaleHeight  = surfaceHeight;
#ifdef SUPPORT_SCALER_STRIDE
            pVpu->scalerInfo.scaleStride = ((surfaceWidth+15)&~15);
#endif
        }

        if (pVpu->scalerInfo.scaleWidth < MINIPIPPEN_MINMUM_RESOLUTION)
            return 0;

        if (pVpu->scalerInfo.scaleHeight < MINIPIPPEN_MINMUM_RESOLUTION)
            return 0;

    }
    else
    {
        if (outPort->omxConfigScale.xWidth != 0x10000 || outPort->omxConfigScale.xHeight != 0x10000)
        {
            /** omxConfigScale.xWidth is 15:16 fixed point */
            pVpu->scalerInfo.scaleWidth = (pVpu->initialInfo.picWidth * outPort->omxConfigScale.xWidth)>>16;
            pVpu->scalerInfo.scaleWidth = ((pVpu->scalerInfo.scaleWidth+15)&~15);

            if (pVpu->scalerInfo.scaleWidth < MINIPIPPEN_MINMUM_RESOLUTION)
                return 0;

            if (pVpu->scalerInfo.scaleWidth > (uint32_t)((pVpu->initialInfo.picWidth+15)&~15))
                return 0;

            pVpu->scalerInfo.scaleHeight = (pVpu->initialInfo.picHeight * outPort->omxConfigScale.xHeight)>>16;
            pVpu->scalerInfo.scaleHeight = (pVpu->scalerInfo.scaleHeight+7)&~7;

            if (pVpu->scalerInfo.scaleHeight < MINIPIPPEN_MINMUM_RESOLUTION)
                return 0;

            if (pVpu->scalerInfo.scaleHeight > (uint32_t)((pVpu->initialInfo.picHeight+7)&~7))
                return 0;
#ifdef SUPPORT_SCALER_STRIDE
            pVpu->scalerInfo.scaleStride =  ((pVpu->scalerInfo.scaleWidth+15)&~15);
#endif

        }
        else
        {
            pVpu->scalerInfo.scaleWidth = ((pVpu->initialInfo.picWidth + 15)&~15);
            pVpu->scalerInfo.scaleHeight = ((pVpu->initialInfo.picHeight + 7)&~7);
#ifdef SUPPORT_SCALER_STRIDE
            pVpu->scalerInfo.scaleStride = ((pVpu->initialInfo.picWidth + 15)&~15);
#endif
        }

    }
#ifdef SUPPORT_SCALER_STRIDE
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Out %s , scaleWidth = %d, scaleHeight = %d, scaleStride=%d\n", __func__, pVpu->scalerInfo.scaleWidth, pVpu->scalerInfo.scaleHeight, pVpu->scalerInfo.scaleStride);
#else
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Out %s , scaleWidth = %d, scaleHeight = %d\n", __func__, pVpu->scalerInfo.scaleWidth, pVpu->scalerInfo.scaleHeight);
#endif

    return 1;
}
#endif

static void calculate_fb_param(OMX_COMPONENTTYPE *openmaxStandComp, OMX_U32 width, OMX_U32 height)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;

    pVpu->fbWidth = VPU_ALIGN32(width);
    pVpu->fbHeight = VPU_ALIGN32(height);

    TiledMapType mapType = pVpu->mapType;
    if (pVpu->coreIdx == VPU_WAVE412_IDX)
    {
        mapType = LINEAR_FRAME_MAP;
        if (pVpu->decOP.pvricFbcEnable)
        {
            mapType = PVRIC_COMPRESSED_FRAME_MAP;
        }

        if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingVP9)
        {
            pVpu->fbWidth = VPU_ALIGN64(width);
            pVpu->fbHeight = VPU_ALIGN64(height);
        }
        else
        {
            pVpu->fbWidth = VPU_ALIGN32(width);
            pVpu->fbHeight = VPU_ALIGN32(height);
        }
    }

    pVpu->fbStride = CalcStride(pVpu->fbWidth, height, pVpu->fbFormat, pVpu->decOP.cbcrInterleave,
        mapType, omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingVP9);

    DEBUG(DEB_LEV_SIMPLE_SEQ, "%s fbWidth %d, fbHeight %d, fbStride %d\n", __func__, pVpu->fbWidth, pVpu->fbHeight, pVpu->fbStride);
}

static bool omx_vpudec_component_HandleError(OMX_COMPONENTTYPE *openmaxStandComp)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    RetCode ret = RETCODE_SUCCESS;

    DEBUG(DEB_LEV_FULL_SEQ, "%s enter, tryCount %d\n", __func__, pVpu->tryCount);
    pVpu->int_reason = 0;

    if (pVpu->tryCount >= VPU_DEC_TIMEOUT_CNT)
    {
        DEBUG(DEB_LEV_ERR, "%s quit due to maximum trycount reaching\n", __func__);
        VPU_SWReset(pVpu->coreIdx, SW_RESET_SAFETY, pVpu->handle);
        return OMX_ErrorHardware;
    }

    VPU_ClearInterrupt(pVpu->coreIdx);
    pVpu->decParam.skipframeMode = 1;
    pVpu->tryCount++;

    return ret;
}

/** This function is used to process the input buffer and provide one output buffer
*/
void omx_vpudec_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_BUFFERHEADERTYPE* pOutputBuffer)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;

    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *inPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    RetCode ret = RETCODE_SUCCESS;
    int size = 0;
    FrameBuffer *fbUserFrame;
    OMX_U8 *chunkData = 0;
    OMX_U32 chunkSize = 0;
    int picType = 0;
    int sizeLength = 0;
    vpu_buffer_t vbStream;
    OMX_BOOL portSettingChangeDetected = OMX_FALSE;
    omx_display_flag *disp_flag_array = &omx_vpudec_component_Private->omx_display_flags[0];
    int dispFlagIdx = 0;
    int i;
    int err = OMX_ErrorStreamCorrupt;
    double ts_start = 0.0;
    double ts_end = 0.0;

    if (omx_vpudec_component_Private->portSettingChangeRequest == OMX_TRUE) // this should not enter vpu_flush_mutex lock to reduce lock delay
    {
        DEBUG(DEB_LEV_SIMPLE_SEQ, "port_setting_change_tsem wait start semval=%d, portSettingChangeRequest=%d\n", (int)omx_vpudec_component_Private->port_setting_change_tsem.semval, (int)omx_vpudec_component_Private->portSettingChangeRequest);
        /* VTS portReconfiguration may fail due to reach DEFAULT_TIMEOUT (100ms)
         * port_setting_change_tsem wait timeout shall be less than this DEFAULT_TIMEOUT
         */
        tsem_timed_down(&omx_vpudec_component_Private->port_setting_change_tsem, 50);
        DEBUG(DEB_LEV_SIMPLE_SEQ, "port_setting_change_tsem wait end semval=%d, portSettingChangeRequest=%d\n", (int)omx_vpudec_component_Private->port_setting_change_tsem.semval, (int)omx_vpudec_component_Private->portSettingChangeRequest);
        if (omx_vpudec_component_Private->portSettingChangeRequest == OMX_TRUE)
        {
            if (pInputBuffer)
                pInputBuffer->nFilledLen = pInputBuffer->nFilledLen; // it means the inputBuffer should remain.
            if (pOutputBuffer)
                pOutputBuffer->nFilledLen = 0; // there is no data to output.

            return;
        }
    }

    pthread_mutex_lock(&omx_vpudec_component_Private->vpu_flush_mutex);

    if (omx_vpudec_component_Private->vpuReady == OMX_FALSE)
    {
        err = omx_vpudec_component_vpuLibInit(omx_vpudec_component_Private);
        if (OMX_ErrorNone != err)
        {
            DEBUG(DEB_LEV_ERR, "In %s omx_vpudec_component_vpuLibInit Failed err=%d\n", __func__, (int)err);
            goto ERR_DEC;
        }
        omx_vpudec_component_Private->vpuReady = OMX_TRUE;

        /* set sram buffer for sec AXI, coda dec using soc-sram*/
        if (pVpu->productId == PRODUCT_ID_980) {
             /*coda988*/
            pVpu->secAxiUse.u.coda9.useBitEnable = USE_BIT_INTERNAL_BUF;
            pVpu->secAxiUse.u.coda9.useIpEnable = USE_IP_INTERNAL_BUF;
            pVpu->secAxiUse.u.coda9.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
            pVpu->secAxiUse.u.coda9.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
            pVpu->secAxiUse.u.coda9.useBtpEnable = USE_BTP_INTERNAL_BUF;
            pVpu->secAxiUse.u.coda9.useOvlEnable = USE_OVL_INTERNAL_BUF;

            ret = VPU_DecGiveCommand(pVpu->handle, SET_SEC_AXI, &pVpu->secAxiUse);
            if (ret != RETCODE_SUCCESS) {
                DEBUG(DEB_LEV_ERR,
                      "VPU_DecGiveCommand ( SET_SEC_AXI ) failed Error code is 0x%x \n", ret);
                goto ERR_DEC;
            }
        }
    }

    /* To make VPU to be worked asynchronously between InputBuffer(EmptyThisBuffer) and outputBuffer(FillThisBuffer) operation.
    * IS_STATE_EMPTYTHISBUFFER == true  : decoding only
    * IS_STATE_FILLTHISBUFFER == true : transfer the output to renderer without decoding.
    */
    if(IS_STATE_EMPTYTHISBUFFER(pInputBuffer) == OMX_TRUE) // pInputBuffer has a values
    {
        if(pInputBuffer->nSize > 0)
        {
                DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, IS_STATE_EMPTYTHISBUFFER InstIdx=%d, video_coding_type=%d, input flags=0x%x, nFilledLen=%d, seqInited=%d,  bAllocateBuffer=%d, bUseOmxInputBufferAsDecBsBuffer=%d\nchunkReuseRequired=%d, inputBuffer addr=%p, data : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x, \n", __func__,
                    (int)pVpu->instIdx, (unsigned int)omx_vpudec_component_Private->video_coding_type, (unsigned int)pInputBuffer->nFlags, (int)pInputBuffer->nFilledLen, pVpu->seqInited, (int)inPort->bAllocateBuffer, (int)omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer, (int)pVpu->chunkReuseRequired, pInputBuffer->pBuffer,
                    (int)pInputBuffer->pBuffer[0],(int)pInputBuffer->pBuffer[1],(int)pInputBuffer->pBuffer[2],(int)pInputBuffer->pBuffer[3],(int)pInputBuffer->pBuffer[4],(int)pInputBuffer->pBuffer[5],(int)pInputBuffer->pBuffer[6],(int)pInputBuffer->pBuffer[7],(int)pInputBuffer->pBuffer[8],(int)pInputBuffer->pBuffer[9],(int)pInputBuffer->pBuffer[10],(int)pInputBuffer->pBuffer[11],(int)pInputBuffer->pBuffer[12],(int)pInputBuffer->pBuffer[13],(int)pInputBuffer->pBuffer[14],(int)pInputBuffer->pBuffer[15]);
        }
#ifdef SUPPORT_NALU_FORMAT_BY_OMX
        if (omx_vpudec_component_Private->bSupportNaluFormat == OMX_TRUE)
        {
            if (OmxPackNaluToAU(omx_vpudec_component_Private, pInputBuffer) == OMX_FALSE) // skip until complete to make AU
                goto NEXT_CHUNK;
        }
#endif

        if (pVpu->decodefinish)
        {
            if (pInputBuffer->nFilledLen > 0)
            {
                // to support loop decoding
                VPU_DecUpdateBitstreamBuffer(pVpu->handle, -1);

                // to support loop decoding, below vars should be reset.
                omx_vpudec_component_Private->bIsOutputEOSReached = OMX_FALSE;
                omx_vpudec_component_Private->bIsEOSReached = OMX_FALSE;
                pVpu->decodefinish = 0;
                DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s decoder already handled EOS procedure. need VPU_DecUpdateBitstreamBuffer to support loop decoding\n", __func__);
            }
            else
            {
                DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s decoder already handled EOS procedure. goto REACH_INPUT_BUFFER_EOS\n", __func__);
                goto REACH_INPUT_BUFFER_EOS;
            }
        }

        if ((pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuffer->nFilledLen == 0))
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX Core send the end of stream indicator frameIdx=%d\n",pVpu->frameIdx);

            if (!pVpu->seqInited)
                goto REACH_INPUT_BUFFER_EOS;

            VPU_DecSetRdPtrEx(pVpu->handle, pVpu->vbStream[0].phys_addr, pVpu->vbStream[0].phys_addr, 1);  // to set to same address (any address)
            VPU_DecUpdateBitstreamBuffer(pVpu->handle, STREAM_END_SIZE);    //tell VPU to reach the end of stream. starting flush decoded output in VPU
            goto FLUSH_BUFFER;
        }

        // for chunk reuse case, go directly to the decoding process, and the bs buffer does not need to be refilled
        if (pVpu->chunkReuseRequired)
        {
            // cool down
            tsem_timed_down(&omx_vpudec_component_Private->disp_Buf_full_tsem, 10);
            goto REUSE_BUFFER;
        }

#ifdef SUPPORT_ONE_BUFFER_CONTAIN_MULTIPLE_FRAMES
        chunkData = (OMX_U8 *)(pInputBuffer->pBuffer + pInputBuffer->nOffset);
#else
        chunkData = (OMX_U8 *)pInputBuffer->pBuffer;
#endif
        chunkSize = pInputBuffer->nFilledLen;

        if (inPort->bAllocateBuffer == OMX_TRUE && omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer == OMX_TRUE)
        {
            if (OmxGetVpuBsBufferByVirtualAddress(openmaxStandComp, &vbStream, pInputBuffer) == OMX_FALSE)
            {
                DEBUG(DEB_LEV_ERR, "Not found bistream buffer in OMX Buffer Header=%p\n", pInputBuffer->pBuffer);
                goto ERR_DEC;
            }
        }
        else
        {
            vbStream = pVpu->vbStream[pVpu->bsQueueIndex];
            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s bsQueueIndex %d\n", __func__, pVpu->bsQueueIndex);
            if (pVpu->coreIdx == VPU_WAVE412_IDX) {
                pVpu->bsQueueIndex++;
                pVpu->bsQueueIndex %= pVpu->bsBufferCount;
            }
        }

#ifdef SUPPORT_ONE_BUFFER_CONTAIN_MULTIPLE_FRAMES
        VPU_DecSetRdPtrEx(pVpu->handle, vbStream.phys_addr, vbStream.phys_addr + pInputBuffer->nOffset, 1);
#else
        VPU_DecSetRdPtrEx(pVpu->handle, vbStream.phys_addr, vbStream.phys_addr, 1);
#endif

        if ((pInputBuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG) || !pVpu->seqInited)
        {
            // case of RV, the width/height info in slice header is correct(more correct than info in sequence header).
            // so both sequence header and slice header should be in the bitstream buffer when SEQ_INIT.
            // that means copy operation should be needed only when SEQ_INIT.
            if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingRV
                || omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingMPEG4
                || omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingWMV)
            {
                if ((pInputBuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG))
                {
                    omx_vpudec_component_Private->seqHeaderSize =
                        BuildOmxSeqHeader(omx_vpudec_component_Private->seqHeader, pInputBuffer, openmaxStandComp, &sizeLength);
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "keep seqHeader data to attach the seqHeader to bitstream buffer with picture data(next chunk) \n");
                    goto NEXT_CHUNK;  //seq header is stored in the omx_vpudec_component_Private->seqHeader, and will be writed to bs buffer at next cycle
                }
            }
            else
            {
                omx_vpudec_component_Private->seqHeaderSize =
                    BuildOmxSeqHeader(omx_vpudec_component_Private->seqHeader, pInputBuffer, openmaxStandComp, &sizeLength);
                DEBUG(DEB_LEV_SIMPLE_SEQ, "BuildOmxSeqHeader with seqHeaderSize = %d \n", omx_vpudec_component_Private->seqHeaderSize);
                if (omx_vpudec_component_Private->seqHeaderSize < 0) // indicate the stream dose not support in VPU.
                {
                    DEBUG(DEB_LEV_ERR, "BuildOmxSeqHeader the stream does not support in VPU \n");
                    err = OMX_ErrorBadParameter; //OMX_ErrorUnsupportedSetting
                    goto ERR_DEC;
                }
            }

            DEBUG(DEB_LEV_SIMPLE_SEQ, "BuildOmxSeqHeader with seqHeaderSize = %d \n", omx_vpudec_component_Private->seqHeaderSize);
            if (omx_vpudec_component_Private->seqHeaderSize > 0)
            {
                size = OmxWriteBsBufFromBufHelper(openmaxStandComp, pVpu, vbStream, omx_vpudec_component_Private->seqHeader, omx_vpudec_component_Private->seqHeaderSize);
                if (size < 0)
                {
                    DEBUG(DEB_LEV_ERR, "OmxWriteBsBufFromBufHelper seqHeader write failed Error code is 0x%x \n", (int)size);
                    goto ERR_DEC;
                }
            }
        }

        // Build and Fill picture Header data which is dedicated for VPU
        omx_vpudec_component_Private->picHeaderSize = BuildOmxPicHeader(omx_vpudec_component_Private->picHeader, pInputBuffer, openmaxStandComp, sizeLength);
        if (omx_vpudec_component_Private->picHeaderSize > 0)
        {
            size = OmxWriteBsBufFromBufHelper(openmaxStandComp, pVpu, vbStream, omx_vpudec_component_Private->picHeader, omx_vpudec_component_Private->picHeaderSize);
            if (size < 0)
            {
                DEBUG(DEB_LEV_ERR, "OmxWriteBsBufFromBufHelper failed Error size=%d\n", (int)size);
                goto NEXT_CHUNK;
            }
        }

        // Fill VCL data
#ifdef OMX_INPUT_BUFFER_FEEDED_BY_FFMPEG_AVPACKET
        if (pVpu->decOP.bitstreamFormat == STD_RV)
        {
            int cSlice = chunkData[0] + 1;
            int nSlice =  chunkSize - 1 - (cSlice * 8);
            chunkData += (1+(cSlice*8));
            chunkSize = nSlice;
        }
#endif
        DEBUG(DEB_LEV_SIMPLE_SEQ, "OmxWriteBsBufFromBufHelper chunkSize %d, prevConsumeBytes %d\n", chunkSize, pVpu->prevConsumeBytes);
        size = OmxWriteBsBufFromBufHelper(openmaxStandComp, pVpu, vbStream, chunkData, chunkSize);
        if (size <0)
        {
            DEBUG(DEB_LEV_ERR, "OmxWriteBsBufFromBufHelper failed Error code is 0x%x \n", (int)size);
            goto NEXT_CHUNK;
        }

        OMX_DUMP_INTPUT_TO_FILE(pVpu);
        ts_start = GetNowMs();

REUSE_BUFFER:
        DEBUG(DEB_LEV_SIMPLE_SEQ, "seqInited %d, bSeqChangeDetected %d\n", pVpu->seqInited, omx_vpudec_component_Private->bSeqChangeDetected);
        if (!pVpu->seqInited || omx_vpudec_component_Private->bSeqChangeDetected)
        {
            if ((ret = VPU_DecIssueSeqInit(pVpu->handle)) != RETCODE_SUCCESS)
            {
                DEBUG(DEB_LEV_ERR, "VPU_DecIssueSeqInit failed Error code is 0x%x \n", ret);
                VPU_DecCompleteSeqInit(pVpu->handle, &pVpu->initialInfo);
                goto ERR_DEC;
            }

            int32_t interruptFlag = 0;
            DEBUG(DEB_LEV_FULL_SEQ, "VPU_WaitInterrupt line %d\n", __LINE__);
            interruptFlag = VPU_WaitInterrupt(pVpu->coreIdx, OMX_VPU_DEC_TIMEOUT); //wait for 10ms to save stream filling time.
            if (interruptFlag == -1)
            {
                VPU_DecCompleteSeqInit(pVpu->handle, &pVpu->initialInfo);
                if (pVpu->tryCount > VPU_DEC_TIMEOUT_CNT)
                {
                    DEBUG(DEB_LEV_ERR, "VPU interrupt wait timeout\n");
                    VPU_SWReset(pVpu->coreIdx, SW_RESET_SAFETY, pVpu->handle);
                    err = OMX_ErrorHardware;
                    goto ERR_DEC;
                }
                DEBUG(DEB_LEV_FULL_SEQ, "VPU_WaitInterrupt -1 line %d, trycount %d\n", __LINE__, pVpu->tryCount);
                pVpu->tryCount++;
                goto NEXT_CHUNK;
            }

            if (interruptFlag)
            {
                VPU_ClearInterrupt(pVpu->coreIdx);
                if (interruptFlag & (1 << INT_BIT_SEQ_INIT))
                {
                    DEBUG(DEB_LEV_FULL_SEQ, "INT_BIT_SEQ_INIT\n");
                    pVpu->tryCount = 0;
                }
                else
                {
                    DEBUG(DEB_LEV_FULL_SEQ, "Non INT_BIT_SEQ_INIT %x\n", interruptFlag);
                    pVpu->tryCount++;
                    goto NEXT_CHUNK;
                }
            }

            if ((ret = VPU_DecCompleteSeqInit(pVpu->handle, &pVpu->initialInfo)) != RETCODE_SUCCESS)
            {
                DEBUG(DEB_LEV_ERR, "[ERROR] Failed to SEQ_INIT(ERROR REASON: %d(0x%x), ret value:%d\n",
                    pVpu->initialInfo.seqInitErrReason, pVpu->initialInfo.seqInitErrReason, ret);
                err = OMX_ErrorStreamCorrupt;
                goto ERR_DEC;
            }

            DEBUG(DEB_LEV_SIMPLE_SEQ,
                "VPU_DecCompleteSeqInit success, picWidth : %d,picHeight : %d, picCropRect.right : %d,"
                "picCropRect.bottom : %d, minFrameBufferCount : %d, nBufferCountActual :%d, RdPtr : 0x%x, WrPtr : 0x%x\n",
                (int)pVpu->initialInfo.picWidth, (int)pVpu->initialInfo.picHeight,
                (int)pVpu->initialInfo.picCropRect.right, (int)pVpu->initialInfo.picCropRect.bottom,
                (int)pVpu->initialInfo.minFrameBufferCount, (int)outPort->sPortParam.nBufferCountActual,
                (int)pVpu->initialInfo.rdPtr, (int)pVpu->initialInfo.wrPtr);

            if (pVpu->initialInfo.picWidth > MAX_DEC_PIC_WIDTH || pVpu->initialInfo.picHeight > MAX_DEC_PIC_HEIGHT)
            {
                DEBUG(DEB_LEV_ERR, "Not Supported Video size MAX_DEC_PIC_WIDTH=%d, MAX_DEC_PIC_HEIGHT=%d\n",
                    MAX_DEC_PIC_WIDTH, MAX_DEC_PIC_HEIGHT);
                err = OMX_ErrorUnsupportedSetting;
                goto ERR_DEC;
            }
            pVpu->seqInited = 1;

            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s xWidth 0x%x, xHeight 0x%x, enScaler %d\n",
                __func__, outPort->omxConfigScale.xWidth, outPort->omxConfigScale.xHeight, pVpu->scalerInfo.enScaler);

            if (pVpu->decOP.pvricFbcEnable) {
                pVpu->scalerInfo.enScaler = TRUE;
                // scale to original size, actually no scale
                pVpu->scalerInfo.scaleWidth = pVpu->initialInfo.picWidth;
                pVpu->scalerInfo.scaleHeight = pVpu->initialInfo.picHeight;
                DEBUG(DEB_LEV_FULL_SEQ, "[SCALE INFO] %dx%d to %dx%d\n", pVpu->initialInfo.picWidth, pVpu->initialInfo.picHeight,
                    pVpu->scalerInfo.scaleWidth, pVpu->scalerInfo.scaleHeight);
            }

            /* TODO: scale support later
             * don't mix up ture scale mode with PVRIC
            if (outPort->omxConfigScale.xWidth != 0x10000 || outPort->omxConfigScale.xHeight != 0x10000)
            {
                pVpu->scalerInfo.enScaler = 1;
                if (!calculate_minipippen_scale_resolution(openmaxStandComp, OMX_FALSE))
                {
                    DEBUG(DEB_LEV_ERR, "fail to calculate_minipippen_scale_resolution\n");
                    goto ERR_DEC;
                }
            }
            else
            {
                if (pVpu->scalerInfo.enScaler) // if scaler is enabled for format convector even no scale ratio. set scaler resolution same as video buffer resolution
                {
                    if (!calculate_minipippen_scale_resolution(openmaxStandComp, SUPPORT_SCALE_TO_SCREEN_SIZE))
                    {
                        DEBUG(DEB_LEV_ERR, "fail to calculate_minipippen_scale_resolution\n");
                        goto ERR_DEC;
                    }
                }
            }
            */

            // calculate framebuffer width, height, stride
            size_t width = pVpu->scalerInfo.enScaler ? pVpu->scalerInfo.scaleWidth : pVpu->initialInfo.picWidth;
            size_t height = pVpu->scalerInfo.enScaler ? pVpu->scalerInfo.scaleHeight : pVpu->initialInfo.picHeight;
            calculate_fb_param(openmaxStandComp, width, height);

            pVpu->frameRate = OmxGetVpuFrameRate(omx_vpudec_component_Private);
            OmxConfigFrameBufferCount(omx_vpudec_component_Private);
            OmxCheckVUIParams(openmaxStandComp, omx_vpudec_component_Private, OMX_FALSE);

            err = OmxCheckCropInfo(openmaxStandComp, omx_vpudec_component_Private);
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s handleInitialInfo Failed err=%d\n", __func__, (int)err);
                goto ERR_DEC;
            }

            if (pVpu->fbStride==0 || pVpu->fbHeight==0 || pVpu->regFbCount==0)
            {
                DEBUG(DEB_LEV_ERR, "wrong sequence information parsed\n");
                goto ERR_DEC;
            }

            if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
                VPU_DecGiveCommand(pVpu->handle, ENABLE_DEC_THUMBNAIL_MODE, 0);
            /* ret = VPU_DecGiveCommand(pVpu->handle, DEC_UPDATE_SCALER_INFO, &pVpu->scalerInfo);
            if (ret != RETCODE_SUCCESS)
            {
                DEBUG(DEB_LEV_ERR, "VPU_DecGiveCommand ( DEC_UPDATE_SCALER_INFO ) failed Error code is 0x%x \n", ret );
                goto ERR_DEC;
            }*/

            portSettingChangeDetected = OMX_FALSE;

            if (outPort->bAllocateBuffer == OMX_TRUE ||
                omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE) // only request to portsettingchange in this case. because this case need to share between decoder buffer and display buffer without copy operation
            {
                if (pVpu->dispFbCount > (int)outPort->sPortParam.nBufferCountActual)
                {
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "%s dispFbCount %d > out nBufferCountActual %d\n",
                        __func__, pVpu->dispFbCount, outPort->sPortParam.nBufferCountActual);
                    portSettingChangeDetected = OMX_TRUE;
                }

                if (pVpu->decOP.pvricFbcEnable)
                {
#ifdef ANDROID
                    if (outPort->sPortParam.format.video.eColorFormat != OMX_SEMI_COLOR_FormatIFBC32x8Tiled)
                    {
                        DEBUG(DEB_LEV_ERR, "Intent to use ifbc, but not supported by display, use linear instead!\n");
                        portSettingChangeDetected = OMX_TRUE;
                        omx_vpudec_component_Private->nUsage |= GRALLOC_USAGE_SW_READ_OFTEN;
                        pVpu->decOP.pvricFbcEnable = FALSE;
                        if (VPU_DecGiveCommand(pVpu->handle, DEC_SET_PVRIC_MODE, (void *)&pVpu->decOP.pvricFbcEnable)) {
                            DEBUG(DEB_LEV_ERR, "Failed to VPU_DecGiveCommand(DEC_SET_PVRIC_MODE)\n");
                            goto ERR_DEC;
                        }

                        memset(&pVpu->scalerInfo.enScaler, 0x0, sizeof(ScalerInfo));
                        if (VPU_DecGiveCommand(pVpu->handle, DEC_SET_SCALER_INFO, (void*)&pVpu->scalerInfo) != RETCODE_SUCCESS) {
                            DEBUG(DEB_LEV_ERR, "Failed to VPU_DecGiveCommand(DEC_SET_SCALER_INFO)\n");
                            goto ERR_DEC;
                        }
                    }
#endif
                }

                if(!pVpu->decOP.pvricFbcEnable && pVpu->scalerInfo.enScaler)
                {
#ifdef SUPPORT_SCALER_STRIDE
                    if ((!omx_vpudec_component_Private->bAdaptiveEnable &&
                         (inPort->sPortParam.format.video.nFrameWidth != (OMX_U32)pVpu->scalerInfo.scaleStride ||
                          inPort->sPortParam.format.video.nFrameHeight != (OMX_U32)pVpu->scalerInfo.scaleHeight)) ||
                        (inPort->sPortParam.format.video.nFrameWidth < (OMX_U32)pVpu->scalerInfo.scaleStride ||
                         inPort->sPortParam.format.video.nFrameHeight < (OMX_U32)pVpu->scalerInfo.scaleHeight))
                    {
                        portSettingChangeDetected = OMX_TRUE;
                        if (omx_vpudec_component_Private->bAdaptiveEnable)
                        {
                            omx_vpudec_component_Private->nMaxFrameWidth = width;
                            omx_vpudec_component_Private->nMaxFrameHeight = height;
                        }
                        DEBUG(DEB_LEV_SIMPLE_SEQ,
                              "----> Scale mode:sPortParam nFrameWidth is not equal scaleStride,"
                              "nFrameWidth=%d, nFrameHeight=%d, nStride=%d vs scaleStride=%d,"
                              "scaleHeight=%d, adaptiveMaxFrameWidth:%d, adaptiveMaxFrameHeight:%d\n",
                              (int)inPort->sPortParam.format.video.nFrameWidth,
                              (int)inPort->sPortParam.format.video.nFrameHeight,
                              (int)inPort->sPortParam.format.video.nStride,
                              pVpu->scalerInfo.scaleStride,
                              pVpu->scalerInfo.scaleHeight,
                              omx_vpudec_component_Private->nMaxFrameWidth,
                              omx_vpudec_component_Private->nMaxFrameHeight);
                    }
#else
                    if( (!omx_vpudec_component_Private->bAdaptiveEnable &&
                        (inPort->sPortParam.format.video.nFrameWidth != (OMX_U32)pVpu->scalerInfo.scaleWidth ||
                        inPort->sPortParam.format.video.nFrameHeight != (OMX_U32)pVpu->scalerInfo.scaleHeight))
                        ||
                        (inPort->sPortParam.format.video.nFrameWidth < (OMX_U32)pVpu->scalerInfo.scaleWidth ||
                        inPort->sPortParam.format.video.nFrameHeight < (OMX_U32)pVpu->scalerInfo.scaleHeight))
                    {
                        portSettingChangeDetected = OMX_TRUE;
                        if (omx_vpudec_component_Private->bAdaptiveEnable)
                        {
                            omx_vpudec_component_Private->nMaxFrameWidth = width;
                            omx_vpudec_component_Private->nMaxFrameHeight = height;
                        }
                        DEBUG(DEB_LEV_SIMPLE_SEQ,
                              "----> Scale mode:sPortParam nFrameWidth is not equal scaleStride,"
                              "nFrameWidth=%d, nFrameHeight=%d, nStride=%d vs scaleWidth=%d,"
                              "scaleHeight=%d, adaptiveMaxFrameWidth:%d, adaptiveMaxFrameHeight:%d\n",
                              (int)inPort->sPortParam.format.video.nFrameWidth,
                              (int)inPort->sPortParam.format.video.nFrameHeight,
                              (int)inPort->sPortParam.format.video.nStride,
                              pVpu->scalerInfo.scaleWidth,
                              pVpu->scalerInfo.scaleHeight,
                              omx_vpudec_component_Private->nMaxFrameWidth,
                              omx_vpudec_component_Private->nMaxFrameHeight);
                    }
#endif
                }
                else
                {
                    if( (!omx_vpudec_component_Private->bAdaptiveEnable &&
                        (outPort->sPortParam.format.video.nFrameWidth != (OMX_U32)pVpu->fbStride ||
                        outPort->sPortParam.format.video.nFrameHeight != (OMX_U32)pVpu->fbHeight))
                        ||
                        (outPort->sPortParam.format.video.nFrameWidth < (OMX_U32)pVpu->fbStride ||
                        outPort->sPortParam.format.video.nFrameHeight < (OMX_U32)pVpu->fbHeight))
                    {
#ifdef ANDROID
                        portSettingChangeDetected = OMX_TRUE;
#else
                        /* TODO: unify portSettingChange with android and linux
                         * using AllocateBuffer, already UpdateFrameSize
                         * no need to call portSettingChange
                         */
                        DEBUG(DEB_LEV_FULL_SEQ, "\nportSettingChange not supported yet on linux\n");
#endif
                        if (omx_vpudec_component_Private->bAdaptiveEnable)
                        {
                            omx_vpudec_component_Private->nMaxFrameWidth = width;
                            omx_vpudec_component_Private->nMaxFrameHeight = height;
                        }
                        DEBUG(DEB_LEV_SIMPLE_SEQ,
                              "\n----> Normal mode:sPortParam nFrameWidth is not equal fbWidth,"
                              "nFrameWidth=%d, nFrameHeight=%d, nStride=%d, vs fbWidth=%d,"
                              "fbHeight=%d, fbStride=%d, adaptiveMaxFrameWidth:%d, adaptiveMaxFrameHeight:%d\n",
                              (int)outPort->sPortParam.format.video.nFrameWidth,
                              (int)outPort->sPortParam.format.video.nFrameHeight,
                              (int)outPort->sPortParam.format.video.nStride,
                              pVpu->fbWidth,
                              pVpu->fbHeight,
                              pVpu->fbStride,
                              omx_vpudec_component_Private->nMaxFrameWidth,
                              omx_vpudec_component_Private->nMaxFrameHeight);
                    }
                }
            }

            if (portSettingChangeDetected == OMX_TRUE)
            {
                DEBUG(DEB_LEV_SIMPLE_SEQ, "---->Sending Port Settings Change Event in video decoder omxWidth=%d, omxHeight=%d, omxStride=%d pVpu->fbStride=%d, pVpu->fbHeight=%d\n", (int)(inPort->sPortParam.format.video.nFrameWidth), (int)inPort->sPortParam.format.video.nFrameHeight, (int)inPort->sPortParam.format.video.nStride, (int)pVpu->fbStride, (int)pVpu->fbHeight );
#ifdef SUPPORT_SCALER_STRIDE
                if (pVpu->scalerInfo.enScaler)
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "---->Sending Port Settings Change Event in video decoder omxWidth=%d, omxHeight=%d, omxStride=%d scaleStride=%d, scaleHeight=%d\n", (int)(inPort->sPortParam.format.video.nFrameWidth), (int)inPort->sPortParam.format.video.nFrameHeight, (int)inPort->sPortParam.format.video.nStride, (int)pVpu->scalerInfo.scaleStride, (int)pVpu->scalerInfo.scaleHeight);
#else
                if (pVpu->scalerInfo.enScaler)
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "---->Sending Port Settings Change Event in video decoder omxWidth=%d, omxHeight=%d, omxStride=%d scaleWidth=%d, scaleHeight=%d\n", (int)(inPort->sPortParam.format.video.nFrameWidth), (int)inPort->sPortParam.format.video.nFrameHeight, (int)inPort->sPortParam.format.video.nStride, (int)pVpu->scalerInfo.scaleWidth, (int)pVpu->scalerInfo.scaleHeight);

#endif
                DEBUG(DEB_LEV_SIMPLE_SEQ, "----> pVpu->dispFbCount=%d, pVpu->decFbCount=%d, pVpu->regFbCount=%d, nBufferCountActual=%d\n",
                    (int)pVpu->dispFbCount, (int)pVpu->decFbCount, (int)pVpu->regFbCount, (int)outPort->sPortParam.nBufferCountActual);

                outPort->nTempBufferCountActual = pVpu->dispFbCount; // do not set outPort->sPortParam.nBufferCountActual in direct before Port Reconfiguration(Close and Open)

                if (outPort->bAllocateBuffer == OMX_TRUE ||
                    omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE) // only need to update omx buffer information in this case. because this case need to have same buffer space to share
                {
                    if (!pVpu->decOP.pvricFbcEnable && pVpu->scalerInfo.enScaler)
                    {
#ifdef SUPPORT_SCALER_STRIDE
                        inPort->sPortParam.format.video.nStride = pVpu->scalerInfo.scaleStride;
                        inPort->sPortParam.format.video.nFrameWidth = pVpu->scalerInfo.scaleStride;
#else
                        inPort->sPortParam.format.video.nStride = pVpu->scalerInfo.scaleWidth;
                        inPort->sPortParam.format.video.nFrameWidth = pVpu->scalerInfo.scaleWidth;
#endif
                        inPort->sPortParam.format.video.nFrameHeight = pVpu->scalerInfo.scaleHeight;
                    }
                    else
                    {
                        inPort->sPortParam.format.video.nStride = pVpu->fbStride;
                        inPort->sPortParam.format.video.nFrameWidth = pVpu->fbStride;
                        inPort->sPortParam.format.video.nFrameHeight = pVpu->fbHeight;
                    }
                }
                DEBUG(DEB_LEV_SIMPLE_SEQ, "UpdateFrameSize stride %d, nFrameW %d, nFrameH %d\n", inPort->sPortParam.format.video.nStride,
                    inPort->sPortParam.format.video.nFrameWidth, inPort->sPortParam.format.video.nFrameHeight);
                UpdateFrameSize(openmaxStandComp);

                /*Send Port Settings changed call back*/
                if (omx_vpudec_component_Private->callbacks->EventHandler)
                {
                    OMX_ERRORTYPE ret;

                    if (pVpu->dispOutputInfo.sequenceChanged == 0)
                    {
                        /* waiting until the owner of all fillbuffers changed to OMX_BUFFER_OWNER_COMPONENT except for the amount of NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS before sending PortSettingsChange*/
                        /* if not, OMX_EventPortSettingsChanged event in OMXCocdec will be failed due to CHECK(mFilledBuffers.empty()) */
                        OmxWaitUntilOutBuffersEmpty(omx_vpudec_component_Private);
                    }


                    DEBUG(DEB_LEV_SIMPLE_SEQ, "Call PortSettingsChange\n");
                    ret = (*(omx_vpudec_component_Private->callbacks->EventHandler))
                        (openmaxStandComp,
                        omx_vpudec_component_Private->callbackData,
                        OMX_EventPortSettingsChanged, /* The command was completed */
                        1, /* This is the output port index */
                        0,
                        NULL);

                    if (ret == OMX_ErrorNone)
                    {
                        omx_vpudec_component_Private->portSettingChangeRequest = OMX_TRUE;
                    }
                    else
                    {
                        DEBUG(DEB_LEV_ERR, "fail to OMX_EventPortSettingsChanged Event for output buffer count\n");
                        goto ERR_DEC;
                    }

                    DEBUG(DEB_LEV_SIMPLE_SEQ, "Call PortSettingsChange Event Done");
                }
                pVpu->seqInited = 0;
                omx_vpudec_component_Private->bSeqChangeDetected = OMX_FALSE;

                omx_vpudec_component_Private->portSettingCount++;
                //don't touch inputBuffer->nFilledLen because we need to call seq init again with current chunk at next.
                goto SKIP_DISPLAY;
            }


            DEBUG(DEB_LEV_SIMPLE_SEQ, "Now Realize pVpu->regFbCount=%d, decFbCount=%d, dispFbCount=%d to nBufferCountActual=%d, useNativeBuffer=%d, bThumbnailMode=%d, bAllocateBuffer=%d\n",
                pVpu->regFbCount, pVpu->decFbCount, pVpu->dispFbCount,(int)outPort->sPortParam.nBufferCountActual, omx_vpudec_component_Private->useNativeBuffer, omx_vpudec_component_Private->bThumbnailMode, outPort->bAllocateBuffer);

                if (pVpu->frameRate && ((inPort->sPortParam.format.video.xFramerate == 0) || (inPort->sPortParam.format.video.xFramerate > MAX_FPS_XRATE))) //  means OMXIL client does not set xFramerate
                {
                    inPort->sPortParam.format.video.xFramerate = (OMX_U32)(pVpu->frameRate << 16);
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "update xFramerate inPort->sPortParam.format.video.xFramerate=%d, pVpu->frameRate=%d\n", (int)inPort->sPortParam.format.video.xFramerate,(int)pVpu->frameRate);
                }

            ret = VPU_DecGiveCommand(pVpu->handle, GET_DRAM_CONFIG, &pVpu->dramCfg);
            if( ret != RETCODE_SUCCESS )
            {
                DEBUG(DEB_LEV_ERR, "VPU_DecGiveCommand[GET_DRAM_CONFIG] failed Error code is 0x%x \n", ret );
                goto ERR_DEC;
            }

            fbUserFrame = NULL; // it means VPU_DecRegisterFrameBuffer should allocation frame buffer by user using(not in VPUAPU) VDI and vpu device driver.

            if (!OmxAllocateFrameBuffers(openmaxStandComp))
            {
                 err = OMX_ErrorHardware;
                 goto ERR_DEC;
            }

            fbUserFrame = &pVpu->fbUser[0];

            // Register frame buffers requested by the decoder.
            DEBUG(DEB_LEV_SIMPLE_SEQ, "VPU_DecRegisterFrameBuffer regFbCount %d, stride %d, height %d, map %d\n",
                    pVpu->regFbCount, pVpu->fbStride, pVpu->fbHeight, pVpu->mapType);
            if (pVpu->productId == PRODUCT_ID_410 || pVpu->productId == PRODUCT_ID_4102  || pVpu->productId == PRODUCT_ID_412) {
                if (pVpu->decOP.bitstreamFormat == STD_VP9 && pVpu->interResChanged) {
                    VPU_DecGiveCommand(pVpu->handle, DEC_SET_INTER_RES_INFO_ON, NULL);
                }
                ret = VPU_DecRegisterFrameBufferEx(pVpu->handle, fbUserFrame, pVpu->decFbCount, pVpu->dispFbCount, pVpu->fbStride, pVpu->fbHeight, pVpu->mapType); // frame map type (can be changed before register frame buffer)

                if (pVpu->decOP.bitstreamFormat == STD_VP9 && pVpu->interResChanged) {
                    VPU_DecGiveCommand(pVpu->handle, DEC_SET_INTER_RES_INFO_OFF, NULL);
                    pVpu->interResChanged = 0;
                }

                if (pVpu->decOP.bitstreamFormat == STD_HEVC || pVpu->decOP.bitstreamFormat == STD_VP9) {
                    uint32_t val = (pVpu->decOP.bitstreamFormat == STD_HEVC) ? SEQ_CHANGE_ENABLE_ALL_HEVC : SEQ_CHANGE_ENABLE_ALL_VP9;
                    VPU_DecGiveCommand(pVpu->handle, DEC_SET_SEQ_CHANGE_MASK, (void*)&val);
                }
            }
            else {
                ret = VPU_DecRegisterFrameBuffer(pVpu->handle, fbUserFrame, pVpu->regFbCount, pVpu->fbStride, pVpu->fbHeight, pVpu->mapType); // frame map type (can be changed before register frame buffer)
            }
            if( ret != RETCODE_SUCCESS )
            {
                DEBUG(DEB_LEV_ERR, "VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", (int)ret);
                err = OMX_ErrorHardware;
                goto ERR_DEC;
            }
#if DEBUG_LEVEL > 0
            {
                int i;
                FrameBuffer fb;
                for (i=0; i < MAX_REG_FRAME; i++)
                {
                    if (VPU_DecGetFrameBuffer(pVpu->handle, i, &fb) == RETCODE_SUCCESS)
                    {
                        DEBUG(DEB_LEV_FULL_SEQ, "VPU_DecGetFrameBuffer index=%d, bufY=0x%x, bufCb=0x%x, bufCr=0x%x, vbUserFb.size=%d, vbUserFb.phys_addr=0x%x, vbUserFb.virt_addr=0x%x, vbUserFb.base=0x%x\n",
                            i, fb.bufY, fb.bufCb, fb.bufCr, (int)pVpu->vbUserFb[i].size, (int)pVpu->vbUserFb[i].phys_addr, (int)pVpu->vbUserFb[i].virt_addr, (int)pVpu->vbUserFb[i].base);
                    }
                }
            }
#endif

            if (outPort->bAllocateBuffer == OMX_TRUE || omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
            {
                pthread_mutex_lock(&omx_vpudec_component_Private->display_flag_mutex);
                DEBUG(DEB_LEV_SIMPLE_SEQ, "%s outPort nBufferCountActual %d\n", __func__, outPort->sPortParam.nBufferCountActual);
                for (i = 0; i < (int)outPort->sPortParam.nBufferCountActual; i++)
                {
                    dispFlagIdx = i;
                    if (disp_flag_array[i].owner == OMX_BUFERR_OWNER_COMPONENT)
                    {
                        VPU_DecClrDispFlag(pVpu->handle, dispFlagIdx);
                        DEBUG(DEB_LEV_FULL_SEQ, "%s of component VPU_DecClrDispFlag for index=%d, dispFlagIdx=%d, owner=0x%x\n", __func__,  i, dispFlagIdx, disp_flag_array[i].owner);
                    }
                    else if(disp_flag_array[i].owner == OMX_BUFFER_OWNER_CLIENT || disp_flag_array[i].owner == OMX_BUFFER_OWNER_NOBODY) // this means that the buffer is using display part. ( NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS )
                    {
                        // if (pVpu->decOP.bitstreamFormat == STD_VP9 && i < 2) continue;      // [FIX ME] VP9 needs more than 2 available framebuffer now. need to fix firmware.
                        // VPU_DecClrDispFlag(pVpu->handle, dispFlagIdx);
                        VPU_DecGiveCommand(pVpu->handle, DEC_SET_DISPLAY_FLAG, &dispFlagIdx);
                        DEBUG(DEB_LEV_FULL_SEQ, "%s of component VPU_DecGiveCommand[DEC_SET_DISPLAY_FLAG] for index=%d, dispFlagIdx=%d, owner=0x%x\n", __func__,  i, dispFlagIdx, disp_flag_array[i].owner);
                    }
                }
                pthread_mutex_unlock(&omx_vpudec_component_Private->display_flag_mutex);
            }


            ret = VPU_DecGiveCommand(pVpu->handle, GET_TILEDMAP_CONFIG, &pVpu->mapCfg);
            if( ret != RETCODE_SUCCESS )
            {
                DEBUG(DEB_LEV_ERR, "VPU_DecGiveCommand[GET_TILEDMAP_CONFIG] failed Error code is 0x%x \n", ret);
                err = OMX_ErrorHardware;
                goto ERR_DEC;
            }

            DEBUG(DEB_LEV_FULL_SEQ, "VPU_DecGiveCommand[GET_TILEDMAP_CONFIG] %d\n", pVpu->mapCfg.mapType);

            pVpu->seqInited = 1;
            omx_vpudec_component_Private->bSeqChangeDetected = OMX_FALSE;

            // goto NEXT_CHUNK;
        }

FLUSH_BUFFER:
        if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
            pVpu->decParam.skipframeMode = 1;

        //ConfigDecReport(pVpu->coreIdx, pVpu->handle, pVpu->decOP.bitstreamFormat);

        if(pVpu->int_reason & (1<<INT_BIT_DEC_FIELD))
        {
            VPU_ClearInterrupt(pVpu->coreIdx);
            pVpu->int_reason = 0;
            goto WAIT_LEFT_FIELD_DECODING_DONE;
        }

        // Start decoding a frame.
        ret = VPU_DecStartOneFrame(pVpu->handle, &pVpu->decParam);
        if (ret != RETCODE_SUCCESS)
        {
            DEBUG(DEB_LEV_ERR, "VPU_DecStartOneFrame failed Error code is 0x%x \n", (int)ret);
            goto DEC_FRAME_FAILED;

        }
        DEBUG(DEB_LEV_FULL_SEQ/*DEB_LEV_SIMPLE_SEQ*/, "VPU_DecStartOneFrame() success Waiting PicDone Interrupt, skipframeMode = %d\n", pVpu->decParam.skipframeMode);

WAIT_LEFT_FIELD_DECODING_DONE:

        while(1)
        {
            pVpu->int_reason = VPU_WaitInterrupt(pVpu->coreIdx, OMX_VPU_DEC_TIMEOUT);
            if (pVpu->int_reason == -1 ) // timeout
            {
                DEBUG(DEB_LEV_ERR, "VPU interrupt timeout\n");
                if (omx_vpudec_component_HandleError(openmaxStandComp) != RETCODE_SUCCESS)
                {
                    err = OMX_ErrorHardware;
                    goto DEC_FRAME_FAILED;
                }
                break;
            }

            //CheckUserDataInterrupt(pVpu->coreIdx, pVpu->handle, pVpu->frameIdx, pVpu->decOP.bitstreamFormat, pVpu->int_reason);
            if(pVpu->int_reason & (1<<INT_BIT_DEC_FIELD))
            {
                if (pVpu->decOP.bitstreamFormat == STD_VP9)
                {
                    DEBUG(DEB_LEV_ERR, "Detect PRESCAN-ERROR\n");
                    err = OMX_ErrorHardware;
                    goto DEC_FRAME_FAILED;
                }
                else
                {
                    PhysicalAddress rdPtr, wrPtr;
                    Uint32 room;
                    VPU_DecGiveCommand(pVpu->handle, DEC_GET_FIELD_PIC_TYPE, &picType);
                    DEBUG(DEB_LEV_FULL_SEQ, "Field Decode Interrupt Field Picture Type : %d\n", picType);
                    VPU_DecGetBitstreamBuffer(pVpu->handle, &rdPtr, &wrPtr, &room);
                    if (rdPtr - vbStream.phys_addr >= chunkSize)
                    {
                        goto NEXT_CHUNK;    // do not clear interrupt until feeding next field picture.
                    }
                }
            }

            if (pVpu->int_reason)
                VPU_ClearInterrupt(pVpu->coreIdx);

            if (pVpu->int_reason & (1<<INT_BIT_PIC_RUN))
                break;
        }

        ret = VPU_DecGetOutputInfo(pVpu->handle, &pVpu->dispOutputInfo);
        if (ret != RETCODE_SUCCESS)
        {
            DEBUG(DEB_LEV_ERR, "VPU_DecGetOutputInfo failed Error code is 0x%x \n", (int)ret);
            err = OMX_ErrorHardware;
            goto ERR_DEC;
        }

        if (pVpu->scalerInfo.enScaler)
        {
            DEBUG(DEB_LEV_FULL_SEQ/*DEB_LEV_SIMPLE_SEQ*/,
                "#%d, instIdx=%d, disIdx=%d, decIdx=%d, prescanIdx=%d, picType=%d,"
                "chunkLen=%d, reuseReuired:%d,  dispFlag=0x%x, dispFlagScaler=0x%x,"
                "RdPtr : 0x%x, WrPtr : 0x%x, display left=%d, top=%d, right=%d, bottom=%d\n",
                (int)pVpu->frameIdx, (int)pVpu->instIdx, (int)pVpu->dispOutputInfo.indexFrameDisplay,
                (int)pVpu->dispOutputInfo.indexFrameDecoded,  (int)pVpu->dispOutputInfo.indexFramePrescan,
                (int)pVpu->dispOutputInfo.picType, (int)chunkSize,
                (int)pVpu->dispOutputInfo.chunkReuseRequired, (int)pVpu->dispOutputInfo.frameDisplayFlag,
                (int)(pVpu->dispOutputInfo.frameDisplayFlag>>pVpu->decFbCount),
                pVpu->dispOutputInfo.rdPtr, pVpu->dispOutputInfo.wrPtr,
                (int)pVpu->dispOutputInfo.rcDisplay.left,  (int)pVpu->dispOutputInfo.rcDisplay.top,
                (int)pVpu->dispOutputInfo.rcDisplay.right,  (int)pVpu->dispOutputInfo.rcDisplay.bottom);
        }
        else
        {
            DEBUG(DEB_LEV_FULL_SEQ/*DEB_LEV_SIMPLE_SEQ*/,
                "#%d, instIdx=%d, disIdx=%d, decIdx=%d, prescanIdx=%d, picType=%d,"
                "chunkLen=%d, reuseReuired:%d, dispFlag=0x%x,RdPtr : 0x%x, WrPtr : 0x%x,"
                "display left=%d, top=%d, right=%d, bottom=%d, %dx%d\n",
                (int)pVpu->frameIdx, (int)pVpu->instIdx, (int)pVpu->dispOutputInfo.indexFrameDisplay,
                (int)pVpu->dispOutputInfo.indexFrameDecoded, (int)pVpu->dispOutputInfo.indexFramePrescan,
                (int)pVpu->dispOutputInfo.picType, (int)chunkSize,
                (int)pVpu->dispOutputInfo.chunkReuseRequired, (int)pVpu->dispOutputInfo.frameDisplayFlag,
                pVpu->dispOutputInfo.rdPtr, pVpu->dispOutputInfo.wrPtr,
                (int)pVpu->dispOutputInfo.rcDisplay.left,  (int)pVpu->dispOutputInfo.rcDisplay.top,
                (int)pVpu->dispOutputInfo.rcDisplay.right,  (int)pVpu->dispOutputInfo.rcDisplay.bottom,
                (int)pVpu->dispOutputInfo.dispPicWidth, (int)pVpu->dispOutputInfo.dispPicHeight);
        }

        DisplayDecodedInformation(pVpu->handle, pVpu->decOP.bitstreamFormat, pVpu->frameIdx, &pVpu->dispOutputInfo);

        if (pVpu->dispOutputInfo.decodingSuccess == 0)
        {
            DEBUG(DEB_LEV_ERR, "VPU_DecGetOutputInfo decode fail framdIdx %d \n", pVpu->frameIdx);
            if (pVpu->dispOutputInfo.errorReason == WAVE4_SYSERR_WATCHDOG_TIMEOUT)
            {
                DEBUG(DEB_LEV_FULL_SEQ, "watchdog timeout\n");
                if (omx_vpudec_component_HandleError(openmaxStandComp) != RETCODE_SUCCESS)
                {
                    err = OMX_ErrorHardware;
                    goto ERR_DEC;
                }
            }

            pVpu->frameIdx++;
#ifdef WORKAROUND_OMX_UNTIL_FIX_FIRMWARE
            pVpu->dispOutputInfo.indexFrameDecoded = -2;    // indexFrameDecoded already should be 2
            if (pVpu->dispOutputInfo.indexFrameDisplay > MAX_REG_FRAME)
                pVpu->dispOutputInfo.indexFrameDisplay = -3;
#endif
            if (pVpu->dispOutputInfo.consumedByte < (int)pInputBuffer->nFilledLen)
                pVpu->dispOutputInfo.consumedByte = pInputBuffer->nFilledLen; // when firmware returns an error. consumedByte has not be updated.
            //must not goto to an error because indexFrameDisplay can have valid index.
        }
        else
        {
            if (pVpu->dispOutputInfo.picType == PIC_TYPE_I || pVpu->dispOutputInfo.picType == PIC_TYPE_IDR)
            {
                pVpu->decParam.skipframeMode = 0;
            }
            pVpu->tryCount = 0;
        }

        //after decode out, we will recheck the vui info
        OmxCheckVUIParams(openmaxStandComp, omx_vpudec_component_Private, OMX_TRUE);
        OmxCheckHDRStaticMetadata(openmaxStandComp, omx_vpudec_component_Private);
        pVpu->chunkReuseRequired = pVpu->dispOutputInfo.chunkReuseRequired;

        if (pVpu->decOP.bitstreamFormat == STD_VP9)
        {
            if(pVpu->dispOutputInfo.rdPtr < pVpu->dispOutputInfo.wrPtr)
            {
                pVpu->chunkReuseRequired = 1;
                pVpu->prevConsumeBytes += pVpu->dispOutputInfo.consumedByte;
                DEBUG(DEB_LEV_SIMPLE_SEQ, "ReUseChunk required, rdptr = 0x%x, wrptr = 0x%x \n", pVpu->dispOutputInfo.rdPtr, pVpu->dispOutputInfo.wrPtr);
            }
            else
            {
                pVpu->chunkReuseRequired = 0;
                pVpu->prevConsumeBytes = 0;
            }
        }

        if (pVpu->chunkReuseRequired)
        {
            if(pVpu->decOP.bitstreamFormat == STD_MPEG4 && pVpu->dispOutputInfo.indexFrameDecoded != -1)
            {
                pVpu->prevConsumeBytes = pVpu->curConsumedByte;
                DEBUG(DEB_LEV_FULL_SEQ, "set PrevConsumedByte = %d\n", pVpu->prevConsumeBytes);
            }
        }
        else
            pVpu->prevConsumeBytes = 0;

        if (pVpu->dispOutputInfo.indexFrameDisplay == -1)
        {
            pVpu->decodefinish = 1;
            DEBUG(DEB_LEV_SIMPLE_SEQ, "decode finished\n");
        }

        if (pVpu->dispOutputInfo.indexFrameDecoded >= 0)
        {
#ifdef USE_IFRAME_SEARCH_FOR_1STFRAME
            if (omx_vpudec_component_Private->bThumbnailMode == OMX_FALSE)
                pVpu->decParam.skipframeMode = 0;
#endif

            if(pVpu->productId == PRODUCT_ID_4102 || pVpu->productId == PRODUCT_ID_420 || pVpu->productId == PRODUCT_ID_412)
                disp_flag_array[pVpu->dispOutputInfo.indexFrameDecoded].nInputTimeStamp =
                    omx_vpudec_component_Private->lastInputStamp != (OMX_TICKS)-1 ?
                    omx_vpudec_component_Private->lastInputStamp :
                    pInputBuffer->nTimeStamp;
            else
                disp_flag_array[pVpu->dispOutputInfo.indexFrameDecoded].nInputTimeStamp = pInputBuffer->nTimeStamp;

            disp_flag_array[pVpu->dispOutputInfo.indexFrameDecoded].picType = pVpu->dispOutputInfo.picType;
            disp_flag_array[pVpu->dispOutputInfo.indexFrameDecoded].rvTrB = pVpu->dispOutputInfo.rvTrB;

            if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
            {
                if (pVpu->scalerInfo.enScaler)
                    pVpu->dispOutputInfo.indexFrameDisplay = pVpu->dispOutputInfo.indexFrameDecoded + pVpu->decFbCount;
                else
                    pVpu->dispOutputInfo.indexFrameDisplay = pVpu->dispOutputInfo.indexFrameDecoded;
            }
        }

        if (pVpu->dispOutputInfo.indexFrameDecoded == -1)   // VPU did not decode a picture because there is not enough frame buffer to continue decoding
        {
            DEBUG(DEB_LEV_FULL_SEQ, "#%d, Display buffer full has been detected.\n", (int)pVpu->frameIdx);
        }

        if (pVpu->dispOutputInfo.indexFrameDecoded >= 0 && pVpu->dispOutputInfo.numOfErrMBs)
        {
            pVpu->totalNumofErrMbs += pVpu->dispOutputInfo.numOfErrMBs;
            DEBUG(DEB_LEV_ERR, "Num of Error Mbs : %d, in Frame : %d \n", pVpu->dispOutputInfo.numOfErrMBs, pVpu->frameIdx);
        }
        if (pVpu->dispOutputInfo.sequenceChanged)
        {
            // PrepareChangingSequence
            if(pVpu->productId == PRODUCT_ID_410 || pVpu->productId == PRODUCT_ID_4102  || pVpu->productId == PRODUCT_ID_412)
            {
                BOOL    bitDepthChanged, resolutionChanged, numDpbChanged;
                bitDepthChanged   = (pVpu->dispOutputInfo.sequenceChanged>>18)&0x01 ? TRUE : FALSE;
                resolutionChanged = (pVpu->dispOutputInfo.sequenceChanged>>16)&0x01 ? TRUE : FALSE;
                numDpbChanged     = (pVpu->dispOutputInfo.sequenceChanged>>19)&0x01 ? TRUE : FALSE;
                DEBUG(DEB_LEV_FULL_SEQ, "bitDepthChanged=%d, resolutionChanged=%d, numDpbChanged=%d\n", bitDepthChanged,resolutionChanged,numDpbChanged);
                if (resolutionChanged || numDpbChanged || bitDepthChanged)
                {
                    DecGetFramebufInfo prevFb;
                    int index;
                    VPU_DecGiveCommand(pVpu->handle, DEC_GET_FRAMEBUF_INFO, &prevFb);
                    VPU_DecFrameBufferFlush(pVpu->handle, NULL, NULL);

                    VPU_DecGiveCommand(pVpu->handle, DEC_RESET_FRAMEBUF_INFO, NULL);
                    VPU_DecGiveCommand(pVpu->handle, DEC_FREE_FRAME_BUFFER, NULL); // free buffers inside API.

                    // free framebuffers for the previous sequence.
                    if (prevFb.vbFrame.size > 0) {
                        vdi_free_dma_memory(pVpu->coreIdx, &prevFb.vbFrame);
                        osal_memset((void*)&prevFb.vbFrame, 0x00, sizeof(vpu_buffer_t));
                    }
                    // fix me.
                    if (pVpu->decOP.bitstreamFormat == STD_VP9) {
                        for ( index=0 ; index<MAX_REG_FRAME; index++) {
                            if(prevFb.vbMvCol[index].size > 0) {
                                vdi_free_dma_memory(pVpu->coreIdx, &prevFb.vbMvCol[index]);
                                osal_memset((void*)&prevFb.vbMvCol[index], 0x00, sizeof(vpu_buffer_t));
                            }
                            if(prevFb.vbFbcYTbl[index].size > 0) {
                                vdi_free_dma_memory(pVpu->coreIdx, &prevFb.vbFbcYTbl[index]);
                                osal_memset((void*)&prevFb.vbFbcYTbl[index], 0x00, sizeof(vpu_buffer_t));
                            }
                            if(prevFb.vbFbcCTbl[index].size > 0) {
                                vdi_free_dma_memory(pVpu->coreIdx, &prevFb.vbFbcCTbl[index]);
                                osal_memset((void*)&prevFb.vbFbcCTbl[index], 0x00, sizeof(vpu_buffer_t));
                            }
                        }
                    }
                    if (prevFb.vbWTL.size > 0) {
                        vdi_free_dma_memory(pVpu->coreIdx, &prevFb.vbWTL);
                        osal_memset((void*)&prevFb.vbWTL, 0x00, sizeof(vpu_buffer_t));
                    }

                    pVpu->dispOutputInfo.consumedByte = 0;      // to reuse current chunk data at next time

                    VPU_DecGiveCommand(pVpu->handle, DEC_GET_SEQ_INFO, &pVpu->initialInfo);
                    omx_vpudec_component_Private->bSeqChangeDetected = OMX_TRUE;
                }
            }
            else
            {
                pVpu->dispOutputInfo.consumedByte = 0;      // to reuse current chunk data at next time
                VPU_DecGiveCommand(pVpu->handle, DEC_FREE_FRAME_BUFFER, NULL); // free buffers inside API.

                VPU_DecGiveCommand(pVpu->handle, DEC_GET_SEQ_INFO, &pVpu->initialInfo);
                omx_vpudec_component_Private->bSeqChangeDetected = OMX_TRUE;
            }
        }

        if (pVpu->decOP.bitstreamFormat == STD_VP9 &&
            (pVpu->dispOutputInfo.sequenceChanged & (1<<17) ||  // inter-resolution change for VP9
            ((pVpu->dispOutputInfo.indexInterFrameDecoded != -1) && pVpu->dispOutputInfo.indexFrameDecoded == -1))) {

            int32_t     fbcCurFrameIdx = 0;
            int32_t     bwbCurFrameIdx = 0;
            int32_t     i;
            DEBUG(DEB_LEV_SIMPLE_SEQ, "----- INTER RESOLUTION CHANGED -----\n");
            fbcCurFrameIdx  = (short)(pVpu->dispOutputInfo.indexInterFrameDecoded & 0xffff);
            bwbCurFrameIdx  = (short)((pVpu->dispOutputInfo.indexInterFrameDecoded >> 16) & 0xffff);

            pVpu->interResChanged   = 1;
            pVpu->fbcCurFrameIdx    = -1;

            DEBUG(DEB_LEV_SIMPLE_SEQ, "Prev SIZE         : WIDTH(%d), HEIGHT(%d)\n", pVpu->initialInfo.picWidth, pVpu->initialInfo.picHeight);
            VPU_DecGiveCommand(pVpu->handle, DEC_SET_BWB_CUR_FRAME_IDX, (void*)&bwbCurFrameIdx);
            VPU_DecGiveCommand(pVpu->handle, DEC_SET_FBC_CUR_FRAME_IDX, (void*)&fbcCurFrameIdx);

            VPU_DecGiveCommand(pVpu->handle, DEC_GET_SEQ_INFO, &pVpu->initialInfo);
            DEBUG(DEB_LEV_SIMPLE_SEQ, "SEQUENCE NO       : %d\n", pVpu->initialInfo.sequenceNo);
            DEBUG(DEB_LEV_SIMPLE_SEQ, "FBC CUR FRAME IDX : %d\n", fbcCurFrameIdx);
            DEBUG(DEB_LEV_SIMPLE_SEQ, "BWB CUR FRAME IDX : %d\n", bwbCurFrameIdx);
            DEBUG(DEB_LEV_SIMPLE_SEQ, "SIZE              : WIDTH(%d), HEIGHT(%d)\n", pVpu->initialInfo.picWidth, pVpu->initialInfo.picHeight);

            if (pVpu->dispOutputInfo.indexInterFrameDecoded != -1)
                for (i = 0; i < pVpu->dispFbCount; i++)
                    VPU_DecClrDispFlag(pVpu->handle, i);

            if (fbcCurFrameIdx >= 0) {
                VPU_DecGiveCommand(pVpu->handle, DEC_FREE_MV_BUFFER, (void*)&fbcCurFrameIdx);
                VPU_DecGiveCommand(pVpu->handle, DEC_FREE_FBC_TABLE_BUFFER, (void*)&fbcCurFrameIdx);

                VPU_DecGiveCommand(pVpu->handle, DEC_ALLOC_MV_BUFFER, (void*)&fbcCurFrameIdx);
                VPU_DecGiveCommand(pVpu->handle, DEC_ALLOC_FBC_Y_TABLE_BUFFER, (void*)&fbcCurFrameIdx);
                VPU_DecGiveCommand(pVpu->handle, DEC_ALLOC_FBC_C_TABLE_BUFFER, (void*)&fbcCurFrameIdx);
            }

            if ((fbcCurFrameIdx >=0) || (bwbCurFrameIdx >=0)) {
                pVpu->fbcCurFrameIdx     = fbcCurFrameIdx;  // to free / re-allocate the specified framebuffer index for reference(FBC).
                pVpu->chunkReuseRequired = 1;
                pVpu->dispOutputInfo.consumedByte = 0;      // to reuse current chunk data at next time
                omx_vpudec_component_Private->bSeqChangeDetected = OMX_TRUE;
            }
            DEBUG(DEB_LEV_SIMPLE_SEQ, "-----INTER SEQUENCE CHANGE END -----\n");
        }

        if (pVpu->productId == PRODUCT_ID_4102 || pVpu->productId == PRODUCT_ID_420 || pVpu->productId == PRODUCT_ID_412)
        {
            if (pVpu->dispOutputInfo.indexFramePrescan == -1)
            {
                // if framebuffer full has been detected while prescaning, current input chunk should be reused.
                DEBUG(DEB_LEV_FULL_SEQ, "Dispaly buffer full for pre-scan\n");
                // VPU_DecSetRdPtrEx(pVpu->handle, vbStream.phys_addr, vbStream.phys_addr + pVpu->prevConsumeBytes, 0);
            }
            else if (pVpu->dispOutputInfo.indexFramePrescan == -2 && !omx_vpudec_component_Private->bSeqChangeDetected)
            {
                DEBUG(DEB_LEV_FULL_SEQ, "There is no VCL in %d chunk\n", pVpu->frameIdx); //there were only header information
                pVpu->frameIdx++;
                pInputBuffer->nFilledLen = 0;
            }
            else if (pVpu->dispOutputInfo.indexFramePrescan >= 0)
            {
                if (pVpu->decOP.bitstreamFormat == STD_VP9 && pVpu->chunkReuseRequired == 1)
                {
                    pInputBuffer->nFilledLen = pInputBuffer->nFilledLen; // super frame (many frames in one chunk)
                }
                else
                {
                    pInputBuffer->nFilledLen = 0; // pre-scanning has been sucessfully done.
                    pVpu->prevConsumeBytes = 0;
                }
            }
        }
        else
        {
#ifdef SUPPORT_ONE_BUFFER_CONTAIN_MULTIPLE_FRAMES
#define MIN_CHUNK_SIZE 6    //rdPtr can be back as many as this size because the size of one frame can't be smaller than this size.
            if (pVpu->dispOutputInfo.consumedByte < (int)(pInputBuffer->nFilledLen-MIN_CHUNK_SIZE)) // it means that InputBuffer has more chunkdata to be decoded
            {
                if (((pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuffer->nFilledLen == 0)) ||
                    (pInputBuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG))
                {
                    pInputBuffer->nFilledLen = 0;
                    pInputBuffer->nOffset = 0;
                }
                else
                {
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "#%d, remain some data that should be decoded consumedByte=%d, remainSize = %d\n",
                        (int)pVpu->frameIdx, (int)pVpu->dispOutputInfo.consumedByte, (int)((pInputBuffer->nFilledLen-pVpu->dispOutputInfo.consumedByte)));

                    pInputBuffer->nOffset += pVpu->dispOutputInfo.consumedByte;
                    pInputBuffer->nFilledLen -= pVpu->dispOutputInfo.consumedByte;
                }
            }
            else
            {
                pInputBuffer->nFilledLen = 0;
                pInputBuffer->nOffset = 0;
            }
#else
            //if seq changed, will re-use the inputput buffer
            if (pVpu->dispOutputInfo.indexFrameDecoded != -1 && !omx_vpudec_component_Private->bSeqChangeDetected)
                pInputBuffer->nFilledLen = 0;
#endif
        }

        if (pVpu->dispOutputInfo.indexFrameDecoded >= 0)
            pVpu->frameIdx++;

        ts_end = GetNowMs();
        omx_bufferheader_queue_enqueue(omx_vpudec_component_Private->in_bufferheader_queue, pInputBuffer);
        omx_vpudec_component_Private->lastInputStamp = pInputBuffer->nTimeStamp;
        DEBUG(DEB_LEV_FULL_SEQ,
            "Out %s instance[%d:%d], seqChanged:%d, Decode Done consume byte : %d,"
            "frameDisplayFlag=0x%x, Input nFilledLen: %d time=%.1fms, pts %lld\n",
            __func__, pVpu->coreIdx, pVpu->handle->instIndex, omx_vpudec_component_Private->bSeqChangeDetected,
            pVpu->dispOutputInfo.consumedByte, pVpu->dispOutputInfo.frameDisplayFlag,
            (int)pInputBuffer->nFilledLen, (ts_end - ts_start), (long long)pInputBuffer->nTimeStamp);
    }


    if(IS_STATE_FILLTHISBUFFER(pOutputBuffer) == OMX_TRUE) // pOutputBuffer has a values
    {
        if (pVpu->decodefinish)
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, EOS detected from decoder (not in input omxheader). send EOS event to ILClient without display anything\n", __func__);
            goto REACH_OUTPUT_BUFFER_EOS;
        }

        pOutputBuffer->nFilledLen = 0;

        if (pVpu->dispOutputInfo.indexFrameDisplay >= 0)
        {
            dispFlagIdx = pVpu->dispOutputInfo.indexFrameDisplay;
            DEBUG(DEB_LEV_SIMPLE_SEQ, "useNativeBuffer %d, bAllocateBuffer %d\n", omx_vpudec_component_Private->useNativeBuffer, outPort->bAllocateBuffer);
            if (omx_vpudec_component_Private->useNativeBuffer) {
                // TODO: temp workaroud, to fix by registering same fb count as actual needed
                pthread_mutex_lock(&omx_vpudec_component_Private->display_flag_mutex);
                if (disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].owner != OMX_BUFERR_OWNER_COMPONENT) {
                    DEBUG(DEB_LEV_ERR, "%s wrong owner %d, dismiss index %d\n", __func__,
                        disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].owner, pVpu->dispOutputInfo.indexFrameDisplay);
                    //wrong owner-ship of this buffer, we can not re-use thsi buffer until it is owned by component,
                    //if we clear the flag here, the error will be propagated
                    VPU_DecGiveCommand(pVpu->handle, DEC_SET_DISPLAY_FLAG, &pVpu->dispOutputInfo.indexFrameDisplay);
                    pthread_mutex_unlock(&omx_vpudec_component_Private->display_flag_mutex);
                    if (pVpu->chunkReuseRequired)
                        goto SKIP_DISPLAY;
                    goto NEXT_CHUNK;
                }
                pthread_mutex_unlock(&omx_vpudec_component_Private->display_flag_mutex);
            }
            if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
                pOutputBuffer->nFilledLen = outPort->sPortParam.nBufferSize;
            else
            {
                // this would be in case or ThumbNail decoding or not use case of AwesomeNativeWindowRenderer at stagefright.
                // do not need to copy decoded buffer to output OMX_HEADER_BUFFER in display order
                // just return nFilledLen that indicates this output is success.
                pOutputBuffer->nFilledLen = outPort->sPortParam.nBufferSize;
                if (outPort->bAllocateBuffer == OMX_FALSE) // if output buffer is not allocated by component. need to copy data
                {
                    if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
                    {
                        // update pVpu->dispOutputInfo informaton of pVpu->dispOutputInfo.indexFrameDisplay of thumbnail case.
                        if (VPU_DecGetFrameBuffer(pVpu->handle, pVpu->dispOutputInfo.indexFrameDisplay, &pVpu->dispOutputInfo.dispFrame) != RETCODE_SUCCESS)
                        {
                            DEBUG(DEB_LEV_ERR, "fail to VPU_DecGetFrameBuffer index=%d, bufY=0x%x, bufCb=0x%x, bufCr=0x%x\n", pVpu->dispOutputInfo.indexFrameDisplay, pVpu->dispOutputInfo.dispFrame.bufY, pVpu->dispOutputInfo.dispFrame.bufCb, pVpu->dispOutputInfo.dispFrame.bufCr);
                            err = OMX_ErrorHardware;
                            goto ERR_DEC;
                        }
                    }

                    if (pVpu->dispOutputInfo.dispFrame.mapType == LINEAR_FRAME_MAP) {
                        pOutputBuffer->nFilledLen = OmxCopyVpuBufferToOmxBuffer(openmaxStandComp, pOutputBuffer->pBuffer, pVpu->dispOutputInfo.dispFrame);  // if customers wants not to copy display buffer information to output buffer. this code can be removed. instead of that. customers can copy omx_usebuffer_display_info_t output buffer as bellow
                        OMX_DUMP_OUTPUT_YUV_TO_FILE(pVpu, pOutputBuffer, 0, 0, OMX_FALSE);
                    }

                    if (pOutputBuffer->pOutputPortPrivate)
                    {
                        ((omx_usebuffer_display_info_t *)pOutputBuffer->pOutputPortPrivate)->bToBeDisplay = OMX_TRUE;
                        ((omx_usebuffer_display_info_t *)pOutputBuffer->pOutputPortPrivate)->dispFlagIdx = pVpu->dispOutputInfo.indexFrameDisplay;
                    }
                }
            }
        }

        // save  dec width, height of PPU to display next decoding time.
        pVpu->rcPrevDisp = pVpu->dispOutputInfo.rcDisplay;
        pVpu->dispOutIdx++;

        DEBUG(DEB_LEV_FULL_SEQ, "Out %s Display index=%d, Decode index=%d, output nFilledLen: %d\n\n", __func__, pVpu->dispOutputInfo.indexFrameDisplay, pVpu->dispOutputInfo.indexFrameDecoded, (int)pOutputBuffer->nFilledLen);

    } // end of IS_STATE_FILLTHISBUFFER()

    pthread_mutex_unlock(&omx_vpudec_component_Private->vpu_flush_mutex);
    return;

NEXT_CHUNK:
    if (pOutputBuffer)
        pOutputBuffer->nFilledLen = 0;
    if (pInputBuffer)
        pInputBuffer->nFilledLen = 0;
    if (pOutputBuffer && pInputBuffer)
        DEBUG(DEB_LEV_SIMPLE_SEQ, "Out %s next chunk pInputBuffer->nFilledLen=%d, pOutputBuffer->nFilledLen=%d\n\n", __func__, (int)pInputBuffer->nFilledLen, (int)pOutputBuffer->nFilledLen);
    pthread_mutex_unlock(&omx_vpudec_component_Private->vpu_flush_mutex);
    return;
REACH_INPUT_BUFFER_EOS:
    if (pInputBuffer)
        pInputBuffer->nFilledLen = 0;

    if (pOutputBuffer) // for output
    {
        pOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
        pOutputBuffer->nFilledLen = 0;
    }
    pthread_mutex_unlock(&omx_vpudec_component_Private->vpu_flush_mutex);
    return;
REACH_OUTPUT_BUFFER_EOS:
    if (pOutputBuffer) // for output
    {
        pOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
        pOutputBuffer->nFilledLen = 0;
    }
    if (pInputBuffer)
    {
        pInputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
        pInputBuffer->nFilledLen = 0; // that makes calling ReturnBufferFuntion for InputBuffer that has EOS flag.
    }
    if (pOutputBuffer && pInputBuffer)
        DEBUG(DEB_LEV_SIMPLE_SEQ, "Out %s reach EOS pInputBuffer->nFilledLen=%d, pOutputBuffer->nFilledLen=%d\n\n", __func__, (int)pInputBuffer->nFilledLen, (int)pOutputBuffer->nFilledLen);
    pthread_mutex_unlock(&omx_vpudec_component_Private->vpu_flush_mutex);
    return;
SKIP_DISPLAY:
    if (pOutputBuffer)
    {
        pOutputBuffer->nFilledLen = 0;
       DEBUG(DEB_LEV_FULL_SEQ, "Out %s skip display pOutputBuffer->nFilledLen=%d\n\n", __func__, (int)pOutputBuffer->nFilledLen);
    }
    pthread_mutex_unlock(&omx_vpudec_component_Private->vpu_flush_mutex);
    return;
DEC_FRAME_FAILED:
    if (pVpu->handle)
        VPU_DecGetOutputInfo(pVpu->handle, &pVpu->dispOutputInfo);
ERR_DEC:
    pVpu->retPicRunCmd = err;
    DEBUG(DEB_LEV_ERR, "Out %s dec error 0x%x\n", __func__, pVpu->retPicRunCmd);
    if (pOutputBuffer)
        pOutputBuffer->nFilledLen = 0;
    if (pInputBuffer)
        pInputBuffer->nFilledLen = 0;
    if (omx_vpudec_component_Private->callbacks->EventHandler)
    {
        (*(omx_vpudec_component_Private->callbacks->EventHandler))
            (openmaxStandComp,
            omx_vpudec_component_Private->callbackData,
            OMX_EventError,
            err,
            0,
            NULL);
    }
    pthread_mutex_unlock(&omx_vpudec_component_Private->vpu_flush_mutex);
    return;
}


OMX_ERRORTYPE omx_vpudec_component_SetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_U32 portIndex;
    OMX_U32 paramIndex;
    /* Check which structure we are being fed and make control its header */
    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    omx_vpudec_component_PortType *port;
    if (pComponentConfigStructure == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    paramIndex = nParamIndex;
    DEBUG(DEB_LEV_FUNCTION_NAME, "Setting parameter 0x%x\n", (int)paramIndex);
    switch (paramIndex)
    {
    case OMX_IndexParamPortDefinition:
        {
            err = omx_base_component_SetParameter(hComponent, nParamIndex, pComponentConfigStructure);
            if (err == OMX_ErrorNone)
            {
                OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE*) pComponentConfigStructure;
                portIndex = pPortDef->nPortIndex;
                port = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[portIndex];

                if (pPortDef->format.video.nFrameWidth > MAX_DEC_PIC_WIDTH || pPortDef->format.video.nFrameHeight > MAX_DEC_PIC_HEIGHT)
                {
                    DEBUG(DEB_LEV_ERR, "Not Supported Video size MAX_DEC_PIC_WIDTH=%d, MAX_DEC_PIC_HEIGHT=%d\n", MAX_DEC_PIC_WIDTH, MAX_DEC_PIC_HEIGHT);
                    err = OMX_ErrorUnsupportedSetting;
                    break;
                }
                if ( pPortDef->nBufferCountActual < port->sPortParam.nBufferCountMin )
                {
                    DEBUG(DEB_LEV_ERR, "component requires at least %u buffers (%u requested)\n", port->sPortParam.nBufferCountMin, pPortDef->nBufferCountActual);
                    err = OMX_ErrorUnsupportedSetting;
                    break;
                }

                port->sPortParam.format.video.nFrameWidth   = pPortDef->format.video.nFrameWidth;
                port->sPortParam.format.video.nFrameHeight  = pPortDef->format.video.nFrameHeight;
                port->sPortParam.format.video.nStride       = pPortDef->format.video.nStride;
                port->sPortParam.format.video.nSliceHeight  = pPortDef->format.video.nSliceHeight;
                port->sPortParam.format.video.xFramerate    = pPortDef->format.video.xFramerate;
                port->sPortParam.nBufferSize                = pPortDef->nBufferSize;

                port->sVideoParam.eColorFormat = pPortDef->format.video.eColorFormat;
#ifdef ANDROID
                if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
                {
                    if(pPortDef->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX && pPortDef->eDomain == OMX_PortDomainVideo)
                    {
                        port->sVideoParam.eColorFormat =
                            (OMX_COLOR_FORMATTYPE)mapOMXColorFormat(pPortDef->format.video.eColorFormat);
                    }
                }
#endif
                port->sVideoParam.eCompressionFormat = pPortDef->format.video.eCompressionFormat;

                port->omxConfigCrop.nWidth = pPortDef->format.video.nFrameWidth;
                port->omxConfigCrop.nHeight = pPortDef->format.video.nFrameHeight;

#ifdef ANDROID
                if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX) {
                    UpdateFrameSize(openmaxStandComp);
                    port->nTempBufferCountActual = port->sPortParam.nBufferCountActual;
                }
#else
                if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX) {
                    UpdateFrameSize(openmaxStandComp);
                } else {
                    port->nTempBufferCountActual = port->sPortParam.nBufferCountActual;
                }
#endif
                DEBUG(DEB_LEV_SIMPLE_SEQ, "Setting parameter pPortDef->nBufferCountActual %d\n", pPortDef->nBufferCountActual);

                DEBUG(DEB_LEV_SIMPLE_SEQ, "Setting parameter OMX_IndexParamPortDefinition nPortIndex=%d, %dx%d, stride %d, eColorFormat=%d, sVideoParam.eColorFormat=%d, eCompressionFormat=0x%x, xFrameRate=%d\n", (int)portIndex,
                    (int)port->sPortParam.format.video.nFrameWidth, (int)port->sPortParam.format.video.nFrameHeight, (int)port->sPortParam.format.video.nStride, (int)pPortDef->format.video.eColorFormat, (int)port->sVideoParam.eColorFormat, (int)pPortDef->format.video.eCompressionFormat, (int)pPortDef->format.video.xFramerate);
            }
            break;
        }
    case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat;
            pVideoPortFormat = pComponentConfigStructure;
            portIndex = pVideoPortFormat->nPortIndex;
            /*Check Structure Header and verify component state*/
            err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoPortFormat, sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE));
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, (int)err);
                break;
            }
            port = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[portIndex];
            if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                memcpy(&port->sVideoParam, pVideoPortFormat, sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE));
                omx_vpudec_component_Private->ports[portIndex]->sPortParam.format.video.eColorFormat = pVideoPortFormat->eColorFormat;
                omx_vpudec_component_Private->ports[portIndex]->sPortParam.format.video.eCompressionFormat = pVideoPortFormat->eCompressionFormat;
                vpu_dec_context_t *pVpu = (vpu_dec_context_t *)&omx_vpudec_component_Private->vpu;
                pVpu->fbFormat = ecolor2fbformat(pVideoPortFormat->eColorFormat);
            }
            else if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
            {
                memcpy(&port->sVideoParam, pVideoPortFormat, sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE));
                omx_vpudec_component_Private->ports[portIndex]->sPortParam.format.video.eColorFormat = pVideoPortFormat->eColorFormat;
                omx_vpudec_component_Private->ports[portIndex]->sPortParam.format.video.eCompressionFormat = pVideoPortFormat->eCompressionFormat;
            }
            else
            {
                return OMX_ErrorBadPortIndex;
            }

            DEBUG(DEB_LEV_SIMPLE_SEQ, "Setting parameter OMX_IndexParamVideoPortFormat portIndex=%d, nIndex=%d, eColorFormat=0x%x, eCompressionFormat=0x%x, useNativeBuffer=%d, bThumbnailMode=%d\n", (int)portIndex, (int)pVideoPortFormat->nIndex,pVideoPortFormat->eColorFormat,
                (int)pVideoPortFormat->eCompressionFormat, (int)omx_vpudec_component_Private->useNativeBuffer, (int)omx_vpudec_component_Private->bThumbnailMode);
            break;
        }
    case OMX_IndexParamVideoAvc:
        {
            OMX_VIDEO_PARAM_AVCTYPE *pVideoAvc;
            pVideoAvc = pComponentConfigStructure;
            portIndex = pVideoAvc->nPortIndex;
            err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoAvc, sizeof (OMX_VIDEO_PARAM_AVCTYPE));
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, (int)err);
                break;
            }
            memcpy(&omx_vpudec_component_Private->codParam.avc, pVideoAvc, sizeof (OMX_VIDEO_PARAM_AVCTYPE));
            break;
        }
    case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *pVideoMpeg4;
            pVideoMpeg4 = pComponentConfigStructure;
            portIndex = pVideoMpeg4->nPortIndex;
            err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoMpeg4, sizeof (OMX_VIDEO_PARAM_MPEG4TYPE));
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, (int)err);
                break;
            }
            if (pVideoMpeg4->nPortIndex == 0)
            {
                memcpy(&omx_vpudec_component_Private->codParam.mpeg4, pVideoMpeg4, sizeof (OMX_VIDEO_PARAM_MPEG4TYPE));
            }
            else
            {
                return OMX_ErrorBadPortIndex;
            }
            break;
        }
    case OMX_IndexParamVideoMSMpeg:
        {
            OMX_VIDEO_PARAM_MSMPEGTYPE* msmpegParam = NULL;
            msmpegParam = (OMX_VIDEO_PARAM_MSMPEGTYPE*) pComponentConfigStructure;
            if (msmpegParam->eFormat != OMX_VIDEO_MSMPEGFormat3) // VPU only support msgmpeg v3
            {
                err = OMX_ErrorUnsupportedSetting;
            }
            else
            {
                memcpy(&omx_vpudec_component_Private->codParam.msmpeg, pComponentConfigStructure,
                    sizeof (OMX_VIDEO_PARAM_MSMPEGTYPE));
            }
            break;
        }
    case OMX_IndexParamVideoWmv:
        {
            OMX_VIDEO_PARAM_WMVTYPE* wmvParam = NULL;
            wmvParam = (OMX_VIDEO_PARAM_WMVTYPE*) pComponentConfigStructure;
#ifdef SUPPORT_CM_OMX_12
            if (wmvParam->eFormat != (OMX_VIDEO_WMVFORMATTYPE)OMX_VIDEO_WMVFormat9)
#else
            if (wmvParam->eFormat != (OMX_VIDEO_WMVFORMATTYPE)OMX_VIDEO_WMVFormat9 &&
                wmvParam->eFormat != (OMX_VIDEO_WMVFORMATTYPE)OMX_VIDEO_WMVFormatVC1)
#endif
            {
                err = OMX_ErrorUnsupportedSetting;
            }
            else
            {
                memcpy(&omx_vpudec_component_Private->codParam.wmv, pComponentConfigStructure,
                    sizeof (OMX_VIDEO_PARAM_WMVTYPE));
            }
            break;
        }
    case OMX_IndexParamVideoRv:
        {
            OMX_VIDEO_PARAM_RVTYPE* rvParam = NULL;
            rvParam = (OMX_VIDEO_PARAM_RVTYPE*) pComponentConfigStructure;
            if (rvParam->eFormat != OMX_VIDEO_RVFormat8 &&
                rvParam->eFormat != OMX_VIDEO_RVFormat9)
            {
                err = OMX_ErrorUnsupportedSetting;
            }
            else
            {
                memcpy(&omx_vpudec_component_Private->codParam.rv, pComponentConfigStructure,
                    sizeof (OMX_VIDEO_PARAM_RVTYPE));
            }
            break;
        }
    case OMX_IndexParamVideoMpeg2:
        {
            memcpy(&omx_vpudec_component_Private->codParam.mpeg2, pComponentConfigStructure,
                sizeof (OMX_VIDEO_PARAM_MPEG2TYPE));
            break;
        }
    case OMX_IndexParamVideoH263:
        {
            memcpy(&omx_vpudec_component_Private->codParam.h263, pComponentConfigStructure,
                sizeof (OMX_VIDEO_PARAM_H263TYPE));
            break;
        }

    case OMX_IndexParamVideoVp8:
        {
            memcpy(&omx_vpudec_component_Private->codParam.vp8, pComponentConfigStructure,
                sizeof (OMX_VIDEO_PARAM_VP8TYPE));
            break;
        }

    case OMX_IndexParamVideoAVS:
        {
            memcpy(&omx_vpudec_component_Private->codParam.avc, pComponentConfigStructure,
                sizeof (OMX_VIDEO_PARAM_AVCTYPE));
            break;
        }

    case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pComponentRole;
            pComponentRole = pComponentConfigStructure;
            if (omx_vpudec_component_Private->state != OMX_StateLoaded && omx_vpudec_component_Private->state != OMX_StateWaitForResources)
            {
                DEBUG(DEB_LEV_ERR, "In %s Incorrect State=%x lineno=%d\n", __func__, (int)omx_vpudec_component_Private->state, __LINE__);
                return OMX_ErrorIncorrectStateOperation;
            }
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone)
            {
                break;
            }
            if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_MPEG4_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingMPEG4;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_H264_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingAVC;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_HEVC_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingHEVC;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_MPEG2_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingMPEG2;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_RV_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingRV;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_WMV_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingWMV;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_H263_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingH263;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_MSMPEG_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingMSMPEG;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_AVS_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingAVS;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_VP8_ROLE) || !strcmp((char *) pComponentRole->cRole, "video_decoder.vp8"))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CODINGTYPE_VP8;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_VC1_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingVC1;
            }
            else if (!strcmp((char *) pComponentRole->cRole, VIDEO_DEC_VP9_ROLE))
            {
                omx_vpudec_component_Private->video_coding_type = OMX_VIDEO_CodingVP9;
            }
            else
            {
                return OMX_ErrorUndefined;
            }
            SetInternalVideoParameters(openmaxStandComp);
            break;
        }

    case OMX_IndexParamNalStreamFormatSelect:
        {
            OMX_NALSTREAMFORMATTYPE *pNalStreamFormat = (OMX_NALSTREAMFORMATTYPE *)pComponentConfigStructure;

            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_NALSTREAMFORMATTYPE))) != OMX_ErrorNone)
                break;

            portIndex = pNalStreamFormat->nPortIndex;

            if(portIndex != OMX_BASE_FILTER_INPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            port = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[pNalStreamFormat->nPortIndex];

            if ((pNalStreamFormat->eNaluFormat & OMX_NaluFormatStartCodes) == 0 &&
                (pNalStreamFormat->eNaluFormat & OMX_NaluFormatFourByteInterleaveLength) == 0)
            {
                DEBUG(DEB_LEV_ERR, "In %s UnsupportedSetting eNaluFormat=0x%x\n", __func__, (int)pNalStreamFormat->eNaluFormat);
                return OMX_ErrorUnsupportedSetting;
            }

            memcpy(&port->nalUFormatSelect, pNalStreamFormat, sizeof(OMX_NALSTREAMFORMATTYPE));
            omx_vpudec_component_Private->bSupportNaluFormat = OMX_TRUE;
            break;
        }
    case OMX_IndexParamCompBufferSupplier:
        {
            OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplierType = (OMX_PARAM_BUFFERSUPPLIERTYPE *)pComponentConfigStructure;
            err = checkHeader(pComponentConfigStructure, sizeof (OMX_PARAM_BUFFERSUPPLIERTYPE));
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Parameter nParamIndex:0x%x Check Error=%x\n",
                  __func__, nParamIndex, (int)err);
                break;
            }

            if (bufferSupplierType->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX &&
                bufferSupplierType->eBufferSupplier == OMX_BufferSupplyVendorDRM)
            {
              omx_vpudec_component_Private->useNativeBuffer = OMX_TRUE;
              DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_IndexParamCompBufferSupplier useNativeBuffer=%d\n", (int)omx_vpudec_component_Private->useNativeBuffer);
            }
            break;
      }
#ifdef ANDROID
    case OMX_IndexParamEnableAndroidBuffers:
        {
            err = checkEnableAndroidBuffersHeader(pComponentConfigStructure);
            if (err != OMX_ErrorNone)
                break;

            err = checkEnableAndroidBuffersPort(pComponentConfigStructure, &portIndex);
            if (err != OMX_ErrorNone)
                break;

            if(portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            port = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
            if (enableAndroidBuffer(pComponentConfigStructure))
            {
                omx_vpudec_component_Private->useNativeBuffer = OMX_TRUE;
                port->sPortParam.format.video.eColorFormat = DEFAULT_NATIVE_OUTPUT_FORMAT;
                port->sVideoParam.eColorFormat = DEFAULT_NATIVE_OUTPUT_FORMAT;
                DEBUG(DEB_LEV_SIMPLE_SEQ, " OMX_IndexParamEnableAndroidBuffers useNativeBuffer=%d, eColorFormat=0x%x\n", (int)omx_vpudec_component_Private->useNativeBuffer, (int)port->sPortParam.format.video.eColorFormat);
            }
            else
            {
                omx_vpudec_component_Private->useNativeBuffer = OMX_FALSE;
#ifdef WORKAROUND_VP8_CTS_TEST
                if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CODINGTYPE_VP8)
                {
                    port->sPortParam.format.video.eColorFormat = DEFAULT_VP8_VIDEO_OUTPUT_FORMAT;
                    port->sVideoParam.eColorFormat = DEFAULT_VP8_VIDEO_OUTPUT_FORMAT;
                }
                else
                {
                    port->sPortParam.format.video.eColorFormat = DEFAULT_VIDEO_OUTPUT_FORMAT;
                    port->sVideoParam.eColorFormat = DEFAULT_VIDEO_OUTPUT_FORMAT;
                }
#else
                port->sPortParam.format.video.eColorFormat = DEFAULT_VIDEO_OUTPUT_FORMAT;
                port->sVideoParam.eColorFormat = DEFAULT_VIDEO_OUTPUT_FORMAT;
#endif
            }
            updateOutSetting(omx_vpudec_component_Private);
            break;
        }

    case OMX_IndexParamGetAndroidNativeBuffer:
    case OMX_IndexParamAndroidNativeBufferUsage:
        {
            err = checkGetAndroidNativeBufferHeader(pComponentConfigStructure);
            if (err != OMX_ErrorNone)
                break;

            err = checkGetAndroidNativeBufferPort(pComponentConfigStructure, &portIndex);
            if (err != OMX_ErrorNone)
                break;

            if(portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            err = setAndroidNativeBufferUsage(pComponentConfigStructure, &omx_vpudec_component_Private->nUsage);

            updateOutSetting(omx_vpudec_component_Private);
            break;
        }

    case (OMX_INDEXTYPE)OMX_IndexParamUseAndroidNativeBuffer:
        {
            err = checkUseAndroidNativeBufferHeader(pComponentConfigStructure);
            if (err != OMX_ErrorNone)
                break;

            err = checkUseAndroidNativeBufferPort(pComponentConfigStructure, &portIndex);
            if (err != OMX_ErrorNone)
                break;

            if(portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            port = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[portIndex];
            DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_IndexParamUseAndroidNativeBuffer sdk %d, portIndex %d, port assigned %d\n", ANDROID_PLATFORM_SDK_VERSION, portIndex, port->nNumAssignedBuffers);

            DEBUG(DEB_LEV_SIMPLE_SEQ, "malloc pInternalBufferStorage %d\n", port->nNumAssignedBuffers);
            port->pInternalBufferStorage[port->nNumAssignedBuffers] = malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (!port->pInternalBufferStorage[port->nNumAssignedBuffers]) {
                DEBUG(DEB_LEV_ERR, "Insufficient memory in %s, line %d\n", __func__, __LINE__);
                return OMX_ErrorInsufficientResources;
            }
            memset(port->pInternalBufferStorage[port->nNumAssignedBuffers], 0x00, sizeof(OMX_BUFFERHEADERTYPE));

            int framebufSize = port->sPortParam.nBufferSize;
            if (framebufSize < 0) {
                return OMX_ErrorUndefined;
            }
            err = useAndroidNativeBuffer(pComponentConfigStructure, &(port->pInternalBufferStorage[port->nNumAssignedBuffers]), &port->sPortParam.format.video.eColorFormat, framebufSize);
            if (err != OMX_ErrorNone)
                break;

            port->pInternalBufferStorage[port->nNumAssignedBuffers]->pPlatformPrivate = port;
            port->bBufferStateAllocated[port->nNumAssignedBuffers] = BUFFER_ASSIGNED;
            port->bBufferStateAllocated[port->nNumAssignedBuffers] |= HEADER_ALLOCATED;
            omx_vpudec_component_Private->omx_display_flags[port->nNumAssignedBuffers].owner = OMX_BUFFER_OWNER_NOBODY;
            DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_IndexParamUseAndroidNativeBuffer nNumAssignedBuffers=%d, nBufferCountActual=%d, eColorFormat=0x%x, bEnabled=%d, portIndex %d\n",
                (int)port->nNumAssignedBuffers, (int)port->sPortParam.nBufferCountActual, (int)port->sPortParam.format.video.eColorFormat, port->sPortParam.bEnabled, portIndex);

            port->nNumAssignedBuffers++;
            if(port->nNumAssignedBuffers == port->sPortParam.nBufferCountActual)
            {
                DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_IndexParamUseAndroidNativeBuffer nNumAssignedBuffers=%d, bPopulated=%d\n", (int)port->nNumAssignedBuffers, (int)port->sPortParam.bPopulated);
                port->sPortParam.bPopulated = OMX_TRUE;
                port->bIsFullOfBuffers = OMX_TRUE;
                tsem_up(port->pAllocSem);
                if (omx_vpudec_component_Private->portSettingChangeRequest == OMX_TRUE)
                {
                    omx_vpudec_component_Private->portSettingChangeRequest = OMX_FALSE;
                    tsem_up(&omx_vpudec_component_Private->port_setting_change_tsem);

                }
            }

            err = OMX_ErrorNone;

            break;
        }
#ifdef SUPPORT_ADAPTIVE_PLAY
    case (OMX_INDEXTYPE)OMX_IndexParamUseAdaptivePlayback:
        {
            OMX_BOOL enable;
            OMX_U32  maxWidth;
            OMX_U32  maxHeight;
            err = checkUseAdaptivePlaybackHeader(pComponentConfigStructure);
            if (err != OMX_ErrorNone)
                break;

            err = checkUseAdaptivePlaybackPort(pComponentConfigStructure, &portIndex);
            if (err != OMX_ErrorNone)
                break;

            if(portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            err = getAdaptivePlayParams(pComponentConfigStructure, &enable, &maxWidth, &maxHeight);
            if (err != OMX_ErrorNone)
                break;

            break;
        }
#endif
#endif
    default: /*Call the base component function*/
        return omx_base_component_SetParameter(hComponent, nParamIndex, pComponentConfigStructure);
    }
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of SetParameter, with err = %d\n", err);
    return err;
}


OMX_ERRORTYPE omx_vpudec_component_GetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    omx_vpudec_component_PortType *port;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    OMX_U32 portIndex;
    OMX_U32 paramIndex;


    if (pComponentConfigStructure == NULL)
    {
        return OMX_ErrorBadParameter;
    }
    paramIndex = (OMX_U32)nParamIndex;
    DEBUG(DEB_LEV_FUNCTION_NAME, "Getting parameter 0x%x, hComponent=0x%p, omx_vpudec_component_Private=0x%p\n", (int)paramIndex, hComponent, omx_vpudec_component_Private);
    /* Check which structure we are being fed and fill its header */
    switch (paramIndex)
    {
    case OMX_IndexParamVideoInit:
        if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone)
        {
            break;
        }
        memcpy(pComponentConfigStructure, &omx_vpudec_component_Private->sPortTypesParam[OMX_PortDomainVideo], sizeof (OMX_PORT_PARAM_TYPE));
        break;

    case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat;
            OMX_U32 nIndex;

            pVideoPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pComponentConfigStructure;

            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone)
            {
                break;
            }

            portIndex = pVideoPortFormat->nPortIndex;
            nIndex = pVideoPortFormat->nIndex;
            port = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[pVideoPortFormat->nPortIndex];

            if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
            {
                pVideoPortFormat->eColorFormat = OMX_COLOR_FormatUnused;
                pVideoPortFormat->eCompressionFormat = port->sPortParam.format.video.eCompressionFormat;
            }
            else if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
            {
                pVideoPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                if (nIndex == 0)
                {
#ifdef WORKAROUND_VP8_CTS_TEST
                    if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CODINGTYPE_VP8)
                        pVideoPortFormat->eColorFormat = DEFAULT_VP8_VIDEO_OUTPUT_FORMAT;
                    else
                        pVideoPortFormat->eColorFormat = DEFAULT_VIDEO_OUTPUT_FORMAT;
#else
                    pVideoPortFormat->eColorFormat = DEFAULT_VIDEO_OUTPUT_FORMAT;
#endif

                    if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
                        pVideoPortFormat->eColorFormat = DEFAULT_NATIVE_OUTPUT_FORMAT;
                }
                else if (nIndex == 1)
                    pVideoPortFormat->eColorFormat = OMX_COLOR_FormatYUV420Planar;
                //TODO: add more color format
                // else if (nIndex == 2)
                //     pVideoPortFormat->eColorFormat = OMX_COLOR_FormatYUV422Planar;
                // else if (nIndex == 3)
                //     pVideoPortFormat->eColorFormat = OMX_COLOR_FormatYUV422SemiPlanar;
                // else if (nIndex == 4)
                //     pVideoPortFormat->eColorFormat = OMX_COLOR_FormatYCbYCr;
                // else if (nIndex == 5)
                //     pVideoPortFormat->eColorFormat = OMX_COLOR_FormatCbYCrY;
                else
                {
                    DEBUG(DEB_LEV_ERR, "Getting parameter: OMX_IndexParamVideoPortFormat OMX_ErrorNoMore\n");
                    err =  OMX_ErrorNoMore;
                }
            }
            else
            {
                DEBUG(DEB_LEV_ERR, "Getting parameter: OMX_IndexParamVideoPortFormat OMX_ErrorBadPortIndex\n");
                err = OMX_ErrorBadPortIndex;
            }

            DEBUG(DEB_LEV_SIMPLE_SEQ, "Getting parameter OMX_IndexParamVideoPortFormat nPortIndex=%d, nIndex=%d,  eColorFormat=0x%x, eCompressionFormat=0x%x, useNativeBuffer=%d, bThumbnailMode=%d\n", (int)portIndex, (int)nIndex,
                (int)pVideoPortFormat->eColorFormat, (int)pVideoPortFormat->eCompressionFormat, (int)omx_vpudec_component_Private->useNativeBuffer, (int)omx_vpudec_component_Private->bThumbnailMode);

            break;
        }
    case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE * pComponentRole;
            pComponentRole = pComponentConfigStructure;
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone)
            {
                break;
            }
            if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingMPEG4)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_MPEG4_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingAVC)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_H264_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingHEVC)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_HEVC_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingMPEG2)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_MPEG2_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingH263)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_H263_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingWMV)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_WMV_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingRV)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_RV_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingMSMPEG)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_MSMPEG_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingAVS)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_AVS_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CODINGTYPE_VP8)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_VP8_ROLE);
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingVC1)
            {
                strcpy((char *) pComponentRole->cRole, VIDEO_DEC_VC1_ROLE);
            }
            else
            {
                strcpy((char *) pComponentRole->cRole, "\0");
            }
            break;
        }
    case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *pVideoMpeg4;
            pVideoMpeg4 = pComponentConfigStructure;
            if (pVideoMpeg4->nPortIndex != 0)
            {
                return OMX_ErrorBadPortIndex;
            }
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_VIDEO_PARAM_MPEG4TYPE))) != OMX_ErrorNone)
            {
                break;
            }
            memcpy(pVideoMpeg4, &omx_vpudec_component_Private->codParam.mpeg4, sizeof (OMX_VIDEO_PARAM_MPEG4TYPE));
            break;
        }
    case OMX_IndexParamVideoAvc:
        {
            OMX_VIDEO_PARAM_AVCTYPE * pVideoAvc;
            pVideoAvc = pComponentConfigStructure;
            if (pVideoAvc->nPortIndex != 0)
            {
                return OMX_ErrorBadPortIndex;
            }
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_VIDEO_PARAM_AVCTYPE))) != OMX_ErrorNone)
            {
                break;
            }
            memcpy(pVideoAvc, &omx_vpudec_component_Private->codParam.avc, sizeof (OMX_VIDEO_PARAM_AVCTYPE));
            break;
        }
    case OMX_IndexParamVideoH263:
        {
            OMX_VIDEO_PARAM_H263TYPE * pVideoParam;
            pVideoParam = pComponentConfigStructure;
            if (pVideoParam->nPortIndex != 0)
            {
                return OMX_ErrorBadPortIndex;
            }
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_VIDEO_PARAM_H263TYPE))) != OMX_ErrorNone)
            {
                break;
            }
            memcpy(pVideoParam, &omx_vpudec_component_Private->codParam.h263, sizeof (OMX_VIDEO_PARAM_H263TYPE));
            break;
        }
    case OMX_IndexParamVideoRv:
        {
            OMX_VIDEO_PARAM_RVTYPE * pVideoParam;
            pVideoParam = pComponentConfigStructure;
            if (pVideoParam->nPortIndex != 0)
            {
                return OMX_ErrorBadPortIndex;
            }
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_VIDEO_PARAM_RVTYPE))) != OMX_ErrorNone)
            {
                break;
            }
            memcpy(pVideoParam, &omx_vpudec_component_Private->codParam.rv, sizeof (OMX_VIDEO_PARAM_RVTYPE));
            break;
        }
    case OMX_IndexParamVideoWmv:
        {
            OMX_VIDEO_PARAM_WMVTYPE * pVideoParam;
            pVideoParam = pComponentConfigStructure;
            if (pVideoParam->nPortIndex != 0)
            {
                return OMX_ErrorBadPortIndex;
            }
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_VIDEO_PARAM_WMVTYPE))) != OMX_ErrorNone)
            {
                break;
            }
            memcpy(pVideoParam, &omx_vpudec_component_Private->codParam.wmv, sizeof (OMX_VIDEO_PARAM_WMVTYPE));
            break;
        }
    case OMX_IndexParamVideoVp8:
        {
            OMX_VIDEO_PARAM_VP8TYPE * pVideoParam;
            pVideoParam = pComponentConfigStructure;
            if (pVideoParam->nPortIndex != 0)
            {
                return OMX_ErrorBadPortIndex;
            }
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_VIDEO_PARAM_VP8TYPE))) != OMX_ErrorNone)
            {
                break;
            }
            memcpy(pVideoParam, &omx_vpudec_component_Private->codParam.vp8, sizeof (OMX_VIDEO_PARAM_VP8TYPE));
            break;
        }
    case OMX_IndexParamVideoMpeg2:
        {
            OMX_VIDEO_PARAM_MPEG2TYPE * pVideoParam;
            pVideoParam = pComponentConfigStructure;
            if (pVideoParam->nPortIndex != 0)
            {
                return OMX_ErrorBadPortIndex;
            }
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_VIDEO_PARAM_MPEG2TYPE))) != OMX_ErrorNone)
            {
                break;
            }
            memcpy(pVideoParam, &omx_vpudec_component_Private->codParam.mpeg2, sizeof (OMX_VIDEO_PARAM_MPEG2TYPE));
            break;
        }
    case OMX_IndexParamVideoMSMpeg:
        {
            OMX_VIDEO_PARAM_MSMPEGTYPE * pVideoParam;
            pVideoParam = pComponentConfigStructure;
            if (pVideoParam->nPortIndex != 0)
            {
                return OMX_ErrorBadPortIndex;
            }
            if ((err = checkHeader(pComponentConfigStructure, sizeof (OMX_VIDEO_PARAM_MSMPEGTYPE))) != OMX_ErrorNone)
            {
                break;
            }
            memcpy(pVideoParam, &omx_vpudec_component_Private->codParam.msmpeg, sizeof (OMX_VIDEO_PARAM_MSMPEGTYPE));
            break;
        }

    case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *pDstProfileLevel = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*)pComponentConfigStructure;

            portIndex = pDstProfileLevel->nPortIndex;

            DEBUG(DEB_LEV_SIMPLE_SEQ, "[GetParameter]OMX_IndexParamVideoProfileLevelQuerySupported portIndex=%d, codingType=%d\n", (int)portIndex, (int)omx_vpudec_component_Private->video_coding_type);

#ifdef SUPPORT_CM_OMX_12
            DEBUG(DEB_LEV_FULL_SEQ, "nIndex=%d, video_encoding_type=0x%x\n",
                (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
#else
            DEBUG(DEB_LEV_FULL_SEQ, "nProfileIndex=%d, video_encoding_type=0x%x\n",
                (int)pDstProfileLevel->nProfileIndex, (int)omx_vpudec_component_Private->video_coding_type);
#endif

            if(portIndex != OMX_BASE_FILTER_INPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pDstProfileLevel, sizeof (OMX_VIDEO_PARAM_PROFILELEVELTYPE));
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, (int)err);
                break;
            }

            if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingAVC)
            {
#ifdef SUPPORT_CM_OMX_12
                if (pDstProfileLevel->nIndex == 0)
#else
                if (pDstProfileLevel->nProfileIndex == 0)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_AVCProfileBaseline;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_AVCLevel42;
                }
#ifdef SUPPORT_CM_OMX_12
                else if (pDstProfileLevel->nIndex == 1)
#else
                else if (pDstProfileLevel->nProfileIndex == 1)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_AVCProfileMain;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_AVCLevel42;
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
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nProfileIndex, (int)omx_vpudec_component_Private->video_coding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingMPEG4)
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
#ifdef SUPPORT_CM_OMX_12
                else if (pDstProfileLevel->nIndex == 1)
#else
                else if (pDstProfileLevel->nProfileIndex == 1)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_MPEG4ProfileAdvancedSimple;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_MPEG4Level5;
                }
                else
                {
#ifdef SUPPORT_CM_OMX_12
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nProfileIndex, (int)omx_vpudec_component_Private->video_coding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingH263)
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
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nProfileIndex, (int)omx_vpudec_component_Private->video_coding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingMPEG2)
            {
#ifdef SUPPORT_CM_OMX_12
                if (pDstProfileLevel->nIndex == 0)
#else
                if (pDstProfileLevel->nProfileIndex == 0)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_MPEG2ProfileSimple;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_MPEG2LevelHL;
                }
#ifdef SUPPORT_CM_OMX_12
                else if (pDstProfileLevel->nIndex == 1)
#else
                else if (pDstProfileLevel->nProfileIndex == 1)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_MPEG2ProfileMain;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_MPEG2LevelHL;
                }
                else
                {
#ifdef SUPPORT_CM_OMX_12
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nProfileIndex, (int)omx_vpudec_component_Private->video_coding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CODINGTYPE_VP8)
            {
#ifdef SUPPORT_CM_OMX_12
                if (pDstProfileLevel->nIndex == 0)
#else
                if (pDstProfileLevel->nProfileIndex == 0)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_VP8ProfileMain;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_VP8Level_Version3;
                }
                else
                {
#ifdef SUPPORT_CM_OMX_12
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                         (int)pDstProfileLevel->nProfileIndex, (int)omx_vpudec_component_Private->video_coding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
#ifdef SUPPORT_CM_OMX_12
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingWMV)
            {
                if (pDstProfileLevel->nIndex == 0)
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_WMVProfileSimple;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_WMVLevelL3;
                    pDstProfileLevel->eCodecType = OMX_VIDEO_WMVFormat7;

                }
                else if (pDstProfileLevel->nIndex == 1)
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_WMVProfileMain;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_WMVLevelL3;
                    pDstProfileLevel->eCodecType = OMX_VIDEO_WMVFormat8;
                }
                else if (pDstProfileLevel->nIndex == 2)
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_WMVProfileAdvanced;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_WMVLevelL3;
                    pDstProfileLevel->eCodecType = OMX_VIDEO_WMVFormat9;
                }
                else
                {
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
                    return OMX_ErrorNoMore;
                }
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingVC1)
            {
                if (pDstProfileLevel->nIndex == 0)
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_VC1ProfileSimple;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_VC1Level3;

                }
                else if (pDstProfileLevel->nIndex == 1)
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_VC1ProfileMain;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_VC1Level3;
                }
                else if (pDstProfileLevel->nIndex == 2)
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_VC1ProfileAdvanced;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_VC1Level3;
                }
                else
                {
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
                    return OMX_ErrorNoMore;
                }
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingRV)
            {
                if (pDstProfileLevel->nIndex == 0)
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_RVFormat8;

                }
                else if (pDstProfileLevel->nIndex == 1)
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_RVFormat9;

                }
                else if (pDstProfileLevel->nIndex == 2)
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_RVFormatG2;

                }
                else
                {
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
                    return OMX_ErrorNoMore;
                }
            }
#endif
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingHEVC) {
#ifdef SUPPORT_CM_OMX_12
                if (pDstProfileLevel->nIndex == 0)
#else
                if (pDstProfileLevel->nProfileIndex == 0)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_HEVCProfileMain;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_HEVCMainTierLevel5;
                }
#ifdef SUPPORT_CM_OMX_12
                else if (pDstProfileLevel->nIndex == 1)
#else
                else if (pDstProfileLevel->nProfileIndex == 1)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_HEVCProfileMain10;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_HEVCMainTierLevel5;
                }
#ifdef SUPPORT_CM_OMX_12
                else if (pDstProfileLevel->nIndex == 2)
#else
                else if (pDstProfileLevel->nProfileIndex == 2)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_HEVCProfileMain10HDR10;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_HEVCMainTierLevel5;
                }
                else
                {
#ifdef SUPPORT_CM_OMX_12
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nProfileIndex, (int)omx_vpudec_component_Private->video_coding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
            else if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingVP9) {
#ifdef SUPPORT_CM_OMX_12
                if (pDstProfileLevel->nIndex == 0)
#else
                if (pDstProfileLevel->nProfileIndex == 0)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_VP9Profile0;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_VP9Level5;
                }
#ifdef SUPPORT_CM_OMX_12
                else if (pDstProfileLevel->nIndex == 1)
#else
                else if (pDstProfileLevel->nProfileIndex == 1)
#endif
                {
                    pDstProfileLevel->eProfile = OMX_VIDEO_VP9Profile2;
                    pDstProfileLevel->eLevel   = OMX_VIDEO_VP9Level5;
                }
                else
                {
#ifdef SUPPORT_CM_OMX_12
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nIndex, (int)omx_vpudec_component_Private->video_coding_type);
#else
                    DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore nProfileIndex=%d, video_encoding_type=0x%x\n",
                        (int)pDstProfileLevel->nProfileIndex, (int)omx_vpudec_component_Private->video_coding_type);
#endif
                    return OMX_ErrorNoMore;
                }
            }
            else
            {
                DEBUG(DEB_LEV_ERR, "OMX_ErrorNoMore Unsupported coding type=0x%x\n",
                    (int)omx_vpudec_component_Private->video_coding_type);
                return OMX_ErrorNoMore;
            }

#ifdef SUPPORT_CM_OMX_12
            DEBUG(DEB_LEV_SIMPLE_SEQ, "[GetParameter]OMX_IndexParamVideoProfileLevelQuerySupported return ideo_encoding_type=0x%x, nIndex=%d, eProfile=0x%x, eLevel=0x%x\n", (int)omx_vpudec_component_Private->video_coding_type, (int)pDstProfileLevel->nIndex, (int)pDstProfileLevel->eProfile, (int)pDstProfileLevel->eLevel);
#else
            DEBUG(DEB_LEV_SIMPLE_SEQ, "[GetParameter]OMX_IndexParamVideoProfileLevelQuerySupported return ideo_encoding_type=0x%x, nProfileIndex=%d, eProfile=0x%x, eLevel=0x%x\n", (int)omx_vpudec_component_Private->video_coding_type, (int)pDstProfileLevel->nProfileIndex, (int)pDstProfileLevel->eProfile, (int)pDstProfileLevel->eLevel);
#endif

            break;
        }

        break;
    case OMX_IndexParamVideoProfileLevelCurrent:
        /* dummy for passing conformance test */
        err = OMX_ErrorNone;
        break;

#ifdef ANDROID
    case OMX_IndexParamGetAndroidNativeBuffer:
    case OMX_IndexParamAndroidNativeBufferUsage:
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s OMX_IndexParamGetAndroidNativeBuffer\n", __func__);
            err = checkGetAndroidNativeBufferHeader(pComponentConfigStructure);
            if (err != OMX_ErrorNone)
                break;

            err = checkGetAndroidNativeBufferPort(pComponentConfigStructure, &portIndex);
            if (err != OMX_ErrorNone)
                break;

            if(portIndex != OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            err = getAndroidNativeBufferUsage(pComponentConfigStructure, omx_vpudec_component_Private->nUsage);

            break;
        }
#endif
    case OMX_IndexParamPortDefinition:
        {
            err = omx_base_component_GetParameter(hComponent, nParamIndex, pComponentConfigStructure);
            if (err == OMX_ErrorNone)
            {
                OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE*) pComponentConfigStructure;
                if (pPortDef)
                {
                    omx_vpudec_component_PortType *port = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[pPortDef->nPortIndex];
                    if (pPortDef->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                    {
                        if (omx_vpudec_component_Private->portSettingChangeRequest && port->sPortParam.nBufferCountActual != port->nTempBufferCountActual)
                        {
                            // it is time to update the new nBufferCountActual value into component. because ILCient does not call SetParameter[OMX_IndexParamPortDefinition] once portSettingChangeRequest.
                            pPortDef->nBufferCountActual = port->nTempBufferCountActual;
                        }
                    }

                    DEBUG(DEB_LEV_SIMPLE_SEQ, "user-buffer %d, pPortDef->eDomain %d, OMX_PortDomainVideo %d\n",
                        omx_vpudec_component_Private->useNativeBuffer, pPortDef->eDomain, OMX_PortDomainVideo);
#ifdef ANDROID
                    if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
                    {
                        if(pPortDef->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX && pPortDef->eDomain == OMX_PortDomainVideo)
                        {
                            pPortDef->format.video.eColorFormat =
                                (OMX_COLOR_FORMATTYPE)mapAndroidPixelFormat(port->sVideoParam.eColorFormat);
                        }
                    }
#endif
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "Getting parameter OMX_IndexParamPortDefinition portIndex=%d, %dx%d, stride %d, eColorFormat=%x-%x, eCompressionFormat=0x%x, xFramerate=%d, nBufferCountActual=%d, nTempBufferCountActual=%d, portSettingChangeRequest=%d\n", (int)pPortDef->nPortIndex,
                        (int)pPortDef->format.video.nFrameWidth, (int)pPortDef->format.video.nFrameHeight, (int)pPortDef->format.video.nStride, port->sVideoParam.eColorFormat, (int)pPortDef->format.video.eColorFormat, (int)pPortDef->format.video.eCompressionFormat, (int)pPortDef->format.video.xFramerate, (int)pPortDef->nBufferCountActual, (int)port->nTempBufferCountActual, (int)omx_vpudec_component_Private->portSettingChangeRequest);
                }
            }

           break;
        }

    case OMX_IndexParamNalStreamFormatSupported:
        {
            OMX_NALSTREAMFORMATTYPE *pNalStreamFormat = (OMX_NALSTREAMFORMATTYPE*)pComponentConfigStructure;

            portIndex = pNalStreamFormat->nPortIndex;

            if(portIndex != OMX_BASE_FILTER_INPUTPORT_INDEX)
                return OMX_ErrorBadPortIndex;

            err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pNalStreamFormat, sizeof (OMX_NALSTREAMFORMATTYPE));
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, (int)err);
                break;
            }
#ifdef SUPPORT_NALU_FORMAT_BY_OMX
            pNalStreamFormat->eNaluFormat = (OMX_NALUFORMATSTYPE)(OMX_NaluFormatStartCodes | OMX_NaluFormatFourByteInterleaveLength);
#else
            pNalStreamFormat->eNaluFormat = (OMX_NALUFORMATSTYPE)(OMX_NaluFormatStartCodes);
#endif

            DEBUG(DEB_LEV_SIMPLE_SEQ, "[GetParameter]OMX_IndexParamNalStreamFormatSupported video_encoding_type=0x%x, nIndex=%d, eNaluFormat=0x%x\n", (int)omx_vpudec_component_Private->video_coding_type, (int)pNalStreamFormat->nPortIndex, (int)pNalStreamFormat->eNaluFormat);

            break;
        }

    default: /*Call the base component function*/
        DEBUG(DEB_LEV_FUNCTION_NAME, "get parameter %x, %d\n", nParamIndex, err);
        return omx_base_component_GetParameter(hComponent, nParamIndex, pComponentConfigStructure);
    }
    DEBUG(DEB_LEV_FUNCTION_NAME, "Finish get parameter %x, %d\n", nParamIndex, err);
    return err;
}


OMX_ERRORTYPE omx_vpudec_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = (omx_vpudec_component_PrivateType*) openmaxStandComp->pComponentPrivate;
    OMX_ERRORTYPE err;
    OMX_STATETYPE eCurrentState = omx_vpudec_component_Private->state;

    DEBUG(DEB_LEV_FULL_SEQ, "In %s messageType=%d, messageParam=%d, eCurrentState=%d\n", __func__, message->messageType, message->messageParam, (int)eCurrentState);


    if (message->messageType == OMX_CommandStateSet)
    {
        if ((message->messageParam == OMX_StateIdle) && (eCurrentState == OMX_StateLoaded))
        {
            err = omx_vpudec_component_Init(openmaxStandComp);
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Video Decoder Init Failed Error=%x\n", __func__, (int)err);
                return err;
            }
        }

        if (((message->messageParam == OMX_StateIdle) && (eCurrentState == OMX_StateExecuting)) ||
            ((message->messageParam == OMX_StatePause) && (eCurrentState == OMX_StateExecuting)))
        {
            tsem_up(&omx_vpudec_component_Private->disp_Buf_full_tsem);
        }
    }

    if (message->messageType == OMX_CommandPortEnable)
    {
        DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_CommandPortEnable !\n");
    }

    if ((message->messageType == OMX_CommandPortDisable) ||
        (message->messageType == OMX_CommandFlush)) //output buffer flush : flush the display buffer
    {

        if(message->messageType == OMX_CommandPortDisable)
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_CommandPortDisable port=%d\n", message->messageParam);
            if(eCurrentState != OMX_StateLoaded)
                tsem_up(&omx_vpudec_component_Private->disp_Buf_full_tsem);
        }
        else
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_CommandFlush port=%d\n", message->messageParam);
            tsem_up(&omx_vpudec_component_Private->disp_Buf_full_tsem);
        }
    }

    if (message->messageType == OMX_CommandStateSet)
    {
        if ((message->messageParam == OMX_StateLoaded) && (eCurrentState == OMX_StateIdle))
        {
            err = omx_vpudec_component_Deinit(openmaxStandComp);
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR, "In %s Video Decoder Deinit Failed Error=%x\n", __func__, (int)err);
                return err;
            }
        }
    }

    // Execute the base message handling
    err = omx_base_component_MessageHandler(openmaxStandComp, message);

    return err;
}


OMX_ERRORTYPE omx_vpudec_component_ComponentRoleEnum(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_U8 *cRole,
    OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    DEBUG(DEB_LEV_SIMPLE_SEQ, "%s:%d Index(%d)\n", __func__, __LINE__, (int)nIndex);

    UNREFERENCED_PARAMETER(hComponent);

    switch (nIndex)
    {
    case 0:
        strcpy((char*) cRole, VIDEO_DEC_H264_NAME);
        break;
    case 1:
        strcpy((char*) cRole, VIDEO_DEC_MPEG2_NAME);
        break;
    case 2:
        strcpy((char*) cRole, VIDEO_DEC_MPEG4_NAME);
        break;
    case 3:
        strcpy((char*) cRole, VIDEO_DEC_RV_NAME);
        break;
    case 4:
        strcpy((char*) cRole, VIDEO_DEC_WMV_NAME);
        break;
    case 5:
        strcpy((char*) cRole, VIDEO_DEC_H263_NAME);
        break;
    case 6:
        strcpy((char*) cRole, VIDEO_DEC_MSMPEG_NAME);
        break;
    case 7:
        strcpy((char*) cRole, VIDEO_DEC_AVS_ROLE);
        break;
    case 8:
        strcpy((char*) cRole, VIDEO_DEC_VP8_ROLE);
        break;
    case 9:
        strcpy((char*) cRole, VIDEO_DEC_THO_ROLE);
        break;
    case 10:
        strcpy((char*) cRole, VIDEO_DEC_JPG_ROLE);
        break;
    case 11:
        strcpy((char*) cRole, VIDEO_DEC_VC1_ROLE);
        break;
    default:
        err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}


OMX_ERRORTYPE omx_vpudec_component_SetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure)
{
    OMX_U32 portIndex;
    /* Check which structure we are being fed and make control its header */
    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    // Possible configs to set
    OMX_CONFIG_RECTTYPE *omxConfigCrop;
    OMX_CONFIG_ROTATIONTYPE *omxConfigRotate;
    OMX_CONFIG_MIRRORTYPE *omxConfigMirror;
    OMX_CONFIG_SCALEFACTORTYPE *omxConfigScale;
    OMX_CONFIG_POINTTYPE *omxConfigOutputPosition;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    omx_vpudec_component_PortType *pPort;
    OMX_U32 paramIndex;

    if (pComponentConfigStructure == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting configuration 0x%08x\n", nParamIndex);

    paramIndex = (OMX_U32)nParamIndex;

    switch (paramIndex) {
    case OMX_IndexConfigCommonInputCrop:
    case OMX_IndexConfigCommonOutputCrop:
        omxConfigCrop = (OMX_CONFIG_RECTTYPE*)pComponentConfigStructure;
        portIndex = omxConfigCrop->nPortIndex;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_RECTTYPE))) != OMX_ErrorNone) {
            break;
        }
        if ( (paramIndex == OMX_IndexConfigCommonOutputCrop && portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)  ||
            (paramIndex == OMX_IndexConfigCommonInputCrop && portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX) ) {
                pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[portIndex];
                pPort->omxConfigCrop.nLeft = omxConfigCrop->nLeft;
                pPort->omxConfigCrop.nTop = omxConfigCrop->nTop;
                pPort->omxConfigCrop.nWidth = omxConfigCrop->nWidth;
                pPort->omxConfigCrop.nHeight = omxConfigCrop->nHeight;

                DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting configuration OMX_IndexConfigCommonOutputCrop nLeft=%d, nTop=%d, nWidth=%d, nHeight=%d\n",
                    (int)omxConfigCrop->nLeft, (int)omxConfigCrop->nTop, (int)omxConfigCrop->nWidth, (int)omxConfigCrop->nHeight);
        } else if (portIndex <= 1) {
            return OMX_ErrorUnsupportedIndex;
        } else {
            return OMX_ErrorBadPortIndex;
        }
        break;
    case OMX_IndexConfigCommonRotate:
        omxConfigRotate = (OMX_CONFIG_ROTATIONTYPE*)pComponentConfigStructure;
        portIndex = omxConfigRotate->nPortIndex;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_ROTATIONTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (portIndex <= 1) {
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[portIndex];
            if (omxConfigRotate->nRotation != 0) {
                //  Rotation not supported (yet)
                return OMX_ErrorUnsupportedSetting;
            }
            pPort->omxConfigRotate.nRotation = omxConfigRotate->nRotation;
        } else {
            return OMX_ErrorBadPortIndex;
        }
        break;
    case OMX_IndexConfigCommonMirror:
        omxConfigMirror = (OMX_CONFIG_MIRRORTYPE*)pComponentConfigStructure;
        portIndex = omxConfigMirror->nPortIndex;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_MIRRORTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (portIndex <= 1) {
            if (omxConfigMirror->eMirror == OMX_MirrorBoth || omxConfigMirror->eMirror == OMX_MirrorHorizontal)  {
                //  Horizontal mirroring not yet supported
                return OMX_ErrorUnsupportedSetting;
            }
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[portIndex];
            pPort->omxConfigMirror.eMirror = omxConfigMirror->eMirror;
        } else {
            return OMX_ErrorBadPortIndex;
        }
        break;
    case OMX_IndexConfigCommonScale:
        omxConfigScale = (OMX_CONFIG_SCALEFACTORTYPE*)pComponentConfigStructure;
        portIndex = omxConfigScale->nPortIndex;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_SCALEFACTORTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (portIndex <= 1) {
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[portIndex];
            pPort->omxConfigScale.xWidth = omxConfigScale->xWidth;
            pPort->omxConfigScale.xHeight = omxConfigScale->xHeight;
        } else {
            return OMX_ErrorBadPortIndex;
        }
        break;
    case OMX_IndexConfigCommonOutputPosition:
        omxConfigOutputPosition = (OMX_CONFIG_POINTTYPE*)pComponentConfigStructure;
        portIndex = omxConfigOutputPosition->nPortIndex;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_POINTTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (portIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX) {
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[portIndex];
            pPort->omxConfigOutputPosition.nX = omxConfigOutputPosition->nX;
            pPort->omxConfigOutputPosition.nY = omxConfigOutputPosition->nY;
        } else if (portIndex == OMX_BASE_FILTER_INPUTPORT_INDEX) {
            return OMX_ErrorUnsupportedIndex;
        } else {
            return OMX_ErrorBadPortIndex;
        }
        break;

    case OMX_IndexConfigThumbnailMode:
        {
            OMX_BOOL * pThumbNailEnable;
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];

            pThumbNailEnable = (OMX_BOOL *)pComponentConfigStructure;

            if (*pThumbNailEnable == OMX_TRUE)
                omx_vpudec_component_Private->bThumbnailMode = OMX_TRUE;
            else
                omx_vpudec_component_Private->bThumbnailMode = OMX_FALSE;

            omx_vpudec_component_Private->useNativeBuffer = OMX_FALSE;

            DEBUG(DEB_LEV_SIMPLE_SEQ, " OMX_IndexParamThumbnailMode useNativeBuffer=%d, bThumbnailMode=%d, *pThumbNailEnable=%d, eColorFormat=0x%x\n",
                (int)omx_vpudec_component_Private->useNativeBuffer, (int)omx_vpudec_component_Private->bThumbnailMode, (int)*pThumbNailEnable, (int)pPort->sPortParam.format.video.eColorFormat);

            break;
        }
#ifdef ANDROID
    case OMX_IndexParamDescribeColorAspects:
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "into OMX_IndexParamDescribeColorAspects\n");
            // init the default color aspect, this value is setted by framework
            err = getColorAspectFromParams(&omx_vpudec_component_Private->mDefaultColorAspects, pComponentConfigStructure);
            handleColorAspectsChange(omx_vpudec_component_Private->video_coding_type,
                &omx_vpudec_component_Private->mDefaultColorAspects, &omx_vpudec_component_Private->mBitstreamColorAspects,
                &omx_vpudec_component_Private->mFinalColorAspects);
            break;
        }
    case OMX_IndexParamDescribeHDRStaticInfo:
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "into OMX_IndexParamDescribeHDRStaticInfo\n");
            // get the hdr info from framework
            err = getHdrStaticInfoFromParams(&omx_vpudec_component_Private->mHdrStaticInfo, pComponentConfigStructure);
            break;
        }
#endif
    default: // delegate to superclass
        return omx_base_component_SetConfig(hComponent, paramIndex, pComponentConfigStructure);
    }
    return err;
}


OMX_ERRORTYPE omx_vpudec_component_GetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nParamIndex,
    OMX_IN OMX_PTR pComponentConfigStructure)
{

    // Possible configs to ask for
    OMX_PARAM_COMPONENTROLETYPE *pComponentRole;
    OMX_CONFIG_RECTTYPE *omxConfigCrop;
    OMX_CONFIG_ROTATIONTYPE *omxConfigRotate;
    OMX_CONFIG_MIRRORTYPE *omxConfigMirror;
    OMX_CONFIG_SCALEFACTORTYPE *omxConfigScale;
    OMX_CONFIG_POINTTYPE *omxConfigOutputPosition;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    omx_vpudec_component_PortType *pPort;
    if (pComponentConfigStructure == NULL) {
        return OMX_ErrorBadParameter;
    }
    DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting configuration 0x%x\n", nParamIndex);
    /* Check which structure we are being fed and fill its header */
    switch ((OMX_U32)nParamIndex) {
    case OMX_IndexParamStandardComponentRole:
        pComponentRole = (OMX_PARAM_COMPONENTROLETYPE *)pComponentConfigStructure;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone) {
            break;
        }

        err = GetRoleByVideoCodingType(omx_vpudec_component_Private->video_coding_type, pComponentRole->cRole, OMX_FALSE);
        break;
    case OMX_IndexConfigCommonInputCrop:
        omxConfigCrop = (OMX_CONFIG_RECTTYPE*)pComponentConfigStructure;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_RECTTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (omxConfigCrop->nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX) {
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[omxConfigCrop->nPortIndex];
            memcpy(omxConfigCrop, &pPort->omxConfigCrop, sizeof(OMX_CONFIG_RECTTYPE));
        } else if (omxConfigCrop->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX) {
            return OMX_ErrorUnsupportedIndex;
        } else {
            return OMX_ErrorBadPortIndex;
        }

        DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting configuration OMX_IndexConfigCommonInputCrop nLeft=%d, nTop=%d, nWidth=%d, nHeight=%d\n",
            (int)omxConfigCrop->nLeft, (int)omxConfigCrop->nTop, (int)omxConfigCrop->nWidth, (int)omxConfigCrop->nHeight);

        break;
    case OMX_IndexConfigCommonOutputCrop:
        omxConfigCrop = (OMX_CONFIG_RECTTYPE*)pComponentConfigStructure;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_RECTTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (omxConfigCrop->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX) {
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[omxConfigCrop->nPortIndex];
            memcpy(omxConfigCrop, &pPort->omxConfigCrop, sizeof(OMX_CONFIG_RECTTYPE));
        } else if (omxConfigCrop->nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX) {
            return OMX_ErrorUnsupportedIndex;
        } else {
            return OMX_ErrorBadPortIndex;
        }

        DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting configuration OMX_IndexConfigCommonOutputCrop nLeft=%d, nTop=%d, nWidth=%d, nHeight=%d\n",
            (int)omxConfigCrop->nLeft, (int)omxConfigCrop->nTop, (int)omxConfigCrop->nWidth, (int)omxConfigCrop->nHeight);
        break;
    case OMX_IndexConfigCommonRotate:
        omxConfigRotate = (OMX_CONFIG_ROTATIONTYPE*)pComponentConfigStructure;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_ROTATIONTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (omxConfigRotate->nPortIndex <= 1) {
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[omxConfigRotate->nPortIndex];
            memcpy(omxConfigRotate, &pPort->omxConfigRotate, sizeof(OMX_CONFIG_ROTATIONTYPE));
        } else {
            return OMX_ErrorBadPortIndex;
        }
        break;
    case OMX_IndexConfigCommonMirror:
        omxConfigMirror = (OMX_CONFIG_MIRRORTYPE*)pComponentConfigStructure;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_MIRRORTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (omxConfigMirror->nPortIndex <= 1) {
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[omxConfigMirror->nPortIndex];
            memcpy(omxConfigMirror, &pPort->omxConfigMirror, sizeof(OMX_CONFIG_MIRRORTYPE));
        } else {
            return OMX_ErrorBadPortIndex;
        }
        break;
    case OMX_IndexConfigCommonScale:
        omxConfigScale = (OMX_CONFIG_SCALEFACTORTYPE*)pComponentConfigStructure;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_SCALEFACTORTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (omxConfigScale->nPortIndex <= 1) {
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[omxConfigScale->nPortIndex];
            memcpy(omxConfigScale, &pPort->omxConfigScale, sizeof(OMX_CONFIG_SCALEFACTORTYPE));
        } else {
            return OMX_ErrorBadPortIndex;
        }
        break;
    case OMX_IndexConfigCommonOutputPosition:
        omxConfigOutputPosition = (OMX_CONFIG_POINTTYPE*)pComponentConfigStructure;
        if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_POINTTYPE))) != OMX_ErrorNone) {
            break;
        }
        if (omxConfigOutputPosition->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX) {
            pPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[omxConfigOutputPosition->nPortIndex];
            memcpy(omxConfigOutputPosition, &pPort->omxConfigOutputPosition, sizeof(OMX_CONFIG_POINTTYPE));
        } else if (omxConfigOutputPosition->nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX) {
            return OMX_ErrorUnsupportedIndex;
        } else {
            return OMX_ErrorBadPortIndex;
        }
        break;
#ifdef ANDROID
    case OMX_IndexParamDescribeColorAspects:
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_IndexParamDescribeColorAspects\n");
            // get the final color aspect, this value is updated by component
            err = setColorAspectToParams(&omx_vpudec_component_Private->mFinalColorAspects, pComponentConfigStructure);
            break;
        }
    case OMX_IndexParamDescribeHDRStaticInfo:
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_IndexParamDescribeHDRStaticInfo\n");
            // get the hdr static info
            err = setHdrStaticInfoToParams(&omx_vpudec_component_Private->mHdrStaticInfo, pComponentConfigStructure);
            break;
        }
#endif
    default: // delegate to superclass
        DEBUG(DEB_LEV_SIMPLE_SEQ, "unknown index\n");
        // return omx_base_component_GetConfig(hComponent, nParamIndex, pComponentConfigStructure);
        return OMX_ErrorUnsupportedIndex;
    }
    return err;
}


OMX_ERRORTYPE omx_vpudec_component_GetExtensionIndex(
    OMX_HANDLETYPE hComponent,
    OMX_STRING cParameterName,
    OMX_INDEXTYPE* pIndexType)
{
        OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
        omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;

        DEBUG(DEB_LEV_FUNCTION_NAME,"In  %s, cParameterName = %s \n",__func__, cParameterName);
#ifdef ANDROID
        if(strcmp(cParameterName, STR_INDEX_PARAM_ENABLE_ANDROID_NATIVE_BUFFER) == 0)
        {
            *pIndexType = (OMX_INDEXTYPE)OMX_IndexParamEnableAndroidBuffers;
            return OMX_ErrorNone;
        }
        else if (strcmp(cParameterName, STR_INDEX_PARAM_GET_ANDROID_NATIVE_BUFFER) == 0)
        {
            *pIndexType = (OMX_INDEXTYPE) OMX_IndexParamGetAndroidNativeBuffer;
            return OMX_ErrorNone;
        }
        else if (strcmp(cParameterName, STR_INDEX_PARAM_ANDROID_NATIVE_BUFFER_USAGE) == 0)
        {
            *pIndexType = (OMX_INDEXTYPE) OMX_IndexParamAndroidNativeBufferUsage;
            return OMX_ErrorNone;
        }
        else if (strcmp(cParameterName, STR_INDEX_PARAM_USE_ANDROID_NATIVE_BUFFER) == 0)
        {
            *pIndexType = (OMX_INDEXTYPE) OMX_IndexParamUseAndroidNativeBuffer;
            return OMX_ErrorNone;
        }
        else if (strcmp(cParameterName, STR_INDEX_PARAM_THUMBNAIL_MODE) == 0)
        {
            *pIndexType = (OMX_INDEXTYPE) OMX_IndexConfigThumbnailMode;
            return OMX_ErrorNone;
        }
        else if (strcmp(cParameterName, STR_INDEX_PARAM_ADAPTIVE_PLAYBACK) == 0)
        {
            *pIndexType = (OMX_INDEXTYPE) OMX_IndexParamUseAdaptivePlayback;
            return OMX_ErrorNone;
        }
        else if (strcmp(cParameterName, STR_INDEX_PARAM_DESCRIBE_COLOR_ASPECT) == 0)
        {
            if (supportsDescribeColorAspects(omx_vpudec_component_Private->video_coding_type)) {
                *pIndexType = (OMX_INDEXTYPE) OMX_IndexParamDescribeColorAspects;
                DEBUG(DEB_LEV_SIMPLE_SEQ, "IN %s, OMX_IndexParamDescribeColorAspects with index %x", __func__, *pIndexType);
                return OMX_ErrorNone;
            }
        }
        else if (strcmp(cParameterName, STR_INDEX_PARAM_DESCRIBE_HDR_STATIC_INFO) == 0)
        {
            if (supportDescribeHdrStaticInfo(omx_vpudec_component_Private->video_coding_type)) {
                *pIndexType = (OMX_INDEXTYPE) OMX_IndexParamDescribeHDRStaticInfo;
                DEBUG(DEB_LEV_SIMPLE_SEQ, "IN %s, OMX_IndexParamDescribeHDRStaticInfo with index %x", __func__, *pIndexType);
                return OMX_ErrorNone;
            }
        }
#else
        if (strcmp(cParameterName, STR_INDEX_PARAM_THUMBNAIL_MODE) == 0)
        {
            *pIndexType = (OMX_INDEXTYPE) OMX_IndexConfigThumbnailMode;
            return OMX_ErrorNone;
        }
#endif

    return omx_base_component_GetExtensionIndex(hComponent, cParameterName, pIndexType);
}


int codingTypeToCodStd(OMX_VIDEO_CODINGTYPE codingType)
{
    int codStd = -1;
    switch ((int)codingType)
    {
    case OMX_VIDEO_CodingMPEG2:
        codStd = STD_MPEG2;
        break;
    case OMX_VIDEO_CodingH263:
        codStd = STD_H263;
        break;
    case OMX_VIDEO_CodingMPEG4:
        codStd = STD_MPEG4;
        break;
    case OMX_VIDEO_CodingWMV:
        codStd = STD_VC1;
        break;
    case OMX_VIDEO_CodingRV:
        codStd = STD_RV;
        break;
    case OMX_VIDEO_CodingMSMPEG:
        codStd = STD_DIV3;
        break;
    case OMX_VIDEO_CODINGTYPE_VP8:
        codStd = STD_VP8;
        break;
    case OMX_VIDEO_CodingAVC:
        codStd = STD_AVC;
        break;
    case OMX_VIDEO_CodingHEVC:
        codStd = STD_HEVC;
        break;
    case OMX_VIDEO_CodingAVS:
        codStd = STD_AVS;
        break;
    case OMX_VIDEO_CodingVC1:
        codStd = STD_VC1;
        break;
    case OMX_VIDEO_CodingVP9:
        codStd = STD_VP9;
        break;
    default:
        codStd = -1;
        break;
    }

    return codStd;

}


int codingTypeToMp4class(OMX_VIDEO_CODINGTYPE codingType, int fourCC)
{
    int mp4Class = 0;
    if (codingType == OMX_VIDEO_CodingMPEG4)
    {
        if (fourCC == MAKE_FOURCC('D', 'I', 'V', 'X') || fourCC == MAKE_FOURCC('D', 'I', 'V', '4'))
            mp4Class = 5;
        else if (fourCC == MAKE_FOURCC('D', 'X', '5', '0')
            || fourCC == MAKE_FOURCC('D', 'I', 'V', '5')
            || fourCC == MAKE_FOURCC('D', 'I', 'V', '6'))
            mp4Class = 1;
        else if (fourCC == MAKE_FOURCC('X', 'V', 'I', 'D'))
            mp4Class = 2;
        else
            mp4Class = 8; // (8 means that firmware try to find mp4class by searching user-data)
    }
    return mp4Class;
}

int BuildOmxSeqHeader(OMX_U8 *pbHeader, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_COMPONENTTYPE *openmaxStandComp, int* sizelength)
{
    omx_vpudec_component_PrivateType* privateType = openmaxStandComp->pComponentPrivate;
    omx_vpudec_component_PortType *inPort = (omx_vpudec_component_PortType *) privateType->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
#ifdef SUPPORT_ONE_BUFFER_CONTAIN_MULTIPLE_FRAMES
    OMX_U8 *pbMetaData = (pInputBuffer->pBuffer+pInputBuffer->nOffset);
#else
    OMX_U8 *pbMetaData = pInputBuffer->pBuffer;
#endif
    int nMetaData = pInputBuffer->nFilledLen;
    OMX_U8* p = pbMetaData;
    OMX_U8 *a = p + 4 - ((OMX_S32) p & 3);
    OMX_U8* t = pbHeader;
    OMX_S32 size; // metadata header size
#ifdef OMX_INPUT_BUFFER_FEEDED_BY_FFMPEG_AVPACKET
    OMX_U32 profile;
#endif
    OMX_U32 codingType;
    int i;
    codingType = privateType->video_coding_type;
    size = 0;
    if (sizelength)
        *sizelength = 4; //default size length (in bytes) = 4

    if (privateType->bUseOmxInputBufferAsDecBsBuffer == OMX_TRUE)
        return 0;

    DEBUG(DEB_LEV_FULL_SEQ, "In %s, codingType=%d, buffer_len=%d\n", __func__, (int)codingType, (int)pInputBuffer->nFilledLen);

    if (codingType == OMX_VIDEO_CodingAVC || codingType == OMX_VIDEO_CodingAVS)
    {
        if (nMetaData > 1 && pbMetaData && pbMetaData[0] == 0x01)// check mov/mo4 file format stream
        {
            int sps, pps, nal;
            p += 4;
            if (sizelength)
                *sizelength = (*p++ & 0x3) + 1;
            sps = (*p & 0x1f); // Number of sps
            p++;
            for (i = 0; i < sps; i++)
            {
                nal = (*p << 8) + *(p + 1) + 2;
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x01);
                PUT_BUFFER(t, p + 2, nal);
                p += nal;
                size += (nal + 4); // 4 => length of start code to be inserted
            }
            pps = *(p++); // number of pps
            for (i = 0; i < pps; i++)
            {
                nal = (*p << 8) + *(p + 1) + 2;
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x00);
                PUT_BYTE(t, 0x01);
                PUT_BUFFER(t, p + 2, nal);
                p += nal;
                size += (nal + 4); // 4 => length of start code to be inserted
            }
            privateType->bSizeLengthDetected = OMX_TRUE;
        }
        else
        {
            size = 0;
            for (; p < a; p++)
            {
                if (p[0] == 0 && p[1] == 0 && p[2] == 1) // find startcode
                {
                    size = 0;
                    //PUT_BUFFER(pbHeader, pInputBuffer->pBuffer, pInputBuffer->nFilledLen);
                    break;
                }
            }
            privateType->bSizeLengthDetected = OMX_FALSE;
        }
    }
    else if (codingType == OMX_VIDEO_CodingWMV)
    {
#ifdef OMX_INPUT_BUFFER_FEEDED_BY_FFMPEG_AVPACKET
        profile = privateType->codParam.wmv.eFormat;
#ifdef SUPPORT_CM_OMX_12
        if ((profile != (int) OMX_VIDEO_WMVFormat7)
            && (profile != (int) OMX_VIDEO_WMVFormat8)
            && (profile != (int) OMX_VIDEO_WMVFormat9)) // VC1 AP has the start code.
#else
        if (profile == (int) OMX_VIDEO_WMVFormatVC1) // VC1 AP has the start code.
#endif
        {
            //if there is no seq startcode in pbMetatData. VPU will be failed at seq_init stage.
            size = nMetaData;
            PUT_BUFFER(pbHeader, pbMetaData, size);
        }
        else
        {
#define RCV_V2
#ifdef RCV_V2
            PUT_LE32(pbHeader, ((0xC5 << 24) | 0));
            size += 4; //version
            PUT_LE32(pbHeader, nMetaData);
            size += 4;
            PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
            size += nMetaData;
            PUT_LE32(pbHeader, inPort->sPortParam.format.video.nFrameHeight);
            size += 4;
            PUT_LE32(pbHeader, inPort->sPortParam.format.video.nFrameWidth);
            size += 4;
            PUT_LE32(pbHeader, 12);
            size += 4;
            PUT_LE32(pbHeader, (2<<29 | 1<<28 | 0x80<<24 | 1<<0));
            size += 4; // STRUCT_B_FRIST (LEVEL:3|CBR:1:RESERVE:4:HRD_BUFFER|24)
            PUT_LE32(pbHeader, inPort->sPortParam.format.video.nBitrate);
            size += 4; // hrd_rate
            PUT_LE32(pbHeader, inPort->sPortParam.format.video.xFramerate);
            size += 4; // framerate
#else   //RCV_V1
            PUT_LE32(pbHeader, ((0x85 << 24) | 0x00));
            size += 4; //frames count will be here
            PUT_LE32(pbHeader, nMetaData);
            size += 4;
            PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
            size += nMetaData;
            PUT_LE32(pbHeader, inPort->sPortParam.format.video.nFrameHeight);
            size += 4;
            PUT_LE32(pbHeader, inPort->sPortParam.format.video.nFrameWidth);
            size += 4;
#endif
        }
#endif
    }
    else if (codingType == OMX_VIDEO_CodingRV)
    {
#ifdef OMX_INPUT_BUFFER_FEEDED_BY_FFMPEG_AVPACKET
        profile = privateType->codParam.rv.eFormat;

        DEBUG(DEB_LEV_FULL_SEQ, "In %s, profile=%d\n", __func__, profile);
        size = 26 + nMetaData;
        PUT_BE32(pbHeader, size); //Length
        PUT_LE32(pbHeader, MAKE_FOURCC('V', 'I', 'D', 'O')); //MOFTag
        if (profile == (int) OMX_VIDEO_RVFormat8)
        {
            PUT_LE32(pbHeader, MAKE_FOURCC('R', 'V', '3', '0')); //SubMOFTagl
        }
        else if (profile == (int) OMX_VIDEO_RVFormat9)
        {
            PUT_LE32(pbHeader, MAKE_FOURCC('R', 'V', '4', '0')); //SubMOFTagl
        }
        else
        {
            PUT_LE32(pbHeader, MAKE_FOURCC('R', 'V', '4', '0')); //SubMOFTagl, default to rv40
        }
        PUT_BE16(pbHeader, inPort->sPortParam.format.video.nFrameWidth);
        PUT_BE16(pbHeader, inPort->sPortParam.format.video.nFrameHeight);
        PUT_BE16(pbHeader, 0x0c); //BitCount;
        PUT_BE16(pbHeader, 0x00); //PadWidth;
        PUT_BE16(pbHeader, 0x00); //PadHeight;
        PUT_LE32(pbHeader, inPort->sPortParam.format.video.xFramerate);
        PUT_BUFFER(pbHeader, pbMetaData, nMetaData); //OpaqueDatata
#endif
    }

    else if (codingType == OMX_VIDEO_CodingMSMPEG)
    {
        PUT_LE32(pbHeader, MAKE_FOURCC('C', 'N', 'M', 'V')); //signature 'CNMV'
        PUT_LE16(pbHeader, 0x00);                      //version
        PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
        PUT_LE32(pbHeader, MAKE_FOURCC('D', 'I', 'V', '3')); //codec FourCC
        PUT_LE16(pbHeader, inPort->sPortParam.format.video.nFrameWidth);                //width
        PUT_LE16(pbHeader, inPort->sPortParam.format.video.nFrameHeight);               //height
        PUT_LE32(pbHeader, inPort->sPortParam.format.video.xFramerate);      //frame rate
        PUT_LE32(pbHeader, 0);      //time scale(?)
        PUT_LE32(pbHeader, -1);      //number of frames in file
        PUT_LE32(pbHeader, 0); //unused
        size += 32;
    }
    else if (codingType == OMX_VIDEO_CodingMJPEG)
        return 0;
    else if (codingType == OMX_VIDEO_CODINGTYPE_VP8)
    {
        PUT_LE32(pbHeader, MAKE_FOURCC('D', 'K', 'I', 'F')); //signature 'DKIF'
        PUT_LE16(pbHeader, 0x00);                      //version
        PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
        PUT_LE32(pbHeader, MAKE_FOURCC('V', 'P', '8', '0')); //codec FourCC
        PUT_LE16(pbHeader, inPort->sPortParam.format.video.nFrameWidth);                //width
        PUT_LE16(pbHeader, inPort->sPortParam.format.video.nFrameHeight);               //height
        PUT_LE32(pbHeader, inPort->sPortParam.format.video.xFramerate);      //frame rate
        PUT_LE32(pbHeader, 0);      //time scale(?)
        PUT_LE32(pbHeader, -1);      //number of frames in file
        PUT_LE32(pbHeader, 0); //unused
        size += 32;
    }
    else if (codingType == OMX_VIDEO_CodingHEVC)
    {
        if (nMetaData > 1 && pbMetaData && pbMetaData[0] == 0x01)// check mov/mo4 file format stream
        {
            static const uint8_t nalu_header[4] = { 0, 0, 0, 1 };
            int numOfArrays = 0;
            //uint16_t nal_unit_type = 0;
            uint16_t numNalus = 0;
            uint16_t nalUnitLength = 0;
            uint32_t offset = 0;

            DEBUG(DEB_LEV_FULL_SEQ, "HEVC detected as mp4 format\n");

            p += 21;
            if (sizelength)
                *sizelength = (*p++ & 0x3) + 1;
            numOfArrays = *p++;

            while(numOfArrays--)
            {
                //nal_unit_type = *p++ & 0x3F;
                numNalus = (*p << 8) + *(p + 1);
                p+=2;
                for(i = 0;i < numNalus;i++)
                {
                    nalUnitLength = (*p << 8) + *(p + 1);
                    p+=2;
                    //if(i == 0)
                    {
                        memcpy(pbHeader + offset, nalu_header, 4);
                        offset += 4;
                        memcpy(pbHeader + offset, p, nalUnitLength);
                        offset += nalUnitLength;
                    }
                    p += nalUnitLength;
                }
            }
            size = offset;
            privateType->bSizeLengthDetected = OMX_TRUE;
        }
        else if(nMetaData > 3)
        {
            size = -1;// return to meaning of invalid stream data;
            DEBUG(DEB_LEV_FULL_SEQ, "HEVC detected as ES format : size = %d, 0x%x 0x%x 0x%x 0x%x\n",nMetaData, p[0], p[1], p[2], p[3]);
            for (; p < a; p++)
            {
                if ( p[0] == 0 && p[1] == 0 && p[2] == 1) // find startcode
                {
                    size = 0;
                    if ( !(p[3] == 0x40 || p[3] == 0x42 || p[3] == 0x44) )  // if not VPS/SPS/PPS, return
                        break;
                    break;
                }
                else if ( p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1)
                {
                    size = 0;
                    if ( !(p[4] == 0x40 || p[4] == 0x42 || p[4] == 0x44) )  // if not VPS/SPS/PPS, return
                        break;
                    //PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
                    break;
                }
            }
            privateType->bSizeLengthDetected = OMX_FALSE;
        }
        else
        {
            privateType->bSizeLengthDetected = OMX_FALSE;
        }
    }
    else if (codingType == OMX_VIDEO_CodingMPEG4)
    {
        PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
        size = nMetaData;
    }
    else
    {
        //PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
        size = 0;
    }
    DEBUG(DEB_LEV_FULL_SEQ, "Out %s, seq size=%d, buffer_len=%d\n", __func__, (int)size, (int)nMetaData);
    return size;
}


int BuildOmxPicHeader(OMX_U8 *pbHeader, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_COMPONENTTYPE *openmaxStandComp, int sizelength)
{
    omx_vpudec_component_PrivateType* privateType = openmaxStandComp->pComponentPrivate;
#ifdef SUPPORT_ONE_BUFFER_CONTAIN_MULTIPLE_FRAMES
    OMX_U8 *pbChunk = (pInputBuffer->pBuffer+pInputBuffer->nOffset);
#else
    OMX_U8 *pbChunk = pInputBuffer->pBuffer;
#endif
    OMX_S32 size;
    OMX_U32 codingType;
#ifdef OMX_INPUT_BUFFER_FEEDED_BY_FFMPEG_AVPACKET
    OMX_U32 cSlice;
    OMX_U32 profile;
    int i, val;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *)&privateType->vpu;
#endif
    OMX_U32 nSlice;
    OMX_U32 offset;
    int has_st_code = 0;

    if (privateType->bUseOmxInputBufferAsDecBsBuffer == OMX_TRUE)
        return 0;

    codingType = privateType->video_coding_type;
    offset = 0;
    size = 0;

    DEBUG(DEB_LEV_FULL_SEQ, "In %s, codingType=%d, buffer_len=%d\n", __func__, (int)codingType, (int)pInputBuffer->nFilledLen);
    if (codingType == OMX_VIDEO_CodingWMV)
    {
#ifdef OMX_INPUT_BUFFER_FEEDED_BY_FFMPEG_AVPACKET
        profile = privateType->codParam.wmv.eFormat;
#ifdef SUPPORT_CM_OMX_12
        if ((profile != (int) OMX_VIDEO_WMVFormat7) && (profile != (int) OMX_VIDEO_WMVFormat8) && (profile != (int) OMX_VIDEO_WMVFormat9)) // VC1 AP has the start code.
#else
        if (profile == (int) OMX_VIDEO_WMVFormatVC1) // VC1 AP has the start code.
#endif
        {
            if (pbChunk[0] != 0 || pbChunk[1] != 0 || pbChunk[2] != 1) // check start code as prefix (0x00, 0x00, 0x01)
            {
                PUT_BYTE(pbHeader, 0x00);
                PUT_BYTE(pbHeader, 0x00);
                PUT_BYTE(pbHeader, 0x01);
                PUT_BYTE(pbHeader, 0x0D);

                size += 4;
            }
        }
        else
        {
            val = pInputBuffer->nFilledLen;
            if (pInputBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME)
                val |= 0x80000000;
            PUT_LE32(pbHeader, val);
            size += 4;
#ifdef RCV_V2
            PUT_LE32(pbHeader, pInputBuffer->nTimeStamp/1000); // milli_sec
            size += 4;
#endif
        }
#endif
    }
    else if (codingType == OMX_VIDEO_CodingVC1)
    {
#ifdef OMX_INPUT_BUFFER_FEEDED_BY_FFMPEG_AVPACKET
            if (pbChunk[0] != 0 || pbChunk[1] != 0 || pbChunk[2] != 1) // check start code as prefix (0x00, 0x00, 0x01)
            {
                PUT_BYTE(pbHeader, 0x00);               // add frame data start code.
                PUT_BYTE(pbHeader, 0x00);
                PUT_BYTE(pbHeader, 0x01);
                PUT_BYTE(pbHeader, 0x0D);

                size += 4;
            }
#endif
    }
    else if (codingType == OMX_VIDEO_CodingRV)
    {
#ifdef OMX_INPUT_BUFFER_FEEDED_BY_FFMPEG_AVPACKET
        profile = privateType->codParam.rv.eFormat;
        if (profile != (int) OMX_VIDEO_RVFormat9 && profile != OMX_VIDEO_RVFormat8)
            return -1;

        cSlice = pbChunk[0] + 1;
        nSlice = pInputBuffer->nFilledLen - 1 - (cSlice * 8);
        size = 20 + (cSlice * 8);
        PUT_BE32(pbHeader, nSlice);
        PUT_BE32(pbHeader, pInputBuffer->nTimeStamp/1000);
        PUT_BE16(pbHeader, pVpu->frameIdx);
        PUT_BE16(pbHeader, 0x02); //Flags
        PUT_BE32(pbHeader, 0x00); //LastPacket
        PUT_BE32(pbHeader, cSlice); //NumSegments
        offset = 1;
        for (i = 0; i < (int) cSlice; i++)
        {
            val = (pbChunk[offset + 3] << 24) | (pbChunk[offset + 2] << 16) | (pbChunk[offset + 1] << 8) | pbChunk[offset];
            PUT_BE32(pbHeader, val); //isValid
            offset += 4;
            val = (pbChunk[offset + 3] << 24) | (pbChunk[offset + 2] << 16) | (pbChunk[offset + 1] << 8) | pbChunk[offset];
            PUT_BE32(pbHeader, val); //Offset
            offset += 4;
        }
#endif
    }
    else if (codingType == OMX_VIDEO_CodingAVC || codingType == OMX_VIDEO_CodingAVS || codingType == OMX_VIDEO_CodingHEVC)
    {
        if(!(privateType->seqHeader && (privateType->bSizeLengthDetected == OMX_TRUE)))      // if not metadata format, search start codes.
        {
            const Uint8 *pbEnd = pbChunk + 4 - ((intptr_t)pbChunk & 3);

            for (; pbChunk < pbEnd ; pbChunk++)
            {
                if ((pbChunk[0] == 0 && pbChunk[1] == 0 && pbChunk[2] == 1) ||
                    (pbChunk[0] == 0 && pbChunk[1] == 0 && pbChunk[2] == 0 && pbChunk[3] == 1) ||
                    (pbChunk[0] == 0 && pbChunk[1] == 0 && pbChunk[2] == 0 && pbChunk[3] == 0 && pbChunk[4] == 1))
                {
                    has_st_code = 1;
                    break;
                }
            }
        }

        if (!has_st_code ) // if no start codes, replace size byte to start codes
        {

#ifdef SUPPORT_ONE_BUFFER_CONTAIN_MULTIPLE_FRAMES
            pbChunk = (pInputBuffer->pBuffer+pInputBuffer->nOffset);
#else
            pbChunk = pInputBuffer->pBuffer;
#endif
            while ((offset+sizelength) < pInputBuffer->nFilledLen)
            {
                if (codingType == OMX_VIDEO_CodingAVC || codingType == OMX_VIDEO_CodingHEVC)
                {
                    if(sizelength == 3)
                    {
                        nSlice = pbChunk[offset] << 16 | pbChunk[offset+1] << 8 | pbChunk[offset+2];
                        if (nSlice == 1)    // workaround for abnormal case. size must not be 1
                            nSlice = pInputBuffer->nFilledLen-3;

                        pbChunk[offset] = 0x00;
                        pbChunk[offset+1] = 0x00;
                        pbChunk[offset+2] = 0x01;

                        offset += 3;
                    }
                    else    // sizeLength = 4
                    {
                        nSlice = pbChunk[offset] << 24 | pbChunk[offset + 1] << 16 | pbChunk[offset + 2] << 8 | pbChunk[offset + 3];
                        if (nSlice == 1)    // workaround for abnormal case. size must not be 1
                            nSlice = pInputBuffer->nFilledLen-4;

                        pbChunk[offset] = 0x00;
                        pbChunk[offset+1] = 0x00;
                        pbChunk[offset+2] = 0x00;
                        pbChunk[offset+3] = 0x01;       //replace size to startcode

                        offset += 4;
                    }
                }
                /* AVS no support
                else
                {
                    nSlice = pbChunk[offset] << 24 | pbChunk[offset + 1] << 16 | pbChunk[offset + 2] << 8 | pbChunk[offset + 3];
                    pbChunk[offset+0] = 0x00;
                    pbChunk[offset+1] = 0x00;
                    pbChunk[offset+2] = 0x00;
                    pbChunk[offset+3] = 0x00;       //replace size to startcode
                    pbChunk[offset+4] = 0x01;

                    offset += 5;
                }*/

                if (codingType == OMX_VIDEO_CodingHEVC)
                {
                    switch ((pbChunk[offset]&0x7E)>>1) /* NAL unit */
                    {
                    case 39: /* PREFIX SEI */
                    case 40: /* SUFFIX SEI */
                    case 32: /* VPS */
                    case 33: /* SPS */
                    case 34: /* PPS */
                        /* check next */
                    break;
                }
                }

                if (codingType == OMX_VIDEO_CodingAVC)
                {
                    switch (pbChunk[offset]&0x1f) /* NAL unit */
                    {
                    case 6: /* SEI */
                    case 7: /* SPS */
                    case 8: /* PPS */
                    case 9: /* AU */
                        /* check next */
                        break;
                    }
                }
                offset += nSlice;
            }
        }
    }
    else if (codingType == OMX_VIDEO_CodingMSMPEG)
    {
        PUT_LE32(pbHeader,pInputBuffer->nFilledLen);
        PUT_LE32(pbHeader,0);
        PUT_LE32(pbHeader,0);
        size += 12;
    }
    else if (codingType == OMX_VIDEO_CODINGTYPE_VP8)
    {
        PUT_LE32(pbHeader,pInputBuffer->nFilledLen);
        PUT_LE32(pbHeader,0);
        PUT_LE32(pbHeader,0);
        size += 12;
    }

    DEBUG(DEB_LEV_FULL_SEQ, "Out %s, header size=%d, buffer_len=%d\n", __func__, (int)size, (int)pInputBuffer->nFilledLen);
    return size;
}

/* OmxAllocateFrameBuffers()
 *   - step 1 : free framebuffers for VPU has been allocated. (when VP9 inter-resolution case, free framebuffer only for indicated re-alloc buffer (fbcCurFrameIdx))
 *   - step 2 : allocate framebuffers for references (DPB) that VPU should be allocated. (when VP9 inter-resolution case, allocate framebuffer only for indicated re-alloc buffer (fbcCurFrameIdx))
 *   - step 3 : allocate framebuffers for display that VPU should be allocated
 * :pVpu->vbDPB     : framebuffer info for the references
 * :pVpu->vbAllocFb : framebuffer info for the display (wtl buffer)
 * :pVpu->vbUser    : framebuffer info for all (both references and display)
 */
int OmxAllocateFrameBuffers(OMX_COMPONENTTYPE *openmaxStandComp)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;

    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *inPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    int framebufSize = 0;
    int i;
    int ret;
    int bufferAllocNum;
    FrameBufferAllocInfo fbAllocInfo;

    UNREFERENCED_PARAMETER(inPort);

    // workaround for fixing buffer count mismatch between Android OMX IL and component
    pVpu->decFbCount = outPort->sPortParam.nBufferCountActual;
    pVpu->dispFbCount = outPort->sPortParam.nBufferCountActual;
    pVpu->regFbCount = outPort->sPortParam.nBufferCountActual;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "%s decFbCount %d\n", __func__, outPort->sPortParam.nBufferCountActual);

    // 1st step : free buffer first. (to support AdaptivePlayback)
    if (outPort && outPort->bAllocateBuffer == OMX_FALSE)
    {
        if (omx_vpudec_component_Private->useNativeBuffer == OMX_FALSE)
        {
            for (i=0; i<MAX_REG_FRAME; i++)
            {
                // all the framebuffers for display(WTL) should be cleared. even though inter-resolution change for VP9 case.
                if (pVpu->vbAllocFb[i].size > 0)
                {
                    DEBUG(DEB_LEV_FULL_SEQ, "OmxAllocateFrameBuffers free vbAllocFb[%d]\n", i);
                    vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbAllocFb[i]);
                }
            }
        }
    }

    for (i=0; i<MAX_REG_FRAME; i++)
    {
        // when VP9 && inter-resolution change happened, re-allocated FBC buffer(1 buffer)
        if (pVpu->decOP.bitstreamFormat == STD_VP9 && pVpu->interResChanged)
        {
            if (pVpu->fbcCurFrameIdx >= 0 && i == pVpu->fbcCurFrameIdx)
            {
                if (pVpu->vbDPB[i].size > 0)
                {
                    vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbDPB[i]);
                }
            }
        }
        else if (pVpu->vbDPB[i].size > 0)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "OmxAllocateFrameBuffers free vbDPB[%d]\n", i);
            vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbDPB[i]);
        }
    }

    uint32_t totalFbCount = pVpu->decFbCount + pVpu->dispFbCount;
    osal_memset((void*)&fbAllocInfo, 0x00, sizeof(fbAllocInfo));
    osal_memset((void*)pVpu->vbUserFb, 0x00, sizeof(vpu_buffer_t)*totalFbCount);
    osal_memset((void*)pVpu->vbDPB, 0x00, sizeof(vpu_buffer_t)*totalFbCount);

    DecInitialInfo          seqInfo;
    VPU_DecGiveCommand(pVpu->handle, DEC_GET_SEQ_INFO, (void*)&seqInfo);

    fbAllocInfo.format          = (FrameBufferFormat)pVpu->fbFormat;
    if (seqInfo.lumaBitdepth > 8 || seqInfo.chromaBitdepth > 8) {
        if (pVpu->coreIdx == VPU_WAVE412_IDX) {
            fbAllocInfo.format = FORMAT_420_P10_16BIT_LSB;
            if (pVpu->decOP.pvricFbcEnable) {
                DEBUG(DEB_LEV_FULL_SEQ, "WAVE: 10bit color depth to 8bit ifbc\n");
            } else {
                DEBUG(DEB_LEV_FULL_SEQ, "WAVE: 10bit color depth to 16bit linear\n");
            }
        } else {
            DEBUG(DEB_LEV_ERR, "CODA: 10bit color depth not supported\n");
            goto ERROR_ALLOCATE_FRAME_BUFFERS;
        }
    }

    fbAllocInfo.cbcrInterleave  = pVpu->decOP.cbcrInterleave;
    fbAllocInfo.mapType         = pVpu->mapType;
    fbAllocInfo.stride          = pVpu->fbStride;
    fbAllocInfo.height          = pVpu->fbHeight;
    framebufSize                = outPort->sPortParam.nBufferSize;

    // 2nd step : alloc buffer for DPB (not framebuffers for display)
    if (pVpu->coreIdx == VPU_WAVE412_IDX)
    {
        if (pVpu->decOP.bitstreamFormat == STD_VP9)
        {
            fbAllocInfo.stride = CalcStride(VPU_ALIGN64(seqInfo.picWidth), seqInfo.picHeight, fbAllocInfo.format, fbAllocInfo.cbcrInterleave, fbAllocInfo.mapType, true);
            fbAllocInfo.height = VPU_ALIGN64(seqInfo.picHeight);
        }
        else
        {
            fbAllocInfo.stride = CalcStride(VPU_ALIGN32(seqInfo.picWidth), seqInfo.picHeight, fbAllocInfo.format, fbAllocInfo.cbcrInterleave, fbAllocInfo.mapType, false);
            fbAllocInfo.height = VPU_ALIGN32(seqInfo.picHeight);
        }
        framebufSize = VPU_GetFrameBufSize(pVpu->coreIdx, fbAllocInfo.stride, fbAllocInfo.height,
            fbAllocInfo.mapType, fbAllocInfo.format, fbAllocInfo.cbcrInterleave, &pVpu->dramCfg);
    }

    fbAllocInfo.size            = framebufSize;
    fbAllocInfo.lumaBitDepth    = seqInfo.lumaBitdepth;
    fbAllocInfo.chromaBitDepth  = seqInfo.chromaBitdepth;
    fbAllocInfo.num             = pVpu->decFbCount;
    fbAllocInfo.endian          = pVpu->decOP.frameEndian;
    fbAllocInfo.type            = FB_TYPE_CODEC;

    if (pVpu->decOP.wtlEnable) {
        VPU_DecGiveCommand(pVpu->handle, DEC_SET_WTL_FRAME_FORMAT, &pVpu->fbFormat);
    }

    DEBUG(DEB_LEV_FULL_SEQ, "OmxAllocateFrameBuffers bAllocateBuffer %d, vbDPB=%d, pVpu->decFbCount %d, framebufSize %d, height %d, stride %d, fbFormat %d\n", outPort->bAllocateBuffer,
            pVpu->vbDPB[0].size, pVpu->decFbCount, framebufSize, pVpu->fbHeight, pVpu->fbStride, pVpu->fbFormat);
    if (pVpu->decOP.wtlEnable || pVpu->scalerInfo.enScaler)
    {
        if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
            bufferAllocNum = 1;
        else
            bufferAllocNum = fbAllocInfo.num;

        DEBUG(DEB_LEV_FULL_SEQ, "%s wtlEnable for DPB bufferAllocNum %d\n", __func__, bufferAllocNum);
        //1.arrangement vbUserFb for all buffer
        if (pVpu->decOP.bitstreamFormat == STD_VP9 && pVpu->interResChanged)
        {
            // allocate 1 framebuffer for re-allocated FBC when inter-resolution change occurred.
            if (pVpu->fbcCurFrameIdx >= 0 && pVpu->vbDPB[pVpu->fbcCurFrameIdx].size > 0)
            {
                pVpu->vbDPB[pVpu->fbcCurFrameIdx].size = framebufSize;
                if (vdi_allocate_dma_cache_memory(pVpu->coreIdx, &pVpu->vbDPB[pVpu->fbcCurFrameIdx]) < 0)
                    goto ERROR_ALLOCATE_FRAME_BUFFERS;
            }
        }
        else
        {
            for (i=0; i<bufferAllocNum; i++)
            {
                pVpu->vbDPB[i].size = framebufSize;
                if (vdi_allocate_dma_cache_memory(pVpu->coreIdx, &pVpu->vbDPB[i]) < 0)
                    goto ERROR_ALLOCATE_FRAME_BUFFERS;
            }
        }

        for (i=0 ;i < bufferAllocNum; i++)
            pVpu->vbUserFb[i] = pVpu->vbDPB[i];


        if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
        {
            for (i=0; i < pVpu->regFbCount; i++)
                pVpu->vbUserFb[i] = pVpu->vbUserFb[0];
        }

        //2.arrangement fbUser for all buffer
        for (i=0; i<fbAllocInfo.num; i++)
        {
            pVpu->fbUser[i].bufY = pVpu->vbUserFb[i].phys_addr;
            pVpu->fbUser[i].bufCb = -1;
            pVpu->fbUser[i].bufCr = -1;
            pVpu->fbUser[i].updateFbInfo = TRUE;
            DEBUG(DEB_LEV_FULL_SEQ, "OmxAllocateFrameBuffers  for DPB Index=%d, pInternalBufferStorage=%p, fbAllocInfo.num=%d, Virtual Address=0x%x, Physical Address=0x%x, framebufSize=%d\n",
                i,  outPort->pInternalBufferStorage[i], (int)fbAllocInfo.num, (int)pVpu->vbUserFb[i].virt_addr, (int)pVpu->fbUser[i].bufY, (int)framebufSize);
        }
    }
    else
    {
        if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
            bufferAllocNum = 1;
        else
            bufferAllocNum = fbAllocInfo.num;

        DEBUG(DEB_LEV_FULL_SEQ, "useNativeBuffer %d, bAllocateBuffer %d, framebufSize %d\n",
            omx_vpudec_component_Private->useNativeBuffer, outPort->bAllocateBuffer, framebufSize);
        if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "%d bufferAllocNum %d\n", __LINE__, bufferAllocNum);
            //1.arrangement vbUserFb for all buffer
            for (i=0; i < bufferAllocNum; i++)
            {
                pVpu->vbUserFb[i].size = framebufSize;
#ifdef ANDROID
                if (getAndroidNativeHandle(outPort->pInternalBufferStorage[i], (OMX_U32 *)&pVpu->vbUserFb[i].buf_handle))
                {
                    DEBUG(DEB_LEV_ERR, "failed to get ion share fd");
                    goto ERROR_ALLOCATE_FRAME_BUFFERS;
                }
#else
                OMX_BUFFERHEADERTYPE *pBufferHdr = outPort->pInternalBufferStorage[i];
                if (pBufferHdr && pBufferHdr->pBuffer) {
                    pVpu->vbUserFb[i].buf_handle = *(OMX_U32 *)pBufferHdr->pBuffer;
                } else {
                    DEBUG(DEB_LEV_ERR, "no pBuffer from gstomx\n");
                    goto ERROR_ALLOCATE_FRAME_BUFFERS;
                }
#endif
                /* map dam buffer--> phys/virt to user space */
                DEBUG(DEB_LEV_FULL_SEQ, "buf_handle %d\n", pVpu->vbUserFb[i].buf_handle);
                if (vdi_device_memory_map(pVpu->coreIdx, &pVpu->vbUserFb[i]) < 0)
                    goto ERROR_ALLOCATE_FRAME_BUFFERS;

            }

            if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
            {
                for (i=0; i < pVpu->regFbCount; i++)
                {
                    pVpu->vbUserFb[i] = pVpu->vbUserFb[0];
                }
            }

            //2.arrangement fbUser for all buffer
            for (i=0; i<fbAllocInfo.num; i++)
            {
                pVpu->fbUser[i].bufY = (PhysicalAddress)pVpu->vbUserFb[i].phys_addr;
                pVpu->fbUser[i].bufCb = -1;
                pVpu->fbUser[i].bufCr = -1;
                pVpu->fbUser[i].updateFbInfo = TRUE;
                DEBUG(DEB_LEV_FULL_SEQ, "useBuffer OmxAllocateFrameBuffers  for DPB display Index=%d, pInternalBufferStorage=%p, fbAllocInfo.num=%d, Virtual Address=0x%x, Physical Address=0x%x\n",
                    i,  outPort->pInternalBufferStorage[i], (int)fbAllocInfo.num, (int)pVpu->vbUserFb[i].virt_addr, (int)pVpu->fbUser[i].bufY);
            }
        }
        else
        {
            if (outPort->bAllocateBuffer == OMX_FALSE)
            {
                DEBUG(DEB_LEV_FULL_SEQ, "OmxAllocateFrameBuffers bufferAllocNum %d, framebufSize %d\n", bufferAllocNum, framebufSize);

                for (i=0; i<bufferAllocNum; i++)
                {
                    pVpu->vbAllocFb[i].size = framebufSize;
                    if (vdi_allocate_dma_cache_memory(pVpu->coreIdx, &pVpu->vbAllocFb[i]) < 0)
                        goto ERROR_ALLOCATE_FRAME_BUFFERS;
                }
                // fix usebuffer not copied
                // outPort->bAllocateBuffer = OMX_TRUE;
            }
            else
            {
                //In use of OMX_AllocateBuffer. omx_videodec_component_AllocateBuffer should allocate some buffers by IL Client.
            }

            //1.arrangement vbUserFb for all buffer
            for (i=0; i < bufferAllocNum; i++)
                pVpu->vbUserFb[i] = pVpu->vbAllocFb[i];

            if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
            {
                for (i=0; i < pVpu->regFbCount; i++)
                    pVpu->vbUserFb[i] = pVpu->vbUserFb[0];
            }

            //2.arrangement fbUser for all buffer
            for (i=0; i<fbAllocInfo.num; i++)
            {
                pVpu->fbUser[i].bufY = (PhysicalAddress)pVpu->vbUserFb[i].phys_addr;
                pVpu->fbUser[i].bufCb = -1;
                pVpu->fbUser[i].bufCr = -1;
                pVpu->fbUser[i].updateFbInfo = TRUE;
                DEBUG(DEB_LEV_FULL_SEQ, "step 1 OmxAllocateFrameBuffers  for DPB display Index=%d, pInternalBufferStorage=%p, fbAllocInfo.num=%d, Virtual Address=0x%x, Physical Address=0x%x, framebufSize=%d\n", i,  outPort->pInternalBufferStorage[i], (int)fbAllocInfo.num, (int)pVpu->vbUserFb[i].virt_addr, (int)pVpu->fbUser[i].bufY, (int)pVpu->vbUserFb[i].size);
            }
        }
    }

    if (RETCODE_SUCCESS != VPU_DecAllocateFrameBuffer(pVpu->handle, fbAllocInfo, &pVpu->fbUser[0])) // this will not allocate frame buffer it will just translate the base address of native buffer to FrameBuffer structure
    {
        DEBUG(DEB_LEV_ERR, "VPU_DecAllocateFrameBuffer for DPB fail to allocate source frame buffer\n");
        goto ERROR_ALLOCATE_FRAME_BUFFERS;
    }

#ifdef SUPPORT_PARALLEL
    if (omx_vpudec_component_Private->bThumbnailMode == OMX_FALSE)
        pVpu->vbMvCol.size = VPU_GetMvColBufSize(pVpu->decOP.bitstreamFormat, pVpu->fbWidth, pVpu->fbHeight, pVpu->decFbCount);
    else
        pVpu->vbMvCol.size = VPU_GetMvColBufSize(pVpu->decOP.bitstreamFormat, pVpu->fbWidth, pVpu->fbHeight, 1);
    if (pVpu->vbMvCol.size > 0)
    {
        if (vdi_allocate_dma_memory(pVpu->coreIdx, &pVpu->vbMvCol) < 0)
        {
            DEBUG(DEB_LEV_ERR, "fail to allocate frame buffer\n" );
            goto ERROR_ALLOCATE_FRAME_BUFFERS;
        }
        pVpu->fbUser[0].bufMvCol = pVpu->vbMvCol.phys_addr;
    }
#endif


    // 3rd step : allocate framebuffers for display
    if (pVpu->decOP.wtlEnable || pVpu->scalerInfo.enScaler)
    {
        /* TODO: add scale support
        if (pVpu->scalerInfo.enScaler)
        {
            // update frame buffer allocation information for scaler.
#ifdef SUPPORT_SCALER_STRIDE
            fbAllocInfo.stride = pVpu->scalerInfo.scaleStride;
#else
            fbAllocInfo.stride = pVpu->scalerInfo.scaleWidth;
#endif
            fbAllocInfo.height = pVpu->scalerInfo.scaleHeight;
            fbAllocInfo.num = pVpu->dispFbCount;
            // fbAllocInfo.format = (pVpu->scalerInfo.imageFormat < YUV_FORMAT_I422)?FORMAT_420:FORMAT_422;
            fbAllocInfo.format = FORMAT_420;
            fbAllocInfo.mapType = LINEAR_FRAME_MAP;
            fbAllocInfo.type = FB_TYPE_CODEC;
            framebufSize = VPU_GetFrameBufSize(pVpu->coreIdx, fbAllocInfo.stride, fbAllocInfo.height, fbAllocInfo.mapType, fbAllocInfo.format, 0, &pVpu->dramCfg);
            DEBUG(DEB_LEV_FULL_SEQ, "enScaler stride %d, num %d, size %d\n", fbAllocInfo.stride, fbAllocInfo.num, framebufSize);
        }
        */

        if (pVpu->decOP.wtlEnable)
        {
            fbAllocInfo.mapType = (pVpu->decOP.wtlMode == FF_FRAME)?LINEAR_FRAME_MAP:LINEAR_FIELD_MAP;
            fbAllocInfo.num = pVpu->dispFbCount;
            fbAllocInfo.format = (FrameBufferFormat)pVpu->fbFormat;
            fbAllocInfo.stride = pVpu->fbStride;
            fbAllocInfo.height = pVpu->fbHeight;

            if (pVpu->decOP.pvricFbcEnable)
            {
                fbAllocInfo.mapType = PVRIC_COMPRESSED_FRAME_MAP;
            }

            fbAllocInfo.size = outPort->sPortParam.nBufferSize;
            DEBUG(DEB_LEV_FULL_SEQ, "pvricFbcEnable %d maptype for WTL = %d, framebufSize for WTL = %d, heigh %d, stride %d, fbFormat %d\n", pVpu->decOP.pvricFbcEnable, fbAllocInfo.mapType, fbAllocInfo.size, fbAllocInfo.height, fbAllocInfo.stride, pVpu->fbFormat);
        }

        if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
            bufferAllocNum = pVpu->decFbCount+1;
        else
            bufferAllocNum = (pVpu->decFbCount+fbAllocInfo.num);

        if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "%s decFbCount %d bufferAllocNum %d, fbAllocInfo.num %d\n", __func__, pVpu->decFbCount, bufferAllocNum, fbAllocInfo.num);
            for (i=pVpu->decFbCount; i<bufferAllocNum; i++)
            {
                pVpu->vbUserFb[i].size = fbAllocInfo.size;
#ifdef ANDROID
                if (getAndroidNativeHandle(outPort->pInternalBufferStorage[i - pVpu->decFbCount], (OMX_U32 *)&pVpu->vbUserFb[i].buf_handle))
                {
                    DEBUG(DEB_LEV_ERR, "failed to get ion share fd");
                    goto ERROR_ALLOCATE_FRAME_BUFFERS;
                }
#else
                OMX_BUFFERHEADERTYPE *pBufferHdr = outPort->pInternalBufferStorage[i - pVpu->decFbCount];
                if (pBufferHdr && pBufferHdr->pBuffer) {
                    pVpu->vbUserFb[i].buf_handle = *(OMX_U32 *)pBufferHdr->pBuffer;
                } else {
                    DEBUG(DEB_LEV_ERR, "no pBuffer from gstomx\n");
                    goto ERROR_ALLOCATE_FRAME_BUFFERS;
                }
#endif
                /* map dam buffer--> phys/virt to user space */
                DEBUG(DEB_LEV_FULL_SEQ, "disp buf_handle %d\n", pVpu->vbUserFb[i].buf_handle);
                if (vdi_device_memory_map(pVpu->coreIdx, &pVpu->vbUserFb[i]) < 0)
                {
                    goto ERROR_ALLOCATE_FRAME_BUFFERS;
                }
            }

            if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
            {
                for (i=pVpu->decFbCount; i < (pVpu->decFbCount+pVpu->regFbCount); i++)
                {
                    pVpu->vbUserFb[i] = pVpu->vbUserFb[pVpu->decFbCount];
                }
            }

            //2.arrangement fbUser for all buffer
            for (i=pVpu->decFbCount; i < (pVpu->decFbCount+fbAllocInfo.num); i++)
            {
                pVpu->fbUser[i].bufY = (PhysicalAddress)pVpu->vbUserFb[i].phys_addr;
                pVpu->fbUser[i].bufCb = -1;
                pVpu->fbUser[i].bufCr = -1;
                pVpu->fbUser[i].updateFbInfo = TRUE;
                DEBUG(DEB_LEV_FULL_SEQ, "OmxAllocateFrameBuffers  for Display Index=%d, pInternalBufferStorage=%p, fbAllocInfo.num=%d, Virtual Address=0x%x, Physical Address=0x%x, framebufferSize=%d\n", i, outPort->pInternalBufferStorage[i-pVpu->decFbCount], (int)fbAllocInfo.num, (int)pVpu->vbUserFb[i].virt_addr, (int)pVpu->fbUser[i].bufY, (int)fbAllocInfo.size);
            }
        }
        else
        {
            if (outPort->bAllocateBuffer == OMX_FALSE)
            {
                for (i=pVpu->decFbCount; i<bufferAllocNum; i++)
                {
                    pVpu->vbAllocFb[i-pVpu->decFbCount].size = framebufSize;
                    if (vdi_allocate_dma_cache_memory(pVpu->coreIdx, &pVpu->vbAllocFb[i-pVpu->decFbCount]) < 0)
                        goto ERROR_ALLOCATE_FRAME_BUFFERS;
                }
            }

            //1.arrangement vbUserFb for all buffer
            for (i=pVpu->decFbCount ;i < bufferAllocNum; i++)
                pVpu->vbUserFb[i] = pVpu->vbAllocFb[i-pVpu->decFbCount];

            if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE) // set all buffer to first buffer address
            {
                for (i=pVpu->decFbCount; i < (pVpu->decFbCount+pVpu->regFbCount); i++)
                {
                    pVpu->vbUserFb[i] = pVpu->vbUserFb[pVpu->decFbCount];
                }
            }


            for (i=pVpu->decFbCount; i<(pVpu->decFbCount+fbAllocInfo.num); i++)
            {
                pVpu->fbUser[i].bufY = (PhysicalAddress)pVpu->vbUserFb[i].phys_addr;
                pVpu->fbUser[i].bufCb = -1; //this means that we want to assign the address at VPU_DecAllocateFrameBuffer function thatn allocate frame buffer
                pVpu->fbUser[i].bufCr = -1;
                pVpu->fbUser[i].updateFbInfo = TRUE;
                DEBUG(DEB_LEV_FULL_SEQ, "OmxAllocateFrameBuffers  for Scaler Index=%d, pInternalBufferStorage=%p, fbAllocInfo.num=%d, Virtual Address=0x%x, Physical Address=0x%x\n", i, outPort->pInternalBufferStorage[i-pVpu->decFbCount], (int)fbAllocInfo.num, (int)pVpu->vbUserFb[i].virt_addr, (int)pVpu->fbUser[i].bufY);
            }
        }

        ret = VPU_DecAllocateFrameBuffer(pVpu->handle, fbAllocInfo, &pVpu->fbUser[pVpu->decFbCount]);
        if( ret != RETCODE_SUCCESS )
        {
            DEBUG(ERR, "VPU_DecAllocateFrameBuffer fail to allocate WTL frame buffer is 0x%x \n", ret );
            goto ERROR_ALLOCATE_FRAME_BUFFERS;
        }
    }

    return 1;

ERROR_ALLOCATE_FRAME_BUFFERS:

    DEBUG(DEB_LEV_ERR, "%s failed\n", __func__);
    return 0;
}


void OmxCheckVersion(int coreIdx)
{
    unsigned int version;
    unsigned int revision;
    unsigned int productId;

    VPU_GetVersionInfo(coreIdx, &version, &revision, &productId);

    DEBUG(DEB_LEV_SIMPLE_SEQ, "VPU coreNum : [%d]\n", coreIdx);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Firmware Version => projectId : %x | version : %04x.%04x.%08x | revision : r%d\n",
        (int)(version>>16), (int)((version>>(12))&0x0f), (int)((version>>(8))&0x0f), (int)((version)&0xff), (int)revision);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Hardware Version => %04x\n", productId);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "API Version => %04x\n\n", API_VERSION);
}


OMX_ERRORTYPE omx_videodec_component_AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_base_component_PrivateType* omx_base_component_Private = (omx_base_component_PrivateType*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_base_PortType *openmaxStandPort;
    omx_vpudec_component_PortType *vpuPort;


    if (nPortIndex >= (omx_base_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainImage].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainOther].nPorts)) {
            DEBUG(DEB_LEV_ERR, "In %s: wrong port index\n", __func__);
            return OMX_ErrorBadPortIndex;
    }

    openmaxStandPort = (omx_base_PortType *) omx_base_component_Private->ports[nPortIndex];
    vpuPort = (omx_vpudec_component_PortType *)openmaxStandPort;

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

        if(nSizeBytes < openmaxStandPort->sPortParam.nBufferSize) {
            DEBUG(DEB_LEV_ERR, "In %s: Requested Buffer Size %d is less than Minimum Buffer Size %u\n", __func__, nSizeBytes, openmaxStandPort->sPortParam.nBufferSize);
            return OMX_ErrorIncorrectStateTransition;
        }

        if (nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
        {
            if (openmaxStandPort->sPortParam.nBufferCountActual >  MAX_DEC_BITSTREAM_BUFFER_COUNT)
            {
                DEBUG(DEB_LEV_ERR, "In %s: nBufferCountActual[%d] is more than MAX_DEC_BITSTREAM_BUFFER_COUNT[%d]\n", __func__, (int)openmaxStandPort->sPortParam.nBufferCountActual, (int)MAX_DEC_BITSTREAM_BUFFER_COUNT);
                return OMX_ErrorInsufficientResources;
            }
        }

        DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s: nBufferCountActual %d\n", __func__, openmaxStandPort->sPortParam.nBufferCountActual);
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
                if (openmaxStandPort->sPortParam.eDir == OMX_DirInput)
                {
                    if (omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer == OMX_TRUE)
                    {
                        /* allocate the buffer */
                        pVpu->vbStream[i].size = nSizeBytes;
                        pVpu->vbStream[i].size = ((pVpu->vbStream[i].size+1023)&~1023);
                        if (vdi_allocate_dma_memory(pVpu->coreIdx, &pVpu->vbStream[i]) < 0)
                        {
                            DEBUG(DEB_LEV_ERR, "fail to allocate bitstream buffer bufferIdx=%d\n", (int)i);
                            return OMX_ErrorInsufficientResources;
                        }
                        DEBUG(DEB_LEV_SIMPLE_SEQ, "allocate bitstream phy %x, buffer=0x%x, size = %d\n", (int)pVpu->vbStream[i].phys_addr, (int)pVpu->vbStream[i].virt_addr, (int)pVpu->vbStream[i].size);
                        openmaxStandPort->pInternalBufferStorage[i]->pBuffer = (OMX_U8 *)pVpu->vbStream[i].virt_addr;
                        memset(openmaxStandPort->pInternalBufferStorage[i]->pBuffer, 0x00, nSizeBytes);
                    }
                    else
                    {
                        openmaxStandPort->pInternalBufferStorage[i]->pBuffer = malloc(nSizeBytes);
                        if(openmaxStandPort->pInternalBufferStorage[i]->pBuffer==NULL) {
                            DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
                            return OMX_ErrorInsufficientResources;
                        }
                        memset(openmaxStandPort->pInternalBufferStorage[i]->pBuffer, 0x00, nSizeBytes);

                    }
                    openmaxStandPort->pInternalBufferStorage[i]->nInputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
                }
                else
                {

                    /* allocate the buffer */
                    pVpu->vbAllocFb[i].size = nSizeBytes;
                    if (vdi_allocate_dma_memory(pVpu->coreIdx, &pVpu->vbAllocFb[i]) < 0)
                    {
                        DEBUG(DEB_LEV_ERR, "fail to allocate framebuffer buffer bufferIdx=%d\n", (int)i);
                        return OMX_ErrorInsufficientResources;
                    }
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "allocate framebuffer buffer = 0x%llx, phy = 0x%llxsize = %d, %p\n", (long long)pVpu->vbAllocFb[i].virt_addr, (long long)pVpu->vbAllocFb[i].phys_addr, (int)pVpu->vbAllocFb[i].size, openmaxStandPort->pInternalBufferStorage[i]);
                    openmaxStandPort->pInternalBufferStorage[i]->pBuffer = (OMX_U8 *)pVpu->vbAllocFb[i].virt_addr;
                    memset(openmaxStandPort->pInternalBufferStorage[i]->pBuffer, 0x00, nSizeBytes);

                    openmaxStandPort->pInternalBufferStorage[i]->nOutputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
                    omx_vpudec_component_Private->omx_display_flags[i].owner = OMX_BUFFER_OWNER_NOBODY;
                }

                openmaxStandPort->pInternalBufferStorage[i]->nAllocLen = nSizeBytes;
                openmaxStandPort->pInternalBufferStorage[i]->pPlatformPrivate = openmaxStandPort;
                openmaxStandPort->pInternalBufferStorage[i]->pAppPrivate = pAppPrivate;
                *ppBuffer = openmaxStandPort->pInternalBufferStorage[i];
                openmaxStandPort->bBufferStateAllocated[i] = BUFFER_ALLOCATED;
                openmaxStandPort->bBufferStateAllocated[i] |= HEADER_ALLOCATED;

                DEBUG(DEB_LEV_SIMPLE_SEQ, "%s nPortIndex %d nNumAssignedBuffers %d\n", __func__, nPortIndex, openmaxStandPort->nNumAssignedBuffers);
                openmaxStandPort->nNumAssignedBuffers++;

                if (openmaxStandPort->sPortParam.nBufferCountActual == openmaxStandPort->nNumAssignedBuffers)
                {
                    openmaxStandPort->sPortParam.bPopulated = OMX_TRUE;
                    openmaxStandPort->bIsFullOfBuffers = OMX_TRUE;
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s for Component=%p, Port=%p, index %d\n", __func__, openmaxStandComp, openmaxStandPort, nPortIndex);
                    vpuPort->bAllocateBuffer = OMX_TRUE;
                    tsem_up(openmaxStandPort->pAllocSem);
                    if (omx_vpudec_component_Private->portSettingChangeRequest == OMX_TRUE)
                    {
                        omx_vpudec_component_Private->portSettingChangeRequest = OMX_FALSE;
                        tsem_up(&omx_vpudec_component_Private->port_setting_change_tsem);
                    }

                    DEBUG(DEB_LEV_SIMPLE_SEQ, "Done of %s  portIndex=%d, nNumAssignedBuffers=%d, bUseOmxInputBufferAsDecBsBuffer=%d\n", __func__, (int)openmaxStandPort->sPortParam.eDir, (int)openmaxStandPort->nNumAssignedBuffers, (int)omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer);
                }
                DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for port %p\n", __func__, openmaxStandPort);
                return OMX_ErrorNone;
            }
        }

        DEBUG(DEB_LEV_SIMPLE_SEQ, "Out of %s for port %p. Error: no available buffers\n",__func__, openmaxStandPort);
        return OMX_ErrorInsufficientResources;
    }
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out %s for Component=%p, Port=%p, buffer %p\n", __func__, openmaxStandComp, openmaxStandPort, ppBuffer);

    return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_videodec_component_UseBuffer(
    OMX_HANDLETYPE hComponent,
    OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_U32 nPortIndex,
    OMX_PTR pAppPrivate,
    OMX_U32 nSizeBytes,
    OMX_U8* pBuffer)
{
    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_base_component_PrivateType* omx_base_component_Private = (omx_base_component_PrivateType*)((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    omx_base_PortType *openmaxStandPort;
    omx_vpudec_component_PortType *vpuPort;
    OMX_BUFFERHEADERTYPE* returnBufferHeader;


    if (nPortIndex >= (omx_base_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainImage].nPorts +
        omx_base_component_Private->sPortTypesParam[OMX_PortDomainOther].nPorts)) {
            DEBUG(DEB_LEV_ERR, "In %s: wrong port index\n", __func__);
            return OMX_ErrorBadPortIndex;
    }

    openmaxStandPort = (omx_base_PortType *) omx_base_component_Private->ports[nPortIndex];
    vpuPort = (omx_vpudec_component_PortType *)openmaxStandPort;

    if (openmaxStandPort)
    {
        unsigned int i;
        DEBUG(DEB_LEV_FUNCTION_NAME, "In %s for Component=%p, Port=%p, index %d\n", __func__, openmaxStandComp, openmaxStandPort, nPortIndex);

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

        if(nSizeBytes < openmaxStandPort->sPortParam.nBufferSize) {
            DEBUG(DEB_LEV_ERR, "In %s: Requested Buffer Size %d is less than Minimum Buffer Size %u\n", __func__, nSizeBytes, openmaxStandPort->sPortParam.nBufferSize);
            return OMX_ErrorIncorrectStateTransition;
        }

        i = openmaxStandPort->nNumAssignedBuffers;
        DEBUG(DEB_LEV_SIMPLE_SEQ, "%s nBufferCountActual %d, nNumAssignedBuffers %d, i %d\n", __func__,
            openmaxStandPort->sPortParam.nBufferCountActual, openmaxStandPort->nNumAssignedBuffers, i);
        if (openmaxStandPort->bBufferStateAllocated[i] == BUFFER_FREE) {
            openmaxStandPort->pInternalBufferStorage[i] = malloc(sizeof(OMX_BUFFERHEADERTYPE));
            if (!openmaxStandPort->pInternalBufferStorage[i]) {
                DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
                return OMX_ErrorInsufficientResources;
            }

            memset(openmaxStandPort->pInternalBufferStorage[i], 0x00, sizeof(OMX_BUFFERHEADERTYPE));
            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s openmaxStandPort->pInternalBufferStorage[%u]=%p (portIndex=%d)", __func__,
                i, openmaxStandPort->pInternalBufferStorage[i], (int)openmaxStandPort->sPortParam.eDir);

            openmaxStandPort->bIsEmptyOfBuffers = OMX_FALSE;
            setHeader(openmaxStandPort->pInternalBufferStorage[i], sizeof(OMX_BUFFERHEADERTYPE));

            openmaxStandPort->pInternalBufferStorage[i]->pBuffer = pBuffer;
            openmaxStandPort->pInternalBufferStorage[i]->nAllocLen = nSizeBytes;
            openmaxStandPort->pInternalBufferStorage[i]->pPlatformPrivate = openmaxStandPort;
            openmaxStandPort->pInternalBufferStorage[i]->pAppPrivate = pAppPrivate;
            openmaxStandPort->bBufferStateAllocated[i] = BUFFER_ASSIGNED;
            openmaxStandPort->bBufferStateAllocated[i] |= HEADER_ALLOCATED;

            if (openmaxStandPort->sPortParam.eDir == OMX_DirOutput) {
                openmaxStandPort->pInternalBufferStorage[i]->pOutputPortPrivate = (OMX_PTR)malloc(sizeof(omx_usebuffer_display_info_t)); // to save display index for OMX_UseBuffer case
                if (openmaxStandPort->pInternalBufferStorage[i]->pOutputPortPrivate)
                {
                    ((omx_usebuffer_display_info_t *)openmaxStandPort->pInternalBufferStorage[i]->pOutputPortPrivate)->bToBeDisplay = OMX_FALSE;
                }
            }

            if (openmaxStandPort->sPortParam.eDir == OMX_DirInput) {
                openmaxStandPort->pInternalBufferStorage[i]->nInputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
            } else {
                openmaxStandPort->pInternalBufferStorage[i]->nOutputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
            }

            returnBufferHeader = openmaxStandPort->pInternalBufferStorage[i];
            *ppBufferHdr = returnBufferHeader;

            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s [%u] ppBufferHdr=%p, returnBufferHeader=%p (portIndex=%d), nNumAssignedBuffers %d\n", __func__,
                    i, ppBufferHdr, returnBufferHeader, (int)openmaxStandPort->sPortParam.eDir, openmaxStandPort->nNumAssignedBuffers);
            openmaxStandPort->nNumAssignedBuffers++;

            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s nBufferCountActual %d, now nNumAssignedBuffers %d\n",
                __func__, openmaxStandPort->sPortParam.nBufferCountActual, openmaxStandPort->nNumAssignedBuffers);
            if (openmaxStandPort->sPortParam.nBufferCountActual == openmaxStandPort->nNumAssignedBuffers) {
                openmaxStandPort->sPortParam.bPopulated = OMX_TRUE;
                openmaxStandPort->bIsFullOfBuffers = OMX_TRUE;
                vpuPort->bAllocateBuffer = OMX_FALSE;

                if (nPortIndex == OMX_BASE_FILTER_INPUTPORT_INDEX)
                {
                    omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer = OMX_FALSE;
                }

                tsem_up(openmaxStandPort->pAllocSem);
                if (omx_vpudec_component_Private->portSettingChangeRequest == OMX_TRUE)
                {
                    omx_vpudec_component_Private->portSettingChangeRequest = OMX_FALSE;
                    tsem_up(&omx_vpudec_component_Private->port_setting_change_tsem);
                }
            }

            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s portIndex=%d, nNumAssignedBuffers=%d, nSizeBytes=%d, pBuffer=%p\n", __func__, (int)openmaxStandPort->sPortParam.eDir, (int)openmaxStandPort->nNumAssignedBuffers, (int)nSizeBytes, openmaxStandPort->pInternalBufferStorage[i]->pBuffer);
            DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for port %p\n", __func__, openmaxStandPort);
            return OMX_ErrorNone;
        }

        DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Error: no available buffers CompName=%s\n",__func__,omx_base_component_Private->name);
        return OMX_ErrorInsufficientResources;
    }
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out %s for Component=%p, Port=%p, buffer %p\n", __func__, openmaxStandComp, openmaxStandPort, ppBufferHdr);

    return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_videodec_component_FreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{

    OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
    omx_base_component_PrivateType* omx_base_component_Private = (omx_base_component_PrivateType*)openmaxStandComp->pComponentPrivate;
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_base_PortType *openmaxStandPort;

    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s for component %p, port:%d, pBuffer=%p\n", __func__, hComponent, (int)nPortIndex, pBuffer);
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
        DEBUG(DEB_LEV_FUNCTION_NAME, "In %s for port %p, nNumAssignedBuffers=%d, nBufferCountActual %d\n", __func__, openmaxStandPort, (int)openmaxStandPort->nNumAssignedBuffers, openmaxStandPort->sPortParam.nBufferCountActual);

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

        for(i=0; i < openmaxStandPort->sPortParam.nBufferCountActual; i++)
        {
            if (openmaxStandPort->bBufferStateAllocated[i] & (BUFFER_ASSIGNED | BUFFER_ALLOCATED))  // OMX_AllocateBuffer or OMX_UseBuffer have been invoked.
            {
                openmaxStandPort->bIsFullOfBuffers = OMX_FALSE;
                if (openmaxStandPort->bBufferStateAllocated[i] & BUFFER_ALLOCATED) // OMX_AllocateBuffer has been invoked by IL Client.
                {
                    if(openmaxStandPort->pInternalBufferStorage[i] == pBuffer)
                    {
                        DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s freeing [%u] pBuffer=%p\n",__func__, i, openmaxStandPort->pInternalBufferStorage[i]->pBuffer);
                        if (openmaxStandPort->sPortParam.eDir == OMX_DirInput)
                        {
                            if (omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer == OMX_TRUE)
                            {
                                DEBUG(DEB_LEV_SIMPLE_SEQ, "freeing  vpu bitstream buffer pBuffer = %p, virt_addr=0x%x, size=%d\n", openmaxStandPort->pInternalBufferStorage[i]->pBuffer, (int)pVpu->vbStream[i].virt_addr, (int)pVpu->vbStream[i].size);
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
                            else
                            {
                                if (openmaxStandPort->pInternalBufferStorage[i]->pBuffer)
                                {
                                    free(openmaxStandPort->pInternalBufferStorage[i]->pBuffer);
                                    openmaxStandPort->pInternalBufferStorage[i]->pBuffer=NULL;
                                }
                            }
                        }
                        else    // for output
                        {
                            DEBUG(DEB_LEV_SIMPLE_SEQ, "freeing  vpu frame buffer useNativeBuffer=%d, pBuffer = %p, phys_addr=0x%x, virt_addr=0x%x, size=%d\n", (int)omx_vpudec_component_Private->useNativeBuffer, openmaxStandPort->pInternalBufferStorage[i]->pBuffer, (int)pVpu->vbAllocFb[i].phys_addr, (int)pVpu->vbAllocFb[i].virt_addr, (int)pVpu->vbAllocFb[i].size);

                            if (omx_vpudec_component_Private->useNativeBuffer == OMX_FALSE)
                            {
                                if (pVpu->vbAllocFb[i].size > 0)
                                {
                                    vdi_free_dma_memory(pVpu->coreIdx, &pVpu->vbAllocFb[i]);
                                    openmaxStandPort->pInternalBufferStorage[i]->pBuffer=NULL;
                                }
                                DEBUG(DEB_LEV_SIMPLE_SEQ, "freeing  vpu frame buffer done\n");
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
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "pInternalBufferStorage[%u]=%p, pBuffer=%p, pVpu->vbAllocFb[%u].size=%u\n",
                                i, openmaxStandPort->pInternalBufferStorage[i], pBuffer, i, pVpu->vbAllocFb[i].size);

                    if (openmaxStandPort->pInternalBufferStorage[i]->pBuffer != pBuffer->pBuffer)
                    {
                        DEBUG(DEB_LEV_FULL_SEQ, "!!!!!! buffer not match with index[%u] , port:%u\n", i, (int)nPortIndex);
                        continue;
                    }

                    DEBUG(DEB_LEV_SIMPLE_SEQ, "freeing pBuffer=%p, [%u], port:%u\n", pBuffer, i, (int)nPortIndex);
                    if(nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX && omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
                    {
                        int idx = i;
                        if (pVpu->coreIdx == VPU_WAVE412_IDX)
                            idx += openmaxStandPort->sPortParam.nBufferCountActual;
                        DEBUG(DEB_LEV_FULL_SEQ, "useNativeBuffer free idx %d, phy 0x%" PRIx64 ", size %u\n",
                            idx, pVpu->vbUserFb[idx].phys_addr, pVpu->vbUserFb[idx].size);
                        if (pVpu->vbUserFb[idx].size > 0) {
                            vdi_device_memory_unmap(pVpu->coreIdx, &pVpu->vbUserFb[idx]);
                            osal_memset(&pVpu->vbUserFb[idx], 0, sizeof(vpu_buffer_t));
                        }
                    }
                }

                if(openmaxStandPort->bBufferStateAllocated[i] & HEADER_ALLOCATED) {
                    if (openmaxStandPort->pInternalBufferStorage[i]) {
                        if (nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX)
                        {
                            if (openmaxStandPort->pInternalBufferStorage[i]->pOutputPortPrivate)
                            {
                                free(openmaxStandPort->pInternalBufferStorage[i]->pOutputPortPrivate);
                                openmaxStandPort->pInternalBufferStorage[i]->pOutputPortPrivate = NULL;
                            }
                        }
                        free(openmaxStandPort->pInternalBufferStorage[i]);
                        openmaxStandPort->pInternalBufferStorage[i]=NULL;
                    }
                }

                openmaxStandPort->bBufferStateAllocated[i] = BUFFER_FREE;

                openmaxStandPort->nNumAssignedBuffers--;
                DEBUG(DEB_LEV_SIMPLE_SEQ, "%s nNumAssignedBuffers %d nPortIndex %d\n", __func__, openmaxStandPort->nNumAssignedBuffers, nPortIndex);

                if (openmaxStandPort->nNumAssignedBuffers == 0) {
                    openmaxStandPort->sPortParam.bPopulated = OMX_FALSE;
                    openmaxStandPort->bIsEmptyOfBuffers = OMX_TRUE;
                    pVpu->seqInited = 0;
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "Out of %s change seqInit\n", __func__);
                    tsem_up(openmaxStandPort->pAllocSem);
                }
                DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for port %p, seqInit %d\n", __func__, openmaxStandPort, pVpu->seqInited);
                return OMX_ErrorNone;
            }
        }
        DEBUG(DEB_LEV_SIMPLE_SEQ, "Out of %s for port %p with OMX_ErrorInsufficientResources\n", __func__, openmaxStandPort);
        return OMX_ErrorInsufficientResources;
    }
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for component %p\n", __func__, hComponent);
    return OMX_ErrorInsufficientResources;

}


OMX_BOOL OmxGetVpuBsBufferByVirtualAddress(OMX_COMPONENTTYPE *openmaxStandComp, vpu_buffer_t *vb, OMX_BUFFERHEADERTYPE *pInputBuffer)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *inPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    int i;

    for (i = 0; i < (int)inPort->nNumAssignedBuffers; i++)
    {
        if (inPort->pInternalBufferStorage[i] == pInputBuffer)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "bitstream buffer for this frame index = %d, pInputBuffer=%p, phys_addr=0x%x, virt_addr=0x%x size=%d\n",
                i, inPort->pInternalBufferStorage[i], (int)pVpu->vbStream[i].phys_addr, (int)pVpu->vbStream[i].virt_addr, (int)pVpu->vbStream[i].size);

            *vb = pVpu->vbStream[i];
            return OMX_TRUE;
        }

    }
    return OMX_FALSE;
}

int OmxWriteBsBufFromBufHelper(OMX_COMPONENTTYPE *openmaxStandComp, vpu_dec_context_t *pVpu, vpu_buffer_t vbStream, BYTE *pChunk,  int chunkSize)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    RetCode ret = RETCODE_SUCCESS;
    Uint32 room = 0;

    PhysicalAddress paRdPtr, paWrPtr;

    if (chunkSize < 1)
    {
        DEBUG(DEB_LEV_ERR, "Invalid chunkSize = %d\n", (int)chunkSize);
        return -1;
    }

    if (chunkSize > (int)vbStream.size)
    {
        DEBUG(DEB_LEV_ERR, "chunkSize is larger than  bitstream buffer size, chunksize=%d, bitstream buffer size=%d\n", (int)chunkSize, (int)vbStream.size);
        return -1;
    }

    ret = VPU_DecGetBitstreamBuffer(pVpu->handle, &paRdPtr, &paWrPtr, &room);
    if( ret != RETCODE_SUCCESS )
    {
        DEBUG(DEB_LEV_ERR, "VPU_DecGetBitstreamBuffer failed Error code is 0x%x \n", ret );
        return -1;
    }

    if(room < (Uint32)chunkSize)
    {
        DEBUG(DEB_LEV_FULL_SEQ, "no room for feeding bitstream. it will take a change to fill stream\n");
        return 0; // no room for feeding bitstream. it will take a change to fill stream
    }


    DEBUG(DEB_LEV_FULL_SEQ, "OmxWriteBsBufFromBufHelper from vbStream.phys_addr=0x%x, chunkSize=%d, to paWrPtr=0x%x, room=%d, bUseOmxInputBufferAsDecBsBuffer=%d\n",
        (int)vbStream.phys_addr,  chunkSize, (int)paWrPtr, (int)room, (int)omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer);

    if (omx_vpudec_component_Private->bUseOmxInputBufferAsDecBsBuffer == OMX_FALSE)
    {
        vdi_write_memory(pVpu->coreIdx, paWrPtr, pChunk, chunkSize, pVpu->decOP.streamEndian);
    }

    ret = VPU_DecUpdateBitstreamBuffer(pVpu->handle, chunkSize);
    if( ret != RETCODE_SUCCESS )
    {
        DEBUG(DEB_LEV_FULL_SEQ, "VPU_DecUpdateBitstreamBuffer failed Error code is 0x%x \n", ret );
        return -1;
    }

    return chunkSize;
}


void OmxConfigFrameBufferCount(omx_vpudec_component_PrivateType* omx_vpudec_component_Private)
{
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];

    // setting buffer count for default option
    pVpu->decFbCount =  pVpu->initialInfo.minFrameBufferCount + OMX_EXTRA_FRAME_BUFFER_NUM;
    pVpu->dispFbCount = pVpu->initialInfo.minFrameBufferCount + OMX_EXTRA_FRAME_BUFFER_NUM;
    outPort->sPortParam.nBufferCountMin = pVpu->initialInfo.minFrameBufferCount > outPort->sPortParam.nBufferCountMin ?
                                             pVpu->initialInfo.minFrameBufferCount : outPort->sPortParam.nBufferCountMin;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "%s decFbCount %d, displayFbCount %d, scaler %d\n", __func__, pVpu->decFbCount, pVpu->dispFbCount, pVpu->scalerInfo.enScaler);

    if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
    {
        pVpu->decFbCount += NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS;
        pVpu->dispFbCount += NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS; // set DPB count as many as display buffer count
        DEBUG(DEB_LEV_SIMPLE_SEQ, "%s add undequeubuffer, %d, %d\n", __func__, pVpu->decFbCount, pVpu->dispFbCount);
    }

    pVpu->decFbCount =  pVpu->decFbCount < DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM ? DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM : pVpu->decFbCount;
    pVpu->dispFbCount =  pVpu->dispFbCount < DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM ? DEFAULT_MIN_VIDEO_OUTPUT_BUFFER_NUM : pVpu->dispFbCount;

    // for format vp9, the display buf index is limited to 16bit, the buf size should limit to 16,
    // acodec will add andother 5 buf, so we need to limit the number of bufs to 11.
    if (pVpu->decOP.bitstreamFormat == STD_VP9)
    {
        pVpu->decFbCount =  pVpu->decFbCount > VP9_MAX_FRAME_BUFFER_NUM ? VP9_MAX_FRAME_BUFFER_NUM : pVpu->decFbCount;
        pVpu->dispFbCount =  pVpu->dispFbCount > VP9_MAX_FRAME_BUFFER_NUM ? VP9_MAX_FRAME_BUFFER_NUM : pVpu->dispFbCount;
    }

    pVpu->regFbCount = pVpu->dispFbCount;

    if (omx_vpudec_component_Private->bThumbnailMode == OMX_TRUE)
    {
        pVpu->dispFbCount = (int)outPort->sPortParam.nBufferCountActual; // set only one buffer
        pVpu->decFbCount = pVpu->initialInfo.minFrameBufferCount;   // regardless OMX_UseBuffer and OMX_AllocateBuffer. thumbnailmode can set minimum buffer count let decoder to act as normal decoding. but need allocated buffer count is just 1
        pVpu->regFbCount = pVpu->initialInfo.minFrameBufferCount;   // no extra display buffer count required
        DEBUG(DEB_LEV_SIMPLE_SEQ, "thumbnail mode  pVpu->regFbCount=%d, pVpu->decFbCount=%d\n", pVpu->regFbCount, pVpu->decFbCount);
    }
}


#ifdef SUPPORT_NALU_FORMAT_BY_OMX
OMX_BOOL OmxPackNaluToAU(omx_vpudec_component_PrivateType* omx_vpudec_component_Private, OMX_BUFFERHEADERTYPE* pInputBuffer)
{
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    OMX_BOOL bPackAU = OMX_FALSE;
    static OMX_S32 auSize;
    OMX_U8 *ptr;

    if (pInputBuffer->nFlags & OMX_BUFFERFLAG_ENDOFSUBFRAME)
    {
        ptr = omx_vpudec_component_Private->picHeader + auSize;
        memcpy(ptr, pInputBuffer->pBuffer, pInputBuffer->nFilledLen);
        ptr[0] = 0x00;
        ptr[1] = 0x00;
        ptr[2] = 0x00;
        ptr[3] = 0x01;
        auSize += pInputBuffer->nFilledLen;
        bPackAU = OMX_FALSE;
    }

    if (pInputBuffer->nFlags & OMX_BUFFERFLAG_ENDOFFRAME)
    {
        if (auSize > 0)
        {
            memcpy(pInputBuffer->pBuffer, omx_vpudec_component_Private->picHeader, auSize);
            pInputBuffer->nFilledLen = auSize;
        }

        auSize = 0;
        bPackAU = OMX_TRUE;
    }
    return bPackAU;
}
#endif


OMX_U32 OmxGetVpuFrameRate(omx_vpudec_component_PrivateType* omx_vpudec_component_Private)
{
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    OMX_U32 framerate;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "OmxGetVpuFrameRate fRateNumerator=%d, fRateDenominator=%d\n", pVpu->initialInfo.fRateNumerator, pVpu->initialInfo.fRateDenominator);

    framerate = 0;

    if (pVpu->initialInfo.fRateDenominator <= 0)
        return 0;

    switch(omx_vpudec_component_Private->video_coding_type)
    {
    case OMX_VIDEO_CodingAVC:
    case OMX_VIDEO_CodingHEVC:
    case OMX_VIDEO_CodingAVS:
    case OMX_VIDEO_CodingMPEG2:
    case OMX_VIDEO_CodingMPEG4:
    case OMX_VIDEO_CodingH263:
    case OMX_VIDEO_CodingRV:
    case OMX_VIDEO_CODINGTYPE_VP8:
        framerate = (OMX_U32)(pVpu->initialInfo.fRateNumerator/pVpu->initialInfo.fRateDenominator);
        break;
    case OMX_VIDEO_CodingVC1:
    case OMX_VIDEO_CodingWMV:
        if (pVpu->initialInfo.profile == 2 && pVpu->initialInfo.fRateNumerator < 8) // AP
        {
            const int TBL_FRAME_RATE_NR[8] = {0, 24, 25, 30, 50, 60, 48, 72};
            framerate = (OMX_U32)(TBL_FRAME_RATE_NR[pVpu->initialInfo.fRateNumerator] * 100)/(pVpu->initialInfo.fRateDenominator + 999);
        }
        else
        {
            framerate = (OMX_U32)((pVpu->initialInfo.fRateNumerator+1)/pVpu->initialInfo.fRateDenominator);
        }
        break;
    default:
        framerate = 0;
        break;
    }


    if (framerate > (OMX_U32)120)
    {
        framerate = 0; // it means we bypass the timestamp of inputbuffer to outpubuffer without timestamp corretion logic.
    }

    return framerate;
}


OMX_TICKS OmxTimeStampCorrection(OMX_COMPONENTTYPE *openmaxStandComp, OMX_TICKS nInputTimeStamp)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    omx_vpudec_component_PortType *inPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_timestamp_correction_t *tsc = &omx_vpudec_component_Private->omx_ts_correction;
    OMX_TICKS nTimeStamp;


    nTimeStamp = nInputTimeStamp;
    tsc->mFrameNumber++;

    DEBUG(DEB_LEV_FULL_SEQ, "%s of component : OmxTimeStampCorrection mFrameNumber=%lld, nInputTimeStamp=%lld, nOutputTimeStamp=%lld, xFramerate=%d\n", __func__, tsc->mFrameNumber, nInputTimeStamp, nTimeStamp, (int)inPort->sPortParam.format.video.xFramerate);

    return nTimeStamp;
}


OMX_BOOL OmxClearDisplayFlag(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE *pBuffer, OMX_BOOL bFillThisBufer)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    int i;
    omx_display_flag *disp_flag_array = &omx_vpudec_component_Private->omx_display_flags[0];
    int dispFlagIdx;

    DEBUG(DEB_LEV_FULL_SEQ, "%s enter, %d\n", __func__, bFillThisBufer);
    pthread_mutex_lock(&omx_vpudec_component_Private->display_flag_mutex);
    if(outPort->bAllocateBuffer == OMX_TRUE || omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
    {
        for (i = 0; i < (int)outPort->sPortParam.nBufferCountActual; i++)
        {
            if (outPort->pInternalBufferStorage[i] == pBuffer)
            {

                if (bFillThisBufer == OMX_TRUE)
                    disp_flag_array[i].owner = OMX_BUFERR_OWNER_COMPONENT;
                else // FillBufferDone
                    disp_flag_array[i].owner = OMX_BUFFER_OWNER_CLIENT;

                if(PORT_IS_BEING_FLUSHED(outPort) || (PORT_IS_BEING_DISABLED(outPort) && PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(outPort)))
                {
                    // no action required to VPU
                    DEBUG(DEB_LEV_FULL_SEQ, "%s of component no action required to VPU\n", __func__);
                }
                else
                {
                    if (pVpu->handle)
                    {

                        dispFlagIdx = i;
                        if (bFillThisBufer == OMX_TRUE)
                        {
                            VPU_DecClrDispFlag(pVpu->handle, dispFlagIdx);
                            DEBUG(DEB_LEV_FULL_SEQ, "%s of component VPU_DecClrDispFlag for OMX_AllocBuffer and useNativeBuffer case index=%d, dispFlagIdx=%d, owner=0x%x\n", __func__,  i, dispFlagIdx, disp_flag_array[i].owner);
                        }
                        else
                        {
                            DEBUG(DEB_LEV_FULL_SEQ, "%s of component no VPU_DecClrDispFlag for OMX_AllocBuffer and useNativeBuffer case index=%d, dispFlagIdx=%d, owner=0x%x\n", __func__,  i, dispFlagIdx, disp_flag_array[i].owner);
                            // SHOULD not set to display flag in hear by HOST. because the owner of setting this flag must be firmware to take case of reference frame.
                            // if this index is reference frame at current. firmware do not use this index for decoding. and firmware remain this flag to 0.
                            // but if host set this flag to 1. decoder will not use this index forever after this index will be available time to decode. ( after output the other buffers )
                        }
                    }
                }

                break;
            }
        }
    }
    else    // OMX_UseBuffer case
    {
        if(PORT_IS_BEING_FLUSHED(outPort) || (PORT_IS_BEING_DISABLED(outPort) && PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(outPort)))
        {
            // no action required to VPU
            DEBUG(DEB_LEV_FULL_SEQ, "%s of component no action required to VPU\n", __func__);
        }
        else
        {
            if (pBuffer->pOutputPortPrivate)
            {
                if (bFillThisBufer == OMX_TRUE)
                {
                    dispFlagIdx = ((omx_usebuffer_display_info_t *)pBuffer->pOutputPortPrivate)->dispFlagIdx;
                    if (((omx_usebuffer_display_info_t *)pBuffer->pOutputPortPrivate)->bToBeDisplay == OMX_TRUE)
                        VPU_DecClrDispFlag(pVpu->handle, dispFlagIdx);
                    DEBUG(DEB_LEV_FULL_SEQ, "%s of component VPU_DecClrDispFlag for OMX_UseBuffer case dispFlagIdx=%d\n", __func__,  dispFlagIdx);
                }
                else
                {
                    DEBUG(DEB_LEV_FULL_SEQ, "%s of component no VPU_DecClrDispFlag for OMX_UseBuffer", __func__);
                    ((omx_usebuffer_display_info_t *)pBuffer->pOutputPortPrivate)->bToBeDisplay = OMX_FALSE;
                    // SHOULD not set to display flag in hear by HOST. because the owner of setting this flag must be firmware to take case of reference frame.
                    // if this index is reference frame at current. firmware do not use this index for decoding. and firmware remain this flag to 0.
                    // but if host set this flag to 1. decoder will not use this index forever after this index will be available time to decode. ( after output the other buffers )
                }
            }
        }
    }

    pthread_mutex_unlock(&omx_vpudec_component_Private->display_flag_mutex);

    return OMX_TRUE;
}


OMX_BOOL OmxUpdateOutputBufferHeaderToDisplayOrder(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE **ppOutputBuffer)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    omx_display_flag *disp_flag_array = &omx_vpudec_component_Private->omx_display_flags[0];
    queue_t* pOutputQueue = outPort->pBufferQueue;
    int omxBufferIndex = 0;
    OMX_U32 nOutputLen = 0;
    int i = 0;

    DEBUG(DEB_LEV_FUNCTION_NAME, "%s of component : dispIdx = %d, nFieldLen = %d, pOutputBuffer=%p, PORT_IS_BEING_FLUSHED=%d, PORT_IS_BEING_DISABLED=%d, state=0x%x\n",
        __func__, pVpu->dispOutputInfo.indexFrameDisplay, (int)(*ppOutputBuffer)->nFilledLen, *ppOutputBuffer, (int)PORT_IS_BEING_FLUSHED(outPort), (int)PORT_IS_BEING_DISABLED(outPort), omx_vpudec_component_Private->state);

    pthread_mutex_lock(&omx_vpudec_component_Private->flush_mutex);
    if(PORT_IS_BEING_FLUSHED(outPort) || (PORT_IS_BEING_DISABLED(outPort) && PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(outPort)) || //  to match with calling OmxSetDisplayFlag in omx_vpudec_component_SendBufferFunction
        pVpu->dispOutputInfo.indexFrameDisplay < 0)
    {
        pthread_mutex_unlock(&omx_vpudec_component_Private->flush_mutex);
        pthread_mutex_lock(&omx_vpudec_component_Private->display_flag_mutex);
        for (i = 0; i < (int)outPort->sPortParam.nBufferCountActual; i++)
        {
            if (disp_flag_array[i].owner == OMX_BUFERR_OWNER_COMPONENT)
            {
                DEBUG(DEB_LEV_FULL_SEQ, "%s of component Found abnormal OutputBuffer for FillBufferDone  index=%d, pBuffer=%p, pInternalBufferStorage=%p prev omx_display_flag owner=0x%x\n",
                    __func__,  i, *ppOutputBuffer, outPort->pInternalBufferStorage[i], disp_flag_array[i].owner );

                *ppOutputBuffer = outPort->pInternalBufferStorage[i];
                (*ppOutputBuffer)->nFilledLen = 0;
                (*ppOutputBuffer)->nTimeStamp = 0;
                break;
            }
        }
        pthread_mutex_unlock(&omx_vpudec_component_Private->display_flag_mutex);
        return OMX_TRUE;
    }

    pthread_mutex_unlock(&omx_vpudec_component_Private->flush_mutex);
    omxBufferIndex = pVpu->dispOutputInfo.indexFrameDisplay;

    DEBUG(DEB_LEV_FULL_SEQ, "%s of component : Found normal OutputBuffer for FillBufferDone dispIdx = %d, omxBufferIndex=%d, ppOutputBuffer=%p, pInternalBufferStorage=%p, picType=%d\n", __func__,
        pVpu->dispOutputInfo.indexFrameDisplay, omxBufferIndex, *ppOutputBuffer, outPort->pInternalBufferStorage[omxBufferIndex], disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].picType);

    if (ppOutputBuffer && *ppOutputBuffer)
    {
        nOutputLen = (*ppOutputBuffer)->nFilledLen;

        if (*ppOutputBuffer != outPort->pInternalBufferStorage[omxBufferIndex])
        {
            //1.queue the outputBuffer which will not return to omx client
            (*ppOutputBuffer)->nFilledLen = 0;
            (*ppOutputBuffer)->nFlags = 0;

            int errQue = queue(pOutputQueue, *ppOutputBuffer);
            if (errQue)
            {
                DEBUG(DEB_LEV_ERR, "[%s]:[%d] queue is full\n", __func__, __LINE__);
                return OMX_FALSE;
            }

            //2.found the real outputBuffer what we will return from the queue
            for (i = 0; i < (int)outPort->sPortParam.nBufferCountActual; i++)
            {
                *ppOutputBuffer = dequeue(pOutputQueue);

                if (*ppOutputBuffer == outPort->pInternalBufferStorage[omxBufferIndex])
                    break;
                else
                {
                    errQue = queue(pOutputQueue, *ppOutputBuffer);
                    if (errQue)
                    {
                        DEBUG(DEB_LEV_ERR, "[%s]:[%d] queue is full\n", __func__, __LINE__);
                        return OMX_FALSE;
                    }
                }
            }

            if (i == (int)outPort->sPortParam.nBufferCountActual)
            {
                DEBUG(DEB_LEV_ERR, "[%s]:[%d] coundn't found real outputBuffer from the queu\n",
                    __func__, __LINE__);
                return OMX_FALSE;
            }
        }

        (*ppOutputBuffer)->nFilledLen = nOutputLen;

        if (disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].picType == PIC_TYPE_I ||
            disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].picType == PIC_TYPE_IDR)
            (*ppOutputBuffer)->nFlags = OMX_BUFFERFLAG_SYNCFRAME;
        else
            (*ppOutputBuffer)->nFlags = 0;
    }

    return OMX_TRUE;
}


omx_bufferheader_queue_item_t* omx_bufferheader_queue_init(int count)
{
    omx_bufferheader_queue_item_t* queue = NULL;

    queue = (omx_bufferheader_queue_item_t *)osal_malloc(sizeof(omx_bufferheader_queue_item_t));
    if (!queue) {
        DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
        return NULL;
    }

    queue->size   = count;
    queue->count  = 0;
    queue->front  = 0;
    queue->rear   = 0;
    queue->buffer = (OMX_BUFFERHEADERTYPE*)osal_malloc(count*sizeof(OMX_BUFFERHEADERTYPE));
    if (!queue->buffer) {
        DEBUG(DEB_LEV_ERR, "Insufficient memory in %s\n", __func__);
        free(queue);
        queue = NULL;
        return NULL;
    }
    pthread_mutex_init(&queue->mutex, NULL);
    return queue;
}

void omx_bufferheader_queue_deinit(omx_bufferheader_queue_item_t* queue)
{
    if (queue == NULL)
        return;

    if (queue->buffer)
        osal_free(queue->buffer);

    pthread_mutex_destroy(&queue->mutex);

    osal_free(queue);
}


/*
* Return 0 on success.
*      -1 on failure
*/
int omx_bufferheader_queue_enqueue(omx_bufferheader_queue_item_t* queue, OMX_BUFFERHEADERTYPE* data)
{
    if (queue == NULL)
        return -1;

    pthread_mutex_lock(&queue->mutex);
    /* Queue is full */
    if (queue->count == queue->size)
    {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    memcpy(&queue->buffer[queue->rear++], data, sizeof(OMX_BUFFERHEADERTYPE));
    queue->rear %= queue->size;
    queue->count++;
    pthread_mutex_unlock(&queue->mutex);

    return 0;
}


/*
* Return 0 on success.
*      -1 on failure
*/
int omx_bufferheader_queue_dequeue(omx_bufferheader_queue_item_t* queue, OMX_BUFFERHEADERTYPE* data)
{
    if (queue == NULL)
        return -1;

    pthread_mutex_lock(&queue->mutex);
    /* Queue is empty */
    if (queue->count == 0)
    {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }
    if (data)
        memcpy(data, &queue->buffer[queue->front], sizeof(OMX_BUFFERHEADERTYPE));
    queue->front++;
    queue->front %= queue->size;
    queue->count--;
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}


int omx_bufferheader_queue_dequeue_all(omx_bufferheader_queue_item_t* queue)
{
    int ret;
    OMX_BUFFERHEADERTYPE data;
    if (queue == NULL) return -1;
    do
    {
        ret = omx_bufferheader_queue_dequeue(queue, &data);
    } while (ret >= 0);
    return 0;
}

OMX_ERRORTYPE omx_vpudec_component_flush_port_buffer(
    omx_vpudec_component_PortType* pPort,
    OMX_BUFFERHEADERTYPE** ppBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    tsem_t* pSem = pPort->pBufferSem;
    queue_t* pQueue = pPort->pBufferQueue;
    OMX_BUFFERHEADERTYPE* pBuffer = *ppBuffer;

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

/** @brief the entry point for sending buffers to the port
*
* This function can be called by the FillThisBuffer. It depends on
* the nature of the port, that can be an output port.
# vpudec_component override this function to customize for vpu decoder  control
*/
OMX_ERRORTYPE omx_vpudec_component_SendBufferFunction(omx_base_PortType *openmaxStandPort, OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE err;
    int errQue;
    OMX_U32 portIndex;

    OMX_COMPONENTTYPE* omxComponent = openmaxStandPort->standCompContainer;
    omx_base_component_PrivateType* omx_base_component_Private = (omx_base_component_PrivateType*)omxComponent->pComponentPrivate;
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = omxComponent->pComponentPrivate;

    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s for port %p\n", __func__, openmaxStandPort);
#if NO_GST_OMX_PATCH
    unsigned int i;
#endif
    portIndex = (openmaxStandPort->sPortParam.eDir == OMX_DirInput)?pBuffer->nInputPortIndex:pBuffer->nOutputPortIndex;
    DEBUG(DEB_LEV_FULL_SEQ, "In %s portIndex %d\n", __func__, portIndex);

    if (portIndex != openmaxStandPort->sPortParam.nPortIndex) {
        DEBUG(DEB_LEV_ERR, "In %s: wrong port for this operation portIndex=%d port->portIndex=%d\n", __func__, (int)portIndex, (int)openmaxStandPort->sPortParam.nPortIndex);
        return OMX_ErrorBadPortIndex;
    }
#ifdef SUPPORT_CM_OMX_12
#else
    if(omx_base_component_Private->state == OMX_StateInvalid) {
        DEBUG(DEB_LEV_ERR, "In %s: we are in OMX_StateInvalid\n", __func__);
        return OMX_ErrorInvalidState;
    }
#endif

    if(omx_base_component_Private->state != OMX_StateExecuting &&
        omx_base_component_Private->state != OMX_StatePause &&
        omx_base_component_Private->state != OMX_StateIdle) {
            DEBUG(DEB_LEV_ERR, "In %s: we are not in executing/paused/idle state, but in %d\n", __func__, omx_base_component_Private->state);
            return OMX_ErrorIncorrectStateOperation;
    }
    if (!PORT_IS_ENABLED(openmaxStandPort) || (PORT_IS_BEING_DISABLED(openmaxStandPort) && !PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(openmaxStandPort)) ||
        ((omx_base_component_Private->transientState == OMX_TransStateExecutingToIdle ||
        omx_base_component_Private->transientState == OMX_TransStatePauseToIdle) &&
        (PORT_IS_TUNNELED(openmaxStandPort) && !PORT_IS_BUFFER_SUPPLIER(openmaxStandPort)))) {
            DEBUG(DEB_LEV_ERR, "In %s: Port %d is disabled comp = %s \n", __func__, (int)portIndex,omx_base_component_Private->name);
            return OMX_ErrorIncorrectStateOperation;
    }

    /* Temporarily disable this check for gst-openmax */
#if NO_GST_OMX_PATCH
    {
        OMX_BOOL foundBuffer = OMX_FALSE;
        if(pBuffer!=NULL && pBuffer->pBuffer!=NULL) {
            for(i=0; i < openmaxStandPort->sPortParam.nBufferCountActual; i++){
                if (pBuffer->pBuffer == openmaxStandPort->pInternalBufferStorage[i]->pBuffer) {
                    foundBuffer = OMX_TRUE;
                    break;
                }
            }
        }
        if (!foundBuffer) {
            return OMX_ErrorBadParameter;
        }
    }
#endif

    if ((err = checkHeader(pBuffer, sizeof(OMX_BUFFERHEADERTYPE))) != OMX_ErrorNone) {
        DEBUG(DEB_LEV_ERR, "In %s: received wrong buffer header on input port\n", __func__);
        return err;
    }
    if (openmaxStandPort->sPortParam.eDir == OMX_DirOutput) {
        DEBUG(DEB_LEV_SIMPLE_SEQ, "%s OmxClearDisplayFlag\n", __func__);
        OmxClearDisplayFlag(omxComponent, pBuffer, OMX_TRUE);
    }

    /* And notify the buffer management thread we have a fresh new buffer to manage */
    if(!PORT_IS_BEING_FLUSHED(openmaxStandPort) && !(PORT_IS_BEING_DISABLED(openmaxStandPort) && PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(openmaxStandPort)))
    {
        if (openmaxStandPort->sPortParam.eDir == OMX_DirInput)
        {
            if ((pBuffer->nFlags&OMX_BUFFERFLAG_CODECCONFIG) == OMX_BUFFERFLAG_CODECCONFIG ||
                (pBuffer->nFlags&OMX_BUFFERFLAG_DECODEONLY) == OMX_BUFFERFLAG_DECODEONLY)
            {
                // do not need to insert InputBuffer in queue. this queue is needed to give display information.
                DEBUG(DEB_LEV_FULL_SEQ, "inputBuffer has OMX_BUFFERFLAG_CODECCONFIG in pBuffer->nFlags=0x%x\n", pBuffer->nFlags);
            }
        }

        errQue = queue(openmaxStandPort->pBufferQueue, pBuffer);
        if (errQue) {
            /* /TODO the queue is full. This can be handled in a fine way with
            * some retrials, or other checking. For the moment this is a critical error
            * and simply causes the failure of this call
            */
            return OMX_ErrorInsufficientResources;
        }

        tsem_up(openmaxStandPort->pBufferSem);

        DEBUG(DEB_LEV_FULL_SEQ, "In %s Signalling bMgmtSem Port Index=%d, pBuffer=%p\n",__func__, (int)portIndex, pBuffer);
        tsem_up(omx_base_component_Private->bMgmtSem);

        if (openmaxStandPort->sPortParam.eDir == OMX_DirOutput)
        {
            tsem_up(&omx_vpudec_component_Private->disp_Buf_full_tsem);
        }
    }
    else if(PORT_IS_BUFFER_SUPPLIER(openmaxStandPort)){
        DEBUG(DEB_LEV_FULL_SEQ, "In %s: Comp %s received io:%d buffer\n",
            __func__,omx_base_component_Private->name,(int)openmaxStandPort->sPortParam.nPortIndex);
        errQue = queue(openmaxStandPort->pBufferQueue, pBuffer);
        if (errQue) {
            /* /TODO the queue is full. This can be handled in a fine way with
            * some retrials, or other checking. For the moment this is a critical error
            * and simply causes the failure of this call
            */
            return OMX_ErrorInsufficientResources;
        }

        tsem_up(openmaxStandPort->pBufferSem);
    }
    else { // If port being flushed and not tunneled then return error
        DEBUG(DEB_LEV_FULL_SEQ, "In %s \n", __func__);
        return OMX_ErrorIncorrectStateOperation;
    }
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for port %p\n", __func__, openmaxStandPort);
    return OMX_ErrorNone;
}


/**
* Returns Input/Output Buffer to the IL client or Tunneled Component
*/
OMX_ERRORTYPE omx_vpudec_component_OutPort_ReturnBufferFunction(omx_base_PortType* openmaxStandPort,OMX_BUFFERHEADERTYPE* pBuffer)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandPort->standCompContainer->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *vpuPort= (omx_vpudec_component_PortType *)openmaxStandPort;
    queue_t* pQueue = openmaxStandPort->pBufferQueue;
    tsem_t* pSem = openmaxStandPort->pBufferSem;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    int errQue;
    omx_display_flag *disp_flag_array = &omx_vpudec_component_Private->omx_display_flags[0];

    OMX_BUFFERHEADERTYPE inBufferHeader;
    OMX_BOOL bDecoderEOS = OMX_FALSE;


    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s for port %p\n", __func__, openmaxStandPort);
    if (PORT_IS_TUNNELED(openmaxStandPort) && ! PORT_IS_BUFFER_SUPPLIER(openmaxStandPort)) {
        if (openmaxStandPort->sPortParam.eDir == OMX_DirInput) {
            pBuffer->nOutputPortIndex = openmaxStandPort->nTunneledPort;
            pBuffer->nInputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
            eError = ((OMX_COMPONENTTYPE*)(openmaxStandPort->hTunneledComponent))->FillThisBuffer(openmaxStandPort->hTunneledComponent, pBuffer);
            if(eError != OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s eError %08x in FillThis Buffer from Component %s Non-Supplier\n",
                    __func__, eError,omx_vpudec_component_Private->name);
            }
        } else {
            pBuffer->nInputPortIndex = openmaxStandPort->nTunneledPort;
            pBuffer->nOutputPortIndex = openmaxStandPort->sPortParam.nPortIndex;
            eError = ((OMX_COMPONENTTYPE*)(openmaxStandPort->hTunneledComponent))->EmptyThisBuffer(openmaxStandPort->hTunneledComponent, pBuffer);
            if(eError != OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s eError %08x in EmptyThis Buffer from Component %s Non-Supplier\n",
                    __func__, eError,omx_vpudec_component_Private->name);
            }
        }
    } else if (PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(openmaxStandPort) &&
        !PORT_IS_BEING_FLUSHED(openmaxStandPort)) {
        if (openmaxStandPort->sPortParam.eDir == OMX_DirInput) {
            eError = ((OMX_COMPONENTTYPE*)(openmaxStandPort->hTunneledComponent))->FillThisBuffer(openmaxStandPort->hTunneledComponent, pBuffer);
            if(eError != OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s eError %08x in FillThis Buffer from Component %s Supplier\n",
                    __func__, eError,omx_vpudec_component_Private->name);
                /*If Error Occured then queue the buffer*/
                errQue = queue(pQueue, pBuffer);
                if (errQue) {
                    /* /TODO the queue is full. This can be handled in a fine way with
                    * some retrials, or other checking. For the moment this is a critical error
                    * and simply causes the failure of this call
                    */
                    return OMX_ErrorInsufficientResources;
                }
                tsem_up(pSem);
            }
        } else {
            eError = ((OMX_COMPONENTTYPE*)(openmaxStandPort->hTunneledComponent))->EmptyThisBuffer(openmaxStandPort->hTunneledComponent, pBuffer);
            if(eError != OMX_ErrorNone) {
                DEBUG(DEB_LEV_ERR, "In %s eError %08x in EmptyThis Buffer from Component %s Supplier\n",
                    __func__, eError,omx_vpudec_component_Private->name);
                /*If Error Occured then queue the buffer*/
                errQue = queue(pQueue, pBuffer);
                if (errQue) {
                    /* /TODO the queue is full. This can be handled in a fine way with
                    * some retrials, or other checking. For the moment this is a critical error
                    * and simply causes the failure of this call
                    */
                    return OMX_ErrorInsufficientResources;
                }
                tsem_up(pSem);
            }
        }
    } else if (!PORT_IS_TUNNELED(openmaxStandPort)){

        bDecoderEOS = (OMX_BOOL)((pBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS); // that is real EOS flag which is got from decoder.
        DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s bDecoderEOS %d, bAllocateBuffer %d, useNativeBuffer %d\n",
            __func__, bDecoderEOS, vpuPort->bAllocateBuffer, omx_vpudec_component_Private->useNativeBuffer);
        if (vpuPort->bAllocateBuffer == OMX_TRUE || omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
        {
            DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s OmxUpdateOutputBufferHeaderToDisplayOrder\n", __func__);
            OmxUpdateOutputBufferHeaderToDisplayOrder((OMX_COMPONENTTYPE *)openmaxStandPort->standCompContainer, &pBuffer);
            OmxClearDisplayFlag((OMX_COMPONENTTYPE *)openmaxStandPort->standCompContainer, pBuffer, OMX_FALSE);
        }
        else
        {
            if (disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].picType == PIC_TYPE_I ||
                disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].picType == PIC_TYPE_IDR)
                pBuffer->nFlags = OMX_BUFFERFLAG_SYNCFRAME;
            else
                pBuffer->nFlags = 0;
        }

        pthread_mutex_lock(&omx_vpudec_component_Private->flush_mutex);
        if(PORT_IS_BEING_FLUSHED(vpuPort) || (PORT_IS_BEING_DISABLED(vpuPort) && PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(vpuPort)))
        {
            pthread_mutex_unlock(&omx_vpudec_component_Private->flush_mutex);
        }
        else
        {
            pthread_mutex_unlock(&omx_vpudec_component_Private->flush_mutex);
            if (omx_bufferheader_queue_dequeue(omx_vpudec_component_Private->in_bufferheader_queue, &inBufferHeader) == -1)
            {
                DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s No Input bufferHeader Found : nFieldLen = %d, pOutputBuffer=%p, nTimeStamp=%lld, PORT_IS_BEING_FLUSHED=%d, PORT_IS_BEING_DISABLED=%d, state=0x%x\n",
                    __func__, (int)pBuffer->nFilledLen, pBuffer, pBuffer->nTimeStamp, (int)PORT_IS_BEING_FLUSHED(openmaxStandPort), (int)PORT_IS_BEING_DISABLED(openmaxStandPort), omx_vpudec_component_Private->state);

                if (bDecoderEOS)
                {
                    pBuffer->nTimeStamp = 0;
                    pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Detect Real EOS in OutputBuffer nTimeStamp=%lld, nFlag=0x%x, \n", __func__, pBuffer->nTimeStamp, (int)pBuffer->nFlags);
                }
                else
                {
                    DEBUG(DEB_LEV_FULL_SEQ, "In %s lastInputStamp %lld, lastTimeStamp %lld", __func__, omx_vpudec_component_Private->lastInputStamp, omx_vpudec_component_Private->lastTimeStamp);
                    pBuffer->nTimeStamp = omx_vpudec_component_Private->lastTimeStamp;
                }
            }
            else
            {
                DEBUG(DEB_LEV_FULL_SEQ, "pInputBufferTimeStamp dequeue timestamp=%lld, indexFrameDisplay=%d, bIsTimeStampReorder=%d\n", inBufferHeader.nTimeStamp, pVpu->dispOutputInfo.indexFrameDisplay, (int)omx_vpudec_component_Private->bIsTimeStampReorder);

                if (bDecoderEOS)
                {
                    pBuffer->nTimeStamp = 0;
                    pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Detect Real EOS in OutputBuffer nTimeStamp=%lld, nFlag=0x%x, \n", __func__, pBuffer->nTimeStamp, (int)pBuffer->nFlags);
                }
                else
                {
                    pBuffer->nFlags |= inBufferHeader.nFlags;
                    pBuffer->nFlags &= ~OMX_BUFFERFLAG_EOS; // remove EOS flag of Input because that is not real.
                    pBuffer->nFlags &= ~OMX_BUFFERFLAG_CODECCONFIG;  // remove CODECCONFIG flag, it's only for inputBuffer

                    if (omx_vpudec_component_Private->bIsTimeStampReorder == OMX_TRUE && pVpu->dispOutputInfo.indexFrameDisplay >= 0)
                    {
                        inBufferHeader.nTimeStamp = disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].nInputTimeStamp;
                        DEBUG(DEB_LEV_FULL_SEQ, "ts reorder : get InputTImestamp nTimeStamp=%lld-%lld, indexFrameDisplay=%d-%d, picType=%d\n", inBufferHeader.nTimeStamp, omx_vpudec_component_Private->lastTimeStamp, (int)pVpu->dispOutputInfo.indexFrameDecoded, (int)pVpu->dispOutputInfo.indexFrameDisplay, disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].picType);

                        /* if current nTimeStamp for indexFrameDisplay not refreshed
                         * reuse lastTimeStamp instead
                         */
                        if (omx_vpudec_component_Private->lastTimeStamp > inBufferHeader.nTimeStamp)
                        {
                            DEBUG(DEB_LEV_FULL_SEQ, "ts reorder: lastTimeStamp ahead of nInputTimeStamp, reuse it!");
                            inBufferHeader.nTimeStamp = omx_vpudec_component_Private->lastTimeStamp;
                        }
                    }

                    pBuffer->nTimeStamp = OmxTimeStampCorrection((OMX_COMPONENTTYPE *)openmaxStandPort->standCompContainer, inBufferHeader.nTimeStamp);
                    omx_vpudec_component_Private->lastTimeStamp = pBuffer->nTimeStamp;

                    if (omx_vpudec_component_Private->bSkipNoneKeyframeDisplay == OMX_TRUE)
                    {
                        if (disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].picType == PIC_TYPE_I ||
                            disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].picType == PIC_TYPE_IDR) {
                            omx_vpudec_component_Private->bSkipNoneKeyframeDisplay = OMX_FALSE;
                        } else {
                            pBuffer->nFilledLen = 0;
                            pBuffer->nTimeStamp = 0;
                            DEBUG(DEB_LEV_SIMPLE_SEQ,
                                    "skip broken Bframe display nTimeStamp=%lld, indexFrameDisplay=%d, picType=%d\n",
                                    disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].nInputTimeStamp,
                                    (int)pVpu->dispOutputInfo.indexFrameDisplay,
                                    disp_flag_array[pVpu->dispOutputInfo.indexFrameDisplay].picType);
                        }
                    }
                }

            }
        }

        OMX_DUMP_OUTPUT_YUV_TO_FILE(pVpu, pBuffer, vpuPort->sPortParam.format.video.nStride,
                                    vpuPort->sPortParam.format.video.nFrameHeight,omx_vpudec_component_Private->useNativeBuffer);

        DEBUG(DEB_LEV_FULL_SEQ, "%s callback FillBufferDone, pBuffer %p\n", __func__, pBuffer);
        (*(openmaxStandPort->BufferProcessedCallback))(
            openmaxStandPort->standCompContainer,
            omx_vpudec_component_Private->callbackData,
            pBuffer);

    } else {
        errQue = queue(pQueue, pBuffer);
        if (errQue) {
            /* /TODO the queue is full. This can be handled in a fine way with
            * some retrials, or other checking. For the moment this is a critical error
            * and simply causes the failure of this call
            */
            return OMX_ErrorInsufficientResources;
        }
        openmaxStandPort->nNumBufferFlushed++;
    }

    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s for port %p\n", __func__, openmaxStandPort);
    return OMX_ErrorNone;
}


void* omx_vpudec_component_BufferMgmtFunction (void* param)
{
    OMX_COMPONENTTYPE* openmaxStandComp = (OMX_COMPONENTTYPE*)param;

    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *pInPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_vpudec_component_PortType *pOutPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    tsem_t* pInputSem = pInPort->pBufferSem;
    tsem_t* pOutputSem = pOutPort->pBufferSem;
    queue_t* pInputQueue = pInPort->pBufferQueue;
    queue_t* pOutputQueue = pOutPort->pBufferQueue;
    OMX_BUFFERHEADERTYPE* pOutputBuffer=NULL;
    OMX_BUFFERHEADERTYPE* pInputBuffer=NULL;
    OMX_BOOL isInputBufferNeeded=OMX_TRUE,isOutputBufferNeeded=OMX_TRUE;
    int inBufExchanged=0,outBufExchanged=0;
    int ret = 0;
    omx_display_flag *disp_flag_array = &omx_vpudec_component_Private->omx_display_flags[0];

    OMX_U32 decodeBufferCounts = 0;
    omx_vpudec_component_Private->bIsOutputEOSReached = OMX_FALSE;
    omx_vpudec_component_Private->bSkipNoneKeyframeDisplay = OMX_TRUE;
    omx_vpudec_component_Private->bellagioThreads->nThreadBufferMngtID = (long int)syscall(__NR_gettid);
    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s of component %p\n", __func__, openmaxStandComp);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s the thread ID is %i\n", __func__, (int)omx_vpudec_component_Private->bellagioThreads->nThreadBufferMngtID);

    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
    /* checks if the component is in a state able to receive buffers */
    while(!omx_vpudec_component_Private->bIsBufMgThreadExit && (omx_vpudec_component_Private->state == OMX_StateIdle || omx_vpudec_component_Private->state == OMX_StateExecuting ||
        omx_vpudec_component_Private->state == OMX_StatePause || omx_vpudec_component_Private->transientState == OMX_TransStateLoadedToIdle))
    {

        /*Wait till the ports are being flushed*/
        pthread_mutex_lock(&omx_vpudec_component_Private->flush_mutex);
        while( PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))
        {
            pthread_mutex_unlock(&omx_vpudec_component_Private->flush_mutex);

            DEBUG(DEB_LEV_FULL_SEQ,
                "In %s 1 signaling flush all cond iE=%d,iF=%d,oE=%d,oF=%d"
                "iSemVal=%d,oSemval=%d,decodeBufferCounts:%d\n",
                __func__,inBufExchanged, (int)isInputBufferNeeded, outBufExchanged,
                (int)isOutputBufferNeeded, (int)pInputSem->semval, (int)pOutputSem->semval,
                decodeBufferCounts);

            if (PORT_IS_BEING_FLUSHED(pOutPort))
            {
                while (decodeBufferCounts > 0)
                {
                    tsem_up(pOutputSem);
                    decodeBufferCounts--;
                }
                omx_vpudec_component_flush_port_buffer(pOutPort, &pOutputBuffer);
                OmxVpuFlush(openmaxStandComp);
                isOutputBufferNeeded = OMX_TRUE;
                outBufExchanged = 0;
            }

            if (PORT_IS_BEING_FLUSHED(pInPort))
            {
                omx_vpudec_component_flush_port_buffer(pInPort, &pInputBuffer);
                isInputBufferNeeded = OMX_TRUE;
                inBufExchanged = 0;
                omx_vpudec_component_Private->lastTimeStamp = (OMX_TICKS)-1;
                omx_vpudec_component_Private->lastInputStamp = (OMX_TICKS)-1;
            }

            DEBUG(DEB_LEV_FULL_SEQ,
                "In %s 2 signaling flush all cond iE=%d,iF=%d,oE=%d,oF=%d"
                "iSemVal=%d,oSemval=%d,decodeBufferCounts:%d\n",
                __func__,inBufExchanged, (int)isInputBufferNeeded, outBufExchanged,
                (int)isOutputBufferNeeded, (int)pInputSem->semval, (int)pOutputSem->semval,
                decodeBufferCounts);

            tsem_up(omx_vpudec_component_Private->flush_all_condition); // this giving signal to resume base_port_FlushProcessingBuffers.
            tsem_down(omx_vpudec_component_Private->flush_condition); // this waiting for all buffer flushed in queue from base_port_FlushProcessingBuffers
            pthread_mutex_lock(&omx_vpudec_component_Private->flush_mutex);
        }
        pthread_mutex_unlock(&omx_vpudec_component_Private->flush_mutex);

        /*No buffer to process. So wait here*/
        if((isInputBufferNeeded==OMX_TRUE && pInputSem->semval==0) &&
#ifdef SUPPORT_CM_OMX_12
            (omx_vpudec_component_Private->state != OMX_StateLoaded)) {
#else
            (omx_vpudec_component_Private->state != OMX_StateLoaded && omx_vpudec_component_Private->state != OMX_StateInvalid)) {
#endif
                //Signaled from EmptyThisBuffer or FillThisBuffer or some thing else
                DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next input buffer omx_vpudec_component_Private->state=0x%x\n", (int)omx_vpudec_component_Private->state);
                tsem_timed_down(omx_vpudec_component_Private->bMgmtSem, 30);
        }

#ifdef SUPPORT_CM_OMX_12
        if(omx_vpudec_component_Private->state == OMX_StateLoaded) {
#else
        if(omx_vpudec_component_Private->state == OMX_StateLoaded || omx_vpudec_component_Private->state == OMX_StateInvalid) {
#endif
            DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Input Buffer Management Thread is exiting\n",__func__);
            break;
        }

        if((isOutputBufferNeeded==OMX_TRUE && pOutputSem->semval==0) &&
#ifdef SUPPORT_CM_OMX_12
            (omx_vpudec_component_Private->state != OMX_StateLoaded) &&
#else
            (omx_vpudec_component_Private->state != OMX_StateLoaded && omx_vpudec_component_Private->state != OMX_StateInvalid) &&
#endif
            !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
                //Signaled from EmptyThisBuffer or FillThisBuffer or some thing else
                DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next output buffer omx_vpudec_component_Private->state=0x%x\n", (int)omx_vpudec_component_Private->state);
                tsem_timed_down(omx_vpudec_component_Private->bMgmtSem, 30);
        }

#ifdef SUPPORT_CM_OMX_12
        if(omx_vpudec_component_Private->state == OMX_StateLoaded) {
#else
        if(omx_vpudec_component_Private->state == OMX_StateLoaded || omx_vpudec_component_Private->state == OMX_StateInvalid) {
#endif
            DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Output Buffer Management Thread is exiting\n",__func__);
            break;
        }

        if(pVpu->retPicRunCmd) {
            DEBUG(DEB_LEV_ERR, "error occurred and wait for executing to idle\n");
            tsem_timed_down(omx_vpudec_component_Private->bStateSem, 10);
            continue;
        }

        DEBUG(DEB_LEV_FULL_SEQ, "Waiting for input/output buffer in queue, input semval=%d output semval=%d in %s\n", (int)pInputSem->semval, (int)pOutputSem->semval, __func__);

        /*When we have input buffer to process then get one output buffer*/
        if(pOutputSem->semval > 0 && isOutputBufferNeeded == OMX_TRUE)
        {
            DEBUG(DEB_LEV_FULL_SEQ, "in isOutputBufferNeeded pOutputQueue->nelem %d\n", pOutputQueue->nelem);
            tsem_down(pOutputSem);
            if(pOutputQueue->nelem > 0)
            {
                outBufExchanged++;
                isOutputBufferNeeded = OMX_FALSE;
                pOutputBuffer = dequeue(pOutputQueue);
                if(pOutputBuffer == NULL)
                {
                    DEBUG(DEB_LEV_ERR, "Had NULL output buffer!! op is=%d,iq=%d\n", (int)pOutputSem->semval, (int)pOutputQueue->nelem);
                    break;
                }
            }
        }

        if(pInputSem->semval > 0 && isInputBufferNeeded == OMX_TRUE)
        {
            tsem_down(pInputSem);
            if(pInputQueue->nelem > 0)
            {
                inBufExchanged++;
                isInputBufferNeeded = OMX_FALSE;
                pInputBuffer = dequeue(pInputQueue);
                if(pInputBuffer == NULL)
                {
                    DEBUG(DEB_LEV_ERR, "Had NULL input buffer!!\n");
                    break;
                }
            }
        }

        if(isInputBufferNeeded == OMX_FALSE)
        {
            if(pInputBuffer->hMarkTargetComponent != NULL){
                if((OMX_COMPONENTTYPE*)pInputBuffer->hMarkTargetComponent ==(OMX_COMPONENTTYPE *)openmaxStandComp) {
                    /*Clear the mark and generate an event*/
                    (*(omx_vpudec_component_Private->callbacks->EventHandler))
                        (openmaxStandComp,
                        omx_vpudec_component_Private->callbackData,
                        OMX_EventMark, /* The command was completed */
                        1, /* The commands was a OMX_CommandStateSet */
                        0, /* The state has been changed in message->messageParam2 */
                        pInputBuffer->pMarkData);
                } else {
                    /*If this is not the target component then pass the mark*/
                    omx_vpudec_component_Private->pMark.hMarkTargetComponent = pInputBuffer->hMarkTargetComponent;
                    omx_vpudec_component_Private->pMark.pMarkData            = pInputBuffer->pMarkData;
                }
                pInputBuffer->hMarkTargetComponent = NULL;
            }
        }

        if(isInputBufferNeeded == OMX_FALSE && isOutputBufferNeeded == OMX_FALSE)
        {
            if(omx_vpudec_component_Private->pMark.hMarkTargetComponent != NULL)
            {
                pOutputBuffer->hMarkTargetComponent = omx_vpudec_component_Private->pMark.hMarkTargetComponent;
                pOutputBuffer->pMarkData            = omx_vpudec_component_Private->pMark.pMarkData;
                omx_vpudec_component_Private->pMark.hMarkTargetComponent = NULL;
                omx_vpudec_component_Private->pMark.pMarkData            = NULL;
            }

            if(omx_vpudec_component_Private->state == OMX_StateExecuting)
            {
                if (pInputBuffer->nFilledLen > 0 || ((pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS)) {
                    if (!pVpu->last_return_ts)
                        pVpu->last_return_ts = GetNowMs();
                    omx_vpudec_component_BufferMgmtCallback(openmaxStandComp, pInputBuffer, pOutputBuffer);
                } else {
                    /*It no buffer management call back the explicitly consume input buffer*/
                    pInputBuffer->nFilledLen = 0;
                }

            } else if(!(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
                DEBUG(DEB_LEV_ERR, "In %s Received Buffer in non-Executing State(%x)\n", __func__, (int)omx_vpudec_component_Private->state);
            } else {
                pInputBuffer->nFilledLen = 0;
            }

            if(((pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS && pInputBuffer->nFilledLen==0) &&
                omx_vpudec_component_Private->bIsEOSReached == OMX_FALSE) {
                    DEBUG(DEB_LEV_FULL_SEQ, "Detected EOS flags in input buffer filled len=%d\n", (int)pInputBuffer->nFilledLen);

                    omx_vpudec_component_Private->bIsEOSReached = OMX_TRUE;
            }

            if(omx_vpudec_component_Private->state==OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
                /*Waiting at paused state*/
                tsem_wait(omx_vpudec_component_Private->bStateSem);
            }

            /*If EOS and Input buffer Filled Len Zero then Return output buffer immediately*/
            if (pOutputBuffer) {
                if((pOutputBuffer->nFilledLen != 0) || ((pOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS)) {
                    if ((pOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS) {
                        DEBUG(DEB_LEV_FULL_SEQ, "Detected EOS flags in output buffer filled len=%d\n", (int)pOutputBuffer->nFilledLen);
                        omx_vpudec_component_Private->bIsOutputEOSReached = OMX_TRUE;
                    }

                    double cur_ms = GetNowMs();
                    double elapsed_ts = cur_ms - pVpu->last_return_ts;
                    DEBUG(DEB_LEV_FULL_SEQ, "instance[%d:%d] Return %d buf elapsed: %.1fms\n", pVpu->coreIdx, pVpu->handle->instIndex, pVpu->dispOutIdx, elapsed_ts);
                    pVpu->last_return_ts = cur_ms;
                    pOutPort->ReturnBufferFunction((omx_base_PortType *)pOutPort, pOutputBuffer);

                    if (omx_vpudec_component_Private->bIsOutputEOSReached == OMX_TRUE) {
                        (*(omx_vpudec_component_Private->callbacks->EventHandler))(openmaxStandComp,
                            omx_vpudec_component_Private->callbackData,
                            OMX_EventBufferFlag,
                            1, /* port index should be output port index */
                            pOutputBuffer->nFlags,
                            NULL);
                    }

                    outBufExchanged--;
                    pOutputBuffer=NULL;
                    isOutputBufferNeeded=OMX_TRUE;
                    DEBUG(DEB_LEV_FULL_SEQ, "after ReturnBufferFunction outBufExchanged %d\n", outBufExchanged);

                    if (pVpu->dispOutputInfo.indexFrameDecoded < 0 && decodeBufferCounts > 0)
                    {
                        /*decode failed, but display success,
                         *it's means we will lose a decoding Buffer
                         *so, sync the status of vpu
                         */
                        tsem_up(pOutputSem);
                        decodeBufferCounts--; //pOutputQueue->nelem - pOutputSem->semval
                        DEBUG(DEB_LEV_FULL_SEQ,
                              "indexFrameDisplay %d, indexFrameDecoded %d, decode buffer Counts:%d\n",
                              pVpu->dispOutputInfo.indexFrameDisplay,
                              pVpu->dispOutputInfo.indexFrameDecoded,
                              decodeBufferCounts);
                    }
                }
                else if (pVpu->dispOutputInfo.indexFrameDecoded == -1)
                {
                    // this would make this thread to avoid too many requests to VPU. and to yield the other job to OS scheduler.
                    if(!(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
                    {
                        DEBUG(DEB_LEV_FULL_SEQ, "display buffer full, wait for fillThisBuffer event, outputSem=%d, pOutputQueue->nelem=%d\n", (int)pOutputSem->semval, (int)pOutputQueue->nelem);
                        tsem_reset(&omx_vpudec_component_Private->disp_Buf_full_tsem); // to wait the next FillThisBuffer from now
                        tsem_timed_down(&omx_vpudec_component_Private->disp_Buf_full_tsem, 30); // but the next FillThisBuffer may not come at some case, so give a chance to return
                        DEBUG(DEB_LEV_FULL_SEQ, "display buffer full, wait for fillThisBuffer done, outputSem=%d, pOutputQueue->nelem=%d\n", (int)pOutputSem->semval, (int)pOutputQueue->nelem);
                    }
                }
                else if ((pVpu->dispOutputInfo.indexFrameDisplay == -3 || pVpu->dispOutputInfo.indexFrameDisplay == -2)
                    && pVpu->dispOutputInfo.indexFrameDecoded >= 0)
                {
                    if(!(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
                    {
                        if((pOutPort->bAllocateBuffer == OMX_TRUE) || (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)) {
                            DEBUG(DEB_LEV_FULL_SEQ, "no display, but decoded reference %d\n", pVpu->dispOutputInfo.indexFrameDecoded);
                            /* reuse buffer, frame only reference */
                            if (pVpu->decOP.bitstreamFormat == STD_VP9) {
                                DEBUG(DEB_LEV_FULL_SEQ, "indexFrameDisplay %d, indexFrameDecoded %d\n", pVpu->dispOutputInfo.indexFrameDisplay, pVpu->dispOutputInfo.indexFrameDecoded);
                            } else {
                                /* In this situation occurs, it means that the vpu needs more buffers to store the decoded data
                                 * instead of displaying it immediately, so increase the decoding buffer size of the vpu,
                                 * This value can be expressed by the number of enmu - semaphores,
                                 * so Here only the semaphore is reduced, which means that the vpu has obtained a decoding buffer
                                 */
                                do {
                                    ret = tsem_timed_down(pOutputSem, 30);
                                    if (ret == 0)
                                    {
                                        decodeBufferCounts++; // pOutputQueue->nelem - pOutputSem->semval
                                        DEBUG(DEB_LEV_FULL_SEQ,
                                              "indexFrameDisplay %d, indexFrameDecoded %d, decode "
                                              "buffer Counts:%d\n",
                                              pVpu->dispOutputInfo.indexFrameDisplay,
                                              pVpu->dispOutputInfo.indexFrameDecoded,
                                              decodeBufferCounts);
                                    }

                                    if (omx_vpudec_component_Private->bIsBufMgThreadExit ||
                                            PORT_IS_BEING_FLUSHED(pInPort) ||
                                            PORT_IS_BEING_FLUSHED(pOutPort))
                                        break;
                                } while(ret != 0);
                            }
                        } else {
                            //decode is success, but the decoded data is not copied to the pOutputBuffer, the pOutputBuffer can be re-use next cycle,
                            //if not reuse here, the buf will lost forever for this componet
                            DEBUG(DEB_LEV_FULL_SEQ, "not display buf case, re-use the buffer %p\n", pOutputBuffer);
                        }
                    }
                }
            }


            if(omx_vpudec_component_Private->state==OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort))) {
                /*Waiting at paused state*/
                tsem_wait(omx_vpudec_component_Private->bStateSem);
            }

            /*Input Buffer has been completely consumed. So, return input buffer*/
            if((isInputBufferNeeded == OMX_FALSE) && (pInputBuffer->nFilledLen == 0)) {
                if (omx_vpudec_component_Private->bIsEOSReached == OMX_TRUE && omx_vpudec_component_Private->bIsOutputEOSReached == OMX_FALSE) {
                    /* don't return input buffer until output buffer reach EOS */
                    DEBUG(DEB_LEV_SIMPLE_SEQ, "need not pInPort->ReturnBufferFunction bIsEOSReached == OMX_TRUE isOutputEos == OMX_FALSE\n");
                } else {
                    pInPort->ReturnBufferFunction((omx_base_PortType *)pInPort,pInputBuffer);
                    inBufExchanged--;
                    pInputBuffer = NULL;
                    isInputBufferNeeded = OMX_TRUE;

                }
            }

            //seq changed, will re-init seq
            if (omx_vpudec_component_Private->bSeqChangeDetected)
            {
                DEBUG(DEB_LEV_FULL_SEQ, "In %s seq changed, will re-init seq, decode buffer Counts:%d\n",
                      __func__, decodeBufferCounts);
                OMX_BUFFERHEADERTYPE *pBuffer = NULL;
                while (decodeBufferCounts > 0)
                {
                    tsem_up(pOutputSem);
                    decodeBufferCounts--;
                }
                for (int i = 0; i < (int)pOutPort->sPortParam.nBufferCountActual; i++)
                {
                    if (disp_flag_array[i].owner == OMX_BUFERR_OWNER_COMPONENT)
                    { //if the buffer is also owned by component
                        pBuffer = pOutPort->pInternalBufferStorage[i];
                        if (pBuffer)
                        {
                            OmxClearDisplayFlag(openmaxStandComp, pBuffer, OMX_TRUE);
                        }
                    }
                }
            }
        }
    }

    omx_vpudec_component_Private->bIsBufMgThreadExit = TRUE;
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s of component %p\n", __func__, openmaxStandComp);
    return NULL;
}


static void OmxWaitUntilOutBuffersEmpty(omx_vpudec_component_PrivateType* omx_vpudec_component_Private)
{
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    omx_display_flag *disp_flag_array = &omx_vpudec_component_Private->omx_display_flags[0];
    int ownedClientBufferCount ;
    int i;

    int minUndequeuBuffer = (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE) ? NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS : 0;
    if (omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
    {
        do {
            /* waiting until the owner of all fillbuffers changed to OMX_BUFFER_OWNER_COMPONENT except for the amount of NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS before sending PortSettingsChange*/
            /* if not, OMX_EventPortSettingsChanged event in OMXCocdec will be failed due to CHECK(mFilledBuffers.empty()) */
            ownedClientBufferCount = 0;
            pthread_mutex_lock(&omx_vpudec_component_Private->display_flag_mutex);
            for (i = 0; i < (int)outPort->sPortParam.nBufferCountActual; i++)
            {
                if (disp_flag_array[i].owner == OMX_BUFFER_OWNER_CLIENT)
                    ownedClientBufferCount++;;
            }
            pthread_mutex_unlock(&omx_vpudec_component_Private->display_flag_mutex);
            if(ownedClientBufferCount > minUndequeuBuffer)
            {
                DEBUG(DEB_LEV_SIMPLE_SEQ,"waiting until the number of buffers with OWNER_CLIENT (%d) <= number of MIN_UNDEQUEUED_BUFFER (%d)", (int)ownedClientBufferCount, (int)NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS);
                tsem_timed_down(&omx_vpudec_component_Private->disp_Buf_full_tsem, 100);
            }

        } while(ownedClientBufferCount > minUndequeuBuffer);
    }
}


OMX_BOOL OmxVpuFlush(OMX_COMPONENTTYPE *openmaxStandComp)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = (omx_vpudec_component_PrivateType*) openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    omx_display_flag *disp_flag_array = &omx_vpudec_component_Private->omx_display_flags[0];
    RetCode ret = RETCODE_SUCCESS;
    int i;
    int dispFlagIdx = 0;
    DecOutputInfo decOutputInfo;

    if (!omx_vpudec_component_Private->vpuReady) {
        return OMX_TRUE;
    }

    pthread_mutex_lock(&omx_vpudec_component_Private->vpu_flush_mutex);

    if (pVpu->int_reason & (1<<INT_BIT_DEC_FIELD))
    {
        DEBUG(DEB_LEV_FULL_SEQ, "INT_BIT_DEC_FIELD interrupt already triggered");
        VPU_ClearInterrupt(pVpu->coreIdx);  // clear field_done interrupt
        pVpu->int_reason = 0;

        // f/w will processing for 2nd field.

        pVpu->int_reason = VPU_WaitInterruptEx(pVpu->handle, OMX_VPU_DEC_TIMEOUT);
        if (pVpu->int_reason == -1 )
        {
            VPU_DecGetOutputInfo(pVpu->handle, &decOutputInfo);
            pVpu->int_reason = 0;
            DEBUG(DEB_LEV_ERR, "VPU timeout when consuming 2nd field");
            pthread_mutex_unlock(&omx_vpudec_component_Private->vpu_flush_mutex);
            return OMX_FALSE;
        }
        VPU_ClearInterrupt(pVpu->coreIdx);  // clear pic_done interrupt
        pVpu->int_reason = 0;
        VPU_DecGetOutputInfo(pVpu->handle, &decOutputInfo);
    }

    if (outPort->bAllocateBuffer == OMX_TRUE || omx_vpudec_component_Private->useNativeBuffer == OMX_TRUE)
    {
        pthread_mutex_lock(&omx_vpudec_component_Private->display_flag_mutex);
        for (i = 0; i < (int)outPort->sPortParam.nBufferCountActual; i++)
        {
            dispFlagIdx = i;
            VPU_DecGiveCommand(pVpu->handle, DEC_SET_DISPLAY_FLAG, &dispFlagIdx);
            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s of component VPU_DecGiveCommand[DEC_SET_DISPLAY_FLAG] for index=%d, dispFlagIdx=%d, owner=0x%x, decFbCount=%d\n", __func__,  i, dispFlagIdx, disp_flag_array[i].owner, pVpu->decFbCount);
        }

        pthread_mutex_unlock(&omx_vpudec_component_Private->display_flag_mutex);
    }
    else
    {
        pthread_mutex_lock(&omx_vpudec_component_Private->display_flag_mutex);
        for (i = 0; i < (int)outPort->sPortParam.nBufferCountActual; i++)
        {
             dispFlagIdx = i;
            VPU_DecClrDispFlag(pVpu->handle, dispFlagIdx);
            DEBUG(DEB_LEV_SIMPLE_SEQ, "%s of component VPU_DecClrDispFlag for index=%d, dispFlagIdx=%d, owner=0x%x, decFbCount=%d\n", __func__,  i, dispFlagIdx, disp_flag_array[i].owner, pVpu->decFbCount);
        }
        pthread_mutex_unlock(&omx_vpudec_component_Private->display_flag_mutex);
    }

    if (pVpu->decOP.bitstreamFormat != STD_VP9 || (pVpu->seqInited && !pVpu->interResChanged))
        ret = VPU_DecFrameBufferFlush(pVpu->handle, NULL, NULL);

    if (ret != RETCODE_SUCCESS)
    {
        DEBUG(DEB_LEV_ERR, "VPU_DecFrameBufferFlush failed Error code is 0x%x \n", (int)ret );
        pthread_mutex_unlock(&omx_vpudec_component_Private->vpu_flush_mutex);
        return OMX_FALSE;
    }
    if (pVpu->decOP.bitstreamFormat != STD_VP9 || !pVpu->interResChanged)   // when inter-resolution change for VP9 case, chunk should be reused.
        pVpu->chunkReuseRequired = 0;

    pVpu->prevConsumeBytes = 0;
    omx_vpudec_component_Private->bSkipNoneKeyframeDisplay = OMX_TRUE;

#ifdef USE_IFRAME_SEARCH_FOR_1STFRAME
    if (pVpu->decOP.bitstreamFormat != STD_VP9)     // skipmode for VP9 under developing at the moment
        pVpu->decParam.skipframeMode = 1;
#endif
    pthread_mutex_unlock(&omx_vpudec_component_Private->vpu_flush_mutex);

    return OMX_TRUE;
}


static int OmxCopyVpuBufferToOmxBuffer(OMX_COMPONENTTYPE *openmaxStandComp, OMX_U8 *pBuffer, FrameBuffer fb)
{
    omx_vpudec_component_PrivateType* omx_vpudec_component_Private = (omx_vpudec_component_PrivateType*) openmaxStandComp->pComponentPrivate;
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *outPort = (omx_vpudec_component_PortType *) omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
    OMX_U8 *pDst;
    OMX_BOOL b422PackedMode = OMX_FALSE;
    int width;
    int height;
    int size;
    int stride;
    int nY, nCb, nCr;
    int lumaSize, chromaSize, chromaStride, chromaWidth, chromaHeight;
    int format;
    double curr_ms;

    width = outPort->sPortParam.format.video.nFrameWidth;
    height = outPort->sPortParam.format.video.nFrameHeight;
    stride = outPort->sPortParam.format.video.nStride;

    if (pVpu->scalerInfo.enScaler)
        // format = (pVpu->scalerInfo.imageFormat < YUV_FORMAT_I422)?FORMAT_420:FORMAT_422;
        format = FORMAT_420;
    else
        format = pVpu->fbFormat;
    DEBUG(DEB_LEV_FULL_SEQ, "%s format %d, width %d, height %d, stride %d\n", __func__, format, width, height, stride);
    switch (format)
    {
    case FORMAT_420:
        nY = (height+1)/2*2;
        nCb = nCr = (height+1) / 2;
        chromaSize = ((stride + 1) / 2) * ((height+1) / 2);
        chromaStride = stride / 2;
        chromaWidth = (stride + 1) / 2;
        chromaHeight = nY;
        break;
    case FORMAT_224:
        nY = (height+1)/2*2;
        nCb = nCr = (height+1) / 2;
        chromaSize = (stride) * ((height+1) / 2);
        chromaStride = stride;
        chromaWidth = stride;
        chromaHeight = nY;
        break;
    case FORMAT_422:
        nY = height;
        nCb = nCr = height;
        chromaSize = ((stride + 1)/2) * height ;
        chromaStride = stride / 2;
        chromaWidth = (stride + 1) / 2;
        chromaHeight = nY*2;
        break;
    case FORMAT_444:
        nY = height;
        nCb = nCr = height;
        chromaSize = stride * height;
        chromaStride = stride;
        chromaWidth = stride;
        chromaHeight = nY*2;
        break;
    case FORMAT_400:
        nY = height;
        nCb = nCr = 0;
        chromaSize = 0;
        chromaStride = 0;
        chromaWidth = 0;
        chromaHeight = 0;
        break;
    default:
        nY = height;
        nCb = nCr = height;
        chromaSize = ((stride + 1)/2) * height ;
        chromaStride = stride / 2;
        chromaWidth = (stride + 1) / 2;
        chromaHeight = nY*2;
        break;
    }

    UNREFERENCED_PARAMETER(chromaHeight);
    UNREFERENCED_PARAMETER(chromaWidth);
    UNREFERENCED_PARAMETER(chromaStride);
    UNREFERENCED_PARAMETER(nCb);
    // if (pVpu->scalerInfo.enScaler &&
    //  format == FORMAT_422 &&
    //  (pVpu->scalerInfo.imageFormat == YUV_FORMAT_UYVY || pVpu->scalerInfo.imageFormat == YUV_FORMAT_YUYV))
    // {
    //  b422PackedMode = OMX_TRUE;
    // }

    lumaSize = stride * nY;
    size = lumaSize + chromaSize*2;


    DEBUG(DEB_LEV_FULL_SEQ, "OmxCopyVpuBufferToOmxBuffer : start copy video data Buffer=%p from index=%d, bufY=0x%x, bufCb=0x%x, bufCr=0x%x, size=%d, lumaSize=%d, chromaSize=%d, fbFormat=%d, b422PackedMode=%d\n",
        pBuffer, fb.myIndex, fb.bufY, fb.bufCb, fb.bufCr, size, lumaSize, chromaSize, (int)format, (int)b422PackedMode);

    curr_ms = GetNowMs();

    if (b422PackedMode == OMX_TRUE)
    {
        pDst = pBuffer;
        vdi_read_cache_memory(pVpu->coreIdx, fb.bufY, (unsigned char*)pDst, size, pVpu->decOP.frameEndian);
    }
    else
    {
        pDst = pBuffer;

        vdi_read_cache_memory(pVpu->coreIdx, fb.bufY, (unsigned char*)pDst, lumaSize, pVpu->decOP.frameEndian);

        if (pVpu->decOP.cbcrInterleave)
        {
            pDst = pBuffer + lumaSize;
            vdi_read_cache_memory(pVpu->coreIdx, fb.bufCb, (unsigned char*)pDst, chromaSize*2, pVpu->decOP.frameEndian);
        }
        else
        {
            pDst = pBuffer + lumaSize;

            vdi_read_cache_memory(pVpu->coreIdx, fb.bufCb, (unsigned char*)pDst, chromaSize, pVpu->decOP.frameEndian);

            pDst = pBuffer + lumaSize + chromaSize;

            vdi_read_cache_memory(pVpu->coreIdx, fb.bufCr, (unsigned char*)pDst, chromaSize, pVpu->decOP.frameEndian);
        }

    }

    DEBUG(DEB_LEV_FULL_SEQ, "OmxCopyVpuBufferToOmxBuffer : end copy video data  time=%.1fms, size=%d\n", GetNowMs()-curr_ms, (int)size);

    return size;
}

OMX_ERRORTYPE OmxCheckCropInfo(OMX_COMPONENTTYPE *openmaxStandComp, omx_vpudec_component_PrivateType* omx_vpudec_component_Private)
{
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *) & omx_vpudec_component_Private->vpu;
    omx_vpudec_component_PortType *inPort =
        (omx_vpudec_component_PortType *)omx_vpudec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
    omx_vpudec_component_PortType *outPort =
        (omx_vpudec_component_PortType *)omx_vpudec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];

    OMX_ERRORTYPE err = OMX_ErrorNone;

    DEBUG(DEB_LEV_SIMPLE_SEQ, "initialInfo picCropRect left : %d, top : %d, right : %d, bottom : %d\n",
          (int)pVpu->initialInfo.picCropRect.left, (int)pVpu->initialInfo.picCropRect.top,
          (int)pVpu->initialInfo.picCropRect.right, (int)pVpu->initialInfo.picCropRect.bottom);
    if (pVpu->scalerInfo.enScaler)
    {
        if (pVpu->initialInfo.picWidth == pVpu->scalerInfo.scaleWidth &&
            pVpu->initialInfo.picHeight == pVpu->scalerInfo.scaleHeight)
            pVpu->scalerInfo.sameSize = TRUE;
        else
            pVpu->scalerInfo.sameSize = FALSE;
        DEBUG(DEB_LEV_FULL_SEQ, "scaler to %dx%d, sameSize %d\n",
              pVpu->scalerInfo.scaleWidth, pVpu->scalerInfo.scaleHeight, pVpu->scalerInfo.sameSize);
        if (VPU_DecGiveCommand(pVpu->handle, DEC_SET_SCALER_INFO, (void *)&pVpu->scalerInfo) != RETCODE_SUCCESS)
        {
            DEBUG(DEB_LEV_ERR, "Failed to VPU_DecGiveCommand(DEC_SET_SCALER_INFO)\n");
            err = OMX_ErrorHardware;
            return err;
        }

        double scaleRatioW;
        double scaleRatioH;
        scaleRatioW = ((double)pVpu->scalerInfo.scaleWidth / pVpu->initialInfo.picWidth);
        scaleRatioH = ((double)pVpu->scalerInfo.scaleHeight / pVpu->initialInfo.picHeight);
        DEBUG(DEB_LEV_SIMPLE_SEQ, "scaleRatioW=%f scaleRatioH=%f ", scaleRatioW, scaleRatioH);
        inPort->omxConfigCrop.nLeft = (OMX_S32)(pVpu->initialInfo.picCropRect.left * scaleRatioW);
        inPort->omxConfigCrop.nTop = (OMX_S32)(pVpu->initialInfo.picCropRect.top * scaleRatioH);
        inPort->omxConfigCrop.nWidth =
            (OMX_S32)(pVpu->initialInfo.picCropRect.right - pVpu->initialInfo.picCropRect.left) * scaleRatioW;
        inPort->omxConfigCrop.nHeight =
            (OMX_S32)(pVpu->initialInfo.picCropRect.bottom - pVpu->initialInfo.picCropRect.top) * scaleRatioH;
    }
    else
    {
        inPort->omxConfigCrop.nLeft = pVpu->initialInfo.picCropRect.left;
        inPort->omxConfigCrop.nTop = pVpu->initialInfo.picCropRect.top;
        inPort->omxConfigCrop.nWidth = pVpu->initialInfo.picCropRect.right - pVpu->initialInfo.picCropRect.left;
        inPort->omxConfigCrop.nHeight = pVpu->initialInfo.picCropRect.bottom - pVpu->initialInfo.picCropRect.top;
    }
    DEBUG(DEB_LEV_SIMPLE_SEQ, "update omxConfigCrop left : %d, top : %d, width : %d, height : %d",
          (int)inPort->omxConfigCrop.nLeft, (int)inPort->omxConfigCrop.nTop,
          (int)inPort->omxConfigCrop.nWidth, (int)inPort->omxConfigCrop.nHeight);

#ifdef ANDROID
    if (inPort->omxConfigCrop.nLeft != outPort->omxConfigCrop.nLeft ||
        inPort->omxConfigCrop.nTop != outPort->omxConfigCrop.nTop ||
        inPort->omxConfigCrop.nWidth != outPort->omxConfigCrop.nWidth ||
        inPort->omxConfigCrop.nHeight != outPort->omxConfigCrop.nHeight)
    {
        DEBUG(DEB_LEV_SIMPLE_SEQ,
              "\n----> Normal mode:Crop size will changed;"
              "current: left:%d, top:%d, width:%d, height:%d;\t"
              "will change to: left:%d, top:%d, width:%d height:%d\n",
              (int)outPort->omxConfigCrop.nLeft,
              (int)outPort->omxConfigCrop.nTop,
              (int)outPort->omxConfigCrop.nWidth,
              (int)outPort->omxConfigCrop.nHeight,
              (int)inPort->omxConfigCrop.nLeft,
              (int)inPort->omxConfigCrop.nTop,
              (int)inPort->omxConfigCrop.nWidth,
              (int)inPort->omxConfigCrop.nHeight);

        outPort->omxConfigCrop.nLeft = inPort->omxConfigCrop.nLeft;
        outPort->omxConfigCrop.nTop = inPort->omxConfigCrop.nTop;
        outPort->omxConfigCrop.nWidth = inPort->omxConfigCrop.nWidth;
        outPort->omxConfigCrop.nHeight = inPort->omxConfigCrop.nHeight;

        if (omx_vpudec_component_Private->callbacks->EventHandler)
        {
            err = (*(omx_vpudec_component_Private->callbacks->EventHandler))(openmaxStandComp,
                                                                             omx_vpudec_component_Private->callbackData,
                                                                             OMX_EventPortSettingsChanged,     /* The command was completed */
                                                                             OMX_BASE_FILTER_OUTPUTPORT_INDEX, /* This is the output port index */
                                                                             OMX_IndexConfigCommonOutputCrop,
                                                                             NULL);
            if (err != OMX_ErrorNone)
            {
                DEBUG(DEB_LEV_ERR,
                      "fail to OMX_EventPortSettingsChanged Event for OMX_IndexConfigCommonOutputCrop, err:%d\n", err);
                return err;
            }
        }
    }
#endif
    return err;
}

OMX_U32 OmxCheckVUIParams(OMX_COMPONENTTYPE *openmaxStandComp, omx_vpudec_component_PrivateType* omx_vpudec_component_Private, OMX_BOOL dispOutputInfo)
{
#ifdef ANDROID
    OMX_BOOL fullRange = OMX_FALSE;
    OMX_S32  primaries = 0, transfer = 0, coeffs = 0;
    OMX_BOOL colorAspectChanged = OMX_FALSE;

    vpu_dec_context_t *pVpu = (vpu_dec_context_t *)&omx_vpudec_component_Private->vpu;

    if (!supportsDescribeColorAspects(omx_vpudec_component_Private->video_coding_type))
        goto DO_NOTHING;

    DEBUG(DEB_LEV_FULL_SEQ, "OmxCheckVUIParams\n");
    switch(omx_vpudec_component_Private->video_coding_type)
    {
    case OMX_VIDEO_CodingAVC:
        DEBUG(DEB_LEV_FULL_SEQ, "colorDescPresent %d\n", pVpu->initialInfo.avcVuiInfo.colorDescPresent);

        AvcVuiInfo avcVuiInfo = dispOutputInfo ? pVpu->dispOutputInfo.avcVuiInfo : pVpu->initialInfo.avcVuiInfo;
        // for format avc, we will get the vui info from the avcVuiInfo
        if (!avcVuiInfo.colorDescPresent)
            goto DO_NOTHING;

        primaries = avcVuiInfo.colorPrimaries;
        transfer  = avcVuiInfo.vuiTransferCharacteristics;
        coeffs    = avcVuiInfo.vuiMatrixCoefficients;
        fullRange = avcVuiInfo.vidFullRange;

        break;
    case OMX_VIDEO_CodingMPEG2:
        DEBUG(DEB_LEV_FULL_SEQ, "colorDescPresent %d\n", pVpu->initialInfo.mp2ColorDescPresentFlag);
        // for format mpeg2, we will get the vui info from initialInfo
        if (dispOutputInfo)
        {
            if (!pVpu->dispOutputInfo.mp2ColorDescPresentFlag)
                goto DO_NOTHING;
            primaries = pVpu->dispOutputInfo.mp2ColorPrimaries;
            transfer = pVpu->dispOutputInfo.mp2TransferChar;
            coeffs = pVpu->dispOutputInfo.mp2MatrixCoeff;
        }
        else
        {
            if (!pVpu->initialInfo.mp2ColorDescPresentFlag)
                goto DO_NOTHING;
            primaries = pVpu->initialInfo.mp2ColorPrimaries;
            transfer = pVpu->initialInfo.mp2TransferChar;
            coeffs = pVpu->initialInfo.mp2MatrixCoeff;
        }
        fullRange = OMX_FALSE;  // mpeg2 video has limited range.
        break;

    case OMX_VIDEO_CodingHEVC:
        // for format hevc, the vui info only after the decode out info
        if (!pVpu->dispOutputInfo.h265Info.colorDescPresent)
            goto DO_NOTHING;
        primaries = pVpu->dispOutputInfo.h265Info.colorPrimaries;
        transfer = pVpu->dispOutputInfo.h265Info.vuiTransferCharacteristics;
        coeffs = pVpu->dispOutputInfo.h265Info.vuiMatrixCoefficients;
        fullRange = pVpu->dispOutputInfo.h265Info.vidFullRange;
        break;
    default:
        break;
    }

    convertIsoColorAspectsToCodecAspects(&omx_vpudec_component_Private->mBitstreamColorAspects, primaries, transfer, coeffs, fullRange);
    colorAspectChanged = handleColorAspectsChange(omx_vpudec_component_Private->video_coding_type,
        &omx_vpudec_component_Private->mDefaultColorAspects, &omx_vpudec_component_Private->mBitstreamColorAspects,
        &omx_vpudec_component_Private->mFinalColorAspects);
    if(colorAspectChanged)
    {
        (*(omx_vpudec_component_Private->callbacks->EventHandler))(openmaxStandComp,
            omx_vpudec_component_Private->callbackData,
            OMX_EventPortSettingsChanged,
            OMX_BASE_FILTER_OUTPUTPORT_INDEX,
            OMX_IndexParamDescribeColorAspects,
            NULL);
    }

DO_NOTHING:
#endif
    return 0;
}


OMX_U32 OmxCheckHDRStaticMetadata(OMX_COMPONENTTYPE *openmaxStandComp, omx_vpudec_component_PrivateType* omx_vpudec_component_Private)
{
#ifdef ANDROID
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *)&omx_vpudec_component_Private->vpu;

    if (!supportDescribeHdrStaticInfo(omx_vpudec_component_Private->video_coding_type))
        goto DO_NOTHING;

    DEBUG(DEB_LEV_FULL_SEQ, "OmxCheckHDRStaticMetadata\n");

    // for format hevc, the hdr info only after the decode out info
    if (!pVpu->dispOutputInfo.h265Info.masterMetaPresent)
        goto DO_NOTHING;

    for (int i = 0; i < 3; i++)
    {
        omx_vpudec_component_Private->mHdrStaticInfo.display_primaries_x[i] = pVpu->dispOutputInfo.h265Info.display_primaries_x[i];
        omx_vpudec_component_Private->mHdrStaticInfo.display_primaries_y[i] = pVpu->dispOutputInfo.h265Info.display_primaries_y[i];
    }

    omx_vpudec_component_Private->mHdrStaticInfo.white_point_x = pVpu->dispOutputInfo.h265Info.white_point_x;
    omx_vpudec_component_Private->mHdrStaticInfo.white_point_y = pVpu->dispOutputInfo.h265Info.white_point_y;

    omx_vpudec_component_Private->mHdrStaticInfo.max_display_mastering_luminance =
                                              pVpu->dispOutputInfo.h265Info.max_display_mastering_luminance;
    omx_vpudec_component_Private->mHdrStaticInfo.min_display_mastering_luminance =
                                              pVpu->dispOutputInfo.h265Info.min_display_mastering_luminance;

    if (pVpu->dispOutputInfo.h265Info.contentMetaPresent) {
        omx_vpudec_component_Private->mHdrStaticInfo.max_content_light_level = pVpu->dispOutputInfo.h265Info.max_content_light_level;
        omx_vpudec_component_Private->mHdrStaticInfo.max_frame_ave_light_level = pVpu->dispOutputInfo.h265Info.max_pic_average_light_level;
    }

    // we should normalize the vendor value to framework
    convertVendorHdrToCodecHdrInfo(&omx_vpudec_component_Private->mHdrStaticInfo);

    (*(omx_vpudec_component_Private->callbacks->EventHandler))(openmaxStandComp,
        omx_vpudec_component_Private->callbackData,
        OMX_EventPortSettingsChanged,
        OMX_BASE_FILTER_OUTPUTPORT_INDEX,
        OMX_IndexParamDescribeHDRStaticInfo,
        NULL);

DO_NOTHING:
#endif
    return 0;
}


static void initDefaultCoreSetting(omx_vpudec_component_PrivateType* omx_vpudec_component_Private)
{
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *)&omx_vpudec_component_Private->vpu;
    OMX_VIDEO_CODINGTYPE codingType = omx_vpudec_component_Private->video_coding_type;

    if (codingType == OMX_VIDEO_CodingHEVC || codingType == OMX_VIDEO_CodingVP9) {
        pVpu->coreIdx = VPU_WAVE412_IDX;
        pVpu->mapType = COMPRESSED_FRAME_MAP;
    } else {
        pVpu->coreIdx = VPU_CODA988_IDX;
        pVpu->mapType = LINEAR_FRAME_MAP;
    }
}


static void updateOutSetting(omx_vpudec_component_PrivateType* omx_vpudec_component_Private)
{
    vpu_dec_context_t *pVpu = (vpu_dec_context_t *)&omx_vpudec_component_Private->vpu;
    if (omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingHEVC
        || omx_vpudec_component_Private->video_coding_type == OMX_VIDEO_CodingVP9) {
        pVpu->mapType = COMPRESSED_FRAME_MAP;

#ifdef ANDROID
        if (!omx_vpudec_component_Private->useNativeBuffer
            || checkTileIFBCDisabled((OMX_STRING)PROP_DISABLE_FBDC, OMX_FALSE)
            || (omx_vpudec_component_Private->nUsage & GRALLOC_USAGE_SW_READ_OFTEN)) {
            pVpu->scalerInfo.enScaler = FALSE;
            pVpu->decOP.pvricFbcEnable = FALSE;
            omx_vpudec_component_Private->nUsage &= (~GRALLOC_USAGE_HW_RENDER);
            omx_vpudec_component_Private->nUsage |= GRALLOC_USAGE_SW_READ_OFTEN;
            DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, WAVE: disable ifbc\n", __func__);
        } else {
            pVpu->scalerInfo.enScaler = TRUE;
            pVpu->decOP.pvricFbcEnable = TRUE;
            omx_vpudec_component_Private->nUsage |= GRALLOC_USAGE_HW_RENDER;
        }
#endif
    } else {
        pVpu->mapType = LINEAR_FRAME_MAP;
#ifdef ANDROID
        if (!omx_vpudec_component_Private->useNativeBuffer
            || checkTileIFBCDisabled((OMX_STRING)PROP_DISABLE_TILE, OMX_FALSE)
            || (omx_vpudec_component_Private->nUsage & GRALLOC_USAGE_SW_READ_OFTEN)) {
            pVpu->mapType = LINEAR_FRAME_MAP;
            omx_vpudec_component_Private->nUsage &= (~GRALLOC_USAGE_PRIVATE_3);
            omx_vpudec_component_Private->nUsage |= GRALLOC_USAGE_SW_READ_OFTEN;
            DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, CODA: disable tiled\n", __func__);
        } else {
            /* Temporarily disable tiled due to hwc not supported yet
             * uncomment following once hwc supported
             */
            // pVpu->mapType = TILED_FRAME_MB_RASTER_MAP;
            // omx_vpudec_component_Private->nUsage |= GRALLOC_USAGE_PRIVATE_3;
            // DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, CODA: enable tiled by default\n", __func__);

            DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s, CODA: disable tiled due to hwc not supported yet\n", __func__);
            omx_vpudec_component_Private->nUsage |= GRALLOC_USAGE_SW_READ_OFTEN;
        }
#endif
    }

    DEBUG(DEB_LEV_SIMPLE_SEQ, "Out %s, mapType %d, nUsage 0x%x\n", __func__, pVpu->mapType, omx_vpudec_component_Private->nUsage);
}

