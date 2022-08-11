/*
* File : g2dapi.h
*/

#ifndef _G2D_API_H_
#define _G2D_API_H_

#include <sdrv_g2d_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif


struct err_index {
	int in_index;
	int out_index;
};

struct img_format_file {
	int format;
	char file_patch[128];
};


enum {
	IN_FORMAT_XRGB8888 = 0,
	IN_FORMAT_XBGR8888,
	IN_FORMAT_BGRX8888,
	IN_FORMAT_ARGB8888,
	IN_FORMAT_ABGR8888,
	IN_FORMAT_BGRA8888,
	IN_FORMAT_RGB888,
	IN_FORMAT_BGR888,
	IN_FORMAT_RGB565,
	IN_FORMAT_BGR565,
	IN_FORMAT_XRGB4444,
	IN_FORMAT_XBGR4444,
	IN_FORMAT_BGRX4444,
	IN_FORMAT_ARGB4444,
	IN_FORMAT_ABGR4444,
	IN_FORMAT_BGRA4444,
	IN_FORMAT_XRGB1555,
	IN_FORMAT_XBGR1555,
	IN_FORMAT_BGRX5551,
	IN_FORMAT_ARGB1555,
	IN_FORMAT_ABGR1555,
	IN_FORMAT_BGRA5551,
	IN_FORMAT_XRGB2101010,
	IN_FORMAT_XBGR2101010,
	IN_FORMAT_BGRX1010102,
	IN_FORMAT_ARGB2101010,
	IN_FORMAT_ABGR2101010,
	IN_FORMAT_BGRA1010102,
	IN_FORMAT_R8,
	IN_FORMAT_YUYV,
	IN_FORMAT_UYVY,
	IN_FORMAT_VYUY,
	IN_FORMAT_AYUV,
	IN_FORMAT_NV12,
	IN_FORMAT_NV21,
	IN_FORMAT_YUV420,
	IN_FORMAT_YVU420,
	IN_FORMAT_YUV422,
	IN_FORMAT_YUV444,
	IN_FORMAT_MAX,
};

enum {
	OUT_FORMAT_R8 = 0,
	OUT_FORMAT_RGB565,
	OUT_FORMAT_RGB888,
	OUT_FORMAT_XRGB4444,
	OUT_FORMAT_ARGB4444,
	OUT_FORMAT_XRGB1555,
	OUT_FORMAT_ARGB1555,
	OUT_FORMAT_XRGB8888,
	OUT_FORMAT_ARGB8888,
	OUT_FORMAT_XRGB2101010,
	OUT_FORMAT_ARGB2101010,
	OUT_FORMAT_YUYV,
	OUT_FORMAT_UYVY,
	OUT_FORMAT_AYUV,
	OUT_FORMAT_NV21,
	OUT_FORMAT_YUV420,
	OUT_FORMAT_YUV422,
	OUT_FORMAT_YUV444,
	OUT_FORMAT_MAX,
};

struct _G2DLayer
{
    int layer_index;
    int src_fd; //输入fd
    int src_w;//输入图像宽度
    int src_h;//输入图像高度
    int src_crop_x;//输入图像crop 坐标x
    int src_crop_y;//输入图像crop 坐标y
    int src_crop_w;//输入图像crop 的宽度
    int src_crop_h;//输入图像crop 的高度
    int dst_crop_x;//输出图像crop 坐标x
    int dst_crop_y;//输出图像crop 坐标y
    int dst_crop_w;//输出图像crop 的宽度
    int dst_crop_h;//输出图像crop 的高度
    int src_format;//输入图像格式
    int blend_mode;//输入图像合成模式
    int alpha;//输入图像alpha通道
    int zpos;
	void *input_vaddr;
};

typedef struct _G2DInput
{
	int layer_num;
	struct _G2DLayer layer[G2D_LAYER_MAX_NUM];
	int dst_fd;//输出fd
    int dst_w;//输出图像宽度
    int dst_h;//输出图像高度
    int dst_format;//输出图像格式
    int rotation;//旋转参数
    void *dst_vaddr;//temp输出图像物理地址，不使用
}G2DInput;

typedef struct _g2d_dev_ops {
    int (*g2dOpen)();
    int (*g2dClose)();
    int (*g2dRotation)(int src_format, int dst_format, int src_fd, int dst_fd,
		int width, int height, rotation_type rotation);
	int (*g2dFormatScale)(int src_format,int dst_format,int src_fd,int dst_fd,
		int src_w,int src_h,int dst_w,int dst_h);
	int (*g2dFormatConvert)(int      src_format,int dst_format,int src_fd,int dst_fd,int width,int height);
	int (*g2dFastcopy)(int src_fd, int dst_fd, unsigned int width, unsigned int height,
		unsigned int src_pitch, unsigned int dst_stride, unsigned int fmt);
	int (*g2dFillrect)(struct g2d_bg_cfg *bgcfg, struct g2d_output_cfg *outcfg);
    int (*g2dConvert)(G2DInput *pInput);
}g2d_dev_ops;

int g2d_open();
int g2d_close();
int g2d_format_convert(int src_format,int dst_format,int src_fd,int dst_fd,int width,int height);
int g2d_format_scale(int src_format,int dst_format,int src_fd,int dst_fd,
	int src_w,int src_h,int dst_w,int dst_h);
int g2d_convert(G2DInput *pInput);
g2d_dev_ops* GetG2dDevOps();
int g2d_get_pitch_format(int format,int width);
int g2d_convert_format(int format);


#ifdef __cplusplus
}
#endif

#endif//  _G2D_API_H_
