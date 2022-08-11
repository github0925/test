#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdm_display.h>
#include <disp_data_type.h>
#include <lib/reg.h>
#if defined (__GNUC__)
  #include <malloc.h>
#elif defined(__ICCARM__)
  #include "heap.h"
#else
  #error Unknown Compiler!
#endif
#include <string.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <kernel/spinlock.h>

//#include <getopt.h>

#include <app.h>
#include <res.h>
#include <trace.h>

#include <chip_res.h>
#include <spi_nor_hal.h>
#include <ext_data.h>
#include <heap.h>
#include <g2dlite_api.h>
//#include "scenery.h"
//#include "ancientry.h"

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
#include "buffer.h"

#define DISPLAY_NUM    5
//static char dwmem[90*50*2+2] EXT_SECTION(dwmem);

void *drmem = NULL;
void *dwmem = NULL;
void *domem = NULL;

struct idx_to_fmt {
    int idx;
    unsigned int fmt;
} itof[] = {
    {0, COLOR_RGB565},
    {1, COLOR_BGR565},
    {2, COLOR_ARGB4444},
    {3, COLOR_BGRA4444},
    {4, COLOR_ARGB1555},
    {5, COLOR_RGB888},
    {6, COLOR_BGR888},
    {7, COLOR_ARGB8888},
    {8, COLOR_BGRA8888},
    {9, COLOR_ARGB2101010},
    {10, COLOR_BGRA2101010},
    {11, COLOR_NV21},
    {12, COLOR_NV12},
    {13, COLOR_YUV420P},
    {14, COLOR_YUYV},
    {-1, 0},
};

enum {
    SCREEN_CLEAR = 0,
    SHOW_PATTERN,
    SHOW_MEM,
};

static int get_fmt_by_idx(int idx)
{
    return itof[idx].fmt;
}

void dshow_pattern(struct bo *bo, int display_id)
{
    int ret = 0;
    ret = bo_init(bo);
    if (ret < 0) {
        LOGE("bo init failed\n");
        return;
    }

    dshow_post(display_id, bo);
}

static int dshow(int argc, const cmd_args *argv)
{
    int8_t i = 1;
    struct bo bo;
    display_handle *handle;

    int display_id = 0;
    uint8_t nplane = 0;
    int func = SCREEN_CLEAR;

    if (argc == 1) {
        LOGE("clear screen\n");
        LOGE("dshow [display_id]\n");
        LOGE("dshow [format] [src_width] [src_heigt] [dst_width] [dst_heigt] [base_addr](0-3) (displayid)\n");
        LOGE("format:\n");
        LOGE("{0, COLOR_RGB565}\n");
        LOGE("{1, COLOR_BGR565}\n");
        LOGE("{2, COLOR_ARGB4444}\n");
        LOGE("{3, COLOR_BGRA4444}\n");
        LOGE("{4, COLOR_ARGB1555}\n");
        LOGE("{5, COLOR_RGB888}\n");
        LOGE("{6, COLOR_BGR888}\n");
        LOGE("{7, COLOR_ARGB8888}\n");
        LOGE("{8, COLOR_BGRA8888}\n");
        LOGE("{9, COLOR_ARGB2101010}\n");
        LOGE("{10, COLOR_BGRA2101010}\n");
        LOGE("{11, COLOR_NV21}\n");
        LOGE("{12, COLOR_NV12}\n");
        LOGE("{13, COLOR_YUV420P}\n");
        LOGE("{14, COLOR_YUYV}\n");
        return 0;
    }

    memset(&bo, 0, sizeof(struct bo));

    if(argc == 2) {
        display_id = atoi(argv[i].str);
        if ((display_id > DISPLAY_NUM-1) || (display_id < 0))
            display_id = 0;
        dprintf(0, "post display id:%d str:%s\n", display_id, argv[i].str);
        func = SCREEN_CLEAR;
        goto exe_func;
    }

    bo.fmt = itof[atoi(argv[i++].str)].fmt;

    bo.src_width = atoi(argv[i++].str);
    bo.src_height = atoi(argv[i++].str);
    bo.dst_width = atoi(argv[i++].str);
    bo.dst_height = atoi(argv[i++].str);

    LOGE("src_w:%d, src_h:%d, d_w:%d, d_h:%d\n", bo.src_width, bo.src_height, bo.dst_width, bo.dst_height);

    nplane = COLOR_GET_PLANE_COUNT(bo.fmt);

    for (int j = 0; j < nplane; j++) {
        if (argv[i].str != NULL) {
            bo.addr[j] = (void *)atoi(argv[i++].str);
            dprintf(0, "plane[%d]:%p\n", j, bo.addr[j]);
        }

        if ((nplane > 1) && (bo.addr[j] == NULL)) {
            LOGE("addr input invalid %d,%p\n", j, bo.addr[j]);
            return -1;
        }
    }

    if (argv[i].str != NULL) {
        display_id = atoi(argv[i].str);
        if ((display_id > DISPLAY_NUM-1) || (display_id < 0))
            display_id = 0;
        dprintf(0, "post display id:%d\n", display_id);
    }

    if (bo.addr[0] == NULL) {
        dprintf(0, "base_addr is null, will show pattern\n");
        func = SHOW_PATTERN;
    } else func = SHOW_MEM;

exe_func:
    switch (func) {
        case SHOW_PATTERN:
            dshow_pattern(&bo, display_id);
            bo_free(&bo);
            break;
        case SHOW_MEM:
            dshow_post(display_id, &bo);
            break;
        case SCREEN_CLEAR:
            handle = hal_get_display_handle(display_id);
            sdm_clear_display(handle);
            break;
        default:
            break;
    }

    return 0;
}

static int blend_gpipe_bypass(void)
{
    bool ret;
    void *handle = NULL;
    struct g2dlite_input input;

    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    if (drmem == NULL) {
        drmem = memalign(256, 90*50*2 + 100);
    }

    if (dwmem == NULL) {
        dwmem = memalign(256, 90*50*4);
        memset(dwmem, 0 , 90*50*4);
    }
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_RGB565;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = 90;
        input.layer[i].src.h = 50;
        input.layer[i].addr[0] = (unsigned long)drmem;
        input.layer[i].src_stride[0] = 90 * 2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = 90;
        input.layer[i].dst.h = 50;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = 90;
    input.output.height = 50;
    input.output.fmt = COLOR_RGB565;
    input.output.addr[0] = (unsigned long)dwmem;
    input.output.stride[0] = 90* 2;
    input.output.rotation = 0;
    hal_g2dlite_blend(handle, &input);

    return 0;
}

static int yuv_to_rgb(void)
{
    bool ret;
    void *handle = NULL;
    struct g2dlite_input input;

    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    if (drmem == NULL) {
        drmem = memalign(256, 1280*50*4);
    }

    if (dwmem == NULL) {
        dwmem = memalign(256, 1280*50*4);
        memset(dwmem, 0 , 1280*50*4);
    }
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_YUYV;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = 1280;
        input.layer[i].src.h = 50;
        input.layer[i].addr[0] = (unsigned long)drmem;
        input.layer[i].src_stride[0] = 1280 * 2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = 1280;
        input.layer[i].dst.h = 50;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = 1280;
    input.output.height = 50;
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)dwmem;
    input.output.stride[0] = 1280 * 4;
    input.output.rotation = 0;
    hal_g2dlite_blend(handle, &input);

    return 0;

}

static int clamp(int value, int min, int max) {
    if (value < min) {
        value = min;
    }
    if (value > max) {
        value = max;
    }
    return value;
}

static int yuv_to_rgb_cpu(void)
{
    int color_r0, color_g0, color_b0;
    int color_r1, color_g1, color_b1;
    char *cdrmem, *cdomem;

    if (drmem == NULL) {
        drmem = memalign(256, 1280*50*4);
    }
    if (domem == NULL) {
        domem = memalign(256, 1280*50*4);
    }

    cdrmem = (char*)drmem;
    cdomem = (char*)domem;

    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 1280; x++) {
            int color_y0 = cdrmem[y * 1280*2 + 4 * x + 0];
            int color_u  = cdrmem[y * 1280*2 + 4 * x + 1];
            int color_y1 = cdrmem[y * 1280*2 + 4 * x + 2];
            int color_v  = cdrmem[y * 1280*2 + 4 * x + 3];

            color_r0 = (int)(color_y0 + 1.5748f * (color_v - 128.0f) + 0.5f);
            color_g0 = (int)(color_y0 - 0.1873f * (color_u - 128.0f) - 0.4681f * (color_v - 128.0f) + 0.5f);
            color_b0 = (int)(color_y0 + 1.8556f * (color_u - 128.0f) + 0.5f);

            color_r0 = clamp(color_r0, 0, 255); // make sure 0 <= color <= 255
            color_g0 = clamp(color_g0, 0, 255);
            color_b0 = clamp(color_b0, 0, 255);

            color_r1 = (int)(color_y1 + 1.5748f * (color_v - 128.0f) + 0.5f);
            color_g1 = (int)(color_y1 - 0.1873f * (color_u - 128.0f) - 0.4681f * (color_v - 128.0f) + 0.5f);
            color_b1 = (int)(color_y1 + 1.8556f * (color_u - 128.0f) + 0.5f);

            color_r1 = clamp(color_r1, 0, 255);
            color_g1 = clamp(color_g1, 0, 255);
            color_b1 = clamp(color_b1, 0, 255);
            cdomem[y * 1280*4 + 8 * x + 0] = color_b0;
            cdomem[y * 1280*4 + 8 * x + 1] = color_g0;
            cdomem[y * 1280*4 + 8 * x + 2] = color_r0;
            cdomem[y * 1280*4 + 8 * x + 3] = 0;

            cdomem[y * 1280*4 + 8 * x + 4] = color_b1;
            cdomem[y * 1280*4 + 8 * x + 5] = color_g1;
            cdomem[y * 1280*4 + 8 * x + 6] = color_r1;
            cdomem[y * 1280*4 + 8 * x + 7] = 0;
        }
    }

    arch_sync_cache_range((addr_t)cdomem, 1280*50*4);

    LOGD("drmem = 0x%lx, dwmem = 0x%lx, domem = 0x%lx\n", p2ap((paddr_t)drmem), p2ap((paddr_t)dwmem), p2ap((paddr_t)domem));
    return 0;
}

static int blend_2_pipe(void)
{
    bool ret;
    void *handle = NULL;
    struct g2dlite_input input;

    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    if (drmem == NULL) {
        drmem = memalign(256, 90*50*2 + 100);
    }

    if (domem == NULL) {
        domem = memalign(256, 90*50*2 + 100);
        /*ram not enough, need we using trace32 to fill data.*/
    }

    if (dwmem == NULL) {
        dwmem = memalign(256, 90*50*4);
        memset(dwmem, 0 , 90*50*4);
    }
    LOGD("drmem = 0x%lx, dwmem = 0x%lx, domem = 0x%lx\n", p2ap((paddr_t)drmem), p2ap((paddr_t)dwmem), p2ap((paddr_t)domem));

    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_RGB565;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = 90;
        input.layer[i].src.h = 50;

        if (i == 0)
            input.layer[i].addr[0] = (unsigned long)drmem;
        else
            input.layer[i].addr[0] = (unsigned long)domem;

        input.layer[i].src_stride[0] = 90 * 2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = 90;
        input.layer[i].dst.h = 50;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0x80;
    }

    input.output.width = 90;
    input.output.height = 50;
    input.output.fmt = COLOR_RGB565;
    input.output.addr[0] = (unsigned long)dwmem;
    input.output.stride[0] = 90* 2;
    input.output.rotation = 0;
    hal_g2dlite_blend(handle, &input);

    return 0;
}

static int scaler_gpipe(void)
{
    bool ret;
    void *handle = NULL;
    struct g2dlite_input input;

    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    if (drmem == NULL) {
        drmem = memalign(256, 90*50*2 + 100);
    }

    if (dwmem == NULL) {
        dwmem = memalign(256, 180*100*4);
        memset(dwmem, 0 , 90*50*4);
    }
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_RGB565;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = 90;
        input.layer[i].src.h = 50;
        input.layer[i].addr[0] = (unsigned long)drmem;
        input.layer[i].src_stride[0] = 90 * 2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = 180;
        input.layer[i].dst.h = 100;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = 180;
    input.output.height = 100;
    input.output.fmt = COLOR_RGB565;
    input.output.addr[0] = (unsigned long)dwmem;
    input.output.stride[0] = 180* 2;
    input.output.rotation = 0;
    hal_g2dlite_blend(handle, &input);

    return 0;
}

static int width_in = 256;
static int rotation_test(void)
{
    #if 0
    ROTATION_TYPE_NONE    = 0b000,
    ROTATION_TYPE_ROT_90  = 0b001,
    ROTATION_TYPE_HFLIP   = 0b010,
    ROTATION_TYPE_VFLIP   = 0b100,
    ROTATION_TYPE_ROT_180 = ROTATION_TYPE_VFLIP | ROTATION_TYPE_HFLIP,
    ROTATION_TYPE_ROT_270 = ROTATION_TYPE_ROT_90 | ROTATION_TYPE_VFLIP | ROTATION_TYPE_HFLIP,
    ROTATION_TYPE_VF_90   = ROTATION_TYPE_VFLIP | ROTATION_TYPE_ROT_90,
    ROTATION_TYPE_HF_90   = ROTATION_TYPE_HFLIP | ROTATION_TYPE_ROT_90,
    #endif
    bool ret;
    void *handle = NULL;
    struct g2dlite_input input;

    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    if (drmem == NULL) {
        drmem = memalign(256, 1280*50*2 + 100);
    }

    if (dwmem == NULL) {
        dwmem = memalign(256, 1280*50*4);
        memset(dwmem, 0 , 1280*50*4);
    }
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_RGB565;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = width_in;
        input.layer[i].src.h = 50;
        input.layer[i].addr[0] = (unsigned long)drmem;
        input.layer[i].src_stride[0] = 1280 * 2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = width_in;
        input.layer[i].dst.h = 50;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

#if 0
    input.output.rotation = ROTATION_TYPE_NONE;
    input.output.width = width_in;
    input.output.stride[0] = width_in * 2;
    input.output.height = 50;
#endif
#if 0
    input.output.rotation = ROTATION_TYPE_VFLIP;
    input.output.width = width_in;
    input.output.stride[0] = width_in * 2;
    input.output.height = 50;
#endif
#if 0
    input.output.rotation = ROTATION_TYPE_HFLIP;
    input.output.width = width_in;
    input.output.stride[0] = width_in * 2;
    input.output.height = 50;
#endif
#if 0
    input.output.rotation = ROTATION_TYPE_ROT_180;
    input.output.width = width_in;
    input.output.stride[0] = width_in * 2;
    input.output.height = 50;
#endif
#if 0
    input.output.rotation = ROTATION_TYPE_ROT_90;
    input.output.width = width_in;
    input.output.stride[0] = 50 * 2;
    input.output.height = 50;
#endif
#if 0
    input.output.rotation = ROTATION_TYPE_HF_90;
    input.output.width = width_in;
    input.output.stride[0] = 50 * 2;
    input.output.height = 50;
#endif
#if 0
    input.output.rotation = ROTATION_TYPE_ROT_270;
    input.output.width = width_in;
    input.output.stride[0] = 50 * 2;
    input.output.height = 50;
#endif
#if 1
    input.output.rotation = ROTATION_TYPE_VF_90;
    input.output.width = width_in;
    input.output.stride[0] = 50 * 2;
    input.output.height = 50;
#endif
    input.output.fmt = COLOR_RGB565;
    input.output.addr[0] = (unsigned long)dwmem;
    hal_g2dlite_rotaion(handle, &input);

    return 0;
}

static int fill_rect(void)
{
    bool ret;
    void *handle = NULL;
    struct g2dlite_output_cfg output;

    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    if (dwmem == NULL) {
        dwmem = memalign(256, 1280*50*4);
        //memset(dwmem, 0 , 1280*50*4);

    }

    memset(&output, 0, sizeof(struct g2dlite_output_cfg));

    static int count = 0;

    output.width = 80;
    output.height = 50;
    output.fmt = COLOR_RGB565;
    output.addr[0] = (unsigned long)dwmem;
    output.stride[0] = 160;
    output.rotation = 0;
    switch (count) {
        case 0:
            hal_g2dlite_fill_rect(handle, 0xFFFFFFFF, 0xFF, 0, 0, 0, &output);
            count++;
            break;
        case 1:
            hal_g2dlite_fill_rect(handle, 0x3ff, 0xFF, 0, 0, 0, &output);
            count++;
            break;
        case 2:
            hal_g2dlite_fill_rect(handle, 0xFFC00, 0xFF, 0, 0, 0, &output);
            count++;
        case 3:
            hal_g2dlite_fill_rect(handle, 0x0, 0xFF, 0, 0, 0, &output);
            count = 0;
            break;
    }


    return 0;
}

static int fastcopy(void)
{
    bool ret;
    void *handle = NULL;
    struct fcopy_t in, out;

    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    if (dwmem == NULL) {
        dwmem = memalign(256, 1280*50*4); //this memory we can use fill_rect to fill data.
    }

    if (drmem == NULL) {
        drmem = memalign(256, 1280*50*4); //this memory we use fastcopy to fill data.
    }

    memset(&in, 0, sizeof(struct fcopy_t));
    memset(&out, 0, sizeof(struct fcopy_t));

    in.addr = (unsigned long)dwmem;
    in.width = 40;
    in.height = 50;
    in.stride = 160;

    out.addr = (unsigned long)drmem;
    out.width = 40;
    out.height = 50;
    out.stride = 160;

    hal_g2dlite_fastcopy(handle, &in, &out);

    return 0;
}

static int porter_duff(void)
{
    bool ret;
    void *handle = NULL;
    struct g2dlite_input input;
    static int count = 0;

    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    input.pd_info.en = 1;
    input.pd_info.zorder = 0;
    input.pd_info.mode = CLEAR + count++;
    input.pd_info.alpha_need = 1;
    LOGD("porter-duff mode = %d\n", input.pd_info.mode);

    if (count == DES_SUB)
        count = CLEAR;

    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_ARGB8888;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = 200;
        input.layer[i].src.h = 200;

        if (i == 0)
            input.layer[i].addr[0] = (unsigned long)0x76800000;
        else
            input.layer[i].addr[0] = (unsigned long)0x77800000;

        input.layer[i].src_stride[0] = 200 * 4;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = 200;
        input.layer[i].dst.h = 200;
        input.layer[i].ckey.en = 0;
        if (i == 0)
            input.layer[i].pd_type = PD_SRC;
        else
            input.layer[i].pd_type = PD_DST;
    }

    input.output.width = 200;
    input.output.height = 200;
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)0x78800000;
    input.output.stride[0] = 200* 4;
    input.output.rotation = 0;
    hal_g2dlite_blend(handle, &input);

    return 0;
}

static int yuv_to_yuv(void)
{
    bool ret;
    void *handle = NULL;
    struct g2dlite_input input;

    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_NV21;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = 1280;
        input.layer[i].src.h = 50;
        input.layer[i].addr[0] = (unsigned long)0x76800000;;
        input.layer[i].addr[1] = (unsigned long)0x76900000;
        input.layer[i].src_stride[0] = 1280;
        input.layer[i].src_stride[1] = 1280;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = 1280;
        input.layer[i].dst.h = 50;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = 1280;
    input.output.height = 50;
    input.output.fmt = COLOR_NV21;
    input.output.addr[0] = (unsigned long)0x77800000;
    input.output.addr[1] = (unsigned long)0x77900000;
    input.output.stride[0] = 1280;
    input.output.stride[1] = 1280;
    input.output.rotation = 0;
    hal_g2dlite_blend(handle, &input);

    return 0;

}


/*------------------------------------------------*/
#define NUM_CASES 20
typedef int (*func_t)(void);

struct test_case_ {
    const char *desc;
    func_t func;
} test_cases[NUM_CASES] = {
    {"rgb565-bypass", blend_gpipe_bypass},
    {"yuyv-to-argb8888", yuv_to_rgb},
    {"cpu-yuyv-to-argb8888", yuv_to_rgb_cpu},
    {"blend_2_pipe", blend_2_pipe},
    {"scaler_gpipe", scaler_gpipe},
    {"rotation_test", rotation_test},
    {"fill_rect", fill_rect},
    {"fastcopy", fastcopy},
    {"porter-duff", porter_duff},
    {"yuv-to-yuv", yuv_to_yuv},
};

int usage(void)
{
    int i = 0;
    int n;
    for (i = 0; i < NUM_CASES; i++) {
        if (test_cases[i].desc == NULL)
            break;
    }
    n = i;
    printf("have %d test case:\n", n);
    for (i = 0; i < n; i++) {
        printf("    %d. %s\n", i, test_cases[i].desc);
    }
    return n;
}

static int dlite(int argc, const cmd_args *argv)
{
    int id = -1;
    int case_num;
    func_t case_func = NULL;

    case_num = usage();
    if (argc < 2) {
        char ch;
        LOGD("Choose number of test case: ");
        ch = getchar();
        id = atoi(&ch);
    }

    if (argc >= 2) {
        id = atoi(argv[1].str);
    }

    if (id > case_num && id <= 0) {
        LOGD("You choose invalid number: %d\n", id);
        return -2;
    }

    if (argc == 3) {
        width_in = atoi(argv[2].str);
    }
    case_func = test_cases[id].func;
    case_func();

    return 0;
}
STATIC_COMMAND_START
STATIC_COMMAND("dshow", "dshow", &dshow)
STATIC_COMMAND("dlite", "dlite", &dlite)
STATIC_COMMAND_END(g2dlite);

#endif
