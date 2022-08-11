//------------------------------------------------------------------------------
// File: main_helper.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#ifndef _MAIN_HELPER_H_
#define _MAIN_HELPER_H_

#include "vpuapifunc.h"
#include "vpuapi.h"
#ifdef PLATFORM_QNX
    #include <sys/stat.h>
#endif

#define MATCH_OR_MISMATCH(_expected, _value, _ret)        ((_ret=(_expected == _value)) ? "MATCH" : "MISMATCH")

#if defined(WIN32) || defined(WIN64)
/*
 ( _MSC_VER => 1200 )     6.0     vs6
 ( _MSC_VER => 1310 )     7.1     vs2003
 ( _MSC_VER => 1400 )     8.0     vs2005
 ( _MSC_VER => 1500 )     9.0     vs2008
 ( _MSC_VER => 1600 )    10.0     vs2010
 */
#if (_MSC_VER == 1200)
#define strcasecmp          stricmp
#define strncasecmp         strnicmp
#else
#define strcasecmp          _stricmp
#define strncasecmp         _strnicmp
#endif
#define inline              _inline
#if (_MSC_VER == 1600)
#define strdup              _strdup
#endif
#endif

#define MAX_FILE_PATH               256
#define MAX_PIC_SKIP_NUM            5
#define ENC_SRC_BUF_NUM             2
#define THREAD_EXIT                 0x1
#define THREAD_EXIT_SUCCESS         0x2

#define EXTRA_SRC_BUFFER_NUM        0
#define VPU_WAIT_TIME_OUT               10  //should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function
#define VPU_WAIT_TIME_OUT_CQ            1
#define PARALLEL_VPU_WAIT_TIME_OUT      1     //the value of timeout is 1 means we want to keep a waiting time to give a chance of an interrupt of the next core.

extern char* productNameList[];

typedef enum {
    MODE_YUV_LOAD = 0,
    MODE_COMP_JYUV,
    MODE_SAVE_JYUV,

    MODE_COMP_CONV_YUV,
    MODE_SAVE_CONV_YUV,

    MODE_SAVE_LOAD_YUV,

    MODE_COMP_RECON,
    MODE_SAVE_RECON,

    MODE_COMP_ENCODED,
    MODE_SAVE_ENCODED
} CompSaveMode;

typedef struct {
    int picX;
    int picY;
    int internalBitDepth;
    int losslessEnable;
    int constIntraPredFlag;
    int gopSize;
    int numTemporalLayers;
    int decodingRefreshType;
    int intraQP;
    int intraPeriod;
    int frameRate;

    int confWinTop;
    int confWinBot;
    int confWinLeft;
    int confWinRight;

    int independSliceMode;
    int independSliceModeArg;
    int dependSliceMode;
    int dependSliceModeArg;
    int intraRefreshMode;
    int intraRefreshArg;

    int useRecommendEncParam;
    int scalingListEnable;
    int cuSizeMode;
    int tmvpEnable;
    int wppenable;
    int maxNumMerge;
    int dynamicMerge8x8Enable;
    int dynamicMerge16x16Enable;
    int dynamicMerge32x32Enable;
    int disableDeblk;
    int lfCrossSliceBoundaryEnable;
    int betaOffsetDiv2;
    int tcOffsetDiv2;
    int skipIntraTrans;
    int saoEnable;
    int intraInInterSliceEnable;
    int intraNxNEnable;
    int rcEnable;

    int bitRate;
    int intraQpOffset;
    int initBufLevelx8;
    int bitAllocMode;
    int fixedBitRatio[MAX_GOP_NUM];
    int cuLevelRCEnable;
    int hvsQPEnable;

    int hvsQpScaleEnable;
    int hvsQpScale;
    int minQp;
    int maxQp;
    int maxDeltaQp;

    int initDelay;

    int transRate;
    int gopPresetIdx;
    // CUSTOM_GOP
    CustomGopParam gopParam;

    // ROI / CTU mode
    HevcCtuOptParam ctuOptParam;
    char roiFileName[MAX_FILE_PATH];

    // VUI
    HevcVuiParam vuiParam;
    Uint32 numUnitsInTick;
    Uint32 timeScale;
    Uint32 numTicksPocDiffOne;

    int encAUD;
    int encEOS;
    int encEOB;

    int chromaCbQpOffset;
    int chromaCrQpOffset;

    Uint32 initialRcQp;

    Uint32  nrYEnable;
    Uint32  nrCbEnable;
    Uint32  nrCrEnable;
    Uint32  nrNoiseEstEnable;
    Uint32  nrNoiseSigmaY;
    Uint32  nrNoiseSigmaCb;
    Uint32  nrNoiseSigmaCr;

    Uint32  nrIntraWeightY;
    Uint32  nrIntraWeightCb;
    Uint32  nrIntraWeightCr;

    Uint32  nrInterWeightY;
    Uint32  nrInterWeightCb;
    Uint32  nrInterWeightCr;

    Uint32 intraMinQp;
    Uint32 intraMaxQp;

    Uint32 useAsLongtermPeriod;
    Uint32 refLongtermPeriod;
    Uint32 vuiDataEnable;
    Uint32 vuiDataSize;
    char   vuiDataFileName[MAX_FILE_PATH];
    Uint32 hrdInVPS;
    Uint32 hrdInVUI;
    Uint32 hrdDataSize;
    char   hrdDataFileName[MAX_FILE_PATH];
    Uint32 prefixSeiEnable;
    Uint32 prefixSeiDataSize;
    Uint32 prefixSeiTimingFlag;
    char   prefixSeiDataFileName[MAX_FILE_PATH];
    Uint32 suffixSeiEnable;
    Uint32 suffixSeiDataSize;
    Uint32 suffixSeiTimingFlag;
    char   suffixSeiDataFileName[MAX_FILE_PATH];
    int    forcedIdrHeaderEnable;
} HEVC_ENC_CFG;

typedef struct {
    char SrcFileName[MAX_FILE_PATH];
    char BitStreamFileName[MAX_FILE_PATH];
    BOOL srcCbCrInterleave;
    int NumFrame;
    int PicX;
    int PicY;
    int FrameRate;

    // MPEG4 ONLY
    int VerId;
    int DataPartEn;
    int RevVlcEn;
    int ShortVideoHeader;
    int AnnexI;
    int AnnexJ;
    int AnnexK;
    int AnnexT;
    int IntraDcVlcThr;
    int VopQuant;

    // H.264 ONLY
    int ConstIntraPredFlag;
    int DisableDeblk;
    int DeblkOffsetA;
    int DeblkOffsetB;
    int ChromaQpOffset;
    int PicQpY;
    // H.264 VUI information
    int VuiPresFlag;
    int VideoSignalTypePresFlag;
    char VideoFormat;
    char VideoFullRangeFlag;
    int ColourDescripPresFlag;
    char ColourPrimaries;
    char TransferCharacteristics;
    char MatrixCoeff;
    int NumReorderFrame;
    int MaxDecBuffering;
    int aud_en;
    int level;
    // COMMON
    int GopPicNum;
    int SliceMode;
    int SliceSizeMode;
    int SliceSizeNum;
    // COMMON - RC
    int RcEnable;
    int RcBitRate;
    int RcInitDelay;
    int RcBufSize;
    int IntraRefreshNum;
    int ConscIntraRefreshEnable;
    int CountIntraMbEnable;
    int FieldSeqIntraRefreshEnable;
    int frameSkipDisable;
    int ConstantIntraQPEnable;
    int MaxQpSetEnable;
    int MaxQp;
    //H.264 only
    int MaxDeltaQpSetEnable;
    int MaxDeltaQp;
    int MinQpSetEnable;
    int MinQp;
    int MinDeltaQpSetEnable;
    int MinDeltaQp;
    int intraCostWeight;

    //MP4 Only
    int RCIntraQP;
    int HecEnable;

    int GammaSetEnable;
    int Gamma;

    // NEW RC Scheme
    int rcIntervalMode;
    int RcMBInterval;
    int skipPicNums[MAX_PIC_SKIP_NUM];
    int RcMaxIntraSize;
    int SearchRangeX;
    int SearchRangeY;
    //H.264 ONLY
    int entropyCodingMode;
    int cabacInitIdc;
    int transform8x8Mode;
    int chroma_format_400;
    int field_flag;
    int field_ref_mode;
    int RcGopIQpOffsetEn;
    int RcGopIQpOffset;

    int MeUseZeroPmv;    // will be removed. must be 264 = 0, mpeg4 = 1 263 = 0
    int MeBlkModeEnable; // only api option
    int IDRInterval;
    int SrcBitDepth;

    HEVC_ENC_CFG hevcCfg;
} ENC_CFG;



extern uint32_t randomSeed;

/* yuv & md5 */
#define NO_COMPARE       0
#define YUV_COMPARE      1
#define MD5_COMPARE      2
#define STREAM_COMPARE   3

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Performance report */
typedef void*   PFCtx;

PFCtx PFMonitorSetup(
    Uint32  coreIndex,
    Uint32  instanceIndex,
    Uint32  referenceClkInMHz,
    Uint32  fps,
    char*   strLogDir
    );

void PFMonitorRelease(
    PFCtx   context
    );

void PFMonitorUpdate(
    PFCtx   context,
    Uint32  cycles
    );

void PrepareDecoderTest(
    DecHandle decHandle
    );

void PreparationWorkForDecTest(
    DecHandle decHandle
    );

void byte_swap(
    unsigned char* data,
    int len
    );

int32_t LoadFirmware(
    int32_t     productId,
    uint8_t**   retFirmware,
    uint32_t*   retSizeInWord,
    const char* path
    );

int32_t HandleDecInitSequenceError(
    DecHandle       handle,
    Uint32          productId,
    DecOpenParam*   openParam,
    DecInitialInfo* seqInfo,
    RetCode         apiErrorCode
    );

void PrintDecSeqWarningMessages(
    Uint32          productId,
    DecInitialInfo* seqInfo
    );

void PrintMemoryAccessViolationReason(
    Uint32          core_idx,
    DecOutputInfo*  out
    );

void DisplayDecodedInformation(
    DecHandle      handle,
    CodStd         codec,
    Uint32         frameNo,
    DecOutputInfo* decodedInfo
    );

void
DisplayEncodedInformation(
    EncHandle      handle,
    CodStd         codec,
    Uint32         frameNo,
    EncOutputInfo* encodedInfo,
    ...
    );

void
PrintVpuStatus(
    Uint32 coreIdx,
    Uint32 productId
    );

void
PrintEncSppStatus(
    Uint32 coreIdx,
    Uint32 productId
    );

void WriteRegVCE(
    Uint32   core_idx,
    Uint32   vce_core_idx,
    Uint32   vce_addr,
    Uint32   udata
    );

Uint32 ReadRegVCE(
    Uint32 core_idx,
    Uint32 vce_core_idx,
    Uint32 vce_addr
    );

/*
 * VPU Helper functions
 */
/************************************************************************/
/* Video                                                                */
/************************************************************************/

#define PUT_BYTE(_p, _b) \
    *_p++ = (unsigned char)_b;

#define PUT_BUFFER(_p, _buf, _len) \
    osal_memcpy(_p, _buf, _len); \
    _p += _len;

#define PUT_LE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>16); \
    *_p++ = (unsigned char)((_var)>>24);

#define PUT_BE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>24);  \
    *_p++ = (unsigned char)((_var)>>16);  \
    *_p++ = (unsigned char)((_var)>>8); \
    *_p++ = (unsigned char)((_var)>>0);


#define PUT_LE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);


#define PUT_BE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>0);

int32_t ConvFOURCCToMp4Class(
    int32_t fourcc
    );

int32_t ConvFOURCCToCodStd(
    uint32_t fourcc
    );

int32_t ConvCodecIdToMp4Class(
    uint32_t codecId
    );

int32_t ConvCodecIdToCodStd(
    uint32_t codecId
    );

int32_t ConvCodecIdToFourcc(
    uint32_t codecId
    );

/*!
 * \brief       wrapper function of StoreYuvImageBurstFormat()
 */
uint8_t* GetYUVFromFrameBuffer(
    DecHandle       decHandle,
    FrameBuffer*    fb,
    VpuRect         rcFrame,
    uint32_t*       retWidth,
    uint32_t*       retHeight,
    uint32_t*       retBpp,
    size_t*        retSize
    );




/************************************************************************/
/* VpuMutex                                                                */
/************************************************************************/
typedef void*   VpuMutex;

VpuMutex VpuMutex_Create(
    void
    );

void VpuMutex_Destroy(
    VpuMutex   handle
    );

BOOL VpuMutex_Lock(
    VpuMutex   handle
    );

BOOL VpuMutex_Unlock(
    VpuMutex   handle
    );

/************************************************************************/
/* Queue                                                                */
/************************************************************************/
typedef struct {
    void*   data;
} QueueData;
typedef struct {
    uint8_t*    buffer;
    uint32_t    size;
    uint32_t    itemSize;
    uint32_t    count;
    uint32_t    front;
    uint32_t    rear;
    VpuMutex    lock;
} Queue;

Queue* Queue_Create(
    uint32_t    itemCount,
    uint32_t    itemSize
    );

Queue* Queue_Create_With_Lock(
    uint32_t    itemCount,
    uint32_t    itemSize
    );

void Queue_Destroy(
    Queue*      queue
    );

/**
 * \brief       Enqueue with deep copy
 */
BOOL Queue_Enqueue(
    Queue*      queue,
    void*       data
    );

/**
 * \brief       Caller has responsibility for releasing the returned data
 */
void* Queue_Dequeue(
    Queue*      queue
    );

void Queue_Flush(
    Queue*      queue
    );

void* Queue_Peek(
    Queue*      queue
    );

uint32_t Queue_Get_Cnt(
    Queue*      queue
    );

/**
 * \brief       @dstQ is NULL, it allocates Queue structure and then copy from @srcQ.
 */
Queue* Queue_Copy(
    Queue*  dstQ,
    Queue*  srcQ
    );

/************************************************************************/
/* VpuThread                                                               */
/************************************************************************/
typedef void*   VpuThread;
typedef void(*VpuThreadRunner)(void*);

VpuThread
VpuThread_Create(
    VpuThreadRunner    func,
    void*           arg
    );

BOOL
VpuThread_Join(
    VpuThread thread
    );

/*!
 * \brief           millisecond sleep
 */
void
MSleep(
    uint32_t    ms          //!<< millisecond
    );

/************************************************************************/
/* ETC                                                                  */
/************************************************************************/
uint32_t GetRandom(
    uint32_t start,
    uint32_t end
    );

#ifdef PLATFORM_WIN32
struct timezone {
    Int32 tz_minuteswest; /* minutes W of Greenwich */
    BOOL  tz_dsttime;     /* type of dst correction */
};

Int32 gettimeofday(
    struct timeval*  tv,
    struct timezone* tz
    );
#endif
/************************************************************************/
/* MD5                                                                  */
/************************************************************************/

typedef struct MD5state_st {
    uint32_t A,B,C,D;
    uint32_t Nl,Nh;
    uint32_t data[16];
    uint32_t num;
} MD5_CTX;

Int32 MD5_Init(
    MD5_CTX *c
    );

Int32 MD5_Update(
    MD5_CTX*    c,
    const void* data,
    size_t      len);

Int32 MD5_Final(
    Uint8*      md,
    MD5_CTX*    c
    );

Uint8* MD5(
    const Uint8*  d,
    size_t        n,
    Uint8*        md
    );
void plane_md5(MD5_CTX *md5_ctx,
    uint8_t *src,
    int src_x,
    int src_y,
    int out_x,
    int out_y,
    int stride,
    int bpp,
    uint16_t zero
);



/************************************************************************/
/* Comparator                                                           */
/************************************************************************/
#define COMPARATOR_SKIP 0xF0F0F0F0
typedef enum {
    COMPARATOR_CONF_SET_GOLDEN_DATA_SIZE,
    COMPARATOR_CONF_SKIP_GOLDEN_DATA,       /*!<< 2nd parameter pointer of Queue
                                                  containing skip command */
    COMPARATOR_CONF_SET_PICINFO,            //!<< This command is followed by YUVInfo structure.
} ComparatorConfType;

typedef void*   Comparator;
typedef struct ComparatorImpl {
    void*       context;
    char*       filename;
    uint32_t    curIndex;
    uint32_t    numOfFrames;
    BOOL        (*Create)(struct ComparatorImpl* impl, char* path);
    BOOL        (*Destroy)(struct ComparatorImpl* impl);
    BOOL        (*Compare)(struct ComparatorImpl* impl, void* data, uint32_t size);
    BOOL        (*Configure)(struct ComparatorImpl* impl, ComparatorConfType type, void* val);
    BOOL        (*Rewind)(struct ComparatorImpl* impl);
    BOOL        eof;
    BOOL        enableScanMode;
    BOOL        usePrevDataOneTime;
} ComparatorImpl;

typedef struct {
    Uint32          totalFrames;
    ComparatorImpl* impl;
} AbstractComparator;


// YUV Comparator
typedef struct {
    uint32_t            width;
    uint32_t            height;
    FrameBufferFormat   format;
    BOOL                cbcrInterleave;
} PictureInfo;

Comparator Comparator_Create(
    uint32_t    type,               //!<<   1: yuv
    char* goldenPath,
    ...
    );

BOOL Comparator_Destroy(
    Comparator  comp
    );

BOOL Comparator_Act(
    Comparator  comp,
    void*       data,
    uint32_t    size
    );

BOOL Comparator_CheckFrameCount(
    Comparator  comp
    );

BOOL Comparator_SetScanMode(
    Comparator  comp,
    BOOL        enable
    );

BOOL Comparator_Rewind(
    Comparator  comp
    );

Queue* Comprator_MakeSkipData(
    Comparator  comp,
    const char* skipCmd
    );

/************************************************************************/
/* Bitstream Feeder                                                     */
/************************************************************************/
typedef enum {
    FEEDING_METHOD_FIXED_SIZE,
    FEEDING_METHOD_FRAME_SIZE,
    FEEDING_METHOD_SIZE_PLUS_ES,
    FEEDING_METHOD_MAX
} FeedingMethod;

typedef struct {
    void*       data;
    uint32_t    size;
    BOOL        eos;        //!<< End of stream
} BSChunk;

typedef void* BSFeeder;

/**
 * \brief           BitstreamFeeder consumes bitstream and updates information of bitstream buffer of VPU.
 * \param handle    handle of decoder
 * \param path      bitstream path
 * \param method    feeding method. see FeedingMethod.
 * \param loopCount If @loopCount is greater than 1 then BistreamFeeder reads the start of bitstream again
 *                  when it encounters the end of stream @loopCount times.
 * \param ...       FEEDING_METHOD_FIXED_SIZE:
 *                      This value of parameter is size of chunk at a time.
 *                      If the size of chunk is equal to zero than the BitstreamFeeder reads bistream in random size.(1Byte ~ 4MB)
 * \return          It returns the pointer of handle containing the context of the BitstreamFeeder.
 */
void* BitstreamFeeder_Create(
    const char*     path,
    FeedingMethod   method,
    PhysicalAddress base,
    Uint32          size,
    ...
    );

/**
 * \brief           This is helper function set to simplify the flow that update bit-stream
 *                  to the VPU.
 */
uint32_t BitstreamFeeder_Act(
    DecHandle   decHandle,
    BSFeeder    feeder,
    EndianMode  endian
    );

/**
 * \brief           Set filling bitstream as ringbuffer mode or linebuffer mode.
 * \param   mode    0 : auto
 *                  1 : ringbuffer
 *                  2 : linebuffer.
 */
#define BSF_FILLING_AUTO                    0
#define BSF_FILLING_RINGBUFFER              1
#define BSF_FILLING_LINEBUFFER              2
/* BSF_FILLING_RINBGUFFER_WITH_ENDFLAG:
 * Scenario:
 * - Application writes 1 ~ 10 frames into bitstream buffer.
 * - Set stream end flag by using VPU_DecUpdateBitstreamBuffer(handle, 0).
 * - Application clears stream end flag by using VPU_DecUpdateBitstreamBuffer(handle, -1).
 *   when indexFrameDisplay is equal to -1.
 * NOTE:
 * - Last frame cannot be a complete frame.
 */
#define BSF_FILLING_RINGBUFFER_WITH_ENDFLAG 3
void BitstreamFeeder_SetFillMode(
    BSFeeder    feeder,
    Uint32      mode
    );

BOOL BitstreamFeeder_IsEos(
    BSFeeder    feeder
    );

BOOL BitstreamFeeder_Destroy(
    BSFeeder    feeder
    );

BOOL BitstreamFeeder_Rewind(
    BSFeeder feeder
    );

int BitstreamFeeder_Seek(
    BSFeeder feeder,
    uint64_t ts_seek
    );

/************************************************************************/
/* YUV Feeder                                                           */
/************************************************************************/
#define SOURCE_YUV                  0
#define SOURCE_YUV_WITH_LOADER      2

#define JPEG_CREATE      0x01        //!<< This command is followed by yuvInfo structure.
typedef struct {
    uint32_t   cbcrInterleave;
    uint32_t   nv21;
    uint32_t   packedFormat;
    uint32_t   srcFormat;
    uint32_t   srcPlanar;
} yuvInfo;
typedef void*   YuvFeeder;
typedef struct YuvFeederImpl {
    void*     context;
    BOOL      (*Create)(struct YuvFeederImpl* impl, const char* path, uint32_t packed, uint32_t fbStride, uint32_t fbHeight);
    BOOL      (*Feed)(struct YuvFeederImpl* impl, int32_t coreIdx, FrameBuffer *fb, size_t picWidth, size_t picHeight, void* arg);
    BOOL      (*Destroy)(struct YuvFeederImpl* impl);
    BOOL      (*Configure)(struct YuvFeederImpl* impl, uint32_t cmd, void* val);
} YuvFeederImpl;


typedef struct {
    YuvFeederImpl* impl;
    uint8_t        pYuv;
} AbstractYuvFeeder;

typedef struct {
    osal_file_t*    fp;
    uint8_t*        pYuv;
    size_t          fbStride;
    size_t          cbcrInterleave;
    BOOL            srcPlanar;
} yuvContext;

YuvFeeder YuvFeeder_Create(
    uint32_t        type,
    const char*     srcFilePath,
    uint32_t        packed,
    uint32_t        fbStride,
    uint32_t        fbHeight,
    ...
    );

BOOL YuvFeeder_Feed(
    YuvFeeder       feeder,
    Uint32          coreIdx,
    FrameBuffer*    fb,
    size_t          picWidth,
    size_t          picHeight,
    void*           arg
    );

BOOL YuvFeeder_Destroy(
    YuvFeeder feeder
    );


/************************************************************************/
/* CNM video helper                                                    */
/************************************************************************/
/**
 *  \param  convertCbcrIntl     If this value is TRUE, it stores YUV as NV12 or NV21 to @fb
 */
BOOL LoadYuvImageBurstFormat(
    Uint32      coreIdx,
    Uint8*      src,
    size_t      picWidth,
    size_t      picHeight,
    FrameBuffer *fb,
    BOOL        convertCbcrIntl
    );



void PreparationWorkForEncTest(
    EncHandle   handle
    );

int ProcessEncodedBitstreamBurst(
    Uint32 core_idx,
    osal_file_t fp,
    int targetAddr,
    PhysicalAddress bsBufStartAddr,
    PhysicalAddress bsBufEndAddr,
    int size,
    int endian,
    Comparator comparator
    );

BOOL LoadTiledImageYuvBurst(
    Uint32          coreIdx,
    BYTE*           pYuv,
    size_t          picWidth,
    size_t          picHeight,
    FrameBuffer*    fb,
    TiledMapConfig  mapCfg
    );

uint32_t StoreYuvImageBurstFormat(
    uint32_t        coreIndex,
    FrameBuffer*    fbSrc,
    TiledMapConfig  mapCfg,
    uint8_t*        pDst,
    VpuRect         cropRect,
    BOOL            enableCrop
    );

/************************************************************************/
/* Bit Reader                                                           */
/************************************************************************/
#define BS_RESET_BUF 0
#define BS_RING_BUF 1
typedef void*   BitstreamReader;
typedef struct BitstreamReaderImpl {
    void*       context;
    BOOL      (*Create)(struct BitstreamReaderImpl* impl, const char* path);
    uint32_t  (*Act)(struct BitstreamReaderImpl* impl, int32_t coreIdx, PhysicalAddress bitstreamBuffer, uint32_t bitstreamBufferSize, int endian, Comparator comparator);
    BOOL      (*Destroy)(struct BitstreamReaderImpl* impl);
    BOOL      (*Configure)(struct BitstreamReaderImpl* impl, uint32_t cmd, void* val);
} BitstreamReaderImpl;

/*!
 * \param   type                0: Linebuffer, 1: Ringbuffer
 * \param   path                output filepath.
 * \param   endian              Endianness of bitstream buffer
 * \param   handle              Pointer of encoder handle
 */
BitstreamReader BitstreamReader_Create(
    uint32_t    type,
    char*       path,
    EndianMode  endian,
    EncHandle*  handle
    );

/*!
 * \param   bitstreamBuffer     base address of bitstream buffer
 * \param   bitstreamBufferSize size of bitstream buffer
 */
BOOL BitstreamReader_Act(
    BitstreamReader reader,
    PhysicalAddress bitstreamBuffer,
    uint32_t        bitstreamBufferSize,
    uint32_t        defaultsize,
    Comparator      comparator
    );

BOOL BitstreamReader_Destroy(
    BitstreamReader reader
    );

/************************************************************************/
/* Binary Reader                                                           */
/************************************************************************/

/************************************************************************/
/* Simple Renderer                                                      */
/************************************************************************/
typedef void*       Renderer;

typedef enum {
    RENDER_DEVICE_NULL,
    RENDER_DEVICE_FBDEV,
    RENDER_DEVICE_HDMI,
    RENDER_DEVICE_MAX
} RenderDeviceType;

typedef struct RenderDevice {
    void*       context;
    DecHandle   decHandle;
    BOOL (*Open)(struct RenderDevice* device);
    void (*Render)(struct RenderDevice* device, DecOutputInfo* fbInfo, uint8_t* yuv, uint32_t width, uint32_t height);
    BOOL (*Close)(struct RenderDevice* device);
} RenderDevice;

Renderer SimpleRenderer_Create(
    DecHandle           decHandle,
    RenderDeviceType    deviceType,
    const char*         yuvPath            //!<< path to store yuv iamge.
    );

uint32_t SimpleRenderer_Act(
    Renderer        renderer,
    DecOutputInfo*  fbInfo,
    uint8_t*        pYuv,
    uint32_t        width,
    uint32_t        height
    );

void* SimpleRenderer_GetFreeFrameInfo(
    Renderer        renderer
    );

/* \brief       Flush display queues and clear display indexes
 */
void SimpleRenderer_Flush(
    Renderer        renderer
    );

BOOL SimpleRenderer_Destroy(
    Renderer    renderer
    );

BOOL SimpleRenderer_SetFrameRate(
    Renderer        renderer,
    Uint32          fps
    );


/************************************************************************/
/* Etc                                                                  */
/************************************************************************/
typedef struct {
    DecOutputInfo*  buffer;
    int             size;
    int             count;
    int             front;
    int             rear;
} frame_queue_item_t;

frame_queue_item_t* frame_queue_init(
    Int32   count
    );

void frame_queue_deinit(
    frame_queue_item_t* queue
    );

Int32 frame_queue_enqueue(
    frame_queue_item_t* queue,
    DecOutputInfo data
    );

Int32 frame_queue_dequeue(
    frame_queue_item_t* queue,
    DecOutputInfo*      data
    );

Int32 frame_queue_dequeue_all(
    frame_queue_item_t* queue
    );

Int32 frame_queue_peekqueue(
    frame_queue_item_t* queue,
    DecOutputInfo*      data
    );

Int32 frame_queue_count(
    frame_queue_item_t* queue
    );

Int32 frame_queue_check_in_queue(
    frame_queue_item_t* queue,
    Int32               index
    );

/*******************************************************************************
 * DATATYPES AND FUNCTIONS RELATED TO REPORT
 *******************************************************************************/
typedef struct
{
    osal_file_t     fpPicDispInfoLogfile;
    osal_file_t     fpPicTypeLogfile;
    osal_file_t     fpSeqDispInfoLogfile;
    osal_file_t     fpUserDataLogfile;
    osal_file_t     fpSeqUserDataLogfile;


    // encoder report file
    osal_file_t     fpEncSliceBndInfo;
    osal_file_t     fpEncQpInfo;
    osal_file_t     fpEncmvInfo;
    osal_file_t     fpEncsliceInfo;

    // Report Information
    BOOL            reportOpened;
    Int32           decIndex;
    vpu_buffer_t    vb_rpt;
    BOOL            userDataEnable;
    BOOL            userDataReportMode;

    Int32           profile;
    Int32           level;
} vpu_rpt_info_t;

typedef struct VpuReportConfig_t {
    PhysicalAddress userDataBufAddr;
    BOOL            userDataEnable;
    Int32           userDataReportMode; // (0 : Int32errupt mode, 1 Int32errupt disable mode)
    Int32           userDataBufSize;

} VpuReportConfig_t;

void OpenDecReport(
    Uint32              core_idx,
    VpuReportConfig_t*  cfg
    );

void CloseDecReport(
    Uint32 core_idx
    );

void ConfigDecReport(
    Uint32      core_idx,
    DecHandle   handle,
    CodStd      bitstreamFormat
    );

void SaveDecReport(
    Uint32          core_idx,
    DecHandle       handle,
    DecOutputInfo*  pDecInfo,
    CodStd          bitstreamFormat,
    Uint32          mbNumX,
    Uint32          mbNumY
    );

void CheckUserDataInterrupt(
    Uint32      core_idx,
    DecHandle   handle,
    Int32       decodeIdx,
    CodStd      bitstreamFormat,
    Int32       int_reason
    );

#define MAX_CFG                 (167)
#define MAX_ROI_LEVEL           (8)
#define LOG2_CTB_SIZE           (6)
#define CTB_SIZE                (1<<LOG2_CTB_SIZE)
#define LAMBDA_SCALE_FACTOR     (100000)
#define FLOATING_POINT_LAMBDA   (1)
#define TEMP_SCALABLE_RC        (1)
#define UI16_MAX                (0xFFFF)
#ifndef INT_MAX
#define INT_MAX                 (2147483647)
#endif

typedef enum {
    INPUT_FILE = 0,
    SOURCE_WIDTH,
    SOURCE_HEIGHT,
    INPUT_BIT_DEPTH,
    FRAME_RATE,                   // 5
    FRAME_SKIP,
    FRAMES_TO_BE_ENCODED,
    INTRA_PERIOD,
    DECODING_REFRESH_TYPE,
    DERIVE_LAMBDA_WEIGHT,         // 10
    GOP_SIZE,
    EN_INTRA_IN_INTER_SLICE,
    INTRA_NXN,
    EN_CU_8X8,
    EN_CU_16X16,                  // 15
    EN_CU_32X32,
    INTRA_TRANS_SKIP,
    CONSTRAINED_INTRA_PRED,
    INTRA_CTU_REFRESH_MODE,
    INTRA_CTU_REFRESH_ARG,        // 20
    MAX_NUM_MERGE,
    EN_DYN_MERGE,
    EN_TEMPORAL_MVP,
    SCALING_LIST,
    INDE_SLICE_MODE,              // 25
    INDE_SLICE_ARG,
    DE_SLICE_MODE,
    DE_SLICE_ARG,
    EN_DBK,
    EN_SAO,                       // 30
    LF_CROSS_SLICE_BOUNDARY_FLAG,
    BETA_OFFSET_DIV2,
    TC_OFFSET_DIV2,
    WAVE_FRONT_SYNCHRO,
    LOSSLESS_CODING,              // 35
    USE_PRESENT_ENC_TOOLS,
    NUM_TEMPORAL_LAYERS,
    GOP_PRESET,
    RATE_CONTROL,
    ENC_BITRATE,                  // 40
    TRANS_BITRATE,
    INITIAL_DELAY,
    EN_HVS_QP,
    CU_LEVEL_RATE_CONTROL,
    CONF_WIND_SIZE_TOP,           // 45
    CONF_WIND_SIZE_BOT,
    CONF_WIND_SIZE_RIGHT,
    CONF_WIND_SIZE_LEFT,
    HVS_QP_SCALE_DIV2,
    MIN_QP,                       // 50
    MAX_QP,
    MAX_DELTA_QP,
    NUM_ROI,
    INTRA_QP,
    ROI_DELTA_QP,                 // 55
    INTRA_QP_OFFSET,
    INIT_BUF_LEVELx8,
    BIT_ALLOC_MODE,
    FIXED_BIT_RATIO,
    INTERNAL_BITDEPTH,            // 60
    EN_USER_DATA,
    USER_DATA_ENC_ORDER,
    USER_DATA_SIZE,
    USER_DATA_SUFFIX_FLAG,
    ROI_ENABLE,                   // 65
    VUI_PARAM_FLAG,
    VUI_ASPECT_RATIO_IDC,
    VUI_SAR_SIZE,
    VUI_OVERSCAN_APPROPRIATE,
    VIDEO_SIGNAL,                 // 70
    VUI_CHROMA_SAMPLE_LOC,
    VUI_DISP_WIN_LEFT_RIGHT,
    VUI_DISP_WIN_TOP_BOTTOM,
    NUM_UNITS_IN_TICK,
    TIME_SCALE,                   // 75
    NUM_TICKS_POC_DIFF_ONE,
    ENC_AUD,
    ENC_EOS,
    ENC_EOB,
    CB_QP_OFFSET,                // 80
    CR_QP_OFFSET,
    RC_INIT_QP,
    EN_NR_Y,
    EN_NR_CB,
    EN_NR_CR,                   // 85
    EN_NOISE_EST,
    NOISE_SIGMA_Y,
    NOISE_SIGMA_CB,
    NOISE_SIGMA_CR,
    INTRA_NOISE_WEIGHT_Y,       // 90
    INTRA_NOISE_WEIGHT_CB,
    INTRA_NOISE_WEIGHT_CR,
    INTER_NOISE_WEIGHT_Y,
    INTER_NOISE_WEIGHT_CB,
    INTER_NOISE_WEIGHT_CR,      // 95
    INTRA_MIN_QP,
    INTRA_MAX_QP,
    MDFLAG0,
    MDFLAG1,
    MDFLAG2,        // 100
    EN_SMART_BG,
    THRPIXELNUMCNT,
    THRMEAN0,
    THRMEAN1,
    THRMEAN2,      // 105
    THRMEAN3,
    MDQPY,
    MDQPC,
    THRDCY0,
    THRDCC0,       // 110
    THRDCY1,
    THRDCC1,
    THRDCY2,
    THRDCC2,
    THRACNUMY0,    // 115
    THRACNUMC0,
    THRACNUMY1,
    THRACNUMC1,
    THRACNUMY2,
    THRACNUMC2,    // 120
    USE_LONGTERM_PRRIOD,
    REF_LONGTERM_PERIOD,
    EN_CTU_MODE,
    EN_CTU_QP,
    CROP_X_POS,     // 125
    CROP_Y_POS,
    CROP_X_SIZE,
    CROP_Y_SIZE,
    EN_VUI_DATA,
    VUI_DATA_SIZE,            // 130
    EN_HRD_IN_VPS,
    EN_HRD_IN_VUI,
    HRD_DATA_SIZE,
    EN_PREFIX_SEI_DATA,
    PREFIX_SEI_DATA_SIZE,   // 135
    PREFIX_SEI_TIMING_FLAG,
    EN_SUFFIX_SEI_DATA,
    SUFFIX_SEI_DATA_SIZE,
    SUFFIX_SEI_TIMING_FLAG,
    EN_REPORT_MV_COL,     // 140
    EN_REPORT_DIST_MAP,
    EN_REPORT_BIT_INFO,
    EN_REPORT_FRAME_DIST,
    EN_REPORT_QP_HISTO,
    BITSTREAM_FILE,                  // 145
    EN_CUSTOM_VPS,
    EN_CUSTOM_SPS,
    EN_CUSTOM_PPS,
    CUSTOM_VPS_PSID,
    CUSTOM_SPS_PSID,                // 150
    CUSTOM_SPS_ACTIVE_VPSID,
    CUSTOM_PPS_ACTIVE_SPSID,
    CUSTOM_VPS_INTFLAG,
    CUSTOM_VPS_AVAILFLAG,
    CUSTOM_VPS_MAXLAYER_MINUS1,     // 155
    CUSTOM_VPS_MAXSUBLAYER_MINUS1,
    CUSTOM_VPS_TEMPID_NESTFLAG,
    CUSTOM_VPS_MAXLAYER_ID,
    CUSTOM_VPS_NUMLAYER_SETMINUS1,
    CUSTOM_VPS_EXTFLAG,             // 160
    CUSTOM_VPS_EXTDATAFLAG,
    CUSTOM_VPS_SUBORDER_INFOFLAG,
    CUSTOM_SPS_SUBORDER_INFOFLAG,
    CUSTOM_VPS_LAYERID_0,
    CUSTOM_VPS_LAYERID_1,           // 165
    CUSTOM_SPS_LOG2_MAXPOC_MINUS4,
    EN_FORCED_IDR_HEADER,
} HevcCfgName;

typedef struct {
    char  *name;
    int    min;
    int    max;
    int    def;
} HevcCfgInfo;

int32_t GetEncOpenParamChange(
    EncOpenParam*   pEncOP,
    char*           cfgFileName,
    ENC_CFG*        pEncCfg,
    EncHandle       handle
    );

void PrintVpuVersionInfo(
    uint32_t coreIdx
    );


void ChangePathStyle(
    char *str
    );

BOOL CalcYuvSize(
    int32_t format,
    int32_t picWidth,
    int32_t picHeight,
    int32_t cbcrInterleave,
    size_t  *lumaSize,
    size_t  *chromaSize,
    size_t  *frameSize,
    int32_t *bitDepth,
    int32_t *packedFormat,
    int32_t *yuv3p4b
    );


int GetPackedFormat (
    int srcBitDepth,
    int packedType,
    int p10bits,
    int msb
    );


BOOL MkDir(
    char* path
    );

char* GetDirname(
    const char* path
    );

char* GetBasename(
    const char* pathname
    );

char* GetFileExtension(
    const char* filename
    );

int parseAvcCfgFile(
    ENC_CFG*    pEncCfg,
    char*       filename
    );

int parseMp4CfgFile(
    ENC_CFG*    pEncCfg,
    char*       filename
    );

int parseHevcCfgFile(
    ENC_CFG*    pEncCfg,
    char*       FileName
    );

int parseRoiCtuModeParam(
    char* lineStr,
    VpuRect* roiRegion,
    int* roiLevel,
    int picX,
    int picY
    );

int64_t GetNowUs();

#ifdef __cplusplus
}
#endif /* __cplusplus */

/************************************************************************/
/* Structure                                                            */
/************************************************************************/
typedef struct TestDecConfig_struct {
    char                outputPath[MAX_FILE_PATH];
    char                inputPath[MAX_FILE_PATH];
    uint32_t            forceOutNum;
    int32_t             bitFormat;
    int32_t             reorder;
    TiledMapType        mapType;
    int32_t             bitstreamMode;
    FeedingMethod       feedingMode;
    BOOL                enableWTL;
    FrameFlag           wtlMode;
    FrameBufferFormat   wtlFormat;
    int32_t             coreIdx;
    int32_t             instIdx;
    BOOL                enableCrop;                 //!<< option for saving yuv
    uint32_t            loopCount;
    BOOL                cbcrInterleave;             //!<< FALSE: None, TRUE: NV12
    BOOL                nv21;                       //!<< FALSE: NV12, TRUE: NV21,
                                                    //!<< This variable is valid when cbcrInterleave is TRUE
    EndianMode          streamEndian;
    EndianMode          frameEndian;
    int32_t             secondaryAXI;
    int32_t             compareType;
    char                md5Path[MAX_FILE_PATH];
    char                fwPath[MAX_FILE_PATH];
    char                refYuvPath[MAX_FILE_PATH];
    RenderDeviceType    renderType;
    BOOL                thumbnailMode;
    int32_t             skipMode;
    size_t              bsSize;
    uint32_t            scaleDownWidth;
    uint32_t            scaleDownHeight;
    struct {
        BOOL        enableMvc;                      //!<< H.264 MVC
        BOOL        enableTiled2Linear;
        FrameFlag   tiled2LinearMode;
        BOOL        enableBWB;
        uint32_t    rotate;                         //!<< 0, 90, 180, 270
        uint32_t    mirror;
        BOOL        enableDering;                   //!<< MPEG-2/4
        BOOL        enableDeblock;                  //!<< MPEG-2/4
        uint32_t    mp4class;                       //!<< MPEG_4
        uint32_t    frameCacheBypass;
        uint32_t    frameCacheBurst;
        uint32_t    frameCacheMerge;
        uint32_t    frameCacheWayShape;
    } coda9;
    struct {
        uint32_t    numVCores;                      //!<< This numVCores is valid on PRODUCT_ID_4102 multi-core version
        uint32_t    fbcMode;
        BOOL        bwOptimization;                 //!<< On/Off bandwidth optimization function
        BOOL        craAsBla;
        BOOL        dualDisplay;
    } wave4;
    BOOL            enableUserData;
    BOOL            pvricFbcEnable;
    Uint32          pvric31HwSupport;
    Uint32          pvricPaddingY;
    Uint32          pvricPaddingC;
    int             sramMode;
    int             invalidDisFlag;
    int             seekflag;
    int             exitFlag;

} TestDecConfig;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void ReleaseVideoMemory(
    uint32_t        coreIndex,
    vpu_buffer_t*   memoryArr,
    uint32_t        count
    );

BOOL AllocateDecFrameBuffer(
    DecHandle       decHandle,
    TestDecConfig*  config,
    uint32_t        tiledFbCount,
    uint32_t        linearFbCount,
    FrameBuffer*    retFbArray,
    vpu_buffer_t*   retFbAddrs,
    uint32_t*       retStride
    );

void HandleDecoderError(
    DecHandle       handle,
    uint32_t        frameIdx,
    TestDecConfig*  param,
    vpu_buffer_t*   fbMem,
    DecOutputInfo*  outputInfo
    );

#define OUTPUT_FP_NUMBER 4
BOOL OpenDisplayBufferFile(
    char *outputPath,
    VpuRect rcDisplay,
    TiledMapType mapType,
    FILE *fp[]
    );

void CloseDisplayBufferFile(
    FILE *fp[]
    );

void SaveDisplayBufferToFile(
    DecHandle handle,
    FrameBuffer dispFrame,
    VpuRect rcDisplay,
    FILE *fp[],
    const char* path,
    Uint32 index
    );

Uint32 CalcScaleDown(
    Uint32 origin,
    Uint32 scaledValue
    );


#ifdef __cplusplus
}
#endif /* __cplusplus */

typedef struct TestEncConfig_struct {
    char    yuvSourceBaseDir[MAX_FILE_PATH];
    char    yuvFileName[MAX_FILE_PATH];
    char    cmdFileName[MAX_FILE_PATH];
    char    bitstreamFileName[MAX_FILE_PATH];
    char    huffFileName[MAX_FILE_PATH];
    char    cInfoFileName[MAX_FILE_PATH];
    char    qMatFileName[MAX_FILE_PATH];
    char    qpFileName[MAX_FILE_PATH];
    char    cfgFileName[MAX_FILE_PATH];
    CodStd  stdMode;
    int     picWidth;
    int     picHeight;
    int     kbps;
    int     rotAngle;
    int     mirDir;
    int     useRot;
    int     qpReport;
    int     ringBufferEnable;
    int     rcIntraQp;
    int     outNum;
    int     skipPicNums[MAX_PIC_SKIP_NUM];
    int     instNum;
    int     coreIdx;
    int     mapType;
    // 2D cache option


    int lineBufIntEn;
    int en_container;                   //enable container
    int container_frame_rate;           //framerate for container
    int picQpY;

    int cbcrInterleave;
    int nv21;
    BOOL needSourceConvert;         //!<< If the format of YUV file is YUV planar mode and EncOpenParam::cbcrInterleave or EncOpenParam::nv21 is true
                                    //!<< the value of needSourceConvert should be true.
    int packedFormat;
    int srcFormat;
    int srcFormat3p4b;
    int bitdepth;
    int secondary_axi;
    int stream_endian;
    int frame_endian;
    int source_endian;

    int compare_type;
#define YUV_MODE_YUV 0
#define YUV_MODE_JYUV_CONV 1
#define YUV_MODE_YUV_LOADER 2
    int yuv_mode;
    char ref_stream_path[MAX_FILE_PATH];
    int loopCount;
    char ref_recon_md5_path[MAX_FILE_PATH];
    char roi_file_name[MAX_FILE_PATH];
    FILE *roi_file;
    int roi_enable;
    int roi_delta_qp;

    //char user_data_file_name[MAX_FILE_PATH];
    //FILE *user_data_fp;
    HevcSEIDataEnc seiDataEnc;

    char hrd_rbsp_file_name[MAX_FILE_PATH];
    FILE *hrd_rbsp_fp;

    char vui_rbsp_file_name[MAX_FILE_PATH];
    FILE *vui_rbsp_fp;

    char prefix_sei_nal_file_name[MAX_FILE_PATH];
    FILE *prefix_sei_nal_fp;

    char suffix_sei_nal_file_name[MAX_FILE_PATH];
    FILE *suffix_sei_nal_fp;

    int encAUD;
    int encEOS;
    int encEOB;
    struct {
        BOOL        enableLinear2Tiled;
        FrameFlag   linear2TiledMode;
    } coda9;
    VpuRect region[MAX_ROI_NUMBER];        /**< The size of the ROI region for H.265 (start X/Y in CTU, end X/Y int CTU)  */
    int roiLevel[MAX_ROI_NUMBER];       /**< An importance level for the given ROI region for H.265. The higher an ROI level is, the more important the region is with a lower QP.  */

    int useAsLongtermPeriod;
    int refLongtermPeriod;
    int sramMode;
    int exitFlag;
} TestEncConfig;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
int32_t GetEncOpenParam(
    EncOpenParam*   pEncOP,
    TestEncConfig*  pEncConfig,
    ENC_CFG*        pEncCfg
    );

Int32 GetEncOpenParamDefault(
    EncOpenParam*   pEncOP,
    TestEncConfig*  pEncConfig
    );

void GenRegionToMap(
    VpuRect *region,        /**< The size of the ROI region for H.265 (start X/Y in CTU, end X/Y in CTU)  */
    int *roiLevel,
    int num,
    Uint32 mapWidth,
    Uint32 mapHeight,
    Uint8 *roiCtuMap
    );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif    /* _MAIN_HELPER_H_ */
