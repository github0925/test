/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 *******************************************************/

#include <common_hdr.h>
#include <soc.h>
#include <srv_timer/srv_timer.h>

void hrng_get_rnd(uint8_t *rnd, int32_t len)
{
#define HRNG_CTRL       (CE2_RBASE + 0x8020)
#define HRNG_NUM        (CE2_RBASE + 0x114U)
#define BM_HRNG_CTRL_NOISE_ENABLE   (0x01U << 0U)
    uint32_t v = readl(HRNG_CTRL);

    if (!(v & BM_HRNG_CTRL_NOISE_ENABLE)) {
        v |= BM_HRNG_CTRL_NOISE_ENABLE;
        writel(v, HRNG_CTRL);
        udelay(1);  /* wait a while for noise ramp up, per design's suggestion */
    }

    volatile uint32_t val = 0;

    if (len > 4) {
        for (int i = 0; i < len / 4; i++) {
            val = readl(HRNG_NUM);
            udelay(5);
            *rnd++ = (uint8_t)val;
            *rnd++ = (uint8_t)(val >> 8);
            *rnd++ = (uint8_t)(val >> 16);
            *rnd++ = (uint8_t)(val >> 24);
        }

        len -= ((len / 4) * 4);
    }

    if (len) {
        val = readl(HRNG_NUM);

        for (int i = 0; i < len; i++) {
            *rnd++ = (uint8_t)(val >> (8 * i));
        }
    }
}

