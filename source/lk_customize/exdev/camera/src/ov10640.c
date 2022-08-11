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

#include "v4l2.h"
#include "i2c_hal.h"
#include "ov10640.h"


#define BIT(nr)     (1 << nr)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define OV10640_LOG 0

static void *i2c_handle;





#define MAX9286_SLAVE_ID 0x55


#if 0

#define MAC9286_REG_SYS_RESET02     0x3002
#define MAC9286_REG_SYS_CLOCK_ENABLE02  0x3006
#define MAC9286_REG_SYS_CTRL0       0x3008
#define MAC9286_REG_CHIP_ID     0x300a







/*
 * FIXME: remove this when a subdev API becomes available
 * to set the MIPI CSI-2 virtual channel.
 */
static unsigned int virtual_channel;

static const int max9286_framerates[] = {
    [OV5640_15_FPS] = 15,
    [OV5640_30_FPS] = 30,
};

/*
 * Image size under 1280 * 960 are SUBSAMPLING
 * Image size upper 1280 * 960 are SCALING
 */
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
    u32 htot;
    u32 vact;
    u32 vtot;
    const struct reg_value *reg_data;
    u32 reg_data_size;
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


struct i2c_client {
    uint16_t flags;
    uint16_t addr;
    uint16_t adapter;
};

struct ov10640_dev {
    struct i2c_client i2c_client;
    struct v4l2_device vdev;
    int device_id;
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
    bool streaming;
#endif
};

static inline struct ov10640_dev *to_ov10640_dev(struct v4l2_device *vdev)
{
    return containerof(vdev, struct ov10640_dev, vdev);
}


static int ov10640_write_reg(struct ov10640_dev *sensor, u8 reg, u8 val)
{
    u8 buf[2];
    int ret;

    buf[0] = reg >> 8;
    buf[1] = val;


    ret = hal_i2c_write_reg_data(i2c_handle, MAX9286_SLAVE_ID, buf, 1,
                                 buf + 1, 1);

    dprintf(OV10640_LOG, "%s(), ret=%d, buf[0]=0x%x-%x\n", __func__, ret,
            buf[0], buf[1]);

    if (ret < 0) {
        dprintf(INFO, "%s: error: reg=%x, val=%x\n",
                __func__, reg, val);
        return ret;
    }

    return ret;
}

static int ov10640_read_reg(struct ov10640_dev *sensor, u8 reg, u8 *val)
{

    u8 buf[1];
    int ret;

    memset(buf, 0, sizeof(buf));


    ret = hal_i2c_read_reg_data(i2c_handle, MAX9286_SLAVE_ID, &reg, 1,
                                buf, 1);

    if (ret < 0) {
        dprintf(INFO, "%s: error: read reg=%x\n", __func__, reg);
        return ret;
    }

    dprintf(OV10640_LOG, "%s(): trans reg[0]=0x%x, rece buf[0]=0x%x\n ",
            __func__, reg,  buf[0]);
    *val = buf[0];
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

static int max9286_set_framefmt(struct max9286_dev *sensor,
                                struct v4l2_mbus_framefmt *format);

/* restore the last set video mode after chip power-on */
static int max9286_restore_mode(struct max9286_dev *sensor)
{
    int ret;

    /* first load the initial register values */
    ret = max9286_load_regs(sensor, &max9286_mode_init_data);

    if (ret < 0)
        return ret;

    sensor->last_mode = &max9286_mode_init_data;

    /*
        ret = max9286_mod_reg(sensor, OV5640_REG_SYS_ROOT_DIVIDER, 0x3f,
                     (ilog2(OV5640_SCLK2X_ROOT_DIVIDER_DEFAULT) << 2) |
                     ilog2(OV5640_SCLK_ROOT_DIVIDER_DEFAULT));
    */
    ret = max9286_mod_reg(sensor, OV5640_REG_SYS_ROOT_DIVIDER, 0x3f, 1);

    if (ret)
        return ret;

    /* now restore the last capture mode */
    ret = max9286_set_mode(sensor);

    if (ret < 0)
        return ret;

    return max9286_set_framefmt(sensor, &sensor->vdev.fmt);
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

static int ov10640_s_power(struct v4l2_device *vdev, int on)
{
    int ret = 0;
#if 0

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
#endif
    return ret;
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

static int ov10640_get_fmt(struct v4l2_device *vdev,
                           struct v4l2_mbus_framefmt *fmt)
{
#if 0
    struct max9286_dev *sensor = to_max9286_dev(vdev);

    mutex_acquire(&sensor->lock);

    *fmt = sensor->vdev.fmt;

    mutex_release(&sensor->lock);
#endif
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

static int ov10640_set_fmt(struct v4l2_device *vdev,
                           struct v4l2_mbus_framefmt format)
{

    struct ov10640_dev *sensor = to_ov10640_dev(vdev);
#if 0

    const struct max9286_mode_info *new_mode;
    struct v4l2_mbus_framefmt *mbus_fmt = &format;
    int ret;

    mutex_acquire(&sensor->lock);

    if (sensor->streaming) {
        ret = -EBUSY;
        goto out;
    }

    ret = max9286_try_fmt_internal(sensor, mbus_fmt,
                                   sensor->current_fr, &new_mode);

    if (ret)
        goto out;

    if ((new_mode != sensor->current_mode) ||
            (mbus_fmt->code != sensor->vdev.fmt.code)) {
        if (mbus_fmt->code != sensor->vdev.fmt.code) {
            sensor->pending_fmt_change = true;
        }

        if (new_mode != sensor->current_mode) {
            sensor->current_mode = new_mode;
            sensor->pending_mode_change = true;
        }

        sensor->vdev.fmt = *mbus_fmt;
    }

out:
    mutex_release(&sensor->lock);
    return ret;
#endif
    sensor->vdev.fmt = format;
    return 0;
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

static int ov10640_s_ctrl(struct v4l2_device *vdev,
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

static int ov10640_enum_frame_size(struct v4l2_frame_size_enum *fse)
{
#if 0
    fse->min_width =
        max9286_mode_data[0][fse->index].hact;
    fse->max_width = fse->min_width;
    fse->min_height =
        max9286_mode_data[0][fse->index].vact;
    fse->max_height = fse->min_height;
#endif
    return 0;
}

static int ov10640_enum_frame_interval(
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


static int ov10640_g_frame_interval(struct v4l2_device *vdev,
                                    struct v4l2_fract *fi)
{
#if 0
    struct max9286_dev *sensor = to_max9286_dev(vdev);
    mutex_acquire(&sensor->lock);
    fi = &sensor->vdev.frame_interval;
    mutex_release(&sensor->lock);
#endif
    return 0;
}

static int ov10640_s_frame_interval(struct v4l2_device *vdev,
                                    struct v4l2_fract frame_interval)
{
#if 0
    struct max9286_dev *sensor = to_max9286_dev(vdev);
    const struct max9286_mode_info *mode;
    unsigned int frame_rate;
    int ret = 0;

    mutex_acquire(&sensor->lock);

    if (sensor->streaming) {
        ret = -EBUSY;
        goto out;
    }

    mode = sensor->current_mode;

    frame_rate = max9286_try_frame_interval(sensor, &frame_interval,
                                            mode->hact, mode->vact);

    if (frame_rate < 0)
        frame_rate = OV5640_15_FPS;

    mode = max9286_find_mode(sensor, frame_rate, mode->hact,
                             mode->vact, true);

    if (!mode) {
        ret = -EINVAL;
        goto out;
    }

    if (mode != sensor->current_mode ||
            frame_rate != sensor->current_fr) {
        sensor->current_fr = frame_rate;
        sensor->vdev.frame_interval = frame_interval;
        sensor->current_mode = mode;
        sensor->pending_mode_change = true;
    }

out:
    mutex_release(&sensor->lock);
    return ret;
#else
    return 0;
#endif
}

static int ov10640_s_stream(struct v4l2_device *vdev, int enable)
{
#if 0
    struct max9286_dev *sensor = to_max9286_dev(vdev);
    int ret = 0;

    mutex_acquire(&sensor->lock);

    if (sensor->streaming == !enable) {
        if (enable && sensor->pending_mode_change) {
            ret = max9286_set_mode(sensor);

            if (ret)
                goto out;
        }

        if (enable && sensor->pending_fmt_change) {
            ret = max9286_set_framefmt(sensor, &sensor->vdev.fmt);

            if (ret)
                goto out;

            sensor->pending_fmt_change = false;
        }

        if (sensor->vdev.ep.bus_type == V4L2_MBUS_CSI2)
            ret = max9286_set_stream_mipi(sensor, enable);
        else
            ret = max9286_set_stream_dvp(sensor, enable);

        if (!ret)
            sensor->streaming = enable;
    }

out:
    mutex_release(&sensor->lock);
    return ret;
#endif
    return 0;
}


static int ov10640_set_interface(struct v4l2_device *vdev,
                                 struct v4l2_fwnode_endpoint ep)
{
#if 0
    struct max9286_dev *sensor = to_max9286_dev(vdev);
    int ret;
    u8 val;

    mutex_acquire(&sensor->lock);

    if (sensor->streaming) {
        ret = -EBUSY;
        goto out;
    }

    if (ep.bus_type == vdev->ep.bus_type) {
        ret = 0;
        goto out;
    }

    switch (ep.bus_type) {
        case V4L2_MBUS_PARALLEL:
            val = 0x00;
            break;

        case V4L2_MBUS_BT656:
            val = 0x01;
            break;

        default:
            ret = -EINVAL;
            goto out;
    }

    ret = max9286_write_reg(sensor, OV5640_REG_CCIR656_CONTROL00, val);

    if (ret)
        return ret;

out:
    mutex_release(&sensor->lock);
    return ret;
#endif
    return 0;

}

static int ov10640_get_interface(struct v4l2_device *vdev,
                                 struct v4l2_fwnode_endpoint *ep)
{
#if 0
    struct max9286_dev *sensor = to_max9286_dev(vdev);

    mutex_acquire(&sensor->lock);

    ep = &sensor->vdev.ep;

    mutex_release(&sensor->lock);
#endif
    return 0;
}


static const struct v4l2_dev_ops ov10640_vdev_ops = {
    .s_power = ov10640_s_power,
    .g_frame_interval = ov10640_g_frame_interval,
    .s_frame_interval = ov10640_s_frame_interval,
    .s_stream = ov10640_s_stream,
    .get_fmt = ov10640_get_fmt,
    .set_fmt = ov10640_set_fmt,
    .set_interface = ov10640_set_interface,
    .get_interface = ov10640_get_interface,
    .enum_frame_size = ov10640_enum_frame_size,
    .enum_frame_interval = ov10640_enum_frame_interval,
    //.g_volatile_ctrl = max9286_g_volatile_ctrl,
    .s_ctrl = ov10640_s_ctrl,
};

#if 0
static int max9286_dump_register(struct max9286_dev *sensor)
{
    int ret = 0;
    u8 rval;

    printf("%s: start\n", __func__);

    for (int i = 0; i < 10; i++) {
        ret = max9286_read_reg(sensor, i, &rval);
        dprintf(MAX9286_LOG, "%s(): ret=%d, reg=0x%x, val=0x%x.\n", __func__, ret,
                i, rval);

        if (ret) {
            dprintf(0, "%s: failed to read register: 0x%x\n",
                    __func__, i);
            return ret;
        }
    }

    printf("%s: end\n", __func__);
    return ret;
}
#endif

struct v4l2_device *ov10640_init(int i2c_bus)
{
    struct ov10640_dev *sensor;
    //struct v4l2_mbus_framefmt *fmt;
    int ret = 0;
    dprintf(OV10640_LOG, "%s\n", __func__);

    sensor = malloc(sizeof(*sensor));

    if (!sensor)
        return NULL;

    memset(sensor, 0, sizeof(*sensor));

    if (i2c_bus == 0) {
        sensor->device_id = 0;
        ret = hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C9);
    }
    else if (i2c_bus == 1) {
        sensor->device_id = 1;
        ret = hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C9);
    }
    else
        printf("ov10640 i2c bus error!\n");


    //ret = max9286_dump_register(sensor);
    printf("%s: ret=%d\n", __func__, ret);
    //if (ret)
    //  return NULL;
#if 0
    ret = max9286_init_controls(sensor);

    if (ret)
        goto entity_cleanup;

    dprintf(OV10640_LOG, "max9286_init() end\n");
#endif

    sensor->vdev.ep.bus_type = V4L2_MBUS_PARALLEL;
    sensor->vdev.ops = ov10640_vdev_ops;

    dprintf(OV10640_LOG, "%s end.\n", __func__);
    return &sensor->vdev;

}



