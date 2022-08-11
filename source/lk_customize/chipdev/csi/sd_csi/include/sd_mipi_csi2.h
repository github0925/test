/*
* sd_mipi_csi2.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* csi interface struct header
*
* Revision History:
* -----------------
* 0.1, 12/21/2018 init version
*/

#ifndef __SD_MIPI_CSI2_H__
#define __SD_MIPI_CSI2_H__

#include <platform/debug.h>
#include <trace.h>
#include <assert.h>

#include "dw_mipi_csi2_reg.h"


#define DT_YUV420_LE_8_O 0x18
#define DT_YUV420_10_O 0x19
#define DT_YUV420_LE_8_E 0x1C
#define DT_YUV420_10_E 0x1D
#define DT_YUV422_8 0x1E
#define DT_YUV422_10 0x1F
#define DT_RGB444 0x20
#define DT_RGB555 0x21
#define DT_RGB565 0x22
#define DT_RGB666 0x23
#define DT_RGB888 0x24
#define DT_RAW8 0x2A
#define DT_RAW10 0x2B



#define CSI_IMG_STORE_MEM0_ADDR 0x50000000
#define CSI_INDEX_MEM_OFFSET    0x10000000
#define CSI_GRP_CHN_OFFSET            0x20
#define CSI_GRP_IMG_OFFSET            0x80

#define CSI_IMG_STORE_GAP        0x1000000   // 16MB per frame, for img0-3 in each csi
#define CSI_LINE_STRIDE            0x10000   // 64KB per line
#define CSI_PLANER_GAP            0x400000   // 4MB for planer gap


typedef struct _int_reg_con_ {
    volatile uint32_t int_st;
    volatile uint32_t int_msk;
    volatile uint32_t int_force;
    volatile uint32_t reserve;
} int_reg_con;

typedef struct _ipi_info_ {
    volatile uint32_t ipi_mode;
    volatile uint32_t ipi_vcid;
    volatile uint32_t ipi_data_type;
    volatile uint32_t ipi_mem_flush;
    volatile uint32_t ipi_hsa_time;
    volatile uint32_t ipi_hbp_time;
    volatile uint32_t ipi_hsd_time;
    volatile uint32_t ipi_adv_features;
} ipi_info;

typedef struct _mipi_csi2_reg_type_ {
    volatile uint32_t version; /* 0x0 */
    volatile uint32_t n_lanes;
    volatile uint32_t csi2_resetn;
    volatile uint32_t int_st_main;
    volatile uint32_t data_ids_1; /* 0x10 */
    volatile uint32_t data_ids_2;
    volatile uint32_t reserve_18_2c[5]; /* 0x18 ~ 0x2c */
    volatile uint32_t int_st_ap_main;
    volatile uint32_t reserve_30_3c[4]; /* 0x30 ~ 0x3c */
    volatile uint32_t phy_shutdownz; /* 0x40 */
    volatile uint32_t dphy_rstz;
    volatile uint32_t phy_rx;
    volatile uint32_t phy_stopstate;
    volatile uint32_t phy_test_ctrl0; /* 0x50 */
    volatile uint32_t phy_test_ctrl1;
    volatile uint32_t reserve_58_7c[10]; /* 0x58 ~ 0x7c */
    volatile uint32_t ipi_mode; /* 0x80 */
    volatile uint32_t ipi_vcid;
    volatile uint32_t ipi_data_type;
    volatile uint32_t ipi_mem_flush;
    volatile uint32_t ipi_hsa_time; /* 0x90 */
    volatile uint32_t ipi_hbp_time;
    volatile uint32_t ipi_hsd_time;
    volatile uint32_t ipi_hline_time;
    volatile uint32_t ipi_softrstn; /* 0xa0 */
    volatile uint32_t reserve_a4;
    volatile uint32_t reserve_a8;
    volatile uint32_t ipi_adv_features;
    volatile uint32_t ipi_vsa_lines; /* 0xb0 */
    volatile uint32_t ipi_vbp_lines;
    volatile uint32_t ipi_vfp_lines;
    volatile uint32_t ipi_vactive_lines;
    volatile uint32_t reserve_c0; /* 0xc0 */
    volatile uint32_t reserve_c4;
    volatile uint32_t reserve_c8;
    volatile uint32_t phy_cal;
    volatile uint32_t reserve_d0_dc[4]; /* 0xd0 ~ 0xdc */
    int_reg_con int_reg_phy_fatal; /* 0xe0 */
    int_reg_con int_reg_pkt_fatal; /* 0xf0 */
    int_reg_con int_reg_frame_fatal; /* 0x100 */
    int_reg_con int_reg_phy; /* 0x110 */
    int_reg_con int_reg_pkt; /* 0x120 */
    int_reg_con int_reg_line; /* 0x130 */
    int_reg_con int_reg_ipi; /* 0x140 */
    int_reg_con int_reg_ipi2; /* 0x150 */
    int_reg_con int_reg_ipi3; /* 0x160 */
    int_reg_con int_reg_ipi4; /* 0x170 */
    int_reg_con int_reg_ap_generic; /* 0x180 */
    int_reg_con int_reg_ap_ipi; /* 0x190 */
    int_reg_con int_reg_ap_ipi2; /* 0x1a0 */
    int_reg_con int_reg_ap_ipi3; /* 0x1b0 */
    int_reg_con int_reg_ap_ipi4; /* 0x1c0 */
    volatile uint32_t reserve_1d0_1fc[12]; /* 0x1d0 ~ 0x1fc */
    ipi_info ipi2_info; /* 0x200 ~ 0x21c */
    ipi_info ipi3_info; /* 0x220 ~ 0x23c */
    ipi_info ipi4_info; /* 0x240 ~ 0x25c */
    volatile uint32_t reserve_260_2dc[32]; /* 0x260 ~ 0x2dc*/
    volatile uint32_t ipi_ram_err_log_ap; /* 0x2e0 */
    volatile uint32_t ipi_ram_err_addr_ap;
    volatile uint32_t ipi2_ram_err_log_ap;
    volatile uint32_t ipi2_ram_err_addr_ap;
    volatile uint32_t ipi3_ram_err_log_ap; /* 0x2f0 */
    volatile uint32_t ipi3_ram_err_addr_ap;
    volatile uint32_t ipi4_ram_err_log_ap;
    volatile uint32_t ipi4_ram_err_addr_ap;
    volatile uint32_t scrambling; /* 0x300 */
    volatile uint32_t scrambling_seed1;
    volatile uint32_t scrambling_seed2;
    volatile uint32_t scrambling_seed3;
    volatile uint32_t scrambling_seed4; /* 0x310 */
    volatile uint32_t reserve_314_33c[11]; /* 0x314 ~ 0x33c */
    volatile uint32_t n_sync; /* 0x340 */
    volatile uint32_t err_inj_ctrl_ap;
    volatile uint32_t err_inj_chk_msk_ap;
    volatile uint32_t err_inj_dh32_msk_ap;
    volatile uint32_t err_inj_dl32_msk_ap; /* 0x350 */
    volatile uint32_t err_inj_st_ap;
    volatile uint32_t reserve_358;
    volatile uint32_t reserve_35c;
    int_reg_con int_reg_fap_phy_fatal; /* 0x360 */
    int_reg_con int_reg_fap_pkt_fatal; /* 0x370 */
    int_reg_con int_reg_fap_frame_fatal; /* 0x380 */
    int_reg_con int_reg_fap_phy; /* 0x390 */
    int_reg_con int_reg_fap_pkt; /* 0x3a0 */
    int_reg_con int_reg_fap_line; /* 0x3b0 */
    int_reg_con int_reg_fap_ipi; /* 0x3c0 */
    int_reg_con int_reg_fap_ipi2; /* 0x3d0 */
    int_reg_con int_reg_fap_ipi3; /* 0x3e0 */
    int_reg_con int_reg_fap_ipi4; /* 0x3f0 */
} mipi_csi2_reg_type;

typedef struct _mipi_csi_device {
    vaddr_t reg_base;
    int host_id;
    int freq_index;
    bool enabled;
} mipi_csi_device;

struct phy_freqrange {
    int index;
    int range_l;
    int range_h;
};

void csi_cfg_common(int index);
mipi_csi_device *sd_mipi_csi2_host_init(uint32_t id, addr_t addr);
void sd_mipi_csi2_init_cfg (mipi_csi_device *dev);

status_t sd_mipi_csi2_set_hline_time(mipi_csi_device *dev, uint32_t hsa,
                                     uint32_t hbp, uint32_t hsd);

status_t sd_mipi_csi2_set_phy_freq(mipi_csi_device *dev, uint32_t freq,
                                   uint32_t lanes);


void sd_mipi_csi2_start (mipi_csi_device *dev);

void sd_mipi_csi2_stop(mipi_csi_device *dev);

void get_phy_status(mipi_csi_device *dev, u32 *power_state,
                    u32 *stop_state);



#endif  //__SD_MIPI_CSI2_H__
