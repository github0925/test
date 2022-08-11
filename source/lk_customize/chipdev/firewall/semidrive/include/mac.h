/*
 * Copyright (c) Semidrive
 */

#ifndef _MAC_H
#define _MAC_H

#include <sys/types.h>
#include <mac_reg.h>
#include <fw_reg.h>

/* MAC Global control */
typedef struct mac_global_ctl_t {
    addr_t    base_addr;
    uint32_t  dom_cfg_lock;
    uint32_t  dom_cfg_mode;
    uint32_t  per_ck_dis_lock;
    uint32_t  per_ck_dis;
    uint32_t  dom_per_ck_lock;
    uint32_t  dom_per_ck_en;
} mac_global_ctl_t;

/* Resource Manager */
typedef struct mac_res_mgr_t {
    addr_t    base_addr;
    uint32_t  mid_lock;
    uint32_t  mid_en;
    uint32_t  pri_per_lock;
    uint32_t  pri_per;
    uint32_t  pri_per_en;
    uint32_t  sec_per_lock;
    uint32_t  sec_per;
    uint32_t  sec_per_en;
    uint32_t  did_lock;
    uint32_t  did;
    uint32_t  did_en;
    uint32_t  res_mgr_en_lock;
    uint32_t  res_mgr_en;
    uint32_t  mid[MASTER_ID_SIZE];
} mac_res_mgr_t;

/* Group manager, support 8 groups */
typedef struct mac_grp_mgr_t {
    addr_t    base_addr;
    uint32_t  mid_lock;
    uint32_t  mid_en;
    uint32_t  pri_per_lock;
    uint32_t  pri_per;
    uint32_t  pri_per_en;
    uint32_t  sec_per_lock;
    uint32_t  sec_per;
    uint32_t  sec_per_en;
    uint32_t  did_lock ;
    uint32_t  did;
    uint32_t  did_en;
    uint32_t  grp_mgr_en_lock;
    uint32_t  grp_mgr_en;
    uint32_t  mid[MASTER_ID_SIZE];
} mac_grp_mgr_t;

/* Domain owner, support 16 domains */
typedef struct mac_dom_owner_t {
    addr_t    base_addr;
    uint32_t  gid_lock;
    uint32_t  gid_gid;
    uint32_t  mid_lock;
    uint32_t  mid_en;
    uint32_t  pri_per_lock;
    uint32_t  pri_per;
    uint32_t  pri_per_en;
    uint32_t  sec_per_lock;
    uint32_t  sec_per;
    uint32_t  sec_per_en;
    uint32_t  own_en_lock;
    uint32_t  own_en;
    uint32_t  mid[MASTER_ID_SIZE];
} mac_dom_owner_t;

/* Master attribute, support 64/128 masters */
typedef struct mac_master_attr_t {
    addr_t    base_addr;
    uint32_t  mda_lock;
    uint32_t  mda_did;
    uint32_t  srid_lock;
    uint32_t  srid;
    uint32_t  pri_lock;
    uint32_t  pri_ov_en;
    uint32_t  pri;
    uint32_t  sec_lock;
    uint32_t  sec_ov_en;
    uint32_t  sec;
} mac_master_attr_t;

#endif
