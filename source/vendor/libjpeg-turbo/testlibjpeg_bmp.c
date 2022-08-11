#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>

#include "jpeglib.h"
#include "jerror.h"

/************************start bmp_utils.h****************************/
#ifndef _BMP_UTILS_H
#define _BMP_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WIN32
#include <Windows.h>
#else
typedef UINT8 BYTE;
typedef UINT16 WORD;
typedef __uint32_t DWORD;
typedef __int32_t LONG;

#pragma pack(push)
#pragma pack(2)

//14
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;

// 40
typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER; //__attribute__ ((aligned(2)));

typedef struct tagRGBQUAD
{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
} BITMAPINFO; // __attribute__ ((aligned(2)));

#pragma pack(pop)

#endif

#undef ALIGN
#define ALIGN(x, n) (((x) + (n)-1) & ~((n)-1))

    void swap_rgb(unsigned char *rgb_buffer, int len);
    int analyse_bmp_file(const char *bmp_file);
    int read_bmp_file(const char *bmp_file, unsigned char **rgb_buffer,
                      int *size, int *width, int *height);
    int read_bmp_file_1(const char *bmp_file, unsigned char **rgb_buffer, int *rgb_size,
                        unsigned char **palette_buf, int *palette_len,
                        int *width, int *height);
    int write_bmp_file(const char *bmp_file, unsigned char *rgb_buffer, int width, int height);
    int write_bmp_file_1(const char *bmp_file, unsigned char *rgb_buffer,
                         unsigned char *palette_buf, int *palette_len,
                         int width, int height);
#ifdef __cplusplus
};
#endif

#endif /* _BMP_UTILS_H */
/************************end bmp_utils.h****************************/

/************************start bmp_utils.c****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

int analyse_bmp_file(const char *bmp_file)
{
#if 0
    FILE* fp;
    BITMAPFILEHEADER bmpHeader;
    BITMAPINFOHEADER bmpInfo;
    int rgb_size1 = 0;
    int rgb_size2 = 0;
    int width = 0;
    int height = 0;
    int padding = 0;
    int stride_byte = 0;
    int color_num = 0;
    int paltette_len = 0;

    char* palette = NULL;

    fp = fopen(bmp_file, "rb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", bmp_file);
        return -1;
    }

    fread(&bmpHeader, 1, sizeof(BITMAPFILEHEADER), fp);
    fread(&bmpInfo, 1, sizeof(BITMAPINFOHEADER), fp);

    if (bmpHeader.bfType != (('M' << 8) | 'B'))
    {
        printf("Sorry, not bmp picture.\n");
        return -1;
    }

    width = bmpInfo.biWidth;
    height = (int)fabs((double)bmpInfo.biHeight);

    switch(bmpInfo.biBitCount)
    {
    case 1:
        color_num = 2;
        break;
    case 4:
        color_num = 16;
        break;
    case 8:
        color_num = 256;
        break;
    case 24:
    default:
        color_num = 0;
        break;
    }

    stride_byte = ALIGN(width*bmpInfo.biBitCount/8, 4);
    padding = stride_byte - width*bmpInfo.biBitCount/8;
    paltette_len = color_num * sizeof(RGBQUAD);

    rgb_size1 = bmpHeader.bfSize - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER) - paltette_len;
    rgb_size2 = stride_byte*height;

    printf("file name: %s\n", bmp_file);
    printf("file type: %c%c %x\n", (bmpHeader.bfType)>>8, (bmpHeader.bfType)&0xff, bmpHeader.bfType);
    printf("file size: %d(B) = %0.2f(KB) = %0.2f(MB)\n", bmpHeader.bfSize, (float)bmpHeader.bfSize/1024.00, (float)bmpHeader.bfSize/1024.00/1024.00);
    printf("offset of image data: %d\n", bmpHeader.bfOffBits);
    //////////////////////////////////

    printf("biSize: %d\n", bmpInfo.biSize);
    printf("width: %d\n", bmpInfo.biWidth);
    printf("height: %d\n", bmpInfo.biHeight);
    printf("Plane: %d\n", bmpInfo.biPlanes);
    printf("BitCount: %d\n", bmpInfo.biBitCount);
    printf("biCompression: %d\n", bmpInfo.biCompression);
    printf("biSizeImage: %d\n", bmpInfo.biSizeImage);
    printf("XPelsPerMeter: %d\n", bmpInfo.biXPelsPerMeter);
    printf("YPelsPerMeter: %d\n", bmpInfo.biYPelsPerMeter);
    printf("biClrUsed: %d\n", bmpInfo.biClrUsed);
    printf("biClrImportant: %d\n", bmpInfo.biClrImportant);

    printf("width*3: %d stride byte: %d padding: %d\n", width*3, stride_byte, padding);

    printf("rgb buffer size: %d %d\n", rgb_size1,rgb_size2);

    if (color_num != 0)
    {
        palette = (char *)malloc(paltette_len * sizeof(char));
        fread(palette, paltette_len, 1, fp);
        printf("palette:\n");
        //dump(palette, paltette_len);
    }
#endif
    return 0;
}

int read_bmp_file(const char *bmp_file, unsigned char **rgb_buffer,
                  int *size, int *width, int *height)
{
    int ret = 0;
    FILE *fp = NULL;
    BITMAPFILEHEADER bmpHeader;
    BITMAPINFOHEADER bmpInfo;
    int tmp_width = 0;
    int tmp_height = 0;
    int rgb_size = 0;
    int stride_byte = 0;
    int width_byte = 0;
    int padding = 0;
    unsigned char *tmp_buf = 0;
    int color_num = 0;
    int palette_len = 0;
    int i = 0;

    fp = fopen(bmp_file, "rb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", bmp_file);
        return -1;
    }

    ret = fread(&bmpHeader, 1, sizeof(BITMAPFILEHEADER), fp);
    if (ret != sizeof(BITMAPFILEHEADER))
    {
        printf("read BITMAPFILEHEADER failed.\n");
        ret = -1;
        goto end;
    }

    ret = fread(&bmpInfo, 1, sizeof(BITMAPINFOHEADER), fp);
    if (ret != sizeof(BITMAPINFOHEADER))
    {
        printf("read BITMAPINFOHEADER failed read: %d %d.\n", ret, sizeof(BITMAPINFOHEADER));
        ret = -1;
        goto end;
    }

    if (bmpHeader.bfType != (('M' << 8) | 'B'))
    {
        printf("Sorry, not bmp picture.\n");
        ret = -1;
        goto end;
    }
    tmp_width = bmpInfo.biWidth;
    tmp_height = (int)fabs((double)bmpInfo.biHeight);

    printf("tmp_width:%d tmp_height:%d\n", tmp_width, tmp_height);
    rgb_size = tmp_width * tmp_height * bmpInfo.biBitCount / 8;

    *width = tmp_width;
    *height = tmp_height;
    *size = rgb_size;
    /**
     *
     * stride_byte = (width * bmpInfo.biBitCount/8+3)/4*4;
     */
    stride_byte = ALIGN(tmp_width * bmpInfo.biBitCount / 8, 4);
    width_byte = tmp_width * bmpInfo.biBitCount / 8;

    /**
     *
     * padding = (4 - width * 3 % 4) % 4;
     *
     */
    padding = stride_byte - width_byte;

    switch (bmpInfo.biBitCount)
    {
    case 1:
        color_num = 2;
        break;
    case 4:
        color_num = 16;
        break;
    case 8:
        color_num = 256;
        break;
    case 24:
    default:
        color_num = 0;
        break;
    }
    palette_len = color_num * sizeof(RGBQUAD);

    if (bmpHeader.bfOffBits != sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + palette_len)
    {
        ret = -1;
        goto end;
    }

    printf("debug--:\nfile size: %d rgb size: %d %d stride byte: %d res: %dx%d padding: %d BitCount: %d\n",
           (int)bmpHeader.bfSize, rgb_size, stride_byte * tmp_height, stride_byte, (int)bmpInfo.biWidth, (int)bmpInfo.biHeight, padding, bmpInfo.biBitCount);

    if (color_num != 0)
    {
        fseek(fp, palette_len, SEEK_CUR);
    }

    *rgb_buffer = (unsigned char *)malloc(sizeof(char) * rgb_size);
    if (*rgb_buffer == NULL)
    {
        ret = -1;
        goto end;
    }

    if (bmpInfo.biHeight > 0)
    {
        tmp_buf = *rgb_buffer + rgb_size;
        for (i = 0; i < tmp_height; i++)
        {
            tmp_buf -= width_byte;
            ret = fread(tmp_buf, 1, width_byte, fp);
            if (ret != width_byte)
            {
                free(*rgb_buffer);
                ret = -1;
                goto end;
            }
            fseek(fp, padding, SEEK_CUR);
        }
    }
    else
    {
        unsigned char *tmp_buf = *rgb_buffer;
        for (i = 0; i < tmp_height; i++)
        {
            ret = fread(tmp_buf, 1, width_byte, fp);
            if (ret != width_byte)
            {
                free(*rgb_buffer);
                ret = -1;
                goto end;
            }
            fseek(fp, padding, SEEK_CUR);
            tmp_buf += width_byte;
        }
    }

end:
    fclose(fp);
    return ret;
}

int write_bmp_file(const char *bmp_file, unsigned char *rgb_buffer, int width, int height)
{
#define BPP 24

    BITMAPFILEHEADER bmpHeader;
    BITMAPINFOHEADER bmpInfo;
    FILE *fp = NULL;
    int offset = 0;
    int stride_byte = 0;
    int width_byte = 0;
    int rgb_size = 0;
    int padding = 0;
    unsigned char *tmp_buf = NULL;
    int i = 0;
    int ret = 0;

    fp = fopen(bmp_file, "wb");
    if (fp == NULL)
    {
        printf("open %s failed\n", bmp_file);
        return -1;
    }

    offset = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
    // stride_byte = ((width * 24 + 31) >> 5) << 2;
    stride_byte = ALIGN(width * BPP / 8, 4);
    width_byte = width * BPP / 8;
    rgb_size = stride_byte * (int)fabs((double)height);

    bmpHeader.bfType = ('M' << 8) | 'B';
    bmpHeader.bfSize = offset + rgb_size;
    bmpHeader.bfReserved1 = 0;
    bmpHeader.bfReserved2 = 0;
    bmpHeader.bfOffBits = offset;

    bmpInfo.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.biWidth = width;
    bmpInfo.biHeight = height;
    bmpInfo.biPlanes = 1;
    bmpInfo.biBitCount = BPP;
    bmpInfo.biCompression = 0;
    bmpInfo.biSizeImage = rgb_size;
    bmpInfo.biXPelsPerMeter = 0;
    bmpInfo.biYPelsPerMeter = 0;
    bmpInfo.biClrUsed = 0;
    bmpInfo.biClrImportant = 0;

    //padding = (4 - width * 3 % 4) % 4;
    padding = stride_byte - width_byte;

    printf("debug--:\nwidth: %d height: %d padding: %d rgb_size: %d, stride_byte: %d\n",
           width, height, padding, rgb_size, stride_byte);
    tmp_buf = (unsigned char *)malloc(sizeof(char) * rgb_size);
    if (tmp_buf == NULL)
    {
        ret = -1;
        goto end;
    }
    memset(tmp_buf, '\0', sizeof(char) * rgb_size);

    if (height > 0)
    {
        for (i = 0; i < height; i++)
        {
            memcpy(tmp_buf + i * stride_byte, rgb_buffer + (height - i - 1) * width_byte, width_byte);
        }
    }
    else
    {
        for (i = 0; i < -height; i++)
        {
            memcpy(tmp_buf + i * stride_byte, rgb_buffer + i * width_byte, width_byte);
        }
    }

    fwrite(&bmpHeader, 1, sizeof(BITMAPFILEHEADER), fp);
    fwrite(&bmpInfo, 1, sizeof(BITMAPINFOHEADER), fp);
    fwrite(tmp_buf, 1, rgb_size, fp);

end:
    if (tmp_buf != NULL)
    {
        free(tmp_buf);
    }
    fclose(fp);

    return ret;
}

// rgb --> bgr or
// bgr --> rgb
void swap_rgb(unsigned char *rgb_buffer, int len)
{
    int i = 0;
    for (i = 0; i < len; i += 3)
    {
        unsigned char tmp;
        tmp = rgb_buffer[i];
        rgb_buffer[i] = rgb_buffer[i + 2];
        rgb_buffer[i + 1] = rgb_buffer[i + 1];
        rgb_buffer[i + 2] = tmp;
    }
}
/************************end bmp_utils.c****************************/

/************************start jpeg_bmp.h****************************/
#ifndef _JPEG_BMP_H
#define _JPEG_BMP_H

#ifdef __cplusplus
extern "C"
{
#endif

    // JPEG-->BMP
    int jpg_to_bmp(const char *jpg_file, const char *bmp_file);
    int jpg_to_bmp1(const char *jpg_file, const char *bmp_file);
    int jpg_to_bmp2(const char *jpg_file, const char *bmp_file);

    // BMP-->JPEG
    int bmp_to_jpg(const char *bmp_file, const char *jpg_file);
    int bmp_to_jpg1(const char *bmp_file, const char *jpg_file);

#ifdef __cplusplus
};
#endif

#endif /* _JPEG_BMP_H */
/************************end jpeg_bmp.h****************************/

/************************start jpeg_bmp.c****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
//#include <math.h>
#include <sys/time.h>

#if 1
unsigned int get_tick_count()
{
#ifdef WIN32
    return GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
#if 0
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned int time = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return time;
#endif
}
#endif

int jpg_to_bmp(const char *jpg_file, const char *bmp_file)
{
    unsigned char *buffer = NULL;
    int size;
    int width, height;

    read_jpeg_file(jpg_file, &buffer, &size, &width, &height);

    swap_rgb(buffer, size);

    write_bmp_file(bmp_file, buffer, width, height);

    free(buffer);

    return 0;
}

int jpg_to_bmp1(const char *jpg_file, const char *bmp_file)
{
    int width, height;
    FILE *fp = NULL;
    unsigned char *jpeg_buffer = NULL;
    unsigned int jpeg_size = 0;
    unsigned int read_size = 0;
    int ret = 0;

    unsigned char *rgb_buffer = NULL;
    int rgb_size = 512 * 512 * 3;

    fp = fopen(jpg_file, "rb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", jpg_file);
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    jpeg_size = ftell(fp);
    rewind(fp);

    jpeg_buffer = (unsigned char *)malloc(sizeof(char) * jpeg_size);
    if (jpeg_buffer == NULL)
    {
        ret = -1;
        goto end;
    }
    read_size = fread(jpeg_buffer, 1, jpeg_size, fp);
    if (read_size != jpeg_size)
    {
        ret = -1;
        goto end;
    }
    rgb_buffer = (unsigned char *)malloc(sizeof(char) * rgb_size);
    jpeg2rgb(jpeg_buffer, jpeg_size, rgb_buffer, &rgb_size, &width, &height);

    swap_rgb(rgb_buffer, rgb_size);

    write_bmp_file(bmp_file, rgb_buffer, width, height);

end:
    if (jpeg_buffer != NULL)
    {
        free(jpeg_buffer);
        jpeg_buffer = NULL;
    }
    fclose(fp);
    return ret;
}

int jpg_to_bmp2(const char *jpg_file, const char *bmp_file)
{
    int width, height, components;
    FILE *fp = NULL;
    unsigned char *jpeg_buffer = NULL;
    unsigned int jpeg_size = 0;
    unsigned int read_size = 0;
    int ret = 0;

    unsigned char *rgb_buffer = NULL;
    int rgb_size = 0;

    fp = fopen(jpg_file, "rb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", jpg_file);
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    jpeg_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    jpeg_buffer = (unsigned char *)malloc(sizeof(char) * jpeg_size);
    if (jpeg_buffer == NULL)
    {
        ret = -1;
        goto end;
    }
    read_size = fread(jpeg_buffer, 1, jpeg_size, fp);
    if (read_size != jpeg_size)
    {
        ret = -1;
        goto end;
    }

    jpeg_header(jpeg_buffer, jpeg_size, &width, &height, &components);

    rgb_size = width * height * components;
    rgb_buffer = (unsigned char *)malloc(sizeof(char) * rgb_size);

    printf("read jpeg header %d %d %d total: %d\n", width, height, components, rgb_size);

    jpeg2rgb1(jpeg_buffer, jpeg_size, rgb_buffer, &rgb_size);

    swap_rgb(rgb_buffer, rgb_size);

    write_bmp_file(bmp_file, rgb_buffer, width, height);

end:
    if (jpeg_buffer != NULL)
    {
        free(jpeg_buffer);
        jpeg_buffer = NULL;
    }
    fclose(fp);
    return ret;
}

int bmp_to_jpg(const char *bmp_file, const char *jpg_file)
{
    unsigned char *buffer = NULL;
    int size;
    int width, height;

    read_bmp_file(bmp_file, &buffer, &size, &width, &height);
    swap_rgb(buffer, size);
    printf("size: %d, width: %d height: %d\n", size, width, height);
    int a = get_tick_count();
    write_jpeg_file(jpg_file, buffer, width, height, 100);
    int b = get_tick_count();
    printf("write jpeg file. time: %d\n", b - a);

    free(buffer);

    return 0;
}

int bmp_to_jpg1(const char *bmp_file, const char *jpg_file)
{
    FILE *fp = NULL;
    unsigned char *buffer = NULL;
    int size;
    int width, height;
    unsigned char *jpg_buffer = NULL;
    unsigned long jpg_size = 0;

    read_bmp_file(bmp_file, &buffer, &size, &width, &height);
    swap_rgb(buffer, size);
    printf("size: %d, width: %d height: %d\n", size, width, height);

    //write_jpeg_file(jpg_file, buffer, width, height, 50);
    int a = get_tick_count();
    rgb2jpeg(buffer, width, height, 100, &jpg_buffer, &jpg_size);
    int b = get_tick_count();

    printf("got jpeg size: %d time: %d\n", (int)jpg_size, b - a);

    fp = fopen(jpg_file, "wb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", jpg_file);
        return -1;
    }
    fwrite(jpg_buffer, 1, jpg_size, fp);

    free(buffer);
    free(jpg_buffer);

    fclose(fp);
    return 0;
}
/************************end jpeg_bmp.c****************************/

/************************start jpeg_utils.h****************************/
#ifndef _JPEG_UTILS_H
#define _JPEG_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

    int read_jpeg_file(const char *jpeg_file, unsigned char **rgb_buffer, int *size, int *width, int *height);
    int write_jpeg_file(const char *jpeg_file, unsigned char *rgb_buffer, int width, int height, int quality);
    int jpeg2rgb(unsigned char *jpeg_buffer, int jpeg_size, unsigned char *rgb_buffer, int *size, int *width, int *height);
    int jpeg_header(unsigned char *jpeg_buffer, int jpeg_size, int *width, int *height, int *components);
    int jpeg2rgb1(unsigned char *jpeg_buffer, int jpeg_size, unsigned char *rgb_buffer, int *size);
    int rgb2jpeg(unsigned char *rgb_buffer, int width, int height, int quality, unsigned char **jpeg_buffer, unsigned long *jpeg_size);

#ifdef __cplusplus
};
#endif

#endif /* _JPEG_UTILS_H */

/************************end jpeg_utils.h****************************/

/************************start jpeg_utils.c****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
//#include <math.h>
#include "jpeglib.h"
#include "jerror.h"

struct my_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr *my_error_ptr;

void my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr)cinfo->err;

    (*cinfo->err->output_message)(cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

int read_jpeg_file(const char *jpeg_file, unsigned char **rgb_buffer, int *size, int *width, int *height)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE *fp;

    JSAMPARRAY buffer;
    int row_stride = 0;
    unsigned char *tmp_buffer = NULL;
    int rgb_size;

    fp = fopen(jpeg_file, "rb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", jpeg_file);
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return -1;
    }

    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, fp);

    jpeg_read_header(&cinfo, TRUE);

#ifdef WITH_JPU_HW
    cinfo.hw_decompress = TRUE;
#endif
    //cinfo.out_color_space = JCS_RGB; //JCS_YCbCr;

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    *width = cinfo.output_width;
    *height = cinfo.output_height;

    rgb_size = row_stride * cinfo.output_height;
    *size = rgb_size;

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    *rgb_buffer = (unsigned char *)malloc(sizeof(char) * rgb_size);

    printf("debug--:\nrgb_size: %d, size: %d w: %d h: %d row_stride: %d \n", rgb_size,
           cinfo.image_width * cinfo.image_height * 3,
           cinfo.image_width,
           cinfo.image_height,
           row_stride);
    tmp_buffer = *rgb_buffer;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(tmp_buffer, buffer[0], row_stride);
        tmp_buffer += row_stride;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(fp);

    return 0;
}

int write_jpeg_file(const char *jpeg_file, unsigned char *rgb_buffer, int width, int height, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int row_stride = 0;
    FILE *fp = NULL;
    JSAMPROW row_pointer[1];

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);
    fp = fopen(jpeg_file, "wb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", jpeg_file);
        return -1;
    }
    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
#ifdef WITH_JPU_HW
    cinfo.hw_encompress = TRUE;
#endif

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, 1); // todo 1 == true
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = width * cinfo.input_components;

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &rgb_buffer[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp);

    return 0;
}

//////////////////////////////////////////////////////////

int jpeg2rgb(unsigned char *jpeg_buffer, int jpeg_size, unsigned char *rgb_buffer, int *size, int *width, int *height)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;

    JSAMPARRAY buffer;
    int row_stride = 0;
    unsigned char *tmp_buffer = NULL;
    int rgb_size;

    if (jpeg_buffer == NULL)
    {
        printf("no jpeg buffer here.\n");
        return -1;
    }
    if (rgb_buffer == NULL)
    {
        printf("you need to alloc rgb buffer.\n");
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        return -1;
    }

    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, jpeg_buffer, jpeg_size);

    jpeg_read_header(&cinfo, TRUE);

#ifdef WITH_JPU_HW
    cinfo.hw_decompress = TRUE;
#endif
    //cinfo.out_color_space = JCS_EXT_BGR;//JCS_RGB; //JCS_YCbCr;

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    *width = cinfo.output_width;
    *height = cinfo.output_height;

    rgb_size = row_stride * cinfo.output_height;
    if (*size < rgb_size)
    {
        printf("rgb buffer to small, we need %d but has only: %d\n", rgb_size, *size);
    }

    *size = rgb_size;

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    printf("debug--:\nrgb_size: %d, size: %d w: %d h: %d row_stride: %d \n", rgb_size,
           cinfo.image_width * cinfo.image_height * 3,
           cinfo.image_width,
           cinfo.image_height,
           row_stride);
    tmp_buffer = rgb_buffer;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(tmp_buffer, buffer[0], row_stride);
        tmp_buffer += row_stride;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return 0;
}

int jpeg_header(unsigned char *jpeg_buffer, int jpeg_size, int *width, int *height, int *components)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;

    //int row_stride = 0;

    if (jpeg_buffer == NULL)
    {
        printf("no jpeg buffer here.\n");
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        return -1;
    }

    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, jpeg_buffer, jpeg_size);

    jpeg_read_header(&cinfo, TRUE);

#ifdef WITH_JPU_HW
    cinfo.hw_decompress = TRUE;
#endif

    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;
    *components = cinfo.output_components;

    return 0;
}

int jpeg2rgb1(unsigned char *jpeg_buffer, int jpeg_size, unsigned char *rgb_buffer, int *size)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;

    JSAMPARRAY buffer;
    int row_stride = 0;
    unsigned char *tmp_buffer = NULL;
    int rgb_size;

    if (jpeg_buffer == NULL)
    {
        printf("no jpeg buffer here.\n");
        return -1;
    }
    if (rgb_buffer == NULL)
    {
        printf("you need to alloc rgb buffer.\n");
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        return -1;
    }

    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, jpeg_buffer, jpeg_size);

    jpeg_read_header(&cinfo, TRUE);

#ifdef WITH_JPU_HW
    cinfo.hw_decompress = TRUE;
#endif
    //cinfo.out_color_space = JCS_EXT_BGR;//JCS_RGB; //JCS_YCbCr;

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;

    rgb_size = row_stride * cinfo.output_height;
    if (*size < rgb_size)
    {
        printf("rgb buffer to small, we need %d but has only: %d\n", rgb_size, *size);
    }

    *size = rgb_size;

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    printf("debug--:\nrgb_size: %d, size: %d w: %d h: %d row_stride: %d \n", rgb_size,
           cinfo.image_width * cinfo.image_height * 3,
           cinfo.image_width,
           cinfo.image_height,
           row_stride);
    tmp_buffer = rgb_buffer;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(tmp_buffer, buffer[0], row_stride);
        tmp_buffer += row_stride;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return 0;
}

int rgb2jpeg(unsigned char *rgb_buffer, int width, int height, int quality, unsigned char **jpeg_buffer, unsigned long *jpeg_size)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int row_stride = 0;
    JSAMPROW row_pointer[1];

    if (jpeg_buffer == NULL)
    {
        printf("you need a pointer for jpeg buffer.\n");
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);

    jpeg_mem_dest(&cinfo, jpeg_buffer, jpeg_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
#ifdef WITH_JPU_HW
    cinfo.hw_encompress = TRUE;
#endif

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, 1); // todo 1 == true
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = width * cinfo.input_components;

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &rgb_buffer[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return 0;
}

/************************end jpeg_utils.c****************************/

int main(int argc, char *argv[])
{
    int count = 0;
    char src_pic[32] = {0};
    char jpg_pic[32] = {0};
    char bmp_pic[32] = {0};
    int width = 0;

    width = 512;

    sprintf(src_pic, "test_%d.bmp", width);
    sprintf(jpg_pic, "test_out_%d_%d.jpg", width, count);
    sprintf(bmp_pic, "test_a_%d_%d.bmp", width, count);

    bmp_to_jpg1(src_pic, jpg_pic);

    jpg_to_bmp(jpg_pic, bmp_pic);

    count++;
    sprintf(bmp_pic, "test_a_%d_%d.bmp", width, count);
    jpg_to_bmp2(jpg_pic, bmp_pic);
    return 0;
}