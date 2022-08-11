#include "tlv320aic23.h"
#include <stdlib.h>
#include <lib/reg.h>
#include "i2c_hal.h"


static void* i2c_handle;

static int tlv320_i2c_write_reg(u32 reg, u32 val)
{
    u8 addr, buf;
    int ret;
    addr = (reg << 1) | (val >> 8 & 0x01);
    buf = (u8)(val & 0xFF);
    // buf[1] = val >> 8;
    ret = hal_i2c_write_reg_data(i2c_handle, TLV320AIC23_I2C_ADDR, &addr, 1,
                                 &buf, 1);
    dprintf(INFO, "%s(), ret=%d, addr=0x%x buf=0x%x\n", __func__, ret, addr,
            buf);
    if (ret < 0) {
        dprintf(CRITICAL, "%s: error: reg=%x, val=%x\n", __func__, reg, val);
    }
    return ret;
}

void tlv320_config_bypass(bool flag)
{
    tlv320_i2c_write_reg(TLV320AIC23_PWR, TLV320AIC23_DEVICE_PWR_OFF);
    /*0x1*/
    if (true == flag) {
        tlv320_i2c_write_reg(TLV320AIC23_ANLG, TLV320AIC23_BYPASS_ON);
    } else {
        tlv320_i2c_write_reg(TLV320AIC23_ANLG, ~TLV320AIC23_BYPASS_ON);
    }
    tlv320_i2c_write_reg(TLV320AIC23_ACTIVE, 0x0001);
    /* Power on the device 22*/
    tlv320_i2c_write_reg(TLV320AIC23_PWR, 0x11);
}

void enable_tlv320_codec_record(void)
{
    u16 digital_interface_format = 0, power_contrl = 0;
    // 1,power off;
    tlv320_i2c_write_reg(TLV320AIC23_RESET, 0x00);
    tlv320_i2c_write_reg(TLV320AIC23_PWR, TLV320AIC23_DEVICE_PWR_OFF);
    /* Left Line Input Channel Volume Control 0b100010111  0x17*/
    tlv320_i2c_write_reg(TLV320AIC23_LINVOL,
                         TLV320AIC23_LRS_ENABLED | TLV320AIC23_LIV_DEFAULT);
    /* Right Line Input Channel Volume Control 0b100010111 0x17 */
    tlv320_i2c_write_reg(TLV320AIC23_RINVOL,
                         TLV320AIC23_LRS_ENABLED | TLV320AIC23_LIV_DEFAULT);
    tlv320_i2c_write_reg(TLV320AIC23_LCHNVOL, TLV320AIC23_LRS_ENABLED);
    tlv320_i2c_write_reg(TLV320AIC23_RCHNVOL, TLV320AIC23_LRS_ENABLED);
    // open adc
    tlv320_i2c_write_reg(TLV320AIC23_ANLG, 0x0);
    // set de-emphasis, any need to de-emphasis?
    tlv320_i2c_write_reg(TLV320AIC23_DIGT, 0x0);
    // set digital format
    digital_interface_format = TLV320AIC23_IWL_16;
    digital_interface_format &= ~TLV320AIC23_MS_MASTER;
    digital_interface_format |= TLV320AIC23_FOR_I2S;
    tlv320_i2c_write_reg(TLV320AIC23_DIGT_FMT, digital_interface_format);
    // set sample rate: dac/adc 48k, normal mode.
    tlv320_i2c_write_reg(TLV320AIC23_SRATE, 0);
    // active and power on
    tlv320_i2c_write_reg(TLV320AIC23_ACTIVE, TLV320AIC23_ACT_ON);
    // power_contrl = ~TLV320AIC23_ADC_OFF;
    // power_contrl &= ~TLV320AIC23_LINE_OFF;
    tlv320_i2c_write_reg(TLV320AIC23_PWR, 0x7a); // TODO: line input & ADC on
}

void enable_tlv320_codec_play(int digital_interface_flag, uint32_t i2c_res)
{
    u16 ifmt_reg = 0;
    hal_i2c_creat_handle(&i2c_handle, i2c_res);
    // reset
    tlv320_i2c_write_reg(TLV320AIC23_RESET, 0x00);
    /* Clear data 0x80*/
    tlv320_i2c_write_reg(TLV320AIC23_PWR, TLV320AIC23_DEVICE_PWR_OFF);
    //
    /* Left Line Input Channel Volume Control 0b100010111  0x17*/
    tlv320_i2c_write_reg(TLV320AIC23_LINVOL, TLV320AIC23_LRS_ENABLED);
    /* Right Line Input Channel Volume Control 0b100010111 0x17 */
    tlv320_i2c_write_reg(TLV320AIC23_RINVOL, TLV320AIC23_LRS_ENABLED);
    tlv320_i2c_write_reg(TLV320AIC23_LCHNVOL,
                         TLV320AIC23_LRS_ENABLED | TLV320AIC23_LHV_MID);
    tlv320_i2c_write_reg(TLV320AIC23_RCHNVOL,
                         TLV320AIC23_LRS_ENABLED | TLV320AIC23_LHV_MID);
    /* 0x10 TLV320AIC23_STA_REG(4) | */
    tlv320_i2c_write_reg(TLV320AIC23_ANLG, TLV320AIC23_DAC_SELECTED);
    /* Digital Audio Path Functions disabled00 */
    tlv320_i2c_write_reg(TLV320AIC23_DIGT, 0x0);
    /* Set format 07 E*/
    ifmt_reg = TLV320AIC23_IWL_16;
    ifmt_reg &= ~TLV320AIC23_MS_MASTER;
    if (digital_interface_flag == STANDARD_I2S_MODE) {
        ifmt_reg |= TLV320AIC23_FOR_I2S;
    } else if (digital_interface_flag == I2S_DSP_MODE) {
        ifmt_reg |= TLV320AIC23_FOR_DSP;
        ifmt_reg |= TLV320AIC23_LRP_ON;
    }
    tlv320_i2c_write_reg(TLV320AIC23_DIGT_FMT, ifmt_reg);
    /* Sample Rate Control MCLK = 12.288*2   48k tx/rx 0x40 */
    tlv320_i2c_write_reg(TLV320AIC23_SRATE, 0);
    /*0x1*/
    tlv320_i2c_write_reg(TLV320AIC23_ACTIVE, 0x0001);
    /* Power on the device 22*/
    tlv320_i2c_write_reg(TLV320AIC23_PWR, 0x67);
}

void disable_tlv320_codec(void)
{
    /* Clear data 0x80*/
    tlv320_i2c_write_reg(TLV320AIC23_ACTIVE, 0x0);
    tlv320_i2c_write_reg(TLV320AIC23_PWR, TLV320AIC23_DEVICE_PWR_OFF);
    hal_i2c_release_handle(i2c_handle);
}