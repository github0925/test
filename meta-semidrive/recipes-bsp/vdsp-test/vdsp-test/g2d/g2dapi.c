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
#include "sdrv_g2d_cfg.h"
#include <linux/videodev2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define UNUSED(x) (void)x
#define G2D_DEVICE_NAME "/dev/g2d0"
#define HAL_PIXEL_FORMAT_VENDOR_EXT(fmt) (0x100 | (fmt & 0xFF))
#define HAL_PIXEL_FORMAT_NV12        HAL_PIXEL_FORMAT_VENDOR_EXT(6)


typedef struct _G2D_Context
{
    int  g2dFd;
    int refCnt;
} G2D_Context;

G2D_Context *gG2dContext = NULL;
pthread_mutex_t gG2dMutex = PTHREAD_MUTEX_INITIALIZER;

int g2d_is_open()
{
    struct g2d_capability cap;
    int ret;

    if ((gG2dContext != NULL) && (gG2dContext->g2dFd > 0) )
    {
        //slog_info("g2d_is_open gG2dContext has been inited");
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
    //slog_info("g2d_is_open succesfully refCnt:%d",gG2dContext->refCnt);
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
        //slog_info("g2d_open gG2dContext has been inited");
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
    //slog_info("g2d_open succesfully refCnt:%d",gG2dContext->refCnt);
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
    //slog_info("g2d_close enter");
    pthread_mutex_lock(&gG2dMutex);
    if(gG2dContext !=NULL)
    {
        //slog_info("g2d_close gG2dContext->refCnt:%d",gG2dContext->refCnt);
        gG2dContext->refCnt--;
        if(gG2dContext->refCnt <= 0)
        {
            if(gG2dContext->g2dFd > 0)
            {
                close(gG2dContext->g2dFd);
                //slog_info("g2d_close finish!!!");
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
    switch(format)
    {
        case DRM_FORMAT_BGRA8888:
        case DRM_FORMAT_RGBA8888:
        case DRM_FORMAT_ARGB8888:
            pitch = width*4;
            break;
        case DRM_FORMAT_NV21:
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_YUV420:
            pitch = width;
            break;
        case DRM_FORMAT_YUYV:
            pitch = width*2;
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
            //slog_info("g2d_convert_format outFormat:DRM_FORMAT_BGRA8888");
            outFormat= DRM_FORMAT_BGRA8888;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            //slog_info("g2d_convert_format :DRM_FORMAT_RGBA8888");
            outFormat= DRM_FORMAT_RGBA8888;
            break;
        case HAL_PIXEL_FORMAT_ARGB_8888:
            //slog_info("g2d_convert_format :DRM_FORMAT_ARGB8888\n");
            outFormat= DRM_FORMAT_ARGB8888;
            break;
        case V4L2_PIX_FMT_NV12:
            //slog_info("g2d_convert_format outFormat:DRM_FORMAT_NV21");
            outFormat = DRM_FORMAT_NV21;
            break;
        case HAL_PIXEL_FORMAT_YCBCR_420_888:
            //slog_info("g2d_convert_format :DRM_FORMAT_YUV420");
            outFormat = DRM_FORMAT_YUV420;
            break;
        case HAL_PIXEL_FORMAT_NV12:
        case HAL_PIXEL_FORMAT_YCRCB_420_SP:
        case V4L2_PIX_FMT_NV21:
            //slog_info("g2d_convert_format outFormat:DRM_FORMAT_NV21");
            outFormat = DRM_FORMAT_NV21;
            break;
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
        case V4L2_PIX_FMT_YUYV:
            //slog_info("g2d_convert_format outFormat:DRM_FORMAT_YUYV");
            outFormat = DRM_FORMAT_YUYV;
            break;
        default:
            slog_err("g2d_convert_format do not support format:%d",format);
        break;
    }
    return outFormat;
}

int g2d_get_format_nplanes(int format)
{
	switch (format) {
		case DRM_FORMAT_NV12:
		case DRM_FORMAT_NV21:
			return 2;
        case DRM_FORMAT_YVU420:
        case DRM_FORMAT_YUV420:
            return 3;
		default:
			return 1;
	}
	return 1;
}

int _g2d_convert(G2DInput *pInput)
{
    int ret;
    struct g2d_input input;
    //pthread_mutex_lock(&gG2dMutex);

    memset(&input,0x0,sizeof(struct g2d_input));
    input.layer_num = 1;
    struct dpc_layer *layer = &input.layer[0];
    layer->index = 2;
    layer->enable = 1;
    layer->format = g2d_convert_format(pInput->src_format);//DRM_FORMAT_ABGR8888
    layer->nplanes = g2d_get_format_nplanes(layer->format);
    layer->alpha = 0xff;
    layer->blend_mode = BLEND_PIXEL_NONE;
    //layer->rotation = pInput->rotation;
    layer->zpos = 0;
    layer->xfbc = 0;
    layer->modifier = 0;
    layer->width = pInput->src_w;
    layer->height = pInput->src_h;
    layer->src_x = pInput->src_crop_x;
    layer->src_y = pInput->src_crop_y;
    layer->src_w = pInput->src_crop_w;
    layer->src_h = pInput->src_crop_h;

    layer->dst_x = pInput->dst_crop_x;
    layer->dst_y = pInput->dst_crop_y;
    layer->dst_w = pInput->dst_crop_w;
    layer->dst_h = pInput->dst_crop_h;
    layer->bufs[0].fd = pInput->src_fd;
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

    input.output.width = pInput->dst_w;
    input.output.height = pInput->dst_h;
    input.output.fmt = g2d_convert_format(pInput->dst_format);
    //input.output.addr[0] = pInput->dst_addr;
    input.output.bufs[0].fd = pInput->dst_fd;
    input.output.nplanes = g2d_get_format_nplanes(input.output.fmt);

    switch(input.output.nplanes)
    {
        case 3:
            input.output.stride[0] = input.output.width;
            input.output.offsets[0] = 0;
            input.output.stride[1] = input.output.width/2;
            input.output.offsets[1] = input.output.width*input.output.height;
            input.output.stride[2] = input.output.width/2;
            input.output.offsets[2] = input.output.width*input.output.height*5/4;
            break;
        case 2:
            input.output.stride[0] = input.output.width;
            input.output.offsets[0] = 0;
            input.output.stride[1] = input.output.width;
            input.output.offsets[1] = input.output.width*input.output.height;
            break;
        case 1:
            input.output.stride[0] = g2d_get_pitch(input.output.fmt, input.output.width);
            break;
        default:
            slog_err("input.output:%d is error value",input.output.nplanes);
            break;
    }
    input.output.rotation = pInput->rotation;
#if  0
    FILE *saveinput;
    saveinput = fopen("/data/input1","w");
    if(saveinput)
    {
        fwrite(&input,1,sizeof(struct g2d_input),saveinput);
        fclose(saveinput);
    }
#endif
    ret = g2d_is_open();
    if(ret < 0)
    {
        return ret;
    }
    ret = ioctl(gG2dContext->g2dFd, G2D_IOCTL_POST_CONFIG, &input);
    if (ret) {
        slog_err("ioctl G2D_IOCTL_POST_CONFIG: ERROR");
        //pthread_mutex_unlock(&gG2dMutex);
        return ret;
    }

    //pthread_mutex_unlock(&gG2dMutex);
    return 0;
}

int g2d_blend(int src1_format,int src2_format,int dst_format,int src1_fd,int src2_fd,int dst_fd,int width,int height)
{
    int ret;
    struct g2d_input input;
	input.layer_num = 2;
    //pthread_mutex_lock(&gG2dMutex);

    memset(&input, 0x0, sizeof(struct g2d_input));
    //memset(input.layer, 0, sizeof(struct dpc_layer) * 4);
    struct dpc_layer *layerA = &input.layer[0];
    layerA->index = 0;
    layerA->enable = 1;
    layerA->format = g2d_convert_format(src1_format);
    layerA->nplanes = g2d_get_format_nplanes(layerA->format);
    layerA->alpha = 0xff;
    layerA->blend_mode = BLEND_PIXEL_COVERAGE;
    layerA->zpos = 0;
    layerA->xfbc = 0;
    layerA->modifier = 0;
    layerA->width = width;
    layerA->height = height;
    layerA->src_x = 0;
    layerA->src_y = 0;
    layerA->src_w = width;
    layerA->src_h = height;

    layerA->dst_x = 0;
    layerA->dst_y = 0;
    layerA->dst_w = width;
    layerA->dst_h = height;
    layerA->bufs[0].fd = src1_fd;
    switch(layerA->nplanes)
    {
        case 3:
            layerA->pitch[0] = layerA->width;
            layerA->offsets[0] = 0;
            layerA->pitch[1] = layerA->width/2;
            layerA->offsets[1] = layerA->width*layerA->height;
            layerA->pitch[2] = layerA->width/2;
            layerA->offsets[2] = layerA->width*layerA->height*5/4;
            break;
        case 2:
            layerA->pitch[0] = layerA->width;
            layerA->offsets[0] = 0;
            layerA->pitch[1] = layerA->width;
            layerA->offsets[1] = layerA->width*layerA->height;
            break;
        case 1:
            layerA->pitch[0] = g2d_get_pitch(layerA->format, layerA->width);
            break;
        default:
            slog_err("layerA->nplanes:%d is error value",layerA->nplanes);
            break;
    }

    struct dpc_layer *layerB = &input.layer[1];
    layerB->index = 1;
    layerB->enable = 1;
    layerB->format = g2d_convert_format(src2_format);
    layerB->nplanes = g2d_get_format_nplanes(layerB->format);
    layerB->alpha = 0xff;
    layerB->blend_mode = BLEND_PIXEL_COVERAGE;
    layerB->zpos = 1;
    layerB->xfbc = 0;
    layerB->modifier = 0;
    layerB->width = width;
    layerB->height = height;
    layerB->src_x = 0;
    layerB->src_y = 0;
    layerB->src_w = width;
    layerB->src_h = height;

    layerB->dst_x = 0;
    layerB->dst_y = 0;
    layerB->dst_w = width;
    layerB->dst_h = height;
    layerB->bufs[0].fd = src2_fd;
    switch(layerB->nplanes)
    {
        case 3:
            layerB->pitch[0] = layerB->width;
            layerB->offsets[0] = 0;
            layerB->pitch[1] = layerB->width/2;
            layerB->offsets[1] = layerB->width*layerB->height;
            layerB->pitch[2] = layerB->width/2;
            layerB->offsets[2] = layerB->width*layerB->height*5/4;
            break;
        case 2:
            layerB->pitch[0] = layerB->width;
            layerB->offsets[0] = 0;
            layerB->pitch[1] = layerB->width;
            layerB->offsets[1] = layerB->width*layerB->height;
            break;
        case 1:
            layerB->pitch[0] = g2d_get_pitch(layerB->format, layerB->width);
            break;
        default:
            slog_err("layerB->nplanes:%d is error value",layerB->nplanes);
            break;
    }

    input.output.width = width;
    input.output.height = height;
    input.output.fmt = g2d_convert_format(dst_format);
    //input.output.addr[0] = pInput->dst_addr;
    input.output.bufs[0].fd = dst_fd;
    input.output.nplanes = g2d_get_format_nplanes(input.output.fmt);

    switch(input.output.nplanes)
    {
        case 3:
            input.output.stride[0] = input.output.width;
            input.output.offsets[0] = 0;
            input.output.stride[1] = input.output.width/2;
            input.output.offsets[1] = input.output.width*input.output.height;
            input.output.stride[2] = input.output.width/2;
            input.output.offsets[2] = input.output.width*input.output.height*5/4;
            break;
        case 2:
            input.output.stride[0] = input.output.width;
            input.output.offsets[0] = 0;
            input.output.stride[1] = input.output.width;
            input.output.offsets[1] = input.output.width*input.output.height;
            break;
        case 1:
            input.output.stride[0] = g2d_get_pitch(input.output.fmt, input.output.width);
            break;
        default:
            slog_err("input.output:%d is error value",input.output.nplanes);
            break;
    }
    input.output.rotation = 0;
    ret = g2d_is_open();
    if(ret < 0)
    {
        return ret;
    }
    ret = ioctl(gG2dContext->g2dFd, G2D_IOCTL_POST_CONFIG, &input);
    if (ret) {
        slog_err("ioctl G2D_IOCTL_POST_CONFIG: ERROR");
        //pthread_mutex_unlock(&gG2dMutex);
        return ret;
    }

    //pthread_mutex_unlock(&gG2dMutex);
    return 0;
}

int g2d_convert(G2DInput *pInput)
{

    pthread_mutex_lock(&gG2dMutex);
    ////slog_info("g2d_convert ENTER,%d-%d,x:%d,y:%d",pInput->src_fd,pInput->dst_fd,pInput->src_crop_x,pInput->src_crop_y);
    int ret = 0;
    ret = _g2d_convert(pInput);
    ////slog_info("g2d_convert exit,%d-%d",pInput->src_fd,pInput->dst_fd);
    pthread_mutex_unlock(&gG2dMutex);
    return ret;
}

int g2d_format_convert(int src_format,int dst_format,int src_fd,int dst_fd,int width,int height)
{

    pthread_mutex_lock(&gG2dMutex);
    ////slog_info("g2d_format_convert ENTER:%d,%d",src_fd,dst_fd);
    int ret = 0;
    G2DInput g2dinput;

    g2dinput.src_fd = src_fd;
    g2dinput.src_w = width;
    g2dinput.src_h = height;
    g2dinput.src_crop_x = 0;
    g2dinput.src_crop_y = 0;
    g2dinput.src_crop_w = width;
    g2dinput.src_crop_h = height;
    g2dinput.dst_fd = dst_fd;
    g2dinput.dst_w = width;
    g2dinput.dst_h = height;
    g2dinput.dst_crop_x = 0;
    g2dinput.dst_crop_y =0;
    g2dinput.dst_crop_w = width;
    g2dinput.dst_crop_h = height;
    g2dinput.rotation = 0;
    g2dinput.src_format = src_format;
    g2dinput.dst_format = dst_format;

    ret =  _g2d_convert(&g2dinput);
    ////slog_info("g2d_format_convert EXIT %d,%d",src_fd,dst_fd);
    pthread_mutex_unlock(&gG2dMutex);
    return ret;
}

int g2d_format_scale(int src_format,int dst_format,int src_fd,int dst_fd,int src_w,int src_h,int dst_w,int dst_h)
{
    pthread_mutex_lock(&gG2dMutex);
    int ret=0;
    G2DInput g2dinput;

    ////slog_info("srcfd:%d,dstfd:%d,src_w:%d,src_h:%d,dst_w:%d,dst_h:%d",src_fd,dst_fd,src_w,src_h,dst_w,dst_h);
    g2dinput.src_fd = src_fd;
    g2dinput.src_w = src_w;
    g2dinput.src_h = src_h;
    g2dinput.src_crop_x = 0;
    g2dinput.src_crop_y = 0;
    g2dinput.src_crop_w = src_w;
    g2dinput.src_crop_h = src_h;
    g2dinput.dst_fd = dst_fd;
    g2dinput.dst_w = dst_w;
    g2dinput.dst_h = dst_h;
    g2dinput.dst_crop_x = 0;
    g2dinput.dst_crop_y =0;
    g2dinput.dst_crop_w = dst_w;
    g2dinput.dst_crop_h = dst_h;
    g2dinput.rotation = 0;
    g2dinput.src_format = src_format;
    g2dinput.dst_format = dst_format;

    ret = _g2d_convert(&g2dinput);
    pthread_mutex_unlock(&gG2dMutex);
    return ret;
}

g2d_dev_ops G2dDevOps =
{
    .g2dOpen=g2d_open,
    .g2dClose=g2d_close,
    .g2dConvert=g2d_convert,
};

g2d_dev_ops* GetG2dDevOps()
{
    return &G2dDevOps;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */


