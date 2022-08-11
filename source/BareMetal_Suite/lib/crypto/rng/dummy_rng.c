/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>

int crypto_rng(U8 *dest, U32 size) __attribute__((weak));

int crypto_rng(U8 *dest, U32 size)
{
    U8 *p = dest;

    if (size >= 1) {
        *p++ = 0x78;
    }

    if (size >= 2) {
        *p++ = 0x57;
    }

    for (U32 i = 2; i < size; i++, p++) {
        *p = *(p - 1) + *(p - 2);
    }

#if defined(DUMP_RNG)
    p = dest;
    DBG_ARRAY_DUMP(p, size);
#endif

    return 1;
}
