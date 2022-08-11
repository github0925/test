//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2011  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#ifndef VPUAPI_UTIL_H_INCLUDED
#define VPUAPI_UTIL_H_INCLUDED


#include "vpuapi.h"

// COD_STD
enum {
    HEVC_DEC = 0,
    AVC_DEC  = 0,
    VC1_DEC  = 1,
    HEVC_ENC  = 1,
    MP2_DEC  = 2,
    MP4_DEC  = 3,
    DV3_DEC  = 3,
    RV_DEC   = 4,
    AVS_DEC  = 5,
    VPX_DEC  = 7,
    MAX_DEC  = 7,
    AVC_ENC  = 8,
    MP4_ENC  = 11,
    MAX_CODECS,
};

// new COD_STD since Coda7Q, WAVE412
enum {
    C7_HEVC_DEC                 = 0x00,
    C7_HEVC_ENC                 = 0x01,

    C7_AVC_DEC                  = 0x10,
    C7_VC1_DEC                  = 0x11,
    C7_MP2_DEC                  = 0x12,
    C7_MP4_DEC                  = 0x13,
    C7_DV3_DEC                  = 0x13,
    C7_RVX_DEC                  = 0x14,
    C7_AVS_DEC                  = 0x15,
    C7_VP9_DEC                  = 0x16,
    C7_VP8_DEC                  = 0x17,
    C7_AVS2_DEC                 = 0x18,

    C7_AVC_ENC                  = 0x88, /* The original value is 0x18 */
    C7_MP4_ENC                  = 0x89, /* The original value is 0x19 */

    STD_UNKNOWN                 = 0xFF
};

// AUX_COD_STD
enum {
    MP4_AUX_MPEG4 = 0,
    MP4_AUX_DIVX3 = 1
};

enum {
    VPX_AUX_THO = 0,
    VPX_AUX_VP6 = 1,
    VPX_AUX_VP8 = 2,
    VPX_AUX_NUM
};

enum {
    AVC_AUX_AVC = 0,
    AVC_AUX_MVC = 1
};

// BIT_RUN command
enum {
    DEC_SEQ_INIT = 1,
    ENC_SEQ_INIT = 1,
    DEC_SEQ_END = 2,
    ENC_SEQ_END = 2,
    PIC_RUN = 3,
    SET_FRAME_BUF = 4,
    ENCODE_HEADER = 5,
    ENC_PARA_SET = 6,
    DEC_PARA_SET = 7,
    DEC_BUF_FLUSH = 8,
    RC_CHANGE_PARAMETER    = 9,
    VPU_SLEEP = 10,
    VPU_WAKE = 11,
    ENC_ROI_INIT = 12,
    FIRMWARE_GET = 0xf
};


enum {
    SRC_BUFFER_EMPTY        = 0,    //!< source buffer doesn't allocated.
    SRC_BUFFER_ALLOCATED    = 1,    //!< source buffer has been allocated.
    SRC_BUFFER_SRC_LOADED   = 2,    //!< source buffer has been allocated.
    SRC_BUFFER_USE_ENCODE   = 3     //!< source buffer was sent to VPU. but it was not used for encoding.
};
#define MAX(_a, _b)             (_a >= _b ? _a : _b)

#define HEVC_MAX_SUB_LAYER_ID   6

#define API_DEBUG
#ifdef API_DEBUG
#ifdef _MSC_VER
#define APIDPRINT(_fmt, ...)            printf(_fmt, __VA_ARGS__)
#else
#define APIDPRINT(_fmt, ...)            printf(_fmt, ##__VA_ARGS__)
#endif
#else
#define APIDPRINT(_fmt, ...)
#endif

/**
 * PRODUCT: CODA960/CODA980/WAVE320
 */
typedef struct {
    union {
        struct {
            int             useBitEnable;
            int             useIpEnable;
            int             useDbkYEnable;
            int             useDbkCEnable;
            int             useOvlEnable;
            int             useBtpEnable;
            int             useMeEnable;        // used only for coda7q encoder
            PhysicalAddress bufBitUse;
            PhysicalAddress bufIpAcDcUse;
            PhysicalAddress bufDbkYUse;
            PhysicalAddress bufDbkCUse;
            PhysicalAddress bufOvlUse;
            PhysicalAddress bufBtpUse;
            PhysicalAddress bufMeUse;           // used only for coda7q encoder
        } coda9;
        struct {
            int             useIpEnable;
            int             useSclEnable;
            int             useSclPackedModeEnable;
            int             useLfRowEnable;
            int             useBitEnable;
            PhysicalAddress bufIp;
            PhysicalAddress bufLfRow;
            PhysicalAddress bufBit;
            PhysicalAddress bufScaler;
            PhysicalAddress bufScalerPackedData;
            int             useEncImdEnable;
            int             useEncRdoEnable;
            int             useEncLfEnable;
            PhysicalAddress bufImd;
            PhysicalAddress bufRdo;
            PhysicalAddress bufLf;
        } wave4;
    } u;
    uint            bufSize;
    PhysicalAddress bufBase;
} SecAxiInfo;

typedef struct {
    DecOpenParam    openParam;
    DecInitialInfo  initialInfo;
    DecInitialInfo  newSeqInfo;     /* Temporal new sequence information */
    PhysicalAddress streamWrPtr;
    PhysicalAddress streamRdPtr;
    Int32           streamBufFull;
    int             streamEndflag;
    int             frameDisplayFlag;
    int             clearDisplayIndexes;
    int             setDisplayIndexes;
    PhysicalAddress streamRdPtrRegAddr;
    PhysicalAddress streamWrPtrRegAddr;
    PhysicalAddress streamBufStartAddr;
    PhysicalAddress streamBufEndAddr;
    PhysicalAddress frameDisplayFlagRegAddr;
    PhysicalAddress currentPC;
    PhysicalAddress busyFlagAddr;
    int             streamBufSize;
    FrameBuffer     frameBufPool[MAX_REG_FRAME];
    vpu_buffer_t    vbFrame;
    vpu_buffer_t    vbWTL;
    vpu_buffer_t    vbPPU;
    vpu_buffer_t    vbMV[MAX_REG_FRAME];
    vpu_buffer_t    vbFbcYTbl[MAX_REG_FRAME];
    vpu_buffer_t    vbFbcCTbl[MAX_REG_FRAME];
    int             chBwbFrameIdx;
    int             chFbcFrameIdx;
    BOOL            interResChange;
    int             frameAllocExt;
    int             ppuAllocExt;
    int             numFrameBuffers;
    int             numFbsForDecoding;                  /*!<< number of framebuffers used in decoding */
    int             numFbsForWTL;                       /*!<< number of linear framebuffer for displaying when DecInfo::wtlEnable is set to 1 */
    int             stride;
    int             frameBufferHeight;
    int             rotationEnable;
    int             mirrorEnable;
    int             deringEnable;
    MirrorDirection mirrorDirection;
    int             rotationAngle;
    FrameBuffer     rotatorOutput;
    int             rotatorStride;
    int             rotatorOutputValid;
    int             initialInfoObtained;
    int             vc1BframeDisplayValid;
    int             mapType;
    int             tiled2LinearEnable;
    int             tiled2LinearMode;
    int             wtlEnable;
    int             wtlMode;
    FrameBufferFormat   wtlFormat;                      /*!<< default value: FORMAT_420 8bit */
    SecAxiInfo          secAxiInfo;
    MaverickCacheConfig cacheConfig;
    int chunkSize;
    int seqInitEscape;

    // Report Information
    PhysicalAddress userDataBufAddr;
    vpu_buffer_t    vbUserData;

    BOOL            userDataEnable;                    /* User Data Enable Flag
                                                          CODA9xx: TRUE or FALSE
                                                          WAVE4xx: Refer to H265_USERDATA_FLAG_xxx values in vpuapi.h */
    int             userDataBufSize;
    int             userDataReportMode;                // User Data report mode (0 : interrupt mode, 1 interrupt disable mode)

    BOOL            cuDataEnable;                       //!<< Enable reporting CU data
    int32_t         cuDataBufSize;                      //!<< Size of CU buffer in bytes
    PhysicalAddress cuDataBufAddr;                      //!<< Physical address


    LowDelayInfo    lowDelayInfo;
    int             frameStartPos;
    int             frameEndPos;
    int             frameDelay;
    vpu_buffer_t    vbSlice;            // AVC, VP8 only
    vpu_buffer_t    vbWork;
    vpu_buffer_t    vbTemp;
    vpu_buffer_t    vbReport;
    int             workBufferAllocExt;
    DecOutputInfo   decOutInfo[MAX_GDI_IDX];
    TiledMapConfig  mapCfg;
    int             reorderEnable;
    Uint32          avcErrorConcealMode;
    DRAMConfig      dramCfg;            //coda960 only
    int             thumbnailMode;
    int             seqChangeMask;      // WAVE410
    Uint32          prevFrameEndPos;      //!<< WAVE410v2: end position of previous frame
    BOOL            scalerEnable;
    BOOL            scalerSameSize;
    Uint32          scaleWidth;
    Uint32          scaleHeight;
    Int32           targetSubLayerId;       //!<< H.265 temporal scalability


    Uint32          div3Width;      // for CODA7Q
    Uint32          div3Height;     // for CODA7Q
    int             rdPtrValidFlag;
} DecInfo;

typedef struct {
    EncOpenParam        openParam;
    EncInitialInfo      initialInfo;
    PhysicalAddress     streamRdPtr;
    PhysicalAddress     streamWrPtr;
    int                 streamEndflag;
    PhysicalAddress     streamRdPtrRegAddr;
    PhysicalAddress     streamWrPtrRegAddr;
    PhysicalAddress     streamBufStartAddr;
    PhysicalAddress     streamBufEndAddr;
    PhysicalAddress     currentPC;
    PhysicalAddress     busyFlagAddr;
    int                 streamBufSize;
    int                 linear2TiledEnable;
    int                 linear2TiledMode;    // coda980 only
    int                 mapType;
    int                 userMapEnable;
    FrameBuffer         frameBufPool[MAX_REG_FRAME];
    vpu_buffer_t        vbFrame;
    vpu_buffer_t        vbPPU;
    int                 frameAllocExt;
    int                 ppuAllocExt;
    vpu_buffer_t        vbSubSampFrame;         /*!<< CODA960 only */
    vpu_buffer_t        vbMvcSubSampFrame;      /*!<< CODA960 only */
    int                 numFrameBuffers;
    int                 stride;
    int                 frameBufferHeight;
    int                 rotationEnable;
    int                 mirrorEnable;
    MirrorDirection     mirrorDirection;
    int                 rotationAngle;
    int                 initialInfoObtained;
    int                 ringBufferEnable;
    SecAxiInfo          secAxiInfo;
    MaverickCacheConfig cacheConfig;

    int                 sliceIntEnable;       /*!<< WAVE420 only */


    int                 ActivePPSIdx;           /*!<< CODA980 */
    int                 frameIdx;               /*!<< CODA980 */
    int                 fieldDone;              /*!<< CODA980 */
    int                 lineBufIntEn;
    vpu_buffer_t        vbWork;
    vpu_buffer_t        vbScratch;

#if defined(WAVE420SN) || defined(WAVE420DU)
    vpu_buffer_t        vbTemp[2];                 //!< Temp buffer (WAVE420)
    uint32_t            vbTempQueueIndex;
#else
    vpu_buffer_t        vbTemp;                 //!< Temp buffer (WAVE420)
#endif
    vpu_buffer_t        vbMV;                   //!< colMV buffer (WAVE420)
    vpu_buffer_t        vbFbcYTbl;              //!< FBC Luma table buffer (WAVE420)
    vpu_buffer_t        vbFbcCTbl;              //!< FBC Chroma table buffer (WAVE420)
    vpu_buffer_t        vbSubSamBuf;            //!< Sub-sampled buffer for ME (WAVE420)

    TiledMapConfig      mapCfg;
    DRAMConfig          dramCfg;                /*!<< CODA960 */


    Uint32 prefixSeiNalEnable;
    Uint32 prefixSeiDataSize;
    Uint32 prefixSeiDataEncOrder;
    PhysicalAddress prefixSeiNalAddr;
    Uint32 suffixSeiNalEnable;
    Uint32 suffixSeiDataSize;
    Uint32 suffixSeiDataEncOrder;
    PhysicalAddress suffixSeiNalAddr;

    Int32   errorReasonCode;
    Uint64          curPTS;             /**! Current timestamp in 90KHz */
    Uint64          ptsMap[32];         /**! PTS mapped with source frame index */
} EncInfo;



typedef struct CodecInst {
    Int32   inUse;
    Int32   instIndex;
    Int32   coreIdx;
    Int32   codecMode;
    Int32   codecModeAux;
    Int32   productId;
    Int32   loggingEnable;
    Uint32  isDecoder;
    union {
        EncInfo encInfo;
        DecInfo decInfo;
    }* CodecInfo;
} CodecInst;


/*******************************************************************************
 * H.265 USER DATA(VUI and SEI)                                           *
 *******************************************************************************/
#define H265_MAX_DPB_SIZE               17
#define H265_MAX_NUM_SUB_LAYER          8
#define H265_MAX_NUM_ST_RPS             64
#define H265_MAX_CPB_CNT                32
#define H265_MAX_NUM_VERTICAL_FILTERS   5
#define H265_MAX_NUM_HORIZONTAL_FILTERS 3
#define H265_MAX_TAP_LENGTH             32
#define H265_MAX_NUM_KNEE_POINT         999
#define H265_USERDATA_BUFFER_SIZE       (10*1024)

typedef struct
{
    uint32_t offset;
    uint32_t size;
} user_data_entry_t;

typedef struct
{
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
} win_t;


typedef struct
{
    uint8_t  nal_hrd_param_present_flag;
    uint8_t  vcl_hrd_param_present_flag;
    uint8_t  sub_pic_hrd_params_present_flag;
    uint8_t  tick_divisor_minus2;
    int8_t   du_cpb_removal_delay_inc_length_minus1;
    int8_t   sub_pic_cpb_params_in_pic_timing_sei_flag;
    int8_t   dpb_output_delay_du_length_minus1;
    int8_t   bit_rate_scale;
    int8_t   cpb_size_scale;
    int8_t   initial_cpb_removal_delay_length_minus1;
    int8_t   cpb_removal_delay_length_minus1;
    int8_t   dpb_output_delay_length_minus1;

    uint8_t  fixed_pic_rate_gen_flag[H265_MAX_NUM_SUB_LAYER];
    uint8_t  fixed_pic_rate_within_cvs_flag[H265_MAX_NUM_SUB_LAYER];
    uint8_t  low_delay_hrd_flag[H265_MAX_NUM_SUB_LAYER];
    int8_t   cpb_cnt_minus1[H265_MAX_NUM_SUB_LAYER];
    int16_t  elemental_duration_in_tc_minus1[H265_MAX_NUM_SUB_LAYER];

    uint32_t nal_bit_rate_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    uint32_t nal_cpb_size_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    uint32_t nal_cpb_size_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    uint32_t nal_bit_rate_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    uint8_t  nal_cbr_flag[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];

    uint32_t vcl_bit_rate_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    uint32_t vcl_cpb_size_value_minus1[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
    uint32_t vcl_cpb_size_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    uint32_t vcl_bit_rate_du_value_minus1[H265_MAX_NUM_SUB_LAYER];
    uint8_t  vcl_cbr_flag[H265_MAX_NUM_SUB_LAYER][H265_MAX_CPB_CNT];
} h265_hrd_param_t;

typedef struct
{
    uint8_t  aspect_ratio_info_present_flag;
    uint8_t  aspect_ratio_idc;
    uint8_t  overscan_info_present_flag;
    uint8_t  overscan_appropriate_flag;

    uint8_t  video_signal_type_present_flag;
    int8_t   video_format;

    uint8_t  video_full_range_flag;
    uint8_t  colour_description_present_flag;

    uint16_t sar_width;
    uint16_t sar_height;

    uint8_t colour_primaries;
    uint8_t transfer_characteristics;
    uint8_t matrix_coefficients;

    uint8_t chroma_loc_info_present_flag;
    int8_t  chroma_sample_loc_type_top_field;
    int8_t  chroma_sample_loc_type_bottom_field;

    uint8_t neutral_chroma_indication_flag;

    uint8_t field_seq_flag;

    uint8_t frame_field_info_present_flag;
    uint8_t default_display_window_flag;
    uint8_t vui_timing_info_present_flag;
    uint8_t vui_poc_proportional_to_timing_flag;

    uint32_t vui_num_units_in_tick;
    uint32_t vui_time_scale;

    uint8_t  vui_hrd_parameters_present_flag;
    uint8_t  bitstream_restriction_flag;

    uint8_t  tiles_fixed_structure_flag;
    uint8_t  motion_vectors_over_pic_boundaries_flag;
    uint8_t  restricted_ref_pic_lists_flag;
    int8_t   min_spatial_segmentation_idc;
    int8_t   max_bytes_per_pic_denom;
    int8_t   max_bits_per_mincu_denom;

    int16_t  vui_num_ticks_poc_diff_one_minus1;
    int8_t   log2_max_mv_length_horizontal;
    int8_t   log2_max_mv_length_vertical;

    win_t    def_disp_win;
    h265_hrd_param_t hrd_param;
} h265_vui_param_t;

typedef struct
{
    uint32_t display_primaries_x[3];
    uint32_t display_primaries_y[3];
    uint32_t white_point_x                   : 16;
    uint32_t white_point_y                   : 16;
    uint32_t max_display_mastering_luminance : 32;
    uint32_t min_display_mastering_luminance : 32;
} h265_mastering_display_colour_volume_t;

typedef struct
{
    uint32_t ver_chroma_filter_idc               : 8;
    uint32_t hor_chroma_filter_idc               : 8;
    uint32_t ver_filtering_field_processing_flag : 1;
    uint32_t target_format_idc                   : 2;
    uint32_t num_vertical_filters                : 3;
    uint32_t num_horizontal_filters              : 3;
    uint8_t  ver_tap_length_minus1[ H265_MAX_NUM_VERTICAL_FILTERS ];
    uint8_t  hor_tap_length_minus1[ H265_MAX_NUM_HORIZONTAL_FILTERS ];
    int32_t  ver_filter_coeff[ H265_MAX_NUM_VERTICAL_FILTERS ][ H265_MAX_TAP_LENGTH ];
    int32_t  hor_filter_coeff[ H265_MAX_NUM_HORIZONTAL_FILTERS ][ H265_MAX_TAP_LENGTH ];
} h265_chroma_resampling_filter_hint_t;

typedef struct
{
    uint32_t knee_function_id;
    uint8_t  knee_function_cancel_flag;

    uint8_t  knee_function_persistence_flag;
    uint32_t input_disp_luminance;
    uint32_t input_d_range;
    uint32_t output_d_range;
    uint32_t output_disp_luminance;
    uint16_t num_knee_points_minus1;
    uint16_t input_knee_point[ H265_MAX_NUM_KNEE_POINT ];
    uint16_t output_knee_point[ H265_MAX_NUM_KNEE_POINT ];
} h265_knee_function_info_t;

typedef struct
{
    uint32_t max_content_light_level;
    uint32_t max_pic_average_light_level;
} content_light_level_info_t;

typedef struct
{
    int8_t status;           // 0 : empty, 1 : occupied
    int8_t pic_struct;
    int8_t source_scan_type;
    int8_t duplicate_flag;
} h265_sei_pic_timing_t;

typedef enum
{
    VP9_NONE,
    VP9_SAMESCALE, //(1 << 0)
    VP9_DOWNSCALE, //(1 << 1)
} VP9ScaleBit;

#ifdef __cplusplus
extern "C" {
#endif


RetCode InitCodecInstancePool(Uint32 coreIdx);
RetCode GetCodecInstance(Uint32 coreIdx, CodecInst ** ppInst);
void    FreeCodecInstance(CodecInst * pCodecInst);

RetCode CheckDecOpenParam(DecOpenParam * pop);
int     DecBitstreamBufEmpty(DecInfo * pDecInfo);
RetCode SetParaSet(DecHandle handle, int paraSetType, DecParamSet * para);
void    DecSetHostParaAddr(Uint32 coreIdx, PhysicalAddress baseAddr, PhysicalAddress paraAddr);


Uint32 ConfigSecAXICoda9(
    Uint32      coreIdx,
    Int32       codecMode,
    SecAxiInfo* sa,
    Uint32      width,
    Uint32      height,
    Uint32      profile
    );

Uint32 ConfigSecAXIWave(
    Uint32      coreIdx,
    CodStd      codStd,
    SecAxiInfo* sa,
    Uint32      width,
    Uint32      height,
    Uint32      profile,
    Uint32      level
    );

Uint32 ConfigSecAXICoda7(
    Uint32      coreIdx,
    CodStd      codStd,
    SecAxiInfo* sa,
    Uint32      width,
    Uint32      height,
    Uint32      profile
    );

RetCode AllocateLinearFrameBuffer(
    TiledMapType            mapType,
    FrameBuffer*            fbArr,
    uint32_t                numOfFrameBuffers,
    uint32_t                sizeLuma,
    uint32_t                sizeChroma
    );

RetCode AllocateTiledFrameBufferGdiV1(
    TiledMapType            mapType,
    PhysicalAddress         tiledBaseAddr,
    FrameBuffer*            fbArr,
    uint32_t                numOfFrameBuffers,
    uint32_t                sizeLuma,
    uint32_t                sizeChroma,
    DRAMConfig*             pDramCfg
    );

RetCode AllocateTiledFrameBufferGdiV2(
    TiledMapType            mapType,
    FrameBuffer*            fbArr,
    uint32_t                numOfFrameBuffers,
    uint32_t                sizeLuma,
    uint32_t                sizeChroma
    );


RetCode CheckEncInstanceValidity(EncHandle handle);
RetCode CheckEncOpenParam(EncOpenParam * pop);
RetCode CheckEncParam(EncHandle handle, EncParam * param);
RetCode GetEncHeader(EncHandle handle, EncHeaderParam * encHeaderParam);
RetCode EncParaSet(EncHandle handle, int paraSetType);
RetCode SetGopNumber(EncHandle handle, Uint32 *gopNumber);
RetCode SetIntraQp(EncHandle handle, Uint32 *intraQp);
RetCode SetBitrate(EncHandle handle, Uint32 *bitrate);
RetCode SetFramerate(EncHandle handle, Uint32 *frameRate);
RetCode SetIntraRefreshNum(EncHandle handle, Uint32 *pIntraRefreshNum);
RetCode SetSliceMode(EncHandle handle, EncSliceMode *pSliceMode);
RetCode SetHecMode(EncHandle handle, int mode);
void    EncSetHostParaAddr(Uint32 coreIdx, PhysicalAddress baseAddr, PhysicalAddress paraAddr);

RetCode EnterLock(Uint32 coreIdx);
RetCode LeaveLock(Uint32 coreIdx);
RetCode SetClockGate(Uint32 coreIdx, Uint32 on);

RetCode EnterDispFlagLock(Uint32 coreIdx);
RetCode LeaveDispFlagLock(Uint32 coreIdx);

void SetPendingInst(Uint32 coreIdx, CodecInst *inst);
void ClearPendingInst(Uint32 coreIdx);
CodecInst *GetPendingInst(Uint32 coreIdx);
int GetPendingInstIdx(Uint32 coreIdx);

Int32 MaverickCache2Config(
    MaverickCacheConfig* pCache,
    BOOL                decoder ,
    BOOL                interleave,
    Uint32              bypass,
    Uint32              burst,
    Uint32              merge,
    TiledMapType        mapType,
    Uint32              wayshape
    );

int SetTiledMapType(Uint32 coreIdx, TiledMapConfig *pMapCfg, int mapType, int stride, int interleave, DRAMConfig *dramCfg);
int GetXY2AXIAddr(TiledMapConfig *pMapCfg, int ycbcr, int posY, int posX, int stride, FrameBuffer *fb);
//for GDI 1.0
void SetTiledFrameBase(Uint32 coreIdx, PhysicalAddress baseAddr);
PhysicalAddress GetTiledFrameBase(Uint32 coreIdx, FrameBuffer *frame, int num);

/**
 * \brief   It returns the stride of framebuffer in byte.
 *
 * \param   width           picture width in pixel.
 * \param   format          YUV format. see FrameBufferFormat structure in vpuapi.h
 * \param   cbcrInterleave
 * \param   mapType         map type. see TiledMapType in vpuapi.h
 */
Int32 CalcStride(
    Uint32              width,
    Uint32              height,
    FrameBufferFormat   format,
    BOOL                cbcrInterleave,
    TiledMapType        mapType,
    VP9ScaleBit         vp9Scale
    );

Int32 CalcLumaSize(
    Int32               productId,
    Int32               stride,
    Int32               height,
    FrameBufferFormat   format,
    BOOL                cbcrIntl,
    TiledMapType        mapType,
    DRAMConfig*         pDramCfg
    );

Int32 CalcChromaSize(
    Int32               productId,
    Int32               stride,
    Int32               height,
    FrameBufferFormat   format,
    BOOL                cbcrIntl,
    TiledMapType        mapType,
    DRAMConfig*         pDramCfg
    );

int LevelCalculation(int MbNumX, int MbNumY, int frameRateInfo, int interlaceFlag, int BitRate, int SliceNum);

/* timescale: 1/90000 */
Uint64 GetTimestamp(
    EncHandle handle
    );
#ifdef __cplusplus
}
#endif

#endif // endif VPUAPI_UTIL_H_INCLUDED

