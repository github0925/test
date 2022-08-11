/*
 * bipc.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __BIPC_H__
#define __BIPC_H__

#include <stdbool.h>

// #include "__regs_ap_bipc_vsn.h"
// #include "__regs_ap_bipc_ddr.h"
// #include "__regs_ap_bipc_enet.h"

/*
 * BIP converters are able to report correctable and non-correctable
 * error interrupts.
 */
enum bipc {
    BIPC_VP6 = 0,
    BIPC_DDR,
    BIPC_ENET,
    BIPC_MAX,
};

enum bipc_axichnl {
    BIPC_AXICHNL_AR,   /* read address */
    BIPC_AXICHNL_AW,   /* write address */
    BIPC_AXICHNL_W,    /* write */
    BIPC_AXICHNL_R,    /* read */
    BIPC_AXICHNL_B,    /* b */
    BIPC_AXICHNL_COR,  /* corrected error */
    BIPC_AXICHNL_MAX,
};

void bipc_init(enum bipc bipc);
void bipc_monitor_signal_err(enum bipc bipc, int chnl_idx,
                             enum bipc_axichnl axichnl, int bit, bool enable);
bool bipc_is_signal_err(enum bipc bipc, int chnl_idx,
                        enum bipc_axichnl axichnl, int bit);
void bipc_clear_signal_err(enum bipc bipc, int chnl_idx,
                           enum bipc_axichnl axichnl, int bit);

#endif
