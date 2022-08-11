/*
 * memtester version 4
 *
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2012 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 */

#include "memtester.h"

#define __version__ "4.3.0"

#define EXIT_FAIL_NONSTARTER    0x01
#define EXIT_FAIL_ADDRESSLINES  0x02
#define EXIT_FAIL_OTHERTEST     0x04
#define LOOPS   1

struct test tests[] = {
    { "Random Value",           test_random_value },
    { "Compare XOR",            test_xor_comparison },
    { "Compare SUB",            test_sub_comparison },
    { "Compare MUL",            test_mul_comparison },
    { "Compare DIV",            test_div_comparison },
    { "Compare OR",             test_or_comparison },
    { "Compare AND",            test_and_comparison },
    { "Sequential Increment",   test_seqinc_comparison },
    { "Solid Bits",             test_solidbits_comparison },
    { "Block Sequential",       test_blockseq_comparison },
    { "Checkerboard",           test_checkerboard_comparison },
    { "Bit Spread",             test_bitspread_comparison },
    { "Bit Flip",               test_bitflip_comparison },
    { "Walking Ones",           test_walkbits1_comparison },
    { "Walking Zeroes",         test_walkbits0_comparison },
#ifdef TEST_NARROW_WRITES
    { "8-bit Writes",           test_8bit_wide_random },
    { "16-bit Writes",          test_16bit_wide_random },
#endif
    { NULL, NULL }
};

int memtester_main(void *args, uint32_t loops)
{
    int loop, i;
    addr_t aligned;
    size_t bufsize, halflen, count;
    ulv *bufa, *bufb;
    int exit_code = 0;
    int testmask = 0;

    mem_range_t *rng = (mem_range_t *)args;

    if (NULL == rng) {
        return -1;
    }

    if (0 == rng->start || 0 == rng->sz) {
        return -2;
    } else {
        aligned = rng->start;
        bufsize = rng->sz;
    }

    halflen = bufsize / 2;
    count = halflen / sizeof(ulv);
    bufa = (ulv *) aligned;
    bufb = (ulv *) ((size_t) aligned + halflen);

    for (loop = 1; ((!loops) || loop <= loops) && (0 != aligned) && (0 != bufsize); loop++) {
        DBG("Loop %d/%d\n", loop, loops);

        if (!test_stuck_address((ulv *)aligned, (ulv *)aligned, bufsize / sizeof(ul))) {
            DBG("  %s: ok\n", "Stuck Address");
        } else {
            DBG("  %s: fail\n", "Stuck Address");
            exit_code |= EXIT_FAIL_ADDRESSLINES;
            break;
        }

        for (i = 0;; i++) {
            if (!tests[i].name) break;

            if (testmask && (!((1 << i) & testmask))) {
                continue;
            }

            if (!tests[i].fp(bufa, bufb, count)) {
                DBG("  %s: ok\n", tests[i].name);
            } else {
                DBG("  %s: fail\n", tests[i].name);
                exit_code |= EXIT_FAIL_OTHERTEST;
                break;
            }
        }

        if (exit_code != 0) {
            break;
        }

        DBG("\n");
    }

    DBG("Done. memtester %s.\n", 0 == exit_code ? "succeed" : "failed");

    return exit_code;
}
