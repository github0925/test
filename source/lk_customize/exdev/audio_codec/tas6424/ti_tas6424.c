/*
 * tas6424.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 *
 */

#include "ti_tas6424.h"
#include "sd_audio.h"
#include "chip_res.h"
#include "i2c_hal.h"
#include "tca9539.h"
#include "tca6408.h"
#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

#define TAS6404_INSTANCE_I2C_6A 0x6a
#define TAS6404_MAX_DEV_NUM 2

#define AUDIO_CODEC_DEBUG 2

/* Codec TAS6404 */

/* Register Address Map */
#define TAS6424_MODE_CTRL 0x00
#define TAS6424_MISC_CTRL1 0x01
#define TAS6424_MISC_CTRL2 0x02
#define TAS6424_SAP_CTRL 0x03
#define TAS6424_CH_STATE_CTRL 0x04
#define TAS6424_CH1_VOL_CTRL 0x05
#define TAS6424_CH2_VOL_CTRL 0x06
#define TAS6424_CH3_VOL_CTRL 0x07
#define TAS6424_CH4_VOL_CTRL 0x08
#define TAS6424_DC_DIAG_CTRL1 0x09
#define TAS6424_DC_DIAG_CTRL2 0x0a
#define TAS6424_DC_DIAG_CTRL3 0x0b
#define TAS6424_DC_LOAD_DIAG_REP12 0x0c
#define TAS6424_DC_LOAD_DIAG_REP34 0x0d
#define TAS6424_DC_LOAD_DIAG_REPLO 0x0e
#define TAS6424_CHANNEL_STATE 0x0f
#define TAS6424_CHANNEL_FAULT 0x10
#define TAS6424_GLOB_FAULT1 0x11
#define TAS6424_GLOB_FAULT2 0x12
#define TAS6424_WARN 0x13
#define TAS6424_PIN_CTRL 0x14
#define TAS6424_AC_DIAG_CTRL1 0x15
#define TAS6424_AC_DIAG_CTRL2 0x16
#define TAS6424_AC_LOAD_DIAG_REP1 0x17
#define TAS6424_AC_LOAD_DIAG_REP2 0x18
#define TAS6424_AC_LOAD_DIAG_REP3 0x19
#define TAS6424_AC_LOAD_DIAG_REP4 0x1a
#define TAS6424_MISC_CTRL3 0x21
#define TAS6424_CLIP_CTRL 0x22
#define TAS6424_CLIP_WINDOW 0x23
#define TAS6424_CLIP_WARN 0x24
#define TAS6424_CBC_STAT 0x25
#define TAS6424_MISC_CTRL4 0x26
#define TAS6424_MAX TAS6424_MISC_CTRL4

/* TAS6424_MODE_CTRL_REG */
#define TAS6424_RESET BIT(7)

/* TAS6424_SAP_CTRL_REG */
#define TAS6424_SAP_RATE_MASK GEN_MASK(7, 6)
#define TAS6424_SAP_RATE_44100 (0x00 << 6)
#define TAS6424_SAP_RATE_48000 (0x01 << 6)
#define TAS6424_SAP_RATE_96000 (0x02 << 6)
#define TAS6424_SAP_TDM_SLOT_LAST BIT(5)
#define TAS6424_SAP_TDM_SLOT_SZ_16 BIT(4)
#define TAS6424_SAP_TDM_SLOT_SWAP BIT(3)
#define TAS6424_SAP_FMT_MASK GEN_MASK(2, 0)
#define TAS6424_SAP_RIGHTJ_24 (0x00 << 0)
#define TAS6424_SAP_RIGHTJ_20 (0x01 << 0)
#define TAS6424_SAP_RIGHTJ_18 (0x02 << 0)
#define TAS6424_SAP_RIGHTJ_16 (0x03 << 0)
#define TAS6424_SAP_I2S (0x04 << 0)
#define TAS6424_SAP_LEFTJ (0x05 << 0)
#define TAS6424_SAP_DSP (0x06 << 0)

/* TAS6424_CH_STATE_CTRL_REG */
#define TAS6424_CH1_STATE_MASK GEN_MASK(7, 6)
#define TAS6424_CH1_STATE_PLAY (0x00 << 6)
#define TAS6424_CH1_STATE_HIZ (0x01 << 6)
#define TAS6424_CH1_STATE_MUTE (0x02 << 6)
#define TAS6424_CH1_STATE_DIAG (0x03 << 6)
#define TAS6424_CH2_STATE_MASK GEN_MASK(5, 4)
#define TAS6424_CH2_STATE_PLAY (0x00 << 4)
#define TAS6424_CH2_STATE_HIZ (0x01 << 4)
#define TAS6424_CH2_STATE_MUTE (0x02 << 4)
#define TAS6424_CH2_STATE_DIAG (0x03 << 4)
#define TAS6424_CH3_STATE_MASK GEN_MASK(3, 2)
#define TAS6424_CH3_STATE_PLAY (0x00 << 2)
#define TAS6424_CH3_STATE_HIZ (0x01 << 2)
#define TAS6424_CH3_STATE_MUTE (0x02 << 2)
#define TAS6424_CH3_STATE_DIAG (0x03 << 2)
#define TAS6424_CH4_STATE_MASK GEN_MASK(1, 0)
#define TAS6424_CH4_STATE_PLAY (0x00 << 0)
#define TAS6424_CH4_STATE_HIZ (0x01 << 0)
#define TAS6424_CH4_STATE_MUTE (0x02 << 0)
#define TAS6424_CH4_STATE_DIAG (0x03 << 0)
#define TAS6424_ALL_STATE_PLAY                                                 \
    (TAS6424_CH1_STATE_PLAY | TAS6424_CH2_STATE_PLAY |                         \
     TAS6424_CH3_STATE_PLAY | TAS6424_CH4_STATE_PLAY)
#define TAS6424_ALL_STATE_HIZ                                                  \
    (TAS6424_CH1_STATE_HIZ | TAS6424_CH2_STATE_HIZ | TAS6424_CH3_STATE_HIZ |   \
     TAS6424_CH4_STATE_HIZ)
#define TAS6424_ALL_STATE_MUTE                                                 \
    (TAS6424_CH1_STATE_MUTE | TAS6424_CH2_STATE_MUTE |                         \
     TAS6424_CH3_STATE_MUTE | TAS6424_CH4_STATE_MUTE)
#define TAS6424_ALL_STATE_DIAG                                                 \
    (TAS6424_CH1_STATE_DIAG | TAS6424_CH2_STATE_DIAG |                         \
     TAS6424_CH3_STATE_DIAG | TAS6424_CH4_STATE_DIAG)

/* TAS6424_DC_DIAG_CTRL1 */
#define TAS6424_LDGBYPASS_SHIFT 0
#define TAS6424_LDGBYPASS_MASK BIT(TAS6424_LDGBYPASS_SHIFT)

/* TAS6424_GLOB_FAULT1_REG */
#define TAS6424_FAULT_CLOCK BIT(4)
#define TAS6424_FAULT_PVDD_OV BIT(3)
#define TAS6424_FAULT_VBAT_OV BIT(2)
#define TAS6424_FAULT_PVDD_UV BIT(1)
#define TAS6424_FAULT_VBAT_UV BIT(0)

/* TAS6424_GLOB_FAULT2_REG */
#define TAS6424_FAULT_OTSD BIT(4)
#define TAS6424_FAULT_OTSD_CH1 BIT(3)
#define TAS6424_FAULT_OTSD_CH2 BIT(2)
#define TAS6424_FAULT_OTSD_CH3 BIT(1)
#define TAS6424_FAULT_OTSD_CH4 BIT(0)

/* TAS6424_WARN_REG */
#define TAS6424_WARN_VDD_UV BIT(6)
#define TAS6424_WARN_VDD_POR BIT(5)
#define TAS6424_WARN_VDD_OTW BIT(4)
#define TAS6424_WARN_VDD_OTW_CH1 BIT(3)
#define TAS6424_WARN_VDD_OTW_CH2 BIT(2)
#define TAS6424_WARN_VDD_OTW_CH3 BIT(1)
#define TAS6424_WARN_VDD_OTW_CH4 BIT(0)

/* TAS6424_MISC_CTRL3_REG */
#define TAS6424_CLEAR_FAULT BIT(7)
#define TAS6424_PBTL_CH_SEL BIT(6)
#define TAS6424_MASK_CBC_WARN BIT(5)
#define TAS6424_MASK_VDD_UV BIT(4)
#define TAS6424_OTSD_AUTO_RECOVERY BIT(3)

/* functions declaration */
static bool sdrv_tas6424_initialize(struct audio_codec_dev *dev);
static bool sdrv_tas6424_start_up(struct audio_codec_dev *dev);
static bool sdrv_tas6424_set_volume(struct audio_codec_dev *dev,
                                    int volume_percent,
                                    enum audio_volume_type vol_type);
static bool sdrv_tas6424_set_format(struct audio_codec_dev dev,
                                    pcm_params_t pcm_info);
static bool sdrv_tas6424_set_hw_params(struct audio_codec_dev dev,
                                       pcm_params_t pcm_info);
static bool sdrv_tas6424_trigger(struct audio_codec_dev dev, int cmd);
static bool sdrv_tas6424_shutdown(struct audio_codec_dev dev);
static bool sdrv_tas6424_set_input_path(struct audio_codec_dev *dev,
                                        uint32_t input_path);
static bool sdrv_tas6424_set_output_path(struct audio_codec_dev *dev,
                                         uint32_t output_path);

static struct audio_codec_dev g_tas6424_instance[TAS6404_MAX_DEV_NUM] = {
    {1, TAS6404_INSTANCE_I2C_6A, "safety", RES_I2C_I2C6, NULL,
     AUDIO_CODEC_SET_INPUT_AS_DEFAULT, AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT, 50,
     100},
    {2, TAS6404_INSTANCE_I2C_6A, "security", RES_I2C_I2C6, NULL,
     AUDIO_CODEC_SET_INPUT_AS_DEFAULT, AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT, 50,
     100},
};

/*IO Expand address is I2C 12, 0x74 */
#define IO_EXPANDER_I2C_BUS (12)
#define IO_EXPANDER_I2C_ADDR (0x74)

/**
 * @brief tas6424 write register.
 *
 * @param dev codec device.
 * @param reg target register
 * @param val value for writing
 * @return int result of operation.
 */
static int tas6424_i2c_write_reg(struct audio_codec_dev dev, u32 reg, u32 val) {
    uint8_t addr, buf;
    int ret;

    addr = (u8)(reg & 0xff);
    buf = (u8)(val & 0xff);

    ret =
        hal_i2c_write_reg_data(dev.i2c_handle, dev.i2c_addr, &addr, 1, &buf, 1);

    dprintf(AUDIO_CODEC_DEBUG, "%s(), ret=%d, addr=0x%x buf=0x%x\n", __func__,
            ret, reg, val);

    if (ret < 0) {
        dprintf(AUDIO_CODEC_DEBUG, "%s: error: reg=%x, val=%x\n", __func__, reg,
                val);
    }

    return ret;
}
/**
 * @brief tas6424 read register.
 *
 * @param dev
 * @param reg
 * @return u32
 */
static u32 tas6424_i2c_read_reg(struct audio_codec_dev dev, u32 reg) {
    unsigned char tx[3] = {0}, rx[3] = {0};
    int wlen, rlen;
    int ret;
    wlen = 1;
    rlen = 1;
    tx[0] = (unsigned char)(reg & 0x7F);
    ret = hal_i2c_read_reg_data(dev.i2c_handle, 0x6a, tx, wlen, rx, rlen);
    dprintf(AUDIO_CODEC_DEBUG,
            "%s: result: read tas6424 addr=0x%x,reg=0x%x 0x%x\n", __func__,
            dev.i2c_addr, reg, rx[0]);
    if (ret < 0) {
        dprintf(AUDIO_CODEC_DEBUG, "%s: error: read tas6424 reg=%x,ret=%x\n",
                __func__, reg, ret);
    }
    return rx[0];
}
/**
 * @brief dump tas6424 register values.
 *
 * @param dev
 */
static void tas6424_i2c_dump_reg(struct audio_codec_dev dev) {
    int reg_val = 0;
    for (int i = 0; i < 0x27; i++) {
        reg_val = tas6424_i2c_read_reg(dev, i);
        dprintf(AUDIO_CODEC_DEBUG, "%s: Read tas6424 reg=0x%x,val=0x%x\n",
                __func__, dev.i2c_addr, reg_val);
    }
}

static const struct au_codec_dev_ctrl_interface tas6424_drv = {
    sdrv_tas6424_initialize,      sdrv_tas6424_start_up,
    sdrv_tas6424_set_volume,      sdrv_tas6424_set_format,
    sdrv_tas6424_set_hw_params,   sdrv_tas6424_trigger,
    sdrv_tas6424_shutdown,        sdrv_tas6424_set_input_path,
    sdrv_tas6424_set_output_path,
};

#if FAST_AUDIO_CFG0 == 1
#define AUDIO_DIG_AMP_STANDBT_N TCA6408_P0
#define AUDIO_DIG_AMP_MUTE_N TCA6408_P1
#define AUDIO_DIG_AMP_FAULT_N TCA6408_P2
#define AUDIO_DIG_AMP_WARN_N TCA6408_P3
#else
#define AUDIO_DIG_AMP_STANDBT_N TCA9539_P02
#define AUDIO_DIG_AMP_MUTE_N TCA9539_P03
#define AUDIO_DIG_AMP_FAULT_N TCA9539_P04
#define AUDIO_DIG_AMP_WARN_N TCA9539_P05
#endif

const struct au_codec_dev_ctrl_interface *
sdrv_tas6424_get_controller_interface(void) {
    return &tas6424_drv;
}
/**
 * @brief
 *
 * @param codec_id
 * @return struct audio_codec_dev*
 */
struct audio_codec_dev *sdrv_tas6424_get_dev(int codec_id) {
    return &g_tas6424_instance[codec_id - 1];
}
/**
 * @brief initialize tas6424
 *
 * @param dev codec device
 *
 * @return \b bool initialize result.
 */
static bool sdrv_tas6424_initialize(struct audio_codec_dev *dev) {
    /* inactive and power down */
    int ret = 0;
#if FAST_AUDIO_CFG0 == 1
    struct tca6408_device *pd;
    pd = tca6408_init(6, 0x20);

    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return false;
    }

    tca6408_output_enable(pd, AUDIO_DIG_AMP_STANDBT_N);
    tca6408_output_val(pd, AUDIO_DIG_AMP_STANDBT_N, 0);
    tca6408_output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
    tca6408_output_val(pd, AUDIO_DIG_AMP_MUTE_N, 0);
    tca6408_deinit(pd);
#else
    struct tca9539_device *pd;
    /*IO Expand address is I2C 12, 0x74 */
    pd = tca9539_init(IO_EXPANDER_I2C_BUS, IO_EXPANDER_I2C_ADDR);
    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return false;
    }
    tca9539_enable_i2cpoll(pd);
    pd->ops.output_enable(pd, AUDIO_DIG_AMP_STANDBT_N);
    pd->ops.output_val(pd, AUDIO_DIG_AMP_STANDBT_N, 0);
    pd->ops.output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
    pd->ops.output_val(pd, AUDIO_DIG_AMP_MUTE_N, 0);
    tca9539_deinit(pd);
#endif
    /* Reset device to establish well-defined startup state */
    ret = tas6424_i2c_write_reg(*dev, TAS6424_MODE_CTRL, TAS6424_RESET);
    if (ret < 0) {
        return false;
    }

    dprintf(AUDIO_CODEC_DEBUG, "sdrv_tas6424_initialize and read.\n");

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
static bool sdrv_tas6424_start_up(struct audio_codec_dev *dev) {
#if FAST_AUDIO_CFG0 == 1
    struct tca6408_device *pd;
    pd = tca6408_init(6, 0x20);

    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return false;
    }

    tca6408_output_enable(pd, AUDIO_DIG_AMP_STANDBT_N);
    tca6408_output_val(pd, AUDIO_DIG_AMP_STANDBT_N, 1);
    tca6408_output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
    tca6408_output_val(pd, AUDIO_DIG_AMP_MUTE_N, 1);
    tca6408_deinit(pd);
#else
    struct tca9539_device *pd;

    pd = tca9539_init(IO_EXPANDER_I2C_BUS, IO_EXPANDER_I2C_ADDR);
    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return false;
    }
    tca9539_enable_i2cpoll(pd);
    pd->ops.output_enable(pd, AUDIO_DIG_AMP_STANDBT_N);
    pd->ops.output_val(pd, AUDIO_DIG_AMP_STANDBT_N, 1);
    pd->ops.output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
    pd->ops.output_val(pd, AUDIO_DIG_AMP_MUTE_N, 1);
    tca9539_deinit(pd);
#endif
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
static bool sdrv_tas6424_set_volume(struct audio_codec_dev *dev,
                                    int volume_percent,
                                    enum audio_volume_type vol_type) {
    int ret = 0;
    int vol = volume_percent * 255 / 100;
    if (vol > 255) {
        dprintf(AUDIO_CODEC_DEBUG, "sdrv_tas6424_initialize volume_percent is "
                                   "bigger than 255 set to 0dB.\n");
        vol = 0xCF;
    }
    if ((vol_type == AUDIO_VOL_INPUT) || (vol_type == AUDIO_VOL_HEADSET_MIC)) {
        return true;
    }
    /* Don't care audio path*/
    dev->hphone_out_vol = volume_percent;
    dev->line_in_vol = volume_percent;
    /* set amp output volume, channel 1~4 is same.  */
    ret = tas6424_i2c_write_reg(*dev, TAS6424_CH1_VOL_CTRL, vol);
    if (ret < 0) {
        return false;
    }
    ret = tas6424_i2c_write_reg(*dev, TAS6424_CH2_VOL_CTRL, vol);
    if (ret < 0) {
        return false;
    }
    ret = tas6424_i2c_write_reg(*dev, TAS6424_CH3_VOL_CTRL, vol);
    if (ret < 0) {
        return false;
    }
    ret = tas6424_i2c_write_reg(*dev, TAS6424_CH4_VOL_CTRL, vol);
    if (ret < 0) {
        return false;
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
static bool sdrv_tas6424_set_format(struct audio_codec_dev dev,
                                    pcm_params_t pcm_info) {
    int ret = 0;
    uint32_t value = 0;
    uint32_t interface = 0;

    /* set i2s interface mode */
    switch (pcm_info.standard) {
    case SD_AUDIO_I2S_STANDARD_PHILLIPS:
        interface |= TAS6424_SAP_I2S;
        break;

    case SD_AUDIO_I2S_LEFT_JUSTIFIED:
        interface |= TAS6424_SAP_LEFTJ;
        break;

    case SD_AUDIO_I2S_RIGHT_JUSTIFIED:
        printf("func<%s>: unsupported i2s right justified mode.\n", __func__);
        break;

    case SD_AUDIO_I2S_DSP_A:
        interface |= TAS6424_SAP_DSP;
        break;

    case SD_AUDIO_I2S_DSP_B:
        /*
         * We can use the fact that the TAS6424 does not care about the
         * LRCLK duty cycle during TDM to receive DSP_B formatted data
         * in LEFTJ mode (no delaying of the 1st data bit).
         */
        interface |= TAS6424_SAP_LEFTJ;
        break;

    default:
        dprintf(AUDIO_CODEC_DEBUG,
                "error: codec interface mode wrong arg or no init.\n");
    }

    /* set master/slave */
    if ((pcm_info.mode & SD_AUDIO_TRANSFER_MODE_ENABLE) ==
        SD_AUDIO_TRANSFER_CODEC_MASTER) {
        dprintf(AUDIO_CODEC_DEBUG, "error: tas6424 cannot be set to master.\n");
        return false;
    }

    interface &= TAS6424_SAP_FMT_MASK;
    value = tas6424_i2c_read_reg(dev, TAS6424_SAP_CTRL);
    value = (value & (uint32_t)(~TAS6424_SAP_FMT_MASK));
    interface = interface | value;
    dprintf(AUDIO_CODEC_DEBUG, "interface 0x%x .\n", interface);
    ret = tas6424_i2c_write_reg(dev, TAS6424_SAP_CTRL, interface);
    if (ret < 0) {
        return false;
    }
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
static bool sdrv_tas6424_set_hw_params(struct audio_codec_dev dev,
                                       pcm_params_t pcm_info) {
    int ret = 0;
    uint32_t sample_rate = 0;
    /* set sample rate */
    sample_rate = pcm_info.sample_rate;
    /*     uint32_t val = 0; */
    uint8_t sap_ctrl = 0;
    uint8_t sap_mask = TAS6424_SAP_RATE_MASK | TAS6424_SAP_TDM_SLOT_SZ_16;
    dprintf(AUDIO_CODEC_DEBUG, "sample rate: %d.\n", sample_rate);
    switch (sample_rate) {

    case SD_AUDIO_SR_44100:
        sap_ctrl |= TAS6424_SAP_RATE_44100;
        break;

    case SD_AUDIO_SR_48000:
        sap_ctrl |= TAS6424_SAP_RATE_48000;
        break;

    case SD_AUDIO_SR_96000:
        sap_ctrl |= TAS6424_SAP_RATE_96000;

        break;

    default:
        dprintf(AUDIO_CODEC_DEBUG, "unsupport sample rate: %d.\n", sample_rate);
    }

    /* set pcm data slot width */
    /*     format = codec_read_reg(dev.id, TLV320AIC23_DIGT_FMT); */

    switch (pcm_info.slot_width) {
    case SD_AUDIO_SLOT_WIDTH_16BITS:
        sap_ctrl |= TAS6424_SAP_TDM_SLOT_SZ_16;
        break;

    case SD_AUDIO_SLOT_WIDTH_24BITS:

        break;

    case SD_AUDIO_SLOT_WIDTH_32BITS:
        break;

    default:
        dprintf(AUDIO_CODEC_DEBUG, "codec slot width wrong arg or no init.\n");
    }

    sap_ctrl = (tas6424_i2c_read_reg(dev, TAS6424_SAP_CTRL) & (~sap_mask)) |
               (sap_ctrl & sap_mask);
    dprintf(AUDIO_CODEC_DEBUG, "sap_ctrl 0x%x .\n", sap_ctrl);
    ret = tas6424_i2c_write_reg(dev, TAS6424_SAP_CTRL, sap_ctrl);
    if (ret < 0) {
        return false;
    }
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
static bool sdrv_tas6424_trigger(struct audio_codec_dev dev, int cmd) {
    uint8_t chan_states = TAS6424_ALL_STATE_MUTE;
    int ret = 0;
#if FAST_AUDIO_CFG0 == 1
    struct tca6408_device *pd;
    pd = tca6408_init(6, 0x20);

    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return false;
    }
    dprintf(AUDIO_CODEC_DEBUG, "sdrv_tas6424_trigger cmd %d .\n", cmd);
    /* handle command */
    if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_START) {
        tca6408_output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
        tca6408_output_val(pd, AUDIO_DIG_AMP_MUTE_N, 1);
        chan_states = TAS6424_ALL_STATE_PLAY;
    } else if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP) {
        tca6408_output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
        tca6408_output_val(pd, AUDIO_DIG_AMP_MUTE_N, 0);
        chan_states = TAS6424_ALL_STATE_MUTE;
    }
    tca6408_deinit(pd);
#else
    struct tca9539_device *pd;


    pd = tca9539_init(IO_EXPANDER_I2C_BUS, IO_EXPANDER_I2C_ADDR);
    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return false;
    }
    tca9539_enable_i2cpoll(pd);
    dprintf(AUDIO_CODEC_DEBUG, "sdrv_tas6424_trigger cmd %d .\n", cmd);
    /* handle command */
    if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_START) {
        pd->ops.output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
        pd->ops.output_val(pd, AUDIO_DIG_AMP_MUTE_N, 1);
        chan_states = TAS6424_ALL_STATE_PLAY;
    } else if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP) {
        pd->ops.output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
        pd->ops.output_val(pd, AUDIO_DIG_AMP_MUTE_N, 0);
        chan_states = TAS6424_ALL_STATE_MUTE;
    }
    tca9539_deinit(pd);
#endif
    ret = tas6424_i2c_write_reg(dev, TAS6424_CH_STATE_CTRL, chan_states);
    if (ret < 0) {
        return false;
    }

    /* tas6424_i2c_dump_reg(dev); */
    return true;
}

/**
 * @brief codec power off
 *
 * @param dev codec dev
 *
 * @return \b true
 */
static bool sdrv_tas6424_shutdown(struct audio_codec_dev dev) {
#if FAST_AUDIO_CFG0 == 1
    struct tca6408_device *pd;
    pd = tca6408_init(6, 0x20);
    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return false;
    }
    tca6408_output_enable(pd, AUDIO_DIG_AMP_STANDBT_N);
    tca6408_output_val(pd, AUDIO_DIG_AMP_STANDBT_N, 1);
    tca6408_output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
    tca6408_output_val(pd, AUDIO_DIG_AMP_MUTE_N, 1);
    tca6408_deinit(pd);
#else
    struct tca9539_device *pd;

    pd = tca9539_init(IO_EXPANDER_I2C_BUS, IO_EXPANDER_I2C_ADDR);
    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return false;
    }
    tca9539_enable_i2cpoll(pd);
    pd->ops.output_enable(pd, AUDIO_DIG_AMP_STANDBT_N);
    pd->ops.output_val(pd, AUDIO_DIG_AMP_STANDBT_N, 0);
    pd->ops.output_enable(pd, AUDIO_DIG_AMP_MUTE_N);
    pd->ops.output_val(pd, AUDIO_DIG_AMP_MUTE_N, 0);
    tca9539_deinit(pd);
#endif
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
static bool sdrv_tas6424_set_input_path(struct audio_codec_dev *dev,
                                        uint32_t input_path) {
    if (input_path == AUDIO_CODEC_SET_INPUT_AS_MIC_IN) {
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
static bool sdrv_tas6424_set_output_path(struct audio_codec_dev *dev,
                                         uint32_t output_path) {
    dev->output_path = output_path;
    return true;
}
