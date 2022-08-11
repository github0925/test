#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <app.h>
#include <lib/console.h>
#include <chip_res.h>
#include "vpu_hal.h"
#include <task.h>
#include <queue.h>
#include <heap.h>

#include "data_structure_def.h"
#include "animation_config.h"
#include "container.h"
#include "res_loader.h"
#include "early_app_cfg.h"



static void* vres_handle = NULL;

typedef struct
{
    container_handle_t container;
    reference_handle_t reference;
}buffer_pool_t;



/**************************** Default configuration table ******************************/

static vpuDecOpenParam Gvpu_decoder_config_table =
{
    (PhysicalAddress)NULL, //bs phy address
    0, // bs size
    NULL, //bs virt address
    JDI_LITTLE_ENDIAN, //bs endian
    JDI_LITTLE_ENDIAN, //frame endian
    CBCR_SEPARATED, //cbcr interleave
    false, //thumb nail enable
    PACKED_FORMAT_NONE, //packed format
    false, //region of insertion
    0, //ROI X
    0, //ROI Y
    0, //ROI width
    0, //ROI height
    PIXEL_16BIT_MSB_JUSTIFIED, //default pixel justification
    0, //slice height
    ((1<<INT_JPU_DONE) | (1<<INT_JPU_ERROR) ), //enable pic done/error
    0, //no rotation
    0, //no mirror
    FORMAT_420, //frame format
    false, //slice mode
};

static vpuDecParam Gvpu_pic_config_table =
{
    0, //scaleDownRatioWidth
    0, //scaleDownRatioHeight
};

/**************************** Default configuration table end ******************************/

static void callback_on_proc_fin(struct vpu_codec_instance_t* codec,void* args)
{
    vpuDecOutputInfo* outputInfo = (vpuDecOutputInfo*)args;
    USDBG("proc fin:mjpeg decoding finish.\n");
    hal_vpu_dec_get_output_info(vres_handle, codec, outputInfo, VPU_INT_PROC_DONE);
}

static void callback_on_err(struct vpu_codec_instance_t* codec,void* args)
{
    vpuDecOutputInfo outputInfo;
    USDBG("proc err cb: err happen on mjpeg proc\n");
    hal_vpu_dec_get_output_info(vres_handle, codec, &outputInfo, VPU_INT_PROC_ERR);
    USDBG("%02d %04d  %8d     %8x %8x %10d  %8x  %8x %10d\n",
            0, outputInfo.indexFrameDisplay, outputInfo.indexFrameDisplay, outputInfo.bytePosFrameStart, outputInfo.ecsPtr, outputInfo.consumedByte, outputInfo.rdPtr, outputInfo.wrPtr, outputInfo.frameCycle);

}

static int proc_single_frame(vpu_codec_handle_t dec_handle,vpuFrameBuffer* frame_buffer,vpuDecOutputInfo* proc_res,vpu_rsp_t* vpu_rsp)
{
    vpuRet decret = hal_vpu_dec_start_one_frame(vres_handle, dec_handle, &Gvpu_pic_config_table);
    int frame_idx = 0;
    int iWaitInt = 0;
    // USDBG("start ret:%d\n",decret);

    if(decret == JPG_RET_SUCCESS)
    {
        iWaitInt = hal_vpu_wait_interrupt(vres_handle,dec_handle,300);
        if(iWaitInt == -1)
            return -1;
        frame_idx = proc_res->indexFrameDisplay;
        USDBG("frm idx:%d\n",frame_idx);

        USDBG("frame Y addr:0x%x,CB addr:0x%x,CR addr:0x%x,frm stride:%d\n",
        frame_buffer[frame_idx].bufY,
        frame_buffer[frame_idx].bufCb,
        frame_buffer[frame_idx].bufCr,
        frame_buffer[frame_idx].stride);
        //convert from vpu view to cpu view
        vpu_rsp->bufY = ap2p(frame_buffer[frame_idx].bufY);
        vpu_rsp->bufCb = ap2p(frame_buffer[frame_idx].bufCb);
        vpu_rsp->bufCr = ap2p(frame_buffer[frame_idx].bufCr);
        vpu_rsp->strideY = frame_buffer[frame_idx].stride;
        vpu_rsp->width = proc_res->decPicWidth;
        vpu_rsp->height = proc_res->decPicHeight;
        vpu_rsp->end = false;

        return frame_idx;
    }
    else
    {
        USDBG("dec single frame non-succ.: %d\n",decret);
        return -1;
    }



}

uint8_t* animation_bs = NULL;


void mjpeg_decoder_task(token_handle_t token)
{
    int ret = 0,frame_idx;
    uint32_t decret;
    uint32_t apiv,hwv,pid;
    vpu_codec_handle_t dec_handle;
    vpuDecInitialInfo header;
    vpuDecParam pic_config;
    vpuDecOutputInfo proc_result;
    vpuFrameBuffer* frame_buffer;
    vpu_buffer_t bs_buffer;
    uint32_t frame_width,frame_height,bufferable_frames_num;
    vpu_rsp_t vpu_rsp;
    buffer_pool_t*   buffered_frames = NULL;

    USDBG("start mjpeg decoder task.\n");

    int ba_video_size = ROUNDUP(res_size(BA_PATH),1024);

    animation_bs = memalign(1024,ba_video_size);

    if(!animation_bs)
    {
        USDBG("Create animation bitstream fail.\n");
        goto FINTOKEN;
    }

    res_load(BA_PATH,animation_bs,ba_video_size,0);

    if(hal_vpu_create_handle((void **)&vres_handle, RES_MJPEG_MJPEG) != true)
    {
        USDBG("create vpu handle fail.\n");
        free(animation_bs);
        goto FINTOKEN;
    }
    if(hal_vpu_init(vres_handle) != 0)
    {
        USDBG("vpu handle init fail.\n");
        free(animation_bs);
        hal_vpu_deinit(vres_handle);
        goto FINTOKEN;
    }
    hal_vpu_get_version(vres_handle, &apiv, &hwv, &pid);
    USDBG("VPU ver Info: API ver:%x IP ver:%x Product ID:%x\n",apiv,hwv,pid);

    Gvpu_decoder_config_table.bitstreamBuffer = p2ap((paddr_t)animation_bs);
    Gvpu_decoder_config_table.bitstreamBufferSize = ba_video_size;
    Gvpu_decoder_config_table.pBitStream = (BYTE*)animation_bs;

    // USDBG("bs buffer created. Addr @ %x size: %x\n",bs_buffer.phys_addr,bs_buffer.size);

    if(hal_vpu_dec_open(vres_handle, &dec_handle, &Gvpu_decoder_config_table) != 0){
        USDBG("dec open fail\n");
        free(animation_bs);
        hal_vpu_deinit(vres_handle);
        hal_vpu_release_handle(vres_handle);
        goto FINTOKEN;
    }

    hal_vpu_register_callback_on_fin(vres_handle,dec_handle,callback_on_proc_fin,&proc_result);
    hal_vpu_register_callback_on_err(vres_handle,dec_handle,callback_on_err,NULL);

    //todo: if we have a dynamic bitstream feeder, we should invoke
    //this feeder then call update interface with feeded size - 1.
    //strm->pop(strm,item);
    decret = hal_vpu_dec_update_bitstream_buffer(vres_handle,dec_handle,ba_video_size-1);
    USDBG("update bs buffer ret:%d\n",decret);
    //bitstream manipulation
    decret = hal_vpu_dec_get_init_info(vres_handle, dec_handle, &header);
    USDBG("get init info:%d\n",decret);

    if (header.sourceFormat == FORMAT_420 || header.sourceFormat == FORMAT_422)
    {
        frame_width = VPU_CEIL(32, header.picWidth);
        frame_height = VPU_CEIL(32, header.picHeight);
    }
    else
    {
        frame_width = VPU_CEIL(8, header.picWidth);
        frame_height = VPU_CEIL(8, header.picHeight);
    }

    USDBG("init info:\n");
    USDBG("pic width:%d\n",header.picWidth);
    USDBG("pic height:%d\n",header.picHeight);
    USDBG("minimal frame:%d\n",header.minFrameBufferCount);
    USDBG("bitdph:%d\n",header.bitDepth);
    USDBG("src format:%d\n",header.sourceFormat);

    //frame buffer allocation
    decret = hal_vpu_create_frame_buffer(Gvpu_decoder_config_table.outputFormat,
                                Gvpu_decoder_config_table.chromaInterleave,
                                Gvpu_decoder_config_table.packedFormat,
                                Gvpu_decoder_config_table.frameEndian,
                                Gvpu_decoder_config_table.rotation,
                                FALSE,
                                frame_width,
                                frame_height,
                                header.bitDepth,
                                header.minFrameBufferCount+1, //mjpeg default is 1. we use a/b buffering.
                                &frame_buffer);

    USDBG("allocate frm buf size per frame:%d\n",decret);

    if(!decret)
    {
        goto FINALE;
    }

    //register frame buffer to vpu for its dma usage
    decret = hal_vpu_dec_register_framebuffer(vres_handle, dec_handle, frame_buffer, header.minFrameBufferCount+1, frame_buffer[0].stride);
    USDBG("reg frame buf:%d\n",decret);

    bufferable_frames_num = header.minFrameBufferCount+1;

    //create frame output structure for post use
    buffered_frames = malloc(sizeof(buffer_pool_t)* bufferable_frames_num);

    //we fill the buffered frame area first
    for(uint32_t i=0;i<bufferable_frames_num;i++)
    {
        //create reference container to contain buffered frame info
        buffered_frames[i].container = container_create(token,true,&buffered_frames[i].reference);
    }

    while(1)
    {
        if(token_getstatus(token) == TOKEN_ABNORMAL)
        {
            break;
        }

#if LOOP_PLAYBACK == 1
_LOOP_PLAYBACK:
#endif

        frame_idx = proc_single_frame(dec_handle,frame_buffer,&proc_result,&vpu_rsp);
        if(frame_idx >= 0)
        {
            container_carryon(buffered_frames[frame_idx].container,&vpu_rsp,sizeof(vpu_rsp));
            container_give(token,buffered_frames[frame_idx].container,true);
        }
        else
        {
#if LOOP_PLAYBACK == 1
            hal_vpu_dec_set_bitstream_ptr(vres_handle,dec_handle,Gvpu_decoder_config_table.bitstreamBuffer,0);
            goto _LOOP_PLAYBACK;
#else
            USDBG("decoder fin.\n");
            break;
#endif
        }

        container_wait_dereferenced(token,true);

    }

    for(uint32_t i=0;i<bufferable_frames_num;i++)
    {
        container_destroy(token,buffered_frames[i].container);
    }


    free(buffered_frames);
    hal_vpu_free_frame_buffer(frame_buffer);

FINALE:

    vpu_rsp.end = true;
    container_handle_t c = container_create(token,false,NULL);
    container_carryon(c,&vpu_rsp,sizeof(vpu_rsp));
    container_give(token,c,true);
    container_destroy(token,c);

    hal_vpu_dec_close(vres_handle,dec_handle);
    free(animation_bs);
    hal_vpu_deinit(vres_handle);
    hal_vpu_release_handle(vres_handle);

FINTOKEN:
    token_setstatus(token,TOKEN_FIN);

}
