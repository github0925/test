/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>
#include <uapi/uapi/err.h>
#include "v4l2.h"
#include "i2c_hal.h"
#include "n4.h"
#include <lib/console.h>

#define n4_LOG 0

unsigned int mclk = 1; //0:1242M 1:1188 2:756M 3:594M
unsigned int i2c = 1; //1:non-continue 0:continue

void *n4_i2c_handle = NULL;
u8 n4_device_address = 0;
struct n4_dev *n4_sensor;

struct n4_dev {
    struct v4l2_device vdev;
    int device_id;
    void *i2c_handle;
    u8 device_address;

    mutex_t lock;
    struct v4l2_fwnode_endpoint ep;
    bool streaming;
};
enum n4_frame_rate {
    n4_25_FPS = 0,
    n4_NUM_FRAMERATES,
};

enum n4_mode_id {
    n4_MODE_720P_1280_720,
    n4_MODE_720P_1280_2880,
    n4_NUM_MODES,
};

static const struct v4l2_fmtdesc n4_formats[] = {
    {0, V4L2_PIX_FMT_YUYV,},
};

enum n4_downsize_mode {
    SUBSAMPLING,
    SCALING,
};


struct n4_mode_info {
    enum n4_mode_id id;
    enum n4_downsize_mode dn_mode;
    u32 hact;
    u32 vact;
    //const struct reg_value *reg_data;
    //u32 reg_data_size;
};


static const struct n4_mode_info
    n4_mode_data[n4_NUM_FRAMERATES][n4_NUM_MODES] = {
    {
        {
            n4_MODE_720P_1280_720, SUBSAMPLING, 1280, 720
        },
        {
            n4_MODE_720P_1280_720, SUBSAMPLING, 1280, 2880
        },
    },
};

static inline struct n4_dev *to_n4_dev(struct v4l2_device *vdev)
{
    return containerof(vdev, struct n4_dev, vdev);
}

static int n4_i2c_write(u8 reg, u8 val)
{
    u8 buf[2];
    int ret = 0;

    buf[0] = reg;
    buf[1] = val;

    ret = hal_i2c_write_reg_data(n4_i2c_handle, n4_device_address, buf, 1, buf + 1, 1);

    if (ret < 0) {
        dprintf(CRITICAL, "%s: error: reg=0x%x, val=%x\n",
                __func__, reg, val);
        return ret;
    }

    return ret;
}

static int n4_i2c_read(u8 reg, u8 *val)
{

    u8 buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));

    ret = hal_i2c_read_reg_data(n4_i2c_handle, n4_device_address, &reg, 1, buf, 1);

    if (ret < 0) {
        dprintf(CRITICAL, "%s: error: read reg=0x%x\n", __func__, reg);
        return ret;
    }

    *val = buf[0];
    return 0;
}



static void n4_common_settings(void)
{
    dprintf(n4_LOG, "n4_common_settings ENTER\n");

    #if 1 //add for reinitial when not power off
    n4_i2c_write(0xff, 0x20);
    n4_i2c_write(0x00, 0x00);
    n4_i2c_write(0x40, 0x11);
    n4_i2c_write(0x40, 0x00);

    n4_i2c_write(0xff, 0x01);
    n4_i2c_write(0x81, 0x41);
    n4_i2c_write(0x80, 0x61);
    n4_i2c_write(0x80, 0x60);
    n4_i2c_write(0x81, 0x40);
    #endif

    n4_i2c_write(0xff, 0x04);
    n4_i2c_write(0xa0, 0x24);
    n4_i2c_write(0xa1, 0x24);
    n4_i2c_write(0xa2, 0x24);
    n4_i2c_write(0xa3, 0x24);
    n4_i2c_write(0xa4, 0x24);
    n4_i2c_write(0xa5, 0x24);
    n4_i2c_write(0xa6, 0x24);
    n4_i2c_write(0xa7, 0x24);
    n4_i2c_write(0xa8, 0x24);
    n4_i2c_write(0xa9, 0x24);
    n4_i2c_write(0xaa, 0x24);
    n4_i2c_write(0xab, 0x24);
    n4_i2c_write(0xac, 0x24);
    n4_i2c_write(0xad, 0x24);
    n4_i2c_write(0xae, 0x24);
    n4_i2c_write(0xaf, 0x24);
    n4_i2c_write(0xb0, 0x24);
    n4_i2c_write(0xb1, 0x24);
    n4_i2c_write(0xb2, 0x24);
    n4_i2c_write(0xb3, 0x24);
    n4_i2c_write(0xb4, 0x24);
    n4_i2c_write(0xb5, 0x24);
    n4_i2c_write(0xb6, 0x24);
    n4_i2c_write(0xb7, 0x24);
    n4_i2c_write(0xb8, 0x24);
    n4_i2c_write(0xb9, 0x24);
    n4_i2c_write(0xba, 0x24);
    n4_i2c_write(0xbb, 0x24);
    n4_i2c_write(0xbc, 0x24);
    n4_i2c_write(0xbd, 0x24);
    n4_i2c_write(0xbe, 0x24);
    n4_i2c_write(0xbf, 0x24);
    n4_i2c_write(0xc0, 0x24);
    n4_i2c_write(0xc1, 0x24);
    n4_i2c_write(0xc2, 0x24);
    n4_i2c_write(0xc3, 0x24);

    n4_i2c_write(0xff, 0x21);
    n4_i2c_write(0x07, 0x80);
    n4_i2c_write(0x07, 0x00);

    n4_i2c_write(0xff, 0x0A);
    n4_i2c_write(0x77, 0x8F);
    n4_i2c_write(0xF7, 0x8F);
    n4_i2c_write(0xff, 0x0B);
    n4_i2c_write(0x77, 0x8F);
    n4_i2c_write(0xF7, 0x8F);

    n4_i2c_write(0xFF, 0x01);
    n4_i2c_write(0xCC, 0x64);
    n4_i2c_write(0xCD, 0x64);
    n4_i2c_write(0xCE, 0x64);
    n4_i2c_write(0xCF, 0x64);
//  n4_i2c_write(0xC8, 0x00);
//  n4_i2c_write(0xC9, 0x00);
//  n4_i2c_write(0xCA, 0x00);
//  n4_i2c_write(0xCB, 0x00);

    n4_i2c_write(0xFF, 0x21);
    if(mclk == 3)
    {
        n4_i2c_write(0x40, 0xAC);
        n4_i2c_write(0x41, 0x10);
        n4_i2c_write(0x42, 0x03);
        n4_i2c_write(0x43, 0x43);
        n4_i2c_write(0x11, 0x04);
        n4_i2c_write(0x10, 0x0A);
        n4_i2c_write(0x12, 0x06);
        n4_i2c_write(0x13, 0x09);
        n4_i2c_write(0x17, 0x01);
        n4_i2c_write(0x18, 0x0D);
        n4_i2c_write(0x15, 0x04);
        n4_i2c_write(0x14, 0x16);
        n4_i2c_write(0x16, 0x05);
        n4_i2c_write(0x19, 0x05);
        n4_i2c_write(0x1A, 0x0A);
        n4_i2c_write(0x1B, 0x08);
        n4_i2c_write(0x1C, 0x07);
    }
    else if(mclk == 1)
    {   n4_i2c_write(0x40, 0xd8);
        n4_i2c_write(0x41, 0x00);
        n4_i2c_write(0x42, 0x04);
        n4_i2c_write(0x43, 0x43);
        n4_i2c_write(0x11, 0x08);
        n4_i2c_write(0x10, 0x12);
        n4_i2c_write(0x12, 0x0b);
        n4_i2c_write(0x13, 0x11);
        n4_i2c_write(0x17, 0x02);
        n4_i2c_write(0x18, 0x11);
        n4_i2c_write(0x15, 0x07);
        n4_i2c_write(0x14, 0x2b);
        n4_i2c_write(0x16, 0x0a);
        n4_i2c_write(0x19, 0x09);
        n4_i2c_write(0x1A, 0x14);
        n4_i2c_write(0x1B, 0x10);
        n4_i2c_write(0x1C, 0x0d);
    }
    else
    {
        n4_i2c_write(0x40, 0xB4);
        n4_i2c_write(0x41, 0x00);
        n4_i2c_write(0x42, 0x03);
        n4_i2c_write(0x43, 0x43);
        n4_i2c_write(0x11, 0x08);
        n4_i2c_write(0x10, 0x13);
        n4_i2c_write(0x12, 0x0B);
        n4_i2c_write(0x13, 0x12);
        n4_i2c_write(0x17, 0x02);
        n4_i2c_write(0x18, 0x12);
        n4_i2c_write(0x15, 0x07);
        n4_i2c_write(0x14, 0x2D);
        n4_i2c_write(0x16, 0x0B);
        n4_i2c_write(0x19, 0x09);
        n4_i2c_write(0x1A, 0x15);
        n4_i2c_write(0x1B, 0x11);
        n4_i2c_write(0x1C, 0x0E);
    }
    n4_i2c_write(0x44, 0x00);
    n4_i2c_write(0x49, 0xF3);
    n4_i2c_write(0x49, 0xF0);
    n4_i2c_write(0x44, 0x02);
    if(i2c==0)
        n4_i2c_write(0x08, 0x48); //0x40:non-continue;0x48:continuous
    else
        n4_i2c_write(0x08, 0x40); //0x40:non-continue;0x48:continuous
    n4_i2c_write(0x0F, 0x01);
    n4_i2c_write(0x38, 0x1E);
    n4_i2c_write(0x39, 0x1E);
    n4_i2c_write(0x3A, 0x1E);
    n4_i2c_write(0x3B, 0x1E);
    n4_i2c_write(0x07, 0x0F); //0x0f:4lane  0x07:2lane
    n4_i2c_write(0x2D, 0x01); //0x01:4lane  0x00:2lane
    n4_i2c_write(0x45, 0x02);

    n4_i2c_write(0xFF, 0x13);
    n4_i2c_write(0x30, 0x00);
    n4_i2c_write(0x31, 0x00);
    n4_i2c_write(0x32, 0x00);

    n4_i2c_write(0xFF, 0x09);
    n4_i2c_write(0x40, 0x00);
    n4_i2c_write(0x41, 0x00);
    n4_i2c_write(0x42, 0x00);
    n4_i2c_write(0x43, 0x00);
    n4_i2c_write(0x44, 0x00);
    n4_i2c_write(0x45, 0x00);
    n4_i2c_write(0x46, 0x00);
    n4_i2c_write(0x47, 0x00);
    n4_i2c_write(0x50, 0x30);
    n4_i2c_write(0x51, 0x6f);
    n4_i2c_write(0x52, 0x67);
    n4_i2c_write(0x53, 0x48);
    n4_i2c_write(0x54, 0x30);
    n4_i2c_write(0x55, 0x6f);
    n4_i2c_write(0x56, 0x67);
    n4_i2c_write(0x57, 0x48);
    n4_i2c_write(0x58, 0x30);
    n4_i2c_write(0x59, 0x6f);
    n4_i2c_write(0x5a, 0x67);
    n4_i2c_write(0x5b, 0x48);
    n4_i2c_write(0x5c, 0x30);
    n4_i2c_write(0x5d, 0x6f);
    n4_i2c_write(0x5e, 0x67);
    n4_i2c_write(0x5f, 0x48);
    n4_i2c_write(0x96, 0x00);
    n4_i2c_write(0x97, 0x00);
    n4_i2c_write(0x98, 0x00);
    n4_i2c_write(0x99, 0x00);
    n4_i2c_write(0x9a, 0x00);
    n4_i2c_write(0x9b, 0x00);
    n4_i2c_write(0x9c, 0x00);
    n4_i2c_write(0x9d, 0x00);
    n4_i2c_write(0x9e, 0x00);
    n4_i2c_write(0xb6, 0x00);
    n4_i2c_write(0xb7, 0x00);
    n4_i2c_write(0xb8, 0x00);
    n4_i2c_write(0xb9, 0x00);
    n4_i2c_write(0xba, 0x00);
    n4_i2c_write(0xbb, 0x00);
    n4_i2c_write(0xbc, 0x00);
    n4_i2c_write(0xbd, 0x00);
    n4_i2c_write(0xbe, 0x00);
    n4_i2c_write(0xd6, 0x00);
    n4_i2c_write(0xd7, 0x00);
    n4_i2c_write(0xd8, 0x00);
    n4_i2c_write(0xd9, 0x00);
    n4_i2c_write(0xda, 0x00);
    n4_i2c_write(0xdb, 0x00);
    n4_i2c_write(0xdc, 0x00);
    n4_i2c_write(0xdd, 0x00);
    n4_i2c_write(0xde, 0x00);
    n4_i2c_write(0xf6, 0x00);
    n4_i2c_write(0xf7, 0x00);
    n4_i2c_write(0xf8, 0x00);
    n4_i2c_write(0xf9, 0x00);
    n4_i2c_write(0xfa, 0x00);
    n4_i2c_write(0xfb, 0x00);
    n4_i2c_write(0xfc, 0x00);
    n4_i2c_write(0xfd, 0x00);
    n4_i2c_write(0xfe, 0x00);

    n4_i2c_write(0xff, 0x0a);
    n4_i2c_write(0x3d, 0x00);
    n4_i2c_write(0x3c, 0x00);
    n4_i2c_write(0x30, 0xac);
    n4_i2c_write(0x31, 0x78);
    n4_i2c_write(0x32, 0x17);
    n4_i2c_write(0x33, 0xc1);
    n4_i2c_write(0x34, 0x40);
    n4_i2c_write(0x35, 0x00);
    n4_i2c_write(0x36, 0xc3);
    n4_i2c_write(0x37, 0x0a);
    n4_i2c_write(0x38, 0x00);
    n4_i2c_write(0x39, 0x02);
    n4_i2c_write(0x3a, 0x00);
    n4_i2c_write(0x3b, 0xb2);
    n4_i2c_write(0x25, 0x10);
    n4_i2c_write(0x27, 0x1e);
    n4_i2c_write(0xbd, 0x00);
    n4_i2c_write(0xbc, 0x00);
    n4_i2c_write(0xb0, 0xac);
    n4_i2c_write(0xb1, 0x78);
    n4_i2c_write(0xb2, 0x17);
    n4_i2c_write(0xb3, 0xc1);
    n4_i2c_write(0xb4, 0x40);
    n4_i2c_write(0xb5, 0x00);
    n4_i2c_write(0xb6, 0xc3);
    n4_i2c_write(0xb7, 0x0a);
    n4_i2c_write(0xb8, 0x00);
    n4_i2c_write(0xb9, 0x02);
    n4_i2c_write(0xba, 0x00);
    n4_i2c_write(0xbb, 0xb2);
    n4_i2c_write(0xa5, 0x10);
    n4_i2c_write(0xa7, 0x1e);

    n4_i2c_write(0xff, 0x0b);
    n4_i2c_write(0x3d, 0x00);
    n4_i2c_write(0x3c, 0x00);
    n4_i2c_write(0x30, 0xac);
    n4_i2c_write(0x31, 0x78);
    n4_i2c_write(0x32, 0x17);
    n4_i2c_write(0x33, 0xc1);
    n4_i2c_write(0x34, 0x40);
    n4_i2c_write(0x35, 0x00);
    n4_i2c_write(0x36, 0xc3);
    n4_i2c_write(0x37, 0x0a);
    n4_i2c_write(0x38, 0x00);
    n4_i2c_write(0x39, 0x02);
    n4_i2c_write(0x3a, 0x00);
    n4_i2c_write(0x3b, 0xb2);
    n4_i2c_write(0x25, 0x10);
    n4_i2c_write(0x27, 0x1e);
    n4_i2c_write(0xbd, 0x00);
    n4_i2c_write(0xbc, 0x00);
    n4_i2c_write(0xb0, 0xac);
    n4_i2c_write(0xb1, 0x78);
    n4_i2c_write(0xb2, 0x17);
    n4_i2c_write(0xb3, 0xc1);
    n4_i2c_write(0xb4, 0x40);
    n4_i2c_write(0xb5, 0x00);
    n4_i2c_write(0xb6, 0xc3);
    n4_i2c_write(0xb7, 0x0a);
    n4_i2c_write(0xb8, 0x00);
    n4_i2c_write(0xb9, 0x02);
    n4_i2c_write(0xba, 0x00);
    n4_i2c_write(0xbb, 0xb2);
    n4_i2c_write(0xa5, 0x10);
    n4_i2c_write(0xa7, 0x1e);

    n4_i2c_write(0xFF, 0x21);
    n4_i2c_write(0x3E, 0x00);
    n4_i2c_write(0x3F, 0x00);
}


#define CH_VALUE_NTSC               0x00 //720*480i@cvbs-ntsc
#define CH_VALUE_PAL                0x10 //720*576i@cbvs-pal
#define CH_VALUE_HD25               0x21 //1280*720p@25
#define CH_VALUE_HD30               0x20 //1280*720p@30
#define CH_VALUE_FHD25              0x31 //1920*1080p@25
#define CH_VALUE_FHD30              0x30 //1920*1080p@30

static int ch_fhd_mode[4]={0,0,0,0};

static void n4_init_ch(unsigned char ch, unsigned char vfmt)
{
    dprintf(n4_LOG, "int ch = %d,vfmt = 0x%x\n",ch,vfmt);
    n4_i2c_write(0xFF, 0x00);
    n4_i2c_write(0x00+ch, 0x10);
    if ((CH_VALUE_HD25 == vfmt) || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25 == vfmt) || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x04+ch, 0x00);
    } else if (CH_VALUE_PAL==vfmt) {
        n4_i2c_write(0x04+ch, 0x05);
    } else if (CH_VALUE_NTSC==vfmt) {
        n4_i2c_write(0x04+ch, 0x04);
    }
    if (CH_VALUE_HD25 == vfmt) {
        n4_i2c_write(0x08+ch, 0x0D);
    } else if (CH_VALUE_HD30==vfmt) {
        n4_i2c_write(0x08+ch, 0x0C);
    } else if (CH_VALUE_FHD25 == vfmt) {
        n4_i2c_write(0x08+ch, 0x03);
    } else if (CH_VALUE_FHD30==vfmt) {
        n4_i2c_write(0x08+ch, 0x02);
    } else {
        n4_i2c_write(0x08+ch, 0x00);
    }

    n4_i2c_write(0x0c+ch, 0x00);
    if ((CH_VALUE_HD25 == vfmt) || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25 == vfmt) || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x10+ch, 0x20);
    } else if (CH_VALUE_PAL == vfmt) {
        n4_i2c_write(0x10+ch, 0xdd);
    } else if (CH_VALUE_NTSC== vfmt) {
        n4_i2c_write(0x10+ch, 0xa0);
    }
    n4_i2c_write(0x14+ch, 0x00);
    n4_i2c_write(0x18+ch, 0x11);
    n4_i2c_write(0x1c+ch, 0x1a);
    n4_i2c_write(0x20+ch, 0x00);
    if ((CH_VALUE_HD25 == vfmt) || (CH_VALUE_HD30 == vfmt)){
        n4_i2c_write(0x24+ch, 0x88);
        n4_i2c_write(0x28+ch, 0x84);
        n4_i2c_write(0x30+ch, 0x03);
        n4_i2c_write(0x34+ch, 0x0f);
    } else if ((CH_VALUE_FHD25 == vfmt) || (CH_VALUE_FHD30 == vfmt)){
        n4_i2c_write(0x24+ch, 0x86);
        n4_i2c_write(0x28+ch, 0x80);
        n4_i2c_write(0x30+ch, 0x00);
        n4_i2c_write(0x34+ch, 0x00);
    } else {
        n4_i2c_write(0x24+ch, 0x90);
        n4_i2c_write(0x28+ch, 0x90);
        n4_i2c_write(0x30+ch, 0x00);
        n4_i2c_write(0x34+ch, 0x00);
    }
    n4_i2c_write(0x40+ch, 0x00);
    n4_i2c_write(0x44+ch, 0x00);
    n4_i2c_write(0x48+ch, 0x00);
    if ((CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x4c+ch, 0xfe);
        n4_i2c_write(0x50+ch, 0xfb);
    } else {
        n4_i2c_write(0x4c+ch, 0x00);
        n4_i2c_write(0x50+ch, 0x00);
    }
    n4_i2c_write(0x58+ch, 0x80);
    n4_i2c_write(0x5c+ch, 0x82);
    n4_i2c_write(0x60+ch, 0x10);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x64+ch, 0x05);
    } else if (CH_VALUE_PAL == vfmt) {
        n4_i2c_write(0x64+ch, 0x0a);
    } else if (CH_VALUE_NTSC== vfmt) {
        n4_i2c_write(0x64+ch, 0x1c);
    }
    if ((CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x68+ch, 0x48);
    } else if (CH_VALUE_HD25 == vfmt) {
        n4_i2c_write(0x68+ch, 0x38);
    } else if (CH_VALUE_HD30== vfmt) {
        n4_i2c_write(0x68+ch, 0x3e);
    } else if (CH_VALUE_PAL == vfmt) {
        n4_i2c_write(0x68+ch, 0xd8);
    } else if (CH_VALUE_NTSC == vfmt) {
        n4_i2c_write(0x68+ch, 0xd4);
    }
    n4_i2c_write(0x6c+ch, 0x00);
    if ((CH_VALUE_NTSC== vfmt)  || (CH_VALUE_PAL == vfmt)) {
        n4_i2c_write(0x70+ch, 0x1e);
    }
    n4_i2c_write(0x78+ch, 0x21);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x7c+ch, 0x00);
    } else {
        n4_i2c_write(0x7c+ch, 0x8f);
    }
    n4_i2c_write(0xFF, 0x01);
    n4_i2c_write(0x7C, 0x00);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x84+ch, 0x04);
        n4_i2c_write(0x88+ch, 0x01);
        n4_i2c_write(0x8c+ch, 0x02);
        n4_i2c_write(0xa0+ch, 0x00);
    } else {
        n4_i2c_write(0x84+ch, 0x06);
        n4_i2c_write(0x88+ch, 0x07);
        n4_i2c_write(0x8c+ch, 0x01);
        n4_i2c_write(0xa0+ch, 0x10);
        n4_i2c_write(0xcc+ch, 0x40);
    }
    n4_i2c_write(0xEC+ch, 0x00);

    n4_i2c_write(0xFF, 0x05+ch);
    n4_i2c_write(0x00, 0xd0);
    n4_i2c_write(0x01, 0x2c);
    if (CH_VALUE_PAL == vfmt || CH_VALUE_NTSC == vfmt) {
        n4_i2c_write(0x05, 0x24); //2021-01-22
        n4_i2c_write(0x08, 0x50);
        n4_i2c_write(0x10, 0x00);
        n4_i2c_write(0x11, 0x00);
    }
    else{
        n4_i2c_write(0x05, 0x24);
        n4_i2c_write(0x08, 0x5A);
        n4_i2c_write(0x10, 0x06);
        n4_i2c_write(0x11, 0x06);
    }
    n4_i2c_write(0x1d, 0x0c);
    n4_i2c_write(0x24, 0x2a);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x25, 0xdc);
    } else if (CH_VALUE_PAL == vfmt) {
        n4_i2c_write(0x25, 0xcc);
    } else if (CH_VALUE_NTSC == vfmt) {
        n4_i2c_write(0x25, 0xdc);
    }
    n4_i2c_write(0x26, 0x40);
    n4_i2c_write(0x27, 0x57);
    n4_i2c_write(0x28, 0x80);
    n4_i2c_write(0x2b, 0xa8);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x31, 0x82);
    } else if (CH_VALUE_PAL == vfmt) {
        n4_i2c_write(0x31, 0x02);
    } else if (CH_VALUE_NTSC == vfmt) {
        n4_i2c_write(0x31, 0x82);
    }
    n4_i2c_write(0x32, 0x10);
    if ((CH_VALUE_FHD25 == vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x38, 0x13);
        n4_i2c_write(0x47, 0xEE);
        n4_i2c_write(0x50, 0xc6);
    } else if ((CH_VALUE_HD25 == vfmt)  || (CH_VALUE_HD30 == vfmt)) {
        n4_i2c_write(0x38, 0x10);
        n4_i2c_write(0x47, 0xEE);
        n4_i2c_write(0x50, 0xc6);
    } else if (CH_VALUE_PAL == vfmt) {
        n4_i2c_write(0x38, 0x1f);
        n4_i2c_write(0x47, 0x04);
        n4_i2c_write(0x50, 0x84);
    } else {
        n4_i2c_write(0x38, 0x1d);
        n4_i2c_write(0x47, 0x04);
        n4_i2c_write(0x50, 0x84);
    }
    n4_i2c_write(0x53, 0x00);
    n4_i2c_write(0x57, 0x00);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x58, 0x77);
    } else {
        n4_i2c_write(0x58, 0x70);
    }
    n4_i2c_write(0x59, 0x00);
    n4_i2c_write(0x5C, 0x78);
    n4_i2c_write(0x5F, 0x00);
    n4_i2c_write(0x62, 0x20);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x64, 0x00);
    } else {
        n4_i2c_write(0x64, 0x01);
    }
    n4_i2c_write(0x65, 0x00);
    n4_i2c_write(0x69, 0x00);
    n4_i2c_write(0x82, 0x00);  //important!!!!
    if ((CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x6E, 0x00);
        n4_i2c_write(0x6F, 0x00);
        n4_i2c_write(0x90, 0x01);
    } else if (CH_VALUE_HD30 == vfmt) {
        n4_i2c_write(0x6E, 0x10);
        n4_i2c_write(0x6F, 0x1C);
        n4_i2c_write(0x90, 0x01);
    } else if (CH_VALUE_HD25 == vfmt) {
        n4_i2c_write(0x6E, 0x00);
        n4_i2c_write(0x6F, 0x00);
        n4_i2c_write(0x90, 0x01);
    } else {
        n4_i2c_write(0x6E, 0x00);
        n4_i2c_write(0x6F, 0x00);
        if ( CH_VALUE_PAL== vfmt) {
            n4_i2c_write(0x90, 0x0d);
        } else {
            n4_i2c_write(0x90, 0x01);
        }
    }
    n4_i2c_write(0x92, 0x00);
    n4_i2c_write(0x94, 0x00);
    n4_i2c_write(0x95, 0x00);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0xa9, 0x00);
        n4_i2c_write(0xb5, 0x80);
    } else if (CH_VALUE_PAL == vfmt) {
        n4_i2c_write(0xa9, 0x0a);
        n4_i2c_write(0xb5, 0x00);
    } else {
        n4_i2c_write(0xa9, 0x1c);
        n4_i2c_write(0xb5, 0x00);
    }
    n4_i2c_write(0xb7, 0xfc);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0xb8, 0x39);
    }else{
        n4_i2c_write(0xb8, 0xb9);
    }
    n4_i2c_write(0xb9, 0x72);
    n4_i2c_write(0xbb, 0x0f);
    n4_i2c_write(0xd1, 0x30);
    n4_i2c_write(0xd5, 0x80);

    n4_i2c_write(0xFF, 0x09);
    if ((CH_VALUE_HD25== vfmt)  || (CH_VALUE_HD30 == vfmt) || (CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        n4_i2c_write(0x96+ch*0x20, 0x00);
        n4_i2c_write(0x97+ch*0x20, 0x00);
    } else {
        n4_i2c_write(0x96+ch*0x20, 0x10);
        n4_i2c_write(0x97+ch*0x20, 0x10);
    }

    if ((CH_VALUE_FHD25== vfmt)  || (CH_VALUE_FHD30 == vfmt)) {
        ch_fhd_mode[ch] = 1;
    } else {
        ch_fhd_mode[ch] = 0;
    }
}


static void n4_mipi_outputs(int mipi_ch_en)
{
    int ch;
    int ret = 0;
    unsigned char reg_value;
    dprintf(n4_LOG, "n4_mipi_outputs mipi_ch_en = 0x%x\n",mipi_ch_en);
    n4_i2c_write(0xFF, 0x20);
    ret = n4_i2c_read(0x01, &reg_value);
    if(ret < 0)
        dprintf(0, "%s : i2c read reg 0x01 failed\n", __func__);
    for(ch=0;ch<4;ch++){
        if( 1 == ch_fhd_mode[ch] ) {
            reg_value &= (~(0x03<<(ch*2)));
        } else {
            reg_value |= (0x01<<(ch*2));
        }
    }
    n4_i2c_write(0x01, reg_value); //0x55
    n4_i2c_write(0x00, 0x00);
    //n4_i2c_write(0x40, 0x01);
    n4_i2c_write(0x0F, 0x00);
    n4_i2c_write(0x0D, 0x01);  //4lane mode
    //n4_i2c_write(0x40, 0x00);
    n4_i2c_write(0x00, mipi_ch_en);  //ch1/2/3/4 enabled FF
    n4_i2c_write(0x40, 0x01);
    n4_i2c_write(0x40, 0x00);

    n4_i2c_write(0xFF, 0x01);
    n4_i2c_write(0xC8, 0x00);
    n4_i2c_write(0xC9, 0x00);
    n4_i2c_write(0xCA, 0x00);
    n4_i2c_write(0xCB, 0x00);
}

static int n4_init_config(struct v4l2_device *vdev)
{
    vdev->ex_info.pclk = 72;
    vdev->ex_info.lanes = 4;
    vdev->ex_info.hsa = 10;
    vdev->ex_info.hbp = 20;
    vdev->ex_info.hsd = 0x60;
    vdev->ex_info.sync = false;
    vdev->ex_info.vcn = 4;
    vdev->ex_info.vc = 0;

    n4_common_settings();

    n4_init_ch(0,CH_VALUE_HD25);
    n4_init_ch(1,CH_VALUE_HD25);
    n4_init_ch(2,CH_VALUE_HD25);
    n4_init_ch(3,CH_VALUE_HD25);

    n4_mipi_outputs(0xFF);

    return 0;
}

static const uint32_t i2c_res[16] = {
    [0] = RES_I2C_I2C1,
    [1] = RES_I2C_I2C2,
    [2] = RES_I2C_I2C3,
    [3] = RES_I2C_I2C4,
    [4] = RES_I2C_I2C5,
    [5] = RES_I2C_I2C6,
    [6] = RES_I2C_I2C7,
    [7] = RES_I2C_I2C8,
    [8] = RES_I2C_I2C9,
    [9] = RES_I2C_I2C10,
    [10] = RES_I2C_I2C11,
    [11] = RES_I2C_I2C12,
    [12] = RES_I2C_I2C13,
    [13] = RES_I2C_I2C14,
    [14] = RES_I2C_I2C15,
    [15] = RES_I2C_I2C16,
};

static int n4_check_chip_id(void)
{
    int ret;
    u8 chip_id = 0;

    ret = n4_i2c_read(0x1e, &chip_id);
    if (ret) {
        dprintf(CRITICAL, "%s: failed to read chip identifier\n",
            __func__);
        ret = -1;
    }
/*
    if (chip_id != n4_DEVICE_ID) {
        dprintf(CRITICAL,
            "%s: wrong chip identifier, expected 0x%x(n4), got 0x%x\n",
            __func__, n4_DEVICE_ID, chip_id);
        ret = -1;
    }
*/
    return ret;
}

/* --------------- Subdev Operations --------------- */
static int n4_s_power(struct v4l2_device *vdev, int on)
{
    return 0;
}

static int n4_g_frame_interval(struct v4l2_device *vdev,
                                    struct v4l2_fract *fi)
{
    struct n4_dev *sensor = to_n4_dev(vdev);
    mutex_acquire(&sensor->lock);
    *fi = sensor->vdev.frame_interval;
    mutex_release(&sensor->lock);

    return 0;
}

static int n4_s_frame_interval(struct v4l2_device *vdev,
                                    struct v4l2_fract frame_interval)
{
    return 0;
}

static int n4_s_stream(struct v4l2_device *vdev, int enable)
{

    struct n4_dev *sensor = to_n4_dev(vdev);
    int ret = 0;

    dprintf(n4_LOG, "%s: enable=%d\n", __func__, enable);

    mutex_acquire(&sensor->lock);

    if (enable == 1) {
        if ((sensor->streaming == 0) && (sensor->vdev.ex_info.vc)) {
            //n4_write_reg(sensor, 0x15, 0x9b);
            sensor->streaming = enable;
        }
    }
    else if (enable == 0) {
        if ((sensor->streaming) && (sensor->vdev.ex_info.vc == 0)) {
            //n4_write_reg(sensor, 0x15, 0x13);
            sensor->streaming = enable;
        }
    }

    mutex_release(&sensor->lock);
    //spin(10000);
    return ret;
}

static int n4_set_interface(struct v4l2_device *vdev,
                                 struct v4l2_fwnode_endpoint ep)
{
    struct n4_dev *sensor = to_n4_dev(vdev);
    int ret = 0;

    mutex_acquire(&sensor->lock);

    if (sensor->streaming) {
        //ret = -EBUSY;
        goto out;
    }

    if (ep.bus_type == vdev->ep.bus_type) {
        ret = 0;
        goto out;
    }

    sensor->vdev.ep = ep;

out:
    mutex_release(&sensor->lock);
    return ret;
}

static int n4_get_interface(struct v4l2_device *vdev,
                                 struct v4l2_fwnode_endpoint *ep)
{
    struct n4_dev *sensor = to_n4_dev(vdev);

    mutex_acquire(&sensor->lock);

    *ep = sensor->vdev.ep;

    mutex_release(&sensor->lock);
    dprintf(n4_LOG, "ep->bus_type=%d\n", ep->bus_type);
    return 0;
}

static int n4_enum_fmt(struct v4l2_device *vdev, struct v4l2_fmtdesc *fe)
{
    uint32_t cnt = sizeof(n4_formats) / sizeof(struct v4l2_fmtdesc);
    dprintf(n4_LOG, "%s: fe->index=%d\n", __func__, fe->index);

    if (fe->index >= cnt) {
        fe->index = 0;
        return -1;
    }

    fe->pixelformat = n4_formats[fe->index].pixelformat;
    return 0;
}

static int n4_get_fmt(struct v4l2_device *vdev, struct v4l2_mbus_framefmt *fmt)
{

    struct n4_dev *sensor = to_n4_dev(vdev);

    mutex_acquire(&sensor->lock);

    *fmt = sensor->vdev.fmt;

    mutex_release(&sensor->lock);

    return 0;
}

static int n4_set_fmt(struct v4l2_device *vdev, struct v4l2_mbus_framefmt format)
{
    int ret = 0;
    struct n4_dev *sensor = to_n4_dev(vdev);

    mutex_acquire(&sensor->lock);

    if (sensor->streaming) {
        //ret = -EBUSY;
        goto out;
    }

    sensor->vdev.fmt = format;

out:
    mutex_release(&sensor->lock);
    return ret;
}

static int n4_enum_frame_size(struct v4l2_frame_size_enum *fse)
{
    uint32_t cnt = sizeof(n4_mode_data[0]) / sizeof(
                       struct n4_mode_info);

    if (fse->index >= cnt) {
        fse->index = 0;
        return -1;
    }

    fse->min_width =
        n4_mode_data[0][fse->index].hact;
    fse->max_width = fse->min_width;
    fse->min_height =
        n4_mode_data[0][fse->index].vact;
    fse->max_height = fse->min_height;

    return 0;
}
static int n4_enum_frame_interval(
    struct v4l2_device *vdev,
    struct v4l2_frame_interval_enum *fie)
{
    return 0;

}
static int n4_s_ctrl(struct v4l2_device *vdev,
                          struct v4l2_ctrl *ctrl)
{
    return 0;
}

int n4_set_sync_mode(struct v4l2_device *vdev, bool sync)
{
    if (!vdev)
        return -1;

    vdev->ex_info.sync = sync;

    return 0;
}

int n4_vc_channel_enable(struct v4l2_device *vdev, bool en,
                              uint8_t ch)
{
    if (!vdev)
        return -1;

    if (en) {
        vdev->ex_info.vc |= ch;
    }
    else {
        vdev->ex_info.vc &= ~ch;
    }

    dprintf(n4_LOG, "%s: vc=0x%x\n", __func__, vdev->ex_info.vc);
    return 0;
}

int n4_deinit(struct v4l2_device *vdev)
{
    struct n4_dev *sensor = to_n4_dev(vdev);
    dprintf(n4_LOG, "%s: vc=0x%x\n", __func__, vdev->ex_info.vc);

    if (!vdev)
      return -1;

    if (vdev->ex_info.vc == 0) {
      hal_i2c_release_handle(sensor->i2c_handle);
      free(sensor);
    }

    return 0;
}


static const struct v4l2_dev_ops n4_vdev_ops = {
    .s_power = n4_s_power,
    .g_frame_interval = n4_g_frame_interval,
    .s_frame_interval = n4_s_frame_interval,
    .s_stream = n4_s_stream,
    .enum_format = n4_enum_fmt,
    .get_fmt = n4_get_fmt,
    .set_fmt = n4_set_fmt,
    .set_interface = n4_set_interface,
    .get_interface = n4_get_interface,
    .enum_frame_size = n4_enum_frame_size,
    .enum_frame_interval = n4_enum_frame_interval,
    //.g_volatile_ctrl = n4_g_volatile_ctrl,
    .s_ctrl = n4_s_ctrl,
    .sync_enable = n4_set_sync_mode,
    .vc_enable = n4_vc_channel_enable,
    .close = n4_deinit,
};


struct v4l2_device *n4_init(int i2c_bus, u8 addr)
{
    struct n4_dev *dev;
    void *i2c_handle;

    dprintf(0, "%s(%d, 0x%x)\n", __func__, i2c_bus, addr);

    if (i2c_bus < 1 || i2c_bus > 16) {
        printf("wrong i2c bus\n");
        return NULL;
    }
    else
        hal_i2c_creat_handle(&i2c_handle, i2c_res[i2c_bus - 1]);

    dev = malloc(sizeof(*dev));

    if (!dev)
        return NULL;

    memset(dev, 0, sizeof(*dev));

    dev->device_address = addr;
    dev->i2c_handle = i2c_handle;

    n4_i2c_handle = i2c_handle;
    n4_device_address = addr;

    if (n4_check_chip_id() == -1)
        goto err;

    mutex_init(&dev->lock);

    dev->vdev.ep.bus_type = V4L2_MBUS_CSI2;
    dev->vdev.frame_interval.numerator = 1;
    dev->vdev.frame_interval.denominator = 25;
    dev->vdev.fmt.code = V4L2_PIX_FMT_YUYV;
    dev->vdev.fmt.field = V4L2_FIELD_NONE;
    dev->vdev.fmt.colorspace = V4L2_COLORSPACE_SRGB;
    dev->vdev.fmt.field = V4L2_FIELD_NONE;
    dev->vdev.ops = n4_vdev_ops;

    if (n4_init_config(&dev->vdev) == -1)
        goto err1;

    n4_sensor = dev;
    return &dev->vdev;
err1:
    mutex_destroy(&dev->lock);
err:
    free(dev);
    return NULL;
}


