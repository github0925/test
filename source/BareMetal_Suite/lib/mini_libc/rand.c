#include <common_hdr.h>
#include <soc.h>
#include <arch.h>

static uint32_t state = 0;

void srand(unsigned seed)
{
    state = (uint32_t)seed;
}

uint32_t rand32(void)
{
    /* linear congruential generator (LCG) */
    state = ((state * 1103515245u) + 12345u) & 0x7fffffffu;
    return state;
}
