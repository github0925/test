/*
 * File : g2dapi.h
 */

#ifndef _G2D_API_H_
#define _G2D_API_H_

#include <gst/gst.h>
#include <gst/video/video.h>

typedef struct _G2DInput
{
  int src_fd;             //输入fd
  int src_w;              //输入图像宽度
  int src_h;              //输入图像高度
  int src_crop_x;         //输入图像crop 坐标x
  int src_crop_y;         //输入图像crop 坐标y
  int src_crop_w;         //输入图像crop 的宽度
  int src_crop_h;         //输入图像crop 的高度
  int dst_fd;             //输出fd
  int dst_w;              //输出图像宽度
  int dst_h;              //输出图像高度
  int dst_crop_x;         //输出图像crop 坐标x
  int dst_crop_y;         //输出图像crop 坐标y
  int dst_crop_w;         //输出图像crop 的宽度
  int dst_crop_h;         //输出图像crop 的高度
  int rotation;           //旋转参数
  int src_format;         //输入图像格式
  int dst_format;         //输出图像格式
  unsigned long dst_addr; // temp输出图像物理地址，不使用
} G2DInput;

typedef struct _g2d_dev_ops
{
  int (*g2dOpen) ();
  int (*g2dClose) ();
  int (*g2dConvert) (G2DInput *pInput);
} g2d_dev_ops;

int g2d_open (GstObject *object);
int g2d_close ();
int g2d_format_convert (int src_format, int dst_format, int src_fd, int dst_fd,
                        int width, int height);
int g2d_format_scale (int src_format, int dst_format, int src_fd, int dst_fd,
                      int src_w, int src_h, int dst_w, int dst_h);
int g2d_convert (G2DInput *pInput);
g2d_dev_ops *GetG2dDevOps ();
int g2d_get_pitch_format (int format, int width);
int g2d_convert_format (int format);

#endif //  _G2D_API_H_
