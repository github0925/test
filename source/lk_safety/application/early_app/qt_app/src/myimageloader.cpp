/******************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Ultralite module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#include "myimageloader.h"
#include <qul/qul.h>
#include "lodepng.h"
#include <ff.h>
#include "heap.h"
#include "res_loader.h"
#include <macros.h>

const char QT_FS_PATH[] = "early_app/qt_app/";

Qul::SharedImage MyImageProvider::requestImage(const char *imageName, size_t imageNameLength)
{
    char *fpath = (char *) memalign(32,strlen(QT_FS_PATH) + imageNameLength + 4);
    memset(fpath, 0, strlen(QT_FS_PATH) + imageNameLength + 4);
    memcpy(fpath, QT_FS_PATH, strlen(QT_FS_PATH));
    memcpy(fpath+strlen(QT_FS_PATH), imageName, imageNameLength);

    printf("requestImage fpath=%s, namesize=%d\n", fpath, imageNameLength);

    int image_file_size = ROUNDUP(res_size(fpath),32);
    printf("res_size(fpath) = ",res_size(fpath));
    printf("requestImage file size %d\n", image_file_size);
    char* image_file_buf = (char *)memalign(0x1000,image_file_size);
    res_load(fpath,image_file_buf,image_file_size,0);

    //创建Image及其对应的智能指针：SharedImage
    //PNG转换为RGBA，使用loadPNG库API
    //输出到：sharedImage.image()->bits()

    /*Decode the PNG image*/
    uint32_t png_width;             /*Will be the width of the decoded image*/
    uint32_t png_height;            /*Will be the width of the decoded image*/
    uint8_t * img_data = NULL;
    uint32_t error;

    /*Decode the loaded image in ARGB8888 */
    error = lodepng_decode32(&img_data, &png_width, &png_height, (const unsigned char*)image_file_buf, image_file_size);
    if (png_width == 0 || png_height == 0 || img_data == NULL) {
        printf("requestImage err %d,%d,%p\n",png_width, png_height,img_data);
    }

    free(fpath);
    free(image_file_buf);

    if(error) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
        return {};
    }

    Qul::Image image(img_data, png_width, png_height, Qul::PixelFormat_ARGB32);

    Qul::SharedImage sharedImage(image);

    //在Image调用endWrite()完成前，什么都不显示，并行结束节点
    sharedImage.image()->endWrite();

    return sharedImage;
}