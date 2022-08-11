/*
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2012 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains the functions for the actual tests, called from the
 * main routine in memtester.c.  See other comments in that file.
 *
 */

#include "memtester.h"

#if defined(PROGRESS_ON)
char progress[] = "-\\|/";
#define PROGRESSLEN 4
#define PROGRESSOFTEN 2500
#endif
#define ONE 0x00000001UL

#define DUMP_BYTES 0x40
#define DUMP_WD    (DUMP_BYTES/sizeof(ulv))

int use_phys = 0;
unsigned long physaddrbase = 0;

/* Function definitions. */

static void dump_fail_data(ulv *addr, uint32_t cnt)
{
    assert(NULL != addr);

    char str[DUMP_WD * (3 + sizeof(ulv) * 2) + 64];
    char *p = &str[0];

    sprintf(p, "at 0x%p:\n", addr);

    for (uint32_t i = 0; i < cnt; i++, addr++) {
        p += strlen(p);
        sprintf(p, "0x%p ", *addr);

        if (i % 4 == 3) {
            p += strlen(p);
            sprintf(p, "\n");
        }
    }

    DBG("%s", str);
}

int compare_regions(ulv *bufa, ulv *bufb, size_t count)
{
    int r = 0;
    size_t i;
    ulv *p1 = bufa;
    ulv *p2 = bufb;

    for (i = 0; i < count; i++, p1++, p2++) {
        if (*p1 != *p2) {
            DBG("\t\tFAILURE: 0x%p(at 0x%p) != 0x%p(at 0x%p)\n", (ul) *p1, (ul)p1, (ul) *p2, (ul)p2);
            r = -1;

            if (((p1 - DUMP_WD / 2) >= bufa) && (p1 + DUMP_WD / 2 < bufa + count)) {
                dump_fail_data(p1 - DUMP_WD / 2, DUMP_WD);
                dump_fail_data(p2 - DUMP_WD / 2, DUMP_WD);
            }

            break;
        }
    }

    return r;
}

int test_stuck_address(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    unsigned int j;
    size_t i;

    DBG("\tStuck address test...\n");

    for (j = 0; j < 2; j++) {
        p1 = (ulv *) bufa;

        for (i = 0; i < count ; i++) {
            *p1 = ((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1);
            /*
             * The orignal code is '*p1++' which issues a read. This may
             * cause side-effect if ECC enabled in DDR controller. The read
             * will cause a cache line fill. Given the DDR memory not been
             * ecc-inited, thus a fill may cause ECC error and gives cpu an abort.
             */
            p1++;
        }

        arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));

        p1 = (ulv *) bufa;

        for (i = 0; i < count; i++, p1++) {
            if (*p1 != (((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1))) {
                DBG("\t\tFAILURE at 0x%p, 0x%p expected but 0x%p\n", (ul)p1,
                    (((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1)), *p1);

                if (((p1 - DUMP_WD / 2) >= bufa) && (p1 + DUMP_WD / 2 < bufa + count)) {
                    dump_fail_data(p1 - DUMP_WD / 2, DUMP_WD);
                }

                return -1;
            }
        }
    }

    return 0;
}

int test_random_value(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;

    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = rand_ul();
    }

    arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
    arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

    return compare_regions(bufa, bufb, count);
}

int test_xor_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ ^= q;
        *p2++ ^= q;
    }

    arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
    arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));
    return compare_regions(bufa, bufb, count);
}

int test_sub_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ -= q;
        *p2++ -= q;
    }

    arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
    arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));
    return compare_regions(bufa, bufb, count);
}

int test_mul_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ *= q;
        *p2++ *= q;
    }

    arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
    arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));
    return compare_regions(bufa, bufb, count);
}

int test_div_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        if (!q) {
            q++;
        }

        *p1++ /= q;
        *p2++ /= q;
    }

    arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
    arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));
    return compare_regions(bufa, bufb, count);
}

int test_or_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ |= q;
        *p2++ |= q;
    }

    arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
    arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));
    return compare_regions(bufa, bufb, count);
}

int test_and_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ &= q;
        *p2++ &= q;
    }

    arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
    arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));
    return compare_regions(bufa, bufb, count);
}

int test_seqinc_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = (i + q);
    }

    arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
    arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));
    return compare_regions(bufa, bufb, count);
}

int test_solidbits_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ul q;
    size_t i;

    DBG("\tSolid Bits test...\n");

    for (j = 0; j < 2; j++) {
        q = (j % 2) == 0 ? UL_ONEBITS : 0;
        //DBG("\tRound %3u\n", j);

        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;

        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }

        arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
        arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }

    return 0;
}

int test_checkerboard_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ul q;
    size_t i;

    DBG("\tCheckerboard test...\n");

    for (j = 0; j < 2; j++) {
        q = (j % 2) == 0 ? CHECKERBOARD1 : CHECKERBOARD2;

        //DBG("\tRound %3u\n", j);

        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;

        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }

        arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
        arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }

    return 0;
}

int test_blockseq_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    DBG("\tBlock Sequential test...\n");

    for (j = 0; j < 256; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;

        if (j % 16 == 15)   DBG("\tRound %3u\n", j);

        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (ul) UL_BYTE(j);
        }

        arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
        arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }

    return 0;
}

int test_walkbits0_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    DBG("\tWalk0 test...\n");

    for (j = 0; j < UL_LEN * 2; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        //DBG("\tRound %3u\n", j);

        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = ONE << j;
            } else { /* Walk it back down. */
                *p1++ = *p2++ = ONE << (UL_LEN * 2 - j - 1);
            }
        }

        arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
        arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }

    return 0;
}

int test_walkbits1_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    DBG("\tBlock Sequential test...\n");

    for (j = 0; j < UL_LEN * 2; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        //DBG("\tRound %3u\n", j);

        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << j);
            } else { /* Walk it back down. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << (UL_LEN * 2 - j - 1));
            }
        }

        arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
        arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }

    return 0;
}

int test_bitspread_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    DBG("\tBit Spread test...\n");

    for (j = 0; j < UL_LEN * 2; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        //DBG("\tRound %3u\n", j);

        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = (i % 2 == 0)
                                ? (ONE << j) | (ONE << (j + 2))
                                : UL_ONEBITS ^ ((ONE << j)
                                                | (ONE << (j + 2)));
            } else { /* Walk it back down. */
                *p1++ = *p2++ = (i % 2 == 0)
                                ? (ONE << (UL_LEN * 2 - 1 - j)) | (ONE << (UL_LEN * 2 + 1 - j))
                                : UL_ONEBITS ^ (ONE << (UL_LEN * 2 - 1 - j)
                                                | (ONE << (UL_LEN * 2 + 1 - j)));
            }
        }

        arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
        arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }

    return 0;
}

int test_bitflip_comparison(ulv *bufa, ulv *bufb, size_t count)
{
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j, k;
    ul q;
    size_t i;

    DBG("\tBitflip test...\n");

    for (k = 0; k < UL_LEN; k++) {
        q = ONE << k;

        for (j = 0; j < 2; j++) {
            q = ~q;
            //DBG("\tRound %3u\n", k * 8 + j);

            p1 = (ulv *) bufa;
            p2 = (ulv *) bufb;

            for (i = 0; i < count; i++) {
                *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
            }

            arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
            arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

            if (compare_regions(bufa, bufb, count)) {
                return -1;
            }
        }
    }

    return 0;
}

#ifdef TEST_NARROW_WRITES
int test_8bit_wide_random(ulv *bufa, ulv *bufb, size_t count)
{
    u8v *p1, *t;
    ulv *p2;
    int attempt;
    unsigned int b, j = 0;
    size_t i;

    serial_putc(' ');

    for (attempt = 0; attempt < 2;  attempt++) {
        if (attempt & 1) {
            p1 = (u8v *) bufa;
            p2 = bufb;
        } else {
            p1 = (u8v *) bufb;
            p2 = bufa;
        }

        for (i = 0; i < count; i++) {
            t = mword8.bytes;
            *p2++ = mword8.val = rand_ul();

            for (b = 0; b < UL_LEN / 8; b++) {
                *p1++ = *t++;
            }

#if defined(PROGRESS_ON)

            if (!(i % PROGRESSOFTEN)) {
                serial_putc('\b');
                serial_putc(progress[++j % PROGRESSLEN]);
            }

#endif
        }

        arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
        arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }

    return 0;
}

int test_16bit_wide_random(ulv *bufa, ulv *bufb, size_t count)
{
    u16v *p1, *t;
    ulv *p2;
    int attempt;
    unsigned int b, j = 0;
    size_t i;

    DBG( ' ' );
    fflush( stdout );

    for (attempt = 0; attempt < 2; attempt++) {
        if (attempt & 1) {
            p1 = (u16v *) bufa;
            p2 = bufb;
        } else {
            p1 = (u16v *) bufb;
            p2 = bufa;
        }

        for (i = 0; i < count; i++) {
            t = mword16.u16s;
            *p2++ = mword16.val = rand_ul();

            for (b = 0; b < UL_LEN / 16; b++) {
                *p1++ = *t++;
            }

#if defined(PROGRESS_ON)

            if (!(i % PROGRESSOFTEN)) {
                DBG('\b');
                DBG("progress %d", progress[++j % PROGRESSLEN]);

            }

#endif
        }

        arch_clean_invalidate_cache_range((const void *)bufa, count * sizeof(ulv));
        arch_clean_invalidate_cache_range((const void *)bufb, count * sizeof(ulv));

        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }

    return 0;
}
#endif
