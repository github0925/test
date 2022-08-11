/*
 * Copyright (c) Semidrive
 */

#ifndef _MPC_H
#define _MPC_H

#include <sys/types.h>
#include "fw_reg.h"

#define MPC_ADDR_SIZE               0x1000
#define MPC_COUNT                   27

#define IRAM5_RANGE_COUNT           8
#define IRAM4_RANGE_COUNT           16
#define MU_RANGE_COUNT              16
#define GPU2_RANGE_COUNT            16
#define GPU1_RANGE_COUNT            16
#define RESERVED2                   0
#define DDR_MEM_RANGE_COUNT         16
#define PCIE2_RANGE_COUNT           16
#define PCIE1_RANGE_COUNT           16
#define GIC5_RANGE_COUNT            1
#define GIC4_RANGE_COUNT            1
#define GIC3_RANGE_COUNT            1
#define GIC2_RANGE_COUNT            1
#define OSPI2_RANGE_COUNT           16
#define OSPI1_RANGE_COUNT           16
#define ROMC2_RANGE_COUNT           16
#define MP_PLATFORM_RANGE_COUNT     16
#define SEC_PLATFORM_RANGE_COUNT    16
#define IRAM3_RANGE_COUNT           16
#define IRAM2_RANGE_COUNT           16
#define IRAM1_RANGE_COUNT           16
#define CE2_RANGE_COUNT             16
#define CE1_RANGE_COUNT             16
#define VDSP_RANGE_COUNT            16
#define DMA1_RANGE_COUNT            1
#define ADSP1_RANGE_COUNT           16
#define AXI2AHB_SEC_RANGE_COUNT     16
#define MPC_RNG_COUNT               IRAM4_RANGE_COUNT + MU_RANGE_COUNT + GPU2_RANGE_COUNT + GPU1_RANGE_COUNT \
                                    + DDR_MEM_RANGE_COUNT + PCIE2_RANGE_COUNT + PCIE1_RANGE_COUNT \
                                    + GIC5_RANGE_COUNT + GIC4_RANGE_COUNT + GIC3_RANGE_COUNT + GIC2_RANGE_COUNT \
                                    + OSPI2_RANGE_COUNT + OSPI1_RANGE_COUNT + ROMC2_RANGE_COUNT + MP_PLATFORM_RANGE_COUNT \
                                    + SEC_PLATFORM_RANGE_COUNT + IRAM3_RANGE_COUNT + IRAM2_RANGE_COUNT + IRAM1_RANGE_COUNT \
                                    + CE2_RANGE_COUNT + CE1_RANGE_COUNT + VDSP_RANGE_COUNT + DMA1_RANGE_COUNT \
                                    + ADSP1_RANGE_COUNT + AXI2AHB_SEC_RANGE_COUNT
/* Domain owner for peripheral,
   configured only by resource and group manager */
typedef struct mpc_dom_assign_t {
    addr_t    base_addr;
    uint32_t  lock;
    uint32_t  set;
    uint32_t  did;
} mpc_dom_assign_t;

/* domain permission configration,
   configured only by resource manager and domain owner. */
typedef struct mpc_dom_per_t {
    addr_t    base_addr;
    uint32_t  dom_lock[DOM_MAX_COUNT];
    uint32_t  dom_per[DOM_MAX_COUNT];
    uint32_t  dom_en[DOM_MAX_COUNT];
} mpc_dom_per_t;

/* secure permission configuration,
   configured by resource/group manager. */
typedef struct mpc_sec_per_t {
    addr_t    base_addr;
    uint32_t  dom_lock[DOM_MAX_COUNT];
    uint32_t  dom_nse_per[DOM_MAX_COUNT];
    uint32_t  dom_sec_per[DOM_MAX_COUNT];
    uint32_t  dom_sec_en[DOM_MAX_COUNT];
} mpc_sec_per_t;

/* privilege permission configuration,
   configured only by resource/group manager. */
typedef struct mpc_pri_per_t {
    addr_t    base_addr;
    uint32_t  dom_lock[DOM_MAX_COUNT];
    uint32_t  dom_use_per[DOM_MAX_COUNT];
    uint32_t  dom_pri_per[DOM_MAX_COUNT];
    uint32_t  dom_pri_en[DOM_MAX_COUNT];
} mpc_pri_per_t;

/* privilege permission configuration,
   configured only by resource/group manager.
   resolution is 4k bytes */
typedef struct mpc_rgn_addr_range_t {
    addr_t    base_addr;
    addr_t    start_addr;
    addr_t    end_addr;
    uint32_t  lock;
    uint32_t  rng_en;
} mpc_rgn_addr_range_t;

/* privilege permission configuration,
   configured only by resource manager?
   resolution is 4k bytes */
typedef struct mpc_rgn_addr_limit_t {
    addr_t    base_addr;
    addr_t    low_limit_addr;
    addr_t    up_limit_addr;
    uint32_t  lock;
} mpc_rgn_addr_limit_t;

typedef struct mpc_range_t {
    mpc_dom_assign_t        mpc_dom_assign;
    mpc_dom_per_t           mpc_dom_per;
    mpc_sec_per_t           mpc_sec_per;
    mpc_pri_per_t           mpc_pri_per;
    mpc_rgn_addr_range_t    mpc_rgn_addr_range;
    mpc_rgn_addr_limit_t    mpc_rgn_addr_limit;
} mpc_range_t;

typedef struct mpc_uncerr_int_t {
    addr_t base_addr;
    uint32_t arid;
    uint32_t araddr;
    uint32_t arctl0;
    uint32_t arctl1;
    uint32_t arvalid;
    uint32_t arready;
    uint32_t awid;
    uint32_t awaddr;
    uint32_t awctl0;
    uint32_t awctl1;
    uint32_t awvalid;
    uint32_t awready;
    uint32_t rid;
    uint32_t rctl;
    uint32_t reobi;
    uint32_t rvalid;
    uint32_t rready;
    uint32_t wctl;
    uint32_t wvalid;
    uint32_t wready;
    uint32_t bid;
    uint32_t bvalid;
    uint32_t bready;
} mpc_uncerr_int_t;

typedef struct mpc_corerr_int_t {
    addr_t base_addr;
    uint32_t araddr;
    uint32_t awaddr;
} mpc_corerr_int_t;

#endif
