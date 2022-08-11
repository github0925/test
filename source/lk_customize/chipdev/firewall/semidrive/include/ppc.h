/*
 * Copyright (c) Semidrive
 */

#ifndef _PPC_H
#define _PPC_H

#include <sys/types.h>
#include <fw_reg.h>
#include <assert.h>

#define PPC_ADDR_SIZE               0x2000
#define PPC_COUNT                   14
#define SLAVE_MAX_COUNT             64

#define MUX1_SLAVE_SLOT_COUNT            64
#define MUX2_SLAVE_SLOT_COUNT            64
#define MUX3_SLAVE_SLOT_COUNT            64
#define MUX4_SLAVE_SLOT_COUNT            64
#define MUX5_SLAVE_SLOT_COUNT            32
#define MUX6_SLAVE_SLOT_COUNT            32
#define MUX7_SLAVE_SLOT_COUNT            16
#define MUX8_SLAVE_SLOT_COUNT            32
#define DDR_CFG_SLAVE_SLOT_COUNT         1
#define SMMU_SLAVE_SLOT_COUNT            64
#define CE2_VIRT_SLOT_COUNT              16
#define SCR4K_SID_SLOT_COUNT             1
#define SCR4K_SSID_SLOT_COUNT            1
#define CSSYS_SLOT_COUNT                 1

#define MUX1_SLAVE_COUNT            60
#define MUX2_SLAVE_COUNT            52
#define MUX3_SLAVE_COUNT            64
#define MUX4_SLAVE_COUNT            61
#define MUX5_SLAVE_COUNT            32
#define MUX6_SLAVE_COUNT            15
#define MUX7_SLAVE_COUNT            12
#define MUX8_SLAVE_COUNT            22
#define DDR_CFG_SLAVE_COUNT         1
#define SMMU_SLAVE_COUNT            18
#define CE2_VIRT_COUNT              8
#define SCR4K_SID_COUNT             1
#define SCR4K_SSID_COUNT            1
#define CSSYS_COUNT                 1

#define MUX1_RESERVED               15
#define MUX2_RESERVED               14
#define MUX3_RESERVED               16
#define MUX4_RESERVED               30
#define MUX5_RESERVED               8
#define MUX6_RESERVED               1
#define MUX7_RESERVED               2
#define MUX8_RESERVED               5
#define DDR_CFG_RESERVED            0
#define SMMU_RESERVED               0
#define CE2_RESERVED                0
#define SCR4K_SID_RESERVED          0
#define SCR4K_SSID_RESERVED         0
#define CSSYS_RESERVED              0

#define RESERVED_COUNT              MUX1_RESERVED + MUX2_RESERVED + MUX3_RESERVED \
                                    + MUX4_RESERVED + MUX5_RESERVED + MUX6_RESERVED \
                                    + MUX7_RESERVED + MUX8_RESERVED + DDR_CFG_RESERVED \
                                    + SMMU_RESERVED + CE2_RESERVED + SCR4K_SID_RESERVED \
                                    + SCR4K_SSID_RESERVED + CSSYS_RESERVED

#define SLAVE_COUNT                 MUX1_SLAVE_COUNT + MUX2_SLAVE_COUNT + MUX3_SLAVE_COUNT \
                                    + MUX4_SLAVE_COUNT + MUX5_SLAVE_COUNT + MUX6_SLAVE_COUNT \
                                    + MUX7_SLAVE_COUNT + MUX8_SLAVE_COUNT + DDR_CFG_SLAVE_COUNT \
                                    + SMMU_SLAVE_COUNT + CE2_VIRT_COUNT + SCR4K_SID_COUNT \
                                    + SCR4K_SSID_COUNT + CSSYS_COUNT

#define SLAVE_COUNT_CFG             SLAVE_COUNT - RESERVED_COUNT

extern const bool mux1_slave_enable[MUX1_SLAVE_COUNT];
extern const bool mux2_slave_enable[MUX2_SLAVE_COUNT];
extern const bool mux3_slave_enable[MUX3_SLAVE_COUNT];
extern const bool mux4_slave_enable[MUX4_SLAVE_COUNT];
extern const bool mux5_slave_enable[MUX5_SLAVE_COUNT];
extern const bool mux6_slave_enable[MUX6_SLAVE_COUNT];
extern const bool mux7_slave_enable[MUX7_SLAVE_COUNT];
extern const bool mux8_slave_enable[MUX8_SLAVE_COUNT];
extern const bool ddr_cfg_slave_enable[DDR_CFG_SLAVE_COUNT];
extern const bool smmu_slave_enable[SMMU_SLAVE_COUNT];
extern const bool ce2_slave_enable[CE2_VIRT_COUNT];
extern const bool scr4k_sid_slave_enable[SCR4K_SID_COUNT];
extern const bool scr4k_ssid_slave_enable[SCR4K_SSID_COUNT];
extern const bool cssys_slave_enable[CSSYS_COUNT];

/* Domain owner for peripheral,
   configured only by resource and group manager. */
typedef struct ppc_dom_assign_t {
    addr_t    base_addr;
    uint32_t  lock;
    uint32_t  set;
    uint32_t  did;
} ppc_dom_assign_t;

/* domain permission configration,
   configured only by resource manager and domain owner. */
typedef struct ppc_dom_per_t {
    addr_t    base_addr;
    uint32_t  dom_lock[DOM_MAX_COUNT];
    uint32_t  dom_per[DOM_MAX_COUNT];
    uint32_t  dom_en[DOM_MAX_COUNT];
} ppc_dom_per_t;

/* secure permission configuration,
   configured only by resource/group manager. */
typedef struct ppc_sec_per_t {
    addr_t    base_addr;
    uint32_t  dom_lock[DOM_MAX_COUNT];
    uint32_t  dom_nse_per[DOM_MAX_COUNT];
    uint32_t  dom_sec_per[DOM_MAX_COUNT];
    uint32_t  dom_sec_en[DOM_MAX_COUNT];
} ppc_sec_per_t;

/* privilege permission configuration,
   configured only by resource/group manager. */
typedef struct ppc_pri_per_t {
    addr_t    base_addr;
    uint32_t  dom_lock[DOM_MAX_COUNT];
    uint32_t  dom_use_per[DOM_MAX_COUNT];
    uint32_t  dom_pri_per[DOM_MAX_COUNT];
    uint32_t  dom_pri_en[DOM_MAX_COUNT];
} ppc_pri_per_t;

/* privilege permission configuration,
   configured only by resource/group manager.
   resolution is 4k bytes */
typedef struct ppc_addr_range_t {
    addr_t    base_addr;
    addr_t    start_addr;
    addr_t    end_addr;
    uint32_t  lock;
    uint32_t  rng_en;
} ppc_addr_range_t;

typedef struct ppc_slave_t {
    ppc_dom_assign_t    ppc_dom_assign;
    ppc_dom_per_t       ppc_dom_per;
    ppc_sec_per_t       ppc_sec_per;
    ppc_pri_per_t       ppc_pri_per;
} ppc_slave_t;

typedef struct ppc_range_t {
    ppc_slave_t         ppc_slave;
    ppc_addr_range_t    ppc_addr_range;
} ppc_range_t;

#endif
