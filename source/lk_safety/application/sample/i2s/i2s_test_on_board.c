/*
test i2s sc play and record on evb board.
*/

#include "dma_hal.h"
#include "i2s_hal.h"
#include "i2c_hal.h"
#include "res.h"
#include "wav.h"
#include "tlv320aic23.h"
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <platform.h>
#include <platform/debug.h>
#include <lib/console.h>
#include <string.h>
#include <lib/reg.h>

#define SIZE_BUFFERS 1024
#define PERIOD_SIZE 2048
#define PERIOD_COUNT 2
#define TEST_DURATION (2000)
#define STANDARD_I2S_MODE 0
#define I2S_DSP_MODE 1
#define AUDIOFREQ 44100

static void *i2s_handle;
void *i2c_handle;
static struct dma_desc *desc_tx, *desc_rx;

static uint8_t src_data[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]
__attribute__((aligned(CACHE_LINE)));
static uint8_t dst_data[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]
__attribute__((aligned(CACHE_LINE)));

static uint8_t cyclic_buffer[ROUNDUP(PERIOD_SIZE,
                                     CACHE_LINE)]__attribute__((aligned(CACHE_LINE)));
static lk_time_t start_ts;

static void handle_get_all(void)
{
    if (!hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C4))
        printf("error:i2c resource get failed.\n");

    if (!hal_i2s_sc_create_handle(&i2s_handle, RES_I2S_SC_I2S_SC1))
        printf("error:i2s resource get failed.\n");
}

static void config_i2s_sc(int mode, int width, i2s_sc_init_t *cfg)
{
    cfg->tx_sample_resolution = width;
    cfg->rx_sample_resolution = width;
    cfg->mode = mode;
    cfg->chn_mode = I2S_SC_CHN_STEREO;
    cfg->standard = I2S_STD_PHILLIPS;
    cfg->audio_freq = 48000; // 44100
    cfg->chn_width = 7;
    cfg->func_mode = I2S_FUNC_WITH_DMA;
}

static int width_map_i2s2dma(int i2s_width)
{
    switch (i2s_width) {
        case I2S_SAMPLE_8_BIT:
            return DMA_DEV_BUSWIDTH_1_BYTE;

        case I2S_SAMPLE_16_BIT:
            return DMA_DEV_BUSWIDTH_2_BYTES;

        case I2S_SAMPLE_24_BIT:
            return DMA_DEV_BUSWIDTH_4_BYTES;

        case I2S_SAMPLE_32_BIT:
            return DMA_DEV_BUSWIDTH_4_BYTES;

        default:
            dprintf(INFO, "unknown i2s sample resolution\n");
            return 0;
    }
}

static int tlv320_i2c_write_reg(u32 reg, u32 val)
{
    u8 addr, buf;
    int ret;

    addr = (reg << 1) | (val >> 8 & 0x01);
    buf = (u8)(val & 0xFF);
    // buf[1] = val >> 8;
    ret = hal_i2c_write_reg_data(i2c_handle, TLV320AIC23_I2C_ADDR, &addr, 1,
                                 &buf, 1);
    dprintf(0, "%s(), ret=%d, addr=0x%x buf=0x%x\n", __func__, ret, addr, buf);

    if (ret < 0) {
        dprintf(0, "%s: error: reg=%x, val=%x\n", __func__, reg, val);
    }

    return ret;
}

static void config_bypass(bool flag)
{
    tlv320_i2c_write_reg(TLV320AIC23_PWR, TLV320AIC23_DEVICE_PWR_OFF);

    /*0x1*/
    if (true == flag) {
        tlv320_i2c_write_reg(TLV320AIC23_ANLG, TLV320AIC23_BYPASS_ON);
    }
    else {
        tlv320_i2c_write_reg(TLV320AIC23_ANLG, ~TLV320AIC23_BYPASS_ON);
    }

    tlv320_i2c_write_reg(TLV320AIC23_ACTIVE, 0x0001);

    /* Power on the device 22*/
    tlv320_i2c_write_reg(TLV320AIC23_PWR, 0x11);
}

static void enable_tlv320_codec_record(void)
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
    //active and power on
    tlv320_i2c_write_reg(TLV320AIC23_ACTIVE, TLV320AIC23_ACT_ON);
    // power_contrl = ~TLV320AIC23_ADC_OFF;
    // power_contrl &= ~TLV320AIC23_LINE_OFF;
    tlv320_i2c_write_reg(TLV320AIC23_PWR, 0x7a);//TODO: line input & ADC on
}

static void enable_tlv320_codec_play(int digital_interface_flag)
{
    u16 ifmt_reg = 0;
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
    }
    else if (digital_interface_flag == I2S_DSP_MODE) {
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

static void disable_tlv320_codec(void)
{
    /* Clear data 0x80*/
    tlv320_i2c_write_reg(TLV320AIC23_ACTIVE, 0x0);
    tlv320_i2c_write_reg(TLV320AIC23_PWR, TLV320AIC23_DEVICE_PWR_OFF);
}

static void playback_irq_evt_handle(enum dma_status status, u32 err,
                                    void *context)
{
    printf("dma irq evt: status(%d) err(%d) context(%p) \n", status, err,
           context);

    if (DMA_COMP != status) {
        /* terminate this channel after error. */
        hal_dma_terminate(desc_tx);
    }
}

static void sc_play(u32 rate, u32 bits)
{
    hal_i2s_sc_init(i2s_handle);
    i2s_sc_init_t sc_cfg = {0};
    sc_cfg.mode = I2S_SC_MOD_MASTER_TX; //  I2S_SC_MOD_MASTER_FULL_DUPLEX;
    sc_cfg.chn_mode = I2S_SC_CHN_STEREO;
    sc_cfg.standard = I2S_STD_PHILLIPS;
    sc_cfg.audio_freq = rate;
    sc_cfg.chn_width = 7;
    sc_cfg.tx_sample_resolution = bits;
    sc_cfg.func_mode = I2S_FUNC_WITH_DMA;
    hal_i2s_sc_config(i2s_handle, &sc_cfg);

    // config dma channel
    struct dma_dev_cfg dma_cfg;
    dma_cfg.direction = DMA_MEM2DEV;
    dma_cfg.src_addr = 0;
    dma_cfg.dst_addr = hal_i2s_sc_get_fifo_addr(i2s_handle);
    dma_cfg.src_addr_width = width_map_i2s2dma(bits);
    dma_cfg.dst_addr_width = width_map_i2s2dma(bits);
    dma_cfg.src_maxburst = DMA_BURST_TR_32ITEMS;
    dma_cfg.dst_maxburst = DMA_BURST_TR_32ITEMS;

    struct dma_chan *tx = hal_dma_chan_req(DMA_PERI_I2S_SC1);
    hal_dma_dev_config(tx, &dma_cfg);
    desc_tx = hal_prep_dma_dev(tx, (void *)wav_data, sizeof(wav_data),
                               DMA_INTERRUPT);
    desc_tx->dmac_irq_evt_handle = playback_irq_evt_handle;
    desc_tx->context = (void *)0xa5a5;
    hal_i2s_sc_start(i2s_handle);
    hal_dma_submit(desc_tx);
    enum dma_status ret = hal_dma_sync_wait(desc_tx, -1);

    hal_i2s_sc_stop(i2s_handle);
    hal_i2s_sc_release_handle(i2s_handle);
    hal_dma_free_desc(desc_tx);
    hal_i2c_release_handle(i2c_handle);
    disable_tlv320_codec();
}

static void sc_play_int(u32 rate, u32 bits)
{
    hal_i2s_sc_init(i2s_handle);
    i2s_sc_init_t sc_cfg = {0};
    sc_cfg.mode = I2S_SC_MOD_MASTER_TX; //  I2S_SC_MOD_MASTER_FULL_DUPLEX;
    sc_cfg.chn_mode = I2S_SC_CHN_STEREO;
    sc_cfg.standard = I2S_STD_PHILLIPS;
    sc_cfg.audio_freq = rate;
    sc_cfg.chn_width = 7;
    sc_cfg.tx_sample_resolution = bits;
    sc_cfg.func_mode = I2S_FUNC_WITH_INT;
    sc_cfg.ptx_buffer = (void *)wav_data;
    sc_cfg.tx_count = sizeof(wav_data);
    //sc_cfg.tx_int_transfer_complete_handle = sc_int_comp_irq;
    hal_i2s_sc_config(i2s_handle, &sc_cfg);
    hal_i2s_sc_start(i2s_handle);
    hal_i2s_sc_wait_tx_comp_intmode(i2s_handle, -1);
    hal_i2s_sc_stop(i2s_handle);
    hal_i2s_sc_release_handle(i2s_handle);
    disable_tlv320_codec();
    hal_i2c_release_handle(i2c_handle);
}

static void record_irq_evt_handle(enum dma_status status, u32 err,
                                  void *context)
{
    printf("dma irq evt: status(%d) err(%d) context(%p) \n", status, err,
           context);

    if ((current_time() - start_ts) > TEST_DURATION) {
        printf("sc record stopped ts(%d)\n", current_time());
        hal_dma_terminate(desc_rx);
    }
}

static void sc_record(u32 rate, u32 bits)
{
    // set up i2s and dma;
    hal_i2s_sc_init(i2s_handle);
    i2s_sc_init_t sc_cfg = {0};
    sc_cfg.mode = I2S_SC_MOD_MASTER_RX;
    sc_cfg.chn_mode = I2S_SC_CHN_STEREO;
    sc_cfg.standard = I2S_STD_PHILLIPS;
    sc_cfg.audio_freq = rate;
    sc_cfg.chn_width = 7;
    sc_cfg.rx_sample_resolution = bits;
    sc_cfg.func_mode = I2S_FUNC_WITH_DMA;
    hal_i2s_sc_config(i2s_handle, &sc_cfg);

    struct dma_dev_cfg dma_cfg;
    dma_cfg.direction = DMA_DEV2MEM;
    dma_cfg.src_addr = hal_i2s_sc_get_fifo_addr(i2s_handle);
    dma_cfg.dst_addr = 0;
    dma_cfg.src_addr_width = width_map_i2s2dma(bits);
    dma_cfg.dst_addr_width = width_map_i2s2dma(bits);
    dma_cfg.src_maxburst = DMA_BURST_TR_32ITEMS;
    dma_cfg.dst_maxburst = DMA_BURST_TR_32ITEMS;

    struct dma_chan *rx = hal_dma_chan_req(DMA_PERI_I2S_SC1);
    hal_dma_dev_config(rx, &dma_cfg);
    desc_rx = hal_prep_dma_cyclic(rx, cyclic_buffer, PERIOD_SIZE,
                                  PERIOD_SIZE / PERIOD_COUNT,
                                  DMA_INTERRUPT);
    desc_rx->context = (void *)0xa5a5;
    start_ts = current_time();
    hal_i2s_sc_start(i2s_handle);
    hal_dma_submit(desc_rx);

    while ((current_time() - start_ts) < TEST_DURATION)
        thread_sleep(500);

    hal_dma_terminate(desc_rx);
    hal_dma_free_desc(desc_rx);
    hal_i2s_sc_stop(i2s_handle);
    disable_tlv320_codec();
    hal_i2s_sc_release_handle(i2s_handle);
    hal_i2c_release_handle(i2c_handle);
}

static void sc_record_int(u32 rate, u32 bits)
{

    // set up i2s and dma;
    memset((void *)cyclic_buffer, 0xff, PERIOD_SIZE);
    hal_i2s_sc_init(i2s_handle);
    i2s_sc_init_t sc_cfg = {0};
    sc_cfg.mode = I2S_SC_MOD_MASTER_RX;
    sc_cfg.chn_mode = I2S_SC_CHN_STEREO;
    sc_cfg.standard = I2S_STD_PHILLIPS;
    sc_cfg.audio_freq = rate;
    sc_cfg.chn_width = 7;
    sc_cfg.rx_sample_resolution = bits;
    sc_cfg.func_mode = I2S_FUNC_WITH_INT;
    sc_cfg.prx_buffer = (void *)cyclic_buffer;
    sc_cfg.rx_count = PERIOD_SIZE;
    hal_i2s_sc_config(i2s_handle, &sc_cfg);
    start_ts = current_time();
    hal_i2s_sc_start(i2s_handle);
    hal_i2s_sc_wait_rx_comp_intmode(i2s_handle, -1);
    hal_i2s_sc_stop(i2s_handle);
    disable_tlv320_codec();
    hal_i2s_sc_release_handle(i2s_handle);
    hal_i2c_release_handle(i2c_handle);
}

static void show_record_buffer(void)
{
    // print cyclic buffer value;
    int i;

    for (i = 0; i < PERIOD_SIZE; i++) {
        printf("%2x\t", cyclic_buffer[i]);

        if ((i + 1) % 16 == 0)
            printf("\n");
    }
}

static void tdm_playback(void)
{
    // tdm mode with stereo.
    hal_i2s_sc_init(i2s_handle);
    i2s_sc_init_t  sc_cfg = {0};
    sc_cfg.mode = I2S_SC_MOD_MASTER_TX;
    sc_cfg.chn_mode = I2S_SC_CHN_TDM;
    sc_cfg.ws_mode = 0;
    sc_cfg.tdm_tx_chn_en = 0x3;
    sc_cfg.standard = I2S_STD_PHILLIPS;
    sc_cfg.audio_freq = 48000;
    sc_cfg.chn_width = 2;
    sc_cfg.tx_sample_resolution = I2S_SAMPLE_16_BIT;
    sc_cfg.func_mode = I2S_FUNC_WITH_DMA;
    hal_i2s_sc_config(i2s_handle, &sc_cfg);

    // config dma channel
    struct dma_dev_cfg dma_cfg;
    dma_cfg.direction = DMA_MEM2DEV;
    dma_cfg.src_addr = 0;
    dma_cfg.dst_addr = hal_i2s_sc_get_fifo_addr(i2s_handle);
    dma_cfg.src_addr_width = width_map_i2s2dma(I2S_SAMPLE_16_BIT);
    dma_cfg.dst_addr_width = width_map_i2s2dma(I2S_SAMPLE_16_BIT);
    dma_cfg.src_maxburst = DMA_BURST_TR_32ITEMS;
    dma_cfg.dst_maxburst = DMA_BURST_TR_32ITEMS;

    struct dma_chan *tx = hal_dma_chan_req(DMA_PERI_I2S_SC1);
    hal_dma_dev_config(tx, &dma_cfg);
    desc_tx = hal_prep_dma_dev(tx, (void *)wav_data, sizeof(wav_data),
                               DMA_INTERRUPT);
    desc_tx->dmac_irq_evt_handle = playback_irq_evt_handle;
    desc_tx->context = (void *)0xa5a5;

    hal_i2s_sc_start(i2s_handle);
    hal_dma_submit(desc_tx);
    enum dma_status ret = hal_dma_sync_wait(desc_tx, -1);
    hal_i2s_sc_stop(i2s_handle);
    hal_i2s_sc_release_handle(i2s_handle);
    hal_dma_free_desc(desc_tx);
    hal_i2c_release_handle(i2c_handle);
    disable_tlv320_codec();
}

static void tdm_playback_int(void)
{
    // tdm mode with stereo.
    hal_i2s_sc_init(i2s_handle);
    i2s_sc_init_t  sc_cfg = {0};
    sc_cfg.mode = I2S_SC_MOD_MASTER_TX;
    sc_cfg.chn_mode = I2S_SC_CHN_TDM;
    sc_cfg.ws_mode = 0;
    sc_cfg.tdm_tx_chn_en = 0x3;
    sc_cfg.standard = I2S_STD_PHILLIPS;
    sc_cfg.audio_freq = 48000;
    sc_cfg.chn_width = 2;
    sc_cfg.tx_sample_resolution = I2S_SAMPLE_16_BIT;
    sc_cfg.func_mode = I2S_FUNC_WITH_INT;
    sc_cfg.ptx_buffer = (void *)wav_data;
    sc_cfg.tx_count = sizeof(wav_data);
    hal_i2s_sc_config(i2s_handle, &sc_cfg);
    hal_i2s_sc_start(i2s_handle);
    hal_i2s_sc_wait_tx_comp_intmode(i2s_handle, -1);
    hal_i2s_sc_stop(i2s_handle);
    hal_i2s_sc_release_handle(i2s_handle);
    hal_i2c_release_handle(i2c_handle);
    disable_tlv320_codec();
}

static void usage_string(void)
{
    printf("usage: i2s_sample [-play mode_num\n");
    printf("                  [-record mode_num\n");
    printf("                  [-init get res handle\n");
    printf("mode num: 1 dma standard i2s mode\n");
    printf("          2 dma dsp mode\n");
    printf("          3 int standard i2s mode\n");
    printf("          4 int dsp mode\n");
}

static int test_cmd(int argc, const cmd_args *argv)
{
    int i, case_type = 0, case_mode = 0;

    for (i = 0; i < argc; i++ ) {
        if (!strcmp(argv[i].str, "-help")) {
            usage_string();
            return 0;
        }
        else if (!strcmp(argv[i].str, "-init")) {
            handle_get_all();
        }
        else if (!strcmp(argv[i].str, "-play")) {
            case_type = 1;
            case_mode = argv[++i].i;
        }
        else if (!strcmp(argv[i].str, "-record")) {
            case_type = 2;
            case_mode = argv[++i].i;
        }
    }

    if (case_type == 1) {
        if ((case_mode == 1) || (case_mode == 3)) {
            enable_tlv320_codec_play(STANDARD_I2S_MODE);

            if (case_mode == 1) {
                sc_play(48000, I2S_SAMPLE_16_BIT);
            }
            else if (case_mode == 3) {
                sc_play_int(48000, I2S_SAMPLE_16_BIT);
            }
        }
        else if ((case_mode == 2) || (case_mode == 4)) {
            enable_tlv320_codec_play(I2S_DSP_MODE);

            if (case_mode == 2) {
                tdm_playback();
            }
            else if (case_mode == 4) {
                tdm_playback_int();
            }
        }
    }
    else if (case_type == 2) {
        enable_tlv320_codec_record();

        if (case_mode == 1) {
            sc_record(48000, I2S_SAMPLE_16_BIT);
        }
        else if (case_mode == 3) {
            sc_record_int(48000, I2S_SAMPLE_16_BIT);
        }
        else if ((case_mode == 2) || (case_mode == 4)) {
            return 0;
        }
    }

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("i2s_sample",
               "test i2s sc play && record , try> i2s_sample -help",
               (console_cmd)&test_cmd)
STATIC_COMMAND("show_record_buffer", "show record cyclic buffer data",
               (console_cmd)&show_record_buffer)
STATIC_COMMAND_END(i2s_onboard_test);
#endif

APP_START(i2s_test_on_board)
.flags = 0
         APP_END
