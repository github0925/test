/*
 * res.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Domain resource management header file.
 *
 * Revision History:
 * -----------------
 */
#ifndef _RES_H
#define _RES_H

#include <sys/types.h>

typedef struct domain_res {
    int         res_num;
    uint32_t    res_id[];
} domain_res_t;

/* Declarations of resource helper functions. */
const int32_t res_get_info_by_id(uint32_t resid, addr_t * paddr, int32_t * index);

/* Transform address from r core to a core */
paddr_t p2ap(paddr_t pa);
/* Transform address from a core to r core */
paddr_t ap2p(paddr_t va);

#endif /* _RES_H */
