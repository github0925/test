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
#include "max9286.h"

#include <lib/console.h>


#define BIT(nr)     (1 << nr)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define MAX9286_LOG 4

//static void *i2c_handle;

#if CSI_BOARD_507
#define MAX9286_SLAVE_ID1 0x48
#define MAX9286_SLAVE_ID2 0x6a
#elif (CSI_BOARD_507_A02P || CSI_BOARD_507_A02)
#define MAX9286_SLAVE_ID1 0x2c
#define MAX9286_SLAVE_ID2 0x2c
#endif

#define MAX_SENSOR_NUM 4
#define MAX96705_SLAVE_ID 0x40

#define MAX9286_DEVICE_ID 0x40

#define LINEDEBUG 1

static int des_dbg_flag = 0;


#if 1

enum max9286_frame_rate {
    MAX9286_25_FPS = 0,
    MAX9286_NUM_FRAMERATES,
};
/*
static const int max9286_framerates[] = {
    [MAX9286_25_FPS] = 25,
};
*/
enum max9286_mode_id {
    MAX9286_MODE_720P_1280_720,
    MAX9286_MODE_720P_1280_2880,
    MAX9286_NUM_MODES,
};





enum max9286_downsize_mode {
    SUBSAMPLING,
    SCALING,
};

struct reg_value {
    u16 reg_addr;
    u8 val;
    u8 mask;
    u32 delay_ms;
};

struct max9286_mode_info {
    enum max9286_mode_id id;
    enum max9286_downsize_mode dn_mode;
    u32 hact;
    u32 vact;
    //const struct reg_value *reg_data;
    //u32 reg_data_size;
};

struct max9286_ctrls {
    bool auto_exp;
    int exposure;

    bool auto_wb;
    int blue_balance;
    int red_balance;

    bool auto_gain;
    int gain;

    int brightness;
    int light_freq;
    int saturation;
    int contrast;
    int hue;
    int test_pattern;
    int hflip;
    int vflip;
};
#endif

static const struct v4l2_fmtdesc max9286_formats[] = {
    {0, V4L2_PIX_FMT_YUYV,},
};

static const struct max9286_mode_info
    max9286_mode_data[MAX9286_NUM_FRAMERATES][MAX9286_NUM_MODES] = {
    {
        {
            MAX9286_MODE_720P_1280_720, SUBSAMPLING, 1280, 720
        },
        {
            MAX9286_MODE_720P_1280_720, SUBSAMPLING, 1280, 2880
        },
    },
};


struct max9286_dev {
    struct v4l2_device vdev;
    int device_id;
    void *i2c_handle;
    u8 device_address;

    mutex_t lock;
    struct v4l2_fwnode_endpoint ep;
#if 0
//  struct v4l2_fwnode_endpoint ep; /* the parsed DT endpoint info */
    uint32_t xclk_freq;

    bool   upside_down;

    /* lock to protect all members below */
    mutex_t lock;

    int power_count;

    struct v4l2_mbus_framefmt fmt;
    bool pending_fmt_change;

    const struct max9286_mode_info *current_mode;
    const struct max9286_mode_info *last_mode;
    enum max9286_frame_rate current_fr;
//  struct v4l2_fract frame_interval;

    struct max9286_ctrls ctrls;

    uint32_t prev_sysclk, prev_hts;
    uint32_t ae_low, ae_high, ae_target;

    bool pending_mode_change;

#endif
    bool streaming;
};

struct max9286_dev *g_sensor;


static inline struct max9286_dev *to_max9286_dev(struct v4l2_device *vdev)
{
    return containerof(vdev, struct max9286_dev, vdev);
}


static int max9286_write_reg(struct max9286_dev *sensor, u8 reg, u8 val)
{
    u8 buf[2];
    int ret = 0;

    buf[0] = reg;
    buf[1] = val;

    ret = hal_i2c_write_reg_data(sensor->i2c_handle, sensor->device_address,
                                 buf, 1, buf + 1, 1);

    /*dprintf(MAX9286_LOG, "%s(), ret=%d, buf[0]=0x%x-%x\n", __func__, ret,
            buf[0], buf[1]);*/

    if (ret < 0) {
        dprintf(CRITICAL, "%s: error: reg=0x%x, val=%x\n",
                __func__, reg, val);
        return ret;
    }

    return ret;
}

static int max9286_read_reg(struct max9286_dev *sensor, u8 reg, u8 *val)
{

    u8 buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));

    ret = hal_i2c_read_reg_data(sensor->i2c_handle, sensor->device_address,
                                &reg, 1, buf, 1);

    if (ret < 0) {
        dprintf(CRITICAL, "%s: error: read reg=0x%x\n", __func__, reg);
        return ret;
    }

    //dprintf(MAX9286_LOG, "%s(): trans reg[0]=0x%x, rece buf[0]=0x%x\n ", __func__, reg,  buf[0]);
    *val = buf[0];
    return 0;
}


static int max96705_write_reg(struct max9286_dev *sensor, int idx, u8 reg,
                              u8 val)
{
    u8 buf[2];
    int ret = 0;

    buf[0] = reg;
    buf[1] = val;


    ret = hal_i2c_write_reg_data(sensor->i2c_handle, MAX96705_SLAVE_ID + idx,
                                 buf, 1, buf + 1, 1);

    /*dprintf(MAX9286_LOG, "%s(), ret=%d, buf[0]=0x%x-%x\n", __func__, ret,
            buf[0], buf[1]);*/

    if (ret < 0) {
        dprintf(CRITICAL, "%s: error: reg=0x%x, val=%x\n",
                __func__, reg, val);
        return ret;
    }

    return ret;
}

static int max96705_read_reg(struct max9286_dev *sensor, int idx, u8 reg,
                             u8 *val)
{

    u8 buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));


    ret = hal_i2c_read_reg_data(sensor->i2c_handle, MAX96705_SLAVE_ID + idx,
                                &reg, 1, buf, 1);

    if (ret < 0) {
        dprintf(CRITICAL, "%s: error: read reg=0x%x\n", __func__, reg);
        return ret;
    }

    /*dprintf(MAX9286_LOG, "%s(): trans reg[0]=0x%x, rece buf[0]=0x%x\n ",
            __func__, reg,  buf[0]);*/
    *val = buf[0];
    return 0;
}


static int max9286_init_setup(struct v4l2_device *vdev)
{
    u8 reg, val;
    int i;
    u8 tmp;

    struct max9286_dev *sensor = to_max9286_dev(vdev);

    dprintf(0, "%s()\n", __func__);

    //csienable=0
    max9286_write_reg(sensor, 0x15, 0x13);
    thread_sleep(2);

    //him enable
    max9286_write_reg(sensor, 0x1c, 0xf4);
    thread_sleep(2);

    //yuv10 to yuv8
    //enable dbl
    max9286_write_reg(sensor, 0x12, 0xf3);
    thread_sleep(2);

    max9286_write_reg(sensor, 0x0c, 0x91);
    thread_sleep(2);

    //??? only link 0 for lock
    //max9286_write_reg(sensor, 0x0, 0xe1);
    //spin(10000);


    //fs, manual pclk
    #if CSI_BOARD_ICL02
    max9286_write_reg(sensor, 0x06, 0xa0);
    max9286_write_reg(sensor, 0x07, 0x5b);
    max9286_write_reg(sensor, 0x08, 0x32);
    #else
    max9286_write_reg(sensor, 0x06, 0x00);
    max9286_write_reg(sensor, 0x07, 0xf2);
    max9286_write_reg(sensor, 0x08, 0x2b);
    thread_sleep(2);

    //vs_dly
    //max96705_write_reg(sensor, 0, 0x44, 0x00);
    //max96705_write_reg(sensor, 0, 0x45, 0x9c);
    //max96705_write_reg(sensor, 0, 0x46, 0x80);
    //vs high
    //max96705_write_reg(sensor, 0, 0x47, 0x00);
    //max96705_write_reg(sensor, 0, 0x48, 0xb0);
    //max96705_write_reg(sensor, 0, 0x49, 0x00);
    //spin(10000);
	#endif


    vdev->ex_info.pclk = 72;
    vdev->ex_info.lanes = 4;
    vdev->ex_info.hsa = 10;
    vdev->ex_info.hbp = 20;
    vdev->ex_info.hsd = 0x60;
    vdev->ex_info.sync = false;
    vdev->ex_info.vcn = 4;
    vdev->ex_info.vc = 0;


    //only link0 for lock status
    //max9286_write_reg(sensor, 0x0, 0xe1);
    max9286_write_reg(sensor, 0x0, 0xef);
    thread_sleep(2);

    //internal fs, debug for output
    max9286_write_reg(sensor, 0x1, 0x40);
    thread_sleep(2);

    //63, 64 for csi data output
    max9286_write_reg(sensor, 0x63, 0x0);
    //spin(10000);
    max9286_write_reg(sensor, 0x64, 0x0);
    //spin(10000);

    //auto mask
    max9286_write_reg(sensor, 0x69, 0x30);
    thread_sleep(2);

    //add 0210
    max9286_write_reg(sensor, 0x19, 0x80);
    thread_sleep(2);


    reg = 0x49;

    for (i = 0; i < 20; i++) {
        val = 0;
        max9286_read_reg(sensor, reg, &val);

        if ((val & 0x0f) == 0x0f)
            break;
        else
            thread_sleep(10);
    }

    dprintf(0, "[0x%x]: link_status: reg=0x%x, val=0x%x, i=%d\n",
            sensor->device_address, reg, val, i);
    if (val==0){
        dprintf(0, "no camera modules\n");
        return -1;
    }

    val = 0;

    for (i = 1; i <= MAX_SENSOR_NUM; i++) {
        //Enable Link control channel
        val |= (0x11 << (i - 1));
        max9286_write_reg(sensor, 0x0A, val);
        thread_sleep(10);

        dprintf(MAX9286_LOG, "val=0x%x, aid=0x%x\n", val, (MAX96705_SLAVE_ID + i));
        //Set MAX9271 new address for link 0
        max96705_write_reg(sensor, 0, 0x00, (MAX96705_SLAVE_ID + i) << 1);
        thread_sleep(5);

        //enable dbl, hven
        max96705_write_reg(sensor, i, 0x7, 0x84);
        thread_sleep(5);
        tmp=0;
        max96705_read_reg(sensor, i, 0x7, &tmp);
        if(tmp!=0x84){
            dprintf(0, "error chan %d val=0x%x, tmp=0x%x\n", i, 0x7, tmp);
            max96705_write_reg(sensor, i, 0x7, 0x84);
            thread_sleep(5);
        }

#if 0
        //vs delay
        //max96705_write_reg(sensor, i, 0x44, 0x00);
        //max96705_write_reg(sensor, i, 0x45, 0x9c);
        //max96705_write_reg(sensor, i, 0x46, 0x80);
        max96705_write_reg(sensor, i, 0x43, 0x25);
        thread_sleep(10);
        //max96705_write_reg(sensor, i, 0x44, 0x06);
        //max96705_write_reg(sensor, i, 0x45, 0xd8);
        max96705_write_reg(sensor, i, 0x45, 0x01);
        thread_sleep(10);
        //max96705_write_reg(sensor, i, 0x47, 0x26);
        max96705_write_reg(sensor, i, 0x47, 0x26);
        thread_sleep(10);
#else
        //vs delay
#if CSI_BOARD_ICL02
        max96705_write_reg(sensor, i, 0x43, 0x25);
        max96705_write_reg(sensor, i, 0x45, 0x01);
        max96705_write_reg(sensor, i, 0x47, 0x29);
#else
        max96705_write_reg(sensor, i, 0x43, 0x25);
        spin(100);
        tmp=0;
        max96705_read_reg(sensor, i, 0x43, &tmp);
        if(tmp!=0x25){
            dprintf(0, "error chan %d val=0x%x, tmp=0x%x\n", i, 0x25, tmp);
            max96705_write_reg(sensor, i, 0x43, 0x25);
            spin(30);
        }
        max96705_write_reg(sensor, i, 0x45, 0x01);
        spin(30);
        tmp=0;
        max96705_read_reg(sensor, i, 0x45, &tmp);
        if(tmp!=0x01){
            dprintf(0, "error chan %d val=0x%x, tmp=0x%x\n", i, 0x01, tmp);
            max96705_write_reg(sensor, i, 0x45, 0x01);
            spin(30);
        }
        max96705_write_reg(sensor, i, 0x47, 0x26);
        spin(30);
        tmp=0;
        max96705_read_reg(sensor, i, 0x47, &tmp);
        if(tmp!=0x26){
            dprintf(0, "error chan %d val=0x%x, tmp=0x%x\n", i, 0x26, tmp);
            max96705_write_reg(sensor, i, 0x47, 0x26);
            spin(30);
        }
#endif
#endif

#if 0
        max96705_write_reg(sensor, i, 0x20, 0x07);
        max96705_write_reg(sensor, i, 0x21, 0x06);
        max96705_write_reg(sensor, i, 0x22, 0x05);
        max96705_write_reg(sensor, i, 0x23, 0x04);
        max96705_write_reg(sensor, i, 0x24, 0x03);
        max96705_write_reg(sensor, i, 0x25, 0x02);
        max96705_write_reg(sensor, i, 0x26, 0x01);
        max96705_write_reg(sensor, i, 0x27, 0x00);
        spin(10000);

        max96705_write_reg(sensor, i, 0x30, 0x17);
        max96705_write_reg(sensor, i, 0x31, 0x16);
        max96705_write_reg(sensor, i, 0x32, 0x15);
        max96705_write_reg(sensor, i, 0x33, 0x14);
        max96705_write_reg(sensor, i, 0x34, 0x13);
        max96705_write_reg(sensor, i, 0x35, 0x12);
        max96705_write_reg(sensor, i, 0x36, 0x11);
        max96705_write_reg(sensor, i, 0x37, 0x10);
        spin(10000);
#else
        int j;
        for(j=0; j<=7; j++){
            max96705_write_reg(sensor, i, 0x20+j, 0x07-j);
            spin(50);
            tmp=0;
            max96705_read_reg(sensor, i, 0x20+j, &tmp);
            if(tmp!=(0x07-j)){
                dprintf(0, "error chan %d val=0x%x, tmp=0x%x\n", i, (0x07-j), tmp);
                max96705_write_reg(sensor, i, 0x20+j, 0x07-j);
                spin(30);
            }
        }
        for(j=0; j<=7; j++){
            max96705_write_reg(sensor, i, 0x30+j, 0x17-j);
            spin(50);
            tmp=0;
            max96705_read_reg(sensor, i, 0x30+j, &tmp);
            if(tmp!=(0x17-j)){
                dprintf(0, "error chan %d val=0x%x, tmp=0x%x\n", i, (0x07-j), tmp);
                max96705_write_reg(sensor, i, 0x30+j, 0x17-j);
                spin(30);
            }
        }
#endif
    }

    return 0;
}



#if 0
static int max9286_mod_reg(struct max9286_dev *sensor, u16 reg,
                           u8 mask, u8 val)
{
    u8 readval;
    int ret;

    ret = max9286_read_reg(sensor, reg, &readval);

    if (ret)
        return ret;

    readval &= ~mask;
    val &= mask;
    val |= readval;

    return max9286_write_reg(sensor, reg, val);
}



static int max9286_get_sysclk(struct max9286_dev *sensor)
{
    /* calculate sysclk */
    u32 xvclk = sensor->xclk_freq / 10000;
    u32 multiplier, prediv, VCO, sysdiv, pll_rdiv;
    u32 sclk_rdiv_map[] = {1, 2, 4, 8};
    u32 bit_div2x = 1, sclk_rdiv, sysclk;
    u8 temp1, temp2;
    int ret;

    ret = max9286_read_reg(sensor, OV5640_REG_SC_PLL_CTRL0, &temp1);

    if (ret)
        return ret;

    temp2 = temp1 & 0x0f;

    if (temp2 == 8 || temp2 == 10)
        bit_div2x = temp2 / 2;

    ret = max9286_read_reg(sensor, OV5640_REG_SC_PLL_CTRL1, &temp1);

    if (ret)
        return ret;

    sysdiv = temp1 >> 4;

    if (sysdiv == 0)
        sysdiv = 16;

    ret = max9286_read_reg(sensor, OV5640_REG_SC_PLL_CTRL2, &temp1);

    if (ret)
        return ret;

    multiplier = temp1;

    ret = max9286_read_reg(sensor, OV5640_REG_SC_PLL_CTRL3, &temp1);

    if (ret)
        return ret;

    prediv = temp1 & 0x0f;
    pll_rdiv = ((temp1 >> 4) & 0x01) + 1;

    ret = max9286_read_reg(sensor, OV5640_REG_SYS_ROOT_DIVIDER, &temp1);

    if (ret)
        return ret;

    temp2 = temp1 & 0x03;

    sclk_rdiv = sclk_rdiv_map[temp2];

    if (!prediv || !sysdiv || !pll_rdiv || !bit_div2x)
        return -EINVAL;

    VCO = xvclk * multiplier / prediv;

    sysclk = VCO / sysdiv / pll_rdiv * 2 / bit_div2x / sclk_rdiv;

    return sysclk;
}

static int max9286_set_night_mode(struct max9286_dev *sensor)
{
    /* read HTS from register settings */
    u8 mode;
    int ret;

    ret = max9286_read_reg(sensor, OV5640_REG_AEC_CTRL00, &mode);

    if (ret)
        return ret;

    mode &= 0xfb;
    return max9286_write_reg(sensor, OV5640_REG_AEC_CTRL00, mode);
}

static int max9286_get_hts(struct max9286_dev *sensor)
{
    /* read HTS from register settings */
    u16 hts;
    int ret;

    ret = max9286_read_reg16(sensor, OV5640_REG_TIMING_HTS, &hts);

    if (ret)
        return ret;

    return hts;
}

static int max9286_get_vts(struct max9286_dev *sensor)
{
    u16 vts;
    int ret;

    ret = max9286_read_reg16(sensor, OV5640_REG_TIMING_VTS, &vts);

    if (ret)
        return ret;

    return vts;
}

static int max9286_set_vts(struct max9286_dev *sensor, int vts)
{
    return max9286_write_reg16(sensor, OV5640_REG_TIMING_VTS, vts);
}



static int max9286_set_virtual_channel(struct max9286_dev *sensor)
{
    u8 temp, channel = virtual_channel;
    int ret;

    if (channel > 3) {
        dprintf(INFO,
                "%s: wrong virtual_channel parameter, expected (0..3), got %d\n",
                __func__, channel);
        return -EINVAL;
    }

    ret = max9286_read_reg(sensor, OV5640_REG_DEBUG_MODE, &temp);

    if (ret)
        return ret;

    temp &= ~(3 << 6);
    temp |= (channel << 6);
    return max9286_write_reg(sensor, OV5640_REG_DEBUG_MODE, temp);
}

static const struct max9286_mode_info *
max9286_find_mode(struct max9286_dev *sensor, enum max9286_frame_rate fr,
                  unsigned int width, unsigned int height, bool nearest)
{
    const struct max9286_mode_info *mode;

    mode = v4l2_find_nearest_size(max9286_mode_data[fr],
                                  ARRAY_SIZE(max9286_mode_data[fr]),
                                  hact, vact,
                                  width, height);

    if (!mode ||
            (!nearest && (mode->hact != width || mode->vact != height)))
        return NULL;

    return mode;
}

/*
 * sensor changes between scaling and subsampling, go through
 * exposure calculation
 */
static int max9286_set_mode_exposure_calc(struct max9286_dev *sensor,
        const struct max9286_mode_info *mode)
{
    u32 prev_shutter, prev_gain16;
    u32 cap_shutter, cap_gain16;
    u32 cap_sysclk, cap_hts, cap_vts;
    u32 light_freq, cap_bandfilt, cap_maxband;
    u32 cap_gain16_shutter;
    u8 average;
    int ret;

    if (!mode->reg_data)
        return -EINVAL;

    /* read preview shutter */
    ret = max9286_get_exposure(sensor);

    if (ret < 0)
        return ret;

    prev_shutter = ret;
    ret = max9286_get_binning(sensor);

    if (ret < 0)
        return ret;

    if (ret && mode->id != OV5640_MODE_720P_1280_720 &&
            mode->id != OV5640_MODE_1080P_1920_1080)
        prev_shutter *= 2;

    /* read preview gain */
    ret = max9286_get_gain(sensor);

    if (ret < 0)
        return ret;

    prev_gain16 = ret;

    /* get average */
    ret = max9286_read_reg(sensor, OV5640_REG_AVG_READOUT, &average);

    if (ret)
        return ret;

    /* turn off night mode for capture */
    ret = max9286_set_night_mode(sensor);

    if (ret < 0)
        return ret;

    /* Write capture setting */
    ret = max9286_load_regs(sensor, mode);

    if (ret < 0)
        return ret;

    /* read capture VTS */
    ret = max9286_get_vts(sensor);

    if (ret < 0)
        return ret;

    cap_vts = ret;
    ret = max9286_get_hts(sensor);

    if (ret < 0)
        return ret;

    if (ret == 0)
        return -EINVAL;

    cap_hts = ret;

    ret = max9286_get_sysclk(sensor);

    if (ret < 0)
        return ret;

    if (ret == 0)
        return -EINVAL;

    cap_sysclk = ret;

    /* calculate capture banding filter */
    ret = max9286_get_light_freq(sensor);

    if (ret < 0)
        return ret;

    light_freq = ret;

    if (light_freq == 60) {
        /* 60Hz */
        cap_bandfilt = cap_sysclk * 100 / cap_hts * 100 / 120;
    }
    else {
        /* 50Hz */
        cap_bandfilt = cap_sysclk * 100 / cap_hts;
    }

    if (!sensor->prev_sysclk) {
        ret = max9286_get_sysclk(sensor);

        if (ret < 0)
            return ret;

        if (ret == 0)
            return -EINVAL;

        sensor->prev_sysclk = ret;
    }

    if (!cap_bandfilt)
        return -EINVAL;

    cap_maxband = (int)((cap_vts - 4) / cap_bandfilt);

    /* calculate capture shutter/gain16 */
    if (average > sensor->ae_low && average < sensor->ae_high) {
        /* in stable range */
        cap_gain16_shutter =
            prev_gain16 * prev_shutter *
            cap_sysclk / sensor->prev_sysclk *
            sensor->prev_hts / cap_hts *
            sensor->ae_target / average;
    }
    else {
        cap_gain16_shutter =
            prev_gain16 * prev_shutter *
            cap_sysclk / sensor->prev_sysclk *
            sensor->prev_hts / cap_hts;
    }

    /* gain to shutter */
    if (cap_gain16_shutter < (cap_bandfilt * 16)) {
        /* shutter < 1/100 */
        cap_shutter = cap_gain16_shutter / 16;

        if (cap_shutter < 1)
            cap_shutter = 1;

        cap_gain16 = cap_gain16_shutter / cap_shutter;

        if (cap_gain16 < 16)
            cap_gain16 = 16;
    }
    else {
        if (cap_gain16_shutter > (cap_bandfilt * cap_maxband * 16)) {
            /* exposure reach max */
            cap_shutter = cap_bandfilt * cap_maxband;

            if (!cap_shutter)
                return -EINVAL;

            cap_gain16 = cap_gain16_shutter / cap_shutter;
        }
        else {
            /* 1/100 < (cap_shutter = n/100) =< max */
            cap_shutter =
                ((int)(cap_gain16_shutter / 16 / cap_bandfilt))
                * cap_bandfilt;

            if (!cap_shutter)
                return -EINVAL;

            cap_gain16 = cap_gain16_shutter / cap_shutter;
        }
    }

    /* set capture gain */
    ret = max9286_set_gain(sensor, cap_gain16);

    if (ret)
        return ret;

    /* write capture shutter */
    if (cap_shutter > (cap_vts - 4)) {
        cap_vts = cap_shutter + 4;
        ret = max9286_set_vts(sensor, cap_vts);

        if (ret < 0)
            return ret;
    }

    /* set exposure */
    return max9286_set_exposure(sensor, cap_shutter);
}

/*
 * if sensor changes inside scaling or subsampling
 * change mode directly
 */
static int max9286_set_mode_direct(struct max9286_dev *sensor,
                                   const struct max9286_mode_info *mode)
{
    if (!mode->reg_data)
        return -EINVAL;

    /* Write capture setting */
    return max9286_load_regs(sensor, mode);
}

static int max9286_set_mode(struct max9286_dev *sensor)
{
    const struct max9286_mode_info *mode = sensor->current_mode;
    const struct max9286_mode_info *orig_mode = sensor->last_mode;
    enum max9286_downsize_mode dn_mode, orig_dn_mode;
    bool auto_gain = sensor->ctrls.auto_gain == 1;
    bool auto_exp =  sensor->ctrls.auto_exp == 1;
    int ret;

    dn_mode = mode->dn_mode;
    orig_dn_mode = orig_mode->dn_mode;

    /* auto gain and exposure must be turned off when changing modes */
    if (auto_gain) {
        ret = max9286_set_autogain(sensor, false);

        if (ret)
            return ret;
    }

    if (auto_exp) {
        ret = max9286_set_autoexposure(sensor, false);

        if (ret)
            goto restore_auto_gain;
    }

    if ((dn_mode == SUBSAMPLING && orig_dn_mode == SCALING) ||
            (dn_mode == SCALING && orig_dn_mode == SUBSAMPLING)) {
        /*
         * change between subsampling and scaling
         * go through exposure calculation
         */
        ret = max9286_set_mode_exposure_calc(sensor, mode);
    }
    else {
        /*
         * change inside subsampling or scaling
         * download firmware directly
         */
        ret = max9286_set_mode_direct(sensor, mode);
    }

    if (ret < 0)
        goto restore_auto_exp_gain;

    /* restore auto gain and exposure */
    if (auto_gain)
        max9286_set_autogain(sensor, true);

    if (auto_exp)
        max9286_set_autoexposure(sensor, true);

    ret = max9286_set_binning(sensor, dn_mode != SCALING);

    if (ret < 0)
        return ret;

    ret = max9286_set_ae_target(sensor, sensor->ae_target);

    if (ret < 0)
        return ret;

    ret = max9286_get_light_freq(sensor);

    if (ret < 0)
        return ret;

    ret = max9286_set_bandingfilter(sensor);

    if (ret < 0)
        return ret;

    ret = max9286_set_virtual_channel(sensor);

    if (ret < 0)
        return ret;

    sensor->pending_mode_change = false;
    sensor->last_mode = mode;

    return 0;

restore_auto_exp_gain:

    if (auto_exp)
        max9286_set_autoexposure(sensor, true);

restore_auto_gain:

    if (auto_gain)
        max9286_set_autogain(sensor, true);

    return ret;
}



static int max9286_set_power_on(struct max9286_dev *sensor)
{
    int ret;

    ret = max9286_init_slave_id(sensor);

    if (ret)
        goto power_off;

    return 0;

power_off:
    return ret;
}

static void max9286_set_power_off(struct max9286_dev *sensor)
{
    return;
}

static int max9286_set_power(struct max9286_dev *sensor, bool on)
{
    int ret = 0;

    if (on) {
        ret = max9286_set_power_on(sensor);

        if (ret)
            return ret;

        ret = max9286_restore_mode(sensor);

        if (ret)
            goto power_off;

        /* We're done here for DVP bus, while CSI-2 needs setup. */
        if (sensor->vdev.ep.bus_type != V4L2_MBUS_CSI2)
            return 0;

        /*
         * Power up MIPI HS Tx and LS Rx; 2 data lanes mode
         *
         * 0x300e = 0x40
         * [7:5] = 010  : 2 data lanes mode (see FIXME note in
         *        "max9286_set_stream_mipi()")
         * [4] = 0  : Power up MIPI HS Tx
         * [3] = 0  : Power up MIPI LS Rx
         * [2] = 0  : MIPI interface disabled
         */
        ret = max9286_write_reg(sensor,
                                OV5640_REG_IO_MIPI_CTRL00, 0x40);

        if (ret)
            goto power_off;

        /*
         * Gate clock and set LP11 in 'no packets mode' (idle)
         *
         * 0x4800 = 0x24
         * [5] = 1  : Gate clock when 'no packets'
         * [2] = 1  : MIPI bus in LP11 when 'no packets'
         */
        ret = max9286_write_reg(sensor,
                                OV5640_REG_MIPI_CTRL00, 0x24);

        if (ret)
            goto power_off;

        /*
         * Set data lanes and clock in LP11 when 'sleeping'
         *
         * 0x3019 = 0x70
         * [6] = 1  : MIPI data lane 2 in LP11 when 'sleeping'
         * [5] = 1  : MIPI data lane 1 in LP11 when 'sleeping'
         * [4] = 1  : MIPI clock lane in LP11 when 'sleeping'
         */
        ret = max9286_write_reg(sensor,
                                OV5640_REG_PAD_OUTPUT00, 0x70);

        if (ret)
            goto power_off;

        /* Give lanes some time to coax into LP11 state. */
        udelay(500);

    }
    else {
        if (sensor->vdev.ep.bus_type == V4L2_MBUS_CSI2) {
            /* Reset MIPI bus settings to their default values. */
            max9286_write_reg(sensor,
                              OV5640_REG_IO_MIPI_CTRL00, 0x58);
            max9286_write_reg(sensor,
                              OV5640_REG_MIPI_CTRL00, 0x04);
            max9286_write_reg(sensor,
                              OV5640_REG_PAD_OUTPUT00, 0x00);
        }

        max9286_set_power_off(sensor);
    }

    return 0;

power_off:
    max9286_set_power_off(sensor);
    return ret;
}
#endif

/* --------------- Subdev Operations --------------- */

static int max9286_s_power(struct v4l2_device *vdev, int on)
{
#if 0
    int ret = 0;
    struct max9286_dev *sensor = to_max9286_dev(vdev);

    mutex_acquire(&sensor->lock);

    /*
     * If the power count is modified from 0 to != 0 or from != 0 to 0,
     * update the power state.
     */
    if (sensor->power_count == !on) {
        ret = max9286_set_power(sensor, !!on);

        if (ret)
            goto out;
    }

    /* Update the power count. */
    sensor->power_count += on ? 1 : -1;
out:
    mutex_release(&sensor->lock);

    return ret;
#endif
    return 0;
}
#if 0
static int max9286_try_frame_interval(struct max9286_dev *sensor,
                                      struct v4l2_fract *fi,
                                      u32 width, u32 height)
{
    const struct max9286_mode_info *mode;
    u32 minfps, maxfps, fps;
    int ret;

    minfps = max9286_framerates[OV5640_15_FPS];
    maxfps = max9286_framerates[OV5640_30_FPS];

    if (fi->numerator == 0) {
        fi->denominator = maxfps;
        fi->numerator = 1;
        return OV5640_30_FPS;
    }

    fps = DIV_ROUND_UP(fi->denominator, fi->numerator);

    fi->numerator = 1;

    if (fps > maxfps)
        fi->denominator = maxfps;
    else if (fps < minfps)
        fi->denominator = minfps;
    else if (2 * fps >= 2 * minfps + (maxfps - minfps))
        fi->denominator = maxfps;
    else
        fi->denominator = minfps;

    ret = (fi->denominator == minfps) ? OV5640_15_FPS : OV5640_30_FPS;

    mode = max9286_find_mode(sensor, ret, width, height, false);
    return mode ? ret : -EINVAL;
}
#endif

static int max9286_enum_fmt(struct v4l2_device *vdev,
                            struct v4l2_fmtdesc *fe)
{
    //struct max9286_dev *sensor = to_max9286_dev(vdev);
    uint32_t cnt = sizeof(max9286_formats) / sizeof(struct v4l2_fmtdesc);
    dprintf(MAX9286_LOG, "%s: fe->index=%d\n", __func__, fe->index);

    if (fe->index >= cnt) {
        fe->index = 0;
        return -1;
    }

    fe->pixelformat = max9286_formats[fe->index].pixelformat;
    return 0;
}
static int max9286_get_fmt(struct v4l2_device *vdev,
                           struct v4l2_mbus_framefmt *fmt)
{

    struct max9286_dev *sensor = to_max9286_dev(vdev);

    mutex_acquire(&sensor->lock);

    *fmt = sensor->vdev.fmt;

    mutex_release(&sensor->lock);

    return 0;
}

#if 0
static int max9286_try_fmt_internal(struct max9286_dev *sensor,
                                    struct v4l2_mbus_framefmt *fmt,
                                    enum max9286_frame_rate fr,
                                    const struct max9286_mode_info **new_mode)
{
    const struct max9286_mode_info *mode;
    unsigned int i;

    mode = max9286_find_mode(sensor, fr, fmt->width, fmt->height, true);

    if (!mode)
        return -EINVAL;

    fmt->width = mode->hact;
    fmt->height = mode->vact;

    if (new_mode)
        *new_mode = mode;

    for (i = 0; i < ARRAY_SIZE(max9286_formats); i++)
        if (max9286_formats[i].code == fmt->code)
            break;

    if (i >= ARRAY_SIZE(max9286_formats)) {
        i = 0;
        printf("%s(): try fr or fmt err\n", __func__);
        return -1;
    }

    fmt->code = max9286_formats[i].code;
    fmt->colorspace = max9286_formats[i].colorspace;

    return 0;
}
#endif

static int max9286_set_fmt(struct v4l2_device *vdev,
                           struct v4l2_mbus_framefmt format)
{
    int ret = 0;
    struct max9286_dev *sensor = to_max9286_dev(vdev);

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
#if 0
static int max9286_set_framefmt(struct max9286_dev *sensor,
                                struct v4l2_mbus_framefmt *format)
{
    int ret = 0;
    bool is_rgb = false;
    bool is_jpeg = false;
    u8 val;

    switch (format->code) {
        case V4L2_PIX_FMT_YUV420SP:
            val = 0x40;
            break;

        case V4L2_PIX_FMT_YUV420XP:
            val = 0xFE;
            break;

        case V4L2_PIX_FMT_UYVY:
        case V4L2_PIX_FMT_UYVYSP:
            /* YUV422, UYVY */
            val = 0x3f;
            break;

        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_YUYVSP:
            /* YUV422, YUYV */
            val = 0x30;
            break;

        case V4L2_PIX_FMT_YUV444: /* Not work */
            val = 0xFA;
            break;

        case V4L2_PIX_FMT_RGB565:
            /* RGB565 {g[2:0],b[4:0]},{r[4:0],g[5:3]} */
            val = 0x6F;
            is_rgb = true;
            break;

        case V4L2_PIX_FMT_RGB565X:
            /* RGB565 {r[4:0],g[5:3]},{g[2:0],b[4:0]} */
            val = 0x61;
            is_rgb = true;
            break;

        case V4L2_PIX_FMT_JPEG:
            /* YUV422, YUYV */
            val = 0x30;
            is_jpeg = true;
            break;

        default:
            return -EINVAL;
    }

    /* FORMAT CONTROL00: YUV and RGB formatting */
    ret = max9286_write_reg(sensor, OV5640_REG_FORMAT_CONTROL00, val);

    if (ret)
        return ret;

    /* FORMAT MUX CONTROL: ISP YUV or RGB */
    ret = max9286_write_reg(sensor, OV5640_REG_ISP_FORMAT_MUX_CTRL,
                            is_rgb ? 0x01 : 0x00);

    if (ret)
        return ret;

    /*
     * TIMING TC REG21:
     * - [5]:   JPEG enable
     */
    ret = max9286_mod_reg(sensor, OV5640_REG_TIMING_TC_REG21,
                          BIT(5), is_jpeg ? BIT(5) : 0);

    if (ret)
        return ret;

    /*
     * SYSTEM RESET02:
     * - [4]:   Reset JFIFO
     * - [3]:   Reset SFIFO
     * - [2]:   Reset JPEG
     */
    ret = max9286_mod_reg(sensor, OV5640_REG_SYS_RESET02,
                          BIT(4) | BIT(3) | BIT(2),
                          is_jpeg ? 0 : (BIT(4) | BIT(3) | BIT(2)));

    if (ret)
        return ret;

    /*
     * CLOCK ENABLE02:
     * - [5]:   Enable JPEG 2x clock
     * - [3]:   Enable JPEG clock
     */
    return max9286_mod_reg(sensor, OV5640_REG_SYS_CLOCK_ENABLE02,
                           BIT(5) | BIT(3),
                           is_jpeg ? (BIT(5) | BIT(3)) : 0);
}


#endif
static int max9286_s_ctrl(struct v4l2_device *vdev,
                          struct v4l2_ctrl *ctrl)
{
#if 0
    struct max9286_dev *sensor = to_max9286_dev(vdev);
    int ret;

    /* v4l2_ctrl_lock() locks our own mutex */

    /*
     * If the device is not powered up by the host driver do
     * not apply any controls to H/W at this time. Instead
     * the controls will be restored right after power-up.
     */
    if (sensor->power_count == 0)
        return 0;

    switch (ctrl->id) {
        case V4L2_CID_AUTOGAIN:
            ret = max9286_set_ctrl_gain(sensor, ctrl->val);
            break;

        case V4L2_CID_EXPOSURE_AUTO:
            ret = max9286_set_ctrl_exposure(sensor, ctrl->val);
            break;

        case V4L2_CID_AUTO_WHITE_BALANCE:
            ret = max9286_set_ctrl_white_balance(sensor, ctrl->val);
            break;

        case V4L2_CID_HUE:
            ret = max9286_set_ctrl_hue(sensor, ctrl->val);
            break;

        case V4L2_CID_CONTRAST:
            ret = max9286_set_ctrl_contrast(sensor, ctrl->val);
            break;

        case V4L2_CID_SATURATION:
            ret = max9286_set_ctrl_saturation(sensor, ctrl->val);
            break;

        case V4L2_CID_TEST_PATTERN:
            ret = max9286_set_ctrl_test_pattern(sensor, ctrl->val);
            break;

        case V4L2_CID_POWER_LINE_FREQUENCY:
            ret = max9286_set_ctrl_light_freq(sensor, ctrl->val);
            break;

        case V4L2_CID_HFLIP:
            ret = max9286_set_ctrl_hflip(sensor, ctrl->val);
            break;

        case V4L2_CID_VFLIP:
            ret = max9286_set_ctrl_vflip(sensor, ctrl->val);
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
#endif
    return 0;
}
#if 0
static int max9286_init_controls(struct max9286_dev *sensor)
{
    struct max9286_ctrls *ctrls = &sensor->ctrls;
    int ret;

    /* Auto/manual white balance */
    ctrls->auto_wb = 1;
    ctrls->blue_balance = 0;
    ctrls->red_balance = 0;
    /* Auto/manual exposure */
    ctrls->auto_exp = 1;
    ctrls->exposure = 0;
    /* Auto/manual gain */
    ctrls->auto_gain = 1;
    ctrls->gain = 0;

    ctrls->saturation = 64;
    ctrls->hue = 0;
    ctrls->contrast = 0;
    ctrls->test_pattern = 0;
    ctrls->hflip = 0;
    ctrls->vflip = 0;

    ctrls->light_freq = 50;

    return 0;

free_ctrls:
    return ret;
}
#endif
static int max9286_enum_frame_size(struct v4l2_frame_size_enum *fse)
{
    uint32_t cnt = sizeof(max9286_mode_data[0]) / sizeof(
                       struct max9286_mode_info);

    if (fse->index >= cnt) {
        fse->index = 0;
        return -1;
    }

    fse->min_width =
        max9286_mode_data[0][fse->index].hact;
    fse->max_width = fse->min_width;
    fse->min_height =
        max9286_mode_data[0][fse->index].vact;
    fse->max_height = fse->min_height;

    return 0;
}

static int max9286_enum_frame_interval(
    struct v4l2_device *vdev,
    struct v4l2_frame_interval_enum *fie)
{
#if 0
    struct max9286_dev *sensor = to_max9286_dev(vdev);
    struct v4l2_fract tpf;
    int ret;

    tpf.numerator = 1;
    tpf.denominator = max9286_framerates[fie->index];

    ret = max9286_try_frame_interval(sensor, &tpf,
                                     fie->width, fie->height);

    if (ret < 0)
        return -EINVAL;

    fie->interval = tpf;
#endif

    return 0;

}

static int max9286_g_frame_interval(struct v4l2_device *vdev,
                                    struct v4l2_fract *fi)
{
    struct max9286_dev *sensor = to_max9286_dev(vdev);
    mutex_acquire(&sensor->lock);
    *fi = sensor->vdev.frame_interval;
    mutex_release(&sensor->lock);

    return 0;
}

static int max9286_s_frame_interval(struct v4l2_device *vdev,
                                    struct v4l2_fract frame_interval)
{
    return 0;
}

static int max9286_s_stream(struct v4l2_device *vdev, int enable)
{

    struct max9286_dev *sensor = to_max9286_dev(vdev);
    int ret = 0;

    dprintf(MAX9286_LOG, "%s: enable=%d\n", __func__, enable);

    mutex_acquire(&sensor->lock);

    if (enable == 1) {
        if ((sensor->streaming == 0) && (sensor->vdev.ex_info.vc)) {
            max9286_write_reg(sensor, 0x15, 0x9b);
            sensor->streaming = enable;
        }
    }
    else if (enable == 0) {
        if ((sensor->streaming) && (sensor->vdev.ex_info.vc == 0)) {
            max9286_write_reg(sensor, 0x15, 0x13);
            sensor->streaming = enable;
        }
    }

    mutex_release(&sensor->lock);
    //spin(10000);
    return ret;
}

static int max9286_set_interface(struct v4l2_device *vdev,
                                 struct v4l2_fwnode_endpoint ep)
{
    struct max9286_dev *sensor = to_max9286_dev(vdev);
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

static int max9286_get_interface(struct v4l2_device *vdev,
                                 struct v4l2_fwnode_endpoint *ep)
{
    struct max9286_dev *sensor = to_max9286_dev(vdev);

    mutex_acquire(&sensor->lock);

    *ep = sensor->vdev.ep;

    mutex_release(&sensor->lock);
    dprintf(MAX9286_LOG, "ep->bus_type=%d\n", ep->bus_type);
    return 0;
}


static const struct v4l2_dev_ops max9286_vdev_ops = {
    .s_power = max9286_s_power,
    .g_frame_interval = max9286_g_frame_interval,
    .s_frame_interval = max9286_s_frame_interval,
    .s_stream = max9286_s_stream,
    .enum_format = max9286_enum_fmt,
    .get_fmt = max9286_get_fmt,
    .set_fmt = max9286_set_fmt,
    .set_interface = max9286_set_interface,
    .get_interface = max9286_get_interface,
    .enum_frame_size = max9286_enum_frame_size,
    .enum_frame_interval = max9286_enum_frame_interval,
    //.g_volatile_ctrl = max9286_g_volatile_ctrl,
    .s_ctrl = max9286_s_ctrl,
    .sync_enable = max9286_set_sync_mode,
    .vc_enable = max9286_vc_channel_enable,
    .close = max9286_deinit,
};


static int max9286_dump_register(struct max9286_dev *sensor)
{
    int i, ip;
    u8 val;

    dprintf(0, "%s: start\n", __func__);

    for (i = 0; i < 0x72; i++) {
        max9286_read_reg(sensor, i, &val);
        dprintf(0, "MAX9286 Reg 0x%02x = 0x%x.\n", i, val);
    }

    dprintf(0, "\n%s: dump_max96705:\n", __func__);

    for (ip = 1; ip <= MAX_SENSOR_NUM; ip++) {
        dprintf(0, "\n%s: dump_max96705-%d:\n", __func__, ip);

        for (i = 0; i < 0xff; i++) {
            max96705_read_reg(sensor, ip, i, &val);
            dprintf(0, "MAX96705 Reg 0x%02x = 0x%x.\n", i, val);
        }
    }

    dprintf(0, "%s: end\n", __func__);
    return 0;
}

static int max9286_check_chip_id(struct max9286_dev *sensor){
    int ret;
    u8 chip_id = 0;

    ret = max9286_read_reg(sensor, 0x1e, &chip_id);
    if (ret) {
        dprintf(CRITICAL, "%s: failed to read chip identifier\n",
            __func__);
        ret = -1;
    }

    if (chip_id != MAX9286_DEVICE_ID) {
        dprintf(CRITICAL,
            "%s: wrong chip identifier, expected 0x%x(max9286), got 0x%x\n",
            __func__, MAX9286_DEVICE_ID, chip_id);
        ret = -1;
    }

    return ret;
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

struct v4l2_device *max9286_init(int i2c_bus, u8 addr)
{
    struct max9286_dev *dev;
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

    if (max9286_check_chip_id(dev) == -1)
        goto err;

    mutex_init(&dev->lock);

    dev->vdev.ep.bus_type = V4L2_MBUS_CSI2;
    dev->vdev.frame_interval.numerator = 1;
    dev->vdev.frame_interval.denominator = 25;
    dev->vdev.fmt.code = V4L2_PIX_FMT_YUYV;
    dev->vdev.fmt.field = V4L2_FIELD_NONE;
    dev->vdev.fmt.colorspace = V4L2_COLORSPACE_SRGB;
    dev->vdev.fmt.field = V4L2_FIELD_NONE;
    dev->vdev.ops = max9286_vdev_ops;

    if (max9286_init_setup(&dev->vdev) == -1)
        goto err1;

    g_sensor = dev;
    return &dev->vdev;
err1:
    mutex_destroy(&dev->lock);
err:
    free(dev);
    return NULL;
}

int max9286_set_sync_mode(struct v4l2_device *vdev, bool sync)
{
    if (!vdev)
        return -1;

    vdev->ex_info.sync = sync;

    return 0;
}
int max9286_vc_channel_enable(struct v4l2_device *vdev, bool en,
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

    dprintf(MAX9286_LOG, "%s: vc=0x%x\n", __func__, vdev->ex_info.vc);
    return 0;
}
int max9286_deinit(struct v4l2_device *vdev)
{
    struct max9286_dev *sensor = to_max9286_dev(vdev);
    dprintf(MAX9286_LOG, "%s: vc=0x%x\n", __func__, vdev->ex_info.vc);

    if (!vdev)
        return -1;

    if (vdev->ex_info.vc == 0) {
        hal_i2c_release_handle(sensor->i2c_handle);
        free(sensor);
    }

    return 0;
}

static int des_dbg_en(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        dprintf( 0, "gdx_dbg_en [0/1]\n");
        return 0;
    }

    des_dbg_flag = atoui(argv[1].str);

    return 0;
}
int max9286_dump_all(int argc, const cmd_args *argv)
{
    if(!g_sensor)
        return 0;
    max9286_dump_register(g_sensor);

    return 0;
}
int max9286_write(int argc, const cmd_args *argv)
{
    int reg;
    u8 val;
    reg = atoui(argv[1].str);
    val = atoui(argv[2].str);
    dprintf(0, "%s: id=%d, val=0x%x\n", __func__, reg, val);
    max9286_write_reg(g_sensor, reg, val);
    return 0;
}
int max96705_write(int argc, const cmd_args *argv){
	int ip, reg;
    u8 val;

    if (argc < 4)
        return 0;

    ip = atoui(argv[1].str);
    reg = atoui(argv[2].str);
    val = atoui(argv[3].str);
    dprintf(0, "%s: idx=%d, id=%d, val=0x%x\n", __func__, ip, reg, val);
    max96705_write_reg(g_sensor, ip, reg, val);
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("des_dbg_en", "des_dbg_en val",
               (console_cmd)&des_dbg_en)
STATIC_COMMAND("max9286_dump_all", "max9286_dump_all",
               (console_cmd)&max9286_dump_all)
STATIC_COMMAND("max9286_write", "max9286_write",
               (console_cmd)&max9286_write)
STATIC_COMMAND("max96705_write", "max96705_write",
               (console_cmd)&max96705_write)
STATIC_COMMAND_END(max9286);
#endif
