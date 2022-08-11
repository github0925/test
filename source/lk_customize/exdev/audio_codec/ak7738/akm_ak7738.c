/*
 * tlv320.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 *
 */

#include "akm_ak7738.h"
#include "sd_audio.h"
#include "chip_res.h"
#include "i2c_hal.h"
#include "tca9539.h"
#include "tca6408.h"
#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)
#ifndef GEN_MASK
#define GEN_MASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32 - 1 - (h))))
#endif
#ifndef BIT
#define BIT(x) (1 << (x))
#endif
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define AK7738_INSTANCE_I2C_1C 0x1c
#define AK7738_MAX_DEV_NUM

/* Codec AK7738 */

#define AK7738_00_STSCLOCK_SETTING1 0x00
#define AK7738_01_STSCLOCK_SETTING2 0x01
#define AK7738_02_MICBIAS_PLOWER 0x02
#define AK7738_03_SYNCDOMAIN1_SETTING1 0x03
#define AK7738_04_SYNCDOMAIN1_SETTING2 0x04
#define AK7738_05_SYNCDOMAIN2_SETTING1 0x05
#define AK7738_06_SYNCDOMAIN2_SETTING2 0x06
#define AK7738_07_SYNCDOMAIN3_SETTING1 0x07
#define AK7738_08_SYNCDOMAIN3_SETTING2 0x08
#define AK7738_09_SYNCDOMAIN4_SETTING1 0x09
#define AK7738_0A_SYNCDOMAIN4_SETTING2 0x0A
#define AK7738_0B_SYNCDOMAIN5_SETTING1 0x0B
#define AK7738_0C_SYNCDOMAIN5_SETTING2 0x0C
#define AK7738_0D_SYNCDOMAIN6_SETTING1 0x0D
#define AK7738_0E_SYNCDOMAIN6_SETTING2 0x0E
#define AK7738_0E_SYNCDOMAIN7_SETTING1 0x0F
#define AK7738_10_SYNCDOMAIN7_SETTING2 0x10
#define AK7738_11_BUS_DSP_CLOCK_SETTING 0x11
#define AK7738_12_BUS_CLOCK_SETTING2 0x12
#define AK7738_13_CLOKO_OUTPUT_SETTING 0x13
#define AK7738_14_RESERVED 0x14
#define AK7738_15_SYNCDOMAIN_SELECT1 0x15
#define AK7738_16_SYNCDOMAIN_SELECT2 0x16
#define AK7738_17_SYNCDOMAIN_SELECT3 0x17
#define AK7738_18_SYNCDOMAIN_SELECT4 0x18
#define AK7738_19_SYNCDOMAIN_SELECT5 0x19
#define AK7738_1A_SYNCDOMAIN_SELECT6 0x1A
#define AK7738_1B_SYNCDOMAIN_SELECT7 0x1B
#define AK7738_1C_SYNCDOMAIN_SELECT8 0x1C
#define AK7738_1D_SYNCDOMAIN_SELECT9 0x1D
#define AK7738_1E_SYNCDOMAIN_SELECT10 0x1E
#define AK7738_1F_SYNCDOMAIN_SELECT11 0x1F
#define AK7738_20_SYNCDOMAIN_SELECT12 0x20
#define AK7738_21_SYNCDOMAIN_SELECT13 0x21
#define AK7738_22_RESERVED 0x22
#define AK7738_23_SDOUT1_DATA_SELECT 0x23
#define AK7738_24_SDOUT2_DATA_SELECT 0x24
#define AK7738_25_SDOUT3_DATA_SELECT 0x25
#define AK7738_26_SDOUT4_DATA_SELECT 0x26
#define AK7738_27_SDOUT5_DATA_SELECT 0x27
#define AK7738_28_SDOUT6_DATA_SELECT 0x28
#define AK7738_29_DAC1_DATA_SELECT 0x29
#define AK7738_2A_DAC2_DATA_SELECT 0x2A
#define AK7738_2B_DSP1_IN1_DATA_SELECT 0x2B
#define AK7738_2C_DSP1_IN2_DATA_SELECT 0x2C
#define AK7738_2D_DSP1_IN3_DATA_SELECT 0x2D
#define AK7738_2E_DSP1_IN4_DATA_SELECT 0x2E
#define AK7738_2F_DSP1_IN5_DATA_SELECT 0x2F
#define AK7738_30_DSP1_IN6_DATA_SELECT 0x30
#define AK7738_31_DSP2_IN1_DATA_SELECT 0x31
#define AK7738_32_DSP2_IN2_DATA_SELECT 0x32
#define AK7738_33_DSP2_IN3_DATA_SELECT 0x33
#define AK7738_34_DSP2_IN4_DATA_SELECT 0x34
#define AK7738_35_DSP2_IN5_DATA_SELECT 0x35
#define AK7738_36_DSP2_IN6_DATA_SELECT 0x36
#define AK7738_37_SRC1_DATA_SELECT 0x37
#define AK7738_38_SRC2_DATA_SELECT 0x38
#define AK7738_39_SRC3_DATA_SELECT 0x39
#define AK7738_3A_SRC4_DATA_SELECT 0x3A
#define AK7738_3B_FSCONV1_DATA_SELECT 0x3B
#define AK7738_3C_FSCONV2_DATA_SELECT 0x3C
#define AK7738_3D_MIXERA_CH1_DATA_SELECT 0x3D
#define AK7738_3E_MIXERA_CH2_DATA_SELECT 0x3E
#define AK7738_3F_MIXERB_CH1_DATA_SELECT 0x3F
#define AK7738_40_MIXERB_CH2_DATA_SELECT 0x40
#define AK7738_41_DIT_DATA_SELECT 0x41
#define AK7738_42_RESERVED 0x42
#define AK7738_43_RESERVED 0x43
#define AK7738_44_CLOCKFORMAT_SETTING1 0x44
#define AK7738_45_CLOCKFORMAT_SETTING2 0x45
#define AK7738_46_CLOCKFORMAT_SETTING3 0x46
#define AK7738_47_RESERVED 0x47
#define AK7738_48_SDIN1_FORMAT 0x48
#define AK7738_49_SDIN2_FORMAT 0x49
#define AK7738_4A_SDIN3_FORMAT 0x4A
#define AK7738_4B_SDIN4_FORMAT 0x4B
#define AK7738_4C_SDIN5_FORMAT 0x4C
#define AK7738_4D_SDIN6_FORMAT 0x4D
#define AK7738_4E_SDOUT1_FORMAT 0x4E
#define AK7738_4F_SDOUT2_FORMAT 0x4F
#define AK7738_50_SDOUT3_FORMAT 0x50
#define AK7738_51_SDOUT4_FORMAT 0x51
#define AK7738_52_SDOUT5_FORMAT 0x52
#define AK7738_53_SDOUT6_FORMAT 0x53
#define AK7738_54_SDOUT_MODE_SETTING 0x54
#define AK7738_55_TDM_MODE_SETTING 0x55
#define AK7738_56_RESERVED 0x56
#define AK7738_57_OUTPUT_PORT_SELECT 0x57
#define AK7738_58_OUTPUT_PORT_ENABLE 0x58
#define AK7738_59_INPUT_PORT_SELECT 0x59
#define AK7738_5A_RESERVED 0x5A
#define AK7738_5B_MIXER_A_SETTING 0x5B
#define AK7738_5C_MIXER_B_SETTING 0x5C
#define AK7738_5D_MICAMP_GAIN 0x5D
#define AK7738_5E_MICAMP_GAIN_CONTROL 0x5E
#define AK7738_5F_ADC1_LCH_VOLUME 0x5F
#define AK7738_60_ADC1_RCH_VOLUME 0x60
#define AK7738_61_ADC2_LCH_VOLUME 0x61
#define AK7738_62_ADC2_RCH_VOLUME 0x62
#define AK7738_63_ADCM_VOLUME 0x63

#define AK7738_64_RESERVED 0x64
#define AK7738_65_RESERVED 0x65
#define AK7738_66_AIN_FILTER 0x66
#define AK7738_67_ADC_MUTE_HPF 0x67
#define AK7738_68_DAC1_LCH_VOLUME 0x68
#define AK7738_69_DAC1_RCH_VOLUME 0x69
#define AK7738_6A_DAC2_LCH_VOLUME 0x6A
#define AK7738_6B_DAC2_RCH_VOLUME 0x6B

#define AK7738_6C_RESERVED 0x6C
#define AK7738_6D_RESERVED 0x6D
#define AK7738_6E_DAC_MUTE_FILTER 0x6E
#define AK7738_6F_SRC_CLOCK_SETTING 0x6F
#define AK7738_70_SRC_MUTE_SETTING 0x70
#define AK7738_71_FSCONV_MUTE_SETTING 0x71
#define AK7738_72_RESERVED 0x72
#define AK7738_73_DSP_MEMORY_ASSIGNMENT1 0x73
#define AK7738_74_DSP_MEMORY_ASSIGNMENT2 0x74
#define AK7738_75_DSP12_DRAM_SETTING 0x75
#define AK7738_76_DSP1_DLRAM_SETTING 0x76
#define AK7738_77_DSP2_DLRAM_SETTING 0x77
#define AK7738_78_FFT_DLP0_SETTING 0x78
#define AK7738_79_RESERVED 0x79
#define AK7738_7A_JX_SETTING 0x7A
#define AK7738_7B_STO_FLAG_SETTING1 0x7B
#define AK7738_7C_STO_FLAG_SETTING2 0x7C
#define AK7738_7D_RESERVED 0x7D
#define AK7738_7E_DIT_STATUS_BIT1 0x7E
#define AK7738_7F_DIT_STATUS_BIT2 0x7F
#define AK7738_80_DIT_STATUS_BIT3 0x80
#define AK7738_81_DIT_STATUS_BIT4 0x81
#define AK7738_82_RESERVED 0x82
#define AK7738_83_POWER_MANAGEMENT1 0x83
#define AK7738_84_POWER_MANAGEMENT2 0x84
#define AK7738_85_RESET_CTRL 0x85
#define AK7738_86_RESERVED 0x86
#define AK7738_87_RESERVED 0x87
#define AK7738_90_RESERVED 0x90
#define AK7738_91_PAD_DRIVE_SEL2 0x91
#define AK7738_92_PAD_DRIVE_SEL3 0x92
#define AK7738_93_PAD_DRIVE_SEL4 0x93
#define AK7738_94_PAD_DRIVE_SEL5 0x94
#define AK7738_95_RESERVED 0x95
#define AK7738_100_DEVICE_ID 0x100
#define AK7738_101_REVISION_NUM 0x101
#define AK7738_102_DSP_ERROR_STATUS 0x102
#define AK7738_103_SRC_STATUS 0x103
#define AK7738_104_STO_READ_OUT 0x104
#define AK7738_105_MICGAIN_READ 0x105
#define AK7738_VIRT_106_DSP1OUT1_MIX 0x106
#define AK7738_VIRT_107_DSP1OUT2_MIX 0x107
#define AK7738_VIRT_108_DSP1OUT3_MIX 0x108
#define AK7738_VIRT_109_DSP1OUT4_MIX 0x109
#define AK7738_VIRT_10A_DSP1OUT5_MIX 0x10A
#define AK7738_VIRT_10B_DSP1OUT6_MIX 0x10B
#define AK7738_VIRT_10C_DSP2OUT1_MIX 0x10C
#define AK7738_VIRT_10D_DSP2OUT2_MIX 0x10D
#define AK7738_VIRT_10E_DSP2OUT3_MIX 0x10E
#define AK7738_VIRT_10F_DSP2OUT4_MIX 0x10F
#define AK7738_VIRT_110_DSP2OUT5_MIX 0x110
#define AK7738_VIRT_111_DSP2OUT6_MIX 0x111

#define AUDIO_CODEC_DEBUG 2
#define s_REG_MAX_ADDR (AK7738_VIRT_111_DSP2OUT6_MIX + 1)

/* common definition */
#define MAX_LOOP_TIMES 3

#define COMMAND_WRITE_REG 0xC0
#define COMMAND_READ_REG 0x40

#define COMMAND_CRC_READ 0x72
#define COMMAND_MIR1_READ 0x76
#define COMMAND_MIR2_READ 0x77

#define COMMAND_WRITE_CRAM_RUN 0x80
#define COMMAND_WRITE_CRAM_EXEC 0xA4

#define COMMAND_WRITE_CRAM 0xB4
#define COMMAND_WRITE_PRAM 0xB8
#define COMMAND_WRITE_OFREG 0xB2

#define COMMAND_WRITE_CRAM_SUB 0xB6
#define COMMAND_WRITE_PRAM_SUB 0xBA

#define TOTAL_NUM_OF_PRAM_MAX 40963
#define TOTAL_NUM_OF_CRAM_MAX 18435
#define TOTAL_NUM_OF_OFREG_MAX 195
#define TOTAL_NUM_OF_SUB_PRAM_MAX 5123
#define TOTAL_NUM_OF_SUB_CRAM_MAX 6147

/* functions declaration */
static bool sdrv_ak7738_initialize(struct audio_codec_dev *dev);
static bool sdrv_ak7738_start_up(struct audio_codec_dev *dev);
static bool sdrv_ak7738_set_volume(struct audio_codec_dev *dev,
                                   int volume_percent,
                                   enum audio_volume_type vol_type);
static bool sdrv_ak7738_set_format(struct audio_codec_dev dev,
                                   pcm_params_t pcm_info);
static bool sdrv_ak7738_set_hw_params(struct audio_codec_dev dev,
                                      pcm_params_t pcm_info);
static bool sdrv_ak7738_trigger(struct audio_codec_dev dev, int cmd);
static bool sdrv_ak7738_shutdown(struct audio_codec_dev dev);
static bool sdrv_ak7738_set_input_path(struct audio_codec_dev *dev,
                                       uint32_t input_path);
static bool sdrv_ak7738_set_output_path(struct audio_codec_dev *dev,
                                        uint32_t output_path);

static struct audio_codec_dev g_ak7738_instance[AK7738_MAX_DEV_NUM] = {
    {1, AK7738_INSTANCE_I2C_1C, "safety", RES_I2C_I2C6, NULL,
     AUDIO_CODEC_SET_INPUT_AS_DEFAULT, AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT, 50,
     100},
    {2, AK7738_INSTANCE_I2C_1C, "safety", RES_I2C_I2C6, NULL,
     AUDIO_CODEC_SET_INPUT_AS_DEFAULT, AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT, 50,
     100},
    {3, AK7738_INSTANCE_I2C_1C, "security", RES_I2C_I2C6, NULL,
     AUDIO_CODEC_SET_INPUT_AS_DEFAULT, AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT, 50,
     100},
};
struct reg_default {
    unsigned int reg;
    unsigned int def;
};
/* static const struct reg_default ak7738_reg[] = {
    {0x0, 0xf},	   {0x1, 0x87},	 {0x2, 0x0},	{0x3, 0x0},    {0x4, 0x27},
    {0x5, 0x90},   {0x6, 0x27},	 {0x7, 0x12},	{0x8, 0x4f},   {0x9, 0x94},
    {0xa, 0x9},	   {0xb, 0x94},	 {0xc, 0x9},	{0xd, 0x0},    {0xe, 0x27},
    {0xf, 0x0},	   {0x10, 0x27}, {0x11, 0x10},	{0x12, 0x4},   {0x13, 0x0},
    {0x14, 0x0},   {0x15, 0x12}, {0x16, 0x34},	{0x17, 0x22},  {0x18, 0x42},
    {0x19, 0x2},   {0x1a, 0x2},	 {0x1b, 0x34},	{0x1c, 0x0},   {0x1d, 0x20},
    {0x1e, 0x0},   {0x1f, 0x30}, {0x20, 0x0},	{0x21, 0x22},  {0x22, 0x0},
    {0x23, 0x0},   {0x24, 0xd},	 {0x25, 0x22},	{0x26, 0x0},   {0x27, 0x2},
    {0x28, 0x0},   {0x29, 0xf},	 {0x2a, 0x10},	{0x2b, 0x1e},  {0x2c, 0x0},
    {0x2d, 0x3},   {0x2e, 0x4},	 {0x2f, 0x5},	{0x30, 0x0},   {0x31, 0x7},
    {0x32, 0x8},   {0x33, 0x9},	 {0x34, 0xa},	{0x35, 0x19},  {0x36, 0x1a},
    {0x37, 0x6},   {0x38, 0x0},	 {0x39, 0x0},	{0x3a, 0x0},   {0x3b, 0x14},
    {0x3c, 0x0},   {0x3d, 0x0},	 {0x3e, 0x0},	{0x3f, 0x0},   {0x40, 0x0},
    {0x41, 0x0},   {0x42, 0x0},	 {0x43, 0x0},	{0x44, 0x0},   {0x45, 0x66},
    {0x46, 0x6},   {0x47, 0x0},	 {0x48, 0x0},	{0x49, 0x32},  {0x4a, 0xa0},
    {0x4b, 0xb0},  {0x4c, 0x80}, {0x4d, 0x0},	{0x4e, 0x0},   {0x4f, 0x32},
    {0x50, 0xa0},  {0x51, 0xb0}, {0x52, 0xb0},	{0x53, 0x0},   {0x54, 0x0},
    {0x55, 0x44},  {0x56, 0x0},	 {0x57, 0x20},	{0x58, 0x6},   {0x59, 0x0},
    {0x5a, 0x0},   {0x5b, 0x0},	 {0x5c, 0x0},	{0x5d, 0xff},  {0x5e, 0x0},
    {0x5f, 0x30},  {0x60, 0x30}, {0x61, 0x30},	{0x62, 0x30},  {0x63, 0x30},
    {0x64, 0x0},   {0x65, 0x0},	 {0x66, 0x0},	{0x67, 0x0},   {0x68, 0x4b},
    {0x69, 0x4b},  {0x6a, 0x4b}, {0x6b, 0x4b},	{0x6c, 0x0},   {0x6d, 0x0},
    {0x6e, 0x82},  {0x6f, 0x0},	 {0x70, 0x0},	{0x71, 0x0},   {0x72, 0x0},
    {0x73, 0x0},   {0x74, 0x0},	 {0x75, 0x0},	{0x76, 0x0},   {0x77, 0x0},
    {0x78, 0x0},   {0x79, 0x0},	 {0x7a, 0x0},	{0x7b, 0x0},   {0x7c, 0x0},
    {0x7d, 0x0},   {0x7e, 0x0},	 {0x7f, 0x4},	{0x80, 0x2},   {0x81, 0x0},
    {0x82, 0x0},   {0x83, 0x6},	 {0x84, 0x0},	{0x85, 0x1d},  {0x86, 0x0},
    {0x87, 0x0},   {0x90, 0x0},	 {0x91, 0x0},	{0x92, 0x0},   {0x93, 0x0},
    {0x94, 0x0},   {0x95, 0x0},	 {0x96, 0x0},	{0x97, 0x0},   {0x98, 0x0},
    {0x99, 0x0},   {0x9a, 0x0},	 {0x9b, 0x0},	{0x9c, 0x0},   {0x9d, 0x0},
    {0x9e, 0x0},   {0x9f, 0x0},	 {0xa0, 0x0},	{0xa1, 0x0},   {0xa2, 0x0},
    {0xa3, 0x0},   {0xa4, 0x0},	 {0xa5, 0x0},	{0xa6, 0x0},   {0xa7, 0x0},
    {0xa8, 0x0},   {0xa9, 0x0},	 {0xaa, 0x0},	{0xab, 0x0},   {0xac, 0x0},
    {0xad, 0x0},   {0xae, 0x0},	 {0xaf, 0x0},	{0xb0, 0x0},   {0xb1, 0x0},
    {0xb2, 0x0},   {0xb3, 0x0},	 {0xb4, 0x0},	{0xb5, 0x0},   {0xb6, 0x0},
    {0xb7, 0x0},   {0xb8, 0x0},	 {0xb9, 0x0},	{0xba, 0x0},   {0xbb, 0x0},
    {0xbc, 0x0},   {0xbd, 0x0},	 {0xbe, 0x0},	{0xbf, 0x0},   {0xc0, 0x0},
    {0xc1, 0x0},   {0xc2, 0x0},	 {0xc3, 0x0},	{0xc4, 0x0},   {0xc5, 0x0},
    {0xc6, 0x0},   {0xc7, 0x0},	 {0xc8, 0x0},	{0xc9, 0x0},   {0xca, 0x0},
    {0xcb, 0x0},   {0xcc, 0x0},	 {0xcd, 0x0},	{0xce, 0x0},   {0xcf, 0x0},
    {0xd0, 0x0},   {0xd1, 0x0},	 {0xd2, 0x0},	{0xd3, 0x0},   {0xd4, 0x0},
    {0xd5, 0x0},   {0xd6, 0x0},	 {0xd7, 0x0},	{0xd8, 0x0},   {0xd9, 0x0},
    {0xda, 0x0},   {0xdb, 0x0},	 {0xdc, 0x0},	{0xdd, 0x0},   {0xde, 0x0},
    {0xdf, 0x0},   {0xe0, 0x0},	 {0xe1, 0x0},	{0xe2, 0x0},   {0xe3, 0x0},
    {0xe4, 0x0},   {0xe5, 0x0},	 {0xe6, 0x0},	{0xe7, 0x0},   {0xe8, 0x0},
    {0xe9, 0x0},   {0xea, 0x0},	 {0xeb, 0x0},	{0xec, 0x0},   {0xed, 0x0},
    {0xee, 0x0},   {0xef, 0x0},	 {0xf0, 0x0},	{0xf1, 0x0},   {0xf2, 0x0},
    {0xf3, 0x0},   {0xf4, 0x0},	 {0xf5, 0x0},	{0xf6, 0x0},   {0xf7, 0x0},
    {0xf8, 0x0},   {0xf9, 0x0},	 {0xfa, 0x0},	{0xfb, 0x0},   {0xfc, 0x0},
    {0xfd, 0x0},   {0xfe, 0x0},	 {0xff, 0x0},	{0x100, 0x38}, {0x101, 0x3},
    {0x102, 0x70}, {0x103, 0x0}, {0x104, 0x80}, {0x105, 0xff},

};
static const struct reg_default ak7738_reg[] = {
    {0x0, 0xf},    {0x1, 0x87},  {0x2, 0x0},    {0x3, 0x90},   {0x4, 0x27},
    {0x5, 0x90},   {0x6, 0x27},  {0x7, 0x12},   {0x8, 0x4f},   {0x9, 0x94},
    {0xa, 0x9},    {0xb, 0x94},  {0xc, 0x9},    {0xd, 0x0},    {0xe, 0x27},
    {0xf, 0x0},    {0x10, 0x27}, {0x11, 0x10},  {0x12, 0x4},   {0x13, 0x0},
    {0x14, 0x0},   {0x15, 0x22}, {0x16, 0x34},  {0x17, 0x22},  {0x18, 0x42},
    {0x19, 0x2},   {0x1a, 0x22}, {0x1b, 0x34},  {0x1c, 0x0},   {0x1d, 0x20},
    {0x1e, 0x0},   {0x1f, 0x30}, {0x20, 0x0},   {0x21, 0x22},  {0x22, 0x0},
    {0x23, 0x0},   {0x24, 0xd},  {0x25, 0x22},  {0x26, 0x0},   {0x27, 0x11},
    {0x28, 0x0},   {0x29, 0xf},  {0x2a, 0x10},  {0x2b, 0x1e},  {0x2c, 0x2},
    {0x2d, 0x3},   {0x2e, 0x4},  {0x2f, 0x5},   {0x30, 0x0},   {0x31, 0x7},
    {0x32, 0x8},   {0x33, 0x9},  {0x34, 0xa},   {0x35, 0x19},  {0x36, 0x1a},
    {0x37, 0x6},   {0x38, 0x0},  {0x39, 0x0},   {0x3a, 0x0},   {0x3b, 0x14},
    {0x3c, 0x0},   {0x3d, 0x0},  {0x3e, 0x0},   {0x3f, 0x0},   {0x40, 0x0},
    {0x41, 0x0},   {0x42, 0x0},  {0x43, 0x0},   {0x44, 0x0},   {0x45, 0x66},
    {0x46, 0x6},   {0x47, 0x0},  {0x48, 0x30},  {0x49, 0x32},  {0x4a, 0xa0},
    {0x4b, 0xb0},  {0x4c, 0x80}, {0x4d, 0x0},   {0x4e, 0x30},  {0x4f, 0x32},
    {0x50, 0xa0},  {0x51, 0xb0}, {0x52, 0xb0},  {0x53, 0x0},   {0x54, 0x0},
    {0x55, 0x44},  {0x56, 0x0},  {0x57, 0x20},  {0x58, 0x6},   {0x59, 0x0},
    {0x5a, 0x0},   {0x5b, 0x0},  {0x5c, 0x0},   {0x5d, 0xff},  {0x5e, 0x0},
    {0x5f, 0x30},  {0x60, 0x30}, {0x61, 0x30},  {0x62, 0x30},  {0x63, 0x30},
    {0x64, 0x0},   {0x65, 0x0},  {0x66, 0x0},   {0x67, 0x0},   {0x68, 0x18},
    {0x69, 0x18},  {0x6a, 0x18}, {0x6b, 0x18},  {0x6c, 0x0},   {0x6d, 0x0},
    {0x6e, 0x82},  {0x6f, 0x0},  {0x70, 0x0},   {0x71, 0x0},   {0x72, 0x0},
    {0x73, 0x0},   {0x74, 0x0},  {0x75, 0x0},   {0x76, 0x0},   {0x77, 0x0},
    {0x78, 0x0},   {0x79, 0x0},  {0x7a, 0x0},   {0x7b, 0x0},   {0x7c, 0x0},
    {0x7d, 0x0},   {0x7e, 0x0},  {0x7f, 0x4},   {0x80, 0x2},   {0x81, 0x0},
    {0x82, 0x0},   {0x83, 0x6},  {0x84, 0x0},   {0x85, 0x1d},  {0x86, 0x0},
    {0x87, 0x0},   {0x90, 0x0},  {0x91, 0x0},   {0x92, 0x0},   {0x93, 0x0},
    {0x94, 0x0},   {0x95, 0x0},  {0x96, 0x0},   {0x97, 0x0},   {0x98, 0x0},
    {0x99, 0x0},   {0x9a, 0x0},  {0x9b, 0x0},   {0x9c, 0x0},   {0x9d, 0x0},
    {0x9e, 0x0},   {0x9f, 0x0},  {0xa0, 0x0},   {0xa1, 0x0},   {0xa2, 0x0},
    {0xa3, 0x0},   {0xa4, 0x0},  {0xa5, 0x0},   {0xa6, 0x0},   {0xa7, 0x0},
    {0xa8, 0x0},   {0xa9, 0x0},  {0xaa, 0x0},   {0xab, 0x0},   {0xac, 0x0},
    {0xad, 0x0},   {0xae, 0x0},  {0xaf, 0x0},   {0xb0, 0x0},   {0xb1, 0x0},
    {0xb2, 0x0},   {0xb3, 0x0},  {0xb4, 0x0},   {0xb5, 0x0},   {0xb6, 0x0},
    {0xb7, 0x0},   {0xb8, 0x0},  {0xb9, 0x0},   {0xba, 0x0},   {0xbb, 0x0},
    {0xbc, 0x0},   {0xbd, 0x0},  {0xbe, 0x0},   {0xbf, 0x0},   {0xc0, 0x0},
    {0xc1, 0x0},   {0xc2, 0x0},  {0xc3, 0x0},   {0xc4, 0x0},   {0xc5, 0x0},
    {0xc6, 0x0},   {0xc7, 0x0},  {0xc8, 0x0},   {0xc9, 0x0},   {0xca, 0x0},
    {0xcb, 0x0},   {0xcc, 0x0},  {0xcd, 0x0},   {0xce, 0x0},   {0xcf, 0x0},
    {0xd0, 0x0},   {0xd1, 0x0},  {0xd2, 0x0},   {0xd3, 0x0},   {0xd4, 0x0},
    {0xd5, 0x0},   {0xd6, 0x0},  {0xd7, 0x0},   {0xd8, 0x0},   {0xd9, 0x0},
    {0xda, 0x0},   {0xdb, 0x0},  {0xdc, 0x0},   {0xdd, 0x0},   {0xde, 0x0},
    {0xdf, 0x0},   {0xe0, 0x0},  {0xe1, 0x0},   {0xe2, 0x0},   {0xe3, 0x0},
    {0xe4, 0x0},   {0xe5, 0x0},  {0xe6, 0x0},   {0xe7, 0x0},   {0xe8, 0x0},
    {0xe9, 0x0},   {0xea, 0x0},  {0xeb, 0x0},   {0xec, 0x0},   {0xed, 0x0},
    {0xee, 0x0},   {0xef, 0x0},  {0xf0, 0x0},   {0xf1, 0x0},   {0xf2, 0x0},
    {0xf3, 0x0},   {0xf4, 0x0},  {0xf5, 0x0},   {0xf6, 0x0},   {0xf7, 0x0},
    {0xf8, 0x0},   {0xf9, 0x0},  {0xfa, 0x0},   {0xfb, 0x0},   {0xfc, 0x0},
    {0xfd, 0x0},   {0xfe, 0x0},  {0xff, 0x0},   {0x100, 0x38}, {0x101, 0x3},
    {0x102, 0x70}, {0x103, 0x0}, {0x104, 0x80}, {0x105, 0xff},
};*/
/*Ref A04 */
static const struct reg_default ak7738_reg[] = {
    {0x0, 0xf},    {0x1, 0x87},  {0x2, 0x0},    {0x3, 0x90},   {0x4, 0x27},
    {0x5, 0x94},   {0x6, 0x9},   {0x7, 0x94},   {0x8, 0x9},    {0x9, 0x94},
    {0xa, 0x9},    {0xb, 0x12},  {0xc, 0x4f},   {0xd, 0x0},    {0xe, 0x27},
    {0xf, 0x0},    {0x10, 0x27}, {0x11, 0x10},  {0x12, 0x4},   {0x13, 0x0},
    {0x14, 0x0},   {0x15, 0x42}, {0x16, 0x44},  {0x17, 0x54},  {0x18, 0x24},
    {0x19, 0x4},   {0x1a, 0x42}, {0x1b, 0x34},  {0x1c, 0x50},  {0x1d, 0x40},
    {0x1e, 0x0},   {0x1f, 0x50}, {0x20, 0x0},   {0x21, 0x44},  {0x22, 0x0},
    {0x23, 0x0},   {0x24, 0xd},  {0x25, 0x1},   {0x26, 0xd},   {0x27, 0x22},
    {0x28, 0x0},   {0x29, 0x11}, {0x2a, 0x12},  {0x2b, 0x1e},  {0x2c, 0x0},
    {0x2d, 0x8},   {0x2e, 0x9},  {0x2f, 0xa},   {0x30, 0x0},   {0x31, 0x2},
    {0x32, 0x3},   {0x33, 0x4},  {0x34, 0x5},   {0x35, 0x19},  {0x36, 0x1a},
    {0x37, 0xb},   {0x38, 0x0},  {0x39, 0x0},   {0x3a, 0x0},   {0x3b, 0x18},
    {0x3c, 0x0},   {0x3d, 0x0},  {0x3e, 0x0},   {0x3f, 0x0},   {0x40, 0x0},
    {0x41, 0x0},   {0x42, 0x0},  {0x43, 0x0},   {0x44, 0x0},   {0x45, 0x60},
    {0x46, 0x6},   {0x47, 0x0},  {0x48, 0x32},  {0x49, 0x30},  {0x4a, 0x80},
    {0x4b, 0x32},  {0x4c, 0xa0}, {0x4d, 0x0},   {0x4e, 0x32},  {0x4f, 0x30},
    {0x50, 0xb0},  {0x51, 0x32}, {0x52, 0xa0},  {0x53, 0x0},   {0x54, 0x0},
    {0x55, 0xc0},  {0x56, 0x0},  {0x57, 0x20},  {0x58, 0xc},   {0x59, 0x0},
    {0x5a, 0x0},   {0x5b, 0x0},  {0x5c, 0x0},   {0x5d, 0xff},  {0x5e, 0x0},
    {0x5f, 0x30},  {0x60, 0x30}, {0x61, 0x30},  {0x62, 0x30},  {0x63, 0x30},
    {0x64, 0x0},   {0x65, 0x0},  {0x66, 0x0},   {0x67, 0x0},   {0x68, 0x4b},
    {0x69, 0x4b},  {0x6a, 0x4b}, {0x6b, 0x4b},  {0x6c, 0x0},   {0x6d, 0x0},
    {0x6e, 0x82},  {0x6f, 0x0},  {0x70, 0x0},   {0x71, 0x0},   {0x72, 0x0},
    {0x73, 0x0},   {0x74, 0x0},  {0x75, 0x0},   {0x76, 0x0},   {0x77, 0x0},
    {0x78, 0x0},   {0x79, 0x0},  {0x7a, 0x0},   {0x7b, 0x0},   {0x7c, 0x0},
    {0x7d, 0x0},   {0x7e, 0x0},  {0x7f, 0x4},   {0x80, 0x2},   {0x81, 0x0},
    {0x82, 0x0},   {0x83, 0x6},  {0x84, 0x0},   {0x85, 0x1d},  {0x86, 0x0},
    {0x87, 0x0},   {0x90, 0x0},  {0x91, 0x0},   {0x92, 0x0},   {0x93, 0x0},
    {0x94, 0x0},   {0x95, 0x0},  {0x96, 0x0},   {0x97, 0x0},   {0x98, 0x0},
    {0x99, 0x0},   {0x9a, 0x0},  {0x9b, 0x0},   {0x9c, 0x0},   {0x9d, 0x0},
    {0x9e, 0x0},   {0x9f, 0x0},  {0xa0, 0x0},   {0xa1, 0x0},   {0xa2, 0x0},
    {0xa3, 0x0},   {0xa4, 0x0},  {0xa5, 0x0},   {0xa6, 0x0},   {0xa7, 0x0},
    {0xa8, 0x0},   {0xa9, 0x0},  {0xaa, 0x0},   {0xab, 0x0},   {0xac, 0x0},
    {0xad, 0x0},   {0xae, 0x0},  {0xaf, 0x0},   {0xb0, 0x0},   {0xb1, 0x0},
    {0xb2, 0x0},   {0xb3, 0x0},  {0xb4, 0x0},   {0xb5, 0x0},   {0xb6, 0x0},
    {0xb7, 0x0},   {0xb8, 0x0},  {0xb9, 0x0},   {0xba, 0x0},   {0xbb, 0x0},
    {0xbc, 0x0},   {0xbd, 0x0},  {0xbe, 0x0},   {0xbf, 0x0},   {0xc0, 0x0},
    {0xc1, 0x0},   {0xc2, 0x0},  {0xc3, 0x0},   {0xc4, 0x0},   {0xc5, 0x0},
    {0xc6, 0x0},   {0xc7, 0x0},  {0xc8, 0x0},   {0xc9, 0x0},   {0xca, 0x0},
    {0xcb, 0x0},   {0xcc, 0x0},  {0xcd, 0x0},   {0xce, 0x0},   {0xcf, 0x0},
    {0xd0, 0x0},   {0xd1, 0x0},  {0xd2, 0x0},   {0xd3, 0x0},   {0xd4, 0x0},
    {0xd5, 0x0},   {0xd6, 0x0},  {0xd7, 0x0},   {0xd8, 0x0},   {0xd9, 0x0},
    {0xda, 0x0},   {0xdb, 0x0},  {0xdc, 0x0},   {0xdd, 0x0},   {0xde, 0x0},
    {0xdf, 0x0},   {0xe0, 0x0},  {0xe1, 0x0},   {0xe2, 0x0},   {0xe3, 0x0},
    {0xe4, 0x0},   {0xe5, 0x0},  {0xe6, 0x0},   {0xe7, 0x0},   {0xe8, 0x0},
    {0xe9, 0x0},   {0xea, 0x0},  {0xeb, 0x0},   {0xec, 0x0},   {0xed, 0x0},
    {0xee, 0x0},   {0xef, 0x0},  {0xf0, 0x0},   {0xf1, 0x0},   {0xf2, 0x0},
    {0xf3, 0x0},   {0xf4, 0x0},  {0xf5, 0x0},   {0xf6, 0x0},   {0xf7, 0x0},
    {0xf8, 0x0},   {0xf9, 0x0},  {0xfa, 0x0},   {0xfb, 0x0},   {0xfc, 0x0},
    {0xfd, 0x0},   {0xfe, 0x0},  {0xff, 0x0},   {0x100, 0x38}, {0x101, 0x3},
    {0x102, 0x70}, {0x103, 0x0}, {0x104, 0x80}, {0x105, 0xff},
};
/**
 * @brief ak7738 read register by i2c interface function.
 *
 * @param dev codec device
 *
 * @param reg ak7738 register address
 *
 * @return \b u32 result reading result of register
 */
/* to read  ak7738 register */
static u32 ak7738_i2c_read_reg(struct audio_codec_dev dev, u32 reg) {
    unsigned char tx[3] = {0}, rx[3] = {0};
    int wlen, rlen;
    int ret;
    wlen = 3;
    rlen = 1;
    tx[0] = (unsigned char)(COMMAND_READ_REG & 0x7F);
    tx[1] = (unsigned char)(0xFF & (reg >> 8));
    tx[2] = (unsigned char)(0xFF & reg);
    ret =
        hal_i2c_read_reg_data(dev.i2c_handle, dev.i2c_addr, tx, wlen, rx, rlen);
    /* 	dprintf(AUDIO_CODEC_DEBUG,
                    "%s: result: read ak7738 addr=0x%x,reg=0x%x 0x%x\n",
       __func__, dev.i2c_addr, reg, rx[0]); */
    if (ret < 0) {
        dprintf(AUDIO_CODEC_DEBUG, "%s: error: read ak7738 reg=%x,ret=%x\n",
                __func__, reg, ret);
    }
    return rx[0];
}

/**
 * @brief ak7738 write register by i2c interface function.
 *
 * @param dev  codec device
 * @param reg ak7738 register address
 * @param val write value
 * @return \b u32 write result if ret < 0 write failed.
 */
static u32 ak7738_i2c_write_reg(struct audio_codec_dev dev, u32 reg, u32 val) {

    uint8_t tx[4];
    int wlen;
    int ret;

    /*  	dprintf(0, "[AK7738] %s (%XH, %XH)\n", __FUNCTION__,
                    reg, val);  */
    wlen = 4;
    tx[0] = (unsigned char)COMMAND_WRITE_REG;
    tx[1] = (unsigned char)(0xFF & (reg >> 8));
    tx[2] = (unsigned char)(0xFF & reg);
    tx[3] = (unsigned char)(0xFF & val);

    ret = hal_i2c_write_reg_data(dev.i2c_handle, dev.i2c_addr, tx, 1, &tx[1],
                                 wlen - 1);

    if (ret < 0) {
        dprintf(AUDIO_CODEC_DEBUG, "%s: error: reg=%x, val=%x\n", __func__, reg,
                val);
    }

    return ret;
}

/**
 * @brief dump ak7738 register values.
 *
 * @param dev
 */
static void ak7738_i2c_dump_reg(struct audio_codec_dev dev) {
    uint32_t reg_val = 0;
    for (uint32_t i = 0; i < 0x88; i++) {
        reg_val = ak7738_i2c_read_reg(dev, i);
        if ((i == ak7738_reg[i].reg) && (reg_val != ak7738_reg[i].def)) {
            dprintf(0, "%s: ***AK7738 Addr,Reg=(%x, 0x%x) : 0x%x\n", __func__,
                    i, reg_val, ak7738_reg[i].def);
        }
    }
}
static bool sdrv_ak7738_set(struct audio_codec_dev dev) {
    uint32_t i;
    for (i = 2; i < ARRAY_SIZE(ak7738_reg); i++) {
        ak7738_i2c_write_reg(dev, ak7738_reg[i].reg, ak7738_reg[i].def);
    }
    return true;
}
static const struct au_codec_dev_ctrl_interface ak7738_drv = {
    sdrv_ak7738_initialize,      sdrv_ak7738_start_up,
    sdrv_ak7738_set_volume,      sdrv_ak7738_set_format,
    sdrv_ak7738_set_hw_params,   sdrv_ak7738_trigger,
    sdrv_ak7738_shutdown,        sdrv_ak7738_set_input_path,
    sdrv_ak7738_set_output_path,
};
//#define AUDIO_I2C_GPIO_ADDR 6
#if FAST_AUDIO_CFG0 == 1
#define AUDIO_DSP_RST_N TCA6408_P4
#else
#define AUDIO_DSP_RST_N TCA9539_P07
#endif
const struct au_codec_dev_ctrl_interface *
sdrv_ak7738_get_controller_interface(void) {
    return &ak7738_drv;
}

struct audio_codec_dev *sdrv_ak7738_get_dev(int codec_id) {
    return &g_ak7738_instance[codec_id - 1];
}
/**
 * @brief initialize ak7738
 *
 * @param dev codec device
 *
 * @return \b bool
 */
static bool sdrv_ak7738_initialize(struct audio_codec_dev *dev) {

    int ak7738_devid;
    int ret;

/*reset pdn*/
#if FAST_AUDIO_CFG0 == 1
    struct tca6408_device *pd;
    pd = tca6408_init(6, 0x20);

    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return false;
    }

    tca6408_output_enable(pd, AUDIO_DSP_RST_N);
    tca6408_output_val(pd, AUDIO_DSP_RST_N, 0);
    mdelay(2);
    tca6408_output_val(pd, AUDIO_DSP_RST_N, 1);
    mdelay(100);

    tca6408_deinit(pd);
#else
    struct tca9539_device *pd;
    pd = tca9539_init(12, 0x74);
    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return false;
    }
    tca9539_enable_i2cpoll(pd);
    pd->ops.output_enable(pd, AUDIO_DSP_RST_N);
    pd->ops.output_val(pd, AUDIO_DSP_RST_N, 0);
    mdelay(2);
    pd->ops.output_val(pd, AUDIO_DSP_RST_N, 1);
    mdelay(100);
    tca9539_deinit(pd);
#endif
    printf("AK7738 dev id %d \n", dev->id);
    /* Read ak7738 version */
    ak7738_devid = ak7738_i2c_read_reg(*dev, AK7738_100_DEVICE_ID);
    dprintf(AUDIO_CODEC_DEBUG, "[AK7738] %s  Device ID = 0x%X\n", __FUNCTION__,
            ak7738_devid);

    /*set ak7738 clock*/
    ret = ak7738_i2c_write_reg(*dev, AK7738_00_STSCLOCK_SETTING1, 0xF);
    if (ret < 0) {
        return false;
    }

    /*set ak7738 bus clock*/
    ret = ak7738_i2c_write_reg(*dev, AK7738_12_BUS_CLOCK_SETTING2, 0x4);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_11_BUS_DSP_CLOCK_SETTING, 0x10);
    if (ret < 0) {
        return false;
    }
    /*set ak7738 sd clock 48k*/
    ret = ak7738_i2c_write_reg(*dev, AK7738_04_SYNCDOMAIN1_SETTING2, 0x27);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_06_SYNCDOMAIN2_SETTING2, 0x27);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_08_SYNCDOMAIN3_SETTING2, 0x4f);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_0A_SYNCDOMAIN4_SETTING2, 0x9);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_0C_SYNCDOMAIN5_SETTING2, 0x9);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_0E_SYNCDOMAIN6_SETTING2, 0x27);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_10_SYNCDOMAIN7_SETTING2, 0x27);
    if (ret < 0) {
        return false;
    }
    /*set ak7738 domain sync*/
    ret = ak7738_i2c_write_reg(*dev, AK7738_15_SYNCDOMAIN_SELECT1, 0x12);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_16_SYNCDOMAIN_SELECT2, 0x34);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_17_SYNCDOMAIN_SELECT3, 0x22);
    if (ret < 0) {
        return false;
    }
    /*set ak7738 set dai format*/
    ret = ak7738_i2c_write_reg(*dev, AK7738_05_SYNCDOMAIN2_SETTING1, 0x10);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_07_SYNCDOMAIN3_SETTING1, 0x12);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_09_SYNCDOMAIN4_SETTING1, 0x94);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_0B_SYNCDOMAIN5_SETTING1, 0x94);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_44_CLOCKFORMAT_SETTING1, 0x0);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_45_CLOCKFORMAT_SETTING2, 0x66);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_46_CLOCKFORMAT_SETTING3, 0x6);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_49_SDIN2_FORMAT, 0x32);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_4A_SDIN3_FORMAT, 0xa0);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_4B_SDIN4_FORMAT, 0xb0);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_4C_SDIN5_FORMAT, 0x80);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_4F_SDOUT2_FORMAT, 0x32);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_50_SDOUT3_FORMAT, 0xa0);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_51_SDOUT4_FORMAT, 0xb0);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_52_SDOUT5_FORMAT, 0xb0);
    if (ret < 0) {
        return false;
    }

    /*Sdout 5 -> 2*/
    ret = ak7738_i2c_write_reg(*dev, AK7738_27_SDOUT5_DATA_SELECT, 0x02);
    if (ret < 0) {
        return false;
    }

    /*     ret = ak7738_i2c_write_reg(*dev, AK7738_55_TDM_MODE_SETTING, 0x44);
        if (ret < 0) {
                return false;
        } */

    ret = ak7738_i2c_write_reg(*dev, 0x18, 0x42);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, 0x19, 0x2);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, 0x1a, 0x2);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, 0x1b, 0x34);
    if (ret < 0) {
        return false;
    }
#if SAF_SYSTEM_CFG == 1
    // Change port to sd1
    ret = ak7738_i2c_write_reg(*dev, AK7738_1A_SYNCDOMAIN_SELECT6, 0x22);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_15_SYNCDOMAIN_SELECT1, 0x22);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_48_SDIN1_FORMAT, 0x30);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_4E_SDOUT1_FORMAT, 0x30);
    if (ret < 0) {
        return false;
    }
    /*Sdout 5 -> 1*/
    ret = ak7738_i2c_write_reg(*dev, AK7738_27_SDOUT5_DATA_SELECT, 0x01);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_03_SYNCDOMAIN1_SETTING1, 0x10);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_04_SYNCDOMAIN1_SETTING2, 0x27);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_55_TDM_MODE_SETTING, 0x44);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_57_OUTPUT_PORT_SELECT, 0x20);
    if (ret < 0) {
        return false;
    }
    /*Fixed parameters*/
    ret = ak7738_i2c_write_reg(*dev, AK7738_05_SYNCDOMAIN2_SETTING1, 0x90);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_1D_SYNCDOMAIN_SELECT9, 0x20);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_1F_SYNCDOMAIN_SELECT11, 0x30);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_21_SYNCDOMAIN_SELECT13, 0x22);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_24_SDOUT2_DATA_SELECT, 0xd);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_25_SDOUT3_DATA_SELECT, 0x22);
    if (ret < 0) {
        return false;
    }

    ret = ak7738_i2c_write_reg(*dev, AK7738_29_DAC1_DATA_SELECT, 0xf);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_2A_DAC2_DATA_SELECT, 0x10);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_2B_DSP1_IN1_DATA_SELECT, 0x1e);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_2C_DSP1_IN2_DATA_SELECT, 0x2);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_2D_DSP1_IN3_DATA_SELECT, 0x3);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_2E_DSP1_IN4_DATA_SELECT, 0x4);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_2F_DSP1_IN5_DATA_SELECT, 0x5);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_31_DSP2_IN1_DATA_SELECT, 0x7);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_32_DSP2_IN2_DATA_SELECT, 0x8);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_33_DSP2_IN3_DATA_SELECT, 0x9);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_34_DSP2_IN4_DATA_SELECT, 0xa);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_35_DSP2_IN5_DATA_SELECT, 0x19);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_36_DSP2_IN6_DATA_SELECT, 0x1a);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_37_SRC1_DATA_SELECT, 0x6);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_3B_FSCONV1_DATA_SELECT, 0x14);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_5D_MICAMP_GAIN, 0xff);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_6E_DAC_MUTE_FILTER, 0x82);
    if (ret < 0) {
        return false;
    }
#endif
    if (dev->id == 2) {
        printf("AK7738 dev id %d \n", dev->id);
        ret = ak7738_i2c_write_reg(*dev, AK7738_27_SDOUT5_DATA_SELECT, 0x0);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_25_SDOUT3_DATA_SELECT, 0x1);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_07_SYNCDOMAIN3_SETTING1, 0x94);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_08_SYNCDOMAIN3_SETTING2, 0x9);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_15_SYNCDOMAIN_SELECT1, 0x42);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_16_SYNCDOMAIN_SELECT2, 0x44);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_18_SYNCDOMAIN_SELECT4, 0x24);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_19_SYNCDOMAIN_SELECT5, 0x4);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_1A_SYNCDOMAIN_SELECT6, 0x42);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_1C_SYNCDOMAIN_SELECT8, 0x50);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_1D_SYNCDOMAIN_SELECT9, 0x40);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_1F_SYNCDOMAIN_SELECT11, 0x50);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_21_SYNCDOMAIN_SELECT13, 0x44);
        if (ret < 0) {
            return false;
        }
        ret = ak7738_i2c_write_reg(*dev, AK7738_50_SDOUT3_FORMAT, 0xb0);
        if (ret < 0) {
            return false;
        }
    }
    return true;
}

/**
 * @brief reset codec and set some common register
 *
 * @param dev codec device
 *
 * @return \b bool
 *
 */
static bool sdrv_ak7738_start_up(struct audio_codec_dev *dev) {
    int ret = 0;
    printf("AK7738 dev id %d \n", dev->id);

    ret = ak7738_i2c_write_reg(*dev, AK7738_01_STSCLOCK_SETTING2, 0x87);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_85_RESET_CTRL, 0x1d);
    if (ret < 0) {
        return false;
    }
    if (dev->id == 2) {
        ret = ak7738_i2c_write_reg(*dev, AK7738_58_OUTPUT_PORT_ENABLE, 0x0C);
    } else {
        ret = ak7738_i2c_write_reg(*dev, AK7738_58_OUTPUT_PORT_ENABLE, 0x06);
    }
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_83_POWER_MANAGEMENT1, 0x06);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(*dev, AK7738_03_SYNCDOMAIN1_SETTING1, 0x90);
    if (ret < 0) {
        return false;
    }
    return true;
}

/**
 * @brief set output path volume
 *
 * @param dev codec device
 *
 * @param volume_percent percentage of full scale volume(0~100)
 *
 * @return \b bool
 */
static bool sdrv_ak7738_set_volume(struct audio_codec_dev *dev,
                                   int volume_percent,
                                   enum audio_volume_type vol_type) {

    if (vol_type == AUDIO_VOL_HEADPHONE) {
        dev->hphone_out_vol = volume_percent;
        /* set phone out volume; left/right volume equal */

    } else if (vol_type == AUDIO_VOL_LINEOUT) {
        dev->line_in_vol = volume_percent;
        /* set line input volume; left/right volume equal */
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
 * @return \b bool
 */
static bool sdrv_ak7738_set_format(struct audio_codec_dev dev,
                                   pcm_params_t pcm_info) {
    u32 val = 0, ret = 0;
    /* set i2s interface mode */
    switch (pcm_info.standard) {
    case SD_AUDIO_I2S_STANDARD_PHILLIPS:
        // interface = TLV320AIC23_FOR_I2S;
        break;

    case SD_AUDIO_I2S_LEFT_JUSTIFIED:
        // interface = TLV320AIC23_FOR_LJUST;
        break;

    case SD_AUDIO_I2S_RIGHT_JUSTIFIED:
        printf("func<%s>: unsupported i2s right justified mode.\n", __func__);
        break;

    case SD_AUDIO_I2S_DSP_A:
        // interface = TLV320AIC23_FOR_DSP | TLV320AIC23_LRP_ON;
        break;

    case SD_AUDIO_I2S_DSP_B:
        // interface = TLV320AIC23_FOR_DSP & (~TLV320AIC23_LRP_ON);
        break;

    default:
        dprintf(AUDIO_CODEC_DEBUG,
                "codec interface mode wrong arg or no init.\n");
    }

    /* set master/slave */
    if ((pcm_info.mode & SD_AUDIO_TRANSFER_MODE_ENABLE) ==
        SD_AUDIO_TRANSFER_CODEC_SLAVE) {
        /*Set sd2 to codec slave mode.*/
        mdelay(10);
#if SAF_SYSTEM_CFG == 1
        val = ak7738_i2c_read_reg(dev, AK7738_03_SYNCDOMAIN1_SETTING1);
        printf("func<%s>: pcm mode is 0x%x 0x%x.\n", __func__, pcm_info.mode,
               val);
        val = SD_AUDIO_PCM_MODE_SET(val, BIT(7), 0);
        printf("func<%s>: pcm mode is 0x%x 0x%x.\n", __func__, pcm_info.mode,
               val);
        mdelay(10);
        ret = ak7738_i2c_write_reg(dev, AK7738_03_SYNCDOMAIN1_SETTING1, val);
        if (ret < 0) {
            return false;
        }
#else
        val = ak7738_i2c_read_reg(dev, AK7738_05_SYNCDOMAIN2_SETTING1);
        printf("func<%s>: pcm mode is 0x%x 0x%x.\n", __func__, pcm_info.mode,
               val);
        val = SD_AUDIO_PCM_MODE_SET(val, BIT(7), 0);
        printf("func<%s>: pcm mode is 0x%x 0x%x.\n", __func__, pcm_info.mode,
               val);
        mdelay(10);
        ret = ak7738_i2c_write_reg(dev, AK7738_05_SYNCDOMAIN2_SETTING1, val);
        if (ret < 0) {
            return false;
        }
#endif
    } else if ((pcm_info.mode & SD_AUDIO_TRANSFER_MODE_ENABLE) ==
               SD_AUDIO_TRANSFER_CODEC_MASTER) {
        /*Set sd2 to codec master mode.*/
        mdelay(10);
#if SAF_SYSTEM_CFG == 1
        val = ak7738_i2c_read_reg(dev, AK7738_03_SYNCDOMAIN1_SETTING1);
        printf("func<%s>: pcm mode is 0x%x 0x%x.\n", __func__, pcm_info.mode,
               val);
        val = SD_AUDIO_PCM_MODE_SET(val, BIT(7), BIT(7));
        printf("func<%s>: pcm mode is 0x%x 0x%x.\n", __func__, pcm_info.mode,
               val);
        mdelay(10);
        ret = ak7738_i2c_write_reg(dev, AK7738_03_SYNCDOMAIN1_SETTING1, val);
        if (ret < 0) {
            return false;
        }
#else
        val = ak7738_i2c_read_reg(dev, AK7738_05_SYNCDOMAIN2_SETTING1);
        printf("func<%s>: pcm mode is 0x%x 0x%x.\n", __func__, pcm_info.mode,
               val);
        val = SD_AUDIO_PCM_MODE_SET(val, BIT(7), BIT(7));
        printf("func<%s>: pcm mode is 0x%x 0x%x.\n", __func__, pcm_info.mode,
               val);
        mdelay(10);
        ret = ak7738_i2c_write_reg(dev, AK7738_05_SYNCDOMAIN2_SETTING1, val);
        if (ret < 0) {
            return false;
        }
#endif
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
 * @return \b bool
 */
static bool sdrv_ak7738_set_hw_params(struct audio_codec_dev dev,
                                      pcm_params_t pcm_info) {
    uint32_t sample_rate = 0;
    /* set sample rate */
    sample_rate = pcm_info.sample_rate;
    /*     uint32_t val = 0; */

    switch (sample_rate) {
    case SD_AUDIO_SR_8000:
        break;

    case SD_AUDIO_SR_32000:

        break;

    case SD_AUDIO_SR_48000:

        break;

    case SD_AUDIO_SR_96000:

        break;

    default:
        dprintf(AUDIO_CODEC_DEBUG, "unsupport sample rate: %d.\n", sample_rate);
    }

    /* set pcm data slot width */
    /*     format = codec_read_reg(dev.id, TLV320AIC23_DIGT_FMT); */

    switch (pcm_info.slot_width) {
    case SD_AUDIO_SLOT_WIDTH_16BITS:

        break;

    case SD_AUDIO_SLOT_WIDTH_20BITS:

        break;

    case SD_AUDIO_SLOT_WIDTH_24BITS:

        break;

    case SD_AUDIO_SLOT_WIDTH_32BITS:
    case SD_AUDIO_SLOT_WIDTH_NO_INIT:
        break;

    default:
        dprintf(AUDIO_CODEC_DEBUG, "codec slot width wrong arg or no init.\n");
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
 * @return \b bool
 */
static bool sdrv_ak7738_trigger(struct audio_codec_dev dev, int cmd) {

    /* handle command */
    if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_START) {
        // ak7738_i2c_dump_reg(dev);
    }

    if (cmd == SD_AUDIO_PCM_TRIGGER_CAPTURE_START) {
    }

    if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP) {
    }

    if (cmd == SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP) {
    }
    return true;
}

/**
 * @brief codec power off
 *
 * @param dev codec dev
 *
 * @return \b bool
 */
static bool sdrv_ak7738_shutdown(struct audio_codec_dev dev) {
    int ret = 0;
    ret = ak7738_i2c_write_reg(dev, AK7738_83_POWER_MANAGEMENT1, 0x0);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(dev, AK7738_58_OUTPUT_PORT_ENABLE, 0x04);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(dev, AK7738_85_RESET_CTRL, 0x0);
    if (ret < 0) {
        return false;
    }
    ret = ak7738_i2c_write_reg(dev, AK7738_01_STSCLOCK_SETTING2, 0x7);
    if (ret < 0) {
        return false;
    }

    return true;
}

/**
 * @brief set input path
 *
 * @param dev codec dev
 *
 * @param input_path line in(default) or mic in
 *
 * @return \b bool
 */
static bool sdrv_ak7738_set_input_path(struct audio_codec_dev *dev,
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
 * @return \b bool
 */
static bool sdrv_ak7738_set_output_path(struct audio_codec_dev *dev,
                                        uint32_t output_path) {
    dev->output_path = output_path;
    return true;
}
