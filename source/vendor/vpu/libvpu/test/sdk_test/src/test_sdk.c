/*
 * test_sdk.c
 *
 * Copyright (C) 2020 Semidrive Technology Co., Ltd.
 *
 * Description: Imple vpu sdk test function
 *
 * Revision Histrory:
 * -----------------
 * 1.0, 01/09/2020  chentianming <tianming.chen@semidrive.com> create this file
 *
 */
/*******************************************************************************
 *                       Include header files                                 *
 ******************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "test_case.h"
#include "main_helper.h"

/*******************************************************************************
 *                        Macro definitions                                   *
 ******************************************************************************/
#define PPU_FB_COUNT              0x2
#define EXTRA_FRAME_BUFFER_NUM    0x1

#define STREAM_BUF_SIZE           0xA00000
#define ENC_STREAM_BUF_SIZE       0x400000
#define USERDATA_BUFFER_SIZE      (512*1024)
#define MAX_FRAME_BUFFER          0x10
#define DEFAULT_READ_SIZE         1460

#define CODA_FW_PATH              "/vendor/firmware/coda980.out"
#define WAVE_FW_PATH              "/vendor/firmware/pissarro.bin"
/*******************************************************************************
 *                        Date struct  definitions                            *
 ******************************************************************************/


/*******************************************************************************
 *                        static function declarations                        *
 ******************************************************************************/
static void sdk_test_coda_decode_param_config(struct test_context *context, DecOpenParam *open_param);
static int sdk_test_coda_decode_init(struct test_context *context);
static int sdk_test_coda_decode_deinit(struct test_context *context);
static int sdk_test_case_encode_open_param_config(struct test_context *context);
static int sdk_test_case_encode_open(struct test_context *context);
static int sdk_test_coda_encode_init(struct test_context *context);
static int sdk_test_coda_encode_deinit(struct test_context *context);
static int sdk_test_coda_init(struct test_context *context);
static int sdk_test_coda_deinit(struct test_context *context);
static void sdk_test_wave_decode_param_config(struct test_context *context, DecOpenParam  *open_param);
static int sdk_test_wave_decode_init(struct test_context *context);
static int sdk_test_wave_decode_deinit(struct test_context *context);
static int sdk_test_wave_init(struct test_context *context);
static int sdk_test_wave_deinit(struct test_context *context);
static int sdk_test_coda_decode_feeder_create(struct test_context *context);
static int sdk_test_code_decode_sequence_init(struct test_context *context);
static int sdk_test_coda_decode_ppu_enable(struct test_context *context);
static int sdk_test_code_decode_buffer_register(struct test_context *context);
static int sdk_test_coda_decode_framebuffer_register(struct test_context *context);
static int sdk_test_coda_decode_preparative_create(struct test_context *context);
static int sdk_test_vpu_encode_sequence_header_build(struct test_context *context);
static int sdk_test_vpu_encode_feeder_create(struct test_context *context);
static int sdk_test_coda_encode_buffer_register(struct test_context *context);
static int sdk_test_coda_encode_framebuffer_register(struct test_context *context);
static int sdk_test_coda_encode_one_frame(struct test_context *context);
static int sdk_test_coda_encode_output_info_get(struct test_context *context);
static int sdk_test_coda_encode_sequence_init(struct test_context *context);
static void sdk_test_coda_encode_sequence_deinit(struct test_context *context);
static int sdk_test_coda_encode_preparative_create(struct test_context *context);
static int sdk_test_vpu_encode_procedure(struct test_context *context);
static int sdk_test_coda_preparative_create(struct test_context *context);
static int sdk_test_wave_decode_sequence_init(struct test_context *context);
static int sdk_test_wave_decode_buffer_register(struct test_context *context);
static void sdk_test_wave_decode_sequence_deinit(struct test_context *context);
static int sdk_test_wave_decode_framebuffer_register(struct test_context *context);
static int sdk_test_wave_decode_feeder_create(struct test_context *context);
static int sdk_test_wave_preparative_create(struct test_context *context);
static int sdk_test_vpu_preparative_create(struct test_context *context);
static void sdk_test_coda_decode_feeder_destory(struct test_context *context);
static void sdk_test_code_decode_framebuffer_unregister(struct test_context *context);
static void sdk_test_coda_decode_preparative_destroy(struct test_context *context);
static void sdk_test_coda_encode_preparative_destroy(struct test_context *context);
static void sdk_test_coda_preparative_destroy(struct test_context *context);
static void sdk_test_wave_decode_feeder_destory(struct test_context *context);
static void sdk_test_wave_decode_framebuffer_unregister(struct test_context *context);
static void sdk_test_wave_preparative_destroy(struct test_context *context);
static void sdk_test_vpu_preparative_destroy(struct test_context *context);
static int sdk_test_code_decode_streams_feeder(struct test_context *context);
static int sdk_test_coda_decode_one_frame_procedure(struct test_context *context, DecOutputInfo *output_info);
static void sdk_test_coda_decode_yuv_save(struct test_context *context, DecOutputInfo *output_info, VpuRect *ppu);
static int sdk_test_coda_decode_output_info_get(struct test_context *context, DecOutputInfo *output_info);
static int sdk_test_coda_decode_procedure(struct test_context *context);
static int sdk_test_wave_decode_streams_feeder(struct test_context *context);
static int sdk_test_wave_decode_one_frame_procedure(struct test_context *context);
static int sdk_test_wave_decode_output_info_get(struct test_context *context, DecOutputInfo *output_info);
static int sdk_test_wave_decode_procedure(struct test_context *context);
static int sdk_test_vpu_decode_procedure(struct test_context *context);
static int sdk_test_frame_seek(struct test_context *context, uint64_t ts_time);
static int sdk_test_frame_seek_procedure(struct test_context *context);
static int sdk_test_decode_coda_seek_procedure(struct test_context *context);
static int sdk_test_decode_wave_seek_procedure(struct test_context *context);
static int sdk_test_vpu_decode_seek_procedure(struct test_context *context);
static int sdk_test_vpu_seek_param_config(struct test_context *context);
static int sdk_test_frame_seek_position_start(struct test_context *context);
static int sdk_test_vpu_decode_loop_param_config(struct test_context  *context);

/*******************************************************************************
 *                        static function definitions                         *
 ******************************************************************************/
/*
 * brief sdk_test_coda_decode_param_config
         param setting for coda decoding
 * param context  context info in test
 * param open_param  param for decoding
 * return 0  success
 *        !0 failure code
 * notes
 */
static void sdk_test_coda_decode_param_config(struct test_context *context, DecOpenParam *open_param)
{
    TestDecConfig *param_cfg = NULL;
    param_cfg = context->test_cfg->dec_config;

    /* set up decoder configurations */
    open_param->bitstreamFormat     = (CodStd)param_cfg->bitFormat;
    open_param->avcExtension        = param_cfg->coda9.enableMvc;
    open_param->coreIdx             = param_cfg->coreIdx;
    open_param->bitstreamBuffer     = context->vpu_dec_info->bstream_buf[0].phys_addr;
    open_param->bitstreamBufferSize = context->vpu_dec_info->bstream_buf[0].size;
    open_param->bitstreamMode       = param_cfg->bitstreamMode;
    open_param->tiled2LinearEnable  = param_cfg->coda9.enableTiled2Linear;
    open_param->tiled2LinearMode    = param_cfg->coda9.tiled2LinearMode;
    open_param->wtlEnable           = param_cfg->enableWTL;
    open_param->wtlMode             = param_cfg->wtlMode;
    open_param->cbcrInterleave      = param_cfg->cbcrInterleave;
    open_param->nv21                = param_cfg->nv21;
    open_param->streamEndian        = param_cfg->streamEndian;
    open_param->frameEndian         = param_cfg->frameEndian;
    open_param->bwbEnable           = param_cfg->coda9.enableBWB;
    open_param->mp4DeblkEnable      = param_cfg->coda9.enableDeblock;
    open_param->mp4Class            = param_cfg->coda9.mp4class;

    log_debug("------------------------------ DECODER OPTIONS ------------------------------\n");
    log_debug("[core_id            ]: %d\n", open_param->coreIdx            );
    log_debug("[map_type            ]: %d\n", param_cfg->mapType           );
    log_debug("[tiled2linear       ]: %d\n", param_cfg->coda9.enableTiled2Linear);
    log_debug("[wtlEnable          ]: %d\n", open_param->wtlEnable          );
    log_debug("[wtlMode            ]: %d\n", open_param->wtlMode            );
    log_debug("[bitstreamBuffer    ]: 0x%08x\n", open_param->bitstreamBuffer);
    log_debug("[bitstreamBufferSize]: %d\n", open_param->bitstreamBufferSize);
    log_debug("[bitstreamMode      ]: %d\n", open_param->bitstreamMode      );
    log_debug("[cbcrInterleave     ]: %d\n", open_param->cbcrInterleave     );
    log_debug("[nv21               ]: %d\n", open_param->nv21               );
    log_debug("[streamEndian       ]: %d\n", open_param->streamEndian       );
    log_debug("[frameEndian        ]: %d\n", open_param->frameEndian        );
    log_debug("[BWB                ]: %d\n", open_param->bwbEnable          );
    log_debug("-----------------------------------------------------------------------------\n");
}

/*
 * brief sdk_test_coda_decode_init
         coda device inition, load fw, config param for decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_init(struct test_context *context)
{
    int ret = RET_CODE_SUCCESS;
    uint32_t core_id = 0;
    TestDecConfig *param_cfg = NULL;
    DecOpenParam  open_param = {0};

    CHECK_PARAM(context);
    param_cfg = context->test_cfg->dec_config;
    core_id = param_cfg->coreIdx;

    if (LoadFirmware(PRODUCT_ID_980, (Uint8 **) & (context->vpu_dec_info->bit_code), &(context->vpu_dec_info->bit_size), CODA_FW_PATH) < 0) {
        log_err("failed to load firmware: path(%s)\n", CODA_FW_PATH);
        return RET_CODE_FAILURE_LOAD_FW;
    }

    ret = VPU_InitWithBitcode(core_id, (const uint16_t *)(context->vpu_dec_info->bit_code), (context->vpu_dec_info->bit_size));

    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
        log_err("failed to boot up VPU core_id(%d)\n", core_id);
        return RET_CODE_FAILURE_LOAD_FW;
    }

    PrintVpuVersionInfo(core_id);
    context->vpu_dec_info->bstream_buf[0].size = STREAM_BUF_SIZE;

    if (vdi_allocate_dma_memory(core_id, &(context->vpu_dec_info->bstream_buf[0])) < 0) {
        log_err("fail to allocate bitstream buffer\n");
        return RET_CODE_FAILURE_ALLOC;
    }

    if ( param_cfg->bitstreamMode != BS_MODE_PIC_END) {
        log_err("only support pic_end mode now ...\n");
        return RET_CODE_INVALID_PARAM;
    }

    log_debug("created bitstreamfeeder for %s\n", param_cfg->inputPath);
    /* set up decoder configurations */
    sdk_test_coda_decode_param_config(context, &open_param);

    if ((ret = VPU_DecOpen(&(context->vpu_dec_info->handle), &open_param)) != RETCODE_SUCCESS) {
        log_err("VPU_DecOpen failed Error code is 0x%x \n", ret );
        goto err_open;
    }

    context->vpu_dec_info->open_param = open_param;
    log_debug(" %s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;

err_open:

    if (context->vpu_dec_info->bstream_buf[0].phys_addr) {
        vdi_free_dma_memory(context->vpu_dec_info->core_id, &(context->vpu_dec_info->bstream_buf[0]));
    }

    if (context->vpu_dec_info->bit_code) {
        free(context->vpu_dec_info->bit_code);
        context->vpu_dec_info->bit_code = NULL;
    }

    return RET_CODE_FAILURE_DEVICE_OPEN;
}

/*
 * brief sdk_test_coda_decode_deinit
         coda device inition and uninit
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_deinit(struct test_context *context)
{
    int ret = RET_CODE_SUCCESS;

    CHECK_PARAM(context);
    ret = VPU_DecClose(context->vpu_dec_info->handle);

    if (context->vpu_dec_info->bstream_buf[0].phys_addr) {
        vdi_free_dma_memory(context->vpu_dec_info->core_id, &(context->vpu_dec_info->bstream_buf[0]));
    }

    if (context->vpu_dec_info->bit_code) {
        free(context->vpu_dec_info->bit_code);
        context->vpu_dec_info->bit_code = NULL;
    }

    log_debug(" %s : %d \n", __FUNCTION__, __LINE__);
    return ret;
}

/*
 * brief sdk_test_case_encode_open_param_config
         encode param config for open device
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_case_encode_open_param_config(struct test_context *context)
{
    int ret = 0;
    int map_type = 0;
    uint32_t core_id = 0;
    TestEncConfig *param = NULL;

    CHECK_PARAM(context);
    core_id = context->test_cfg->enc_config->coreIdx;
    param = context->test_cfg->enc_config;

    context->vpu_enc_info->enc_open_param.bitstreamFormat = param->stdMode;
    map_type = (TiledMapType)param->mapType;
    context->vpu_enc_info->enc_open_param.linear2TiledEnable = param->coda9.enableLinear2Tiled;

    if (context->vpu_enc_info->enc_open_param.linear2TiledEnable == TRUE) {
        context->vpu_enc_info->enc_open_param.linear2TiledMode = FF_FRAME;
    }

    ret = GetEncOpenParam(&(context->vpu_enc_info->enc_open_param), param, &(context->vpu_enc_info->cfg_param));

    if (ret == 0) {
        log_err("failed to parse CFG file\n");
        return  RET_CODE_FAILURE_ENC_OPEN_PARAM;
    }

    log_debug("------------------------------ ENCODER PARAM ------------------------------\n");
    log_debug("[yuvSourceBaseDir   ]: %s\n", param->yuvSourceBaseDir );
    log_debug("[yuvFileName        ]: %s\n", param->yuvFileName      );
    log_debug("[bitstreamFileName  ]: %s\n", param->bitstreamFileName);
    log_debug("[map_type            ]: %d\n", param->mapType          );
    log_debug("[picWidth           ]: %d\n", param->picWidth         );
    log_debug("[picHeight          ]: %d\n", param->picHeight        );
    log_debug("[SrcFileName        ]: %s\n", context->vpu_enc_info->cfg_param.SrcFileName    );
    log_debug("[NumFrame           ]: %d\n", context->vpu_enc_info->cfg_param.NumFrame       );
    log_debug("[FrameRate          ]: %d\n", context->vpu_enc_info->cfg_param.FrameRate      );
    log_debug("[GOP                ]: %d\n", context->vpu_enc_info->cfg_param.GopPicNum      );
    log_debug("[PicQpY             ]: %d\n", context->vpu_enc_info->cfg_param.PicQpY      );
    log_debug("[SliceMode          ]: %d\n", context->vpu_enc_info->cfg_param.SliceMode      );
    log_debug("[SliceSizeNum       ]: %d\n", context->vpu_enc_info->cfg_param.SliceSizeNum      );
    log_debug("[RcBitRate          ]: %d\n", context->vpu_enc_info->cfg_param.RcBitRate      );
    log_debug("[transform8x8Mode   ]: %d\n", context->vpu_enc_info->cfg_param.transform8x8Mode      );
    log_debug("[IDRInterval        ]: %d\n", context->vpu_enc_info->cfg_param.IDRInterval      );
    log_debug("[frameSkipDisable   ]: %d\n", context->vpu_enc_info->cfg_param.frameSkipDisable);
    log_debug("[sramMode           ]: %d\n", param->sramMode);
    log_debug("---------------------------------------------------------------------------\n");

    if (context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.fieldFlag == TRUE) {
        if (param->rotAngle != 0 || param->mirDir != 0) {
            log_debug("warning %s:%d When field Flag is enabled. VPU doesn't support rotation or mirror in field encoding mode.\n",
                      __FUNCTION__, __LINE__);
            param->rotAngle = 0;
            param->mirDir   = MIRDIR_NONE;
        }
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_encode_init
         alloc bitstreams memory, decoding param config
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_case_encode_open(struct test_context *context)
{
    int ret = 0;
    uint32_t core_id = 0;
    TestEncConfig *param = NULL;

    CHECK_PARAM(context);
    core_id = context->test_cfg->enc_config->coreIdx;
    param = context->test_cfg->enc_config;
    context->vpu_enc_info->bstream_buf.size = ENC_STREAM_BUF_SIZE;

    if (vdi_allocate_dma_memory(core_id, &(context->vpu_enc_info->bstream_buf)) < 0) {
        log_err("fail to allocate bitstream buffer\n");
        return  RET_CODE_FAILURE_ALLOC;
    }

    context->vpu_enc_info->enc_open_param.bitstreamBuffer       = context->vpu_enc_info->bstream_buf.phys_addr;
    context->vpu_enc_info->enc_open_param.bitstreamBufferSize   = context->vpu_enc_info->bstream_buf.size;
    context->vpu_enc_info->enc_open_param.ringBufferEnable      = param->ringBufferEnable;
    context->vpu_enc_info->enc_open_param.cbcrInterleave        = param->cbcrInterleave;
    context->vpu_enc_info->enc_open_param.nv21                  = param->nv21;
    context->vpu_enc_info->enc_open_param.frameEndian           = param->frame_endian;
    context->vpu_enc_info->enc_open_param.streamEndian          = param->stream_endian;
    context->vpu_enc_info->enc_open_param.bwbEnable             = VPU_ENABLE_BWB;
    context->vpu_enc_info->enc_open_param.lineBufIntEn          = param->lineBufIntEn;
    context->vpu_enc_info->enc_open_param.coreIdx               = core_id;
    context->vpu_enc_info->enc_open_param.linear2TiledEnable    = param->coda9.enableLinear2Tiled;
    context->vpu_enc_info->enc_open_param.linear2TiledMode      = param->coda9.linear2TiledMode;
    context->vpu_enc_info->enc_open_param.cbcrOrder             = CBCR_ORDER_NORMAL;            // YV12 = CBCR_OERDER_REVERSED

    log_debug("[VPU] STEP 4 VPU_EncOpen\n");
    log_debug("------------------------------ ENCODER OPTIONS ------------------------------\n");
    log_debug("[core_id            ]: %d\n", context->vpu_enc_info->enc_open_param.coreIdx                 );
    log_debug("[bitstreamBufferSize]: %d\n", context->vpu_enc_info->enc_open_param.bitstreamBufferSize     );
    log_debug("[bitstreamFormat    ]: %d\n", context->vpu_enc_info->enc_open_param.bitstreamFormat         );
    log_debug("[ringBufferEnable   ]: %d\n", context->vpu_enc_info->enc_open_param.ringBufferEnable        );
    log_debug("[picWidth           ]: %d\n", context->vpu_enc_info->enc_open_param.picWidth                );
    log_debug("[picHeight          ]: %d\n", context->vpu_enc_info->enc_open_param.picHeight               );
    log_debug("[linear2TiledEnable ]: %d\n", context->vpu_enc_info->enc_open_param.linear2TiledEnable      );
    log_debug("[linear2TiledMode   ]: %d\n", context->vpu_enc_info->enc_open_param.linear2TiledMode        );
    log_debug("[frameRateInfo      ]: %d\n", context->vpu_enc_info->enc_open_param.frameRateInfo           );
    log_debug("[idrInterval        ]: %d\n", context->vpu_enc_info->enc_open_param.idrInterval             );
    log_debug("[meBlkMode          ]: %d\n", context->vpu_enc_info->enc_open_param.meBlkMode               );
    log_debug("[sliceMode          ]: %d\n", context->vpu_enc_info->enc_open_param.sliceMode               );
    log_debug("[bitRate            ]: %d\n", context->vpu_enc_info->enc_open_param.bitRate                 );
    log_debug("[cbcrInterleave     ]: %d\n", context->vpu_enc_info->enc_open_param.cbcrInterleave          );
    log_debug("[frameEndian        ]: %d\n", context->vpu_enc_info->enc_open_param.frameEndian             );
    log_debug("[streamEndian       ]: %d\n", context->vpu_enc_info->enc_open_param.streamEndian            );
    log_debug("[sourceEndian       ]: %d\n", context->vpu_enc_info->enc_open_param.sourceEndian            );
    log_debug("[bwbEnable          ]: %d\n", context->vpu_enc_info->enc_open_param.bwbEnable               );
    log_debug("[packedFormat       ]: %d\n", context->vpu_enc_info->enc_open_param.packedFormat            );
    log_debug("[srcFormat          ]: %d\n", context->vpu_enc_info->enc_open_param.srcFormat               );
    log_debug("[srcBitDepth        ]: %d\n", context->vpu_enc_info->enc_open_param.srcBitDepth             );
    log_debug("[nv21               ]: %d\n", context->vpu_enc_info->enc_open_param.nv21                    );
    log_debug("[enablePTS          ]: %d\n", context->vpu_enc_info->enc_open_param.enablePTS               );
    log_debug("[rcGopIQpOffsetEn   ]: %d\n", context->vpu_enc_info->enc_open_param.rcGopIQpOffsetEn        );
    log_debug("[rcGopIQpOffset     ]: %d\n", context->vpu_enc_info->enc_open_param.rcGopIQpOffset          );
    log_debug("[gopSize            ]: %d\n", context->vpu_enc_info->enc_open_param.gopSize                 );
    log_debug("[meBlkMode          ]: %d\n", context->vpu_enc_info->enc_open_param.meBlkMode               );
    log_debug("[sliceMode          ]: %d\n", context->vpu_enc_info->enc_open_param.sliceMode.sliceMode     );
    log_debug("[intraRefresh       ]: %d\n", context->vpu_enc_info->enc_open_param.intraRefresh            );
    log_debug("[ConscIntraRefreshEnable]: %d\n", context->vpu_enc_info->enc_open_param.ConscIntraRefreshEnable);
    log_debug("[CountIntraMbEnable ]: %d\n", context->vpu_enc_info->enc_open_param.CountIntraMbEnable         );
    log_debug("[userQpMax          ]: %d\n", context->vpu_enc_info->enc_open_param.userQpMax                  );
    log_debug("[maxIntraSize       ]: %d\n", context->vpu_enc_info->enc_open_param.maxIntraSize               );
    log_debug("[userMaxDeltaQp     ]: %d\n", context->vpu_enc_info->enc_open_param.userMaxDeltaQp             );
    log_debug("[userQpMin          ]: %d\n", context->vpu_enc_info->enc_open_param.userQpMin                  );
    log_debug("[userMinDeltaQp     ]: %d\n", context->vpu_enc_info->enc_open_param.userMinDeltaQp             );
    log_debug("[MEUseZeroPmv       ]: %d\n", context->vpu_enc_info->enc_open_param.MEUseZeroPmv               );
    log_debug("[userGamma          ]: %d\n", context->vpu_enc_info->enc_open_param.userGamma                  );
    log_debug("[rcIntervalMode     ]: %d\n", context->vpu_enc_info->enc_open_param.rcIntervalMode             );
    log_debug("[mbInterval         ]: %d\n", context->vpu_enc_info->enc_open_param.mbInterval                 );
    log_debug("[rcEnable           ]: %d\n", context->vpu_enc_info->enc_open_param.rcEnable                   );
    log_debug("[cbcrOrder          ]: %d\n", context->vpu_enc_info->enc_open_param.cbcrOrder                  );
    log_debug("[encodeVuiRbsp      ]: %d\n", context->vpu_enc_info->enc_open_param.encodeVuiRbsp              );
    log_debug("[avcParam.level     ]: %d\n", context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.level          );
    log_debug("[avcParam.profile   ]: %d\n", context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.profile        );
    log_debug("[avcParam.chroma    ]: %d\n", context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.chromaFormat400);
    log_debug("[avcParam.ppsNum    ]: %d\n", context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.ppsNum         );
    log_debug("-----------------------------------------------------------------------------\n");

    /* open an instance and get initial information for encoding.*/
    if ((ret = VPU_EncOpen(&(context->vpu_enc_info->handle), &(context->vpu_enc_info->enc_open_param))) != RETCODE_SUCCESS) {
        log_err("VPU_EncOpen failed Error code is 0x%x \n", ret);

        if (context->vpu_enc_info->bstream_buf.phys_addr) {
            vdi_free_dma_memory(core_id, &(context->vpu_enc_info->bstream_buf));
            context->vpu_enc_info->bstream_buf.phys_addr = 0;
        }

        return  ret;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_encode_init
         coda device  init oper for encoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_encode_init(struct test_context *context)
{
    int ret = 0;
    uint32_t core_id = 0;
    TestEncConfig *param = NULL;

    CHECK_PARAM(context);
    core_id = context->test_cfg->enc_config->coreIdx;
    param = context->test_cfg->enc_config;

    if (LoadFirmware(PRODUCT_ID_980, (Uint8 **) & (context->vpu_enc_info->bit_code), &(context->vpu_enc_info->bit_size), CODA_FW_PATH) < 0) {
        log_err("%s:%d failed to load firmware: %s\n", __FUNCTION__, __LINE__);
        return RET_CODE_FAILURE_LOAD_FW;
    }

    ret = VPU_InitWithBitcode(core_id, (const Uint16 *)(context->vpu_enc_info->bit_code), context->vpu_enc_info->bit_size);

    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
        log_err("failed to boot up VPU core_id(%d), ret(%d)\n", __FUNCTION__, __LINE__, core_id, ret);
        return ret;
    }

    /*set ME buffer  mode= 2 144K  */
    if (RET_CODE_SUCCESS != vdi_set_sram_cfg(core_id, param->sramMode)) {
        log_err("falied to set sram cfg mode value mode(%d) \n", param->sramMode);
        return RET_CODE_FAILURE_SRMA_CFG;
    }
    else {
        context->vpu_enc_info->enc_param.sramMode = param->sramMode;
    }

    CALL_FUNCTION(sdk_test_case_encode_open_param_config(context));
    PrintVpuVersionInfo(core_id);
    CALL_FUNCTION(sdk_test_case_encode_open(context));
    log_debug(" %s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_encode_deinit
         coda device deinit oper for encoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_encode_deinit(struct test_context *context)
{
    CHECK_PARAM(context);
    VPU_EncClose(context->vpu_enc_info->handle);

    if (context->vpu_enc_info->bit_code) {
        free(context->vpu_enc_info->bit_code);
        context->vpu_enc_info->bit_code = NULL;
    }

    if (context->vpu_enc_info->bstream_buf.phys_addr) {
        vdi_free_dma_memory(context->vpu_enc_info->core_id, &(context->vpu_enc_info->bstream_buf));
        context->vpu_enc_info->bstream_buf.phys_addr = 0;
    }

    log_debug(" %s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_init
         coda device init oper
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_init(struct test_context *context)
{
    CHECK_PARAM(context);

    if (context->test_cfg->is_decode) {
        CALL_FUNCTION(sdk_test_coda_decode_init(context));
    }
    else if (context->test_cfg->is_encode) {
        CALL_FUNCTION(sdk_test_coda_encode_init(context));
    }
    else {
        log_err("coda can not know action \n");
        return RET_CODE_INVALID_PARAM;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_deinit
         coda device deinit oper
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_deinit(struct test_context *context)
{
    CHECK_PARAM(context);

    if (context->test_cfg->is_decode) {
        CALL_FUNCTION(sdk_test_coda_decode_deinit(context));
    }
    else if (context->test_cfg->is_encode) {
        CALL_FUNCTION(sdk_test_coda_encode_deinit(context));
    }
    else {
        log_err("coda can not know action \n");
        return RET_CODE_INVALID_PARAM;
    }

    log_debug("coda deinit success \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_wave_decode_param_config
         wave device decoding param cfg
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static void sdk_test_wave_decode_param_config(struct test_context *context, DecOpenParam  *open_param)
{
    TestDecConfig *param_cfg = NULL;

    param_cfg = context->test_cfg->dec_config;
    open_param->bitstreamFormat     = (CodStd)param_cfg->bitFormat;
    open_param->coreIdx             = context->test_cfg->core_id;
    open_param->bitstreamBuffer     = context->vpu_dec_info->bstream_buf[0].phys_addr;
    open_param->bitstreamBufferSize = context->vpu_dec_info->bstream_buf[0].size ;
    open_param->bitstreamMode       = param_cfg->bitstreamMode;
    open_param->wtlEnable           = param_cfg->enableWTL;
    open_param->pvricFbcEnable      = param_cfg->pvricFbcEnable;
    open_param->pvric31HwSupport    = param_cfg->pvric31HwSupport;
    open_param->wtlMode             = param_cfg->wtlMode;
    open_param->cbcrInterleave      = param_cfg->cbcrInterleave;
    open_param->nv21                = param_cfg->nv21;
    open_param->streamEndian        = param_cfg->streamEndian;
    open_param->frameEndian         = param_cfg->frameEndian;
    open_param->fbc_mode            = param_cfg->wave4.fbcMode;
    open_param->bwOptimization      = param_cfg->wave4.bwOptimization;

    log_debug("------------------------------ DECODER OPTIONS ------------------------------\n");
    log_debug("[coreIdx            ]: %d\n", open_param->coreIdx            );
    log_debug("[CODEC              ]: %d\n", open_param->bitstreamFormat    );
    log_debug("[tiled2linear       ]: %d\n", param_cfg->coda9.enableTiled2Linear);
    log_debug("[wtlEnable          ]: %d\n", open_param->wtlEnable          );
    log_debug("[wtlMode            ]: %d\n", open_param->wtlMode            );
    log_debug("[bitstreamBuffer    ]: 0x%x\n", open_param->bitstreamBuffer );
    log_debug("[bitstreamBufferSize]: %d\n", open_param->bitstreamBufferSize);
    log_debug("[bitstreamMode      ]: %d\n", open_param->bitstreamMode      );
    log_debug("[cbcrInterleave     ]: %d\n", open_param->cbcrInterleave     );
    log_debug("[nv21               ]: %d\n", open_param->nv21               );
    log_debug("[streamEndian       ]: %d\n", open_param->streamEndian       );
    log_debug("[frameEndian        ]: %d\n", open_param->frameEndian        );
    log_debug("[FBC                ]: %x\n", open_param->fbc_mode           );
    log_debug("[BWOPT              ]: %d\n", open_param->bwOptimization     );
    log_debug("[PVRIC              ]: %d\n", open_param->pvricFbcEnable     );
    log_debug("[PVRIC31HW          ]: %d\n", open_param->pvric31HwSupport   );
    log_debug("-----------------------------------------------------------------------------\n");
}

/*
 * brief sdk_test_wave_decode_init
         wave device inition flow oper
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_init(struct test_context *context)
{
    int ret = 0;
    uint32_t index = 0;
    uint32_t product_id = 0;
    uint32_t core_id = 0;
    TestDecConfig *param_cfg = NULL;

    CHECK_PARAM(context);
    param_cfg = context->test_cfg->dec_config;
    core_id  = context->test_cfg->core_id;
    context->vpu_dec_info->core_id = core_id;
    context->vpu_dec_info->bstream_num = MAX_BUF_NUM;

    /* Check parameters */
    if (param_cfg->bitFormat != STD_HEVC && param_cfg->bitFormat != STD_VP9) {
        log_err("not supported video standard (%d)\n", param_cfg->bitFormat);
        return RET_CODE_INVALID_PARAM;
    }

    if (param_cfg->pvricFbcEnable == TRUE
            && (param_cfg->cbcrInterleave == FALSE
                || (param_cfg->cbcrInterleave == TRUE
                    && param_cfg->nv21 == TRUE))) {
        log_err("only NV12 can be support on PVRIC fbc mode\n");
        return RET_CODE_INVALID_PARAM;
    }

    if (param_cfg->pvricFbcEnable == TRUE && param_cfg->enableWTL == FALSE) {
        log_err("enableWTL should be always 1 when PVRIC fbc enabled.\n");
        return RET_CODE_INVALID_PARAM;
    }

    if ((product_id = (ProductId)VPU_GetProductId(core_id)) != PRODUCT_ID_412) {
        log_err("failed to get product ID\n");
        return RET_CODE_FAILURE_PRODUCT_ID;
    }

    if (LoadFirmware(product_id, (Uint8 **) & (context->vpu_dec_info->bit_code), \
                     & (context->vpu_dec_info->bit_size), WAVE_FW_PATH) < 0) {
        log_err("failed to load firmware: %s\n", WAVE_FW_PATH);
        return RET_CODE_FAILURE_PRODUCT_ID;
    }

    ret = VPU_InitWithBitcode(core_id, (const uint16_t *)(context->vpu_dec_info->bit_code), (context->vpu_dec_info->bit_size));

    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
        log_err("failed to boot up VPU(core_id: %d, product_id: %d, ret:%d)\n", core_id, product_id, ret);
        return RET_CODE_FAILURE_LOAD_FW;
    }

    PrintVpuVersionInfo(core_id);

    for (index = 0; index < context->vpu_dec_info->bstream_num; index++) {
        context->vpu_dec_info->bstream_buf[index].size = STREAM_BUF_SIZE;

        if (vdi_allocate_dma_memory(core_id, &(context->vpu_dec_info->bstream_buf[index])) < 0) {
            log_err("fail to allocate bitstream buffer\n" );
            return RET_CODE_FAILURE_ALLOC;
        }
    }

    /* set up decoder configurations */
    sdk_test_wave_decode_param_config(context, &(context->vpu_dec_info->open_param));

    if ((ret = VPU_DecOpen(&(context->vpu_dec_info->handle), &(context->vpu_dec_info->open_param))) != RETCODE_SUCCESS) {
        log_err("VPU_DecOpen failed error code is 0x%x \n", ret );
        goto err_open;
    }

    return RET_CODE_SUCCESS;

err_open:

    for (index = 0; index < context->vpu_dec_info->bstream_num; index++) {
        if (context->vpu_dec_info->bstream_buf[index].phys_addr) {
            vdi_free_dma_memory(core_id, &(context->vpu_dec_info->bstream_buf[index]));
        }
    }

    if (context->vpu_dec_info->bit_code) {
        free(context->vpu_dec_info->bit_code);
        context->vpu_dec_info->bit_code = NULL;
    }

    return RET_CODE_FAILURE_DEVICE_OPEN;
}

/*
 * brief sdk_test_wave_deinit
         wave device close and free bitstreams memory
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_deinit(struct test_context *context)
{
    int ret = 0;
    uint32_t index = 0;
    CHECK_PARAM(context);

    ret = VPU_DecClose(context->vpu_dec_info->handle);

    for (index = 0; index < context->vpu_dec_info->bstream_num; index++) {
        if (context->vpu_dec_info->bstream_buf[index].phys_addr) {
            vdi_free_dma_memory(context->vpu_dec_info->core_id, &(context->vpu_dec_info->bstream_buf[index]));
        }
    }

    if (context->vpu_dec_info->bit_code) {
        free(context->vpu_dec_info->bit_code);
        context->vpu_dec_info->bit_code = NULL;
    }

    log_debug(" %s : %d \n", __FUNCTION__, __LINE__);
    return ret;
}


/*
 * brief sdk_test_wave_init
         wave device open and alloc bitstreams memory
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_init(struct test_context *context)
{
    CHECK_PARAM(context);

    if (context->test_cfg->is_decode) {
        CALL_FUNCTION(sdk_test_wave_decode_init(context));
    }
    else {
        log_err("wave can decode only \n");
        return RET_CODE_INVALID_PARAM;
    }

    log_debug(" %s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_wave_deinit
         wave device close and free bitstreams memory
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_deinit(struct test_context *context)
{
    CHECK_PARAM(context);

    if (context->test_cfg->is_decode) {
        CALL_FUNCTION(sdk_test_wave_decode_deinit(context));
    }
    else {
        log_err("wave can decode only \n");
        return RET_CODE_INVALID_PARAM;
    }

    log_debug(" %s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_decode_feeder_create
         create feeder for code decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_feeder_create(struct test_context *context)
{

    uint32_t size = 0;
    uint32_t bit_format = 0;
    char *input_path = NULL;
    uint64_t phys_addr = 0;

    input_path = context->test_cfg->dec_config->inputPath;
    phys_addr = context->vpu_dec_info->bstream_buf[0].phys_addr;
    size = context->vpu_dec_info->bstream_buf[0].size;
    bit_format = context->test_cfg->dec_config->bitFormat;

    if (context->vpu_dec_info->bstream_buf[0].phys_addr) {
        CodStd codec_id = 0;
        uint32_t mp4_id = 0;
        context->vpu_dec_info->feeder = BitstreamFeeder_Create(input_path, FEEDING_METHOD_FRAME_SIZE, \
                                        phys_addr, size, bit_format, \
                                        &codec_id, &mp4_id, NULL, NULL);

        if (!(context->vpu_dec_info->feeder)) {
            log_err("create bitstream feeder failure \n");
            return RET_CODE_FAILURE_FEEDER;
        }

        BitstreamFeeder_SetFillMode(context->vpu_dec_info->feeder, BSF_FILLING_LINEBUFFER);
        context->test_cfg->dec_config->bitFormat = codec_id;
        context->test_cfg->dec_config->coda9.mp4class = mp4_id;
        log_debug("create bitstream feeder info:  base(%d) phy(%p) size(%d) format(%d) \n",
                  context->vpu_dec_info->bstream_buf[0].base,
                  context->vpu_dec_info->bstream_buf[0].phys_addr,
                  context->vpu_dec_info->bstream_buf[0].size,
                  context->test_cfg->dec_config->bitFormat);
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_code_decode_sequence_init
         sequence_inition for code decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_code_decode_sequence_init(struct test_context *context)
{
    int size = 0;
    int ret = 0;
    int interrupt_flag = 0;
    uint32_t time_counts = 0;
    BOOL seq_inited = FALSE;

    size = BitstreamFeeder_Act(context->vpu_dec_info->handle, context->vpu_dec_info->feeder, \
                               context->test_cfg->dec_config->streamEndian);

    if (size < 0) {
        log_err("feeder data error \n");
        return RET_CODE_FAILURE_FEED_STREAM;
    }

    if (context->vpu_dec_info->open_param.bitstreamFormat != STD_THO
            && context->vpu_dec_info->open_param.bitstreamFormat != STD_H263
            && context->vpu_dec_info->open_param.bitstreamFormat != STD_RV) {
        /**
         * Need first picture. In case of Theora,
         * ffmpeg returns a coded frame including sequence
         * header.
         **/
        size = BitstreamFeeder_Act(context->vpu_dec_info->handle, context->vpu_dec_info->feeder, \
                                   context->test_cfg->dec_config->streamEndian);

        if (size < 0) {
            log_err("feeder data error \n");
            return RET_CODE_FAILURE_FEED_STREAM;
        }
    }

    if ((ret = VPU_DecSetEscSeqInit(context->vpu_dec_info->handle, FALSE)) != RETCODE_SUCCESS) {
        log_debug("Wanning! can not to set seqInitEscape in the current bitstream mode Option \n");
    }

    if ((ret = VPU_DecIssueSeqInit(context->vpu_dec_info->handle)) != RETCODE_SUCCESS) {
        log_err("issue seq init failed Error code is 0x%x \n", ret);
        return ret;
    }

    while (!seq_inited) {
        interrupt_flag = VPU_WaitInterrupt(context->vpu_dec_info->core_id, VPU_WAIT_TIME_OUT);      //wait for 10ms to save stream filling time.
        log_debug("coda issue seq intflag %x\n", interrupt_flag);

        if (interrupt_flag == -1) {
            if (time_counts * VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                VPU_SWReset(context->vpu_dec_info->core_id, SW_RESET_SAFETY, context->vpu_dec_info->handle);
                log_err("coda interrupt wait timeout\n");
                return RET_CODE_FAILURE_SEQINIT;
            }

            log_debug("waring coda issue seqinit time %d\n", time_counts);
            time_counts++;
            interrupt_flag = 0;
        }

        if (interrupt_flag) {
            VPU_ClearInterrupt(context->vpu_dec_info->core_id);

            if (interrupt_flag & (1 << INT_BIT_SEQ_INIT)) {
                log_debug("VPU_ClearInterrupt\n");
                seq_inited = TRUE;
                break;
            }
        }

    }

    log_debug("coda start decode seqinit succeess\n");

    if ((ret = VPU_DecCompleteSeqInit(context->vpu_dec_info->handle, &(context->vpu_dec_info->sequence_info))) != RETCODE_SUCCESS) {
        log_err("coda failed to SEQ_INIT(ERROR REASON: %d(0x%x)\n",
                context->vpu_dec_info->sequence_info.seqInitErrReason,
                context->vpu_dec_info->sequence_info.seqInitErrReason);
        return ret;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_decode_ppu_enable -
          post processor  operation for code decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_ppu_enable(struct test_context *context)
{
    int32_t index = 0;
    uint32_t core_id = 0;
    int ret = 0;
    int32_t ppu_fb_count  = PPU_FB_COUNT;
    BOOL enable_ppu = FALSE;
    TestDecConfig *param = NULL;
    vpu_buffer_t *pvb = NULL;
    DecOpenParam  decopen_param = {0};
    DecInitialInfo sequence_info = {0};
    FrameBuffer ppu_frame[MAX_REG_FRAME] = {{0}};
    FrameBufferAllocInfo fballoc_info = {0};

    CHECK_PARAM(context);
    param = context->test_cfg->dec_config;
    decopen_param = context->vpu_dec_info->open_param;
    core_id = context->vpu_dec_info->core_id;
    enable_ppu = (BOOL)(param->coda9.rotate > 0
                        || param->coda9.mirror > 0
                        || decopen_param.tiled2LinearEnable == TRUE
                        || param->coda9.enableDering == TRUE);
    context->vpu_dec_info->ppu_enable = enable_ppu;
    context->vpu_dec_info->wait_post_processing = enable_ppu;

    if (enable_ppu == TRUE) {
        uint32_t stride_ppu = 0;
        uint32_t size_ppu_fb = 0;
        uint32_t rotate = param->coda9.rotate;
        uint32_t rotate_width  = sequence_info.picWidth;
        uint32_t rotate_height = sequence_info.picHeight;

        if (rotate == 90 || rotate == 270) {
            rotate_width  = sequence_info.picHeight;
            rotate_height = sequence_info.picWidth;
        }

        rotate_width  = VPU_ALIGN32(rotate_width);
        rotate_height = VPU_ALIGN32(rotate_height);
        stride_ppu = CalcStride(rotate_width, rotate_height, FORMAT_420, decopen_param.cbcrInterleave, LINEAR_FRAME_MAP, FALSE);
        size_ppu_fb = VPU_GetFrameBufSize(core_id, stride_ppu, rotate_height, LINEAR_FRAME_MAP, FORMAT_420, decopen_param.cbcrInterleave, NULL);

        for (index = 0; index < ppu_fb_count; index++) {
            pvb = &(context->vpu_dec_info->ppu_fb_mem[index]);
            pvb->size = size_ppu_fb;

            if (vdi_allocate_dma_memory(core_id, pvb) < 0) {
                log_err("%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                goto err_ppu;
            }

            ppu_frame[index].bufY  = pvb->phys_addr;
            ppu_frame[index].bufCb = -1;
            ppu_frame[index].bufCr = -1;
            ppu_frame[index].updateFbInfo = TRUE;
        }

        fballoc_info.mapType        = LINEAR_FRAME_MAP;
        fballoc_info.cbcrInterleave = decopen_param.cbcrInterleave;
        fballoc_info.nv21           = decopen_param.nv21;
        fballoc_info.format         = FORMAT_420;
        fballoc_info.stride         = stride_ppu;
        fballoc_info.height         = rotate_height;
        fballoc_info.endian         = decopen_param.frameEndian;
        fballoc_info.lumaBitDepth   = 8;
        fballoc_info.chromaBitDepth = 8;
        fballoc_info.num            = ppu_fb_count;
        fballoc_info.type           = FB_TYPE_PPU;

        if (RETCODE_SUCCESS != (ret = VPU_DecAllocateFrameBuffer(context->vpu_dec_info->handle, fballoc_info, ppu_frame))) {
            log_err("%s:%d failed to VPU_DecAllocateFrameBuffer() ret(%d)\n", ret);
            goto err_ppu;
        }

        /* please keep the below call sequence. */
        VPU_DecGiveCommand(context->vpu_dec_info->handle, SET_ROTATION_ANGLE, (void *)&param->coda9.rotate);
        VPU_DecGiveCommand(context->vpu_dec_info->handle, SET_MIRROR_DIRECTION, (void *)&param->coda9.mirror);
        VPU_DecGiveCommand(context->vpu_dec_info->handle, SET_ROTATOR_STRIDE, (void *)&stride_ppu);

        if ((context->vpu_dec_info->ppu_queue = Queue_Create(MAX_REG_FRAME, sizeof(FrameBuffer))) == NULL) {
            goto err_ppu;
        }

        for (index = 0; index < ppu_fb_count; index++) {
            Queue_Enqueue(context->vpu_dec_info->ppu_queue, (void *)&ppu_frame[index]);
        }
    }

    log_debug("coda ppu enable success enable_ppu(%d) \n", enable_ppu);
    return ret;

err_ppu:

    for (index = 0; index < MAX_REG_FRAME; index++) {
        if (context->vpu_dec_info->ppu_fb_mem[index].size > 0)
            vdi_free_dma_memory(core_id, &(context->vpu_dec_info->ppu_fb_mem[index]));
    }

    return RET_CODE_FAILURE_PPU;
}

/*
 * brief sdk_test_code_decode_buffer_register -
          alloc and register framebuffer for code decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_code_decode_buffer_register(struct test_context *context)
{
    int ret = 0;
    uint32_t framebuf_height = 0;
    uint32_t framebuf_stride = 0;
    uint32_t framebuf_size = 0;
    uint32_t fb_count = 0;
    uint32_t index = 0;
    uint32_t core_id = 0;
    FrameBufferAllocInfo fballoc_info = {0};
    DecInitialInfo sequence_info = {0};
    DecOpenParam  decopen_param = {0};
    TestDecConfig  *param = NULL;
    vpu_buffer_t *pvb = NULL;
    FrameBuffer pframe[MAX_REG_FRAME] = {{0}};
    MaverickCacheConfig cacheCfg = {};

    CHECK_PARAM(context);
    sequence_info = context->vpu_dec_info->sequence_info;
    decopen_param = context->vpu_dec_info->open_param;
    param = context->test_cfg->dec_config;
    core_id = context->vpu_dec_info->core_id;

    framebuf_height = VPU_ALIGN32(sequence_info.picHeight);
    framebuf_stride = CalcStride(sequence_info.picWidth, sequence_info.picHeight, FORMAT_420, decopen_param.cbcrInterleave, param->mapType, FALSE);
    framebuf_size = VPU_GetFrameBufSize(core_id, framebuf_stride, framebuf_height, param->mapType, FORMAT_420, decopen_param.cbcrInterleave, NULL);
    fb_count = sequence_info.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;
    log_debug("minFrameBufferCount(%d), stride(%d), height(%d), maptype(%d)\n",
              sequence_info.minFrameBufferCount,
              framebuf_stride, framebuf_height,
              param->mapType);

    osal_memset((void *)&fballoc_info, 0x00, sizeof(fballoc_info));
    osal_memset((void *)pframe, 0x00, sizeof(FrameBuffer)*fb_count);
    fballoc_info.format          = FORMAT_420;
    fballoc_info.cbcrInterleave  = decopen_param.cbcrInterleave;
    fballoc_info.mapType         = param->mapType;
    fballoc_info.stride          = framebuf_stride;
    fballoc_info.height          = framebuf_height;
    fballoc_info.lumaBitDepth    = sequence_info.lumaBitdepth;
    fballoc_info.chromaBitDepth  = sequence_info.chromaBitdepth;
    fballoc_info.num             = fb_count;
    fballoc_info.endian          = decopen_param.frameEndian;
    fballoc_info.type            = FB_TYPE_CODEC;
    log_debug("framebuffer count (%d)\n", fb_count);

    for (index = 0; index < fb_count; index++) {
        pvb = &(context->vpu_dec_info->pfb_mem[index]);
        pvb->size = framebuf_size;

        if (vdi_allocate_dma_memory(core_id, pvb) < 0) {
            log_debug("%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
            goto error_register;
        }

        pframe[index].bufY  = pvb->phys_addr;
        pframe[index].bufCb = -1;
        pframe[index].bufCr = -1;
        pframe[index].updateFbInfo = TRUE;
    }

    if (RETCODE_SUCCESS != (ret = VPU_DecAllocateFrameBuffer(context->vpu_dec_info->handle, fballoc_info, pframe))) {
        log_err("failed to VPU_DecAllocateFrameBuffer(), ret(%d)\n", ret);
        goto error_register;
    }

    /* alloc wtl buffer */
    if (decopen_param.wtlEnable == TRUE) {
        TiledMapType linearMapType = decopen_param.wtlMode == FF_FRAME ? LINEAR_FRAME_MAP : LINEAR_FIELD_MAP;
        uint32_t     strideWtl;

        strideWtl    = CalcStride(sequence_info.picWidth, sequence_info.picHeight, FORMAT_420, decopen_param.cbcrInterleave, linearMapType, FALSE);
        framebuf_size = VPU_GetFrameBufSize(core_id, strideWtl, framebuf_height, linearMapType, FORMAT_420, decopen_param.cbcrInterleave, NULL);
        log_debug("wtl enable framebuf_size(%d)\n)", framebuf_size);

        for (index = fb_count; index < fb_count * 2; index++) {
            pvb       = &(context->vpu_dec_info->pfb_mem[index]);
            pvb->size = framebuf_size;

            if (vdi_allocate_dma_memory(core_id, pvb) < 0) {
                log_err("%s:%d fail to allocate frame buffer\n");
                goto error_register;
            }

            pframe[index].bufY  = pvb->phys_addr;
            pframe[index].bufCb = -1;
            pframe[index].bufCr = -1;
            pframe[index].updateFbInfo = TRUE;
        }

        fballoc_info.mapType        = linearMapType;
        fballoc_info.cbcrInterleave = decopen_param.cbcrInterleave;
        fballoc_info.nv21           = decopen_param.nv21;
        fballoc_info.format         = FORMAT_420;
        fballoc_info.stride         = strideWtl;
        fballoc_info.height         = framebuf_height;
        fballoc_info.endian         = decopen_param.frameEndian;
        fballoc_info.lumaBitDepth   = 8;
        fballoc_info.chromaBitDepth = 8;
        fballoc_info.num            = fb_count;
        fballoc_info.type           = FB_TYPE_CODEC;
        ret = VPU_DecAllocateFrameBuffer(context->vpu_dec_info->handle, fballoc_info, &pframe[fb_count]);

        if (ret != RETCODE_SUCCESS) {
            log_err("failed to VPU_DecAllocateFrameBuffer() ret(%d)\n", ret);
            goto error_register;
        }

        log_debug("fb_count(%d)\n", fb_count);
    }

    /*set sram buffer for sec AXI  */
    SecAxiUse secAxiUse = {0};
    secAxiUse.u.coda9.useBitEnable = (param->secondaryAXI >> 0) & 0x01;
    secAxiUse.u.coda9.useIpEnable = (param->secondaryAXI >> 1) & 0x01;
    secAxiUse.u.coda9.useDbkYEnable = (param->secondaryAXI >> 2) & 0x01;
    secAxiUse.u.coda9.useDbkCEnable = (param->secondaryAXI >> 3) & 0x01;
    secAxiUse.u.coda9.useOvlEnable = (param->secondaryAXI >> 4) & 0x01;
    secAxiUse.u.coda9.useBtpEnable = (param->secondaryAXI >> 5) & 0x01;
    VPU_DecGiveCommand(context->vpu_dec_info->handle, SET_SEC_AXI, &secAxiUse);
    log_debug("useBitEnable \n", secAxiUse.u.coda9.useBitEnable);
    log_debug("useIpEnable(%d)\n", secAxiUse.u.coda9.useIpEnable);
    log_debug("useDbkYEnable(%d)\n", secAxiUse.u.coda9.useDbkYEnable);
    log_debug("useDbkCEnable(%d)\n", secAxiUse.u.coda9.useDbkCEnable);
    log_debug("useOvlEnable(%d)\n", secAxiUse.u.coda9.useOvlEnable);
    log_debug("useBtpEnable(%d)\n", secAxiUse.u.coda9.useBtpEnable);

    MaverickCache2Config(&cacheCfg, TRUE /*Decoder*/, param->cbcrInterleave,
                         param->coda9.frameCacheBypass,
                         param->coda9.frameCacheBurst,
                         param->coda9.frameCacheMerge,
                         param->mapType,
                         param->coda9.frameCacheWayShape);
    VPU_DecGiveCommand(context->vpu_dec_info->handle, SET_CACHE_CONFIG, &cacheCfg);

    framebuf_stride = CalcStride(sequence_info.picWidth, sequence_info.picHeight, FORMAT_420, decopen_param.cbcrInterleave,
                                 decopen_param.wtlEnable == TRUE ? LINEAR_FRAME_MAP : param->mapType, FALSE);
    log_debug("framebuf_stride(%d)\n", framebuf_stride);
    ret = VPU_DecRegisterFrameBuffer(context->vpu_dec_info->handle, pframe, fb_count, framebuf_stride, framebuf_height, (int)param->mapType);

    if (RETCODE_SUCCESS != ret) {
        if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
            PrintMemoryAccessViolationReason(core_id, NULL);

        log_err("VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto error_register;
    }

    /*
     *ALLOCATE PPU FRAMEBUFFERS when rotator, mirror or tiled2linear are enabled.
     *NOTE: VPU_DecAllocateFrameBuffer() WITH PPU FRAMEBUFFER SHOULD BE CALLED
     *      AFTER VPU_DecRegisterFrameBuffer()
     */
    if (RET_CODE_SUCCESS != (ret = sdk_test_coda_decode_ppu_enable(context))) {
        log_err("ppu enable failure\n");
        goto error_register;
    }

    return RET_CODE_SUCCESS;

error_register:

    for (index = 0; index < MAX_REG_FRAME; index++) {
        if ((context->vpu_dec_info->pfb_mem[index]).size > 0)
            vdi_free_dma_memory(core_id, &(context->vpu_dec_info->pfb_mem[index]));
    }

    return RET_CODE_FAILURE_FRAMEBUFFER_REGISTER;
}

/*
 * brief sdk_test_coda_decode_framebuffer_register -
          sequence initiont and register framebuffer for code decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_framebuffer_register(struct test_context *context)
{
    CHECK_PARAM(context);
    CALL_FUNCTION(sdk_test_code_decode_sequence_init(context));
    log_debug("decode init sequence success \n");
    CALL_FUNCTION(sdk_test_code_decode_buffer_register(context));

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_decode_preparative_create -
         create feeder, alloc and register framebuffer for code decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_preparative_create(struct test_context *context)
{
    int ret = 0;

    CHECK_PARAM(context);
    CALL_FUNCTION(sdk_test_coda_decode_feeder_create(context));

    if (RET_CODE_SUCCESS != (ret = sdk_test_coda_decode_framebuffer_register(context))) {
        sdk_test_coda_decode_feeder_destory(context);
        log_err("coda decode preparative framebuffer register failure ret(%d)\n", ret);
        return ret;
    }

    return ret;
}

/*
 * brief sdk_test_vpu_encode_sequence_header_build -
         encode header and save to file
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_vpu_encode_sequence_header_build(struct test_context *context)
{
    int ret = 0;
    int i = 0;
    uint32_t core_id = 0;
    EncHandle handle = NULL;
    EncHeaderParam enc_header_param = {0};

    handle = context->vpu_enc_info->handle;
    core_id = context->vpu_enc_info->core_id;
    context->vpu_enc_info->enc_param.forceIPicture = 0;
    context->vpu_enc_info->enc_param.skipPicture   = 0;
    context->vpu_enc_info->enc_param.quantParam  = context->test_cfg->enc_config->picQpY;
    enc_header_param.zeroPaddingEnable = 0;

    if (context->vpu_enc_info->enc_open_param.ringBufferEnable == FALSE) {
        enc_header_param.buf =  context->vpu_enc_info->bstream_buf.phys_addr;
        enc_header_param.size = context->vpu_enc_info->bstream_buf.size;
    }

    if (context->vpu_enc_info->enc_open_param.bitstreamFormat == STD_MPEG4) {
        enc_header_param.headerType = 0;
        enc_header_param.size       = context->vpu_enc_info->bstream_buf.size;
        VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &enc_header_param);

        if (context->vpu_enc_info->enc_open_param.ringBufferEnable == FALSE) {
            EnterLock(core_id);
            ret = BitstreamReader_Act(context->vpu_enc_info->bs_reader, enc_header_param.buf, context->vpu_enc_info->enc_open_param.bitstreamBufferSize, 0, NULL);
            LeaveLock(core_id);

            if (ret == FALSE) {
                return RET_CODE_FAILURE_READER;
            }
        }

#ifdef MP4_ENC_VOL_HEADER
        enc_header_param.headerType = VOL_HEADER;
        enc_header_param.size       = context->vpu_enc_info->bstream_buf.size;

        if ((ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &enc_header_param)) != RETCODE_SUCCESS) {
            log_err("VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOL_HEADER failed Error code is 0x%x \n", ret);
            return RET_CODE_FAILURE_ENCODE_SEQINIT;
        }

        if (context->vpu_enc_info->enc_open_param.ringBufferEnable == FALSE) {
            EnterLock(core_id);
            ret = BitstreamReader_Act(context->vpu_enc_info->bs_reader, enc_header_param.buf, context->vpu_enc_info->enc_open_param.bitstreamBufferSize, 0, comparator);
            LeaveLock(core_id);

            if (ret == FALSE) {
                return RET_CODE_FAILURE_READER;
            }
        }

#endif
    }
    else if (context->vpu_enc_info->enc_open_param.bitstreamFormat == STD_AVC) {
        enc_header_param.headerType = SPS_RBSP;

        if (context->vpu_enc_info->enc_open_param.ringBufferEnable == FALSE) {
            enc_header_param.buf  = context->vpu_enc_info->bstream_buf.phys_addr;
        }

        enc_header_param.size = context->vpu_enc_info->bstream_buf.size;

        if ((ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &enc_header_param)) != RETCODE_SUCCESS) {
            log_err("VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for SPS_RBSP failed Error code is 0x%x \n", ret);
            return RET_CODE_FAILURE_ENCODE_SEQINIT;
        }

        if (context->vpu_enc_info->enc_open_param.ringBufferEnable == FALSE) {
            EnterLock(core_id);
            ret = BitstreamReader_Act(context->vpu_enc_info->bs_reader, enc_header_param.buf, enc_header_param.size, 0, NULL);
            log_debug("%s %d \n", __FUNCTION__, __LINE__);
            LeaveLock(core_id);

            if (ret == FALSE) {
                return RET_CODE_FAILURE_READER;
            }
        }

        enc_header_param.headerType = PPS_RBSP;
        log_debug("ENC_PUT_VIDEO_HEADER, ppsNum %d, header %d\n", context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.ppsNum, enc_header_param.size);

        for (i = 0; i < context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.ppsNum; i++) {
            if (context->vpu_enc_info->enc_open_param.ringBufferEnable == FALSE) {
                enc_header_param.buf  = context->vpu_enc_info->bstream_buf.phys_addr;
                enc_header_param.pBuf = (BYTE *)context->vpu_enc_info->bstream_buf.virt_addr;
            }

            enc_header_param.size = context->vpu_enc_info->bstream_buf.size;
            ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &enc_header_param);

            if (ret != RETCODE_SUCCESS) {
                log_err("VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for PPS_RBSP failed Error code is 0x%x \n", ret);
                return RET_CODE_FAILURE_ENCODE_SEQINIT;
            }

            if (context->vpu_enc_info->enc_open_param.ringBufferEnable == FALSE) {
                EnterLock(core_id);
                ret = BitstreamReader_Act(context->vpu_enc_info->bs_reader, enc_header_param.buf, enc_header_param.size, 0, NULL);
                LeaveLock(core_id);

                if (ret == FALSE) {
                    return RET_CODE_FAILURE_READER;
                }
            }
        }
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_vpu_encode_feeder_create -
         create yuv feeder
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_vpu_encode_feeder_create(struct test_context *context)
{
    char yuv_path[MAX_FILE_PATH] = {0};

    sprintf(yuv_path, "%s/%s", context->test_cfg->enc_config->yuvSourceBaseDir, context->test_cfg->enc_config->yuvFileName);
    ChangePathStyle(yuv_path);
    log_debug("yuv feeder file path(%s)\n", yuv_path);

    if ((context->vpu_enc_info->yuv_feeder = YuvFeeder_Create(SOURCE_YUV, yuv_path, FALSE, context->vpu_enc_info->enc_open_param.picWidth, context->vpu_enc_info->enc_open_param.picHeight, context->vpu_enc_info->enc_open_param.cbcrInterleave, TRUE)) == NULL) {
        log_err("failed to YuvFeeder_Create\n");
        return RET_CODE_FAILURE_FEEDER;
    }

    DisplayEncodedInformation(context->vpu_enc_info->handle, context->vpu_enc_info->enc_open_param.bitstreamFormat, 0, NULL);
    VPU_EncGiveCommand(context->vpu_enc_info->handle, GET_TILEDMAP_CONFIG, &(context->vpu_enc_info->map_config));
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_encode_buffer_register -
         alloc and register framebuffer
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_encode_buffer_register(struct test_context *context)
{
    int size_fb = 0;
    int i = 0;
    int ret = 0;
    uint32_t core_id = 0;
    int32_t framebuf_stride = 0, framebuf_width = 0, framebuf_height = 0;
    EncHandle handle = NULL;
    FrameBuffer fb_recon[MAX_REG_FRAME] = {{0}};
    FrameBufferAllocInfo fb_allocInfo = {0};
    FrameBufferFormat srcframe_format = FORMAT_420;
    FrameBufferFormat framebuf_format = FORMAT_420;
    TiledMapType  map_type = 0;

    core_id = context->vpu_enc_info->core_id;
    handle = context->vpu_enc_info->handle;
    map_type = (TiledMapType)context->test_cfg->enc_config->mapType;

    if (context->vpu_enc_info->enc_open_param.bitstreamFormat == STD_AVC) {
        srcframe_format = context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.chromaFormat400 ? FORMAT_400 : FORMAT_420;
        framebuf_format = context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.chromaFormat400 ? FORMAT_400 : FORMAT_420;
    }
    else {
        srcframe_format = FORMAT_420;
        framebuf_format = FORMAT_420;
    }

    framebuf_width    = (context->test_cfg->enc_config->rotAngle == 90 || context->test_cfg->enc_config->rotAngle == 270) ? context->vpu_enc_info->enc_open_param.picHeight : context->vpu_enc_info->enc_open_param.picWidth;
    framebuf_height   = (context->test_cfg->enc_config->rotAngle == 90 || context->test_cfg->enc_config->rotAngle == 270) ? context->vpu_enc_info->enc_open_param.picWidth : context->vpu_enc_info->enc_open_param.picHeight;
    framebuf_width    = VPU_ALIGN16(framebuf_width);
    framebuf_height   = VPU_ALIGN32(framebuf_height); // To cover interlaced picture
    framebuf_stride   = CalcStride(framebuf_width, framebuf_height, framebuf_format, \
                                   context->vpu_enc_info->enc_open_param.cbcrInterleave, map_type, FALSE);

    context->vpu_enc_info->regframe_buffer_count =  context->vpu_enc_info->enc_initinfo.minFrameBufferCount;
    osal_memset((void *)&fb_allocInfo, 0x00, sizeof(fb_allocInfo));
    osal_memset((void *)fb_recon, 0x00, sizeof(fb_recon));
    fb_allocInfo.format          = FORMAT_420;
    fb_allocInfo.cbcrInterleave  = context->vpu_enc_info->enc_open_param.cbcrInterleave;
    fb_allocInfo.mapType         = map_type;
    fb_allocInfo.stride          = framebuf_stride;
    fb_allocInfo.height          = framebuf_height;
    fb_allocInfo.lumaBitDepth    = 8;
    fb_allocInfo.chromaBitDepth  = 8;
    fb_allocInfo.num             = context->vpu_enc_info->regframe_buffer_count;
    fb_allocInfo.endian          = context->vpu_enc_info->enc_open_param.frameEndian;
    fb_allocInfo.type            = FB_TYPE_CODEC;
    size_fb = VPU_GetFrameBufSize(core_id, framebuf_stride, framebuf_height, map_type, framebuf_format, context->vpu_enc_info->enc_open_param.cbcrInterleave, NULL);

    for (i = 0; i < context->vpu_enc_info->regframe_buffer_count; i++) {
        context->vpu_enc_info->register_frame_buffer[i].size = size_fb;

        if (vdi_allocate_dma_memory(core_id, &(context->vpu_enc_info->register_frame_buffer[i])) < 0) {
            log_err("fail to allocate frame buffer\n");
            goto err_register;
        }

        fb_recon[i].bufY  = context->vpu_enc_info->register_frame_buffer[i].phys_addr;
        fb_recon[i].bufCb = (PhysicalAddress) - 1;
        fb_recon[i].bufCr = (PhysicalAddress) - 1;
        fb_recon[i].updateFbInfo = TRUE;
    }

    log_debug("VPU_EncAllocateFrameBuffer recon_cnt %d, stride %d, height %d, maptype %d, format %d, size %d\n",
              context->vpu_enc_info->regframe_buffer_count, framebuf_stride, framebuf_height, map_type, framebuf_format, size_fb);
    ret = VPU_EncAllocateFrameBuffer(handle, fb_allocInfo, fb_recon);

    if (ret != RETCODE_SUCCESS) {
        log_err("VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
        goto err_register;
    }

    ret = VPU_EncRegisterFrameBuffer(handle, fb_recon, context->vpu_enc_info->regframe_buffer_count, framebuf_width, framebuf_height, map_type);

    if ( ret != RETCODE_SUCCESS ) {
        log_err("VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret);
        goto err_register;
    }

    VPU_EncGiveCommand(handle, GET_TILEDMAP_CONFIG, &(context->vpu_enc_info->map_config));
    fb_allocInfo.format         = srcframe_format;
    fb_allocInfo.cbcrInterleave = context->vpu_enc_info->enc_open_param.cbcrInterleave;
    fb_allocInfo.nv21           = context->vpu_enc_info->enc_open_param.nv21;

    if (context->vpu_enc_info->enc_open_param.linear2TiledEnable == TRUE) {
        fb_allocInfo.mapType = LINEAR_FRAME_MAP;
        fb_allocInfo.stride  = context->vpu_enc_info->enc_open_param.picWidth;
    }
    else {
        fb_allocInfo.mapType = map_type;
        fb_allocInfo.stride  = CalcStride(context->vpu_enc_info->enc_open_param.picWidth, context->vpu_enc_info->enc_open_param.picHeight, srcframe_format, context->vpu_enc_info->enc_open_param.cbcrInterleave, map_type, FALSE);
    }

    fb_allocInfo.height  = VPU_ALIGN16(context->vpu_enc_info->enc_open_param.picHeight);
    fb_allocInfo.num     = ENC_SRC_BUF_NUM;
    fb_allocInfo.endian  = context->vpu_enc_info->enc_open_param.frameEndian;
    fb_allocInfo.type    = FB_TYPE_PPU;

    size_fb = VPU_GetFrameBufSize(core_id, fb_allocInfo.stride, fb_allocInfo.height, \
                                  fb_allocInfo.mapType, srcframe_format, context->vpu_enc_info->enc_open_param.cbcrInterleave, NULL);

    for (i = 0; i < ENCODE_SRC_BUF_NUM; i++) {
        context->vpu_enc_info->enc_src_frame_mem[i].size = size_fb;

        if (vdi_allocate_dma_memory(core_id, &(context->vpu_enc_info->enc_src_frame_mem[i])) < 0) {
            log_err("fail to allocate frame buffer\n");
            goto err_register;
        }

        context->vpu_enc_info->enc_frame_src[i].bufY  = context->vpu_enc_info->enc_src_frame_mem[i].phys_addr;
        context->vpu_enc_info->enc_frame_src[i].bufCb = (PhysicalAddress) - 1;
        context->vpu_enc_info->enc_frame_src[i].bufCr = (PhysicalAddress) - 1;
        context->vpu_enc_info->enc_frame_src[i].updateFbInfo = TRUE;
    }

    log_debug("VPU_EncAllocateFrameBuffer stride %d, height %d, maptype %d, format %d, size %d\n",
              fb_allocInfo.stride, fb_allocInfo.height, fb_allocInfo.mapType, srcframe_format, size_fb);
    ret = VPU_EncAllocateFrameBuffer(handle, fb_allocInfo, context->vpu_enc_info->enc_frame_src);

    if (ret != RETCODE_SUCCESS) {
        log_err("VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
        goto err_register;
    }

    log_debug("%s %d \n", __FUNCTION__, __LINE__);
    return ret;

err_register:

    sdk_test_vpu_preparative_destroy(context);
    return ret;
}

/*
 * brief sdk_test_coda_encode_framebuffer_register -
         register framebuffer, encode header, ctreate feeder
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_encode_framebuffer_register(struct test_context *context)
{
    int ret = 0;

    if (RET_CODE_SUCCESS != (ret = sdk_test_coda_encode_buffer_register(context))) {
        log_err("sdk test encode buffer register error ret(%d)\n", ret);
        goto err_register;
    }

    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_encode_sequence_header_build(context))) {
        log_err("sdk test encode header build error ret(%d)\n", ret);
        goto err_register;
    }

    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_encode_feeder_create(context))) {
        log_err("sdk test encode creat feeder error ret(%d)\n", ret);
        goto err_register;
    }

    return ret;

err_register:

    sdk_test_vpu_preparative_destroy(context);
    return ret;

}

/*
 * brief sdk_test_coda_encode_frame_config -
         encoding param config
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static void sdk_test_coda_encode_frame_config(struct test_context *context, BOOL filed_done)
{
    int i = 0;
    uint32_t ipic_cnt = 0;
    uint32_t min_skip_num = 0;
    TestEncConfig  *param = NULL;
    BOOL  is_idr = FALSE;
    BOOL  is_ipic = FALSE;

    param = context->test_cfg->enc_config;

    if (context->vpu_enc_info->enc_open_param.ringBufferEnable == 0) {
        context->vpu_enc_info->enc_param.picStreamBufferAddr = context->vpu_enc_info->bstream_buf.phys_addr;
        context->vpu_enc_info->enc_param.picStreamBufferSize = context->vpu_enc_info->bstream_buf.size;
    }

    context->vpu_enc_info->enc_param.sourceFrame->endian = context->vpu_enc_info->enc_open_param.frameEndian;
    context->vpu_enc_info->enc_param.sourceFrame->cbcrInterleave = context->vpu_enc_info->enc_open_param.cbcrInterleave;
    context->vpu_enc_info->enc_param.sourceFrame->sourceLBurstEn = FALSE;

    if (context->vpu_enc_info->enc_open_param.EncStdParam.avcParam.fieldFlag) {
        context->vpu_enc_info->enc_param.fieldRun = TRUE;
    }

    if (context->vpu_enc_info->frames_count == 0) {
        is_idr  = TRUE;
        is_ipic = TRUE;
    }
    else {
        if ((context->vpu_enc_info->enc_open_param.idrInterval > 0) && (context->vpu_enc_info->cfg_param.GopPicNum > 0)) {
            if ((context->vpu_enc_info->frames_count % context->vpu_enc_info->cfg_param.GopPicNum) == 0) {
                is_ipic = TRUE;
                ipic_cnt++;
            }
        }
    }

    if (context->vpu_enc_info->enc_open_param.bitstreamFormat == STD_MPEG4 ) {
        if (is_ipic == TRUE && context->vpu_enc_info->enc_open_param.idrInterval > 0) {
            if ((ipic_cnt % context->vpu_enc_info->enc_open_param.idrInterval) == 0) {
                is_idr = TRUE;
            }
        }
    }

    context->vpu_enc_info->enc_param.forceIPicture = filed_done ? FALSE : is_idr;
    context->vpu_enc_info->enc_param.skipPicture = 0;
    min_skip_num = 0;

    for (i = 0; i < MAX_PIC_SKIP_NUM; i++) {
        uint32_t num_pic_skip = context->vpu_enc_info->cfg_param.field_flag ? param->skipPicNums[i] / 2 : param->skipPicNums[i];

        if (num_pic_skip > min_skip_num && num_pic_skip == (uint32_t)context->vpu_enc_info->frames_count) {
            if (context->vpu_enc_info->cfg_param.field_flag == FALSE) {
                context->vpu_enc_info->enc_param.skipPicture = TRUE;
            }
            else {
                if (param->skipPicNums[i] % 2)
                    context->vpu_enc_info->enc_param.skipPicture = filed_done;
                else
                    context->vpu_enc_info->enc_param.skipPicture = !filed_done;

                /* check next skip field */
                if ((i + 1) < MAX_PIC_SKIP_NUM) {
                    num_pic_skip = param->skipPicNums[i + 1] / 2;

                    if (num_pic_skip == (uint32_t)context->vpu_enc_info->frames_count)
                        context->vpu_enc_info->enc_param.skipPicture = TRUE;
                }
            }

            break;
        }
    }

#ifdef SUPPORT_ROI_50
    {
        int i;
        context->vpu_enc_info->enc_param.setROI.mode = 1;
        context->vpu_enc_info->enc_param.setROI.number = 10;    // up to 50

        for (i = 0; i < context->vpu_enc_info->enc_param.setROI.number; i++) {
            context->vpu_enc_info->enc_param.setROI.region[i].bottom = 16 + i * 16;
            context->vpu_enc_info->enc_param.setROI.region[i].top = i * 16;
            context->vpu_enc_info->enc_param.setROI.region[i].right = 16 + i * 16;
            context->vpu_enc_info->enc_param.setROI.region[i].left = i * 16;

            if ((i % 2) == 0)
                context->vpu_enc_info->enc_param.setROI.qp[i] = 8;
            else
                context->vpu_enc_info->enc_param.setROI.qp[i] = 5;
        }
    }

#endif
    log_debug("[VPU] STEP 12 VPU_EncStartOneFrame\n");
    log_debug("------------------------------ ENCODER PARAM ------------------------------\n");
    log_debug("[forceIPicture                  ]: %d\n", context->vpu_enc_info->enc_param.forceIPicture);
    log_debug("[skipPicture                    ]: %d\n", context->vpu_enc_info->enc_param.skipPicture);
    log_debug("[quantParam                     ]: %d\n", context->vpu_enc_info->enc_param.quantParam);
    log_debug("[forcePicQpEnable               ]: %d\n", context->vpu_enc_info->enc_param.forcePicQpEnable);
    log_debug("[forcePicQpP                    ]: %d\n", context->vpu_enc_info->enc_param.forcePicQpP);
    log_debug("[forcePicTypeEnable             ]: %d\n", context->vpu_enc_info->enc_param.forcePicTypeEnable);
    log_debug("[forcePicType                   ]: %d\n", context->vpu_enc_info->enc_param.forcePicType);
    log_debug("[setROI.mode                    ]: %d\n", context->vpu_enc_info->enc_param.setROI.mode);
    log_debug("[setROI.num                     ]: %d\n", context->vpu_enc_info->enc_param.setROI.number);
    log_debug("[codeOption.implicitHeaderEncode]: %d\n", context->vpu_enc_info->enc_param.codeOption.implicitHeaderEncode);
    log_debug("[codeOption.encodeVCL           ]: %d\n", context->vpu_enc_info->enc_param.codeOption.encodeVCL);

}

/*
 * brief sdk_test_code_encode_one_frame_action -
         encoding one frame
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static BOOL sdk_test_code_encode_one_frame_action(struct test_context *context)
{
    int ret = 0;
    uint32_t timeout_count = 0;
    uint32_t interrupt_flag = 0;
    uint32_t core_id = 0;
    BOOL  success = TRUE;
    DecHandle handle = NULL;

    core_id = context->vpu_enc_info->core_id;
    handle = context->vpu_enc_info->handle;
    context->vpu_enc_info->performance.start_us = GetNowUs();

    if ((ret = VPU_EncStartOneFrame(handle, &(context->vpu_enc_info->enc_param))) != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
        return RET_CODE_FAILURE_ENCODE;
    }

    timeout_count = 0;

    while (timeout_count < VPU_ENC_TIMEOUT) {
        interrupt_flag = VPU_WaitInterrupt(core_id, VPU_WAIT_TIME_OUT * 10);

        if (interrupt_flag == (int32_t) -1) {
            if (timeout_count * VPU_WAIT_TIME_OUT > VPU_ENC_TIMEOUT) {
                log_err("error: encoder timeout happened\n");
                LeaveLock(core_id);
                VPU_SWReset(core_id, SW_RESET_SAFETY, handle);
                success = FALSE;
                break;
            }

            interrupt_flag = 0;
            timeout_count++;
        }

        if (success == TRUE) {
            if (context->vpu_enc_info->enc_open_param.ringBufferEnable == TRUE) {
                ret = BitstreamReader_Act(context->vpu_enc_info->bs_reader, context->vpu_enc_info->enc_open_param.bitstreamBuffer, context->vpu_enc_info->enc_open_param.bitstreamBufferSize, DEFAULT_READ_SIZE, NULL);

                if (ret == FALSE) {
                    int room;
                    PhysicalAddress rdPtr, wrPtr;
                    success = FALSE;
                    /* Flush bitstream buffer */
                    VPU_EncGetBitstreamBuffer(handle, &rdPtr, &wrPtr, &room);
                    VPU_EncUpdateBitstreamBuffer(handle, room);
                }
            }
            else {
                // Linebuffer is Full interrupt when linebuffer interrupt mode is set to 1
                if (interrupt_flag & (1 << INT_BIT_BIT_BUF_FULL)) {
                    ret = BitstreamReader_Act(context->vpu_enc_info->bs_reader, context->vpu_enc_info->enc_open_param.bitstreamBuffer, context->vpu_enc_info->enc_open_param.bitstreamBufferSize, DEFAULT_READ_SIZE, NULL);

                    if (ret == FALSE) {
                        VPU_ClearInterrupt(core_id);
                        VPU_EncGetOutputInfo(handle, &(context->vpu_enc_info->enc_output_info));
                        success = FALSE;
                        log_err("sdk test coda encode reader error\n");
                        return RET_CODE_FAILURE_FEEDER;
                    }
                }
            }
        }

        if (interrupt_flag > 0) {
            VPU_ClearInterrupt(core_id);

            if (interrupt_flag & (1 << INT_BIT_PIC_RUN)) {
                break;
            }
        }
    }

    return success;
}

/*
 * brief sdk_test_coda_encode_one_frame -
         encode one frame operation
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_encode_one_frame(struct test_context *context)
{
    int ret = 0;
    uint32_t src_framebuffer_index = 0;
    TestEncConfig  *param = NULL;
    BOOL  filed_done = FALSE;
    BOOL  success = TRUE;

    param = context->test_cfg->enc_config;
    src_framebuffer_index = (context->vpu_enc_info->frames_count % ENC_SRC_BUF_NUM);

    if (YuvFeeder_Feed(context->vpu_enc_info->yuv_feeder, context->vpu_enc_info->core_id, \
                       & (context->vpu_enc_info->enc_frame_src[src_framebuffer_index]), \
                       context->vpu_enc_info->enc_open_param.picWidth, \
                       context->vpu_enc_info->enc_open_param.picHeight, \
                       & (context->vpu_enc_info->map_config)) == FALSE) {

        log_debug("read YUV done!!!\n");
        return RET_CODE_FAILURE_FEEDER;
    }

    context->vpu_enc_info->enc_param.sourceFrame =  &(context->vpu_enc_info->enc_frame_src[src_framebuffer_index]);
    filed_done = FALSE;

FILED_ENCODE:

    sdk_test_coda_encode_frame_config(context, filed_done);
    success = sdk_test_code_encode_one_frame_action(context);
    ret = sdk_test_coda_encode_output_info_get(context);

    if (ret != RETCODE_SUCCESS) {
        log_err("failed error code is 0x%x \n", ret );
        return RET_CODE_FAILURE_OUTPUT_INFO;
    }

    /* mismatch in WaitInterrupt loop */
    if (success == FALSE) {
        log_err("sdk test coda encode error\n");
        return RET_CODE_FAILURE_ENCODE;
    }

    if (context->vpu_enc_info->enc_param.fieldRun && filed_done == FALSE) {
        filed_done = TRUE;
        goto FILED_ENCODE;
    }

    context->vpu_enc_info->frames_count++;
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_encode_output_info_get -
         get encoding data and save to file
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int  sdk_test_coda_encode_output_info_get(struct test_context *context)
{
    int ret = 0;
    uint32_t core_id = 0;
    DecHandle handle = NULL;

    handle = context->vpu_enc_info->handle;
    core_id = context->vpu_enc_info->core_id;
    ret = VPU_EncGetOutputInfo(handle, &(context->vpu_enc_info->enc_output_info));

    if (ret != RETCODE_SUCCESS) {
        log_err("failed error code is 0x%x \n", ret );
        return RET_CODE_FAILURE_OUTPUT_INFO;
    }

    context->vpu_enc_info->performance.diff_us = GetNowUs() - context->vpu_enc_info->performance.start_us;
    context->vpu_enc_info->performance.total_us += context->vpu_enc_info->performance.diff_us;

    if (context->vpu_enc_info->enc_output_info.picType == PIC_TYPE_I
            || context->vpu_enc_info->enc_output_info.picType == PIC_TYPE_P
            || context->vpu_enc_info->enc_output_info.picType == PIC_TYPE_B
            || context->vpu_enc_info->enc_output_info.picType == PIC_TYPE_IDR) {
        VLOG(TRACE, "encoding time=%.1fms\n", (double)context->vpu_enc_info->performance.diff_us / 1000);
    }

    DisplayEncodedInformation(handle, context->vpu_enc_info->enc_open_param.bitstreamFormat, context->vpu_enc_info->frames_count, &(context->vpu_enc_info->enc_output_info));

    if (context->vpu_enc_info->enc_open_param.ringBufferEnable == FALSE) {
        if (context->vpu_enc_info->enc_output_info.bitstreamWrapAround == TRUE) {
            // If LineBuffer interrupt is set to 1, it is ok to work.
            log_err("Warning!! BitStream buffer wrap-arounded. prepare more large buffer. Consumed all remained stream\n");
            EnterLock(core_id);
            ret = BitstreamReader_Act(context->vpu_enc_info->bs_reader, context->vpu_enc_info->enc_open_param.bitstreamBuffer, context->vpu_enc_info->enc_open_param.bitstreamBufferSize, 0, NULL);
            LeaveLock(core_id);

            if (ret == FALSE) {
                log_err("sdk test coda encode reader error\n");
                return RET_CODE_FAILURE_FEEDER;
            }
        }

        if (context->vpu_enc_info->enc_output_info.bitstreamSize == 0) {
            log_debug("warning bitstreamsize = 0 \n");
        }

        EnterLock(core_id);
        ret = BitstreamReader_Act(context->vpu_enc_info->bs_reader, context->vpu_enc_info->enc_open_param.bitstreamBuffer, context->vpu_enc_info->enc_open_param.bitstreamBufferSize, 0, NULL);
        LeaveLock(core_id);

        if (ret == FALSE) {
            log_err("sdk test coda encode reader error\n");
            return RET_CODE_FAILURE_FEEDER;;
        }
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_encode_sequence_init -
         create reader(reading from vpu), get init-info for encoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_encode_sequence_init(struct test_context *context)
{
    int ret = 0;
    TiledMapType map_type = {0};
    SecAxiUse secAxiUse = {0};
    EncHandle handle = NULL;
    MaverickCacheConfig enc_cache_config = {};

    CHECK_PARAM(context);
    map_type = (TiledMapType)context->test_cfg->enc_config->mapType;
    handle = context->vpu_enc_info->handle;

    // VPU_EncGiveCommand(handle, ENABLE_LOGGING, NULL);
    if ((context->vpu_enc_info->bs_reader = BitstreamReader_Create(context->vpu_enc_info->enc_open_param.ringBufferEnable, \
                                            context->test_cfg->enc_config->bitstreamFileName, \
                                            (EndianMode)context->test_cfg->enc_config->stream_endian, \
                                            & (context->vpu_enc_info->handle))) == NULL) {
        log_err("failed to BitstreamReader_Create\n");
        return RET_CODE_FAILURE_SEQINIT;
    }

    if (context->test_cfg->enc_config->useRot == TRUE) {
        VPU_EncGiveCommand(handle, ENABLE_ROTATION, NULL);
        VPU_EncGiveCommand(handle, ENABLE_MIRRORING, NULL);
        VPU_EncGiveCommand(handle, SET_ROTATION_ANGLE, (void *) & (context->test_cfg->enc_config->rotAngle));
        VPU_EncGiveCommand(handle, SET_MIRROR_DIRECTION, (void *) & (context->test_cfg->enc_config->mirDir));
    }

    secAxiUse.u.coda9.useBitEnable  = (context->test_cfg->enc_config->secondary_axi >> 0) & 0x01;
    secAxiUse.u.coda9.useIpEnable   = (context->test_cfg->enc_config->secondary_axi >> 1) & 0x01;
    secAxiUse.u.coda9.useDbkYEnable = (context->test_cfg->enc_config->secondary_axi >> 2) & 0x01;
    secAxiUse.u.coda9.useDbkCEnable = (context->test_cfg->enc_config->secondary_axi >> 3) & 0x01;
    secAxiUse.u.coda9.useBtpEnable  = (context->test_cfg->enc_config->secondary_axi >> 4) & 0x01;
    secAxiUse.u.coda9.useOvlEnable  = (context->test_cfg->enc_config->secondary_axi >> 5) & 0x01;
    VPU_EncGiveCommand(handle, SET_SEC_AXI, &secAxiUse);

    log_debug("set secondary axi info 0x%x, 0x%x, 0x%x, 0x%x\n",
              secAxiUse.u.coda9.useBitEnable, secAxiUse.u.coda9.useIpEnable, \
              secAxiUse.u.coda9.useDbkYEnable, secAxiUse.u.coda9.useDbkCEnable);

    ret = VPU_EncGetInitialInfo(handle, &(context->vpu_enc_info->enc_initinfo));

    if ( ret != RETCODE_SUCCESS ) {
        log_err("VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret );
        PrintVpuStatus(context->vpu_enc_info->core_id, PRODUCT_ID_980);

        BitstreamReader_Destroy(context->vpu_enc_info->bs_reader);
        context->vpu_enc_info->bs_reader = NULL;
        return RET_CODE_FAILURE_ENC_INIT;;
    }

    /* Note: The below values of MaverickCache configuration are best values. */
    MaverickCache2Config(
        &enc_cache_config,
        0, //encoder
        context->vpu_enc_info->enc_open_param.cbcrInterleave, // cb cr interleave
        0, /* bypass */
        0, /* burst  */
        3, /* merge mode */
        map_type,
        15 /* shape */);
    VPU_EncGiveCommand(handle, SET_CACHE_CONFIG, &enc_cache_config);

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_encode_sequence_deinit -
         destrory feeder for encoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static void sdk_test_coda_encode_sequence_deinit(struct test_context *context)
{
    if (context->vpu_enc_info->bs_reader) {
        BitstreamReader_Destroy(context->vpu_enc_info->bs_reader);
        context->vpu_enc_info->bs_reader = NULL;
    }
}

/*
 * brief sdk_test_coda_encode_preparative_create -
         sequence inition, alloc and register framebuffer for encoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_encode_preparative_create(struct test_context *context)
{
    int ret = 0;

    CALL_FUNCTION(sdk_test_coda_encode_sequence_init(context));

    if (RET_CODE_SUCCESS != (ret = sdk_test_coda_encode_framebuffer_register(context))) {
        sdk_test_coda_encode_sequence_deinit(context);
        log_err("coda encode register frame buffer error ret(%d)\n", ret);
        return ret;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_vpu_encode_procedure -
         encode operation
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_vpu_encode_procedure(struct test_context *context)
{
    int ret = 0;

    while (TRUE) {
        ret = sdk_test_coda_encode_one_frame(context);

        if (RET_CODE_SUCCESS != ret) {
            log_err("coda encode one frame error ret(%d)\n", ret);
            return ret;
        }

        if (context->vpu_enc_info->frames_count >= context->test_cfg->enc_config->outNum) {
            log_debug("coda encode success frames_count(%d)\n", context->vpu_enc_info->frames_count);
            break;
        }
    }

    return ret;
}


/*
 * brief sdk_test_coda_preparative_create -
         create feeder, alloc and register framebuffer
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_preparative_create(struct test_context *context)
{
    CHECK_PARAM(context);

    if (context->test_cfg->is_decode) {
        CALL_FUNCTION(sdk_test_coda_decode_preparative_create(context));
    }
    else if (context->test_cfg->is_encode) {
        CALL_FUNCTION(sdk_test_coda_encode_preparative_create(context));
    }
    else {
        log_err("coda preparative create failure core id(%d)\n", context->test_cfg->core_id);
        return RET_CODE_INVALID_PARAM;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_wave_decode_sequence_init -
         wave decoding header action
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_sequence_init(struct test_context *context)
{
    int size = 0;
    int ret = 0;
    int interrupt_flag = 0;
    uint32_t index = 0;
    uint32_t time_counts = 0;
    Uint8 *pbase  = NULL;

    context->vpu_dec_info->user_data.size = USERDATA_BUFFER_SIZE;

    if (vdi_allocate_dma_memory(context->vpu_dec_info->core_id, &(context->vpu_dec_info->user_data)) < 0) {
        log_err("wave failed to allocate user data memory core_id(%d)\n", context->vpu_dec_info->core_id);
        return RET_CODE_FAILURE_ALLOC;
    }

    if (NULL == (context->vpu_dec_info->wave_init_data = (char *)osal_malloc(USERDATA_BUFFER_SIZE))) {
        vdi_free_dma_memory(context->vpu_dec_info->core_id, &(context->vpu_dec_info->user_data));
        log_err("wave failed to allocate user data memory core id(%d)\n", context->vpu_dec_info->core_id);
        return RET_CODE_FAILURE_ALLOC;
    }

    pbase = (Uint8 *)context->vpu_dec_info->wave_init_data;
    VPU_DecGiveCommand(context->vpu_dec_info->handle, SET_ADDR_REP_USERDATA, (void *) & (context->vpu_dec_info->user_data.phys_addr));
    VPU_DecGiveCommand(context->vpu_dec_info->handle, SET_SIZE_REP_USERDATA, (void *) & (context->vpu_dec_info->user_data.size));
    VPU_DecGiveCommand(context->vpu_dec_info->handle, ENABLE_REP_USERDATA, (void *) & (context->test_cfg->dec_config->enableUserData));

    if (context->test_cfg->dec_config->thumbnailMode == TRUE) {
        log_err("[VPU] EXTRA OPT - enable thumbnail mode.\n");
        VPU_DecGiveCommand(context->vpu_dec_info->handle, ENABLE_DEC_THUMBNAIL_MODE, NULL);
    }

    size = BitstreamFeeder_Act(context->vpu_dec_info->handle, context->vpu_dec_info->feeder, \
                               context->test_cfg->dec_config->streamEndian);

    if (size < 0) {
        goto err_sequence;
    }

    if ((ret = VPU_DecSetEscSeqInit(context->vpu_dec_info->handle, FALSE)) != RETCODE_SUCCESS) {
        log_debug("Wanning! can not to set seqInitEscape in the current bitstream mode Option \n");
    }

    if ((ret = VPU_DecIssueSeqInit(context->vpu_dec_info->handle)) != RETCODE_SUCCESS) {
        log_err("wave issue seq init failed Error code is 0x%x \n", ret);
        goto err_sequence;;
    }

    while (TRUE) {
        interrupt_flag = VPU_WaitInterrupt(context->vpu_dec_info->core_id, VPU_WAIT_TIME_OUT * 10);    //wait for 10ms to save stream filling time.
        log_debug("wave issue seq intflag %x\n", interrupt_flag);

        if (interrupt_flag == -1) {
            if (time_counts * VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                VPU_SWReset(context->vpu_dec_info->core_id, SW_RESET_SAFETY, context->vpu_dec_info->handle);
                log_err("wave interrupt wait timeout\n");
                goto err_sequence;;

            }

            log_err("wave issue seqinit time %d\n", time_counts);
            time_counts++;
            interrupt_flag = 0;
        }

        if (interrupt_flag) {
            VPU_ClearInterrupt(context->vpu_dec_info->core_id);

            if (interrupt_flag & (1 << INT_BIT_SEQ_INIT)) {
                log_debug("VPU_ClearInterrupt\n");
                break;
            }
        }

    }

    if ((ret = VPU_DecCompleteSeqInit(context->vpu_dec_info->handle, &(context->vpu_dec_info->sequence_info))) != RETCODE_SUCCESS) {
        EnterLock(context->vpu_dec_info->core_id);
        HandleDecInitSequenceError(context->vpu_dec_info->handle, PRODUCT_ID_412,
                                   &(context->vpu_dec_info->open_param), &(context->vpu_dec_info->sequence_info), ret);
        LeaveLock(context->vpu_dec_info->core_id);
        log_err("failed to SEQ_INIT(ERROR REASON: %d(0x%x)\n",
                context->vpu_dec_info->sequence_info.seqInitErrReason,
                context->vpu_dec_info->sequence_info.seqInitErrReason);
        goto err_sequence;;
    }

    if (context->vpu_dec_info->sequence_info.userDataHeader != 0) {
        user_data_entry_t *pEntry = (user_data_entry_t *)pbase;
        log_debug("===== USER DATA(SEI OR VUI) : NUM(%d) =====\n", context->vpu_dec_info->sequence_info.userDataNum);
        VpuReadMem(context->vpu_dec_info->core_id, context->vpu_dec_info->user_data.phys_addr, pbase, \
                   context->vpu_dec_info->user_data.size, VPU_USER_DATA_ENDIAN);

        if (context->vpu_dec_info->sequence_info.userDataHeader & (1 << H265_USERDATA_FLAG_VUI)) {
            h265_vui_param_t  *vui = (h265_vui_param_t *)(pbase + pEntry[H265_USERDATA_FLAG_VUI].offset);
            log_debug( " VUI SAR(%d, %d)\n", vui->sar_width, vui->sar_height);
            log_debug( "     VIDEO FORMAT(%d)\n", vui->video_format);
        }

        if (context->vpu_dec_info->sequence_info.userDataHeader & (1 << H265_USERDATA_FLAG_MASTERING_COLOR_VOL)) {
            h265_mastering_display_colour_volume_t *mastering;

            mastering = (h265_mastering_display_colour_volume_t *)(pbase + pEntry[H265_USERDATA_FLAG_MASTERING_COLOR_VOL].offset);
            log_debug( " MASTERING DISPLAY COLOR VOLUME\n");

            for (index = 0; index < 3; index++) {
                log_debug( " PRIMARIES_X%d : %10d PRIMARIES_Y%d : %10d\n", index, mastering->display_primaries_x[index], index, mastering->display_primaries_y[index]);
            }

            log_debug( " WHITE_POINT_X: %10d WHITE_POINT_Y: %10d\n", mastering->white_point_x, mastering->white_point_y);
            log_debug( " MIN_LUMINANCE: %10d MAX_LUMINANCE: %10d\n", mastering->min_display_mastering_luminance, mastering->max_display_mastering_luminance);
        }

        if (context->vpu_dec_info->sequence_info.userDataHeader & (1 << H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT)) {
            h265_chroma_resampling_filter_hint_t *c_resampleing_filter_hint;
            uint32_t i, j;
            c_resampleing_filter_hint = (h265_chroma_resampling_filter_hint_t *)(pbase + pEntry[H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT].offset);
            log_debug( " CHROMA_RESAMPLING_FILTER_HINT\n");
            log_debug( " VER_CHROMA_FILTER_IDC: %10d HOR_CHROMA_FILTER_IDC: %10d\n", c_resampleing_filter_hint->ver_chroma_filter_idc, c_resampleing_filter_hint->hor_chroma_filter_idc);
            log_debug( " VER_FILTERING_FIELD_PROCESSING_FLAG: %d \n", c_resampleing_filter_hint->ver_filtering_field_processing_flag);

            if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1 || c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                log_debug( " TARGET_FORMAT_IDC: %d \n", c_resampleing_filter_hint->target_format_idc);

                if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1) {
                    log_debug( " NUM_VERTICAL_FILTERS: %d \n", c_resampleing_filter_hint->num_vertical_filters);

                    for (i = 0; i < c_resampleing_filter_hint->num_vertical_filters; i++) {
                        log_debug( " VER_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->ver_tap_length_minus1[i]);

                        for (j = 0; j < c_resampleing_filter_hint->ver_tap_length_minus1[i]; j++) {
                            log_debug( " VER_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->ver_filter_coeff[i][j]);
                        }
                    }
                }

                if (c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                    log_debug( " NUM_HORIZONTAL_FILTERS: %d \n", c_resampleing_filter_hint->num_horizontal_filters);

                    for (i = 0; i < c_resampleing_filter_hint->num_horizontal_filters; i++) {
                        log_debug( " HOR_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->hor_tap_length_minus1[i]);

                        for (j = 0; j < c_resampleing_filter_hint->hor_tap_length_minus1[i]; j++) {
                            log_debug( " HOR_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->hor_filter_coeff[i][j]);
                        }
                    }
                }
            }
        }

        if (context->vpu_dec_info->sequence_info.userDataHeader & (1 << H265_USERDATA_FLAG_KNEE_FUNCTION_INFO)) {
            h265_knee_function_info_t *knee_function;

            knee_function = (h265_knee_function_info_t *)(pbase + pEntry[H265_USERDATA_FLAG_KNEE_FUNCTION_INFO].offset);
            log_debug( " FLAG_KNEE_FUNCTION_INFO\n");
            log_debug( " KNEE_FUNCTION_ID: %10d\n", knee_function->knee_function_id);
            log_debug( " KNEE_FUNCTION_CANCEL_FLAG: %d\n", knee_function->knee_function_cancel_flag);

            if (knee_function->knee_function_cancel_flag) {
                log_debug( " KNEE_FUNCTION_PERSISTENCE_FLAG: %10d\n", knee_function->knee_function_persistence_flag);
                log_debug( " INPUT_D_RANGE: %d\n", knee_function->input_d_range);
                log_debug( " INPUT_DISP_LUMINANCE: %d\n", knee_function->input_disp_luminance);
                log_debug( " OUTPUT_D_RANGE: %d\n", knee_function->output_d_range);
                log_debug( " OUTPUT_DISP_LUMINANCE: %d\n", knee_function->output_disp_luminance);
                log_debug( " NUM_KNEE_POINTS_M1: %d\n", knee_function->num_knee_points_minus1);

                for (index = 0; index < knee_function->num_knee_points_minus1; index++) {
                    log_debug( " INPUT_KNEE_POINT: %10d OUTPUT_KNEE_POINT: %10d\n", knee_function->input_knee_point[index], knee_function->output_knee_point[index]);
                }
            }
        }
    }

    log_debug("wave start decode seqinit succeess\n");
    return RET_CODE_SUCCESS;

err_sequence:

    if (context->vpu_dec_info->user_data.size > 0) {
        vdi_free_dma_memory(context->vpu_dec_info->core_id, &(context->vpu_dec_info->user_data));
        context->vpu_dec_info->user_data.size = 0;
    }

    if (context->vpu_dec_info->wave_init_data) {
        free(context->vpu_dec_info->wave_init_data);
        context->vpu_dec_info->wave_init_data = NULL;
    }

    log_err("wave seq init failure\n");
    return RET_CODE_FAILURE_SEQINIT;
}

/*
 * brief sdk_test_wave_decode_buffer_register - alloc & register framebuffer for wave decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_buffer_register(struct test_context *context)
{
    int ret = 0;
    int val = 0;
    uint32_t framebuf_stride = 0;
    uint32_t compressed_fb_count = 0;
    uint32_t linear_fb_count = 0;
    FrameBuffer pframe[MAX_REG_FRAME] = {{0}};

    CHECK_PARAM(context);

    if (context->test_cfg->dec_config->pvricFbcEnable == TRUE && (context->vpu_dec_info->sequence_info.lumaBitdepth > 8 || context->vpu_dec_info->sequence_info.chromaBitdepth > 8) && context->test_cfg->dec_config->wtlFormat != FORMAT_420_P10_16BIT_LSB) {
        // Fix AllocateDecFrameBuffer format mismatch bug, should be FORMAT_420_P10_16BIT_LSB
        context->test_cfg->dec_config->wtlFormat = FORMAT_420_P10_16BIT_LSB;
        log_debug("10bit wtlFormat shall be FORMAT_420_P10_16BIT_MSB when PVRIC FBC enabled\n");
    }

    if (context->test_cfg->dec_config->pvricFbcEnable == TRUE && (context->vpu_dec_info->sequence_info.lumaBitdepth == 8 && context->vpu_dec_info->sequence_info.chromaBitdepth == 8) && context->test_cfg->dec_config->wtlFormat != FORMAT_420) {
        context->test_cfg->dec_config->wtlFormat = FORMAT_420;
        log_debug("8bit wtlFormat shall be FORMAT_420 when PVRIC FBC enabled\n");
    }


    if (context->test_cfg->dec_config->pvricFbcEnable) {
        if (!context->test_cfg->dec_config->scaleDownWidth || !context->test_cfg->dec_config->scaleDownHeight)
            context->test_cfg->dec_config->scaleDownWidth = context->vpu_dec_info->sequence_info.picWidth;

        context->test_cfg->dec_config->scaleDownHeight = context->vpu_dec_info->sequence_info.picHeight;
    }

    PrintDecSeqWarningMessages(PRODUCT_ID_412, &context->vpu_dec_info->sequence_info);

    /* Set up the secondary AXI is depending on H/W configuration.*/
    if (vdi_set_sram_cfg(context->vpu_dec_info->core_id, context->test_cfg->dec_config->sramMode) == RETCODE_SUCCESS) {
        SecAxiUse  secAxiUse  = {0};
        secAxiUse.u.wave4.useIpEnable    = (context->test_cfg->dec_config->secondaryAXI & 0x01) ? TRUE : FALSE;
        secAxiUse.u.wave4.useLfRowEnable = (context->test_cfg->dec_config->secondaryAXI & 0x02) ? TRUE : FALSE;
        secAxiUse.u.wave4.useBitEnable   = (context->test_cfg->dec_config->secondaryAXI & 0x04) ? TRUE : FALSE;
        secAxiUse.u.wave4.useSclEnable   = (context->test_cfg->dec_config->secondaryAXI & 0x08) ? TRUE : FALSE;
        secAxiUse.u.wave4.useSclPackedModeEnable = (context->test_cfg->dec_config->secondaryAXI & 0x10) ? TRUE : FALSE;

        log_debug("[VPU] STEP 8.A - set sram config\n");
        log_debug("[VPU] useIpEnable: %d\n", secAxiUse.u.wave4.useIpEnable);
        log_debug("[VPU] useLfRowEnable: %d\n", secAxiUse.u.wave4.useLfRowEnable);
        log_debug("[VPU] useBitEnable: %d\n", secAxiUse.u.wave4.useBitEnable);
        log_debug("[VPU] useSclEnable: %d\n", secAxiUse.u.wave4.useSclEnable);
        log_debug("[VPU] useSclPackedModeEnable: %d\n", secAxiUse.u.wave4.useSclPackedModeEnable);
        VPU_DecGiveCommand(context->vpu_dec_info->handle, SET_SEC_AXI, &secAxiUse);
    }

    if (context->test_cfg->dec_config->scaleDownWidth > 0 || context->test_cfg->dec_config->scaleDownHeight > 0) {
        ScalerInfo sclInfo = {0};
        sclInfo.scaleWidth  = CalcScaleDown(context->vpu_dec_info->sequence_info.picWidth, context->test_cfg->dec_config->scaleDownWidth);
        sclInfo.scaleHeight = CalcScaleDown(context->vpu_dec_info->sequence_info.picHeight, context->test_cfg->dec_config->scaleDownHeight);
        log_debug( "[SCALE INFO] %dx%d to %dx%d\n", context->vpu_dec_info->sequence_info.picWidth, context->vpu_dec_info->sequence_info.picHeight, sclInfo.scaleWidth, sclInfo.scaleHeight);
        sclInfo.enScaler = TRUE;
        VPU_DecGiveCommand(context->vpu_dec_info->handle, DEC_SET_SCALER_INFO, (void *)&sclInfo);
    }

    compressed_fb_count = context->vpu_dec_info->sequence_info.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM + 4;   // max_dec_pic_buffering

    if (context->test_cfg->dec_config->enableWTL == TRUE) {
        linear_fb_count = compressed_fb_count;
        VPU_DecGiveCommand(context->vpu_dec_info->handle, DEC_SET_WTL_FRAME_FORMAT, &context->test_cfg->dec_config->wtlFormat);
    }
    else {
        linear_fb_count = 0;
    }

    log_debug( "compressed_fb_count=%d, linear_fb_count=%d\n", compressed_fb_count, linear_fb_count);
    osal_memset((void *)(context->vpu_dec_info->pfb_mem), 0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);

    if (!(AllocateDecFrameBuffer(context->vpu_dec_info->handle, context->test_cfg->dec_config, compressed_fb_count, linear_fb_count, pframe, context->vpu_dec_info->pfb_mem, &framebuf_stride))) {
        log_err("wave alloc dec frame buffer failure \n");
        return RET_CODE_FAILURE_FRAMEBUFFER_REGISTER;
    }

    log_debug("frame buffer allocated: compressed fb:%d linear fb:%d framebufstride:%d\n", compressed_fb_count,
              linear_fb_count, framebuf_stride);

    /* register frame buffer */
    ret = VPU_DecRegisterFrameBufferEx(context->vpu_dec_info->handle, pframe, compressed_fb_count, linear_fb_count, framebuf_stride, context->vpu_dec_info->sequence_info.picHeight, COMPRESSED_FRAME_MAP);

    if ( ret != RETCODE_SUCCESS ) {
        if (ret == RETCODE_MEMORY_ACCESS_VIOLATION) {
            EnterLock(context->vpu_dec_info->core_id);
            PrintMemoryAccessViolationReason(context->vpu_dec_info->core_id, NULL);
            LeaveLock(context->vpu_dec_info->core_id);
        }

        log_err("VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto error_buffer;
    }

    DisplayDecodedInformation(context->vpu_dec_info->handle, context->vpu_dec_info->open_param.bitstreamFormat, 0, NULL);
    val = (context->test_cfg->dec_config->bitFormat == STD_HEVC) ? SEQ_CHANGE_ENABLE_ALL_HEVC : SEQ_CHANGE_ENABLE_ALL_VP9;
    VPU_DecGiveCommand(context->vpu_dec_info->handle, DEC_SET_SEQ_CHANGE_MASK, (void *)&val);

    return ret;

error_buffer:

    ReleaseVideoMemory(context->vpu_dec_info->core_id, context->vpu_dec_info->pfb_mem, compressed_fb_count + linear_fb_count);
    log_err("wave register frame buffer failure\n");
    return RET_CODE_FAILURE_FRAMEBUFFER_REGISTER;
}

/*
 * brief sdk_test_wave_decode_sequence_deinit - free mem for user-data
 * param context  context info in test
 * return void
 *
 * notes
 */
static void sdk_test_wave_decode_sequence_deinit(struct test_context *context)
{
    if (context->vpu_dec_info->user_data.size > 0) {
        vdi_free_dma_memory(context->vpu_dec_info->core_id, &(context->vpu_dec_info->user_data));
        context->vpu_dec_info->user_data.size = 0;
    }

    if (context->vpu_dec_info->wave_init_data) {
        free(context->vpu_dec_info->wave_init_data);
        context->vpu_dec_info->wave_init_data = NULL;
    }
}

/*
 * brief sdk_test_wave_decode_framebuffer_register -
          alloc framebuffer & register framebuffer
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_framebuffer_register(struct test_context *context)
{
    int ret = 0;

    CHECK_PARAM(context);
    CALL_FUNCTION(sdk_test_wave_decode_sequence_init(context));

    if (RET_CODE_SUCCESS != (ret = sdk_test_wave_decode_buffer_register(context))) {
        sdk_test_wave_decode_sequence_deinit(context);
        log_err("wave decode register buffer failure\n");
        return ret;
    }

    log_debug("wave decode init sequence success \n");
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_wave_decode_feeder_create -
         create feeder
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_feeder_create(struct test_context *context)
{
    uint32_t size = 0;
    uint32_t bit_format = 0;
    uint64_t phys_addr = 0;
    char *input_path = NULL;

    input_path = context->test_cfg->dec_config->inputPath;
    phys_addr = context->vpu_dec_info->bstream_buf[0].phys_addr;
    size = context->vpu_dec_info->bstream_buf[0].size;
    bit_format = context->test_cfg->dec_config->bitFormat;

    if (context->vpu_dec_info->bstream_buf[0].phys_addr) {
        context->vpu_dec_info->feeder = BitstreamFeeder_Create(input_path, FEEDING_METHOD_FRAME_SIZE, \
                                        phys_addr, size, bit_format, \
                                        NULL, NULL, NULL, NULL);

        if (!(context->vpu_dec_info->feeder)) {
            log_err("wave create bitstream feeder failure \n");
            return RET_CODE_FAILURE_FEEDER;
        }

        BitstreamFeeder_SetFillMode(context->vpu_dec_info->feeder, BSF_FILLING_LINEBUFFER);

        log_debug("wave create bitstream feeder info:  base(%d) phy(%p) size(%d) format(%d) \n",
                  context->vpu_dec_info->bstream_buf[0].base,
                  context->vpu_dec_info->bstream_buf[0].phys_addr,
                  context->vpu_dec_info->bstream_buf[0].size,
                  context->test_cfg->dec_config->bitFormat);
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_wave_preparative_create -
         create feeder, register framebuffer malloc memory for fw in wave decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_preparative_create(struct test_context *context)
{
    int ret = 0;

    CHECK_PARAM(context);

    if (!context->test_cfg->is_decode) {
        log_err("wave preparative create failure core id(%d)\n", context->test_cfg->core_id);
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_wave_decode_feeder_create(context));

    if (RET_CODE_SUCCESS != (ret = sdk_test_wave_decode_framebuffer_register(context))) {
        sdk_test_wave_decode_feeder_destory(context);
        log_err("wave decode preparative framebuffer register failure ret(%d)\n", ret);
        return ret;
    }

    log_debug("wave preparative create success \n");
    return ret;
}

/*
 * brief sdk_test_vpu_preparative_create -
         create feeder, register framebuffer malloc memory for fw
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_vpu_preparative_create(struct test_context *context)
{
    CHECK_PARAM(context);

    if (context->test_cfg->core_id == CODA_CORE_ID) {
        CALL_FUNCTION(sdk_test_coda_preparative_create(context));
    }
    else if (context->test_cfg->core_id == WAVE_CORE_ID) {
        CALL_FUNCTION(sdk_test_wave_preparative_create(context));
    }
    else {
        log_err("preparative_create core id(%d) error\n", context->test_cfg->dec_config->coreIdx);
        return RET_CODE_INVALID_CORE_ID;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_decode_feeder_destory -
         destrory feeder in coda decoding
 * param context  context info in test
 * return void
 * notes
 */
static void sdk_test_coda_decode_feeder_destory(struct test_context *context)
{
    if (context->vpu_dec_info->feeder) {
        BitstreamFeeder_Destroy(context->vpu_dec_info->feeder);
        context->vpu_dec_info->feeder = NULL;
    }

    log_debug("coda decode feeder destory \n");
}

/*
 * brief sdk_test_code_decode_framebuffer_unregister -
         unregister framebuffer in coda decoding
 * param context  context info in test
 * return void
 * notes
 */
static void sdk_test_code_decode_framebuffer_unregister(struct test_context *context)
{
    uint32_t index = 0;

    for (index = 0; index < MAX_REG_FRAME; index++) {
        if (context->vpu_dec_info->pfb_mem[index].size > 0)
            vdi_free_dma_memory(context->test_cfg->core_id, &(context->vpu_dec_info->pfb_mem[index]));

        if (context->vpu_dec_info->ppu_fb_mem[index].size > 0)
            vdi_free_dma_memory(context->test_cfg->core_id, &(context->vpu_dec_info->ppu_fb_mem[index]));
    }

    if (context->vpu_dec_info->ppu_queue) {
        Queue_Destroy(context->vpu_dec_info->ppu_queue);
    }

}


/*
 * brief sdk_test_coda_decode_preparative_destroy -
         destrory feeder unregister framebuffer in coda decoding
 * param context  context info in test
 * return void
 * notes
 */
static void  sdk_test_coda_decode_preparative_destroy(struct test_context *context)
{
    sdk_test_coda_decode_feeder_destory(context);
    sdk_test_code_decode_framebuffer_unregister(context);
    log_debug("coda decode preparative destory \n");
}

/*
 * brief sdk_test_coda_encode_preparative_destroy -
         destrory feeder unregister framebuffer  free memory in coda encode
 * param context  context info in test
 * return void
 * notes
 */
static void  sdk_test_coda_encode_preparative_destroy(struct test_context *context)
{
    int i = 0;

    if (context->vpu_enc_info->bit_code) {
        free(context->vpu_enc_info->bit_code);
        context->vpu_enc_info->bit_code = NULL;
    }

    if (context->vpu_enc_info->bstream_buf.size) {
        vdi_free_dma_memory(context->vpu_enc_info->core_id, &(context->vpu_enc_info->bstream_buf));
        context->vpu_enc_info->bstream_buf.size = 0;
    }

    for (i = 0; i < context->vpu_enc_info->regframe_buffer_count; i++) {
        if (context->vpu_enc_info->register_frame_buffer[i].size) {
            vdi_free_dma_memory(context->vpu_enc_info->core_id, &(context->vpu_enc_info->register_frame_buffer[i]));
            context->vpu_enc_info->register_frame_buffer[i].size = 0;
        }
    }

    for (i = 0; i < ENCODE_SRC_BUF_NUM; i++) {
        if (context->vpu_enc_info->enc_src_frame_mem[i].size) {
            vdi_free_dma_memory(context->vpu_enc_info->core_id, &(context->vpu_enc_info->enc_src_frame_mem[i]));
            context->vpu_enc_info->enc_src_frame_mem[i].size = 0;
        }
    }

    if (context->vpu_enc_info->bs_reader) {
        BitstreamReader_Destroy(context->vpu_enc_info->bs_reader);
        context->vpu_enc_info->bs_reader = NULL;
    }

    if (context->vpu_enc_info->yuv_feeder) {
        YuvFeeder_Destroy(context->vpu_enc_info->yuv_feeder);
        context->vpu_enc_info->yuv_feeder = NULL;
    }

    log_debug("coda encode preparative destroy \n");
}

/*
 * brief sdk_test_coda_preparative_destroy -
         destrory feeder unregister framebuffer in cada decoding
 * param context  context info in test
 * return void
 * notes
 */
static void  sdk_test_coda_preparative_destroy(struct test_context *context)
{
    if (context->test_cfg->is_decode) {
        sdk_test_coda_decode_preparative_destroy(context);
    }
    else if (context->test_cfg->is_encode) {
        sdk_test_coda_encode_preparative_destroy(context);
    }
    else {
        log_err("coda preparative create failure core id(%d)\n", context->test_cfg->core_id);
    }
}

/*
 * brief sdk_test_wave_decode_framebuffer_unregister -
         destrory feeder in wave decoding
 * param context  context info in test
 * return void
 * notes
 */
static void sdk_test_wave_decode_feeder_destory(struct test_context *context)
{
    if (context->vpu_dec_info->feeder) {
        BitstreamFeeder_Destroy(context->vpu_dec_info->feeder);
        context->vpu_dec_info->feeder = NULL;
    }

    log_debug("wave decode feeder destory \n");
}

/*
 * brief sdk_test_wave_decode_framebuffer_unregister -
         unregister framebuffer in wave decoding
 * param context  context info in test
 * return void
 * notes
 */
static void sdk_test_wave_decode_framebuffer_unregister(struct test_context *context)
{
    if (context->vpu_dec_info->core_id == WAVE_CORE_ID) {
        if (context->vpu_dec_info->user_data.size > 0) {
            vdi_free_dma_memory(context->vpu_dec_info->core_id, &(context->vpu_dec_info->user_data));
            context->vpu_dec_info->user_data.size = 0;
        }

        if (context->vpu_dec_info->wave_init_data) {
            free(context->vpu_dec_info->wave_init_data);
            context->vpu_dec_info->wave_init_data = NULL;
        }

        ReleaseVideoMemory(context->vpu_dec_info->core_id, context->vpu_dec_info->pfb_mem, MAX_REG_FRAME);
        log_debug("wave decode frame buffer unregister\n");
    }
}

/*
 * brief sdk_test_wave_preparative_destroy -
         destrory feeder, unregister framebuffer in wave decoding
 * param context  context info in test
 * return void
 * notes
 */
static void sdk_test_wave_preparative_destroy(struct test_context *context)
{
    sdk_test_wave_decode_feeder_destory(context);
    sdk_test_wave_decode_framebuffer_unregister(context);
    log_debug("wave decode preparative destory \n");
}

/*
 * brief sdk_test_vpu_preparative_destroy -
         destrory feeder, unregister framebuffer
 * param context  context info in test
 * return void
 * notes
 */
static void sdk_test_vpu_preparative_destroy(struct test_context *context)
{
    if (context->test_cfg->core_id == CODA_CORE_ID) {
        sdk_test_coda_preparative_destroy(context);
    }
    else if (context->test_cfg->core_id == WAVE_CORE_ID) {
        sdk_test_wave_preparative_destroy(context);
    }
    else {
        log_err("preparative_destroy core id(%d) error\n", context->test_cfg->dec_config->coreIdx);
    }
}

/*
 * brief sdk_test_code_decode_streams_feeder - feeder streams to coda device
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_code_decode_streams_feeder(struct test_context *context)
{
    uint32_t size_write = 0;
    char *feeder = NULL;
    DecHandle handle = NULL;

    feeder = context->vpu_dec_info->feeder;
    handle = context->vpu_dec_info->handle;
    VPU_DecSetRdPtr(handle, context->vpu_dec_info->open_param.bitstreamBuffer, TRUE);

    if ( 0 > (size_write = BitstreamFeeder_Act(context->vpu_dec_info->handle, \
                           feeder, context->vpu_dec_info->open_param.streamEndian))) {
        log_err("coda bistream act error size_write(%d)\n", size_write);
        return size_write;
    }

    return size_write;
}

/*
 * brief sdk_test_coda_decode_one_frame_procedure -
         decoding one frame operation
 * param context  context info in test
 * parma output_info save result decoding data
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_one_frame_procedure(struct test_context *context, DecOutputInfo *output_info)
{
    int ret = 0;
    uint32_t core_id = 0;
    uint32_t interrupt_flag = 0;
    uint32_t timeout_count = 0;
    BOOL ppu_enable = FALSE;
    BOOL repeat = FALSE;
    FrameBuffer *ppu_fb = NULL;
    Queue *ppu_queue = NULL;

    ppu_enable = context->vpu_dec_info->ppu_enable;
    core_id = context->vpu_dec_info->core_id;

    if (ppu_enable == TRUE) {
        if ((ppu_fb = (FrameBuffer *)Queue_Dequeue(ppu_queue)) == NULL) {
            context->vpu_dec_info->need_stream = FALSE;
            log_err("coda ppu one frame procedure error \n");
            return RET_CODE_FAILURE_PPU;
        }

        VPU_DecGiveCommand(context->vpu_dec_info->handle, SET_ROTATOR_OUTPUT, (void *)ppu_fb);

        if (context->test_cfg->dec_config->coda9.rotate > 0) {
            VPU_DecGiveCommand(context->vpu_dec_info->handle, ENABLE_ROTATION, NULL);
        }

        if (context->test_cfg->dec_config->coda9.mirror > 0) {
            VPU_DecGiveCommand(context->vpu_dec_info->handle, ENABLE_MIRRORING, NULL);
        }

        if (context->test_cfg->dec_config->coda9.enableDering == TRUE) {
            VPU_DecGiveCommand(context->vpu_dec_info->handle, ENABLE_DERING, NULL);
        }
    }

    context->vpu_dec_info->performance.start_us = GetNowUs();

    if ((ret = VPU_DecStartOneFrame(context->vpu_dec_info->handle, &(context->vpu_dec_info->dec_param))) != RETCODE_SUCCESS) {
        log_err("start one frame failed rrror code is 0x%x \n", ret );
        return RET_CODE_FAILURE_START_FRAME;
    }

    timeout_count = 0;
    repeat = TRUE;

    while (TRUE) {
        if ((interrupt_flag = VPU_WaitInterrupt(core_id, VPU_WAIT_TIME_OUT)) == -1) {
            if (timeout_count * VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                PrintVpuStatus(core_id,  PRODUCT_ID_412);
                VPU_SWReset(core_id, SW_RESET_SAFETY, context->vpu_dec_info->handle);
                LeaveLock(core_id);
                return RET_CODE_FAILURE_TIME_OUT;
            }

            timeout_count++;
            interrupt_flag = 0;
        }

        CheckUserDataInterrupt(core_id, context->vpu_dec_info->handle, \
                               output_info->indexFrameDecoded, \
                               context->vpu_dec_info->open_param.bitstreamFormat, \
                               interrupt_flag);

        if (interrupt_flag & (1 << INT_BIT_PIC_RUN)) {
            repeat = FALSE;
        }

        if (interrupt_flag > 0) {
            log_debug("VPU_ClearInterrupt line %d\n", __LINE__);
            VPU_ClearInterrupt(core_id);
        }

        if (repeat == FALSE)
            break;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_coda_decode_yuv_save - save yuv data to file
 * param context  context info in test
 * parma output_info save result decoding data
 * param ppu rect save
 * return 0  success
 *        !0 failure code
 * notes
 */
static void sdk_test_coda_decode_yuv_save(struct test_context *context, DecOutputInfo *output_info, VpuRect *ppu)
{
    uint32_t width = 0, height = 0, Bpp = 0;
    size_t  frame_size = 0;
    BOOL dump_image = FALSE;
    char *pyuv = NULL;
    VpuRect  rc_ppu = {0};

    memcpy(&rc_ppu, ppu, sizeof(VpuRect));
    VpuRect rc = context->vpu_dec_info->ppu_enable  == TRUE ? rc_ppu : output_info->rcDisplay;

    if (strlen(context->test_cfg->dec_config->outputPath) > 0)
        dump_image = TRUE;

    if (dump_image == TRUE) {
        pyuv = (char *)GetYUVFromFrameBuffer(context->vpu_dec_info->handle, &(output_info->dispFrame), rc, &width, &height, &Bpp, &frame_size);

        if (pyuv) {
            if (strlen(context->test_cfg->dec_config->outputPath) > 0) {
                if (context->vpu_dec_info->save_fp[0] == NULL) {
                    if ((context->vpu_dec_info->save_fp[0] = osal_fopen(context->test_cfg->dec_config->outputPath, "wb")) == NULL) {
                        log_err("coda decode open save file error\n");
                        return;
                    }
                }

                osal_fwrite (pyuv, frame_size, 1, context->vpu_dec_info->save_fp[0]);
            }

            free(pyuv);
            pyuv = NULL;
        }

    }
}

/*
 * brief sdk_test_coda_decode_output_info_get -
         get output data when one frame decode finish
 * param context  context info in test
 * parma output_info save result decoding data
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_output_info_get(struct test_context *context, DecOutputInfo *output_info)
{
    int ret = 0;
    static int prev_fb_index = -1;
    uint32_t core_id = 0;
    static uint32_t nofb_count = 0;
    BOOL ppu_enable = FALSE;
    Queue *ppu_queue = NULL;
    FrameBuffer *ppu_fb = NULL;
    VpuRect  rc_ppu = {0};

    core_id = context->vpu_dec_info->core_id;
    ppu_enable = context->vpu_dec_info->ppu_enable;
    ppu_queue = context->vpu_dec_info->ppu_queue;
    ret = VPU_DecGetOutputInfo(context->vpu_dec_info->handle, output_info);

    if (ret != RETCODE_SUCCESS) {
        log_err("VPU_DecGetOutputInfo failed Error code is 0x%x\n", ret);

        if (ret == RETCODE_MEMORY_ACCESS_VIOLATION) {
            EnterLock(core_id);
            PrintVpuStatus(core_id, PRODUCT_ID_980);
            PrintMemoryAccessViolationReason(core_id, output_info);
            LeaveLock(core_id);
        }

        return RET_CODE_DECODE_CONTINUE;
    }

    if (output_info->decodingSuccess) {
        context->vpu_dec_info->performance.diff_us = GetNowUs() - \
                context->vpu_dec_info->performance.start_us;
        context->vpu_dec_info->performance.total_us += context->vpu_dec_info->performance.diff_us;

        if (output_info->picType == PIC_TYPE_I
                || output_info->picType == PIC_TYPE_P
                || output_info->picType == PIC_TYPE_B
                || output_info->picType == PIC_TYPE_IDR) {
            log_debug("decoding time=%.1fms\n", (double)context->vpu_dec_info->performance.diff_us / 1000);
        }
    }

    if (output_info->indexFrameDecoded == DECODED_IDX_FLAG_NO_FB &&
            output_info->indexFrameDisplay == DISPLAY_IDX_FLAG_NO_FB) {
        if (nofb_count++ == 100) {
            log_debug("warning coda decode no framebuffer nofb_count(%d)\n", nofb_count);
            return RET_CODE_FAILURE_NO_FB;
        }
    }

    if (output_info->indexFrameDecoded >= 0) {
        context->vpu_dec_info->dec_frame_count++;
    }

    DisplayDecodedInformation(context->vpu_dec_info->handle, context->vpu_dec_info->open_param.bitstreamFormat, \
                              context->vpu_dec_info->dec_frame_count, output_info);

    /* sequeue change request no support */
    if (output_info->sequenceChanged) {
        log_err("coda decode no support sequence changed\n");
        return RET_CODE_FAILURE_SQE_CHANGE;
    }

    if (ppu_enable == TRUE) {
        if (prev_fb_index >= 0) {
            VPU_DecClrDispFlag(context->vpu_dec_info->handle, prev_fb_index);
        }

        prev_fb_index = output_info->indexFrameDisplay;

        if (context->vpu_dec_info->wait_post_processing == TRUE) {
            if (output_info->indexFrameDisplay >= 0) {
                context->vpu_dec_info->wait_post_processing = FALSE;
            }

            rc_ppu = output_info->rcDisplay;
            /* Release framebuffer for PPU */
            Queue_Enqueue(ppu_queue, (void *)ppu_fb);
            context->vpu_dec_info->need_stream = TRUE;

            if (output_info->chunkReuseRequired == TRUE) {
                context->vpu_dec_info->need_stream = FALSE;
            }

            /* Not ready for ppu buffer.*/
            log_debug("coda enable ppu \n");
            return RET_CODE_SUCCESS;
        }
        else {
            if (output_info->indexFrameDisplay < 0) {
                context->vpu_dec_info->wait_post_processing = TRUE;
            }
        }
    }

    if (output_info->indexFrameDisplay >= 0 || ppu_enable == TRUE) {
        sdk_test_coda_decode_yuv_save(context, output_info, &rc_ppu);
        VPU_DecClrDispFlag(context->vpu_dec_info->handle, output_info->indexFrameDisplay);
        context->vpu_dec_info->dis_frame_count++;
    }

    if (output_info->indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END) {
        log_debug("coda decode success now \n");
        return RET_CODE_DECODE_END;
    }

    if (output_info->indexFrameDecoded == DECODED_IDX_FLAG_NO_FB) {
        context->vpu_dec_info->need_stream = FALSE;
    }
    else {
        context->vpu_dec_info->need_stream = TRUE;
    }

    if (output_info->chunkReuseRequired == TRUE) {
        context->vpu_dec_info->need_stream = FALSE;
    }

    /*
        SaveDecReport(core_id, context->vpu_dec_info->handle, output_info, context->vpu_dec_info->open_param.bitstreamFormat,
                      ((context->vpu_dec_info->sequence_info.picWidth + 15) & ~15) / 16,
                      ((context->vpu_dec_info->sequence_info.picHeight + 15) & ~15) / 16);
    */
    return RET_CODE_SUCCESS;
}


/*
 * brief sdk_test_coda_decode_procedure - coda device decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_procedure(struct test_context *context)
{

    int ret = 0;
    BOOL success = TRUE;
    DecOutputInfo output_info = {0};

    CHECK_PARAM(context);
    context->vpu_dec_info->need_stream = FALSE;

    while (true) {

        if (context->vpu_dec_info->need_stream) {
            if (0 > (ret = sdk_test_code_decode_streams_feeder(context))) {
                log_debug("coda feeder data end now ...\n");
            }

            context->vpu_dec_info->need_stream = FALSE;
        }

        if (RET_CODE_SUCCESS != (ret = sdk_test_coda_decode_one_frame_procedure(context, &output_info))) {
            log_err("coda decode start one frame errorr ret(%d)\n", ret);
            success = FALSE;
            break;
        }

        ret = sdk_test_coda_decode_output_info_get(context, &output_info);

        if (ret == RET_CODE_DECODE_CONTINUE) {
            log_err("coda decode ouput info continue ret(%d)\n", ret);
            continue;
        }

        if (ret == RET_CODE_DECODE_END) {
            log_debug("coda decode success now \n");
            break;
        }

        if (ret != RET_CODE_SUCCESS) {
            log_err("coda decode output info error ret(%d)\n", ret);
            success = FALSE;
            break;
        }

        if ((context->vpu_dec_info->dec_frame_count > 0)
                && (context->vpu_dec_info->dec_frame_count >= context->test_cfg->dec_config->forceOutNum)) {
            log_debug("coda decode display-frames(%d), decode-frames(%d)\n",
                      context->vpu_dec_info->dis_frame_count,
                      context->vpu_dec_info->dec_frame_count);
            break;
        }
    }

    if (context->vpu_dec_info->save_fp[0]) {
        osal_fclose(context->vpu_dec_info->save_fp[0]);
        context->vpu_dec_info->save_fp[0] = NULL;
    }

    if (success)
        return RET_CODE_SUCCESS;

    return RET_CODE_FAILURE_DEOCDE;
}

/*
 * brief sdk_test_wave_decode_streams_feeder - feeder one frame data
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_streams_feeder(struct test_context *context)
{
    uint32_t size_write = 0;
    uint32_t bstream_index = 0;
    char *feeder = NULL;

    feeder = context->vpu_dec_info->feeder;
    context->vpu_dec_info->bstream_index = (context->vpu_dec_info->bstream_index + 1) % MAX_BUF_NUM;
    bstream_index = context->vpu_dec_info->bstream_index;

    VPU_DecSetRdPtrEx(context->vpu_dec_info->handle, context->vpu_dec_info->bstream_buf[bstream_index].phys_addr, \
                      context->vpu_dec_info->bstream_buf[bstream_index].phys_addr, TRUE);

    if ( 0 > (size_write = BitstreamFeeder_Act(context->vpu_dec_info->handle, \
                           feeder, context->test_cfg->dec_config->streamEndian))) {
        log_err("wave bistream act error size_write(%d)\n", size_write);
        return size_write;
    }

    return size_write;
}

/*
 * brief sdk_test_wave_decode_one_frame_procedure -
         decoding one frame using wave
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_one_frame_procedure(struct test_context *context)
{
    int32_t ret = 0;
    uint32_t core_id = 0;
    uint32_t timeout_count = 0;
    uint32_t interrupt_flag = 0;
    BOOL success = TRUE;
    DecParam *dec_param = NULL;
    TestDecConfig *cfg_param = NULL;

    dec_param = &(context->vpu_dec_info->dec_param);
    cfg_param = context->test_cfg->dec_config;
    core_id = context->vpu_dec_info->core_id;
    dec_param->craAsBlaFlag = cfg_param->wave4.craAsBla;
    dec_param->pixelLumaPad   = cfg_param->pvricPaddingY;  // default padding pixel value = 0
    dec_param->pixelChromaPad = cfg_param->pvricPaddingC;
    context->vpu_dec_info->performance.start_us = GetNowUs();

    if (RETCODE_SUCCESS != (ret = VPU_DecStartOneFrame(context->vpu_dec_info->handle, dec_param))) {
        log_err("VPU_DecStartOneFrame failed Error code is 0x%x \n", ret );
        LeaveLock(core_id);
        return RET_CODE_FAILURE_START_FRAME;
    }

    timeout_count = 0;

    while (TRUE) {
        if ((interrupt_flag = VPU_WaitInterrupt(core_id, VPU_WAIT_TIME_OUT)) == -1) {
            if (timeout_count * VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                HandleDecoderError(context->vpu_dec_info->handle, context->vpu_dec_info->dec_frame_count, \
                                   context->test_cfg->dec_config, \
                                   context->vpu_dec_info->pfb_mem, NULL);
                PrintVpuStatus(core_id, PRODUCT_ID_412);
                log_err("\n VPU interrupt wait timeout\n");
                LeaveLock(core_id);
                VPU_SWReset(core_id, SW_RESET_SAFETY, context->vpu_dec_info->handle);
                success = FALSE;
                break;
            }

            timeout_count++;
            interrupt_flag = 0;
        }

        if (interrupt_flag > 0) {
            VPU_ClearInterrupt(core_id);
        }

        if (interrupt_flag & (1 << INT_WAVE_DEC_PIC)) {
            break;
        }

        if (interrupt_flag & (1 << INT_WAVE_BIT_BUF_EMPTY)) {
            log_debug("wave empty interrput \n");
        }
    }

    if (!success) {
        log_err("wave decode one frame error \n");
        return RET_CODE_FAILURE_DEOCDE;
    }

    return ret;
}

/*
 * brief sdk_test_wave_decode_output_uesr_data_get -
         get user-output data when one frame decode finish
 * param context  context info in test
 * parma output_info save result decoding data
 * return 0  success
 *        !0 failure code
 * notes  has not been measured yet
 */
static void sdk_test_wave_decode_output_uesr_data_get(struct test_context *context, DecOutputInfo *output_info)
{
    uint32_t core_id = 0;
    uint32_t index = 0;
    unsigned char *pbase = NULL;

    core_id = context->vpu_dec_info->core_id;
    pbase = (unsigned char *)context->vpu_dec_info->wave_init_data;

    if (output_info->decOutputExtData.userDataNum > 0) {
        user_data_entry_t *pEntry = (user_data_entry_t *)pbase;
        VpuReadMem(core_id, context->vpu_dec_info->user_data.phys_addr, pbase, \
                   (context->vpu_dec_info->user_data).size, VPU_USER_DATA_ENDIAN);

        for (index = 0; index < 32; index++) {
            if (output_info->decOutputExtData.userDataHeader & (1 << index)) {
                log_debug("USERDATA INDEX: %02d offset: %8d size: %d\n", index, pEntry[index].offset, pEntry[index].size);

                if (output_info->decOutputExtData.userDataHeader & (1 << H265_USERDATA_FLAG_MASTERING_COLOR_VOL)) {
                    h265_mastering_display_colour_volume_t *mastering;
                    int i;

                    mastering = (h265_mastering_display_colour_volume_t *)(pbase + pEntry[H265_USERDATA_FLAG_MASTERING_COLOR_VOL].offset);
                    log_debug(" MASTERING DISPLAY COLOR VOLUME\n");

                    for (i = 0; i < 3; i++) {
                        log_debug(" PRIMARIES_X%d : %10d PRIMARIES_Y%d : %10d\n", i, mastering->display_primaries_x[i], i, mastering->display_primaries_y[i]);
                    }

                    log_debug(" WHITE_POINT_X: %10d WHITE_POINT_Y: %10d\n", mastering->white_point_x, mastering->white_point_y);
                    log_debug(" MIN_LUMINANCE: %10d MAX_LUMINANCE: %10d\n", mastering->min_display_mastering_luminance, mastering->max_display_mastering_luminance);
                }

                if (output_info->decOutputExtData.userDataHeader & (1 << H265_USERDATA_FLAG_VUI)) {
                    h265_vui_param_t *vui;
                    vui = (h265_vui_param_t *)(pbase + pEntry[H265_USERDATA_FLAG_VUI].offset);
                    log_debug(" VUI SAR(%d, %d)\n", vui->sar_width, vui->sar_height);
                    log_debug("     VIDEO FORMAT(%d)\n", vui->video_format);
                    log_debug("     COLOUR PRIMARIES(%d)\n", vui->colour_primaries);
                    log_debug("log2_max_mv_length_horizontal: %d\n", vui->log2_max_mv_length_horizontal);
                    log_debug("log2_max_mv_length_vertical  : %d\n", vui->log2_max_mv_length_vertical);
                }

                if (output_info->decOutputExtData.userDataHeader & (1 << H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT)) {
                    h265_chroma_resampling_filter_hint_t *c_resampleing_filter_hint;
                    uint32_t i, j;

                    c_resampleing_filter_hint = (h265_chroma_resampling_filter_hint_t *)(pbase + pEntry[H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT].offset);
                    log_debug(" CHROMA_RESAMPLING_FILTER_HINT\n");
                    log_debug( " VER_CHROMA_FILTER_IDC: %10d HOR_CHROMA_FILTER_IDC: %10d\n", c_resampleing_filter_hint->ver_chroma_filter_idc, c_resampleing_filter_hint->hor_chroma_filter_idc);
                    log_debug(" VER_FILTERING_FIELD_PROCESSING_FLAG: %d \n", c_resampleing_filter_hint->ver_filtering_field_processing_flag);

                    if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1 || c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                        log_debug(" TARGET_FORMAT_IDC: %d \n", c_resampleing_filter_hint->target_format_idc);

                        if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1) {
                            log_debug(" NUM_VERTICAL_FILTERS: %d \n", c_resampleing_filter_hint->num_vertical_filters);

                            for (i = 0; i < c_resampleing_filter_hint->num_vertical_filters; i++) {
                                log_debug(" VER_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->ver_tap_length_minus1[i]);

                                for (j = 0; j < c_resampleing_filter_hint->ver_tap_length_minus1[i]; j++) {
                                    log_debug(" VER_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->ver_filter_coeff[i][j]);
                                }
                            }
                        }

                        if (c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                            log_debug(" NUM_HORIZONTAL_FILTERS: %d \n", c_resampleing_filter_hint->num_horizontal_filters);

                            for (i = 0; i < c_resampleing_filter_hint->num_horizontal_filters; i++) {
                                log_debug(" HOR_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->hor_tap_length_minus1[i]);

                                for (j = 0; j < c_resampleing_filter_hint->hor_tap_length_minus1[i]; j++) {
                                    log_debug(" HOR_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->hor_filter_coeff[i][j]);
                                }
                            }
                        }
                    }
                }

                if (output_info->decOutputExtData.userDataHeader & (1 << H265_USERDATA_FLAG_KNEE_FUNCTION_INFO)) {
                    h265_knee_function_info_t *knee_function;

                    knee_function = (h265_knee_function_info_t *)(pbase + pEntry[H265_USERDATA_FLAG_KNEE_FUNCTION_INFO].offset);
                    log_debug(" FLAG_KNEE_FUNCTION_INFO\n");
                    log_debug(" KNEE_FUNCTION_ID: %10d\n", knee_function->knee_function_id);
                    log_debug(" KNEE_FUNCTION_CANCEL_FLAG: %d\n", knee_function->knee_function_cancel_flag);

                    if (knee_function->knee_function_cancel_flag) {
                        log_debug(" KNEE_FUNCTION_PERSISTENCE_FLAG: %10d\n", knee_function->knee_function_persistence_flag);
                        log_debug(" INPUT_D_RANGE: %d\n", knee_function->input_d_range);
                        log_debug(" INPUT_DISP_LUMINANCE: %d\n", knee_function->input_disp_luminance);
                        log_debug(" OUTPUT_D_RANGE: %d\n", knee_function->output_d_range);
                        log_debug(" OUTPUT_DISP_LUMINANCE: %d\n", knee_function->output_disp_luminance);
                        log_debug(" NUM_KNEE_POINTS_M1: %d\n", knee_function->num_knee_points_minus1);

                        for (index = 0; index < knee_function->num_knee_points_minus1; index++) {
                            log_debug(" INPUT_KNEE_POINT: %10d OUTPUT_KNEE_POINT: %10d\n", knee_function->input_knee_point[index], knee_function->output_knee_point[index]);
                        }
                    }
                }
            }
        }
    }
}


/*
 * brief sdk_test_wave_decode_output_info_get -
         get output data when one frame decode finish
 * param context  context info in test
 * param output_info save decoding data
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_output_info_get(struct test_context *context, DecOutputInfo *output_info)
{
    int ret = 0;
    uint32_t core_id = 0;
    static uint32_t no_fb_count = 0;
    DecHandle handle = NULL;

    core_id = context->vpu_dec_info->core_id;
    handle = context->vpu_dec_info->handle;
    ret = VPU_DecGetOutputInfo(handle, output_info);

    if (ret != RETCODE_SUCCESS) {
        log_err("VPU_DecGetOutputInfo failed Error code is 0x%x\n", ret);

        if (ret == RETCODE_MEMORY_ACCESS_VIOLATION) {
            EnterLock(core_id);
            PrintMemoryAccessViolationReason(core_id, output_info);
            PrintVpuStatus(core_id, PRODUCT_ID_412);
            LeaveLock(core_id);
            return RET_CODE_FAILURE_OUTPUT_INFO;
        }

        return RET_CODE_DECODE_CONTINUE;
    }

    sdk_test_wave_decode_output_uesr_data_get(context, output_info);

    if ((output_info->decodingSuccess & 0x01) == 0) {
        log_debug("output dec imcomplete \n");

        if (output_info->indexFramePrescan == -2) {
            log_debug("stream is insufficient to prescan\n");
        }
        else {
            log_err("instance(%d) VPU_DecGetOutputInfo decode fail framdIdx %d error(0x%08x) reason(0x%08x), reasonExt(0x%08x)\n",
                    handle->instIndex, context->vpu_dec_info->dec_frame_count, output_info->decodingSuccess, \
                    output_info->errorReason, output_info->errorReasonExt);
        }
    }
    else {
        context->vpu_dec_info->performance.diff_us = GetNowUs() -  \
                context->vpu_dec_info->performance.start_us;
        context->vpu_dec_info->performance.total_us  += context->vpu_dec_info->performance.diff_us;

        if (output_info->picType == PIC_TYPE_I
                || output_info->picType == PIC_TYPE_P
                || output_info->picType == PIC_TYPE_B
                || output_info->picType == PIC_TYPE_IDR) {
            log_debug("decoding one frame time(%.1f)ms\n", (double)(context->vpu_dec_info->performance.diff_us) / 1000);
        }
    }

    /* no support sequence change now ... */
    // sdk_test_decode_wave_sequence_change(context);

    DisplayDecodedInformation(handle, context->vpu_dec_info->open_param.bitstreamFormat, \
                              context->vpu_dec_info->dec_frame_count, output_info);

    if (output_info->indexFrameDisplay >= 0) {
        VpuRect     rcDisplay;
        rcDisplay.left   = 0;
        rcDisplay.top    = 0;
        rcDisplay.right  = output_info->dispPicWidth;
        rcDisplay.bottom = output_info->dispPicHeight;

        if (strlen(context->test_cfg->dec_config->outputPath) > 0) {
            if (!context->vpu_dec_info->save_fp[0]) {
                if ((context->vpu_dec_info->save_fp[0] = osal_fopen(context->test_cfg->dec_config->outputPath, "wb")) == NULL)
                    return RET_CODE_FAILURE_FILE;
            }

            SaveDisplayBufferToFile(handle, output_info->dispFrame, rcDisplay, (FILE **)context->vpu_dec_info->save_fp, \
                                    context->test_cfg->dec_config->outputPath, \
                                    context->vpu_dec_info->dis_frame_count);
            log_debug("yuv stored one frame.size(%d)\n", output_info->dispFrame.size);
            context->vpu_dec_info->dis_frame_count++;
        }

        /* clear the display flay */
        VPU_DecClrDispFlag(handle, output_info->indexFrameDisplay);
    }
    else if (output_info->indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END) {
        log_debug("wave decode finish, the stream is end \n");
        return RET_CODE_DECODE_END;

    }
    else {
        log_debug("waring wave decode indexFrameDisplay(%d) < 0\n", output_info->indexFrameDisplay);
    }

    if (output_info->indexFrameDecoded >= 0) {
        context->vpu_dec_info->dec_frame_count++;
    }
    else if (output_info->indexFrameDecoded == DECODED_IDX_FLAG_NO_FB) {
        log_debug("wave decode indexFrameDecoded(-1), have no framebufer\n");
        context->vpu_dec_info->need_stream = FALSE;

        if (no_fb_count++ == 100) {
            log_err("wave decode have no frame buffer count(%d)\n", no_fb_count);
            return RET_CODE_FAILURE_NO_FB;
        }
    }
    else {
        log_debug("waring wave decode output info indexFrameDecoded(%d)\n", output_info->indexFrameDecoded);
    }

    context->vpu_dec_info->need_stream = TRUE;

    /* super frame(many frames in one packet) */
    if (handle->codecMode == C7_VP9_DEC) {
        if (handle->CodecInfo->decInfo.streamRdPtr < handle->CodecInfo->decInfo.streamWrPtr)
            context->vpu_dec_info->need_stream = FALSE;
    }

    if (handle->codecMode == C7_HEVC_DEC ) {
        if (output_info->indexFramePrescan == -1 || output_info->sequenceChanged != 0) {
            log_debug("Waring wave decode hevc indexFramePrescan(%d)\n", output_info->indexFramePrescan);
            context->vpu_dec_info->need_stream = FALSE;
            VPU_DecSetRdPtrEx(handle, \
                              context->vpu_dec_info->bstream_buf[context->vpu_dec_info->bstream_index].phys_addr, \
                              context->vpu_dec_info->bstream_buf[context->vpu_dec_info->bstream_index].phys_addr, TRUE);
        }
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_vpu_decode_procedure - wave device decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_procedure(struct test_context *context)
{
    int ret = 0;
    BOOL success = TRUE;
    DecOutputInfo output_info = {0};

    context->vpu_dec_info->need_stream = FALSE;

    while (true) {

        if (context->vpu_dec_info->need_stream) {
            if (0 > (ret = sdk_test_wave_decode_streams_feeder(context))) {
                log_debug("waring wave feeder data end now ...\n");
            }

            context->vpu_dec_info->need_stream = FALSE;
        }

        if (RET_CODE_SUCCESS != (ret = sdk_test_wave_decode_one_frame_procedure(context))) {
            log_err("coda decode start one frame errorr ret(%d)\n", ret);
            success = FALSE;
            break;
        }

        ret = sdk_test_wave_decode_output_info_get(context, &output_info);

        if (ret == RET_CODE_DECODE_CONTINUE) {
            log_err("wave decode ouput info continue ret(%d)\n", ret);
            continue;
        }

        if (ret == RET_CODE_DECODE_END) {
            log_debug("wave decode success now \n");
            break;
        }

        if (ret != RET_CODE_SUCCESS) {
            log_err("wave decode output info error ret(%d)\n", ret);
            success = FALSE;
            break;
        }

        if ((context->vpu_dec_info->dis_frame_count > 0)
                && (context->vpu_dec_info->dis_frame_count >= context->test_cfg->dec_config->forceOutNum)) {
            log_debug("wave decode display-frames(%d), decode-frames(%d)\n",
                      context->vpu_dec_info->dis_frame_count,
                      context->vpu_dec_info->dec_frame_count);
            break;
        }
    }

    if (context->vpu_dec_info->save_fp[0]) {
        osal_fclose(context->vpu_dec_info->save_fp[0]);
        context->vpu_dec_info->save_fp[0] = NULL;
    }

    if (!success)
        return RET_CODE_FAILURE_DEOCDE;

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_vpu_decode_procedure - vpu device decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_vpu_decode_procedure(struct test_context *context)
{
    CHECK_PARAM(context);

    if (context->vpu_dec_info->core_id == CODA_CORE_ID) {
        CALL_FUNCTION(sdk_test_coda_decode_procedure(context));
    }
    else if (context->vpu_dec_info->core_id == WAVE_CORE_ID) {
        CALL_FUNCTION(sdk_test_wave_decode_procedure(context));
    }
    else {
        log_err("vpu decode param error core id(%d)\n", context->vpu_dec_info->core_id);
        return RET_CODE_INVALID_PARAM;
    }

    log_debug("vpu decode procedure success \n");
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_frame_seek_position_zero - bitstreams seek starting position
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_frame_seek_position_start(struct test_context *context)
{
    BOOL success = TRUE;
    CHECK_PARAM(context);

    if (!(success = BitstreamFeeder_Rewind((void *)(context->vpu_dec_info->feeder)))) {
        log_err("seek starting position error \n");
        return RET_CODE_FAILURE_START_FRAME;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_frame_seek - bitstreams seek
 * param context  context info in test
 * param ts_time seek timestamp
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_frame_seek(struct test_context *context, uint64_t ts_time)
{
    int ret = 0;

    if (0 > (ret = BitstreamFeeder_Seek((void *)(context->vpu_dec_info->feeder), ts_time))) {
        log_err("seek frame error ret(%d)\n", ret);
        return RET_CODE_FAILURE_SEEK;
    }

    return ret;
}

/*
 * brief sdk_test_frame_seek_procedure -
         bitstream seek operation and vpu flush operation before seek-frame(I-type)
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_frame_seek_procedure(struct test_context *context)
{
    int ret = 0;
    uint32_t num = 0;
    uint32_t i = 0;
    static int interval = 0;
    DecOutputInfo remain[MAX_FRAME_BUFFER] = {0};

    interval++;

    if ((context->vpu_dec_info->seek_frame_interval == interval)
            && (context->vpu_dec_info->ts_end >= context->vpu_dec_info->ts_seek)) {

        interval = 0;
        context->vpu_dec_info->ts_seek = context->vpu_dec_info->ts_seek + context->vpu_dec_info->ts_offset;
        log_debug("seek oper ts_seek(%"PRId64"), ts_offset(%"PRId64"), ts_end(%"PRId64"), frame_interval(%d)\n",
                  context->vpu_dec_info->ts_seek,
                  context->vpu_dec_info->ts_offset,
                  context->vpu_dec_info->ts_end,
                  context->vpu_dec_info->seek_frame_interval);

        if ((ret = sdk_test_frame_seek(context, context->vpu_dec_info->ts_seek))) {
            log_err("seek frames error ret(%d)\n", ret);
            return RET_CODE_FAILURE_SEEK;
        }

        /*vpu flush clear all of remain display indexes that FW owns */
        VPU_DecFrameBufferFlush(context->vpu_dec_info->handle, remain, &num);

        for (i = 0; i < num; i++) {
            log_debug("set display-flag indexFrameDisplay(%d)\n", i);
            VPU_DecClrDispFlag(context->vpu_dec_info->handle, remain[i].indexFrameDisplay);
        }

        /* clear host display index */
        for (i = 0; i < MAX_FRAME_BUFFER; i++) {

            ret = VPU_DecClrDispFlag(context->vpu_dec_info->handle, i);
            log_debug("VPU_DecClrDispFlag ret(%d)\n", ret);
        }

        context->vpu_dec_info->need_stream = TRUE;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_decode_coda_seek_procedure -
         test coda device seek operation in decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_decode_coda_seek_procedure(struct test_context *context)
{
    int ret = 0;
    BOOL success = TRUE;
    DecOutputInfo output_info = {0};

    context->vpu_dec_info->need_stream = FALSE;

    while (true) {

        if (context->vpu_dec_info->need_stream) {
            if (0 > (ret = sdk_test_code_decode_streams_feeder(context))) {
                log_debug("warning coda feeder data end now ...\n");
            }

            context->vpu_dec_info->need_stream = FALSE;
        }

        if (RET_CODE_SUCCESS != (ret = sdk_test_coda_decode_one_frame_procedure(context, &output_info))) {
            log_err("coda decode start one frame errorr ret(%d)\n", ret);
            success = FALSE;
            break;
        }

        ret = sdk_test_coda_decode_output_info_get(context, &output_info);

        if (ret == RET_CODE_DECODE_CONTINUE) {
            log_err("coda decode ouput info continue ret(%d)\n", ret);
            continue;
        }

        if (ret == RET_CODE_DECODE_END) {
            log_debug("coda decode success now \n");
            break;
        }

        if (ret != RET_CODE_SUCCESS) {
            log_err("coda decode output info error ret(%d)\n", ret);
            success = FALSE;
            break;
        }

        if ((context->vpu_dec_info->dec_frame_count > 0)
                && (context->vpu_dec_info->dec_frame_count >= context->test_cfg->dec_config->forceOutNum)) {
            log_debug("coda decode display-frames(%d), decode-frames(%d)\n",
                      context->vpu_dec_info->dis_frame_count,
                      context->vpu_dec_info->dec_frame_count);
            break;
        }

        if (RET_CODE_SUCCESS != (ret = sdk_test_frame_seek_procedure(context))) {
            log_err("sdk test seek frames is error ret(%d)\n", ret);
            success = FALSE;
            break;
        }
    }

    if (context->vpu_dec_info->save_fp[0]) {
        osal_fclose(context->vpu_dec_info->save_fp[0]);
        context->vpu_dec_info->save_fp[0] = NULL;
    }

    if (success)
        return RET_CODE_SUCCESS;

    return RET_CODE_FAILURE_DEOCDE;
}

/*
 * brief sdk_test_decode_wave_seek_procedure -
         test wave device seek operation in decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_decode_wave_seek_procedure(struct test_context *context)
{
    int ret = 0;
    BOOL success = TRUE;
    DecOutputInfo output_info = {0};

    context->vpu_dec_info->need_stream = FALSE;

    while (true) {

        if (context->vpu_dec_info->need_stream) {
            if (0 > (ret = sdk_test_wave_decode_streams_feeder(context))) {
                log_debug("waring wave feeder data end now ...\n");
            }

            context->vpu_dec_info->need_stream = FALSE;
        }

        if (RET_CODE_SUCCESS != (ret = sdk_test_wave_decode_one_frame_procedure(context))) {
            log_err("coda decode start one frame errorr ret(%d)\n", ret);
            success = FALSE;
            break;
        }

        ret = sdk_test_wave_decode_output_info_get(context, &output_info);

        if (ret == RET_CODE_DECODE_CONTINUE) {
            log_debug("wave decode ouput info continue ret(%d)\n", ret);
            continue;
        }

        if (ret == RET_CODE_DECODE_END) {
            log_debug("wave decode success now \n");
            break;
        }

        if (ret != RET_CODE_SUCCESS) {
            log_err("wave decode output info error ret(%d)\n", ret);
            success = FALSE;
            break;
        }

        if ((context->vpu_dec_info->dis_frame_count > 0)
                && (context->vpu_dec_info->dis_frame_count >= context->test_cfg->dec_config->forceOutNum)) {
            log_debug("wave decode display-frames(%d), decode-frames(%d)\n",
                      context->vpu_dec_info->dis_frame_count,
                      context->vpu_dec_info->dec_frame_count);
            break;
        }

        if (RET_CODE_SUCCESS != (ret = sdk_test_frame_seek_procedure(context))) {
            log_err("sdk test seek frames is error ret(%d)\n", ret);
            success = FALSE;
            break;
        }
    }

    if (context->vpu_dec_info->save_fp[0]) {
        osal_fclose(context->vpu_dec_info->save_fp[0]);
        context->vpu_dec_info->save_fp[0] = NULL;
    }

    if (success)
        return RET_CODE_SUCCESS;

    return RET_CODE_FAILURE_DEOCDE;
}

/*
 * brief sdk_test_vpu_decode_seek_procedure - test device seek operation in decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_vpu_decode_seek_procedure(struct test_context *context)
{
    CHECK_PARAM(context);

    if (context->test_cfg->core_id == CODA_CORE_ID) {
        CALL_FUNCTION(sdk_test_decode_coda_seek_procedure(context));
    }
    else if (context->test_cfg->core_id == WAVE_CORE_ID) {
        CALL_FUNCTION(sdk_test_decode_wave_seek_procedure(context));
    }
    else {
        log_err("core id (%d) is invalid\n", context->test_cfg->core_id);
        return RET_CODE_INVALID_PARAM;
    }

    log_debug(" %s : %d vpu core_id(%d) seek success \n", __FUNCTION__, __LINE__, context->test_cfg->core_id);
    return RET_CODE_SUCCESS;
}


/*
 * brief sdk_test_vpu_seek_param_config - test seek param config
 *       seek start-time, end-time, offset-time, seek-interval frames
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes  should be a numeric integer
 */
static int sdk_test_vpu_seek_param_config(struct test_context *context)
{
    int i = 0;
    char *arg = NULL;
    char *ptr = NULL;
    char string[64] = {0};

    log_debug("\n#############SEEK INFO CONFIG#############\n");
    log_debug("please input seek info and separate with \",\"\n");
    log_debug("seek start time, seek offset, seek end time, seek frequency(frames)\n");

    scanf("%s", string);
    ptr = strtok_r(string, ",", &arg);

    while (ptr != NULL && (i < 4)) {
        switch (i) {
            case 0:
                context->vpu_dec_info->ts_start = atoi(ptr);
                context->vpu_dec_info->ts_seek = context->vpu_dec_info->ts_start;
                break;

            case 1:
                context->vpu_dec_info->ts_offset = atoi(ptr);
                break;

            case 2:
                context->vpu_dec_info->ts_end = atoi(ptr);
                break;

            case 3:
                context->vpu_dec_info->seek_frame_interval = atoi(ptr);
                break;

            default:
                log_err("input value error ptr(%s), i(%d)\n", ptr, i);
                return RET_CODE_FAILURE_SEEK;
        }

        i++;
        ptr = strtok_r(NULL, ",", &arg);
    }

    log_debug("seek-info: ts_start(%"PRId64"), ts_offset(%"PRId64"),ts_end(%"PRId64"),interval(%d)\n \n",
              context->vpu_dec_info->ts_start,
              context->vpu_dec_info->ts_offset,
              context->vpu_dec_info->ts_end,
              context->vpu_dec_info->seek_frame_interval);

    if (context->vpu_dec_info->seek_frame_interval == 0
        || context->vpu_dec_info->ts_end == 0
        || context->vpu_dec_info->ts_offset == 0
        || context->vpu_dec_info->ts_end <= context->vpu_dec_info->ts_start) {
        log_err("decode seek config info error \n");
        return RET_CODE_FAILURE_SEEK_PARAM;
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_wave_decode_loop_procedure - wave device loop decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_wave_decode_loop_procedure(struct test_context *context)
{
    int ret = 0;
    BOOL success = TRUE;
    DecOutputInfo output_info = {0};

    context->vpu_dec_info->need_stream = FALSE;

    while (context->vpu_dec_info->loop_times--) {
        while (true) {

            if (context->vpu_dec_info->need_stream) {
                if (0 > (ret = sdk_test_wave_decode_streams_feeder(context))) {
                    log_debug("waring wave feeder data end now ...\n");
                }

                context->vpu_dec_info->need_stream = FALSE;
            }

            if (RET_CODE_SUCCESS != (ret = sdk_test_wave_decode_one_frame_procedure(context))) {
                log_err("coda decode start one frame errorr ret(%d)\n", ret);
                success = FALSE;
                break;
            }

            ret = sdk_test_wave_decode_output_info_get(context, &output_info);

            if (ret == RET_CODE_DECODE_CONTINUE) {
                log_err("wave decode ouput info continue ret(%d)\n", ret);
                continue;
            }

            if (ret == RET_CODE_DECODE_END) {
                log_debug("wave decode success now \n");
                break;
            }

            if (ret != RET_CODE_SUCCESS) {
                log_err("wave decode output info error ret(%d)\n", ret);
                success = FALSE;
                break;
            }
#if 0
            if ((context->vpu_dec_info->dis_frame_count > 0)
                    && (context->vpu_dec_info->dis_frame_count >= context->test_cfg->dec_config->forceOutNum)) {
                log_debug("wave decode display-frames(%d), decode-frames(%d)\n",
                          context->vpu_dec_info->dis_frame_count,
                          context->vpu_dec_info->dec_frame_count);
                break;
            }
#endif
        }

        if (context->vpu_dec_info->save_fp[0]) {
            osal_fclose(context->vpu_dec_info->save_fp[0]);
            context->vpu_dec_info->save_fp[0] = NULL;
        }

        if (!success) {
            log_err("wave decode failure\n");
            return RET_CODE_FAILURE_DEOCDE;
        }

        log_debug("sdk test wave decode loop one time finish, loop-times(%d)\n",context->vpu_dec_info->loop_times);
        context->vpu_dec_info->need_stream = TRUE;
        /* seek to starting position */
        CALL_FUNCTION(sdk_test_frame_seek_position_start(context));
    }

    return RET_CODE_SUCCESS;
}


/*
 * brief sdk_test_coda_decode_loop_procedure - coda device loop decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_coda_decode_loop_procedure(struct test_context *context)
{

    int ret = 0;
    BOOL success = TRUE;
    DecOutputInfo output_info = {0};

    CHECK_PARAM(context);
    context->vpu_dec_info->need_stream = FALSE;

    while (context->vpu_dec_info->loop_times--) {
        while (true) {

            if (context->vpu_dec_info->need_stream) {
                if (0 > (ret = sdk_test_code_decode_streams_feeder(context))) {
                    log_debug("coda feeder data end now ...\n");
                }

                context->vpu_dec_info->need_stream = FALSE;
            }

            if (RET_CODE_SUCCESS != (ret = sdk_test_coda_decode_one_frame_procedure(context, &output_info))) {
                log_err("coda decode start one frame errorr ret(%d)\n", ret);
                success = FALSE;
                break;
            }

            ret = sdk_test_coda_decode_output_info_get(context, &output_info);

            if (ret == RET_CODE_DECODE_CONTINUE) {
                log_err("coda decode ouput info continue ret(%d)\n", ret);
                continue;
            }

            if (ret == RET_CODE_DECODE_END) {
                log_debug("coda decode success now \n");
                break;
            }

            if (ret != RET_CODE_SUCCESS) {
                log_err("coda decode output info error ret(%d)\n", ret);
                success = FALSE;
                break;
            }
        }

        if (context->vpu_dec_info->save_fp[0]) {
            osal_fclose(context->vpu_dec_info->save_fp[0]);
            context->vpu_dec_info->save_fp[0] = NULL;
        }

        if (!success) {
            log_err("sdk loop test failure, times(%d)\n", context->vpu_dec_info->loop_times);
            return RET_CODE_FAILURE_LOOP;
        }

        log_debug("sdk test coda decode loop one time finish, loop-times(%d)\n",context->vpu_dec_info->loop_times);
        context->vpu_dec_info->need_stream = TRUE;
        /* seek to starting position */
        CALL_FUNCTION(sdk_test_frame_seek_position_start(context));
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_vpu_decode_loop_param_config - config decode loop times
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_vpu_decode_loop_param_config(struct test_context  *context)
{
    char *arg = NULL;
    char *ptr = NULL;
    char string[64] = {0};
    log_debug("\n#########please input loop times##########\n");
    scanf("%s", string);
    ptr = strtok_r(string, ",", &arg);

    context->vpu_dec_info->loop_times = atoi(ptr);

    if (context->vpu_dec_info->loop_times == 0) {
        context->vpu_dec_info->loop_times = DEFAULT_TEST_TEIMS;
    }

    log_debug("sdk test get loop times(%d), atio-prt(%d)\n",
              context->vpu_dec_info->loop_times, atoi(ptr));

    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_vpu_decode_loop_procedure - vpu device loop decoding
 * param context  context info in test
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_vpu_decode_loop_procedure(struct test_context  *context)
{
    CHECK_PARAM(context);

    if (context->vpu_dec_info->core_id == CODA_CORE_ID) {
        CALL_FUNCTION(sdk_test_coda_decode_loop_procedure(context));
    }
    else if (context->vpu_dec_info->core_id == WAVE_CORE_ID) {
        CALL_FUNCTION(sdk_test_wave_decode_loop_procedure(context));
    }
    else {
        log_err("vpu decode param error core id(%d)\n", context->vpu_dec_info->core_id);
        return RET_CODE_INVALID_PARAM;
    }

    log_debug("vpu decode loop procedure success \n");
    return RET_CODE_SUCCESS;
}

/*******************************************************************************
 *                        Global function definitions                         *
 ******************************************************************************/
/*
 * @brief sdk_test_vpu_init - sdk test for vpu device init
 * @param context  context info in test
 * @return 0  success
 *         !0 failure code
 * @notes
 */
int sdk_test_vpu_init(struct test_context  *context)
{
    CHECK_PARAM(context);

    if (context->test_cfg->core_id == CODA_CORE_ID) {
        CALL_FUNCTION(sdk_test_coda_init(context));
    }
    else if (context->test_cfg->core_id == WAVE_CORE_ID) {
        CALL_FUNCTION(sdk_test_wave_init(context));
    }
    else {
        log_err("core id (%d) is invalid\n", context->test_cfg->core_id);
        return RET_CODE_INVALID_PARAM;
    }

    log_debug(" %s : %d vpu core_id(%d) init success \n", __FUNCTION__, __LINE__, context->test_cfg->core_id);
    return RET_CODE_SUCCESS;
}

/*
 * @brief sdk_test_vpu_deinit - sdk test for vpu device deinit
 * @param context  context info in test
 * @return 0  success
 *         !0 failure code
 * @notes
 */
int sdk_test_vpu_deinit(struct test_context  *context)
{
    CHECK_PARAM(context);

    if (context->test_cfg->core_id == CODA_CORE_ID) {
        CALL_FUNCTION(sdk_test_coda_deinit(context));
    }
    else if (context->test_cfg->core_id == WAVE_CORE_ID) {
        CALL_FUNCTION(sdk_test_wave_deinit(context));
    }
    else {
        log_err("core id (%d) is invalid\n", context->test_cfg->core_id);
        return RET_CODE_INVALID_PARAM;
    }

    log_debug(" %s : %d vpu deinit success \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * @brief sdk_test_vpu_decode_loop - test vpu device loop decoding
 * @param context context info in test
 * @return 0  success
 *         !0 failure code
 * @notes
 */
int sdk_test_vpu_decode_loop(struct test_context  *context)
{
    int ret = RET_CODE_SUCCESS;
    CHECK_PARAM(context);

    context->vpu_dec_info->core_id = context->test_cfg->core_id;
    context->test_cfg->is_decode = 1;
    CALL_FUNCTION(sdk_test_vpu_decode_loop_param_config(context));

    /* open device */
    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_init(context))) {
        log_err("vpu init error ret(%d)\n", ret);
        return ret;
    }

    /* feeder create & register frame buffer */
    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_preparative_create(context))) {
        log_err("sdk test preparative create failure\n");
        CALL_FUNCTION(sdk_test_vpu_deinit(context));
        return ret;
    }

    ret = sdk_test_vpu_decode_loop_procedure(context);
    sdk_test_vpu_preparative_destroy(context);
    CALL_FUNCTION(sdk_test_vpu_deinit(context));
    return ret;
}

/*
 * @brief sdk_test_vpu_decode - sdk test for decoding and seek operation
 * @param enable_seek
 *        1: seek operation in decoding
          0: decoding
 * @param context context info in test
 * @return 0  success
 *         !0 failure code
 * @notes
 */
int sdk_test_vpu_decode(struct test_context  *context, BOOL enable_seek)
{
    int ret = RET_CODE_SUCCESS;
    CHECK_PARAM(context);

    context->vpu_dec_info->core_id = context->test_cfg->core_id;
    context->test_cfg->is_decode = 1;

    if (enable_seek) {
        CALL_FUNCTION(sdk_test_vpu_seek_param_config(context));
    }

    /* open device */
    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_init(context))) {
        log_err("vpu init error ret(%d)\n", ret);
        return ret;
    }

    /* feeder create & register frame buffer */
    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_preparative_create(context))) {
        log_err("sdk test preparative create failure\n");
        CALL_FUNCTION(sdk_test_vpu_deinit(context));
        return ret;
    }

    if (enable_seek) {
        ret = sdk_test_vpu_decode_seek_procedure(context);
    }
    else {
        ret = sdk_test_vpu_decode_procedure(context);
    }

    sdk_test_vpu_preparative_destroy(context);
    CALL_FUNCTION(sdk_test_vpu_deinit(context));
    return ret;
}

/*
 * @brief sdk_test_vpu_encode - sdk test for encoding
 * @param context context info in test
 * @return 0  success
 *         !0 failure code
 * @notes
 */
int sdk_test_vpu_encode(struct test_context  *context)
{
    int ret = RET_CODE_SUCCESS;
    CHECK_PARAM(context);

    context->vpu_enc_info->core_id = context->test_cfg->core_id;
    context->test_cfg->is_encode = 1;

    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_init(context))) {
        log_err("vpu init error ret(%d)\n", ret);
        return ret;
    }

    /* feeder create & register frame buffer */
    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_preparative_create(context))) {
        log_err("sdk test preparative create failure\n");
        CALL_FUNCTION(sdk_test_vpu_deinit(context));
        return ret;
    }

    ret = sdk_test_vpu_encode_procedure(context);
    sdk_test_vpu_preparative_destroy(context);
    CALL_FUNCTION(sdk_test_vpu_deinit(context));

    return ret;
}
