//------------------------------------------------------------------------------
// File: main.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
// Modify by semidrive 2020/7/20
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include "main_helper.h"
#include "vdi.h"
#ifdef PLATFORM_LINUX
#include <pthread.h>
#include <unistd.h>
#endif

#define STREAM_BUF_SIZE           0xA00000    /* max bitstream size */
#define STREAM_BUF_SIZE_VP9       0x1400000   /* max bitstream size */

#define DEC_COD_STD(x)         (x < 14)
#define ENC_COD_STD(x)         (x >= 14)
#define CONVERT_ENC_COD_STD(x) (CodStd)(x - 14)

extern int TestEncoder(
    TestEncConfig *param
);

extern BOOL TestDecoderWave(
    TestDecConfig *param
);

extern BOOL TestDecoderCoda(
    TestDecConfig *param
);

/**
 *Enumeration for declaring codec type variables
 **/
typedef enum {
    MULTI_STD_AVC,
    MULTI_STD_VC1,
    MULTI_STD_MPEG2,
    MULTI_STD_MPEG4,
    MULTI_STD_H263,
    MULTI_STD_DIV3,
    MULTI_STD_RV,
    MULTI_STD_AVS,
    MULTI_STD_THO = 9,
    MULTI_STD_VP3,
    MULTI_STD_VP8,
    MULTI_STD_HEVC,
    MULTI_STD_VP9,
    MULTI_STD_AVC_ENC  = 14,
    MULTI_STD_MP4_ENC  = 17,
    MULTI_STD_H263_ENC = 18,
    MULTI_STD_HEVC_ENC = 26
} MultiCodStd;

/**
 *Brief Description of intance config info
 *param inputFilePath  input file  encode--encoder.cfg
 *param outputFilePath output file
 *param stdMode bitstream format
 *param bsmode pic_end/interrupt mode
 *param enableWTL write to line
 *param TestDecConfig decoding config info
 *param encConfig encoding config info
 *param numFrames num of decoding
 *param sramCfg sram config info
 *param loopCount loop test in decoding thread
 */
typedef struct {
    char             inputFilePath[256];
    char             outputFilePath[256];
    char             refFilePath[256];
    MultiCodStd      stdMode;
    BOOL             afbce;
    BOOL             afbcd;
    BOOL             scaler;
    size_t           sclw;
    size_t           sclh;
    int32_t          bsmode;
    BOOL             enableWTL;
    BOOL             enableMVC;
    int              compareType;
    TestDecConfig    decConfig;
    Uint32           wtlformat;
    TestEncConfig    encConfig;
    Uint32           pvricMode;
    Uint32           numFrames;
    int32_t          sramCfg;
    int32_t          loopCount;
    int32_t          nv21;
    int32_t          cbcrinterLeave;
} InstTestConfig;

/**
 *Brief Description of Mult instance info
 *param numMulti num of instance
 *param testNum  test loop times
 *param showVersion
 *param instConfig  every instance config info
 */
typedef struct {
    int               numMulti;
    int               showVersion;
    int               testNum;
    BOOL              isAutoTest;
    InstTestConfig    instConfig[MAX_NUM_INSTANCE];
} TestMultiConfig;


TestMultiConfig multiConfig = {0};

/*
 * Decoder entry
 */
static BOOL TestDecoder(
    TestDecConfig *param
)
{
    if (!param)
        return false;

    if ( param->bitFormat == MULTI_STD_VP9 ||
            param->bitFormat == MULTI_STD_HEVC ) {
        VLOG(INFO, "%s : %d wave decode starting \n", __FUNCTION__, __LINE__);
        TestDecoderWave(param);
    }
    else {
        VLOG(INFO, "%s : %d code decode starting \n", __FUNCTION__, __LINE__);
        TestDecoderCoda(param);
    }

    return true;
}


/**
 *Brief set encoding instance param
 *param multiConfig multi instance param info
 *return void
 *note:
 */
static void SetEncMultiParam(
    TestMultiConfig *multiConfig,
    int idx
)
{
    multiConfig->instConfig[idx].encConfig.stdMode       = CONVERT_ENC_COD_STD(multiConfig->instConfig[idx].stdMode);
    multiConfig->instConfig[idx].encConfig.frame_endian  = VDI_LITTLE_ENDIAN;
    multiConfig->instConfig[idx].encConfig.stream_endian = VDI_LITTLE_ENDIAN;
    multiConfig->instConfig[idx].encConfig.yuv_mode      = YUV_MODE_YUV_LOADER;
    multiConfig->instConfig[idx].encConfig.mapType       = LINEAR_FRAME_MAP;
    multiConfig->instConfig[idx].encConfig.ringBufferEnable = FALSE;
    multiConfig->instConfig[idx].encConfig.compare_type = (TRUE == (BOOL)multiConfig->instConfig[idx].compareType) ? STREAM_COMPARE : NO_COMPARE;
    multiConfig->instConfig[idx].encConfig.sramMode = multiConfig->instConfig[idx].sramCfg;

    sprintf(multiConfig->instConfig[idx].encConfig.yuvSourceBaseDir, "./");
    strcpy(multiConfig->instConfig[idx].encConfig.cfgFileName, multiConfig->instConfig[idx].inputFilePath);
    strcpy(multiConfig->instConfig[idx].encConfig.bitstreamFileName, multiConfig->instConfig[idx].outputFilePath);
    strcpy(multiConfig->instConfig[idx].encConfig.ref_stream_path, multiConfig->instConfig[idx].refFilePath);

    if (multiConfig->instConfig[idx].encConfig.stdMode != STD_AVC
            && multiConfig->instConfig[idx].encConfig.stdMode != STD_MPEG4
            && multiConfig->instConfig[idx].encConfig.stdMode != STD_H263) {
        VLOG(ERR, "Encoder format not supported!!!\n");
        return;
    }

    if (multiConfig->instConfig[idx].encConfig.cfgFileName[0] == '\0')
        strcpy(multiConfig->instConfig[idx].encConfig.cfgFileName, "encoder.cfg");

    if (multiConfig->instConfig[idx].encConfig.bitstreamFileName[0] == '\0')
        sprintf(multiConfig->instConfig[idx].encConfig.bitstreamFileName, "output.bin");

    if (multiConfig->instConfig[idx].encConfig.mapType == TILED_FRAME_MB_RASTER_MAP
            || multiConfig->instConfig[idx].encConfig.mapType == TILED_FIELD_MB_RASTER_MAP) {
        multiConfig->instConfig[idx].encConfig.cbcrInterleave = TRUE;
    }

    if (multiConfig->instConfig[idx].encConfig.rotAngle > 0 || multiConfig->instConfig[idx].encConfig.mirDir > 0) {
        multiConfig->instConfig[idx].encConfig.useRot = TRUE;
    }

    if (multiConfig->instConfig[idx].sramCfg) {
        /* config linebuffer 144k sram */
        multiConfig->instConfig[idx].encConfig.sramMode = MODE_SRAM_LINEBUFFER;
    }
}

/**
 *Brief set decoding instance param
 *param multiConfig  multi instance param info
 *param idx instance index
 *return void
 *note:
 */
static void SetDecMultiParam(
    TestMultiConfig *multiConfig,
    int idx
)
{
    /* copy dynamic config for instance */
    if (BS_MODE_PIC_END == multiConfig->instConfig[idx].bsmode) {
        multiConfig->instConfig[idx].decConfig.bitstreamMode = BS_MODE_PIC_END;
        multiConfig->instConfig[idx].decConfig.feedingMode = FEEDING_METHOD_FRAME_SIZE;
    }

    multiConfig->instConfig[idx].decConfig.bitFormat            = multiConfig->instConfig[idx].stdMode;
    multiConfig->instConfig[idx].decConfig.frameEndian          = 0; //VPU_FRAME_ENDIAN;
    multiConfig->instConfig[idx].decConfig.cbcrInterleave       = FALSE;
    multiConfig->instConfig[idx].decConfig.nv21                 = FALSE;
    multiConfig->instConfig[idx].decConfig.enableWTL            = multiConfig->instConfig[idx].enableWTL;
    multiConfig->instConfig[idx].decConfig.coda9.enableMvc      = multiConfig->instConfig[idx].enableMVC;
    multiConfig->instConfig[idx].decConfig.wtlMode              = FF_FRAME;
    multiConfig->instConfig[idx].decConfig.wtlFormat            = multiConfig->instConfig[idx].wtlformat;
    multiConfig->instConfig[idx].decConfig.mapType              = LINEAR_FRAME_MAP;
    multiConfig->instConfig[idx].decConfig.bsSize               = STREAM_BUF_SIZE;
    multiConfig->instConfig[idx].decConfig.wave4.bwOptimization = TRUE;
    multiConfig->instConfig[idx].decConfig.scaleDownWidth   = multiConfig->instConfig[idx].sclw;
    multiConfig->instConfig[idx].decConfig.scaleDownHeight  = multiConfig->instConfig[idx].sclh;
    multiConfig->instConfig[idx].decConfig.loopCount = multiConfig->instConfig[idx].loopCount;

    if (multiConfig->instConfig[idx].numFrames)
        multiConfig->instConfig[idx].decConfig.forceOutNum       = multiConfig->instConfig[idx].numFrames;
    else
        multiConfig->instConfig[idx].decConfig.forceOutNum      = 10;

    if (multiConfig->instConfig[idx].pvricMode != 0) {
        multiConfig->instConfig[idx].decConfig.pvricFbcEnable = TRUE;
        multiConfig->instConfig[idx].decConfig.pvricPaddingY  = 0;
        multiConfig->instConfig[idx].decConfig.pvricPaddingC  = 0;
        multiConfig->instConfig[idx].decConfig.cbcrInterleave = TRUE;
        multiConfig->instConfig[idx].decConfig.nv21           = FALSE;

        if (multiConfig->instConfig[idx].pvricMode == 2) {
            multiConfig->instConfig[idx].decConfig.pvric31HwSupport = TRUE;
        }
    }

    strcpy(multiConfig->instConfig[idx].decConfig.inputPath, multiConfig->instConfig[idx].inputFilePath);
    strcpy(multiConfig->instConfig[idx].decConfig.outputPath, multiConfig->instConfig[idx].outputFilePath);

    if (multiConfig->instConfig[idx].compareType == MD5_COMPARE) {
        multiConfig->instConfig[idx].decConfig.compareType = MD5_COMPARE;
        strcpy(multiConfig->instConfig[idx].decConfig.md5Path, multiConfig->instConfig[idx].refFilePath);
    }
    else if (multiConfig->instConfig[idx].compareType == YUV_COMPARE) {
        multiConfig->instConfig[idx].decConfig.compareType = YUV_COMPARE;
        strcpy(multiConfig->instConfig[idx].decConfig.refYuvPath, multiConfig->instConfig[idx].refFilePath);
    }
    else {
        multiConfig->instConfig[idx].decConfig.compareType = NO_COMPARE;
    }

    /* exclusive confige of decode */
    if (multiConfig->instConfig[idx].decConfig.bitFormat != MULTI_STD_HEVC && multiConfig->instConfig[idx].decConfig.bitFormat != MULTI_STD_VP9) {
        multiConfig->instConfig[idx].decConfig.coreIdx = 0;   /* coda coreIdx */
        multiConfig->instConfig[idx].decConfig.coda9.enableBWB          = VPU_ENABLE_BWB;
        multiConfig->instConfig[idx].decConfig.coda9.frameCacheBypass   = 0;
        multiConfig->instConfig[idx].decConfig.coda9.frameCacheBurst    = 0;
        multiConfig->instConfig[idx].decConfig.coda9.frameCacheMerge    = 3;
        multiConfig->instConfig[idx].decConfig.coda9.frameCacheWayShape = 15;

        /* only config for deocde */
        if (multiConfig->instConfig[idx].sramCfg == 1) {
            multiConfig->instConfig[idx].decConfig.secondaryAXI = 0xffff; /* enable all AXI*/
            multiConfig->instConfig[idx].decConfig.sramMode = MODE_SRAM_AXI_EXCLUSIVE; /* 168K for coda AXI*/
        }

        if (multiConfig->instConfig[idx].decConfig.bitstreamMode == BS_MODE_INTERRUPT)
            multiConfig->instConfig[idx].decConfig.feedingMode = FEEDING_METHOD_FIXED_SIZE;
    }
    else if (multiConfig->instConfig[idx].decConfig.bitFormat == MULTI_STD_HEVC) {
        multiConfig->instConfig[idx].decConfig.coreIdx = 1; /* wave coreIdx */
        multiConfig->instConfig[idx].decConfig.mapType = COMPRESSED_FRAME_MAP;
        multiConfig->instConfig[idx].decConfig.bsSize = STREAM_BUF_SIZE;
        multiConfig->instConfig[idx].decConfig.streamEndian = VDI_128BIT_LITTLE_ENDIAN;
        multiConfig->instConfig[idx].decConfig.frameEndian = VDI_128BIT_LITTLE_ENDIAN;
        /* normal: 0x0C - Best prediction: 0x00 - Basic prediction: 0x3C */
        multiConfig->instConfig[idx].decConfig.wave4.fbcMode = 0x0C;
        multiConfig->instConfig[idx].decConfig.wave4.bwOptimization = FALSE;
        multiConfig->instConfig[idx].decConfig.enableWTL = TRUE;

        if (multiConfig->instConfig[idx].sramCfg == 1) {
            multiConfig->instConfig[idx].decConfig.secondaryAXI = 0xffff; /* enable all AXI*/
            multiConfig->instConfig[idx].decConfig.sramMode = MODE_SRAM_AXI_EXCLUSIVE; /* soc sram2 for wave AXI*/
        }

        if (multiConfig->instConfig[idx].decConfig.bitstreamMode == BS_MODE_INTERRUPT)
            multiConfig->instConfig[idx].decConfig.feedingMode = FEEDING_METHOD_FIXED_SIZE;
    }
    else if (multiConfig->instConfig[idx].decConfig.bitFormat == MULTI_STD_VP9) {
        multiConfig->instConfig[idx].decConfig.coreIdx = 1; /*wave coreIdx*/
        multiConfig->instConfig[idx].decConfig.mapType = COMPRESSED_FRAME_MAP;
        multiConfig->instConfig[idx].decConfig.bitstreamMode = BS_MODE_PIC_END;
        multiConfig->instConfig[idx].decConfig.streamEndian = VDI_128BIT_LITTLE_ENDIAN;
        multiConfig->instConfig[idx].decConfig.frameEndian = VDI_128BIT_LITTLE_ENDIAN;
        multiConfig->instConfig[idx].decConfig.bsSize = STREAM_BUF_SIZE_VP9;
        multiConfig->instConfig[idx].decConfig.feedingMode = FEEDING_METHOD_FRAME_SIZE;
        /*normal: 0x0C - Best prediction: 0x00 - Basic prediction:0x3C */
        multiConfig->instConfig[idx].decConfig.wave4.fbcMode = 0x0C;
        multiConfig->instConfig[idx].decConfig.wave4.bwOptimization = FALSE;
        multiConfig->instConfig[idx].decConfig.enableWTL = TRUE;

        if (multiConfig->instConfig[idx].sramCfg == 1) {
            multiConfig->instConfig[idx].decConfig.secondaryAXI = 0xffff; /* enable all AXI*/
            multiConfig->instConfig[idx].decConfig.sramMode = MODE_SRAM_AXI_EXCLUSIVE; /*   soc sram2 for wave AXI*/
        }

    }
}

/**
 *Brief mult instance process
 *param multiConfig  multi instance param info
 *return 0 success   != 0 failure
 *note:
 */
static int MultiInstanceTest(
    TestMultiConfig *multiConfig
)
{
    int32_t i = 0;
    int32_t result = 0;
    TestDecConfig *decConfig = NULL;
    TestEncConfig *encConfig = NULL;
    pthread_t thread_id[MAX_NUM_INSTANCE];
    void *ret[MAX_NUM_INSTANCE] = {NULL};

    for (i = 0; i < multiConfig->numMulti; i++) {
        /* encode instance */
        if (ENC_COD_STD(multiConfig->instConfig[i].stdMode)) {
            encConfig = &multiConfig->instConfig[i].encConfig;
            encConfig->instNum = i;
            pthread_create(&thread_id[i], NULL, (void *)TestEncoder, encConfig);
        }

        /* decode instance */
        if (DEC_COD_STD(multiConfig->instConfig[i].stdMode)) {
            decConfig = &multiConfig->instConfig[i].decConfig;
            decConfig->instIdx = i;
            pthread_create(&thread_id[i], NULL, (void *)TestDecoder, decConfig);
        }

        /* waiting for first instance to initialize vdi */
        usleep(3000 * 1000);
    }

    for (i = 0; i < multiConfig->numMulti; i++) {
        pthread_join(thread_id[i], &ret[i]);
    }

    for (i = 0; i < multiConfig->numMulti; i++) {
        VLOG(INFO, "thread return = %p\n", ret[i]);

        if ((intptr_t)ret[i] != 1) { /* success = 1 */
            result = 1;
        }
    }

    return result;
}


/**
 *Brief print every instance param info
 *param multiConfig multi instance param info
 *return void
 *note:
 */
static void  PrintMultiParam(
    TestMultiConfig *multiConfig
)
{
    int32_t i = 0;
    VLOG(INFO, "Start Multi Instance Test num = %d, Instance num %d\n",
         multiConfig->testNum,
         multiConfig->numMulti);

    for (i = 0; i < multiConfig->numMulti; i++) {
        if (DEC_COD_STD(multiConfig->instConfig[i].stdMode)) {
            VLOG(INFO, "\n==================Instance %d decode Param=============\n", i);
            VLOG(INFO, "coreIdx         %d\n", multiConfig->instConfig[i].decConfig.coreIdx);
            VLOG(INFO, "forceOutNum     %d\n", multiConfig->instConfig[i].decConfig.forceOutNum);
            VLOG(INFO, "bitFormat       %d\n", multiConfig->instConfig[i].decConfig.bitFormat);
            VLOG(INFO, "mapType         %d\n", multiConfig->instConfig[i].decConfig.mapType);
            VLOG(INFO, "enableWTL       %d\n", multiConfig->instConfig[i].decConfig.enableWTL);
            VLOG(INFO, "wtlMode         %d\n", multiConfig->instConfig[i].decConfig.wtlMode);
            VLOG(INFO, "cbcrInterleave  %d\n", multiConfig->instConfig[i].decConfig.cbcrInterleave);
            VLOG(INFO, "nv21            %d\n", multiConfig->instConfig[i].decConfig.nv21);
            VLOG(INFO, "pvricFbcEnable  %d\n", multiConfig->instConfig[i].decConfig.pvricFbcEnable);
            VLOG(INFO, "streamEndian    %d\n", multiConfig->instConfig[i].decConfig.streamEndian);
            VLOG(INFO, "frameEndian     %d\n", multiConfig->instConfig[i].decConfig.frameEndian);
            VLOG(INFO, "scaleDownWidth  %d\n", multiConfig->instConfig[i].decConfig.scaleDownWidth);
            VLOG(INFO, "scaleDownHeight %d\n", multiConfig->instConfig[i].decConfig.scaleDownHeight);
            VLOG(INFO, "wtlFormat       %d\n", multiConfig->instConfig[i].decConfig.wtlFormat);
            VLOG(INFO, "bsmode          %d\n", multiConfig->instConfig[i].decConfig.bitstreamMode);
            VLOG(INFO, "feedingMode     %d\n", multiConfig->instConfig[i].decConfig.feedingMode);
            VLOG(INFO, "invalidDisFlag  %d\n", multiConfig->instConfig[i].decConfig.invalidDisFlag);
            VLOG(INFO, "seekflag        %d\n", multiConfig->instConfig[i].decConfig.seekflag);
            VLOG(INFO, "input file      %s\n", multiConfig->instConfig[i].decConfig.inputPath);
            VLOG(INFO, "outpu file      %s\n", multiConfig->instConfig[i].decConfig.outputPath);
            VLOG(INFO, "scw             %d\n", multiConfig->instConfig[i].decConfig.scaleDownWidth);
            VLOG(INFO, "sch             %d\n", multiConfig->instConfig[i].decConfig.scaleDownHeight);
            VLOG(INFO, "bitstreamBufferSize] %d\n", multiConfig->instConfig[i].decConfig.bsSize);
            VLOG(INFO, "BWOPT           %d\n", multiConfig->instConfig[i].decConfig.wave4.bwOptimization);
            VLOG(INFO, "PVRIC           %d\n", multiConfig->instConfig[i].decConfig.pvricFbcEnable);
            VLOG(INFO, "PVRIC31HW       %d\n", multiConfig->instConfig[i].decConfig.pvric31HwSupport);
            VLOG(INFO, "sramMode        %d\n", multiConfig->instConfig[i].decConfig.sramMode);

        }
        else if (ENC_COD_STD(multiConfig->instConfig[i].stdMode)) { /* encodec */
            VLOG(INFO, "\n==================Instance %d encode Param=============\n", i);
            VLOG(INFO, "stdMode            %d\n", multiConfig->instConfig[i].encConfig.stdMode);
            VLOG(INFO, "yuvSourceBaseDir   %s\n", multiConfig->instConfig[i].encConfig.yuvSourceBaseDir);
            VLOG(INFO, "yuvFileName        %s\n", multiConfig->instConfig[i].encConfig.yuvFileName);
            VLOG(INFO, "bitstreamFileName  %s\n", multiConfig->instConfig[i].encConfig.bitstreamFileName);
            VLOG(INFO, "mapType            %d\n", multiConfig->instConfig[i].encConfig.mapType);
            VLOG(INFO, "picWidth           %d\n", multiConfig->instConfig[i].encConfig.picWidth);
            VLOG(INFO, "picHeight          %d\n", multiConfig->instConfig[i].encConfig.picHeight);
            VLOG(INFO, "srcFormat          %d\n", multiConfig->instConfig[i].encConfig.srcFormat);
            VLOG(INFO, "coreIdx            %d\n", multiConfig->instConfig[i].encConfig.coreIdx);
            VLOG(INFO, "sramMode           %d\n", multiConfig->instConfig[i].encConfig.sramMode);
            VLOG(INFO, "ringBufferEnable   %d\n", multiConfig->instConfig[i].encConfig.ringBufferEnable);
            VLOG(INFO, "cbcrInterleave     %d\n", multiConfig->instConfig[i].encConfig.cbcrInterleave);
            VLOG(INFO, "nv21               %d\n", multiConfig->instConfig[i].encConfig.nv21);
            VLOG(INFO, "frameEndian        %d\n", multiConfig->instConfig[i].encConfig.frame_endian);
            VLOG(INFO, "streamEndian       %d\n", multiConfig->instConfig[i].encConfig.stream_endian);
            VLOG(INFO, "lineBufIntEn       %d\n", multiConfig->instConfig[i].encConfig.lineBufIntEn);
            VLOG(INFO, "cfgFileName        %s\n", multiConfig->instConfig[i].encConfig.cfgFileName);
            VLOG(INFO, "outputfile         %s\n", multiConfig->instConfig[i].encConfig.bitstreamFileName);
            VLOG(INFO, "sramMode           %d\n", multiConfig->instConfig[i].encConfig.sramMode);
        }
        else {
            VLOG(ERR, "Not support format %d; instance num %d\n", multiConfig->instConfig[i].stdMode, i);
        }
    }
}

/**
 *Brief print help information
 *param programName program name
 *return void
 *note:
 */
static void Help(
    const char *programName
)
{
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s(API v%d.%d.%d)\n", GetBasename(programName), API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    VLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    VLOG(INFO, "\tSample program controlling the Chips&Media VPU\n");
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s [option] --input <stream list file, aka cmd file>\n", GetBasename(programName));
    VLOG(INFO, "-h                          help\n");
    VLOG(INFO, "-v                          print version information\n");
    VLOG(INFO, "-n                          decode frames number \n");
    VLOG(INFO, "-c                          enable comparison mode\n");
    VLOG(INFO, "                            1 : compare with golden stream that specified --ref_file_path option\n");
    VLOG(INFO, "                            2 : compare with golden md5 that specified --ref_file_path option\n");
    VLOG(INFO, "--instance-num              0~4\n");
    VLOG(INFO, "--codec                     The index of codec\n");
    VLOG(INFO, "                            AVC = 0, VC1 = 1, MPEG2 = 2, MPEG4 = 3, H263 = 4\n");
    VLOG(INFO, "                            DIV3 = 5, RV = 6, AVS = 7, THO = 9, VP3 = 10, VP8 = 11, HEVC = 12, VP9 = 13\n");
    VLOG(INFO, "                            AVC_ENC = 14, MP4_ENC = 17, H263_ENC = 18, HEVC_ENC = 26\n");
    VLOG(INFO, "--input                     bitstream(decoder) or cfg(encoder) path\n");
    VLOG(INFO, "--output                    yuv(decoder) or bitstream(encoder) path\n");
    VLOG(INFO, "--ref_file_path             Golden md5 or stream path\n");
    VLOG(INFO, "--sclw                      set scale width value.\n");
    VLOG(INFO, "--sclh                      set scale height value.\n");
    VLOG(INFO, "--bsmode                    set bitstream mode.\n");
    VLOG(INFO, "--enable-mvc                enable mvc option. default 0\n");
    VLOG(INFO, "--enable-wtl                enable wtl option. default 0\n");
    VLOG(INFO, "--wtl-format                wtl format option. default 0\n");
    VLOG(INFO, "--stream-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--frame-endian              16~31, default 31(LE) Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--enable-cbcrinterleave     enable cbcrInterleave(NV12), default off\n");
    VLOG(INFO, "--enable-nv21               enable NV21, default off\n");
    VLOG(INFO, "                            0  : YUV420 8bit\n");
    VLOG(INFO, "                            1  : YUV422 8bit\n");
    VLOG(INFO, "                            2  : YUV224 8bit\n");
    VLOG(INFO, "                            3  : YUV444 8bit\n");
    VLOG(INFO, "                            4  : YUV400 8bit\n");
    VLOG(INFO, "                            5  : YUV420 16bit MSB\n");
    VLOG(INFO, "                            6  : YUV420 16bit LSB\n");
    VLOG(INFO, "                            7  : YUV420 32bit MSB \n");
    VLOG(INFO, "                            8  : YUV420 32bit LSB \n");
    VLOG(INFO, "                            9  : YUV422 16bit MSB\n");
    VLOG(INFO, "                            10 : YUV422 16bit LSB\n");
    VLOG(INFO, "                            11 : YUV422 32bit MSB (10bit)\n");
    VLOG(INFO, "                            12 : YUV422 32bit LSB (10bit)\n");
    VLOG(INFO, "                            13 : YUYV (8bit)\n");
    VLOG(INFO, "                            14 : YUYV 16bit MSB \n");
    VLOG(INFO, "                            15 : YUYV 16bit LSB \n");
    VLOG(INFO, "                            18 : YVYU (8bit)\n");
    VLOG(INFO, "                            19 : YVYU 16bit LSB \n");
    VLOG(INFO, "                            20 : YVYU 16bit LSB \n");
    VLOG(INFO, "                            23 : UYVY (8bit)\n");
    VLOG(INFO, "                            24 : UYVY 16bit MSB \n");
    VLOG(INFO, "                            25 : UYVY 16bit LSB \n");
    VLOG(INFO, "                            28 : VYUY (8bit)\n");
    VLOG(INFO, "                            29 : VYUY 16bit MSB \n");
    VLOG(INFO, "                            30 : VYUY 16bit LSB \n");
    VLOG(INFO, "--test-num                  loop count for test. default 1\n");
    VLOG(INFO, "--sram-cfg                  disable/enable sram cfg: decode 0 or 3  168K second AXI \n");
    VLOG(INFO, "                                                     encode 2   144K Linebuffer\n");
    VLOG(INFO, "                                                            1   72 Linebuffer && 96Kb second AXI\n");
    VLOG(INFO, "--loop-count                loop test mean looping in decoding/encode thread \n");
}


/**
 *Brief handle process for receiving signal
 *param signal num
 *return void
 *note:
 */
static void sigterm_handler(int sig)
{
    int  i = 0;
    int  isExit = 0;
    VLOG(WARN, "SIGNAL value %d exit...\n", sig);

    /* set exitFlag to make instance thread must be exit */
    for (i = 0; i < multiConfig.numMulti; i++) {
        if (ENC_COD_STD(multiConfig.instConfig[i].stdMode)) {
            multiConfig.instConfig[i].encConfig.exitFlag = THREAD_EXIT;
        }
        if (DEC_COD_STD(multiConfig.instConfig[i].stdMode)) {
            multiConfig.instConfig[i].decConfig.exitFlag = THREAD_EXIT;
        }
    }

    /* check the instance thead whether have exited success */
    do {
        isExit = 1;
        for (i = 0; i < multiConfig.numMulti; i++) {
            if ((ENC_COD_STD(multiConfig.instConfig[i].stdMode))
                && (multiConfig.instConfig[i].encConfig.exitFlag != THREAD_EXIT_SUCCESS)) {
                    isExit = 0;
                    break;
            }
            if ((DEC_COD_STD(multiConfig.instConfig[i].stdMode))
                && (multiConfig.instConfig[i].decConfig.exitFlag != THREAD_EXIT_SUCCESS)) {
                    isExit = 0;
                    break;
            }
        }

        if (isExit) {
            break;
        }

        usleep(1000 * 1000);
    } while (TRUE);

    exit(1);
}

/**
 *Brief config mult instance param
 *param argc user input param num
 *param argv user input param info
 *param multiConfig multi instance info
 *return void
 *note:
 */
static void SetMultiInstParam(
    int argc,
    char **argv,
    TestMultiConfig
    *multiConfig)
{
    int32_t opt, index, i;
    char *tempArg = NULL;
    char *optString         = "fvc:hn:";
    const char *optName;

    struct option options[] = {
        {"instance-num",        1, NULL, 0},
        {"codec",               1, NULL, 0},
        {"input",               1, NULL, 0},
        {"output",              1, NULL, 0},
        {"ref_file_path",       1, NULL, 0},
        {"sclw",                1, NULL, 0},
        {"sclh",                1, NULL, 0},
        {"bsmode",              1, NULL, 0},
        {"enable-mvc",          1, NULL, 0},
        {"enable-wtl",          1, NULL, 0},
        {"enable-pvric",        1, NULL, 0},
        {"enable-sync",         1, NULL, 0},
        {"enable-nv21",         1, NULL, 0},
        {"enable-tiled2linear", 1, NULL, 0},
        {"wtl-format",          1, NULL, 0},
        {"test-num",            1, NULL, 0},
        {"sram-cfg",            1, NULL, 0},
        {"loop-count",          1, NULL, 0},
        {NULL,                  0, NULL, 0},
    };

    /* default config */
    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
        multiConfig->testNum = 1; /* loop one time */
        multiConfig->numMulti = 1; /* one instance */
        multiConfig->instConfig[i].bsmode =  BS_MODE_PIC_END;
        multiConfig->instConfig[i].numFrames = 0x7fffffff;  /* default decode whole file */
    }

    /* dynamic config */
    setvbuf(stdout, NULL, _IONBF, 0);

    while ((opt = getopt_long(argc, argv, optString, options, &index)) != -1) {
        switch (opt) {
            case 'v':
                multiConfig->showVersion = TRUE;
                break;

            case 'n':
                tempArg = strtok(optarg, ",");

                for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                    multiConfig->instConfig[i].numFrames = atoi(tempArg);
                    tempArg = strtok(NULL, ",");

                    if (tempArg == NULL)
                        break;
                }

                break;

            case 'c':
                tempArg = strtok(optarg, ",");

                for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                    multiConfig->instConfig[i].compareType = atoi(tempArg);
                    tempArg = strtok(NULL, ",");

                    if (tempArg == NULL)
                        break;
                }

                break;

            case 'h':
                Help(argv[0]);
                exit(0);

            case 0:
                optName = options[index].name;

                if (0 == strcmp("instance-num", optName)) {
                    multiConfig->numMulti = atoi(optarg);
                }
                else if (0 == strcmp("codec", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].stdMode = (MultiCodStd)atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("input", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        memcpy(multiConfig->instConfig[i].inputFilePath, tempArg, strlen(tempArg));
                        ChangePathStyle(multiConfig->instConfig[i].inputFilePath);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("output", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        if (0 != strcmp("null", tempArg)) {
                            memcpy(multiConfig->instConfig[i].outputFilePath, tempArg, strlen(tempArg));
                            ChangePathStyle(multiConfig->instConfig[i].outputFilePath);
                        }

                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("ref_file_path", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        memcpy(multiConfig->instConfig[i].refFilePath, tempArg, strlen(tempArg));
                        ChangePathStyle(multiConfig->instConfig[i].refFilePath);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("sclw", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].sclw = atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("sclh", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].sclh = atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("bsmode", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].bsmode = atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("enable-mvc", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].enableMVC = atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("enable-wtl", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].enableWTL = atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("wtl-format", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].wtlformat = atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("enable-pvric", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].pvricMode = atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("test-num", optName)) {
                    tempArg = strtok(optarg, ",");
                    multiConfig->testNum = atoi(tempArg);
                    break;
                }
                else if (0 == strcmp("sram-cfg", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].sramCfg = atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else if (0 == strcmp("loop-count", optName)) {
                    tempArg = strtok(optarg, ",");

                    for (i = 0; i < MAX_NUM_INSTANCE; i++) {
                        multiConfig->instConfig[i].loopCount = atoi(tempArg);
                        tempArg = strtok(NULL, ",");

                        if (tempArg == NULL)
                            break;
                    }
                }
                else {
                    VLOG(ERR, "unknown option: %s\n", optName);
                    exit(1) ;
                }

                break;

            default:
                VLOG(ERR, "%s\n", optarg);
                exit(1) ;
        }
    }

    /* exclusive config */
    for (i = 0; i < multiConfig->numMulti; i++) {
        /*decode */
        if (DEC_COD_STD(multiConfig->instConfig[i].stdMode))
            SetDecMultiParam(multiConfig, i);

        /* encode */
        if (ENC_COD_STD(multiConfig->instConfig[i].stdMode))
            SetEncMultiParam(multiConfig, i);
    }
}

/**
 *@Brief main enrty for test
 *@param argc user input param num
 *@param argv user input param info
 *@return int
 *@note:
 */
int main(
    int argc,
    char **argv
)
{
    memset(&multiConfig, 0, sizeof(TestMultiConfig));
    signal(SIGINT, sigterm_handler);  /* Interrupt (ANSI).    */
    signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */
    SetMultiInstParam(argc, argv, &multiConfig);

    do {
        InitLog();
        PrintMultiParam(&multiConfig);
        osal_init_keyboard();

        if (MultiInstanceTest(&multiConfig) != 0) {
            VLOG(ERR, "Failed to MultiInstanceTest()\n");
            osal_close_keyboard();
            DeInitLog();
            return 1;
        }

        osal_close_keyboard();
        multiConfig.testNum--;
        DeInitLog();
    } while (multiConfig.testNum > 0);

    return 0;
}
