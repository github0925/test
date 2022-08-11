/*
 * doip_service.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _DOIP_SERVICE_H
#define _DOIP_SERVICE_H

/* DOIP SERVICE */

typedef struct sdpe_doip_addr {
    uint8_t mtype;
    uint8_t tatype;
    uint16_t sa;
    uint16_t ta;
    uint16_t ae;
} sdpe_doip_addr_t;

int vdoip_service_init(void);
int vdoip_service_open(void);
int vdoip_service_close(void);
int vdoip_data_confirm(struct sdpe_doip_addr *addr, uint32_t result);
int vdoip_data_som_ind(struct sdpe_doip_addr *addr, uint32_t t_len);
int vdoip_data_ind(struct sdpe_doip_addr *addr, uint32_t t_result,
                   uint32_t t_len, uint8_t *data);

#endif
