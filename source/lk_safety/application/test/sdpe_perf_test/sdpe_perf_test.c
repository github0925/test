#include <kernel/thread.h>
#include <app.h>
#include "sdpe_stat.h"

static volatile bool g_sdpe_perf = true;
static uint32_t g_cnt_freq = 12500U;

static int sdpe_perf_dump(void *arg)
{
    while (g_sdpe_perf) {
        stat_dump((float)g_cnt_freq/1000U);
        thread_sleep(100U);
    }

    return 0;
}

static void sdpe_stat_entry(const struct app_descriptor *app, void *args)
{
    thread_t *t = thread_create("sdpe_perf_test",
                                sdpe_perf_dump,
                                NULL,
                                LOW_PRIORITY,
                                DEFAULT_STACK_SIZE);

    if (t) {
        thread_detach_and_resume(t);
    }
}

APP_START(sdpe_pref_test)
//.entry = sdpe_stat_entry,
APP_END

#if WITH_LIB_CONSOLE
#include <lib/console.h>

#include "sdpe_ctrl_service.h"

static int sdpe_stat_start(int argc, const cmd_args *argv)
{
    static bool thread_init;

    if (argc < 4) {
        printf("Usage: sss <en> <port> <timer type> <timer frequency>\n");
        printf("       en: 1 - enable, 0 - disable\n");
        printf("       port: SDPE Port id\n");
        printf("       timer type: 0 - ARM cycle counter, 1 - System counter\n");
        printf("       timer frequency: uint 0.001MHz\n");
        return 1;
    }

    uint8_t en = argv[1].u;

    g_cnt_freq = argv[4].u;

    if (en) {
        struct {
            uint32_t port_id;
            uint32_t timer;
        } stat_cfg = {
            argv[2].u,
            argv[3].u
        };
        sdpe_monitor_event(9U, 1U, sizeof(stat_cfg), (uint8_t *)&stat_cfg);

        /* Wait for the ongoing writing
         * in SDPE completed.
         */
        thread_sleep(1U);
        stat_flush();

        g_sdpe_perf = true;

        if (!thread_init) {
            thread_init = true;
            thread_t *t = thread_create("sdpe_perf_test",
                                    sdpe_perf_dump,
                                    NULL,
                                    LOW_PRIORITY,
                                    DEFAULT_STACK_SIZE);

            if (t) {
                thread_detach_and_resume(t);
            }
        }
    }
    else {
        uint8_t port_id = argv[2].u;
        sdpe_monitor_event(9U, 0U, sizeof(uint8_t), (uint8_t *)&port_id);

        g_sdpe_perf = false;
        thread_init = false;
    }

    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("sss", "Start SDPE statistics", sdpe_stat_start)
STATIC_COMMAND_END(sdpe_stat);
#endif