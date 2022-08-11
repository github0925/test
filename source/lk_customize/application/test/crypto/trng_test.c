/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <app.h>
#include <lib/console.h>
#include <platform.h>

#include <sd_rng.h>
#include <trace.h>

#include "ce_test.h"
#define LOCAL_TRACE 0//close local trace 1->0

uint32_t trng_init_test(void)
{
    uint32_t ret = trng_init(1);

    LTRACEF("trng init test result: %d\n", ret);
    return ret;
}

uint32_t trng_get_rand_test(void)
{
    uint8_t output[64] = {0};
    int i, j = 0;
    void* crypto_handle;

    //get random
    for (int m = 0; m < 5000; m++) {
        hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);
        hal_trng_gen(crypto_handle, output, 64);
        hal_crypto_delete_handle(crypto_handle);

        ce_printf_binary("rand number", output, 64);

        // verify that we got some random, different from init value
        for (i = 0; i < 49; i++) {
            if (output[i] == 0x00) {
                j++;
            }
        }

        LTRACEF("check 0 result: %s, time: %d\n", j > 5 ? "FAIL" : "PASS", m);
        j = 0;
    }

    return 0;
}

void trng_get_hrng_test(void)
{
    uint32_t hrng;
    void* crypto_handle;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    for (int i = 0; i < 10; i++) {
        hrng = trng_get_hrng(0);
        hal_prng_gen(crypto_handle, (void*)&hrng, sizeof(uint32_t));

        LTRACEF("for times: %d  hrng: 0x%x, hrng: %d\n", i, hrng, hrng);
    }

    hal_crypto_delete_handle(crypto_handle);
}

/* fifo test & get 1000 * 128k data for certification */
void trng_fifo_test(void)
{
    uint8_t rng[128];
    uint32_t count = 128;
    uint32_t ret;
    uint32_t size;

    for (int i = 0; i < 1000; i++) {
        dprintf(INFO, "once circle for 128k data, i : %d\n", i);
        size = 128 * 1000;

        while (size > 0) {
            ret = trng_get_rand_by_fifo(rng, count);
            if (ret) {
                LTRACEF("trng_fifo_test fail: %x\n", ret);
                continue;
            }

            for (uint32_t i = 0; i < count; i++) {
                printf("%02x", rng[i]);
                if (0 == (i + 1) % 32) {
                    printf("\n");
                }
            }

            size -= count;
        }
    }
}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("trng_init_test", "rand number init test", (console_cmd)&trng_init_test)
STATIC_COMMAND("trng_get_rand", "acquire rand number", (console_cmd)&trng_get_rand_test)
STATIC_COMMAND("trng_get_hrng", "acquire hrand number", (console_cmd)&trng_get_hrng_test)
STATIC_COMMAND("trng_fifo_test", "fifo_test", (console_cmd)&trng_fifo_test)
STATIC_COMMAND_END(trng_test);

#endif

APP_START(trng_test)
.flags = 0
APP_END
