/*
 * res.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Domain resource management source file.
 *
 * Revision History:
 * -----------------
 */
#include "chip_res.h"
#include "res.h"

/* Generated resources of this domain. */
#include "domain_res.h"
#include "__regs_base.h"

#define RES_TYPE_MAX 6 
#define SUB_RES_TYPE_MAX 8

/* Parse resource ID to acquire base address and index of resource */
int32_t res_parse_info(uint32_t res_id, addr_t * paddr)
{
    uint32_t base_addr[RES_TYPE_MAX]={IRAM_BASE,PLATFORM_R5_SP0_BASE,MB_BASE,APBMUX1_BASE,GPV1_SP0_BASE,GIC1_SP0_BASE};
    uint32_t type_index;
    uint32_t sub_type_index;
    int32_t  res = 0;


    type_index = (res_id >> 12) & 0xFF;
    if(type_index >= RES_TYPE_MAX)
        return -1;
    sub_type_index = res_id & 0xFFF;
    if(sub_type_index >= SUB_RES_TYPE_MAX)
        return -1;
    
    *paddr=base_addr[type_index]+sub_type_index*0x1000;

    return res;
}

/* Get resource info by ID. */
const int32_t res_get_info_by_id(uint32_t res_id, addr_t * paddr, int32_t * index)
{
    uint32_t cat_id = (res_id >> 12) & 0xFF;

    if (cat_id >= (sizeof(g_res_cat) / sizeof(domain_res_t))) {
        return -1;
    }

    if (NULL != g_res_cat[cat_id]) { //check avaibility of resource
        for (int i = 0; i < g_res_cat[cat_id]->res_num; i++) {
            if (g_res_cat[cat_id]->res_id[i] == res_id){
                *index = i;
                return res_parse_info(res_id, paddr);
            }
        }
    }

    return -1;
}

paddr_t p2ap(paddr_t pa)
{
    return pa;
}
