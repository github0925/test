/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include "spi_hal_slave_test.h"

static void *spislave_handle;
static uint8_t *t_spislave_rbuf;
static uint8_t *t_spislave_tbuf;

static void spislave_print_buf(void)
{
    dprintf(ALWAYS, "================================\n");

    if (t_spislave_rbuf == NULL || t_spislave_tbuf == NULL) {
        dprintf(ALWAYS, "buf not inited\n");
        return;
    }

    for (int i = 0; i < SPISLAVE_BUF_SIZE; i++)
        dprintf(ALWAYS, "spislave rbuf[%d]=%u\n", i, t_spislave_rbuf[i]);

    for (int i = 0; i < SPISLAVE_BUF_SIZE; i++)
        dprintf(ALWAYS, "spislave tbuf[%d]=%u\n", i, t_spislave_tbuf[i]);

    dprintf(ALWAYS, "=================================\n");
}

static spitest_res_t *spitest_slave_info;
static bool inited;
void hal_spi_slave_test_entry(int argc, const cmd_args *argv)
{
    if (!inited) {
        spitest_get_res(&spitest_slave_info);
        inited = true;
    }

    if (!strcmp(argv[1].str, "print")) {
        spislave_print_buf();
    }
    else if (!strcmp(argv[1].str, "dump")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_slave_info[num].valid) {
            dprintf(ALWAYS, "spislave no valid base addr\n");
            return;
        }

        spitest_dump_reg(spitest_slave_info[num].base);
    }
    else if (!strcmp(argv[1].str, "readdr")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_slave_info[num].valid) {
            dprintf(ALWAYS, "spislave no valid base addr\n");
            return;
        }

        dprintf(ALWAYS, "spislave read data reg 0x%x\n",
                spitest_readdr(spitest_slave_info[num].base));
    }
    else if (!strcmp(argv[1].str, "writedr")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_slave_info[num].valid) {
            dprintf(ALWAYS, "spislave no valid base addr\n");
            return;
        }

        uint32_t val = atoui(argv[3].str);
        dprintf(ALWAYS, "spislave write data reg 0x%x\n", val);
        spitest_writedr(spitest_slave_info[num].base, val);
    }
    else if (!strcmp(argv[1].str, "enable")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_slave_info[num].valid) {
            dprintf(ALWAYS, "spislave no valid base addr\n");
            return;
        }

        if (!strcmp(argv[3].str, "1"))
            spitest_enable(spitest_slave_info[num].base, true);
        else if (!strcmp(argv[3].str, "0"))
            spitest_enable(spitest_slave_info[num].base, false);
    }
    else if (!strcmp(argv[1].str, "dis-irq")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_slave_info[num].valid) {
            dprintf(ALWAYS, "spislave no valid base addr\n");
            return;
        }

        uint32_t mask = atoui(argv[3].str);
        dprintf(ALWAYS, "spislave mask 0x%x\n", mask);
        spitest_mask_irq(spitest_slave_info[num].base, mask);
    }
    else if (!strcmp(argv[1].str, "en-irq")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_slave_info[num].valid) {
            dprintf(ALWAYS, "spislave no valid base addr\n");
            return;
        }

        uint32_t mask = atoui(argv[3].str);
        dprintf(ALWAYS, "spislave mask 0x%x\n", mask);
        spitest_umask_irq(spitest_slave_info[num].base, mask);
    }
    else if (!strcmp(argv[1].str, "get-irq")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_slave_info[num].valid) {
            dprintf(ALWAYS, "spislave no valid base addr\n");
            return;
        }

        dprintf(ALWAYS, "spislave irq status %x\n",
                spitest_get_irq_stat(spitest_slave_info[num].base));
    }
    else if (!strcmp(argv[1].str, "clear-irq")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_slave_info[num].valid) {
            dprintf(ALWAYS, "spislave no valid base addr\n");
            return;
        }

        spitest_clear_irq(spitest_slave_info[num].base);
    }
    else if (!strcmp(argv[1].str, "creat")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_slave_info[num].valid) {
            dprintf(ALWAYS, "spislave no valid base addr\n");
            return;
        }

        if (!hal_spi_slave_creat_handle(&spislave_handle,
                                        spitest_slave_info[num].id))
            dprintf(ALWAYS, "spislave creat handle fail\n");
    }
    else if (!strcmp(argv[1].str, "del")) {
        if (!hal_spi_slave_release_handle(&spislave_handle))
            dprintf(ALWAYS, "spislave del handle fail\n");
    }
    else if (!strcmp(argv[1].str, "init")) {
        hal_spi_slave_init_buf(&t_spislave_rbuf, &t_spislave_tbuf);
    }
    else if (!strcmp(argv[1].str, "read")) {
        for (int i = 0; i < SPISLAVE_BUF_SIZE; i++) {
            dprintf(ALWAYS, "spislave read rbuf[%d]=%u\n", i, t_spislave_rbuf[i]);
        }
    }
    else if (!strcmp(argv[1].str, "write")) {
        uint32_t num = atoui(argv[2].str);

        if (num > 512) {
            dprintf(ALWAYS, "spislave max num 512 byte\n");
            return;
        }

        uint8_t buf[SPISLAVE_BUF_SIZE] = {0};

        for (uint32_t i = 0; i < num; i++)
            buf[i] = i % 256;

        memcpy(t_spislave_tbuf, buf, SPISLAVE_BUF_SIZE);
    }
    else {
        dprintf(ALWAYS,
                "===========spislave support command===========            \n"
                "spislave dump 1~8[1~8:controller num]                     \n"
                "spislave readdr 1~8                                       \n"
                "spislave writedr 1~8 0xXX[1byte data]                     \n"
                "spislave enable 1~8 0/1[1:enable]                         \n"
                "spislave dis-irq 1~8 0~3f                                 \n"
                "spislave en-irq 1~8 0~3f                                  \n"
                "spislave get-irq 1~8                                      \n"
                "spislave clear-irq 1~8                                    \n"
                "spislave print                                            \n"
                "====below cmd simulate slave controller communication==== \n"
                "spislave creat 1~8[creat slave instance on controller 1~8]\n"
                "spislave del 1~8[del slave controller on controller 1~8]  \n"
                "spislave read                                             \n"
                "spislave write num[1~512 byte]                            \n"
                "============spislave support command===========         \n\n");
    }
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START STATIC_COMMAND("spislave", "spi slave function test",
                                    (console_cmd)&hal_spi_slave_test_entry)
STATIC_COMMAND_END(spislave);
#endif
