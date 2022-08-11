/*
 * cfg_g9x_ref.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include "cfg.h"
#include "board_diag.h"
#include "debug.h"

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

void filter_move_step(uint16_t sum, uint16_t len, uint16_t val,
                      uint16_t *arraydata)
{
    if (sum < len) {
        *(arraydata + sum) = val;
    }
    else {
        for (uint16_t i = 0; i < len; i++) {
            *(arraydata + i) = *(arraydata + i + 1);
        }

        *(arraydata + len - 1) = val;
    }
}
/*filter*/
uint16_t anti_inter_filter_algo(uint16_t sum, uint16_t *arraydata)
{
    uint16_t maxdata = 0;
    uint16_t mindata = 0;
    unsigned long averagedata = 0;
    maxdata = *(arraydata + 0);
    mindata = *(arraydata + 0);

    for (uint16_t i = 0; i < sum; i++) {
        if (*(arraydata + i) > maxdata) {
            maxdata = *(arraydata + i);           //max value
        }
        else {
            if (*(arraydata + i) < mindata) {
                mindata = *(arraydata + i);      //min value
            }
        }

        averagedata += (unsigned long) * (arraydata + i);
    }

    if (unlikely(sum < 3)) {
        return (averagedata / sum);             //average
    }
    else {
        averagedata -= maxdata;
        averagedata -= mindata;
        return (averagedata / (sum - 2));       //average
    }
}
