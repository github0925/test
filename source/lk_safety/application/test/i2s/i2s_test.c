/*

i2s dma mode transfer test
    width:     8,    16,  24(32),   32      bits
audiofreq: 4.41k,  4,8k,     96k,  192k     hertz
sc loopback;
sc full duplex loopback;
mc loopback;
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <arch.h>
#include "i2s_hal.h"
#if ENABLE_SD_DMA
#include "dma_hal.h"
#endif
#include "res.h"
#define SIZE_BUFFERS 1024

static uint8_t src1[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]__attribute__((aligned(CACHE_LINE)))= { 0 };
static uint8_t dst1[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]__attribute__((aligned(CACHE_LINE)))= { 0 };
static uint8_t src2[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]__attribute__((aligned(CACHE_LINE)))= { 0 };
static uint8_t dst2[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]__attribute__((aligned(CACHE_LINE)))= { 0 };
static void *i2s_sc1_handle;
static void *i2s_sc2_handle;
static i2s_sc_init_t i2s_sc1_cfg, i2s_sc2_cfg;
#if 0
static void *i2s_mc1_handle;
static i2s_mc_init_t i2s_mc1_cfg;
#endif

static void i2s_test_usage_string(void)
{
    printf("i2s_test -t <option> -a <option> -w <option>");
    printf("         -t 1/2/3 sc/sc_fd/mc loopback\n");
    printf("         -a audiofreq 44100/48000/96000/192000\n");
    printf("         -w width 8/16/24/32\n");
}
#if 1
static void test_result(uint8_t *source, uint8_t *dest)
{
    int zero_count  = 0, source_count = 0;
    int fail_flag = 0, leading_flag = 0;

    for(int i = 0; i < SIZE_BUFFERS; i++){
        if(!leading_flag && dest[i] == 0x0){
            zero_count++;
        }
        else if(dest[i] != source[source_count]){
            dprintf(INFO, "test result failed (at source buffer count %d and  with %d leading zero)\n", i, zero_count);
            fail_flag = 1;
            break;
        }else{
            source_count++;
            leading_flag = 1;
        }
    }
    if(!fail_flag)
        dprintf(INFO, "test result passed (with %d leading zero)\n", zero_count);
}
#endif
static void dmac_irq_evt_handle(enum dma_status status, u32 err, void *context)
{
    printf("dma irq evt: status(%d) err(%d) context(%p) \n", status, err, context);
}
static void modifyData(uint8_t *src, uint8_t *dst, int words)
{
    int i;
    dprintf(INFO, "src addr:0x%lx\n", src);
    dprintf(INFO, "dst addr:0x%lx\n", dst);
    // Put some data in the source buffer for test
    for (i = 0; i < words; i++) {
        src[i] = i % 255 + 1;
        // dst[i] = ~src[i]; //255 - i % 255;
    }
}

static int width_map_i2s2dma(int i2s_width)
{
    switch (i2s_width)
    {
        case  I2S_SAMPLE_8_BIT:
            return DMA_DEV_BUSWIDTH_1_BYTE;
        case  I2S_SAMPLE_16_BIT:
            return DMA_DEV_BUSWIDTH_2_BYTES;
        case  I2S_SAMPLE_24_BIT:
            return DMA_DEV_BUSWIDTH_4_BYTES;
        case  I2S_SAMPLE_32_BIT:
            return DMA_DEV_BUSWIDTH_4_BYTES;
        default:
            return 0;
    }
}

static void private_sc_config(i2s_sc_init_t *cfg, int sres, int mode, int freq)
{
    cfg->tx_sample_resolution = sres;
    cfg->rx_sample_resolution = sres;
    cfg->mode = mode;
    cfg->func_mode = I2S_FUNC_WITH_DMA;
    cfg->chn_width = 7;
    cfg->chn_mode = I2S_SC_CHN_STEREO;
    cfg->standard = I2S_STD_PHILLIPS;
    cfg->audio_freq = freq;
}

static void private_mc_config(i2s_mc_init_t *cfg, int sres, int freq)
{
    cfg->tx_mode = I2S_MC_MOD_SLAVE;
    cfg->tx_standard = I2S_STD_PHILLIPS;
    cfg->tx_audio_freq = freq;
    cfg->tx_sample_resolution = sres;

    cfg->rx_mode = I2S_MC_MOD_MASTER;
    cfg->rx_standard = I2S_STD_PHILLIPS;
    cfg->rx_audio_freq = freq;
    cfg->rx_sample_resolution = sres;

    cfg->chn_enable = 0xff;
    cfg->chn_int_en = 0x55;
    cfg->func_mode = I2S_FUNC_WITH_DMA;
    cfg->loop_back_test_en = 1;
}

static void run_sc_loopback(int width, int freq)
{
    // i2s part
    hal_i2s_sc_create_handle(&i2s_sc1_handle, RES_I2S_SC_I2S_SC1);
    hal_i2s_sc_init(i2s_sc1_handle);
    hal_i2s_sc_create_handle(&i2s_sc2_handle, RES_I2S_SC_I2S_SC2);
    hal_i2s_sc_init(i2s_sc2_handle);
    private_sc_config(&i2s_sc1_cfg, width, I2S_SC_MOD_MASTER_TX, freq);
    private_sc_config(&i2s_sc2_cfg, width, I2S_SC_MOD_SLAVE_RX, freq);
    hal_i2s_sc_config(i2s_sc1_handle, &i2s_sc1_cfg);
    hal_i2s_sc_config(i2s_sc2_handle, &i2s_sc2_cfg);

    // dma part
    size_t len = SIZE_BUFFERS;
    struct dma_dev_cfg dma_cfg;
    dma_cfg.direction = DMA_MEM2DEV;
    dma_cfg.dst_addr = hal_i2s_sc_get_fifo_addr(i2s_sc1_handle);
    dma_cfg.src_addr_width = width_map_i2s2dma(width);
    dma_cfg.dst_addr_width = width_map_i2s2dma(width);
    dma_cfg.src_maxburst = DMA_BURST_TR_4ITEMS;
    dma_cfg.dst_maxburst = DMA_BURST_TR_4ITEMS;
    struct dma_chan * tx_chan = hal_dma_chan_req(DMA_PERI_I2S_SC1);
    hal_dma_dev_config(tx_chan, &dma_cfg);
    struct dma_desc *desc_tx;
    desc_tx = hal_prep_dma_dev(tx_chan, (void*)src1, len, DMA_INTERRUPT);
    desc_tx->dmac_irq_evt_handle = dmac_irq_evt_handle;
    desc_tx->context = (void *)0xa5a5;

    dma_cfg.direction = DMA_DEV2MEM;
    dma_cfg.src_addr = hal_i2s_sc_get_fifo_addr(i2s_sc2_handle);
    struct dma_chan *rx_chan = hal_dma_chan_req(DMA_MEM);
    hal_dma_dev_config(rx_chan, &dma_cfg);
    struct dma_desc *desc_rx;
    desc_rx = hal_prep_dma_dev(rx_chan, (void*)dst1, len, DMA_INTERRUPT);
    desc_rx->dmac_irq_evt_handle = dmac_irq_evt_handle;
    desc_rx->context = (void*)0x5a5a;

    hal_i2s_sc_start(i2s_sc1_handle);
    hal_i2s_sc_start(i2s_sc2_handle);
    hal_dma_submit(desc_tx);
    hal_dma_submit(desc_rx);
    enum dma_status ret_rx = hal_dma_sync_wait(desc_rx, 100);
    if(DMA_COMP != ret_rx)
    {
       hal_dma_terminate(desc_tx);
       hal_dma_terminate(desc_rx);
    }
    dprintf(INFO,"transfer completed with ret(%d) \n", ret_rx);

    hal_dma_free_desc(desc_tx);
    hal_dma_free_desc(desc_rx);

    hal_i2s_sc_stop(i2s_sc1_handle);
    hal_i2s_sc_stop(i2s_sc2_handle);
    hal_i2s_sc_release_handle(i2s_sc1_handle);
    hal_i2s_sc_release_handle(i2s_sc2_handle);
}

static void run_sc_full_duplex_loopback(int width, int freq)
{
    // i2s part
    hal_i2s_sc_create_handle(&i2s_sc1_handle, RES_I2S_SC_I2S_SC1);
    hal_i2s_sc_init(i2s_sc1_handle);
    hal_i2s_sc_create_handle(&i2s_sc2_handle, RES_I2S_SC_I2S_SC2);
    hal_i2s_sc_init(i2s_sc2_handle);
    private_sc_config(&i2s_sc1_cfg, width, I2S_SC_MOD_MASTER_FULL_DUPLEX, freq);
    private_sc_config(&i2s_sc2_cfg, width, I2S_SC_MOD_SLAVE_FULL_DUPLEX, freq);
    hal_i2s_sc_config(i2s_sc1_handle, &i2s_sc1_cfg);
    hal_i2s_sc_config(i2s_sc2_handle, &i2s_sc2_cfg);

    // dma part
    size_t len = SIZE_BUFFERS;
    dprintf(INFO, "len: (%d) \n",len);
    // sc3 tx
    struct dma_dev_cfg cfg;
    cfg.direction = DMA_MEM2DEV;
    cfg.dst_addr  = hal_i2s_sc_get_fifo_addr(i2s_sc1_handle);
    cfg.src_addr_width = width_map_i2s2dma(width);
    cfg.dst_addr_width = width_map_i2s2dma(width);
    cfg.src_maxburst = DMA_BURST_TR_4ITEMS;
    cfg.dst_maxburst = DMA_BURST_TR_4ITEMS;
    struct dma_chan *sc3_tx_chan = hal_dma_chan_req(DMA_PERI_I2S_SC1);
    hal_dma_dev_config(sc3_tx_chan, &cfg);
    struct dma_desc*sc3_desc_tx;
    sc3_desc_tx= hal_prep_dma_dev(sc3_tx_chan, (void*)src1, len, DMA_INTERRUPT);
    sc3_desc_tx->dmac_irq_evt_handle = dmac_irq_evt_handle;

    // sc4 tx
    cfg.dst_addr  = hal_i2s_sc_get_fifo_addr(i2s_sc2_handle);
    struct dma_chan *sc4_tx_chan = hal_dma_chan_req(DMA_PERI_I2S_SC2);
    hal_dma_dev_config(sc4_tx_chan, &cfg);
    struct dma_desc*sc4_desc_tx;
    sc4_desc_tx = hal_prep_dma_dev(sc4_tx_chan, (void*)src2, len, DMA_INTERRUPT);
    sc4_desc_tx->dmac_irq_evt_handle = dmac_irq_evt_handle;

    // sc3 rx
    cfg.direction = DMA_DEV2MEM;
    cfg.src_addr  = hal_i2s_sc_get_fifo_addr(i2s_sc1_handle);
    struct dma_chan *sc3_rx_chan = hal_dma_chan_req(DMA_MEM);
    hal_dma_dev_config(sc3_rx_chan, &cfg);
    struct dma_desc*sc3_desc_rx;
    sc3_desc_rx = hal_prep_dma_dev(sc3_rx_chan, (void*)dst1, len, 0);

    // sc4 rx
    cfg.src_addr  = hal_i2s_sc_get_fifo_addr(i2s_sc2_handle);
    struct dma_chan *sc4_rx_chan = hal_dma_chan_req(DMA_MEM);
    hal_dma_dev_config(sc4_rx_chan, &cfg);
    struct dma_desc*sc4_desc_rx;
    sc4_desc_rx = hal_prep_dma_dev(sc4_rx_chan, (void*)dst2, len,  0);
    hal_i2s_sc_start(i2s_sc1_handle);
    hal_i2s_sc_start(i2s_sc2_handle);
    hal_dma_submit(sc3_desc_tx);
    hal_dma_submit(sc3_desc_rx);
    hal_dma_submit(sc4_desc_tx);
    hal_dma_submit(sc4_desc_rx);

    enum dma_status ret_rx3 = hal_dma_sync_wait(sc3_desc_rx, 100);
    enum dma_status ret_rx4 = hal_dma_sync_wait(sc4_desc_rx, 100);
    if(DMA_COMP != (ret_rx3&ret_rx4))
    {
       hal_dma_terminate(sc3_desc_tx);
       hal_dma_terminate(sc3_desc_rx);
       hal_dma_terminate(sc4_desc_tx);
       hal_dma_terminate(sc4_desc_rx);
    }
    dprintf(INFO,"transfer completed with ret(%d) \n", ret_rx4&ret_rx4);

    hal_dma_free_desc(sc3_desc_tx);
    hal_dma_free_desc(sc3_desc_rx);
    hal_dma_free_desc(sc4_desc_tx);
    hal_dma_free_desc(sc4_desc_rx);
    hal_i2s_sc_stop(i2s_sc1_handle);
    hal_i2s_sc_stop(i2s_sc2_handle);
    hal_i2s_sc_release_handle(i2s_sc1_handle);
    hal_i2s_sc_release_handle(i2s_sc2_handle);
}

static void run_mc_loopback(int width, int freq)
{
#if 0
    hal_i2s_mc_create_handle(&i2s_mc1_handle, RES_I2S_MC_I2S_MC1);
    hal_i2s_mc_init(i2s_mc1_handle);
    private_mc_config(&i2s_mc1_cfg, width, freq);
    hal_i2s_mc_config(i2s_mc1_handle, &i2s_mc1_cfg);

    size_t len = SIZE_BUFFERS;
    dprintf(INFO, "len: (%d) \n",len);

    struct dma_dev_cfg cfg;
    cfg.direction = DMA_MEM2DEV;
    cfg.src_addr  = hal_i2s_mc_get_fifo_addr(i2s_mc1_handle);
    cfg.dst_addr  = hal_i2s_mc_get_fifo_addr(i2s_mc1_handle);
    cfg.src_addr_width = width_map_i2s2dma(width);
    cfg.dst_addr_width = width_map_i2s2dma(width);
    cfg.src_maxburst = DMA_BURST_TR_32ITEMS;
    cfg.dst_maxburst = DMA_BURST_TR_32ITEMS;
    struct dma_chan *tx_chan = hal_dma_chan_req(DMA_PERI_I2S_MC1);
    hal_dma_dev_config(tx_chan, &cfg);
    struct dma_desc* desc_tx;
    desc_tx= hal_prep_dma_dev(tx_chan, (void*)src1, len, DMA_INTERRUPT);
    desc_tx->dmac_irq_evt_handle = dmac_irq_evt_handle;
    desc_tx->context = (void *)0xa5a5;

    cfg.direction = DMA_DEV2MEM;
    struct dma_chan *rx_chan = hal_dma_chan_req(DMA_MEM);
    hal_dma_dev_config(rx_chan, &cfg);
    struct dma_desc* desc_rx;
    desc_rx = hal_prep_dma_dev(rx_chan, (void*)dst1, len, DMA_INTERRUPT);
    desc_rx->dmac_irq_evt_handle = dmac_irq_evt_handle;
    desc_rx->context = (void*)0x5a5a;

    hal_i2s_mc_start(i2s_mc1_handle);
    hal_dma_submit(desc_tx);
    hal_dma_submit(desc_rx);
    enum dma_status ret_rx = hal_dma_sync_wait(desc_rx, 100);
    if(DMA_COMP != ret_rx)
    {
       hal_dma_terminate(desc_tx);
       hal_dma_terminate(desc_rx);
    }
    dprintf(INFO,"transfer completed with ret(%d) \n", ret_rx);

    hal_dma_free_desc(desc_tx);
    hal_dma_free_desc(desc_rx);
    hal_i2s_mc_stop(i2s_mc1_handle);
    hal_i2s_mc_release_handle(i2s_mc1_handle);

#endif
}

static void run_test(int type, int freq, int width)
{
    modifyData(src1, dst1, SIZE_BUFFERS);
    memset(dst1, 0xaa, SIZE_BUFFERS);
    if(type == 1) //sc
    {
        run_sc_loopback(width, freq);
        test_result(src1, dst1);
    }else
    if(type == 2) // sc fd
    {
        modifyData(src2, dst2, SIZE_BUFFERS);
        memset(dst2, 0xaa, SIZE_BUFFERS);
        run_sc_full_duplex_loopback(width, freq);
        dprintf(INFO, "buffer 1\n");
        test_result(src1, dst1);
        dprintf(INFO, "buffer 2\n");
        test_result(src2, dst2);
    }else
    if(type == 3) // mc
    {
        run_mc_loopback(width, freq);
        test_result(src1, dst1);
    }
}

static int i2s_test(int argc, const cmd_args *argv)
{
    int i2s_type = 1;
    int audiofreq = 44100;
    int width = I2S_SAMPLE_16_BIT;

    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i].str, "-help"))
        {
            i2s_test_usage_string();
            return 0;
        }else
        if(!strcmp(argv[i].str, "-t") && i+1 < argc)
        {
            i2s_type = argv[++i].i;
            dprintf(INFO, "i2s_type(sc-1/mc-2):%d\n", i2s_type);
        }else
        if(!strcmp(argv[i].str, "-sr") && i+1 < argc)
        {
            audiofreq = argv[++i].i;
            dprintf(INFO, "audiofreq:%d\n", audiofreq);
        }else
        if(!strcmp(argv[i].str, "-w") && i+1 < argc)
        {
            width = argv[++i].i - 1;
            dprintf(INFO, "width:%d\n", width);
        }
    }

    run_test(i2s_type, audiofreq, width);
    return 0;
}
#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("i2s_test", "i2s_test (defatult)-i2s_type 1 -a 4800 -w 16", (console_cmd)&i2s_test)
STATIC_COMMAND_END(i2s_test);
#endif
APP_START(i2s_test_sample)
.flags = 0,
APP_END
