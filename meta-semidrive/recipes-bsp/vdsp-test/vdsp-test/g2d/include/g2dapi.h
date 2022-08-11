/*
* File : g2dapi.h
*/

#ifndef _G2D_API_H_
#define _G2D_API_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_PIXEL_FORMAT_RGBA_8888 = 1,
    HAL_PIXEL_FORMAT_RGBX_8888 = 2,
    HAL_PIXEL_FORMAT_RGB_888 = 3,
    HAL_PIXEL_FORMAT_RGB_565 = 4,
    HAL_PIXEL_FORMAT_BGRA_8888 = 5,
    HAL_PIXEL_FORMAT_ARGB_8888 = 6,
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
} android_pixel_format_t;

#define slog_info(...) do { \
    printf("%s %s:%d INFO:", __FUNCTION__,__FILE__,__LINE__)&& \
    printf(__VA_ARGS__)&& \
    printf("\n"); \
}while(0)

#define slog_war(...) do { \
    printf("%s %s:%d WAR:", __FUNCTION__,__FILE__,__LINE__)&& \
    printf(__VA_ARGS__)&& \
    printf("\n"); \
}while(0)

#define slog_err(...) do { \
    printf("%s %s:%d ERR:", __FUNCTION__,__FILE__,__LINE__)&& \
    printf(__VA_ARGS__)&& \
    printf("\n"); \
}while(0)

typedef struct _G2DInput
{
    int src_fd; //输入fd
    int src_w;//输入图像宽度
    int src_h;//输入图像高度
    int src_crop_x;//输入图像crop 坐标x
    int src_crop_y;//输入图像crop 坐标y
    int src_crop_w;//输入图像crop 的宽度
    int src_crop_h;//输入图像crop 的高度
    int dst_fd;//输出fd
    int dst_w;//输出图像宽度
    int dst_h;//输出图像高度
    int dst_crop_x;//输出图像crop 坐标x
    int dst_crop_y;//输出图像crop 坐标y
    int dst_crop_w;//输出图像crop 的宽度
    int dst_crop_h;//输出图像crop 的高度
    int rotation;//旋转参数
    int src_format;//输入图像格式
    int dst_format;//输出图像格式
    unsigned long dst_addr;//temp输出图像物理地址，不使用
}G2DInput;

typedef struct _g2d_dev_ops {
    int (*g2dOpen)();
    int (*g2dClose)();
    int (*g2dConvert)(G2DInput *pInput);
}g2d_dev_ops;

int g2d_open();
int g2d_close();
int g2d_format_convert(int src_format,int dst_format,int src_fd,int dst_fd,int width,int height);
int g2d_format_scale(int src_format,int dst_format,int src_fd,int dst_fd,int src_w,int src_h,int dst_w,int dst_h);
int g2d_convert(G2DInput *pInput);
g2d_dev_ops* GetG2dDevOps();
int g2d_get_pitch_format(int format,int width);
int g2d_blend(int src1_format,int src2_format,int dst_format,int src1_fd,int src2_fd,int dst_fd,int width,int height);


#ifdef __cplusplus
}
#endif

#endif//  _G2D_API_H_
