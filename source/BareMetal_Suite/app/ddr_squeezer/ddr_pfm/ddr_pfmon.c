/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/
#include "ddr_pfmon.h"
#include <stdio.h>
static struct pfm_channel_config channel_config[MAX_CHANNEL];

static const int multi_config[MAX_CHANNEL][4] = {
    //AP domain
    {3, 0x1fc0, 3, 0x1fc0},
    //GPU1 domain
    {56, 0x1e00, 56, 0x1e00},
    //GPU2 domain
    {64, 0x1e00, 64, 0x1e00},
    //VPU2 domain
    {30, 0x1fc0, 30, 0x1fc0},
    //DP1 domain
    {23, 0x1fc0, 23, 0x1fc0},
    //DP2 domain
    {24, 0x1fc0, 24, 0x1fc0},
    //DP3 domain
    {25, 0x1fc0, 25, 0x1fc0},
    //DC1 domain
    {19, 0x1fc0, 19, 0x1fc0},
    //DC2 domain
    {20, 0x1fc0, 20, 0x1fc0},
    //DC3 domain
    {21, 0x1fc0, 21, 0x1fc0},
    //DC4 domain
    {22, 0x1fc0, 22, 0x1fc0},
    //DC5 domain
    {26, 0x1fc0, 26, 0x1fc0},
    //CSI domain
    {16, 0x1f80, 18, 0x1fc0},
    //VDSP domain
    {5, 0x1fc0, 5, 0x1fc0},
    //VPU1 domain
    {29, 0x1fc0, 29, 0x1fc0},
    //all
    {0, 0, 0, 0}
};

static void enable_pfm(void)
{
    int channel=0;
    for(channel=0;channel<MAX_CHANNEL;channel++){
        channel_config[channel].sum_read = 0;
        channel_config[channel].sum_write = 0;
        channel_config[channel].sum_read_burst = 0;
        channel_config[channel].sum_write_burst = 0;
    }
    writel(1, APB_DDR_PERF_MON_BASE + PFMON_BASE_CNT_CTL);
}

static void disable_pfm(void)
{
    int channel=0;
    for(channel=0;channel<MAX_CHANNEL;channel++){
        channel_config[channel].sum_read = 0;
        channel_config[channel].sum_write = 0;
        channel_config[channel].sum_read_burst = 0;
        channel_config[channel].sum_write_burst = 0;
    }
    writel(0, APB_DDR_PERF_MON_BASE + PFMON_BASE_CNT_CTL);
}

static void clean_register(void)
{
    writel(2, APB_DDR_PERF_MON_BASE + PFMON_BASE_CNT_CTL);
}


void ddr_profiling_init(void)
{
    int channel=0;
    disable_pfm();
    /* global config */
    /* config base counter windows */ 
    writel(DDR_PFM_BASE_COUNT_WINDOW_MS, APB_DDR_PERF_MON_BASE + PFMON_BASE_CNT_CMP);
    /* config axi channel enable (mask) */
    writel(AXI_CNT_EN, APB_DDR_PERF_MON_BASE + AXI_CNT_CTL0);
    /* config mode: enable  auto restart */
    writel(AXI_CNT_AUTO_RESTART_EN, APB_DDR_PERF_MON_BASE + AXI_CNT_CTL1);

    /* channel config */
    for(channel=0;channel<MAX_CHANNEL;channel++){
        channel_config[channel].initialized = true;

        channel_config[channel].current_group = channel;
        channel_config[channel].master_id0 = (multi_config[channel][0] << 6);
        channel_config[channel].master_id1 = (multi_config[channel][2] << 6);

        //MID[12:6] default is 0x1FC0
        channel_config[channel].master_msk_id0 = multi_config[channel][1];
        channel_config[channel].master_msk_id1 = multi_config[channel][3];

        writel(channel_config[channel].master_id0, APB_DDR_PERF_MON_BASE + AXI_MST0_ID0 + channel*16);
        writel(channel_config[channel].master_id1, APB_DDR_PERF_MON_BASE + AXI_MST0_ID1 + channel*16);
        writel(channel_config[channel].master_msk_id0, APB_DDR_PERF_MON_BASE + AXI_MST0_ID0_MSK + channel*16);
        writel(channel_config[channel].master_msk_id1, APB_DDR_PERF_MON_BASE + AXI_MST0_ID1_MSK + channel*16);

        //only for first 8 channnals
        if (channel < 8) {
            channel_config[channel].read_threshold = 0xffffffff;
            channel_config[channel].write_threshold = 0xffffffff;
            channel_config[channel].read_burst_threshold = 0xffffffff;
            channel_config[channel].write_burst_threshold = 0xffffffff;

            writel(channel_config[channel].read_threshold, APB_DDR_PERF_MON_BASE + AXI_MST0_RD_THR + channel*16);
            writel(channel_config[channel].write_threshold, APB_DDR_PERF_MON_BASE + AXI_MST0_WR_THR + channel*16);
            writel(channel_config[channel].read_burst_threshold, APB_DDR_PERF_MON_BASE + AXI_MST0_RD_BCNT_THR + channel*16);
            writel(channel_config[channel].write_burst_threshold, APB_DDR_PERF_MON_BASE + AXI_MST0_WR_BCNT_THR + channel*16);
        }
    }
    /* */
    enable_pfm();
}

int32_t pfm_poll_handler(void)
{

    uint32_t status = readl(APB_DDR_PERF_MON_BASE + MISC_ST);
    if (status & time_window_bit) {
        struct pfm_record tmp;
        for (unsigned int i=0; i < MAX_CHANNEL; i++) {
            if (channel_config[i].initialized) {
                tmp.read_count = readl(APB_DDR_PERF_MON_BASE + AXI_MST0_RD_CNT + i*24);
                tmp.write_count = readl(APB_DDR_PERF_MON_BASE + AXI_MST0_WR_CNT + i*24);
                tmp.read_burst_count_h = readl(APB_DDR_PERF_MON_BASE + AXI_MST0_RD_BCNTH + i*24) & 0x000000FF;
                tmp.read_burst_count_l = readl(APB_DDR_PERF_MON_BASE + AXI_MST0_RD_BCNTL + i*24);
                tmp.write_burst_count_h = readl(APB_DDR_PERF_MON_BASE + AXI_MST0_WR_BCNTH + i*24) & 0x000000FF;
                tmp.write_burst_count_l = readl(APB_DDR_PERF_MON_BASE + AXI_MST0_WR_BCNTL + i*24);

                channel_config[i].sum_read += tmp.read_count;
                channel_config[i].sum_write += tmp.write_count;
                channel_config[i].sum_read_burst += tmp.read_burst_count_l;
                channel_config[i].sum_write_burst += tmp.write_burst_count_l;
                // DBG("%d is %d  \n",i,channel_config[i].sum_read_burst+channel_config[i].sum_write_burst);
            }
            else {
                DBG("channel disabled,but get IRQ, should not happen\n");
            }
        }
        clean_register();
        return 0;
    }
    else if (status & DDRPHY_watchdog_bit) {
        DBG("DDRPHY watchdog timeout\n");
    }
    return -1;
}


void print_channel_record(int channel)
{
    uint32_t result = 0;
    if (channel_config[channel].initialized) {
        result += channel_config[channel].sum_read_burst;
        result += channel_config[channel].sum_write_burst;
        if(result){
            DBG("%d \n",result/(10000));
        }
        enable_pfm();
    }
    else {
        DBG("channel %d is not available.\n", channel);
    }
}

