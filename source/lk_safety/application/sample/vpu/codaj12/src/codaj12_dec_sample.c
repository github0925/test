/**
 * File:  codaj12_decode_sample.c
 * Brief: decode sample
 * Data:  4/29/2020
 *
 * Copyright (C) 2020 Semidrive Technology Co., Ltd.
 * Author: chentianming<tianming.chen@semidrive.com>
 *
 */

/*******************************************************************************
*                       Include header files                                 *
******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include <strings.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <storage_device.h>
#include <storage_dev_ospi.h>
#include <partition_parser.h>
#include <spi_nor_hal.h>
#include <heap.h>

#include "jpulog.h"
#include "vpu_hal.h"
#include "timer.h"
#include "getopt.h"


/*******************************************************************************
 *                        Macro definitions                                   *
 ******************************************************************************/
#define NUM_FRAME_BUF               MAX_FRAME
#define EXTRA_FRAME_BUFFER_NUM      1
#define OSPI_RESOURCE_NAME          "misc"
#define RET_SUCCESS                 0
#define YUV_DATA_ADDR               0x50000000     /* ddr addr for save yuv */
#define YUV_DATA_SAVE_SIZE          (64*1024*1024)
#define BIT_STREAM_SIZE             0x800000

#define RES_STORAGE_CARRIER     RES_OSPI_REG_OSPI1
#define OSPI_SECTOR_SIZE        4096
#define GPT_SECTOR_IDX          2
#define OSPI_RESOURCE_PT_NAME   "misc"


/*******************************************************************************
 *                        Date struct  declarations                          *
 ******************************************************************************/
typedef enum {
    FEEDING_METHOD_FIXED_SIZE,
    FEEDING_METHOD_FRAME_SIZE,      /*!<< use FFMPEG demuxer */
    FEEDING_METHOD_MAX
} FeedingMethod;

/*
 * brief
 *
 * param
 * param
 * param
 * param
 */
struct dec_config_param {
    uint32_t          out_num;
    uint32_t          checkeos;
    uint32_t          StreamEndian;
    uint32_t          FrameEndian;
    uint32_t          iHorScaleMode;
    uint32_t          iVerScaleMode;
    //ROI
    uint32_t          roiEnable;
    uint32_t          roiWidth;
    uint32_t          roiHeight;
    uint32_t          roiOffsetX;
    uint32_t          roiOffsetY;
    uint32_t          roiWidthInMcu;
    uint32_t          roiHeightInMcu;
    uint32_t          roiOffsetXInMcu;
    uint32_t          roiOffsetYInMcu;
    uint32_t          rotation;
    JpgMirrorDirection mirror;
    FrameFormat     subsample;
    PackedFormat    packedFormat;
    CbCrInterLeave  cbcrInterleave;
    uint32_t          bsSize;
    uint32_t          pixelJustification;
    uint32_t          sliceHeight;
    BOOL            sliceInterruptEnable;
    FeedingMethod   feedingMode;
    BOOL            enableThread;
    mutex_t         *plock;
    uint32_t        *pNeedDecFrms;
};

/*
 * brief  struct  yuv_data
 *        Description yuv data in memory
 * param  base save base addr
 * param
 * param
 * param
 */
struct yuv_data {
    Uint8 *base;
    uint32_t total_size;
    uint32_t max_size;
    int frame_size;
    uint32_t frame_num;
    Uint8 *current_addr;
};

/*
 * brief  struct  decode_info
 *        context info in decoding
 * param
 * param
 * param
 * param
 */
struct decode_info {
    uint32_t apiv;
    uint32_t hwv;
    uint32_t pid;
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t bufferable_frames_num;
    vpuDecOutputInfo output_param;
    vpuFrameBuffer *frame_buffer;
    uint32_t frame_index;
    struct yuv_data yuv_420;
    vpuDecOpenParam dec_open_param;
    struct dec_config_param  dec_config;
    jpu_buffer_t    bitstream_buf;
    vpuDecParam    param;
};

/* trace log-time for performance */
/*
 * brief  struct  vpu_src_t
 *
 * param
 * param
 * param
 * param
 */
struct decode_time {
    uint64_t startDecTime;
    uint64_t endDecTime;
    uint64_t averageFrameTime;
    uint64_t frameStartTime;
    uint64_t frameEndTime;
    uint64_t currentFrameTime;
    uint64_t totalFrameTime;
    uint64_t firstFrameTime;
    int  flag;
    uint32_t  fps;
};

static struct decode_time decTime = {0};


/*******************************************************************************
 *                        Local function definitions                           *
 ******************************************************************************/
/**
 *brief set_start_time
 *param void
 *return
 */
static void set_time_start_decode(struct decode_time *time)
{

    time->startDecTime = current_time();
    JLOG(TRACE, "CodaJ12 start decode time %lld ms\n", time->startDecTime);

};

/**
 *brief
 *param void
 *return
 */
static void set_time_end_decode(struct decode_time *time)
{

    time->endDecTime = current_time();
    JLOG(TRACE, "CodaJ12 end decode time %lld ms, decode cust time %lld ms \n", time->endDecTime, time->endDecTime - time->startDecTime);
}

/**
 *brief
 *param void
 *return
 */
static int64_t set_time_average_frame_time(struct decode_time *time, int frameNum)
{

    int64_t temp = 0;

    if (frameNum > 0) {
        temp = (time->totalFrameTime) / (frameNum);
        time->averageFrameTime = (time->totalFrameTime - time->firstFrameTime) / (frameNum - 1);
        JLOG(TRACE, "CodaJ12 average decode time(except first frame ) %lld ms; average decode time  %lld ms; total time %lld ms; first frame time %lld ms ; frameNum %d \n",
             time->averageFrameTime,
             temp,
             time->totalFrameTime,
             time->firstFrameTime,
             frameNum);

        return time->averageFrameTime;
    }

    JLOG(INFO, "CodaJ12 average decode time: 0 \n");
    return 0;
}

/**
 *brief
 *param void
 *return
 */
static void set_time_frame_start(struct decode_time *time)
{
    time->frameStartTime = current_time();
}

static void set_time_frame_end(struct decode_time *time)
{

    time->frameEndTime = current_time();
    time->currentFrameTime = time->frameEndTime - time->frameStartTime;
    time->totalFrameTime = time->totalFrameTime + time->currentFrameTime;

    if (0 == time->flag) {
        time->firstFrameTime = time->currentFrameTime;
        time->flag = 1;
    }

    JLOG(TRACE, "CodaJ12 current frame decode time %lld ms; start time %lld ms; end time %lld ms; totalFrame time %lld ms \n",
         time->currentFrameTime,
         time->frameStartTime,
         time->frameEndTime,
         time->totalFrameTime);
}



/**
 * brief
 *
 * param
 * return void
 * note
 */
static void callback_on_proc_fin(struct vpu_codec_instance_t *codec, void *args)
{
    JLOG(INFO, "CodaJ12 proc fin:mjpeg decoding finish.\n");
}

/**
 * brief
 *
 * param
 * return void
 * note
 */
static void callback_on_err(struct vpu_codec_instance_t *codec, void *args)
{
    JLOG(ERR, "Warning proc err cb: err happen on mjpeg proc\n");
}

/**
 * brief
 *
 * param
 * return void
 * note
 */
static void callback_on_underrun(struct vpu_codec_instance_t *codec, void *args)
{
    JLOG(ERR, "Warning proc no stream happen on mjpeg proc\n");
}

/**
 * brief
 *
 * param
 * return void
 * note
 */
static uint64_t get_gpt_location(storage_device_t *storage)
{
    return (0 + OSPI_SECTOR_SIZE * GPT_SECTOR_IDX);
}

/**
 * brief
 *
 * param
 * return success true; false failure
 * note
 */
static bool bitstream_load(const char *res_name, void *dest, int size)
{

    static struct spi_nor_cfg ospi_cfg = {
        .cs = SPI_NOR_CS0,
        .bus_clk = SPI_NOR_CLK_25MHZ,
        .octal_ddr_en = 0,
    };
    storage_device_t *storage = NULL;
    partition_device_t *ptdev  = NULL;
    unsigned long long res_on_storage = 0;
    int sz = 0;

    if (!res_name || !dest) {
        JLOG(ERR, "Err: Invalid parameters \n");
        return false;
    }

    if (NULL == (storage = setup_storage_dev(OSPI, RES_STORAGE_CARRIER, &ospi_cfg))) {
        JLOG(ERR, "Err: Get storage fail  \n");
        return false;
    }

    if (NULL == (ptdev = ptdev_setup(storage, get_gpt_location(storage)))) {
        JLOG(ERR, "Err: ptdev_setup fail  \n");
        return false;
    }

    ptdev_read_table(ptdev);
    res_on_storage = ptdev_get_offset(ptdev, res_name);
    JLOG(INFO, "CodaJ12 res_on_storage:0x%x storage %#x, ptdev %#x\n", res_on_storage, storage, ptdev);
    sz = ptdev_get_size(ptdev, res_name);

    if (size > sz) {
        JLOG(INFO, "Warning: Loading overflow! Res size:0x%x required size:0x%x\n", sz, size);
        size = sz;
    }

    /*read bitstream to dest memory */
    storage->read(storage, res_on_storage, (uint8_t *)dest, size);

    /*release storage */
    ptdev->storage->release(ptdev->storage);

    return true;
}

/**
 * brief  save yuv data
 *
 * param
 * return
 * note
 */
static int strore_yuv420_to_memory(int chroma_stride, Uint8 *dst, uint32_t pic_width, uint32_t pic_height, uint32_t bit_depth, vpuFrameBuffer *fb, int endian, CbCrInterLeave interLeave, PackedFormat packed)
{
    int size = 0;
    int y, nY = 0, nCb, nCr;
    unsigned int addr;
    int luma_size, chroma_size = 0, chroma_width = 0, chroma_height = 0;
    Uint8 *puc;
    int chroma_stride_temp = 0;
    uint32_t  bytes_pixel = (bit_depth + 7) / 8;

    chroma_stride_temp = chroma_stride;
    switch (fb->format) {
        case FORMAT_420:
            nY = pic_height;
            nCb = nCr = pic_height / 2;
            chroma_size  = (pic_width / 2) * (pic_height / 2);
            chroma_width = pic_width / 2;
            chroma_height = nY;
            break;

        default:
            return 0;
    }

    puc = dst;
    addr = fb->bufY;

    if (packed) {
        if (packed == PACKED_FORMAT_444)
            pic_width *= 3;
        else
            pic_width *= 2;

        chroma_size = 0;
    }

    luma_size = pic_width * nY;
    size = luma_size + chroma_size * 2;
    luma_size    *= bytes_pixel;
    chroma_size  *= bytes_pixel;
    size        *= bytes_pixel;
    pic_width    *= bytes_pixel;
    chroma_width *= bytes_pixel;

    if (interLeave) {
        chroma_size = chroma_size * 2;
        chroma_width = chroma_width * 2;
        chroma_stride_temp = chroma_stride_temp;
    }

    if ((pic_width == fb->stride) && (chroma_width == chroma_stride_temp)) {
        JpuReadMem(addr, (Uint8 *)(puc), luma_size, endian);

        if (packed)
            return size;

        if (interLeave) {
            puc = dst + luma_size;
            addr = fb->bufCb;
            JpuReadMem(addr, (Uint8 *)(puc), chroma_size, endian);
        }
        else {
            puc = dst + luma_size;
            addr = fb->bufCb;
            JpuReadMem(addr, (Uint8 *)(puc), chroma_size, endian);

            puc = dst + luma_size + chroma_size;
            addr = fb->bufCr;
            JpuReadMem(addr, (Uint8 *)(puc), chroma_size, endian);
        }
    }
    else {
        JLOG(INFO, "nY=%d\n", nY);

        for (y = 0; y < nY; ++y) {
            JpuReadMem(addr + fb->stride * y, (Uint8 *)(puc + y * pic_width), pic_width,  endian);
        }

        if (packed)
            return size;

        if (interLeave) {
            JLOG(INFO, "nC=%d\n", chroma_height / 2);

            puc = dst + luma_size;
            addr = fb->bufCb;

            for (y = 0; y < (chroma_height / 2); ++y) {
                JpuReadMem(addr + (chroma_stride_temp)*y, (Uint8 *)(puc + y * (chroma_width)), (chroma_width), endian);
            }
        }
        else {
            puc = dst + luma_size;
            addr = fb->bufCb;

            for (y = 0; y < nCb; ++y) {
                JpuReadMem(addr + chroma_stride_temp * y, (Uint8 *)(puc + y * chroma_width), chroma_width,  endian);
            }

            puc = dst + luma_size + chroma_size;
            addr = fb->bufCr;

            for (y = 0; y < nCr; ++y) {
                JpuReadMem(addr + chroma_stride_temp * y, (Uint8 *)(puc + y * chroma_width), chroma_width,  endian);
            }
        }
    }
    return size;
}


/**
 * brief  set decode configuraiton with user's config
 *
 * param
 * return
 * note
 */
static void set_dec_param(struct decode_info *dec_info)
{

    (dec_info->dec_open_param).streamEndian          = dec_info->dec_config.StreamEndian;
    (dec_info->dec_open_param).frameEndian           = dec_info->dec_config.FrameEndian;
    (dec_info->dec_open_param).bitstreamBuffer       = ((dec_info->bitstream_buf).phys_addr);   /*jpu view */
    (dec_info->dec_open_param).bitstreamBufferSize   = (dec_info->bitstream_buf).size;
    (dec_info->dec_open_param).pBitStream            = (BYTE *)(dec_info->bitstream_buf).virt_addr;
    (dec_info->dec_open_param).chromaInterleave      = (dec_info->dec_config).cbcrInterleave;
    (dec_info->dec_open_param).packedFormat          = (dec_info->dec_config).packedFormat;
    (dec_info->dec_open_param).roiEnable             = (dec_info->dec_config).roiEnable;
    (dec_info->dec_open_param).roiOffsetX            = (dec_info->dec_config).roiOffsetX;
    (dec_info->dec_open_param).roiOffsetY            = (dec_info->dec_config).roiOffsetY;
    (dec_info->dec_open_param).roiWidth              = (dec_info->dec_config).roiWidth;
    (dec_info->dec_open_param).roiHeight             = (dec_info->dec_config).roiHeight;
    (dec_info->dec_open_param).rotation              = (dec_info->dec_config).rotation;
    (dec_info->dec_open_param).mirror                = (dec_info->dec_config).mirror;
    (dec_info->dec_open_param).pixelJustification    = (dec_info->dec_config).pixelJustification;
    (dec_info->dec_open_param).outputFormat          = (dec_info->dec_config).subsample;
    (dec_info->dec_open_param).intrEnableBit         = ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_BIT_BUF_EMPTY));

    JLOG(INFO, "\n #########################decode config info ##########################\n");
    JLOG(INFO, "streamEndian:         %d\n", (dec_info->dec_open_param).streamEndian);
    JLOG(INFO, "frameEndian:          %d\n", (dec_info->dec_open_param).frameEndian);
    JLOG(INFO, "bitstreamBuffer:      %#x\n", (dec_info->dec_open_param).bitstreamBuffer);
    JLOG(INFO, "bitstreamBufferSize:  %#x\n", (dec_info->dec_open_param).bitstreamBufferSize);
    JLOG(INFO, "pBitStream:           %#x\n", (dec_info->dec_open_param).pBitStream);
    JLOG(INFO, "chromaInterleave:     %d\n", (dec_info->dec_open_param).chromaInterleave);
    JLOG(INFO, "packedFormat:         %d\n", (dec_info->dec_open_param).packedFormat);
    JLOG(INFO, "roiEnable:            %d\n", (dec_info->dec_open_param).roiEnable);
    JLOG(INFO, "roiOffsetX:           %d\n", (dec_info->dec_open_param).roiOffsetX);
    JLOG(INFO, "roiOffsetY:           %d\n", (dec_info->dec_open_param).roiOffsetY);
    JLOG(INFO, "roiWidth:             %d\n", (dec_info->dec_open_param).roiWidth);
    JLOG(INFO, "roiHeight:            %d\n", (dec_info->dec_open_param).roiHeight);
    JLOG(INFO, "rotation:             %d\n", (dec_info->dec_open_param).rotation);
    JLOG(INFO, "mirror:               %d\n", (dec_info->dec_open_param).mirror);
    JLOG(INFO, "pixelJustification:   %d\n", (dec_info->dec_open_param).pixelJustification);
    JLOG(INFO, "outputFormat:         %d\n", (dec_info->dec_open_param).outputFormat);
    JLOG(INFO, "intrEnableBit:        %d\n", (dec_info->dec_open_param).intrEnableBit);
    JLOG(INFO, "out_num:               %d\n", dec_info->dec_config.out_num);
}



/**
 * brief  only decode one frame
 *
 * param
 * return success 0; false  fail value
 * note
 */
static int decode_one_frame(void *res_handle, vpu_codec_handle_t dec_handle, vpuDecParam *decParam, vpuDecOutputInfo *output_info)
{
    int ret = JPG_RET_SUCCESS;
    int reason = 0;

    set_time_frame_start(&decTime);
    ret = hal_vpu_dec_start_one_frame(res_handle, dec_handle, decParam);

    if (ret != JPG_RET_SUCCESS && ret != JPG_RET_EOS) {
        if (ret == JPG_RET_BIT_EMPTY) {
            JLOG(TRACE, "BITSTREAM NOT ENOUGH....feedSize(%d)\n");
            return ret;
        }

        JLOG(ERR, "Err: JPU_DecStartOneFrame failed Error code is 0x%x \n", ret);
        return ret;
    }

    /*file end */
    if (ret == JPG_RET_EOS) {
        hal_vpu_dec_get_output_info(res_handle, dec_handle, output_info, 0);
        return ret;
    }

    /*wait interruput */
    if (ret == JPG_RET_SUCCESS) {
        reason = hal_vpu_wait_interrupt(res_handle, dec_handle, 5000);
        JLOG(TRACE, "%s : %d reson %x \n", __FUNCTION__, __LINE__, reason);

        /*stream end */
        if (reason & (1 << INT_JPU_BIT_BUF_EMPTY)) {
            JLOG(TRACE, "%s : %d\n", __FUNCTION__, __LINE__);
            return JPG_RET_BIT_EMPTY;
        }

        if (reason & (1 << INT_JPU_ERROR)) {
            JLOG(ERR, "Err: JPU_DecStartOneFrame failed Error code is 0x%x \n", ret);
            return (1 << INT_JPU_ERROR);
        }
    }
    else {
        /*some error happen */
        JLOG(ERR, "Err: happen now ...\n");
        return JPG_RET_FAILURE;
    }

    /*get info of decode*/
    if ((ret = hal_vpu_dec_get_output_info(res_handle, dec_handle, output_info, reason)) != JPG_RET_SUCCESS) {
        JLOG(ERR, "Err: JPU_DecGetOutputInfo failed Error code is 0x%x \n", ret);
        return JPG_RET_FAILURE;
    }

    /* condition of decode success */
    if (output_info->decodingSuccess == 0) {
        JLOG(ERR, "Err: JPU_DecGetOutputInfo decode fail framdIdx \n");
        return JPG_RET_FAILURE;
    }

    JLOG(TRACE, "%8d     %8x %8x %10d  %8x  %8x %10d\n",
         output_info->indexFrameDisplay,
         output_info->bytePosFrameStart,
         output_info->ecsPtr,
         output_info->consumedByte,
         output_info->rdPtr,
         output_info->wrPtr,
         output_info->frameCycle);

    set_time_frame_end(&decTime);
    return JPG_RET_SUCCESS;

}

/*
 *brief  sample_decode: decoder in mjpeg format program.
 *param  param : user's configuraiton for decode
 *return void
 *notes  1:This is just a sample example of decoding, please follow steps
 *         below to write your own specific application code.
 *         More detail please refer to codaj12-program-guide.
 *       2:Bitstream is burned to ospi partition.
 *         Output-yuv save in DDR
 *
 */
static void sample_decode(struct dec_config_param *param)
{
    struct decode_info  *dec_info = NULL;
    vpu_codec_handle_t dec_handle = NULL;
    void *res_handle = NULL;
    int ret = 0;
    vpuDecInitialInfo header = {0};

    if (!param) {
        JLOG(ERR, "Err: param null \n");
        return;
    }

    if (NULL == (dec_info = (struct decode_info *)malloc(sizeof(struct decode_info)))) {
        JLOG(ERR, "Err: malloc memory fail. \n");
        goto DEC_END;
    }

    memset(dec_info, 0, sizeof(struct decode_info));
    memcpy(&(dec_info->dec_config), param, sizeof(struct dec_config_param));

    JLOG(INFO, "Codaj12 start mjpeg decode ...\n");

    /* Step 1 create resource handle */
    if (!(hal_vpu_create_handle((void **)&res_handle, RES_MJPEG_MJPEG))) {
        JLOG(ERR, "Err : Create vpu handle fail.\n");
        goto DEC_END;
    }

    /* Step 2 vpu init */
    if (RET_SUCCESS != (ret = hal_vpu_init(res_handle))) {
        JLOG(ERR, "Err: vpu handle init fail. vaule %d\n", ret);
        goto DEC_END;
    }

    hal_vpu_get_version(res_handle, &(dec_info->apiv), &(dec_info->hwv), &(dec_info->pid));
    JLOG(INFO, "CodaJ12 ver Info: API :%x IP ver:%x Product ID:%x\n", dec_info->apiv, dec_info->hwv, dec_info->pid);

    /* bitsteam buffer must be aligned 1024 */
    (dec_info->bitstream_buf).size = BIT_STREAM_SIZE;

    /* Step 3 alloate bitstream buffer */
    if (RET_SUCCESS != (ret = hal_vpu_allocate_dma_memory(&(dec_info->bitstream_buf)))) {
        JLOG(ERR, "Err: alloate memory fail vaule %d\n", ret);
        goto DEC_END;
    }

    /* load bitstream from ospi to memory */
    if (!(bitstream_load(OSPI_RESOURCE_NAME, (void *)((dec_info->bitstream_buf).virt_addr), (dec_info->bitstream_buf).size))) {
        JLOG(ERR, "Err : Load bitstream fail.\n");
        goto DEC_END;
    }

    /* set dec_open_param config with user's configuraiton */
    set_dec_param(dec_info);

    /*Step 4 decode instance open */
    if (RET_SUCCESS != (ret = hal_vpu_dec_open(res_handle, &dec_handle, &(dec_info->dec_open_param)))) {
        JLOG(ERR, "Err: open instance fail value %d\n", ret);
        goto DEC_END;
    }

    /* Step 5 register callback function */
    hal_vpu_register_callback_on_fin(res_handle, dec_handle, callback_on_proc_fin, NULL);
    hal_vpu_register_callback_on_err(res_handle, dec_handle, callback_on_err, NULL);

    /* Step 6 updata stream to jpu */
    /*
     * Note: first: please burned bitstream to ospi
     *       second: load bitstream to bitstream buffer
     *               bitstream_buf.phy = ospi
     */
    if (RET_SUCCESS != (ret = hal_vpu_dec_update_bitstream_buffer(res_handle, dec_handle, BIT_STREAM_SIZE - 1))) {
        JLOG(ERR, "Err: Update bitstream buffer fail value %d\n", ret);
        goto DEC_END;
    }

    /* Step 7 get decodec init-info */
    if (RET_SUCCESS != (ret = hal_vpu_dec_get_init_info(res_handle, dec_handle, &header))) {
        JLOG(ERR, "Err: Get initinfo fail value %d\n", ret);
        goto DEC_END;
    }

    /* Step 8 calculate frame buffer info(size,stride)*/
    if (header.sourceFormat == FORMAT_420 || header.sourceFormat == FORMAT_422) {
        dec_info->frame_width = VPU_CEIL(16, header.picWidth);
        dec_info->frame_height = VPU_CEIL(16, header.picHeight);
    }
    else {
        dec_info->frame_width = VPU_CEIL(8, header.picWidth);
        dec_info->frame_height = VPU_CEIL(8, header.picHeight);
    }

    /* print debug init-info */
    JLOG(INFO, "init info:\n");
    JLOG(INFO, "pic width:%d\n", header.picWidth);
    JLOG(INFO, "pic height:%d\n", header.picHeight);
    JLOG(INFO, "minimal frame:%d\n", header.minFrameBufferCount);
    JLOG(INFO, "bitdph:%d\n", header.bitDepth);
    JLOG(INFO, "src format:%d\n", header.sourceFormat);

    /* Step 9 alloate output framebuffer's memory */
    dec_info->yuv_420.frame_size = hal_vpu_create_frame_buffer((dec_info->dec_open_param).outputFormat,
                                   (dec_info->dec_open_param).chromaInterleave,
                                   (dec_info->dec_open_param).packedFormat,
                                   (dec_info->dec_open_param).frameEndian,
                                   (dec_info->dec_open_param).rotation,
                                   FALSE,
                                   dec_info->frame_width,
                                   dec_info->frame_height,
                                   header.bitDepth,
                                   header.minFrameBufferCount + 1, /*use two buffer. (a/b) buffer */
                                   & (dec_info->frame_buffer));

    /* Step 10 register framebufers to jpu */
    if (RET_SUCCESS != (ret = hal_vpu_dec_register_framebuffer(res_handle, dec_handle, dec_info->frame_buffer, header.minFrameBufferCount + 1, (dec_info->frame_buffer)[0].stride))) {
        JLOG(ERR, "Err: register frame buffer fail value %d\n", ret);
        goto DEC_END;
    }

    dec_info->bufferable_frames_num = header.minFrameBufferCount + 1;

    /* Step 11 allocate memory to save data have been decoded  */
    /*
     * Note:
     * 1: safety run only in sram1, 256K mem.
     * 2: save yuv data to ddr, if only safety
     */
#if WITH_KERNEL_VM
    dec_info->yuv_420.base  = (Uint8 *)paddr_to_kvaddr(YUV_DATA_ADDR);
#else
    dec_info->yuv_420.base = (Uint8 *)YUV_DATA_ADDR;
#endif
    dec_info->yuv_420.max_size = YUV_DATA_SAVE_SIZE;   /* Max 64 MB to save yuv data*/
    dec_info->yuv_420.current_addr = dec_info->yuv_420.base;
    set_time_start_decode(&decTime);

    /* Step 12 start decode one frame */
    while (true) {
        int save_idx = 0;
        FrameBuffer  *fb = NULL;
        ret =  decode_one_frame(res_handle, dec_handle, &(dec_info->param), &(dec_info->output_param));

        if (ret != JPG_RET_SUCCESS) {
            JLOG(ERR, "Err: decode frame some erro vaule %d\n", ret);
            break;
        }

        /*Step 12.1 save yuv data; if have filesystem, can save into file */
        save_idx = dec_info->output_param.indexFrameDisplay;
        fb = &(dec_info->frame_buffer[save_idx]);
        dec_info->yuv_420.frame_size = strore_yuv420_to_memory(fb->strideC, dec_info->yuv_420.current_addr, header.picWidth, header.picHeight, header.bitDepth,
                                                                    fb, 0, 0, 0);
        if (dec_info->yuv_420.frame_size <= 0) {
            JLOG(ERR, "Err: save ouput0-data error\n");
            break;
        }

        dec_info->yuv_420.current_addr = (Uint8 *) dec_info->yuv_420.base + dec_info->yuv_420.frame_size;
        dec_info->yuv_420.frame_num++;
        dec_info->yuv_420.total_size = dec_info->yuv_420.total_size + dec_info->yuv_420.frame_size;

        if (dec_info->yuv_420.frame_num == dec_info->dec_config.out_num || (int)(dec_info->yuv_420.current_addr) >= YUV_DATA_ADDR + YUV_DATA_SAVE_SIZE) {
            JLOG(INFO, "Codaj12 decode done, frames nums %d , frame size %#x , total size %d\n",
                 dec_info->yuv_420.frame_num,
                 dec_info->yuv_420.frame_size,
                 dec_info->yuv_420.total_size);
            break;
        }
    }

    set_time_end_decode(&decTime);

DEC_END:

    /* Step 13 un-init resource */
    /* frame_buffer must free before vpu_deinit  */
    if (dec_info && dec_info->frame_buffer)  {
        hal_vpu_free_frame_buffer(dec_info->frame_buffer);
        memset(dec_info, 0, sizeof(struct decode_info));
    }

    if (res_handle) {
        hal_vpu_dec_close(res_handle, dec_handle);
        hal_vpu_deinit(res_handle);
        hal_vpu_release_handle(res_handle);
        dec_handle = NULL;
        res_handle = NULL;
    }

    if (dec_info) {
        free(dec_info);
        dec_info = NULL;
    }

}

/*
 * brief  help function
 *
 * param
 * return
 * note
 */
static void Help(const char *programName)
{
    JLOG(INFO, "------------------------------------------------------------------------------\n");
    JLOG(INFO, " CODAJ12 Decoder\n");
    JLOG(INFO, "------------------------------------------------------------------------------\n");
    JLOG(INFO, "%s [options] --input=jpg_file_path\n", programName);
    JLOG(INFO, "-h                      help\n");
    JLOG(INFO, "-out_num                 decode frame Num  default = 5 \n");
    JLOG(INFO, "--stream-endian=ENDIAN  bitstream endianness. refer to datasheet Chapter 4.\n");
    JLOG(INFO, "--frame-endian=ENDIAN   pixel endianness of 16bit input source. refer to datasheet Chapter 4.\n");
    JLOG(INFO, "--pixelj=JUSTIFICATION  16bit-pixel justification. 0(default) - msb justified, 1 - lsb justified in little-endianness\n");
    JLOG(INFO, "--bs-size=SIZE          bitstream buffer size in byte\n");
    JLOG(INFO, "--roi=                  ROI region  0 disalbe 1: enable default 0\n");
    JLOG(INFO, "--subsample             conversion sub-sample(ignore case): NONE, 420, 422, 444\n");
    JLOG(INFO, "--rotation              0, 90, 180, 270\n");
    JLOG(INFO, "--mirror                0(none), 1(V), 2(H), 3(VH)\n");
    JLOG(INFO, "--scaleH                Horizontal downscale: 0(none), 1(1/2), 2(1/4), 3(1/8)\n");
    JLOG(INFO, "--scaleV                Vertical downscale  : 0(none), 1(1/2), 2(1/4), 3(1/8)\n");
    JLOG(INFO, "--roi_x                 roi x start\n");
    JLOG(INFO, "--roi_y                 roi y start\n");
    JLOG(INFO, "--roi_w                 roi weight\n");
    JLOG(INFO, "--roi_h                 roi height\n");
    JLOG(INFO, "--packformat            format   0:420 1:422yuyv 2: 422uyvy 3: 422yvyu 4: vyuy  5: 444\n");
    //exit(0);
}


/*******************************************************************************
 *                        Global function definitions                         *
 ******************************************************************************/
/*
 * @brief  main entry for decoding
 *
 * @param
 * @return
 * @note
 */
int main_dec(int argc, char **argv)
{
    int  c, index;
    static  struct option options[] = {
        { "stream-endian",      required_argument, NULL, 0 },
        { "frame-endian",       required_argument, NULL, 0 },
        { "output",             required_argument, NULL, 0 },
        { "input",              required_argument, NULL, 0 },
        { "pixelj",             required_argument, NULL, 0 },
        { "bs-size",            required_argument, NULL, 0 },
        { "roi",                required_argument, NULL, 0 },
        { "subsample",          required_argument, NULL, 0 },
        { "rotation",           required_argument, NULL, 0 },
        { "mirror",             required_argument, NULL, 0 },
        { "scaleH",             required_argument, NULL, 0 },
        { "scaleV",             required_argument, NULL, 0 },
        { "slice-height",       required_argument, NULL, 0 },
        { "enable-slice-intr",  required_argument, NULL, 0 },
        { "roi_x",              required_argument, NULL, 0 },
        { "roi_y",              required_argument, NULL, 0 },
        { "roi_w",              required_argument, NULL, 0 },
        { "roi_h",              required_argument, NULL, 0 },
        { "packformat",         required_argument, NULL, 0 },
        { NULL,                 no_argument,       NULL, 0 },
    };

    char *short_opt = (char *)"hn:";
    struct dec_config_param config = {0};
    config.subsample = FORMAT_420;
    config.StreamEndian = 0;
    config.FrameEndian = 0;
    config.out_num = 5;

    optind = 1;
    while ((c = getopt_long(argc, argv, short_opt, options, &index)) != -1) {
        switch (c) {
            case '?':
            case 'h':
                Help(argv[0]);
                return 1;

            case 'n':
                config.out_num = atoi(optarg);
                break;

            case 0:
                switch (index) {
                    case 0:
                        config.StreamEndian = atoi(optarg);
                        break;

                    case 1:
                        config.FrameEndian = atoi(optarg);
                        break;

                    case 2:
                        config.pixelJustification = atoi(optarg);
                        break;

                    case 3:
                        config.bsSize = atoi(optarg);
                        break;

                    case 4:
                        config.roiEnable = atoi(optarg);
                        break;

                    case 5:
                        config.subsample = atoi(optarg);
                        break;

                    case 6:
                        config.rotation = atoi(optarg);
                        break;

                    case 7:
                        config.mirror = atoi(optarg);
                        break;

                    case 8:
                        config.iHorScaleMode = atoi(optarg);
                        break;

                    case 9:
                        config.iVerScaleMode = atoi(optarg);
                        break;

                    case 10:
                        config.sliceHeight = atoi(optarg);
                        break;

                    case 11:
                        config.sliceInterruptEnable = atoi(optarg);
                        break;

                    case 12:
                        config.roiOffsetX = atoi(optarg);
                        break;

                    case 13:
                        config.roiOffsetY = atoi(optarg);
                        break;

                    case 14:
                        config.roiWidth = atoi(optarg);
                        break;

                    case 15:
                        config.roiHeight = atoi(optarg);
                        break;

                    case 16:
                        config.packedFormat = atoi(optarg);
                        break;
                }

                break;

            default:
                Help(argv[0]);
                return 1;
        }
    }

    /* CHECK PARAMETERS */
    if ((config.iHorScaleMode || config.iVerScaleMode) && config.roiEnable) {
        JLOG(ERR, "Err : Invalid operation mode : ROI cannot work with the scaler\n");
        return 1;
    }

    if (config.packedFormat && config.roiEnable) {
        JLOG(ERR, "Err : Invalid operation mode : ROI cannot work with the packed format conversion\n");
        return 1;
    }

    if (config.roiEnable && (config.rotation || config.mirror)) {
        JLOG(ERR, "Err : Invalid operation mode : ROI cannot work with the PPU.\n");
        return 1;
    }

    /* create decode thread */
    thread_t *decoder = thread_create("decoder", (void *)sample_decode, &config, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE * 2);
    thread_detach(decoder);
    thread_resume(decoder);

    JLOG(INFO, "%s:%d exit now ...\n", __FUNCTION__, __LINE__);
    return 0;
}


