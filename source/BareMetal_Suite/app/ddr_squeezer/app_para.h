/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef _APP_PARA_H_
#define _APP_PARA_H_

#include <common_hdr.h>

#define APP_PARA_TAG    0x50415001u

typedef struct {
    uint32_t tag;   /* 'PAR | ver' */
    uint32_t tty;
    uint32_t cores; /* bit15-8: 0 for AP1, 1 for AP2;
                     * bit7-0: number of cores (for AP1 only)
                     */
    uint32_t rsvd[5];
} app_para_t;

#endif
