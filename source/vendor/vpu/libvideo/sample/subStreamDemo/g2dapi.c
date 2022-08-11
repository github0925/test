#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "G2D_API"
#else
#define LOG_TAG "G2D_API"
#endif
//#define LOG_NDEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#ifdef __ANDROID__
#include <log/log.h>
#include <cutils/properties.h> // for property_get
#else
#include <fcntl.h>
#endif
#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/mman.h>
#include "g2dapi.h"
#include <pthread.h>
#include <drm/drm_fourcc.h>
#include <sdrv_g2d_cfg.h>
#include "g2d_test_utils.h"
#include <linux/videodev2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define UNUSED(x) (void)x
#define G2D_DEVICE_NAME "/dev/g2d0"
#define HAL_PIXEL_FORMAT_VENDOR_EXT(fmt) (0x100 | (fmt & 0xFF))
#define HAL_PIXEL_FORMAT_NV12        HAL_PIXEL_FORMAT_VENDOR_EXT(6)

typedef enum {
    HAL_PIXEL_FORMAT_RGBA_8888 = 1,
    HAL_PIXEL_FORMAT_RGBX_8888 = 2,
    HAL_PIXEL_FORMAT_BGRA_8888 = 5,
    HAL_PIXEL_FORMAT_YCBCR_422_SP = 16,
    HAL_PIXEL_FORMAT_YCRCB_420_SP = 17,
    HAL_PIXEL_FORMAT_YCBCR_422_I = 20,
    HAL_PIXEL_FORMAT_RGBA_FP16 = 22,
    HAL_PIXEL_FORMAT_RAW16 = 32,
    HAL_PIXEL_FORMAT_BLOB = 33,
    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED = 34,
    HAL_PIXEL_FORMAT_YCBCR_420_888 = 35,
    HAL_PIXEL_FORMAT_RAW_OPAQUE = 36,
    HAL_PIXEL_FORMAT_RAW10 = 37,
    HAL_PIXEL_FORMAT_RAW12 = 38,
    HAL_PIXEL_FORMAT_RGBA_1010102 = 43,
    HAL_PIXEL_FORMAT_Y8 = 538982489,
    HAL_PIXEL_FORMAT_Y16 = 540422489,
    HAL_PIXEL_FORMAT_YV12 = 842094169,
    HAL_PIXEL_FORMAT_RGB_888 = 875710290,
    HAL_PIXEL_FORMAT_RGB_565 = 909199186,
} android_pixel_format_t;

typedef struct _G2D_Context
{
    int  g2dFd;
    int refCnt;
}G2D_Context;

G2D_Context *gG2dContext = NULL;
pthread_mutex_t gG2dMutex = PTHREAD_MUTEX_INITIALIZER;

int g2d_is_open()
{
    struct g2d_capability cap;
    int ret;

    if ((gG2dContext != NULL) && (gG2dContext->g2dFd > 0) )
    {
        slog_info("g2d_is_open gG2dContext has been inited");
        return 0;
    }
    gG2dContext = (G2D_Context*)malloc(sizeof(G2D_Context));
    if (gG2dContext == NULL) {
        slog_err("g2d_is_open malloc failed");
        goto ERROR_OUT1;
    }

    memset((void*)gG2dContext, 0, sizeof(G2D_Context));
    gG2dContext->g2dFd = open(G2D_DEVICE_NAME, O_RDWR);

    if (gG2dContext->g2dFd < 0)
    {
        slog_err("g2d_is_open ion_open failed");
        goto ERROR_OUT1;
    }
     /* 初始化设备 */
    ret = ioctl(gG2dContext->g2dFd, G2D_IOCTL_GET_CAPABILITIES, &cap);
    if (ret) {
        slog_err("g2d_is_open G2D_IOCTL_GET_CAPABILITIES failed");
        goto ERROR_OUT1;
    }
    gG2dContext->refCnt++;
    slog_info("g2d_is_open succesfully refCnt:%d",gG2dContext->refCnt);
    return 0;
ERROR_OUT1:
    if (gG2dContext != NULL
        && gG2dContext->g2dFd > 0)
    {
        close(gG2dContext->g2dFd);
        gG2dContext->g2dFd = 0;
    }

    if (gG2dContext != NULL)
    {
        free(gG2dContext);
        gG2dContext = NULL;
    }
    slog_err("g2d_is_open failed!!!!");
    return -1;
}

int g2d_open()
{
    struct g2d_capability cap;
    int ret;

    pthread_mutex_lock(&gG2dMutex);
    if ((gG2dContext != NULL) && (gG2dContext->g2dFd > 0) )
    {
        slog_info("g2d_open gG2dContext has been inited");
        goto SUCCEED_OUT;
    }
    gG2dContext = (G2D_Context*)malloc(sizeof(G2D_Context));
    if (gG2dContext == NULL) {
        slog_err("g2d_open malloc failed");
        goto ERROR_OUT;
    }

    memset((void*)gG2dContext, 0, sizeof(G2D_Context));
    gG2dContext->g2dFd = open(G2D_DEVICE_NAME, O_RDWR);

    if (gG2dContext->g2dFd < 0)
    {
        slog_err("g2d_open ion_open failed");
        goto ERROR_OUT;
    }
     /* 初始化设备 */
    ret = ioctl(gG2dContext->g2dFd, G2D_IOCTL_GET_CAPABILITIES, &cap);
    if (ret) {
        slog_err("g2d_open G2D_IOCTL_GET_CAPABILITIES failed");
        goto ERROR_OUT;
    }
SUCCEED_OUT:
    gG2dContext->refCnt++;
    slog_info("g2d_open succesfully refCnt:%d",gG2dContext->refCnt);
    pthread_mutex_unlock(&gG2dMutex);
    return 0;
ERROR_OUT:
    if (gG2dContext != NULL
        && gG2dContext->g2dFd > 0)
    {
        close(gG2dContext->g2dFd);
        gG2dContext->g2dFd = 0;
    }

    if (gG2dContext != NULL)
    {
        free(gG2dContext);
        gG2dContext = NULL;
    }
    slog_err("g2d_open failed!!!!");
    pthread_mutex_unlock(&gG2dMutex);
    return -1;
}

int g2d_close()
{
    slog_info("g2d_close enter");
    pthread_mutex_lock(&gG2dMutex);
    if(gG2dContext !=NULL)
    {
        slog_info("g2d_close gG2dContext->refCnt:%d",gG2dContext->refCnt);
        gG2dContext->refCnt--;
        if(gG2dContext->refCnt <= 0)
        {
            if(gG2dContext->g2dFd > 0)
            {
                close(gG2dContext->g2dFd);
                slog_info("g2d_close finish!!!");
            }
            gG2dContext->g2dFd = 0;
            free(gG2dContext);
            gG2dContext = NULL;
        }
    }
    pthread_mutex_unlock(&gG2dMutex);
    return 0;
}

int g2d_get_pitch(int format,int width)
{
    int pitch = width;
    switch (format)
    {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
        case DRM_FORMAT_NV16:
        case DRM_FORMAT_NV61:
        case DRM_FORMAT_YUV420:
        case DRM_FORMAT_YVU420:
            pitch = width;
            break;

        case DRM_FORMAT_ARGB4444:
        case DRM_FORMAT_XRGB4444:
        case DRM_FORMAT_ABGR4444:
        case DRM_FORMAT_XBGR4444:
        case DRM_FORMAT_RGBA4444:
        case DRM_FORMAT_RGBX4444:
        case DRM_FORMAT_BGRA4444:
        case DRM_FORMAT_BGRX4444:
        case DRM_FORMAT_ARGB1555:
        case DRM_FORMAT_XRGB1555:
        case DRM_FORMAT_ABGR1555:
        case DRM_FORMAT_XBGR1555:
        case DRM_FORMAT_RGBA5551:
        case DRM_FORMAT_RGBX5551:
        case DRM_FORMAT_BGRA5551:
        case DRM_FORMAT_BGRX5551:
        case DRM_FORMAT_RGB565:
        case DRM_FORMAT_BGR565:
        case DRM_FORMAT_UYVY:
        case DRM_FORMAT_VYUY:
        case DRM_FORMAT_YUYV:
        case DRM_FORMAT_YVYU:
            pitch = width * 2;
            break;

        case DRM_FORMAT_BGR888:
        case DRM_FORMAT_RGB888:
            pitch = width * 3;
            break;

        case DRM_FORMAT_ARGB8888:
        case DRM_FORMAT_XRGB8888:
        case DRM_FORMAT_ABGR8888:
        case DRM_FORMAT_XBGR8888:
        case DRM_FORMAT_RGBA8888:
        case DRM_FORMAT_RGBX8888:
        case DRM_FORMAT_BGRA8888:
        case DRM_FORMAT_BGRX8888:
        case DRM_FORMAT_ARGB2101010:
        case DRM_FORMAT_XRGB2101010:
        case DRM_FORMAT_ABGR2101010:
        case DRM_FORMAT_XBGR2101010:
        case DRM_FORMAT_RGBA1010102:
        case DRM_FORMAT_RGBX1010102:
        case DRM_FORMAT_BGRA1010102:
        case DRM_FORMAT_BGRX1010102:
        case DRM_FORMAT_AYUV:
            pitch = width * 4;
            break;
        default:
            slog_war("g2d_get_pitch_format do not support format:%d",format);
            break;
    }
    return pitch;
}

/*
*convert format frome android to drm format
*/
int g2d_convert_format(int format)
{
    int outFormat=DRM_FORMAT_NV12;
    switch(format)
    {
        case HAL_PIXEL_FORMAT_BGRA_8888:
            slog_info("g2d_convert_format outFormat:DRM_FORMAT_BGRA8888");
            outFormat= DRM_FORMAT_BGRA8888;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            slog_info("g2d_convert_format :DRM_FORMAT_RGBA8888");
            outFormat= DRM_FORMAT_RGBA8888;
            break;
        case V4L2_PIX_FMT_NV12:
            slog_info("g2d_convert_format outFormat:DRM_FORMAT_NV21");
            outFormat = DRM_FORMAT_NV21;
            break;
        case HAL_PIXEL_FORMAT_YCBCR_420_888:
            slog_info("g2d_convert_format :DRM_FORMAT_YUV420");
            outFormat = DRM_FORMAT_YUV420;
            break;
        case HAL_PIXEL_FORMAT_NV12:
        case HAL_PIXEL_FORMAT_YCRCB_420_SP:
        case V4L2_PIX_FMT_NV21:
            slog_info("g2d_convert_format outFormat:DRM_FORMAT_NV21");
            outFormat = DRM_FORMAT_NV21;
            break;
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
        case V4L2_PIX_FMT_YUYV:
            slog_info("g2d_convert_format outFormat:DRM_FORMAT_YUYV");
            outFormat = DRM_FORMAT_YUYV;
            break;
        default:
            slog_err("g2d_convert_format do not support format:%d",format);
        break;
    }
    return outFormat;
}

static int g2d_get_format_nplanes(unsigned int format)
{
    switch (format)
    {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
        case DRM_FORMAT_NV16:
        case DRM_FORMAT_NV61:
        case DRM_FORMAT_NV24:
        case DRM_FORMAT_NV42:
            return 2;
        case DRM_FORMAT_YVU420:
        case DRM_FORMAT_YUV420:
        case DRM_FORMAT_YUV410:
        case DRM_FORMAT_YVU410:
        case DRM_FORMAT_YUV411:
        case DRM_FORMAT_YVU411:
        case DRM_FORMAT_YUV422:
        case DRM_FORMAT_YVU422:
        case DRM_FORMAT_YUV444:
        case DRM_FORMAT_YVU444:
            return 3;
        default:
            return 1;
    }
}

int _g2d_convert(G2DInput *pInput)
{
    int ret;
    struct g2d_input input;
    unsigned int rot_width;
    unsigned int rot_height;
    //pthread_mutex_lock(&gG2dMutex);

    memset(&input,0x0,sizeof(struct g2d_input));
    input.layer_num = pInput->layer_num;
    for (int i = 0; i < pInput->layer_num; i++)
    {
        struct dpc_layer *layer = &input.layer[i];
        layer->index = pInput->layer[i].layer_index;
        layer->enable = 1;
        layer->format = pInput->layer[i].src_format;//g2d_convert_format(pInput->src_format);
        layer->nplanes = g2d_get_format_nplanes(layer->format);
        layer->alpha = pInput->layer[i].alpha;
        layer->blend_mode = pInput->layer[i].blend_mode;
        //layer->rotation = pInput->rotation;
        layer->zpos = pInput->layer[i].zpos;
        layer->xfbc = 0;
        layer->modifier = 0;
        layer->width = pInput->layer[i].src_w;
        layer->height = pInput->layer[i].src_h;
        layer->src_x = pInput->layer[i].src_crop_x;
        layer->src_y = pInput->layer[i].src_crop_y;
        layer->src_w = pInput->layer[i].src_crop_w;
        layer->src_h = pInput->layer[i].src_crop_h;

        layer->dst_x = pInput->dst_crop_x;
        layer->dst_y = pInput->dst_crop_y;
        layer->dst_w = pInput->dst_crop_w;
        layer->dst_h = pInput->dst_crop_h;
        layer->bufs[0].fd = pInput->layer[i].src_fd;
        switch(layer->nplanes)
        {
            case 3:
                layer->pitch[0] = layer->width;
                layer->offsets[0] = 0;
                layer->pitch[1] = layer->width/2;
                layer->offsets[1] = layer->width*layer->height;
                layer->pitch[2] = layer->width/2;
                layer->offsets[2] = layer->width*layer->height*5/4;
                break;
            case 2:
                layer->pitch[0] = layer->width;
                layer->offsets[0] = 0;
                layer->pitch[1] = layer->width;
                layer->offsets[1] = layer->width*layer->height;
                break;
            case 1:
                layer->pitch[0] = g2d_get_pitch(layer->format, layer->width);
                break;
            default:
                slog_err("layer->nplanes:%d is error value",layer->nplanes);
                break;
            }
        }

    input.output.width = pInput->dst_w;
    input.output.height = pInput->dst_h;
    input.output.fmt = pInput->dst_format;//g2d_convert_format(pInput->dst_format);
    //input.output.addr[0] = pInput->dst_addr;
    input.output.bufs[0].fd = pInput->dst_fd;
    input.output.nplanes = g2d_get_format_nplanes(input.output.fmt);
    input.output.rotation = pInput->rotation;
    switch (input.output.rotation)
    {
        case ROTATION_TYPE_ROT_90:
        case ROTATION_TYPE_HF_90:
        case ROTATION_TYPE_ROT_270:
        case ROTATION_TYPE_VF_90:
            rot_width = input.output.height;
            rot_height = input.output.width;
            break;
        case ROTATION_TYPE_HFLIP:
        case ROTATION_TYPE_VFLIP:
        case ROTATION_TYPE_ROT_180:
        case ROTATION_TYPE_NONE:
            rot_width = input.output.width;
            rot_height = input.output.height;
        default:
            break;
    }
    switch(input.output.nplanes)
    {
        case 3:
            if ((input.output.fmt == DRM_FORMAT_YUV420) || (input.output.fmt == DRM_FORMAT_YVU420))
            {
                input.output.stride[0] = rot_width;
                input.output.offsets[0] = 0;
                input.output.stride[1] = rot_width/2;
                input.output.offsets[1] = rot_width*rot_height;
                input.output.stride[2] = rot_width/2;
                input.output.offsets[2] = rot_width*rot_height*5/4;
            } else if (input.output.fmt == DRM_FORMAT_YUV422)
            {
                input.output.stride[0] = rot_width;
                input.output.offsets[0] = 0;
                input.output.stride[1] = rot_width/2;
                input.output.offsets[1] = rot_width*rot_height;
                input.output.stride[2] = rot_width/2;
                input.output.offsets[2] = rot_width*rot_height*3/2;
            } else if (input.output.fmt == DRM_FORMAT_YUV444)
            {
                input.output.stride[0] = rot_width;
                input.output.offsets[0] = 0;
                input.output.stride[1] = rot_width;
                input.output.offsets[1] = rot_width*rot_height;
                input.output.stride[2] = rot_width;
                input.output.offsets[2] = rot_width*rot_height*2;
            } else
            {
                slog_err("output nplanes 3 ,set stride do not support format : %x", input.output.fmt);
                return -1;
            }
            break;
        case 2:
            input.output.stride[0] = rot_width;
            input.output.offsets[0] = 0;
            input.output.stride[1] = rot_width;
            input.output.offsets[1] = rot_width * rot_height;
            break;
        case 1:
            input.output.stride[0] = g2d_get_pitch(input.output.fmt, rot_width);
            break;
        default:
            slog_err("input.output:%d is error value",input.output.nplanes);
            break;
    }

    for (int i = 0; i < input.output.nplanes; i++)
    {
        slog_info("stride[%d] = %d, offsets[%d] = %d", i, input.output.stride[i], i, input.output.offsets[i]);
    }

#if  DEBUG_INPUT_ARGS
    FILE *saveinput;
    saveinput = fopen("/data/input_args","w+b");
    if(saveinput)
    {
        fwrite(&input,1,sizeof(struct g2d_input),saveinput);
        slog_verbose("save input args to /data/input_args");
        fclose(saveinput);
    }
#endif

    ret = g2d_is_open();
    if(ret < 0)
    {
        return ret;
    }

    int count = 0;
    do {
        ret = ioctl(gG2dContext->g2dFd, G2D_IOCTL_POST_CONFIG, &input);
        if (ret)
        {
            slog_err("****************************ioctl G2D_IOCTL_POST_CONFIG: ERROR***************");
            slog_err("cuishang >>>> do post config count = %d", count);
            //pthread_mutex_unlock(&gG2dMutex);
            return ret;
        }
        count++;
    } while(false);

    slog_info("****************************ioctl G2D_IOCTL_POST_CONFIG: SUCCESED***************");
    //pthread_mutex_unlock(&gG2dMutex);
    return 0;
}

int g2d_convert(G2DInput *pInput)
{
    pthread_mutex_lock(&gG2dMutex);
    //slog_info("g2d_convert ENTER,%d-%d,x:%d,y:%d",pInput->src_fd,pInput->dst_fd,pInput->src_crop_x,pInput->src_crop_y);
    int ret = 0;
    ret = _g2d_convert(pInput);
    //slog_info("g2d_convert exit,%d-%d",pInput->src_fd,pInput->dst_fd);
    pthread_mutex_unlock(&gG2dMutex);
    return ret;
}

int g2d_format_convert(int      src_format,int dst_format,int src_fd,int dst_fd,int width,int height)
{
    pthread_mutex_lock(&gG2dMutex);
    //slog_info("g2d_format_convert ENTER:%d,%d",src_fd,dst_fd);
    int ret = 0;
    G2DInput g2dinput;

    g2dinput.layer_num = 1;
    g2dinput.layer[0].layer_index = 2;
    g2dinput.layer[0].src_fd = src_fd;
    g2dinput.layer[0].src_w = width;
    g2dinput.layer[0].src_h = height;
    g2dinput.layer[0].src_crop_x = 0;
    g2dinput.layer[0].src_crop_y = 0;
    g2dinput.layer[0].src_crop_w = width;
    g2dinput.layer[0].src_crop_h = height;
    g2dinput.dst_fd = dst_fd;
    g2dinput.dst_w = width;
    g2dinput.dst_h = height;
    g2dinput.dst_crop_x = 0;
    g2dinput.dst_crop_y =0;
    g2dinput.dst_crop_w = width;
    g2dinput.dst_crop_h = height;
    g2dinput.rotation = 0;
    g2dinput.layer[0].src_format = src_format;
    g2dinput.dst_format = dst_format;

    ret =  _g2d_convert(&g2dinput);
    //slog_info("g2d_format_convert EXIT %d,%d",src_fd,dst_fd);
    pthread_mutex_unlock(&gG2dMutex);
    return ret;
}

int g2d_format_scale(int src_format,int dst_format,int src_fd,int dst_fd,int src_w,int src_h,int dst_w,int dst_h)
{
    pthread_mutex_lock(&gG2dMutex);
    int ret=0;
    G2DInput g2dinput;

    //slog_info("srcfd:%d,dstfd:%d,src_w:%d,src_h:%d,dst_w:%d,dst_h:%d",src_fd,dst_fd,src_w,src_h,dst_w,dst_h);
    g2dinput.layer_num = 1;
    g2dinput.layer[0].layer_index = 2;
    g2dinput.layer[0].src_fd = src_fd;
    g2dinput.layer[0].src_w = src_w;
    g2dinput.layer[0].src_h = src_h;
    g2dinput.layer[0].src_crop_x = 0;
    g2dinput.layer[0].src_crop_y = 0;
    g2dinput.layer[0].src_crop_w = src_w;
    g2dinput.layer[0].src_crop_h = src_h;
    g2dinput.dst_fd = dst_fd;
    g2dinput.dst_w = dst_w;
    g2dinput.dst_h = dst_h;
    g2dinput.dst_crop_x = 0;
    g2dinput.dst_crop_y =0;
    g2dinput.dst_crop_w = dst_w;
    g2dinput.dst_crop_h = dst_h;
    g2dinput.rotation = 0;
    g2dinput.layer[0].src_format = src_format;
    g2dinput.dst_format = dst_format;

    ret = _g2d_convert(&g2dinput);
    pthread_mutex_unlock(&gG2dMutex);
    return ret;
}

int g2d_rotation(int src_format, int dst_format, int src_fd, int dst_fd,
        int width, int height, rotation_type rotation)
{
    pthread_mutex_lock(&gG2dMutex);
    int ret=0;
    G2DInput g2dinput;
    memset((void *)&g2dinput, 0, sizeof(G2DInput));

    g2dinput.layer_num = 1;
    g2dinput.layer[0].layer_index = 0;
    g2dinput.layer[0].src_fd = src_fd;
    g2dinput.layer[0].src_w = width;
    g2dinput.layer[0].src_h = height;
    g2dinput.layer[0].src_crop_x = 0;
    g2dinput.layer[0].src_crop_y = 0;
    g2dinput.layer[0].src_crop_w = width;
    g2dinput.layer[0].src_crop_h = height;
    g2dinput.dst_fd = dst_fd;
    g2dinput.dst_w = width;
    g2dinput.dst_h = height;
    g2dinput.dst_crop_x = 0;
    g2dinput.dst_crop_y =0;
    g2dinput.dst_crop_w = width;
    g2dinput.dst_crop_h = height;
    g2dinput.rotation = rotation;
    g2dinput.layer[0].src_format = src_format;
    g2dinput.dst_format = dst_format;

    ret = _g2d_convert(&g2dinput);
    pthread_mutex_unlock(&gG2dMutex);
    return ret;
}

int g2d_Splice(int src_format, int dst_format, int src_fd, int dst_fd,
        int width, int height, rotation_type rotation)
{
    return 0;
}

int g2d_blend(int src_format, int dst_format, int src_fd, int dst_fd,
        int width, int height, rotation_type rotation)
{
    return 0;
}

int g2d_fastcopy(int src_fd, int dst_fd, unsigned int width, unsigned int height,
        unsigned int src_stride, unsigned int dst_stride, unsigned int fmt)
{
    pthread_mutex_lock(&gG2dMutex);
    int ret=0;
    struct g2d_input input;
    memset(&input,0x0,sizeof(struct g2d_input));

    input.layer_num = 1;
    input.layer[0].nplanes = g2d_get_format_nplanes(fmt);
    input.output.nplanes = g2d_get_format_nplanes(fmt);
    input.layer[0].bufs[0].fd = src_fd;
    input.output.bufs[0].fd = dst_fd;
    input.output.width = width;
    input.output.height = height;
    input.layer[0].pitch[0] = src_stride;
    input.output.stride[0] = dst_stride;

    slog_info("src_stride %d , dst_stride %d", src_stride, dst_stride);

    ret = ioctl(gG2dContext->g2dFd, G2D_IOCTL_FAST_COPY, &input);
    if (ret)
    {
        slog_info("err :G2D_IOCTL_POST_CONFIG  failed \n");
    }
    pthread_mutex_unlock(&gG2dMutex);
    return ret;
}

int g2d_fill_rect(struct g2d_bg_cfg *bgcfg, struct g2d_output_cfg *outcfg)
{
    pthread_mutex_lock(&gG2dMutex);
    int ret=0;
    struct g2d_input input;
    memset(&input,0x0,sizeof(struct g2d_input));

    /*alph config*/
    input.bg_layer.en = bgcfg->en;
    input.bg_layer.color = bgcfg->color;
    input.bg_layer.bpa = bgcfg->bpa;
    input.bg_layer.aaddr = bgcfg->aaddr;
    input.bg_layer.abufs.fd = bgcfg->abufs.fd;
    input.bg_layer.astride = bgcfg->astride;
    input.bg_layer.g_alpha = bgcfg->g_alpha;
    input.bg_layer.x = bgcfg->x;
    input.bg_layer.y = bgcfg->y;
    input.bg_layer.width = bgcfg->width;
    input.bg_layer.height = bgcfg->height;
    input.bg_layer.zorder = bgcfg->zorder;
    input.bg_layer.pd_type = bgcfg->pd_type;

    /*output config*/
    input.output.nplanes = g2d_get_format_nplanes(outcfg->fmt);
    input.output.width = outcfg->width;
    input.output.height = outcfg->height;
    input.output.fmt = outcfg->fmt;
    debug_fmt(input.output.fmt);
    input.output.bufs[0].fd = outcfg->bufs[0].fd;
    input.output.stride[0] = g2d_get_pitch(outcfg->fmt, outcfg->width);;

    slog_info("start ioctl ");
    ret = ioctl(gG2dContext->g2dFd, G2D_IOCTL_FILL_RECT, &input);
    if (ret)
    {
        slog_info("err :G2D_IOCTL_FILL_RECT  failed \n");
    }

    pthread_mutex_unlock(&gG2dMutex);

    return ret;
}

g2d_dev_ops G2dDevOps =
{
    .g2dOpen = g2d_open,
    .g2dClose = g2d_close,
    .g2dRotation = g2d_rotation,
    .g2dFormatScale = g2d_format_scale,
    .g2dFormatConvert = g2d_format_convert,
    .g2dFastcopy = g2d_fastcopy,
    .g2dFillrect = g2d_fill_rect,
    .g2dConvert = g2d_convert,
};

g2d_dev_ops* GetG2dDevOps()
{
    return &G2dDevOps;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */


