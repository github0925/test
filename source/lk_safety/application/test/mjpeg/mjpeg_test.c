#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <strings.h>

#include <app.h>
#include <vpu_hal.h>
#include <res.h>
#include <trace.h>
#include <heap.h>
#include <chip_res.h>
#include <spi_nor_hal.h>
#include <res_loader.h>
#include <lib/console.h>
#include <lib/slt_module_test.h>

#define LOCAL_TRACE 0

#define TEST_ANIMATION_DATA_RUN_ON_RAM_SIZE    (0xA00000) //start @ 10m

#define MJPEG_TEST_BIN_PATH "early_app/BootAnimation/mjpeg_green_pic.bin"

typedef enum mjpeg_test_result {
    MJPEG_TEST_RESULT_VALUE_PASS  = 0x0,
    MJPEG_TEST_RESULT_VALUE_NO_MEM,
    MJPEG_TEST_RESULT_VALUE_RES_LOAD_FAIL,
    MJPEG_TEST_RESULT_VALUE_CREAT_HANDLE_FAIL,
    MJPEG_TEST_RESULT_VALUE_VPU_INIT_FAIL,
    MJPEG_TEST_RESULT_VALUE_VPU_DEC_FAIL,
    MJPEG_TEST_RESULT_VALUE_PATTERN_FAIL,
} mjpeg_test_result_t;

/**************************** Default configuration table ******************************/

static vpuDecOpenParam test_gvpu_decoder_config_table = {
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
    ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_BIT_BUF_EMPTY)), //enable pic done/error/bs empty interrupt
    0, //no rotation
    0, //no mirror
    FORMAT_420, //frame format
    false, //slice mode
};

static vpuDecParam test_gvpu_pic_config_table = {
    0, //scaleDownRatioWidth
    0, //scaleDownRatioHeight
};

/**************************** Default configuration table end ******************************/
static void* vpu_handle = NULL;

uint8_t* vpu_output[4];
uint8_t yuv_golden_value[3] = {0x7b, 0x66, 0x41};

static void mjpeg_test_callback_on_proc_fin(struct vpu_codec_instance_t* codec, void* args)
{
    vpuDecOutputInfo* outputInfo = (vpuDecOutputInfo*)args;
    LTRACEF("proc fin:mjpeg decoding finish.\n");
    hal_vpu_dec_get_output_info(vpu_handle, codec, outputInfo, ((struct vpu_instance*)codec)->int_res);
}

static void mjpeg_test_callback_on_err(struct vpu_codec_instance_t* codec, void* args)
{
    vpuDecOutputInfo outputInfo;
    LTRACEF("proc err cb: err happen on mjpeg proc\n");
    hal_vpu_dec_get_output_info(vpu_handle, codec, &outputInfo, ((struct vpu_instance*)codec)->int_res);
    LTRACEF("%02d %04d  %8d     %8x %8x %10d  %8x  %8x %10d\n",
            0, outputInfo.indexFrameDisplay, outputInfo.indexFrameDisplay, outputInfo.bytePosFrameStart, outputInfo.ecsPtr, outputInfo.consumedByte, outputInfo.rdPtr, outputInfo.wrPtr, outputInfo.frameCycle);

}

int mjpeg_decoder_offscreen_task(void)
{
    int i = 0;
    int j = 0;
    int ret;
    int fail_num = 0;
    int stride_num = 0;
    int rollback_num = 0;
    int decret;
    uint32_t apiv, hwv, pid;
    vpu_codec_handle_t dec_handle;
    vpuDecInitialInfo header;
    vpuDecOutputInfo proc_result;
    vpuFrameBuffer* frame_buffer;
    uint32_t frame_width, frame_height;
    int img_size = 0;
    uint8_t* image_buff = NULL;

    img_size = ROUNDUP(res_size(MJPEG_TEST_BIN_PATH), 1024);
    image_buff = (uint8_t*)memalign(1024, img_size);

    if (image_buff == NULL) {
        LTRACEF("mjpeg test create animation bitstream fail.\n");
        return MJPEG_TEST_RESULT_VALUE_NO_MEM;
    }

    ret = res_load(MJPEG_TEST_BIN_PATH, image_buff, img_size, 0);

    if (ret < 0) {
        LTRACEF("can't load mjpeg res\n");
        free(image_buff);
        image_buff = NULL;
        return MJPEG_TEST_RESULT_VALUE_RES_LOAD_FAIL;
    }

#if LOCAL_TRACE

    LTRACEF("ANIMATION_ADDR in ddr 0x%x dump=\n", (uint32_t)image_buff);
    hexdump(image_buff, 20);

#endif

    if (hal_vpu_create_handle((void**)&vpu_handle, RES_MJPEG_MJPEG) != true) {
        LTRACEF("create vpu handle fail.\n");
        free(image_buff);
        return MJPEG_TEST_RESULT_VALUE_CREAT_HANDLE_FAIL;
    }

    if (hal_vpu_init(vpu_handle) != 0) {
        printf("vpu handle init fail.\n");
        free(image_buff);
        hal_vpu_deinit(vpu_handle);
        hal_vpu_release_handle(vpu_handle);
        return MJPEG_TEST_RESULT_VALUE_VPU_INIT_FAIL;
    }

    hal_vpu_get_version(vpu_handle, &apiv, &hwv, &pid);
    LTRACEF("VPU ver Info: API ver:%x IP ver:%x Product ID:%x\n", apiv, hwv, pid);

    test_gvpu_decoder_config_table.bitstreamBuffer = p2ap((paddr_t)image_buff);
    test_gvpu_decoder_config_table.bitstreamBufferSize = TEST_ANIMATION_DATA_RUN_ON_RAM_SIZE;
    test_gvpu_decoder_config_table.pBitStream = (BYTE*)image_buff;

    LTRACEF("hal_vpu_dec_open before\n");
    if(hal_vpu_dec_open(vpu_handle, &dec_handle, &test_gvpu_decoder_config_table) != 0){
        printf("dec open fail\n");
        free(image_buff);
        hal_vpu_deinit(vpu_handle);
        hal_vpu_release_handle(vpu_handle);
        return MJPEG_TEST_RESULT_VALUE_VPU_INIT_FAIL;
    }

    hal_vpu_register_callback_on_fin(vpu_handle, dec_handle, mjpeg_test_callback_on_proc_fin, &proc_result);
    hal_vpu_register_callback_on_err(vpu_handle, dec_handle, mjpeg_test_callback_on_err, NULL);

    decret = hal_vpu_dec_update_bitstream_buffer(vpu_handle, dec_handle, TEST_ANIMATION_DATA_RUN_ON_RAM_SIZE - 1);
    LTRACEF("update bs buffer ret:%d\n", decret);
    //bitstream manipulation
    decret = hal_vpu_dec_get_init_info(vpu_handle, dec_handle, &header);
    LTRACEF("get init info:%d\n", decret);

    if(header.picWidth != 1280){
        printf("get header info fail\n");
        free(image_buff);
        hal_vpu_deinit(vpu_handle);
        hal_vpu_release_handle(vpu_handle);
        return MJPEG_TEST_RESULT_VALUE_VPU_DEC_FAIL;
    }

    if (header.sourceFormat == FORMAT_420 || header.sourceFormat == FORMAT_422) {
        frame_width = VPU_CEIL(16, header.picWidth);
        frame_height = VPU_CEIL(16, header.picHeight);
    }
    else {
        frame_width = VPU_CEIL(8, header.picWidth);
        frame_height = VPU_CEIL(8, header.picHeight);
    }

    LTRACEF("init info:\n");
    LTRACEF("pic width:%d\n", header.picWidth);
    LTRACEF("pic height:%d\n", header.picHeight);
    LTRACEF("minimal frame:%d\n", header.minFrameBufferCount);
    LTRACEF("bitdph:%d\n", header.bitDepth);
    LTRACEF("src format:%d\n", header.sourceFormat);

    //frame buffer allocation
    decret = hal_vpu_create_frame_buffer(test_gvpu_decoder_config_table.outputFormat,
                                         test_gvpu_decoder_config_table.chromaInterleave,
                                         test_gvpu_decoder_config_table.packedFormat,
                                         test_gvpu_decoder_config_table.frameEndian,
                                         test_gvpu_decoder_config_table.rotation,
                                         FALSE,
                                         frame_width,
                                         frame_height,
                                         header.bitDepth,
                                         header.minFrameBufferCount + 1,
                                         &frame_buffer);

    LTRACEF("allocate frame buf:%d\n", decret);
    uint32_t frame_size = decret;
    //register frame buffer to vpu for its dma usage
    decret = hal_vpu_dec_register_framebuffer(vpu_handle, dec_handle, frame_buffer, header.minFrameBufferCount + 1, frame_buffer[0].stride);
    LTRACEF("reg frame buf:%d\n", decret);

    do {
        if (rollback_num == 10) {
            printf("vpu dec error\n");
            break;
        }

        LTRACEF("hal_vpu_dec_start_one_frame before\n");
        decret = hal_vpu_dec_start_one_frame(vpu_handle, dec_handle, &test_gvpu_pic_config_table);
        LTRACEF("start ret:%d\n", decret);

        if (decret != 0) {
            hal_vpu_dec_set_bitstream_ptr(vpu_handle, dec_handle, test_gvpu_decoder_config_table.bitstreamBuffer, 0);
            printf("vpu rollback, rollback_num =%d\n", rollback_num);
            rollback_num++;
            continue;
        }

        hal_vpu_wait_interrupt(vpu_handle, dec_handle, 5000);
        LTRACEF("frm idx:%d\n", proc_result.indexFrameDisplay);

        if ((proc_result.indexFrameDisplay < 0) || (proc_result.indexFrameDisplay > header.minFrameBufferCount)) {

            LTRACEF("after start_one_frame frm idx error=:%d\n", proc_result.indexFrameDisplay);
            printf("vpu rollback, rollback_num =%d\n", rollback_num);
            rollback_num++;
            continue;
        }

        LTRACEF("frame Y addr:0x%x,CB addr:0x%x,CR addr:0x%x,frm stride:%d,frm size:0x%x\n", frame_buffer[proc_result.indexFrameDisplay].bufY, frame_buffer[proc_result.indexFrameDisplay].bufCb, frame_buffer[proc_result.indexFrameDisplay].bufCr, frame_buffer[proc_result.indexFrameDisplay].stride, frame_size);
        vpu_output[0] = (uint8_t*)ap2p(frame_buffer[proc_result.indexFrameDisplay].bufY);
        vpu_output[1] = (uint8_t*)ap2p(frame_buffer[proc_result.indexFrameDisplay].bufCb);
        vpu_output[2] = (uint8_t*)ap2p(frame_buffer[proc_result.indexFrameDisplay].bufCr);
        stride_num = frame_buffer[proc_result.indexFrameDisplay].stride;
        LTRACEF("stride_num = %d\n", stride_num);

        for (i = 0; i < 3; i++) {

            if (i > 0) {
                stride_num = stride_num >> 2;
            }

            LTRACEF("fvpu_output%d=%p,stride_num=%d\n", i, vpu_output[i], stride_num);

            for (j = 0; j < stride_num; j++) {
                if (*(vpu_output[i] + j) == yuv_golden_value[i]) {

                }
                else {
                    fail_num++;
                    LTRACEF("vpu dec error num = %d\n", fail_num);
                    break;
                }
            }

        }

        thread_sleep(40);

    }
    while (proc_result.indexFrameDisplay != header.minFrameBufferCount); //-1 means no more output display frame,strm over

    LTRACEF("hal_vpu_dec_close before\n");
    hal_vpu_dec_close(vpu_handle, dec_handle);
    // hal_vpu_free_dma_memory();

    hal_vpu_free_frame_buffer(frame_buffer);

    free(image_buff);

    hal_vpu_deinit(vpu_handle);

    hal_vpu_release_handle(vpu_handle);

    if (rollback_num == 10) {
        return MJPEG_TEST_RESULT_VALUE_VPU_DEC_FAIL;
    }
    else if (fail_num == 0) {
        return MJPEG_TEST_RESULT_VALUE_PASS;
    }
    else {
        return MJPEG_TEST_RESULT_VALUE_PATTERN_FAIL;
    }

}

int slt_module_test_mjpeg_test(uint times, uint timeout, char* result_string)
{
    int ret = 0;

    LTRACEF("mjpeg_decoder_offscreen_task  start\n");
    ret = mjpeg_decoder_offscreen_task();

    if (result_string != NULL) {
        if (ret  != 0) {
            strcpy(result_string, "mjpeg test fail, detail cause see error code");
        }
        else {
            strcpy(result_string, "mjpeg test pass");
        }
    }

    return ret;
}

uint32_t mjpeg_test_start(void)
{
    char out_print[64];
    LTRACEF("mjpeg_test_start start\n");
    //pvt setting
    slt_module_test_mjpeg_test(0, 0, (char*)&out_print);

    return 0;

}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("mjpeg_test", "mjpeg_test\n", (console_cmd)&mjpeg_test_start)
STATIC_COMMAND_END(mjpeg_test);

#endif

SLT_MODULE_TEST_HOOK(mjpeg_test, slt_module_test_mjpeg_test);

APP_START(mjpeg_test)
.flags = 0,
APP_END
