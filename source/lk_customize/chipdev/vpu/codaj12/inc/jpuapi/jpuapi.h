//------------------------------------------------------------------------------
// File: JpuApi.h
//
// Copyright (c) 2006~2011, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef JPUAPI_H_INCLUDED
#define JPUAPI_H_INCLUDED

#include "jpuconfig.h"
#include "jdi.h"
#include "jputypes.h"

/* _n: number, _s: significance */
#define JPU_CEIL(_s, _n)        (((_n)+(_s-1))&~(_s-1))
#define JPU_FLOOR(_s, _n)       (_n&~(_s-1))
//------------------------------------------------------------------------------
// common struct and definition
//------------------------------------------------------------------------------

typedef enum {
    /* Non-differential, Huffman coding */
    JPEG_BASELINE_DCT,
    JPEG_EXTENDED_SEQUENTIAL_DCT,
    /* The others are not supported on CODAJ12 */
} JpgProfile;


typedef enum {
    SET_JPG_SCALE_HOR,
    SET_JPG_SCALE_VER,
    SET_JPG_USE_STUFFING_BYTE_FF,
    ENC_JPG_GET_HEADER,
    ENABLE_LOGGING,
    DISABLE_LOGGING,
    JPG_CMD_END
} JpgCommand;



typedef enum {
    JPG_RET_SUCCESS,
    JPG_RET_FAILURE,
    JPG_RET_BIT_EMPTY,
    JPG_RET_EOS,
    JPG_RET_INVALID_HANDLE,
    JPG_RET_INVALID_PARAM,
    JPG_RET_INVALID_COMMAND,
    JPG_RET_ROTATOR_OUTPUT_NOT_SET,
    JPG_RET_ROTATOR_STRIDE_NOT_SET,
    JPG_RET_FRAME_NOT_COMPLETE,
    JPG_RET_INVALID_FRAME_BUFFER,
    JPG_RET_INSUFFICIENT_FRAME_BUFFERS,
    JPG_RET_INVALID_STRIDE,
    JPG_RET_WRONG_CALL_SEQUENCE,
    JPG_RET_CALLED_BEFORE,
    JPG_RET_NOT_INITIALIZED,
    JPG_RET_INSUFFICIENT_RESOURCE,
    JPG_RET_INST_CTRL_ERROR,
    JPG_RET_NOT_SUPPORT,
} JpgRet;


typedef enum {
    ENCODE_STATE_NEW_FRAME = 0,
    ENCODE_STATE_FRAME_DONE = 0,
    ENCODE_STATE_SLICE_DONE = 1
} EncodeState;

typedef enum {
    DECODE_STATE_NEW_FRAME = 0,
    DECODE_STATE_FRAME_DONE = 0,
    DECODE_STATE_SLICE_DONE = 1
} DecodeState;

typedef enum {
    MIRDIR_NONE,
    MIRDIR_VER,
    MIRDIR_HOR,
    MIRDIR_HOR_VER
} JpgMirrorDirection;

typedef enum {
    FORMAT_420 = 0,
    FORMAT_422 = 1,
    FORMAT_440 = 2,
    FORMAT_444 = 3,
    FORMAT_400 = 4,
    FORMAT_MAX
} FrameFormat;


typedef enum {
    CBCR_ORDER_NORMAL,
    CBCR_ORDER_REVERSED
} CbCrOrder;


typedef enum {
    CBCR_SEPARATED = 0,
    CBCR_INTERLEAVE,
    CRCB_INTERLEAVE
} CbCrInterLeave;

typedef enum {
    PACKED_FORMAT_NONE,
    PACKED_FORMAT_422_YUYV,
    PACKED_FORMAT_422_UYVY,
    PACKED_FORMAT_422_YVYU,
    PACKED_FORMAT_422_VYUY,
    PACKED_FORMAT_444,
    PACKED_FORMAT_MAX
} PackedFormat;

typedef enum {
    O_FMT_NONE,
    O_FMT_420,
    O_FMT_422,
    O_FMT_444,
    O_FMT_MAX
} OutputFormat;

/* Assume that pixel endianness is big-endian.
 * b0 is low address in a framebuffer.
 * b1 is high address in a framebuffer.
 * pixel consists of b0 and b1.
 * RIGHT JUSTIFICATION: (default)
 * lsb         msb
 * |----|--------|
 * |0000| pixel  |
 * |----|--------|
 * | b0   |   b1 |
 * |-------------|
 * LEFT JUSTIFICATION:
 * lsb         msb
 * |--------|----|
 * | pixel  |0000|
 * |--------|----|
 * | b0   |   b1 |
 * |-------------|
 */
enum {
    PIXEL_16BIT_MSB_JUSTIFIED,
    PIXEL_16BIT_LSB_JUSTIFIED,
    PIXEL_16BIT_JUSTIFICATION_MAX,
};

typedef enum {
    INT_JPU_DONE = 0,
    INT_JPU_ERROR = 1,
    INT_JPU_BIT_BUF_EMPTY = 2,
    INT_JPU_BIT_BUF_FULL = 2,
    INT_JPU_SLICE_DONE = 9,
} InterruptJpu;

typedef enum {
    JPG_TBL_NORMAL,
    JPG_TBL_MERGE
} JpgTableMode;

typedef enum {
    ENC_HEADER_MODE_NORMAL,
    ENC_HEADER_MODE_SOS_ONLY
} JpgEncHeaderMode;

enum {
    JPG_SCALE_DOWN_NONE,
    JPG_SCALE_DOWN_ONE_HALF,
    JPG_SCALE_DOWN_ONE_QUARTER,
    JPG_SCALE_DOWN_ONE_EIGHTS,
    JPG_SCALE_DOWN_MAX
};

enum {
    PRODUCT_ID_CODAJ12 = 12,
};

typedef struct {
    PhysicalAddress bufY;
    PhysicalAddress bufCb;
    PhysicalAddress bufCr;
    Uint32          stride;         /*!<< luma stride */
    Uint32          strideC;        /*!<< chroma stride */
    FrameFormat     format;
    Uint32          endian;
} FrameBuffer;


struct JpgInst;
typedef struct JpgInst *JpgHandle;

//------------------------------------------------------------------------------
// decode struct and definition
//------------------------------------------------------------------------------

typedef struct JpgInst JpgDecInst;
typedef JpgDecInst *JpgDecHandle;

/* JPU Capabilities */
typedef struct {
    Uint32  productId;                          /*!<< Product ID number */
    Uint32  revisoin;                           /*!<< Revision number */
    BOOL    support12bit;                       /*!<< able to encode or decode a extended sequential JPEG */
} JPUCap;

typedef struct {
    PhysicalAddress bitstreamBuffer;
    Uint32          bitstreamBufferSize;
    BYTE           *pBitStream;
    Uint32          streamEndian;
    Uint32          frameEndian;
    CbCrInterLeave  chromaInterleave;
    BOOL            thumbNailEn;
    PackedFormat    packedFormat;
    BOOL            roiEnable;
    Uint32          roiOffsetX;
    Uint32          roiOffsetY;
    Uint32          roiWidth;
    Uint32          roiHeight;
    Uint32          pixelJustification;         /*!<< avaliable in 16bit pixel. */
    Uint32          sliceHeight;
    Uint32          intrEnableBit;
    Uint32          rotation;                       /*!<< 0, 90, 180, 270 */
    JpgMirrorDirection  mirror;                     /*!<< 0(none), 1(vertical), 2(mirror), 3(both) */
    FrameFormat     outputFormat;
    BOOL            sliceInstMode;
} JpgDecOpenParam;

typedef struct {
    int         picWidth;
    int         picHeight;
    int         minFrameBufferCount;
    FrameFormat sourceFormat;
    int         ecsPtr;
    int         roiFrameWidth;
    int         roiFrameHeight;
    int         roiFrameOffsetX;
    int         roiFrameOffsetY;
    int         roiMCUSize;
    int         colorComponents;
    Uint32      bitDepth;
} JpgDecInitialInfo;


typedef struct {
    int scaleDownRatioWidth;
    int scaleDownRatioHeight;
} JpgDecParam;


typedef struct {
    int indexFrameDisplay;
    int numOfErrMBs;
    int decodingSuccess;
    int decPicHeight;
    int decPicWidth;
    int consumedByte;
    int bytePosFrameStart;
    int ecsPtr;
    Uint32  frameCycle;         /*!<< clock cycle */
    Uint32  rdPtr;
    Uint32  wrPtr;
    Uint32  decodedSliceYPos;
    DecodeState decodeState;
} JpgDecOutputInfo;



//------------------------------------------------------------------------------
// encode struct and definition
//------------------------------------------------------------------------------

typedef struct JpgInst JpgEncInst;
typedef JpgEncInst *JpgEncHandle;


typedef struct {
    PhysicalAddress bitstreamBuffer;
    Uint32          bitstreamBufferSize;
    Uint32          picWidth;
    Uint32          picHeight;
    FrameFormat     sourceFormat;
    Uint32          restartInterval;
    Uint32          streamEndian;
    Uint32          frameEndian;
    CbCrInterLeave  chromaInterleave;
    BYTE            huffVal[8][256];
    BYTE            huffBits[8][256];
    short           qMatTab[4][64];
    BOOL            jpg12bit;
    BOOL            q_prec0;
    BOOL            q_prec1;
    PackedFormat    packedFormat;
    Uint32          pixelJustification;
    Uint32          tiledModeEnable;
    Uint32          sliceHeight;
    Uint32          intrEnableBit;
    BOOL            sliceInstMode;
    Uint32          rotation;
    Uint32          mirror;
} JpgEncOpenParam;



typedef struct {
    FrameBuffer *sourceFrame;
} JpgEncParam;


typedef struct {
    PhysicalAddress bitstreamBuffer;
    Uint32 bitstreamSize;
    PhysicalAddress streamRdPtr;
    PhysicalAddress streamWrPtr;
    Uint32 encodedSliceYPos;
    EncodeState encodeState;
    Uint32  frameCycle;         /*!<< clock cycle */
} JpgEncOutputInfo;

typedef struct {
    PhysicalAddress paraSet;
    BYTE *pParaSet;
    int size;
    int headerMode;
    int quantMode;
    int huffMode;
    int disableAPPMarker;
    int enableSofStuffing;
} JpgEncParamSet;



#ifdef __cplusplus
extern "C" {
#endif


Uint32      JPU_IsInit(void);
Int32       JPU_InterruptSortOut(Int32* instIndex, Uint32* reason);
Int32       JPU_WaitInterrupt(JpgHandle handle, int timeout);
int         JPU_IsBusy(JpgHandle handle);
Uint32      JPU_GetStatus(JpgHandle handle);
void        JPU_ClrStatus(JpgHandle handle, Uint32 val);
void        JPU_ClrStatusEx(Int32 instIdx, Uint32 val);

JpgRet      JPU_Init(unsigned long phy_base, unsigned int reg_range);
void        JPU_DeInit(void);
int         JPU_GetOpenInstanceNum(void);
JpgRet      JPU_GetVersionInfo(
    Uint32 *apiVersion,
    Uint32 *hwRevision,
    Uint32 *hwProductId);

// function for decode
JpgRet JPU_DecOpen(JpgDecHandle *, JpgDecOpenParam *);
JpgRet JPU_DecClose(JpgDecHandle);
JpgRet JPU_DecGetInitialInfo(
    JpgDecHandle handle,
    JpgDecInitialInfo *info);
JpgRet JPU_DecRegisterFrameBuffer(
    JpgDecHandle handle,
    FrameBuffer *bufArray,
    int num,
    int stride);
JpgRet JPU_DecGetBitstreamBuffer(
    JpgDecHandle handle,
    PhysicalAddress *prdPrt,
    PhysicalAddress *pwrPtr,
    int *size );
JpgRet JPU_DecUpdateBitstreamBuffer(
    JpgDecHandle handle,
    int size);
JpgRet JPU_HWReset(void);
JpgRet JPU_SWReset(JpgHandle handle);
JpgRet JPU_DecStartOneFrame(
    JpgDecHandle handle,
    JpgDecParam *param );
JpgRet JPU_DecGetOutputInfo(
    JpgDecHandle handle,
    JpgDecOutputInfo *info);
JpgRet JPU_DecGiveCommand(
    JpgDecHandle handle,
    JpgCommand cmd,
    void *parameter);
JpgRet JPU_DecSetRdPtr(
    JpgDecHandle handle,
    PhysicalAddress addr,
    BOOL updateWrPtr);
JpgRet JPU_DecSetRdPtrEx(
    JpgDecHandle handle,
    PhysicalAddress addr,
    BOOL updateWrPtr);

// function for encode
JpgRet JPU_EncOpen(JpgEncHandle *, JpgEncOpenParam *);
JpgRet JPU_EncClose(JpgEncHandle);
JpgRet JPU_EncGetBitstreamBuffer(
    JpgEncHandle handle,
    PhysicalAddress *prdPrt,
    PhysicalAddress *pwrPtr,
    int *size);
JpgRet JPU_EncUpdateBitstreamBuffer(
    JpgEncHandle handle,
    int size);
JpgRet JPU_EncStartOneFrame(
    JpgEncHandle handle,
    JpgEncParam *param );
JpgRet JPU_EncGetOutputInfo(
    JpgEncHandle handle,
    JpgEncOutputInfo *info);
JpgRet JPU_DecGetOutputInfoEx(
    JpgDecHandle handle,
    JpgDecOutputInfo *info,
    Int32 pic_status);
JpgRet JPU_EncGiveCommand(
    JpgEncHandle handle,
    JpgCommand cmd,
    void *parameter);
void JPU_EncSetHostParaAddr(
    PhysicalAddress baseAddr,
    PhysicalAddress paraAddr);

JpgRet JPU_EncGetOutputInfoEx(
    JpgEncHandle handle,
    JpgEncOutputInfo *info,
    int reason
);


#ifdef __cplusplus
}
#endif

#endif
