/*
 * Copyright (C) 2006 Chips & Media.
 * Copyright (C) 2020 Semidrive Technology Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <getopt.h>

#include "main_helper.h"
#include "test_case.h"


#define PRINT_STR(x) VLOG(INFO, "codec: %s\n", #x)
#define CODEC_STR(x) \
    do { \
        switch (x) { \
            case STD_AVC: \
                PRINT_STR(STD_AVC); \
                break; \
            case STD_VC1: \
                PRINT_STR(STD_VC1); \
                break; \
            case STD_MPEG2: \
                PRINT_STR(STD_MPEG2); \
                break; \
            case STD_MPEG4: \
                PRINT_STR(STD_MPEG4); \
                break; \
            case STD_H263: \
                PRINT_STR(STD_H263); \
                break; \
            case STD_DIV3: \
                PRINT_STR(STD_DIV3); \
                break; \
            case STD_RV: \
                PRINT_STR(STD_RV); \
                break; \
            case STD_AVS: \
                PRINT_STR(STD_AVS); \
                break; \
            case STD_THO: \
                PRINT_STR(STD_THO); \
                break; \
            case STD_VP3: \
                PRINT_STR(STD_VP3); \
                break; \
            case STD_VP8: \
                PRINT_STR(STD_VP8); \
                break; \
            case STD_HEVC: \
                PRINT_STR(STD_HEVC); \
                break; \
            case STD_VP9: \
                PRINT_STR(STD_VP9); \
                break; \
            default: \
                VLOG(ERR, "unknown codec\n"); \
                break; \
        } \
    } while (0)

#define STREAM_BUF_SIZE_HEVC                0xA00000    // max bitstream size(HEVC:10MB,VP9:not specified)
#define STREAM_BUF_SIZE_VP9                 0x1400000    // max bitstream size(HEVC:10MB,VP9:not specified)

extern BOOL TestDecoderCoda(
    TestDecConfig *param
);

extern BOOL TestDecoderWave(
    TestDecConfig *param
);

extern BOOL TestEncoder(
    TestEncConfig *param
);

static TestDecConfig decConfig;
static TestEncConfig encConfig;
static bool is_sdk_test = false;

static void Help(const char *programName)
{
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s(API v%d.%d.%d)\n", GetBasename(programName), API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    // VLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s [option] --input bistream\n", GetBasename(programName));
    VLOG(INFO, "-h                          help\n");
    VLOG(INFO, "-n [num]                    output frame number\n");
    VLOG(INFO, "-v                          print version information\n");
    VLOG(INFO, "-e                          encoder test (configured by .cfg)\n");
    VLOG(INFO, "-c                          compare with golden\n");
    VLOG(INFO, "                            0 : no comparison\n");
    VLOG(INFO, "                            1 : compare with golden yuv that specified --ref-yuv option\n");
    VLOG(INFO, "-s                          1 : enable sdk test option \n");
    VLOG(INFO, "--input                     bitstream path\n");
    VLOG(INFO, "--output                    YUV path\n");
    VLOG(INFO, "--codec                     codec index, HEVC:12,VP9:13,AVC:0,MPEG4:3,H2634,RV:6,\n");
    VLOG(INFO, "--bsmode                    0: INTERRUPT MODE, 1: reserved, 2: PICEND MODE\n");
    VLOG(INFO, "--coreIdx                   core index: default 0\n");
    VLOG(INFO, "--loop-count                integer number. loop test, default 0\n");
    VLOG(INFO, "--stream-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--frame-endian              16~31, default 31(LE) Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--enable-cbcrinterleave     enable cbcrInterleave(NV12), default off\n");
    VLOG(INFO, "--enable-nv21               enable NV21, default off\n");
    VLOG(INFO, "--secondary-axi             0~7: bit oring values, Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--enable-wtl                enable WTL. default off.\n");
    VLOG(INFO, "--num-vcores                number vcores to be used, default 1\n");
    VLOG(INFO, "--wtl-format                yuv format. default 0.\n");
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
    VLOG(INFO, "--sclw                      scale width of picture down. ceil8(width/8) <= scaledW <= width\n");
    VLOG(INFO, "                            99 for random test\n");
    VLOG(INFO, "--sclh                      scale height of picture down, ceil8(height/8) <= scaledH <= height\n");
    VLOG(INFO, "                            99 for random test\n");
    // VLOG(INFO, "--cra-bla                   Handle CRA as BLA\n");
    VLOG(INFO, "--userdata      hexadecimal 0 - disable userdata         1 - Reserved               2 - Reserved          4 - VUI\n"
               "                            8 - mastering_color_volume  10 - pic_timing            20 - itu_t_35 prefix  40 - unregistered prefix\n"
               "                            80 - itu_t_35 suffix        100 - unregistered suffix 200 - recovery point  400 - mastering_display_color\n"
               "                            All bits can be or-ing. ex) 404 -> mastering_color_volume | VUI\n");
    // VLOG(INFO, "--enable-thumbnail          enable thumbnail mode. default off\n");
    VLOG(INFO, "--skip-mode                 0: off, 1: Non-IRAP, 2: Non-Ref\n");
    VLOG(INFO, "--bwopt                     1 : enable bandwidth optimization function for saving bandwidth, default\n"
               "                            0 : disable bandwidth optimization function\n");
    VLOG(INFO, "--render                    0 : no rendering picture\n"
               "                            1 : render a picture with the framebuffer device\n");
    VLOG(INFO, "--ref-yuv                   golden yuv path\n");
    VLOG(INFO, "--feeding                   0: auto, 1: fixed-size, 2: ffempg, 3: size(4byte)+es\n");
    VLOG(INFO, "--pvric                     1: Enable PVRIC(Imagination Power VR Image Compress), 2: PVRIC3.0, 0: Disable\n");
    VLOG(INFO, "--sram-cfg                  0: 168K, 1: 96K, 2: 24K, 255: disable\n");
    // VLOG(INFO, "--pvric-padding-y=pixel     pixel value: 0 ~ 255 for 8bit, 0~1023 for 10bit\n");
    // VLOG(INFO, "--pvric-padding-c=pixel     pixel value: 0 ~ 255 for 8bit, 0~1023 for 10bit\n");
    VLOG(INFO, "--disflag                   1: rand three invaild display flag default 0\n");
    VLOG(INFO, "--seekflag                  1: rand seek flag  default 0\n");
}

static int do_vpu_test_dec(TestDecConfig *decConfig)
{
    CODEC_STR(decConfig->bitFormat);
    RetCode ret = RETCODE_SUCCESS;
    VLOG(INFO, "load firmware for %s\n", (decConfig->bitFormat == STD_HEVC || decConfig->bitFormat == STD_VP9) ? "wave" : "coda");

    pthread_t thread;
    void *retval;
    if (decConfig->bitFormat == STD_HEVC || decConfig->bitFormat == STD_VP9) {
        pthread_create(&thread, NULL, (void *)&TestDecoderWave, (void *)decConfig);
    } else {
        pthread_create(&thread, NULL, (void *)&TestDecoderCoda, (void *)decConfig);
    }

    pthread_join(thread, &retval);

    return ret;
}

static int do_vpu_test_enc(TestEncConfig *encConfig)
{
    UNREFERENCED_PARAMETER(encConfig);
    RetCode ret = RETCODE_SUCCESS;

    CODEC_STR(encConfig->stdMode);

    if (encConfig->stdMode != STD_AVC && encConfig->stdMode != STD_MPEG4 && encConfig->stdMode != STD_H263) {
        VLOG(ERR, "Encoder not supported!!!\n");
        return RETCODE_FAILURE;
    }

    // encConfig->stdMode       = STD_AVC;
    encConfig->mapType       = LINEAR_FRAME_MAP;
    encConfig->frame_endian  = VDI_LITTLE_ENDIAN;
    encConfig->stream_endian = VDI_LITTLE_ENDIAN;
    encConfig->ringBufferEnable = FALSE;
    strcpy(encConfig->cfgFileName, "encoder.cfg");
    sprintf(encConfig->bitstreamFileName, "output.bin");
    sprintf(encConfig->yuvSourceBaseDir, "./");

    if (encConfig->mapType == TILED_FRAME_MB_RASTER_MAP || encConfig->mapType == TILED_FIELD_MB_RASTER_MAP) {
        encConfig->cbcrInterleave = TRUE;
    }

    if (encConfig->rotAngle > 0 || encConfig->mirDir > 0) {
        encConfig->useRot = TRUE;
    }

    if (is_sdk_test) {
        struct test_config  testConfig = {0};

        testConfig.enc_config = encConfig;
        testConfig.is_encode = 1;
        testConfig.core_id = encConfig->coreIdx;
        ret = sdk_test_entry(&testConfig);
    }
    else {
        TestEncoder(encConfig);
    }

    return ret;
}

int main(int argc, char **argv)
{
    RetCode ret = RETCODE_SUCCESS;
    int opt, index;
    bool is_encoder = false;
    char **stop = NULL;
    int mode = 0x2;

static   struct option options[] = {
        {(char *)"output",                1, NULL, 0},    /*  0 */
        {(char *)"input",                 1, NULL, 0},
        {(char *)"codec",                 1, NULL, 0},
        {(char *)"render",                1, NULL, 0},
        {(char *)"maptype",               1, NULL, 0},
        {(char *)"enable-wtl",            0, NULL, 0},
        {(char *)"coreIdx",               1, NULL, 0},
        {(char *)"loop-count",            1, NULL, 0},
        {(char *)"enable-cbcrinterleave", 0, NULL, 0},
        {(char *)"stream-endian",         1, NULL, 0},
        {(char *)"frame-endian",          1, NULL, 0},    /* 10 */
        {(char *)"enable-nv21",           0, NULL, 0},
        {(char *)"enable-tiled2linear",   0, NULL, 0},
        {(char *)"pvric",                 0, NULL, 0},
        {(char *)"sclw",                  1, NULL, 0},
        {(char *)"sclh",                  1, NULL, 0},
        {(char *)"secondary-axi",         1, NULL, 0},
        {(char *)"bsmode",                1, NULL, 0},
        {(char *)"wtl-format",            1, NULL, 0},
        {(char *)"enable-mvc",            0, NULL, 0},
        {(char *)"sram-cfg",              1, NULL, 0},
        {(char *)"disflag",               1, NULL, 0},
        {(char *)"seekflag",              1, NULL, 0},
        {NULL,                    0, NULL, 0},
    };
    char *optString   = (char *)"hcesn:";

    char *buf[argc];
    for (int i = 0; i < argc; i++) {
        buf[i] = (char *)argv[i];
        // VLOG(TRACE, "buf[%d] %s\n", i, buf[i]);
    }

    InitLog();

    //default setting.
    osal_memset(&decConfig, 0, sizeof(decConfig));
    osal_memset(&encConfig, 0, sizeof(encConfig));

    decConfig.bitstreamMode            = BS_MODE_PIC_END;
    decConfig.feedingMode              = FEEDING_METHOD_FRAME_SIZE;
    decConfig.streamEndian             = VDI_LITTLE_ENDIAN;
    decConfig.frameEndian              = VDI_LITTLE_ENDIAN;
    decConfig.wtlFormat                = FORMAT_420;
    decConfig.cbcrInterleave           = FALSE;
    decConfig.nv21                     = FALSE;
    decConfig.bitFormat                = STD_AVC;
    decConfig.renderType               = RENDER_DEVICE_NULL;
    decConfig.mapType                  = LINEAR_FRAME_MAP;
    decConfig.enableWTL                = FALSE;
    decConfig.pvricFbcEnable           = FALSE;
    decConfig.wtlMode                  = FF_FRAME;
    decConfig.scaleDownHeight          = 0;
    decConfig.scaleDownWidth           = 0;
    decConfig.forceOutNum              = 5;
    decConfig.instIdx                  = 0;
    decConfig.coreIdx                  = 0;
    decConfig.compareType              = NO_COMPARE;

    // VLOG(INFO, "###################################################################\n");
    while ((opt=getopt_long(argc, buf, optString, options, &index)) != -1) {
        switch (opt) {
            case '?':
            case 'h':
                Help(buf[0]);
                goto exit;
                break;

            case 'c':
                decConfig.compareType = YUV_COMPARE;
                break;

            case 'e':
                is_encoder = true;
                break;

            case 'n':
                decConfig.forceOutNum = atoi(optarg);
                // VLOG(INFO, "forceNum %d\n", decConfig.forceOutNum);
                break;
            case 's':
                is_sdk_test = true;
                break;
            case 0:
                switch (index) {
                    case 0:
                        memcpy(decConfig.outputPath, optarg, strlen(optarg));
                        ChangePathStyle(decConfig.outputPath);
                        break;

                    case 1:
                        memcpy(decConfig.inputPath, optarg, strlen(optarg));
                        ChangePathStyle(decConfig.inputPath);
                        break;

                    case 2:
                        decConfig.bitFormat = atoi(optarg);
                        encConfig.stdMode = atoi(optarg);
                        VLOG(INFO, "bitformat %d\n", decConfig.bitFormat);
                        break;

                    case 3:
                        decConfig.renderType = (RenderDeviceType)atoi(optarg);
                        if (decConfig.renderType < RENDER_DEVICE_NULL || decConfig.renderType >= RENDER_DEVICE_MAX) {
                            VLOG(ERR, "unknown render device type(%d)\n", decConfig.renderType);
                            Help(buf[0]);
                            ret = RETCODE_FAILURE;
                            goto exit;
                        }
                        break;

                    case 4:
                        decConfig.mapType = (TiledMapType)atoi(optarg);
                        break;

                    case 5:
                        decConfig.enableWTL = TRUE;
                        break;

                    case 6:
                        decConfig.coreIdx = atoi(optarg);
                        break;

                    case 7:
                        decConfig.loopCount = atoi(optarg);
                        break;

                    case 8:
                        decConfig.cbcrInterleave = TRUE;
                        break;

                    case 9:
                        decConfig.streamEndian = (EndianMode)atoi(optarg);
                        break;

                    case 10:
                        decConfig.frameEndian = (EndianMode)atoi(optarg);
                        break;

                    case 11:
                        decConfig.nv21           = TRUE;
                        decConfig.cbcrInterleave = TRUE;
                        break;

                    case 12:
                        decConfig.coda9.enableTiled2Linear = TRUE;
                        decConfig.coda9.tiled2LinearMode   = FF_FRAME;
                        decConfig.enableWTL                = FALSE;
                        break;

                    case 13:
                        decConfig.pvricFbcEnable = TRUE;
                        break;

                    case 14:
                        decConfig.scaleDownWidth = atoi(optarg);
                        VLOG(INFO, "scal wi %d\n", decConfig.scaleDownWidth);
                        break;

                    case 15:
                        decConfig.scaleDownHeight = atoi(optarg);
                        break;

                    case 16:
                        decConfig.secondaryAXI = strtoul(optarg, stop, !strncmp("0x", optarg, 2) ? 16 : 10);
                        encConfig.secondary_axi = strtoul(optarg, stop, !strncmp("0x", optarg, 2) ? 16 : 10);
                        break;

                    case 17:
                        decConfig.bitstreamMode = atoi(optarg);
                        break;

                    case 18:
                        decConfig.wtlFormat = atoi(optarg);
                        break;

                    case 19:
                        decConfig.coda9.enableMvc = TRUE;
                        break;

                    case 20: {
                         mode = strtoul(optarg, stop, !strncmp("0x", optarg, 2) ? 16 : 10);
                    //     vpu_set_sram_cfg(mode);
                    } break;

                    case 21:
                        decConfig.invalidDisFlag = atoi(optarg);
                        break;
                    case 22:
                        decConfig.seekflag = atoi(optarg);
                        break;
                }
                break;

            default:
                VLOG(ERR, "%s\n", optarg);
                Help(buf[0]);
                ret = RETCODE_FAILURE;
        }
    }

    if (is_encoder) {
        VLOG(INFO, "encoder now ... mode(%d) \n", mode);
        encConfig.sramMode = mode;
        ret = do_vpu_test_enc(&encConfig);
    }
    else {
        /* Check combination of parameters of decoder */
        if (decConfig.bitFormat != STD_HEVC && decConfig.bitFormat != STD_VP9) {
            decConfig.coda9.enableBWB          = VPU_ENABLE_BWB;
            decConfig.coda9.frameCacheBypass   = 0;
            decConfig.coda9.frameCacheBurst    = 0;
            decConfig.coda9.frameCacheMerge    = 3;
            decConfig.coda9.frameCacheWayShape = 15;
            if(decConfig.bitstreamMode == BS_MODE_INTERRUPT)
                decConfig.feedingMode = FEEDING_METHOD_FIXED_SIZE;
        } else if (decConfig.bitFormat == STD_HEVC) {
            decConfig.coreIdx = 1; //wave coreIdx
            decConfig.mapType = COMPRESSED_FRAME_MAP;
            decConfig.bsSize = STREAM_BUF_SIZE_HEVC;
            decConfig.streamEndian = VDI_128BIT_LITTLE_ENDIAN;
            decConfig.frameEndian = VDI_128BIT_LITTLE_ENDIAN;
            decConfig.wave4.fbcMode = 0x0C; //normal: 0x0C - Best prediction: 0x00 - Basic prediction: 0x3C
            decConfig.wave4.bwOptimization = FALSE;
            if(decConfig.bitstreamMode == BS_MODE_INTERRUPT)
                decConfig.feedingMode = FEEDING_METHOD_FIXED_SIZE;
        } else if (decConfig.bitFormat == STD_VP9) {
            decConfig.coreIdx = 1; //wave coreIdx
            decConfig.mapType = COMPRESSED_FRAME_MAP;
            decConfig.bitstreamMode = BS_MODE_PIC_END;
            decConfig.streamEndian = VDI_128BIT_LITTLE_ENDIAN;
            decConfig.frameEndian = VDI_128BIT_LITTLE_ENDIAN;
            decConfig.bsSize = STREAM_BUF_SIZE_VP9;
            decConfig.feedingMode = FEEDING_METHOD_FRAME_SIZE;
            decConfig.wave4.fbcMode = 0x0C; //normal: 0x0C - Best prediction: 0x00 - Basic prediction: 0x3C
            decConfig.wave4.bwOptimization = FALSE;
        }

        decConfig.sramMode = mode;
        VLOG(INFO, "################# decConfig ###################\n");
        VLOG(INFO, "forceOutNum     %d\n", decConfig.forceOutNum);
        VLOG(INFO, "bitFormat       %d\n", decConfig.bitFormat);
        VLOG(INFO, "mapType         %d\n", decConfig.mapType);
        VLOG(INFO, "enableWTL       %d\n", decConfig.enableWTL);
        VLOG(INFO, "cbcrInterleave  %d\n", decConfig.cbcrInterleave);
        VLOG(INFO, "nv21            %d\n", decConfig.nv21);
        VLOG(INFO, "pvricFbcEnable  %d\n", decConfig.pvricFbcEnable);
        VLOG(INFO, "streamEndian    %d\n", decConfig.streamEndian);
        VLOG(INFO, "frameEndian     %d\n", decConfig.frameEndian);
        VLOG(INFO, "scaleDownWidth  %d\n", decConfig.scaleDownWidth);
        VLOG(INFO, "scaleDownHeight %d\n", decConfig.scaleDownHeight);
        VLOG(INFO, "wtlFormat       %d\n", decConfig.wtlFormat);
        VLOG(INFO, "sramMode        %d\n", decConfig.sramMode);
        VLOG(INFO, "bsmode          %d\n", decConfig.bitstreamMode);
        VLOG(INFO, "feedingMode     %d\n", decConfig.feedingMode);
        VLOG(INFO, "invalidDisFlag  %d\n", decConfig.invalidDisFlag);
        VLOG(INFO, "seekflag        %d\n", decConfig.seekflag);

        if (is_sdk_test) {
            struct test_config  testCfg = {0};
            testCfg.dec_config = &decConfig;
            testCfg.is_decode = 1;
            testCfg.core_id = decConfig.coreIdx;
            ret = sdk_test_entry(&testCfg);
        }
        else {
            ret = do_vpu_test_dec(&decConfig);
        }
    }
exit:
    DeInitLog();
    return ret;
}
