#ifndef __FUNC_ETH_H_
#define __FUNC_ETH_H_

#include "board_cfg.h"

typedef struct {
#define ETH_MAX_CHN  15
#define ETH_MAX_ERROR_NUM 5
#define ETH_MAX_ERROR_TIME 200
    uint8_t  cnt;
    uint16_t time_val;
} switch_error_arg_t;

typedef struct {
    uint8_t rate_type_id;
    uint8_t eth_chn_table_len;
    const eth_chn_table_t *eth_chn_table;
} eth_pdev_t;

extern switch_error_arg_t switch_error_arg[ETH_MAX_CHN];

#endif