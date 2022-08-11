#ifndef _TLV320AIC23_H
#define _TLV320AIC23_H

#define STANDARD_I2S_MODE 0
#define I2S_DSP_MODE 1

#define TLV320AIC23_I2C_ADDR 0x1a
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
#define TLV320AIC23_LHV_MID 0x0058
#define TLV320AIC23_LHV_MIN 0x0000

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
#define TLV320AIC23_DEVICE_PWR_OFF 0x80
#define TLV320AIC23_CLK_OFF 0x40
#define TLV320AIC23_OSC_OFF 0x20
#define TLV320AIC23_OUT_OFF 0x10
#define TLV320AIC23_DAC_OFF 0x08
#define TLV320AIC23_ADC_OFF 0x04
#define TLV320AIC23_MIC_OFF 0x02
#define TLV320AIC23_LINE_OFF 0x01

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
#endif