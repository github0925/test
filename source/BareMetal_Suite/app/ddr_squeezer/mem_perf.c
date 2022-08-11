/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

#include <common_hdr.h>
#include <srv_timer/srv_timer.h>
#include <debug.h>
#include "shell/shell.h"
#if defined(TGT_ap)
#include <mini_libc.h>

#define usage "Usage: memperf rd[wr] src sz\n\tmemperf cp dst src sz\n"

uint32_t cmd_memperf(uint32_t argc, char *argv[])
{
    if ((argc != 4) && (argc != 5)) {
        DBG("%s", usage);
        return 0;
    }

    unsigned long long dst = 0, src = 0, sz = 0;

    uint64_t tick1 = 0ull, tick2 = 0ull;

    if (0 == strncmp(argv[1], "rd", 2)) {
        src = strtoull(argv[2], NULL, 0);
        sz = strtoull(argv[3], NULL, 0);

        if ((0 != src % 64) | (0 != sz % 64)) {
            DBG("src/sz shall be 64 bytes aligned\n");
            return -1;
        }

        tick1 = tmr_tick();
        mem_rd_only_aligned((uint64_t *)src, sz);
        tick2 = tmr_tick();
        DBG("It takes %d us to read 0x%p bytes, and perf is %d MB/s\n",
            (uint32_t)(tick2 - tick1), sz, sz / (tick2 - tick1));
    } else if (0 == strncmp(argv[1], "wr", 2)) {
        src = strtoull(argv[2], NULL, 0);
        sz = strtoull(argv[3], NULL, 0);

        if ((0 != src % 64) | (0 != sz % 64)) {
            DBG("src/sz shall be 64 bytes aligned\n");
            return -1;
        }

        tick1 = tmr_tick();
        mem_wr_only_aligned((uint64_t *)src, sz);
        tick2 = tmr_tick();
        DBG("It takes %d us to write 0x%p bytes, and perf is %d MB/s\n",
            (uint32_t)(tick2 - tick1), sz, sz / (tick2 - tick1));

    } else if (0 == strncmp(argv[1], "cp", 2)) {
        dst = strtoull(argv[2], NULL, 0);
        src = strtoull(argv[3], NULL, 0);
        sz = strtoull(argv[4], NULL, 0);

        if ((0 != src % 64) || (0 != sz % 64) || (0 != dst % 64)) {
            DBG("dst/src/sz shall be 64 bytes aligned\n");
            return -1;
        }

        tick1 = tmr_tick();
        memcpy_aligned((uint64_t *)dst, (uint64_t *)src, sz);
        tick2 = tmr_tick();
        DBG("It takes %d us to cp 0x%p bytes, and perf is %d MB/s\n",
            (uint32_t)(tick2 - tick1), sz, sz / (tick2 - tick1));
    }

    return 0;
}

SHELL_CMD("memperf", cmd_memperf, usage)

#endif  /* TGT_ap */
