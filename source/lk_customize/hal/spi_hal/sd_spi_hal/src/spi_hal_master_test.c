/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include "spi_hal_master_test.h"

extern spi_instance_t *t_spiInstance;
extern spictrl_t *t_spictrl;
extern int spiloop_mode;
static void *spimaster_handle = NULL;

static void spimaster_print_info(void)
{
    for (int i = 0; i < MAX_SPI_DEVICE_NUM; i++) {
        dprintf(ALWAYS,
                "instance info: num=%d, base=%lx, idx=%d, clk_div=%d, occupied=%d, ",
                i, t_spiInstance[i].spictrl->base, t_spiInstance[i].spictrl->bus_index,
                t_spiInstance[i].clk_div, t_spiInstance[i].occupied);
        dprintf(ALWAYS,
                "bpw=%d, bitmode=%u, pollmode=%d, slavenum=%d, speedHZ=%d\n",
                t_spiInstance[i].spidev.bits_per_word, t_spiInstance[i].spidev.bit_mode,
                t_spiInstance[i].spidev.poll_mode, t_spiInstance[i].spidev.slave_num,
                t_spiInstance[i].spidev.speed_hz);
    }

    for (int i = 0; i < SPI_RES_NUM; i++) {
        dprintf(ALWAYS,
                "spictrl info: num=%u, id=%x, base=%lx, irq=%u, bpw_mask=%08x, ",
                t_spictrl[i].bus_index, t_spictrl[i].id, t_spictrl[i].base,
                t_spictrl[i].irq, t_spictrl[i].bpw_mask);
        dprintf(ALWAYS,
                "fifo_len=%u, max_slave_num=%d, max_speedHZ=%d, instance=%p, opmode=%d\n",
                t_spictrl[i].fifo_len, t_spictrl[i].max_slave_num,
                t_spictrl[i].max_speed_hz, t_spictrl[i].instance,
                t_spictrl[i].spi_opmode);
    }
}

static spitest_res_t *spitest_master_info;
static bool inited;
void hal_spi_master_test_entry(int argc, const cmd_args *argv)
{
    if (!inited) {
        spitest_get_res(&spitest_master_info);
        inited = true;
    }

    if (!strcmp(argv[1].str, "print")) {
        spimaster_print_info();
    }
    else if (!strcmp(argv[1].str, "dump")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        spitest_dump_reg(spitest_master_info[num].base);
    }
    else if (!strcmp(argv[1].str, "readdr")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        dprintf(ALWAYS, "spimaster read data reg 0x%x\n",
                spitest_readdr(spitest_master_info[num].base));
    }
    else if (!strcmp(argv[1].str, "writedr")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        uint32_t val = atoui(argv[3].str);
        dprintf(ALWAYS, "spimaster write data reg 0x%x\n", val);
        spitest_writedr(spitest_master_info[num].base, val);
    }
    else if (!strcmp(argv[1].str, "enable")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        if (!strcmp(argv[3].str, "1"))
            spitest_enable(spitest_master_info[num].base, true);

        if (!strcmp(argv[3].str, "0"))
            spitest_enable(spitest_master_info[num].base, false);
    }
    else if (!strcmp(argv[1].str, "setcs")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        uint32_t snum = atoui(argv[3].str);

        if (snum > 16) {
            dprintf(ALWAYS, "spimaster no valid slave num\n");
            return;
        }

        spitest_set_cs(spitest_master_info[num].base, true, snum);
    }
    else if (!strcmp(argv[1].str, "setclk")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        uint32_t speed = atoui(argv[3].str);

        if (speed > 120000000) {
            dprintf(ALWAYS, "max speed 120000000\n");
            return;
        }

        uint32_t div = (DIV_ROUND_UP(120000000, speed) + 1) & 0xfffe;
        spitest_set_clk(spitest_master_info[num].base, div);
    }
    else if (!strcmp(argv[1].str, "dis-irq")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        uint32_t mask = atoui(argv[3].str);
        dprintf(ALWAYS, "spimaster mask 0x%x\n", mask);
        spitest_mask_irq(spitest_master_info[num].base, mask);
    }
    else if (!strcmp(argv[1].str, "en-irq")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        uint32_t mask = atoui(argv[3].str);
        dprintf(ALWAYS, "spimaster mask 0x%x\n", mask);
        spitest_umask_irq(spitest_master_info[num].base, mask);
    }
    else if (!strcmp(argv[1].str, "get-irq")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        dprintf(ALWAYS, "spimaster irq status %x\n",
                spitest_get_irq_stat(spitest_master_info[num].base));
    }
    else if (!strcmp(argv[1].str, "clear-irq")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        spitest_clear_irq(spitest_master_info[num].base);
    }
    else if (!strcmp(argv[1].str, "creat")) {
        uint32_t num = atoui(argv[2].str) - 1;

        if (num > 7 || !spitest_master_info[num].valid) {
            dprintf(ALWAYS, "spimaster no valid base addr\n");
            return;
        }

        if (!hal_spi_creat_handle(&spimaster_handle, spitest_master_info[num].id))
            dprintf(ALWAYS, "spimaster creat handle fail\n");
    }
    else if (!strcmp(argv[1].str, "del")) {
        hal_spi_release_handle(spimaster_handle);
        spimaster_handle = NULL;
    }
    else if (!strcmp(argv[1].str, "init")) {
        spidev_t info;
        info.slave_num     = atoui(argv[2].str);
        info.speed_hz      = atoui(argv[3].str);
        info.bits_per_word = atoui(argv[4].str);
        info.bit_mode      = atoui(argv[5].str);
        info.poll_mode     = atoui(argv[6].str);
        dprintf(ALWAYS,
                "slavenum=%d, speed=%d, bpw=%d, bitmode=%d, pollmode=%d\n", info.slave_num,
                info.speed_hz, info.bits_per_word, info.bit_mode, info.poll_mode);

        if (hal_spi_init(spimaster_handle, &info) < 0)
            dprintf(ALWAYS, "spimaster init fail\n");
    }
    else if (!strcmp(argv[1].str, "read")) {
        uint8_t rbuf[512];
        hal_spi_read(spimaster_handle, &rbuf, 512);

        for (int i = 0; i < 512; i++)
            dprintf(ALWAYS, "spimaster read buf[%d]=%u\n", i, rbuf[i]);
    }
    else if (!strcmp(argv[1].str, "write")) {
        uint32_t num = atoui(argv[2].str);

        if (num > 512) {
            dprintf(ALWAYS, "spimaster max num 512 byte\n");
            return;
        }

        uint8_t tbuf[512] = {0};

        for (uint32_t i = 0; i < num; i++)
            tbuf[i] = i % 256;

        hal_spi_write(spimaster_handle, &tbuf, 512);
    }
    else if (!strcmp(argv[1].str, "loop")) {
        uint8_t rbuf[512] = {0};
        uint8_t tbuf[512] = {0};
        uint32_t i = 0;

        for (i = 0; i < 512; i++)
            tbuf[i] = i % 256;

        spiloop_mode = 1;
        hal_spi_parallel_rw(spimaster_handle, &tbuf, &rbuf, 512);
        spiloop_mode = 0;

        for (i = 0; i < 512; i++) {
            if (rbuf[i] != tbuf[i]) {
                dprintf(ALWAYS, "spimaster loop err num=%u rbuf=%u, tbuf=%u\n",
                        i, rbuf[i], tbuf[i]);
                break;
            }

            dprintf(ALWAYS, "spimaster loop ok\n");
        }
    }
    else {
        dprintf(ALWAYS,
                "===========spimaster support command===========            \n"
                "spimaster dump 1~8[1~8:controller num]                     \n"
                "spimaster readdr 1~8                                       \n"
                "spimaster writedr 1~8 0xXX[1byte data]                     \n"
                "spimaster enable 1~8 0/1[1:enable]                         \n"
                "spimaster setcs 1~8 0~15[0~15:slave num]                   \n"
                "spimaster setclk 1~8 0~60000000[HZ]                        \n"
                "spimaster dis-irq 1~8 0~3f                                 \n"
                "spimaster en-irq 1~8 0~3f                                  \n"
                "spimaster get-irq 1~8                                      \n"
                "spimaster clear-irq 1~8                                    \n"
                "spimaster print                                            \n"
                "====below cmd simulate slave communication====             \n"
                "spimaster creat 1~8[creat slave instance on controller 1~8]\n"
                "spimaster del 1~8[del slave instance on controller 1~8]    \n"
                "spimaster init 0~16 0~60000000 8/16 0/1/2/3 0/1            \n"
                "spimaster read                                             \n"
                "spimaster write num[1~512 byte]                            \n"
                "spimaster loop                                             \n"
                "============spimaster support command===========         \n\n");
    }
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START STATIC_COMMAND("spimaster",
                                    "spi master function test",
                                    (console_cmd)&hal_spi_master_test_entry)
STATIC_COMMAND_END(spimaster);
#endif
