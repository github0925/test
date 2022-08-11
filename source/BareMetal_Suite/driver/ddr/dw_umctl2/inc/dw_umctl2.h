/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 *******************************************************/
#ifndef __DW_UMCTL2_H__
#define __DW_UMCTL2_H__

#include <stdint.h>
#include <sys/types.h>
#include <debug.h>
#include "DWC_ddr_umctl2_reg.h"
#include "dwc_ddrphy_top_reg.h"
#include "ddr_init_helper.h"

#define DDR_FW_ENTRIES_CNT_MAX 8
#ifdef DDR_DEBUG
#define DDR_DBG(fmt, args...)   DBG(fmt, ##args)
#else
#define DDR_DBG(fmt, args...)
#endif

typedef struct {
    uint32_t id;
    const char *str;
} train_string_t;

uint32_t dwc_ddrphy_fw_monitor_loop(uint32_t fw);
void *ddrphy_apb_memcpy(uint32_t *to, uint16_t *from, size_t sz);
void _dwc_ddrphy_phyinit_userCustom_H_readMsgBlock_ (int Train2D);
void _timing_reg_update_after_training_(void);
int load_ddr_training_fw(uint32_t mem, ddrphy_fw_e fw);
int load_ddr_training_fw_piecewise(const char *name, const char *pt_name, uint32_t *to, uint64_t * entry_sz, bool is_phy_apb);
void run_ddr_diag(int diag_flag);

#endif  /* __DW_UMCTL2_H__ */

