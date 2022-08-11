/*
 * tlv320.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 *
 */

#include "tlv320.h"
#include "sd_audio.h"
#include "i2c_hal.h"
#include "chip_res.h"

#define TLV320AIC23_INSTANCE_I2C_1A 0x1a
#define TLV320AIC23_INSTANCE_I2C_1B 0x1b
#define TLV320AI23_MAX_DEV_NUM 3

/* Codec TLV320AIC23 */
#define TLV320AIC23_LINVOL 0x00
#define TLV320AIC23_RINVOL 0x01
#define TLV320AIC23_LCHNVOL 0x02
#define TLV320AIC23_RCHNVOL 0x03
#define TLV320AIC23_ANLG 0x04
#define TLV320AIC23_DIGT 0x05
#define TLV320AIC23_PWR 0x06
#define TLV320AIC23_DIGT_FMT 0x07
#define TLV320AIC23_SRATE 0x08
#define TLV320AIC23_ACTIVE 0x09
#define TLV320AIC23_RESET 0x0F

/* Left (right) line input volume control register */
#define TLV320AIC23_LRS_ENABLED 0x0100
#define TLV320AIC23_LIM_MUTED 0x0080
#define TLV320AIC23_LIV_DEFAULT 0x0017
#define TLV320AIC23_LIV_MAX 0x001f
#define TLV320AIC23_LIV_MIN 0x0000

/* Left (right) channel headphone volume control register */
#define TLV320AIC23_LZC_ON 0x0080
#define TLV320AIC23_LHV_DEFAULT 0x0079
#define TLV320AIC23_LHV_MAX 0x007f
#define TLV320AIC23_LHV_MIN 0x0030

/* Analog audio path control register */
#define TLV320AIC23_STA_REG(x) ((x) << 6)
#define TLV320AIC23_STE_ENABLED 0x0020
#define TLV320AIC23_DAC_SELECTED 0x0010
#define TLV320AIC23_BYPASS_ON 0x0008
#define TLV320AIC23_INSEL_MIC 0x0004
#define TLV320AIC23_MICM_MUTED 0x0002
#define TLV320AIC23_MICB_20DB 0x0001

/* Digital audio path control register */
#define TLV320AIC23_DACM_MUTE 0x0008
#define TLV320AIC23_DEEMP_32K 0x0002
#define TLV320AIC23_DEEMP_44K 0x0004
#define TLV320AIC23_DEEMP_48K 0x0006
#define TLV320AIC23_ADCHP_ON 0x0001

/* Power control down register */
#define TLV320AIC23_DEVICE_PWR_OFF 0x0080
#define TLV320AIC23_CLK_OFF 0x0040
#define TLV320AIC23_OSC_OFF 0x0020
#define TLV320AIC23_OUT_OFF 0x0010
#define TLV320AIC23_DAC_OFF 0x0008
#define TLV320AIC23_ADC_OFF 0x0004
#define TLV320AIC23_MIC_OFF 0x0002
#define TLV320AIC23_LINE_OFF 0x0001

/* Digital audio interface register */
#define TLV320AIC23_MS_MASTER 0x0040
#define TLV320AIC23_LRSWAP_ON 0x0020
#define TLV320AIC23_LRP_ON 0x0010
#define TLV320AIC23_IWL_16 0x0000
#define TLV320AIC23_IWL_20 0x0004
#define TLV320AIC23_IWL_24 0x0008
#define TLV320AIC23_IWL_32 0x000C
#define TLV320AIC23_FOR_I2S 0x0002
#define TLV320AIC23_FOR_DSP 0x0003
#define TLV320AIC23_FOR_LJUST 0x0001

/* Sample rate control register */
#define TLV320AIC23_CLKOUT_HALF 0x0080
#define TLV320AIC23_CLKIN_HALF 0x0040
#define TLV320AIC23_BOSR_384fs 0x0002 /* BOSR_272fs in USB mode */
#define TLV320AIC23_USB_CLK_ON 0x0001
#define TLV320AIC23_SR_MASK 0xf
#define TLV320AIC23_CLKOUT_SHIFT 7
#define TLV320AIC23_CLKIN_SHIFT 6
#define TLV320AIC23_SR_SHIFT 2
#define TLV320AIC23_BOSR_SHIFT 1

/* Digital interface register */
#define TLV320AIC23_ACT_ON 0x0001

#define AUDIO_CODEC_DEBUG 2
#define TLV320AIC23B_Q1_REG_MAX_ADDR (0xf + 1)

/* functions declaration */
static bool sdrv_tlv320_initialize(struct audio_codec_dev *dev);
static bool sdrv_tlv320_start_up(struct audio_codec_dev *dev);
static bool sdrv_tlv320_set_volume(struct audio_codec_dev *dev,
                                   int volume_percent,
                                   enum audio_volume_type vol_type);
static bool sdrv_tlv320_set_format(struct audio_codec_dev dev,
                                   pcm_params_t pcm_info);
static bool sdrv_tlv320_set_hw_params(struct audio_codec_dev dev,
                                      pcm_params_t pcm_info);
static bool sdrv_tlv320_trigger(struct audio_codec_dev dev, int cmd);
static bool sdrv_tlv320_shutdown(struct audio_codec_dev dev);
static bool sdrv_tlv320_set_input_path(struct audio_codec_dev *dev,
                                       uint32_t input_path);
static bool sdrv_tlv320_set_output_path(struct audio_codec_dev *dev,
                                        uint32_t output_path);

static struct audio_codec_dev
    g_tlv320aic23_instance[TLV320AI23_MAX_DEV_NUM] = {
    {
        1, TLV320AIC23_INSTANCE_I2C_1A, "safety", RES_I2C_I2C4,
        NULL, AUDIO_CODEC_SET_INPUT_AS_DEFAULT,
        AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT, 50, 100
    },
    {
        2, TLV320AIC23_INSTANCE_I2C_1A, "security", RES_I2C_I2C6,
        NULL, AUDIO_CODEC_SET_INPUT_AS_DEFAULT,
        AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT, 50, 100
    },
    {
        3, TLV320AIC23_INSTANCE_I2C_1B, "security", RES_I2C_I2C6,
        NULL, AUDIO_CODEC_SET_INPUT_AS_DEFAULT,
        AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT, 50, 100
    },
};

/* to save register value */
static uint32_t
codec_register_val[TLV320AI23_MAX_DEV_NUM][TLV320AIC23B_Q1_REG_MAX_ADDR];
const static uint32_t
codec_register_after_reset[TLV320AIC23B_Q1_REG_MAX_ADDR] = {
    0x97, 0x97, 0xf9, 0xf9, 0x1a, 0x04, 0x7f, 0x01,
    0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

/* to read register */
#define codec_read_reg(codec_id, reg) (codec_register_val[codec_id - 1][reg])

/* tlv320aic23b_q1 is a write-only device */
static int codec_i2c_write_reg(struct audio_codec_dev dev, u32 reg,
                               u32 val)
{
    uint8_t addr, buf;
    int ret;

    addr = (reg << 1) | (val >> 8 & 0x01);
    buf = (u8)(val & 0xff);

    ret = hal_i2c_write_reg_data(dev.i2c_handle, dev.i2c_addr, &addr, 1, &buf,
                                 1);
    /* local register update */
    codec_register_val[dev.id - 1][reg] = val;

    dprintf(AUDIO_CODEC_DEBUG, "%s(), ret=%d, addr=0x%x buf=0x%x\n", __func__,
            ret, reg, val);

    if (ret < 0) {
        dprintf(AUDIO_CODEC_DEBUG, "%s: error: reg=%x, val=%x\n", __func__, reg,
                val);
    }

    return ret;
}

static const struct au_codec_dev_ctrl_interface tlv320aic23b_q1_drv = {
    sdrv_tlv320_initialize,
    sdrv_tlv320_start_up,
    sdrv_tlv320_set_volume,
    sdrv_tlv320_set_format,
    sdrv_tlv320_set_hw_params,
    sdrv_tlv320_trigger,
    sdrv_tlv320_shutdown,
    sdrv_tlv320_set_input_path,
    sdrv_tlv320_set_output_path,
};

const struct au_codec_dev_ctrl_interface
*sdrv_tlv320_get_controller_interface(void)
{
    return &tlv320aic23b_q1_drv;
}

struct audio_codec_dev *sdrv_tlv320_get_dev(int codec_id)
{
    return &g_tlv320aic23_instance[codec_id - 1];
}
static bool sdrv_tlv320_initialize(struct audio_codec_dev *dev) {
    return true;
}
/**
 * @brief reset codec and set some common register
 *
 * @param dev codec device
 *
 * @return \b true
 *
*/
static bool sdrv_tlv320_start_up(struct audio_codec_dev *dev)
{
    uint32_t power;
    /* reset */
    codec_i2c_write_reg(*dev, TLV320AIC23_RESET, 0x0);

    /* local register updata */
    for (int i = 0; i < TLV320AIC23B_Q1_REG_MAX_ADDR; i++) {
        codec_register_val[dev->id - 1][i] = codec_register_after_reset[i];
    }

    /* power down */
    power = codec_read_reg(dev->id,
                           TLV320AIC23_PWR) | TLV320AIC23_DEVICE_PWR_OFF;
    codec_i2c_write_reg(*dev, TLV320AIC23_PWR, power);

    /* set common register */
    codec_i2c_write_reg(*dev, TLV320AIC23_DIGT, 0x0);
    /* set line in as default */
    codec_i2c_write_reg(*dev, TLV320AIC23_ANLG, TLV320AIC23_DAC_SELECTED);

    /* set default volume, both input and output */
    sdrv_tlv320_set_volume(dev, dev->hphone_out_vol, true);
    sdrv_tlv320_set_volume(dev, dev->line_in_vol, false);

    return true;
}


/**
 * @brief set output path volume
 *
 * @param dev codec device
 *
 * @param volume_percent percentage of full scale volume(0~100)
 *
 * @return \b true
*/
static bool sdrv_tlv320_set_volume(struct audio_codec_dev *dev,
				   int volume_percent,
				   enum audio_volume_type vol_type)
{
    int step;
    uint32_t vol;

    if (vol_type == AUDIO_VOL_HEADPHONE) {
	    dev->hphone_out_vol = volume_percent;
	    /* set phone out volume; left/right volume equal */
	    step = (TLV320AIC23_LHV_MAX - TLV320AIC23_LHV_MIN) / 10;
	    vol = (volume_percent / 10 * step) & TLV320AIC23_LHV_MAX;
	    vol += TLV320AIC23_LHV_MIN;
	    codec_i2c_write_reg(*dev, TLV320AIC23_LCHNVOL,
				TLV320AIC23_LRS_ENABLED | vol);
	    codec_i2c_write_reg(*dev, TLV320AIC23_RCHNVOL,
				TLV320AIC23_LRS_ENABLED | vol);
    } else if (vol_type == AUDIO_VOL_LINEOUT) {
	    dev->line_in_vol = volume_percent;
	    /* set line input volume; left/right volume equal */
	    step = (TLV320AIC23_LIV_MAX - TLV320AIC23_LIV_MIN) / 10;
	    vol = (volume_percent / 10 * step) & TLV320AIC23_LIV_MAX;
	    codec_i2c_write_reg(*dev, TLV320AIC23_LINVOL,
				TLV320AIC23_LRS_ENABLED | vol);
	    codec_i2c_write_reg(*dev, TLV320AIC23_RINVOL,
				TLV320AIC23_LRS_ENABLED | vol);
    }

    return true;
}

/**
 * @brief set i2s interface format and master/slave mode
 *
 * @param dev codec device
 *
 * @param pcm_info pcm data info
 *
 * @return \b true
*/
static bool sdrv_tlv320_set_format(struct audio_codec_dev dev,
                                   pcm_params_t pcm_info)
{
    int interface = 0, ms = 0;

    /* set i2s interface mode */
    switch (pcm_info.standard) {
        case SD_AUDIO_I2S_STANDARD_PHILLIPS:
            interface = TLV320AIC23_FOR_I2S;
            break;

        case SD_AUDIO_I2S_LEFT_JUSTIFIED:
            interface = TLV320AIC23_FOR_LJUST;
            break;

        case SD_AUDIO_I2S_RIGHT_JUSTIFIED:
            printf("func<%s>: unsupported i2s right justified mode.\n", __func__);
            break;

        case SD_AUDIO_I2S_DSP_A:
            interface = TLV320AIC23_FOR_DSP | TLV320AIC23_LRP_ON;
            break;

        case SD_AUDIO_I2S_DSP_B:
            interface = TLV320AIC23_FOR_DSP & (~TLV320AIC23_LRP_ON);
            break;

        default:
            dprintf(AUDIO_CODEC_DEBUG, "codec interface mode wrong arg or no init.\n");
    }

    /* set master/slave */
    if ((pcm_info.mode & SD_AUDIO_TRANSFER_MODE_ENABLE) ==
            SD_AUDIO_TRANSFER_CODEC_MASTER) {
        ms = TLV320AIC23_MS_MASTER;
    }

    codec_i2c_write_reg(dev, TLV320AIC23_DIGT_FMT, interface | ms);
    return true;
}

/**
 * @brief set slot width and sample rate
 *
 * @param dev codec device
 *
 * @param pcm_info pcm data info
 *
 * @return \b true
*/
static bool sdrv_tlv320_set_hw_params(struct audio_codec_dev dev,
                                      pcm_params_t pcm_info)
{
    uint32_t sample_rate = 0, slot_width = 0, format = 0;
    /* set sample rate */
    sample_rate = pcm_info.sample_rate;
    uint32_t val = 0;

    switch (sample_rate) {
        case SD_AUDIO_SR_8000:
            val = 0x3 << TLV320AIC23_SR_SHIFT;
            break;

        case SD_AUDIO_SR_32000:
            val = 0x6 << TLV320AIC23_SR_SHIFT;
            break;

        case SD_AUDIO_SR_48000:
            val = 0;
            break;

        case SD_AUDIO_SR_96000:
            val = 0x7 << TLV320AIC23_SR_SHIFT;
            break;

        default:
            dprintf(AUDIO_CODEC_DEBUG, "unsupport sample rate: %d.\n", sample_rate);
    }

    codec_i2c_write_reg(dev, TLV320AIC23_SRATE, val);

    /* set pcm data slot width */
    format = codec_read_reg(dev.id, TLV320AIC23_DIGT_FMT);

    switch (pcm_info.slot_width) {
        case SD_AUDIO_SLOT_WIDTH_16BITS:
            slot_width = TLV320AIC23_IWL_16;
            break;

        case SD_AUDIO_SLOT_WIDTH_20BITS:
            slot_width = TLV320AIC23_IWL_20;
            break;

        case SD_AUDIO_SLOT_WIDTH_24BITS:
            slot_width = TLV320AIC23_IWL_24;
            break;

        case SD_AUDIO_SLOT_WIDTH_32BITS:
        case SD_AUDIO_SLOT_WIDTH_NO_INIT:
            slot_width = TLV320AIC23_IWL_32;
            break;

        default:
            dprintf(AUDIO_CODEC_DEBUG, "codec slot width wrong arg or no init.\n");
    }

    format |= slot_width;
    codec_i2c_write_reg(dev, TLV320AIC23_DIGT_FMT, format);
    return true;
}

/**
 * @brief trigger codec playback/capture start/stop
 *
 * @param dev codec dev
 *
 * @param cmd trigger command
 *
 * @return \b true
*/
static bool sdrv_tlv320_trigger(struct audio_codec_dev dev, int cmd)
{
    uint32_t power = 0;

    /* inactive digital interface and power off */
    codec_i2c_write_reg(dev, TLV320AIC23_ACTIVE, 0x0);
    power = codec_read_reg(dev.id,
                           TLV320AIC23_PWR) | TLV320AIC23_DEVICE_PWR_OFF;
    codec_i2c_write_reg(dev, TLV320AIC23_PWR, power);

    /* handle command */
    if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_START) {
        power &= ~(TLV320AIC23_DAC_OFF | TLV320AIC23_OUT_OFF);
    }

    if (cmd == SD_AUDIO_PCM_TRIGGER_CAPTURE_START) {
        if (dev.input_path == AUDIO_CODEC_SET_INPUT_AS_LINE_IN ||
                dev.input_path == AUDIO_CODEC_SET_INPUT_AS_DEFAULT)
            power &= ~(TLV320AIC23_ADC_OFF | TLV320AIC23_LINE_OFF);
        else
            power &= ~(TLV320AIC23_ADC_OFF | TLV320AIC23_MIC_OFF);
    }

    if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP) {
        power |= TLV320AIC23_DAC_OFF | TLV320AIC23_OUT_OFF;
    }

    if (cmd == SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP) {
        power |= TLV320AIC23_ADC_OFF | TLV320AIC23_LINE_OFF | TLV320AIC23_MIC_OFF;
    }

    codec_i2c_write_reg(dev, TLV320AIC23_ACTIVE, TLV320AIC23_ACT_ON);
    codec_i2c_write_reg(dev, TLV320AIC23_PWR,
                        power & ~TLV320AIC23_DEVICE_PWR_OFF);

    return true;
}

/**
 * @brief codec power off
 *
 * @param dev codec dev
 *
 * @return \b true
*/
static bool sdrv_tlv320_shutdown(struct audio_codec_dev dev)
{
    /* inactive and power down */
    codec_i2c_write_reg(dev, TLV320AIC23_ACTIVE, 0x0);
    codec_i2c_write_reg(dev, TLV320AIC23_PWR, 0xff);
    return true;
}

/**
 * @brief set input path
 *
 * @param dev codec dev
 *
 * @param input_path line in(default) or mic in
 *
 * @return \b true
*/
static bool sdrv_tlv320_set_input_path(struct audio_codec_dev *dev,
                                       uint32_t input_path)
{
    if (input_path == AUDIO_CODEC_SET_INPUT_AS_MIC_IN) {
        uint32_t analog_au_path;
        analog_au_path = codec_read_reg(dev->id,
                                        TLV320AIC23_ANLG) | TLV320AIC23_INSEL_MIC;
        codec_i2c_write_reg(*dev, TLV320AIC23_ANLG, analog_au_path);
    }

    dev->input_path = input_path;

    return true;
}

/**
 * @brief set output path
 *
 * @param dev codec dev
 *
 * @param output_path line out(defalut) or phone out
 *
 * @return \b true
*/
static bool sdrv_tlv320_set_output_path(struct audio_codec_dev *dev,
                                        uint32_t output_path)
{
    dev->output_path = output_path;
    return true;
}
