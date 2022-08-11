
/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/
#ifndef _DDR_PFMON_H_
#define _DDR_PFMON_H_
#include "soc.h"
#include "debug.h"
/*
 * pfmon base counter
 */
#define PFMON_BASE_CNT_CTL  0x0 /* base counter */
#define PFMON_BASE_CNT_CTL_EN       (1 << 0)    /* write 1 to enable base counter */
#define PFMON_BASE_CNT_CTL_CLR      (1 << 1)    /* write 1 to clear base, perf monitor, bw, axi counter */

#define PFMON_BASE_CNT_CMP  0x4 /* base counter will timeout and stop when reach cmp value */

/*
 * performance monitor counter
 */
#define PFMON_CNT0_CTL      0x10    /* perf monitor counter 0-3 ctrl */
#define PFMON_CNT1_CTL      0x14
#define PFMON_CNT2_CTL      0x18
#define PFMON_CNT3_CTL      0x1C
#define PFMON_CNT_CTL_CNT_EN        (1 << 0)    /* write 1 to enable performance counter */
#define PFMON_CNT_CTL_SRC_SEL       (0xff << 8) /* select perf_log_* */
#define PFMON_CNT_CTL_BANK_SEL      (7 << 16)   /* bank select */
#define PFMON_CNT_CTL_BG_SEL        (3 << 19)   /* bank group select */
#define PFMON_CNT_CTL_RANK_SEL      (1 << 21)   /* rank select */
#define PFMON_CNT_CTL_HIT_EN        (1 << 22)   /* write 1 to enable *_sel to specific DDR area to monitor */

#define PFMON_CNT0_DAT          0x20    /* perf monitor counter 0-3 value, RO */
#define PFMON_CNT1_DAT          0x24
#define PFMON_CNT2_DAT          0x28
#define PFMON_CNT3_DAT          0x2C

/*
 * bandwidth counter
 */
#define BW_CNT_CTL      0x40    /* bandwidth counter */
#define BW_CNT_CTL_EN           (1 << 0)    /* write 1 to enable bandwidth counter */

#define BW_CNT_DAT      0x44    /* bandwidth counter value, RO */

/*
 * misc mrr
 */
#define MISC_MRR_CTL        0x60    /* mrr data capture control */
#define MISC_MRR_CTL_LP_MODE_SEL    (1 << 0)    /* 1: select lpddr mode, 0: select ddr mode */
#define MISC_MRR_CTL_SER_PAR_SEL    (1 << 1)    /* 1: serial mode for ddr4, 0: parallel mode for ddr4 */
#define MISC_MRR_CTL_CLR        (1 << 2)    /* write 1 to clear previous mrr data and status */
#define MISC_MRR_CTL_VALID_OUT      (1 << 3)    /* 1: valid mrr data avaliable, 0: no valiad data, RO */

#define MISC_MRR_DAT        0x64    /* mmr data capture out, RO */

/*
 * misc watchdog control
 */
#define MISC_WDG_CTL        0x70    /* read watchdog control */
#define MISC_WDG_CTL_CMP        (0xfffff << 0)  /* watchdog will timeout when reach cmp */
#define MISC_WDG_CTL_CLR        (1 << 30)   /* write 1 to clear wdg counter & timeout flag */
#define MISC_WDG_CTL_EN         (1 << 31)   /* write 1 to enable watchdog */

#define MISC_WDG_CTL1       0x74    /* DDRPHY init watchdog control */
#define MISC_DDRPHY_INIT_WDG_EN     (1 << 0)    /* write 1 to enable DDRPHY init watchdog */
#define MISC_DDRPHY_INIT_WDG_CLR    (1 << 1)    /* write 1 to clear DDRPHY init wdg cnter&timeout flag */

#define MISC_WDG_CTL2       0x78    /* DDRPHY init wdg timeout cmp */
#define MISC_DDRPHY_INIT_WDG_CMP    (0xffffffff << 0)   /* DDRPHY init wdg will timeout when reach cmp */

/*
 * DDR controller
 */
#define MISC_DDR_CTL        0x80    /* DDR controller */
#define MISC_DDR_PWROK          (1 << 0)    /* 1:assert ddr_pwrok, 0:deassert ddr_pwrok */
#define MISC_DDR_ARBITER_TIMEOUT_VAL    (0xff << 8) /* ddr arbiter timeout value */
#define MISC_DDR_CTL_PCLK_EN        (1 << 17)   /* 1:always enable APB clock of DDR controller, 0:gate by psel */

/*
 * mode counter control
 */
#define MODE_CNT_CTL        0x90    /* mode counter control */
#define MODE_CNT_CTL_EN         (1 << 0)    /* 1:enable mode counter,0:disable*/
#define MODE_CNT_CTL_CLR        (1 << 1)    /* write 1 to clear mode counter value, overflow, valid flag */

#define MODE_CNT_ST     0x94    /* mode counter status */
#define MODE_CNT_ST_SELFREF_MODE_ENTER  (1 << 0)    /* 1:indicate ddr enter slefrefresh mode, write 1 to clear */
#define MODE_CNT_ST_VALUE_VALID     (1 << 1)    /* 1:indicate mode counter mersure done and value valid, RO */
#define MODE_CNT_ST_OVERFLOW        (1 << 2)    /* 1:indicate mode counter overflow, RO */

#define MODE_CNT        0x98    /* mode counter value */
#define MODE_CNT_VAL        (0xffffffff << 0)   /* mode counter value, RO */

/*
 * misc status register
 */
#define MISC_ST         0xA0    /* RO */
#define MISC_ST_BASE_CNT_TIMEOUT    (1 << 4)    /* 1:base counter timeout */
#define MISC_ST_WDG_TIMEOUT     (1 << 16)   /* 1:read watchdog timeout */
#define MISC_ST_DDRPHY_INIT_WDG_TIMEOUT (1 << 17)   /* 1:DDRPHY init watchdog timeout */

/*
 * misc interrupt ctrl register
 */
#define MISC_INT_CTL        0xA4
#define MISC_INT_CTL_BASE_CNT_TIMEOUT_INT_EN        (1 << 0)    /* 1:enable base counter timeout interrupt */
#define MISC_INT_CTL_MODE_CNT_INT_EN            (1 << 1)    /* 1:enable mode counter int when valid or ovf */
#define MISC_INT_CTL_WDG_TIMEOUT_INT_EN         (1 << 16)   /* 1:enable read wdg timeout interrupt */
#define MISC_INT_CTL_DDRPHY_INIT_WDG_TIMEOUT_INT_EN (1 << 17)   /* 1:enable DDRPHY init wdg timeout interrupt */

/*
 * pa mask
 */
#define PA_MSK          0xC0
#define PA_MSK_RMASK        (3 << 0)    /* pa_rmask */
#define PA_MSK_WMASK        (1 << 2)    /* pa_wmask */

/*
 * axi counter control register
 */
#define AXI_CNT_CTL0        0x100   /* axi counter enable for 16 axi counter groups */
#define AXI_CNT_EN      (0xffff << 0)   /* 1:enable the specific axi counter group */

#define AXI_CNT_CTL1        0x104   /* axi counter interrupt & auto restart */
#define AXI_CNT_AUTO_RESTART_EN (1 << 0)    /* 1:enable axi counter auto restart */
#define AXI_CNT_INT_EN      (1 << 1)    /* 1:enable axi counter interrupt when round counter timeout or reach thres*/


/*
 * axi master read threshold for master i(0~7)
 */
#define AXI_MST0_RD_THR     0x110   /* axi read counter threshold, will trigger axi counter stop for first 8 group */
#define AXI_MST0_WR_THR     0x114   /* axi write counter threshold */
#define AXI_MST0_RD_BCNT_THR    0x118   /* axi read burst counter threshold */
#define AXI_MST0_WR_BCNT_THR    0x11C   /* axi write burst counter threshold */

/*
 * axi transaction calculate for master i(0~15) 2 master ID(id0, id1) for 1 axi counter
 */
#define AXI_MST0_ID0        0x200   /* calculate axi transaction for which master ID matches mst_id0*/
#define AXI_MST0_ID0_MSK    0x204   /* 1:not mask the specific id bit during master id comparison */
#define AXI_MST0_ID1        0x208   /* calculate axi transaction for which master ID matches mst_id1*/
#define AXI_MST0_ID1_MSK    0x20C   /* 1:not mask the specific id bit during master id comparison*/

/*
 * axi auto restart rounds count, RO
 */
#define AXI_RND_CNT     0x1000  /* indicate how many rounds axi counter auto-restart have been performed */

/*
 * axi counter reach threshold
 */
#define AXI_CNT_ST      0x1004  /* master i(0~7), RO */
#define AXI_CNT_ST_REACH_THR        (0xff << 0) /* 1: axi counter reach threshold */

/*
 * axi master transaction counter for master i(0~15), RO
 */
#define AXI_MST0_RD_CNT     0x1010  /* read transaction counter value */
#define AXI_MST0_WR_CNT     0x1014  /* write transaction counter value */
#define AXI_MST0_RD_BCNTL   0x1018  /* read burst counter value low 32bit */
#define AXI_MST0_RD_BCNTH   0x101C  /* read burst counter value hi 8bit */
#define AXI_MST0_RD_BCNTH_VAL       (0xff << 0) /* read burst counter value hi 8bit */
#define AXI_MST0_WR_BCNTL   0x1020  /* write burst counter value low 32bit */
#define AXI_MST0_WR_BCNTH   0x1024  /* write burst counter value hi 8bit */
#define AXI_MST0_WR_BCNTH_VAL       (0xff << 0) /* write burst counter value hi 8bit */


#define FPGA_DDR_PFMON_INT  218 //TMP

#define PFMON_DMA_MODE 0
#define PFMON_IRQ_MODE 1

#define MAX_CHANNEL 16

#define BASE_CNT_OFFSET 0x1010


enum master_id {
    MASTER_SAF_PLATFORM = 0,
    MASTER_SEC_PLATFORM,
    MASTER_MP_PLATFORM,
    MASTER_AP1,
    MASTER_AP2,
    MASTER_VDSP,
    MASTER_ADSP,
    MASTER_TCU,
    MASTER_DMA1,
    MASTER_DMA2,
    MASTER_DMA3,
    MASTER_DMA4,
    MASTER_DMA5,
    MASTER_DMA6,
    MASTER_DMA7,
    MASTER_DMA8,
    MASTER_CSI1,
    MASTER_CSI2,
    MASTER_CSI3,
    MASTER_DC1,
    MASTER_DC2,
    MASTER_DC3,
    MASTER_DC4,
    MASTER_DP1,
    MASTER_DP2,
    MASTER_DP3,
    MASTER_DC5,
    MASTER_G2D1,
    MASTER_G2D2,
    MASTER_VPU1,
    MASTER_VPU2,
    MASTER_MJPEG,
    MASTER_MSHC1,
    MASTER_MSHC2,
    MASTER_MSHC3,
    MASTER_MSHC4,
    MASTER_ENET_QOS1,
    MASTER_ENET_QOS2,
    MASTER_USB1,
    MASTER_USB2,
    MASTER_AI,
    MASTER_RESERVED2,
    MASTER_RESERVED3,
    MASTER_RESERVED4,
    MASTER_RESERVED5,
    MASTER_RESERVED6,
    MASTER_RESERVED7,
    MASTER_CE1,
    MASTER_CE2_VCE1,
    MASTER_CE2_VCE2,
    MASTER_CE2_VCE3,
    MASTER_CE2_VCE4,
    MASTER_CE2_VCE5,
    MASTER_CE2_VCE6,
    MASTER_CE2_VCE7,
    MASTER_CE2_VCE8,
    MASTER_GPU1_OS1,
    MASTER_GPU1_OS2,
    MASTER_GPU1_OS3,
    MASTER_GPU1_OS4,
    MASTER_GPU1_OS5,
    MASTER_GPU1_OS6,
    MASTER_GPU1_OS7,
    MASTER_GPU1_OS8,
    MASTER_GPU2_OS1,
    MASTER_GPU2_OS2,
    MASTER_GPU2_OS3,
    MASTER_GPU2_OS4,
    MASTER_GPU2_OS5,
    MASTER_GPU2_OS6,
    MASTER_GPU2_OS7,
    MASTER_GPU2_OS8,
    MASTER_PTB,
    MASTER_CSSYS,
    MASTER_RESERVED8,
    MASTER_RESERVED9,
    MASTER_RESERVED10,
    MASTER_RESERVED11,
    MASTER_RESERVED12,
    MASTER_RESERVED13,
    MASTER_PCIE1_0,
    MASTER_PCIE1_1,
    MASTER_PCIE1_2,
    MASTER_PCIE1_3,
    MASTER_PCIE1_4,
    MASTER_PCIE1_5,
    MASTER_PCIE1_6,
    MASTER_PCIE1_7,
    MASTER_PCIE1_8,
    MASTER_PCIE1_9,
    MASTER_PCIE1_10,
    MASTER_PCIE1_11,
    MASTER_PCIE1_12,
    MASTER_PCIE1_13,
    MASTER_PCIE1_14,
    MASTER_PCIE1_15,
    MASTER_PCIE2_0,
    MASTER_PCIE2_1,
    MASTER_PCIE2_2,
    MASTER_PCIE2_3,
    MASTER_PCIE2_4,
    MASTER_PCIE2_5,
    MASTER_PCIE2_6,
    MASTER_PCIE2_7,
    MASTER_RESERVED14,
    MASTER_RESERVED15,
    MASTER_RESERVED16,
    MASTER_RESERVED17,
    MASTER_RESERVED18,
    MASTER_RESERVED19,
    MASTER_RESERVED20,
    MASTER_RESERVED21,
    MASTER_RESERVED22,
    MASTER_RESERVED23,
    MASTER_RESERVED24,
    MASTER_RESERVED25,
    MASTER_RESERVED26,
    MASTER_RESERVED27,
    MASTER_RESERVED28,
    MASTER_RESERVED29,
    MASTER_RESERVED30,
    MASTER_RESERVED31,
    MASTER_RESERVED32,
    MASTER_RESERVED33,
    MASTER_RESERVED34,
    MASTER_RESERVED35,
    MASTER_RESERVED36,
    MASTER_RESERVED37,
};

enum pfm_mode {
    PFM_DMA_MODE = 0,
    PFM_AUTO_RESTART = 1,
    PFM_IRQ_MODE = 2,
};

struct pfm_record {
    uint32_t timestamp;
    uint32_t read_count;
    uint32_t write_count;
    uint32_t read_burst_count_l;
    uint32_t read_burst_count_h;  //bit 7:0
    uint32_t write_burst_count_l;
    uint32_t write_burst_count_h; //bit 7:0
};

struct pfm_global_config {
    uint32_t active_group; //bitmap of group,total 16
    uint32_t time_window;
    uint32_t rounds;
    uint32_t pool_size;
    uint32_t pfm_collect_size;
    enum pfm_mode mode;
};

struct pfm_channel_config {
    uint32_t current_group;
    bool initialized;

    uint32_t sum_read;
    uint32_t sum_write;
    uint32_t sum_read_burst;
    uint32_t sum_write_burst;

    enum master_id master_id0;
    enum master_id master_id1;
    uint32_t master_msk_id0;
    uint32_t master_msk_id1;
    //only apply to first 8 AXI counter group
    uint32_t read_threshold;
    uint32_t write_threshold;
    uint32_t read_burst_threshold;
    uint32_t write_burst_threshold;
};


#define time_window_bit  0x10
#define ddr_arbiter_timeout 0x8000
#define read_watchdog_bit 0x10000
#define DDRPHY_watchdog_bit 0x20000


/* APP defined */
#define DDR_CLOCK 1066000000
#define DDR_PFM_BASE_COUNT_WINDOW_MS   (10*1066000ul)

#define PFM_POOL_BASE
#define PFM_POOL_SIZE

int32_t pfm_poll_handler(void);
void ddr_profiling_init(void);
void print_channel_record(int channel);

#endif
