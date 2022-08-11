/*
 * sd_ddr_pfmod.c
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Pfm driver.
 *
 **/

#include <reg.h>
#include <lib/console.h>
#include <irq.h>
#include <platform/interrupts.h>
#include <lib/heap.h>
#include <string.h>
#include "sdrv_ddr_pfmon.h"
#include <app.h>
#include <platform.h>
#include <kernel/thread.h>
#include "clkgen_hal.h"
#include "chip_res.h"
#include <platform/timer.h>

#ifndef SWAP_32
#define SWAP_32(x) \
    (((uint32_t)(x) << 24) | (((uint32_t)(x) & 0xff00) << 8) |(((uint32_t)(x) & 0x00ff0000) >> 8) | ((uint32_t)(x) >> 24))
#endif

static struct pfm_record_irq *current;
static uint32_t rounds;

extern lk_time_t current_time(void);

static enum handler_return pfm_irq_handler(void *arg)
{
    uint32_t group;
    uint32_t *timestamp;
    if (pfm_timeout_irq()) {
        group = pfm_group_nr();
        if (group == 0)
        {
            printf("no channel enabled\n");
            pfm_disable_counter();
            return INT_NO_RESCHEDULE;
        }
        if (!current) {
            printf("no pfm record buffer specified\n");
            pfm_disable_counter();
            return INT_NO_RESCHEDULE;
        }

        if ((uint32_t)current + group * sizeof(*current) + sizeof(uint32_t)
                        >= (PFM_RECORD_BUFFER + PFM_RECORD_BUFFER_SIZE)) {
            current = (struct pfm_record_irq *)(PFM_RECORD_BUFFER + PFM_MON_HEAD_OFFSET);
        }

        timestamp = (uint32_t *)current;
        *timestamp = current_time();

        current = (struct pfm_record_irq *)((uint32_t)current + sizeof(uint32_t));

        pfm_transaction_record(group, current);

        current += group;
        rounds++;

        pfm_enable_counter();
    }
    return INT_NO_RESCHEDULE;
}

static void pfm_register_irq_handle(void)
{
    register_int_handler(DDR_SS_DDR_SS_INT_NUM, &pfm_irq_handler, NULL);
    unmask_interrupt(DDR_SS_DDR_SS_INT_NUM);
}

int pfm_enable(int argc, const cmd_args *argv)
{
    if (argc != 2) {
       printf("not enough arguments\n");
       return -1;
    }
    if (!strcmp(argv[1].str, "irq")) {
        current = (struct pfm_record_irq *)(PFM_RECORD_BUFFER + PFM_MON_HEAD_OFFSET);
        rounds = 0;
        pfm_head_t *head = (pfm_head_t *)(PFM_RECORD_BUFFER);
        head->start_time = current_time();
        pfm_enable_irq_mode();
        pfm_register_irq_handle();
        pfm_enable_counter();
        printf("run pfm irq mode\n");
        return 0;
    } else if (!strcmp(argv[1].str, "dma")) {
        pfm_enable_dma_mode();
        pfm_enable_counter();
        return 0;
    }

    printf("usage: pfm_enable irq/dma \n");
    return -1;
}

static unsigned long get_ddr_rate(void)
{
    void *g_handle;
    int ret;
    unsigned long rate;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("failed to create clock handle\n");
        return 0;
    }

    rate = hal_clock_uuuclk_get(g_handle, RES_UUU_WRAP_SOC_DDR, mon_ref_clk_24M, 0);
    if (rate < 0) {
        printf("failed to get ddr rate\n");
        hal_clock_release_handle(g_handle);
        return 0;
    }

    hal_clock_release_handle(g_handle);
    return rate;
}

int pfm_disable(int argc, const cmd_args *argv)
{
    unsigned long rate = 0;
    pfm_disable_counter();
    pfm_head_t *head = (pfm_head_t *)(PFM_RECORD_BUFFER);
    rate = get_ddr_rate();
    if (rate) {
        /* Time window in ms */
        rate = rate/1000;
        head->time_window = readl(PFM_MON_BASE_CNT_CMP)/rate;
    } else {
        printf("failed to get ddr rate\n");
        return -1;
    }
    pfm_save_config(head);
    head->rounds = rounds;
    head->pool_size = PFM_RECORD_BUFFER_SIZE;
    head->record_offset = PFM_MON_HEAD_OFFSET;
    arch_clean_cache_range(PFM_RECORD_BUFFER, PFM_RECORD_BUFFER_SIZE);
    printf("PFM pool at 0x%lx size 0x%x\n", PFM_RECORD_BUFFER, PFM_RECORD_BUFFER_SIZE);
    return 0;
}

int pfm_time_window(int argc, const cmd_args *argv)
{
    unsigned long rate;
    if (argc != 2) {
        printf("bad arguments\n");
        return -1;
    }
    uint32_t time_window = argv[1].u;

    rate = get_ddr_rate();
    if (rate) {
        /* Time window in ms */
        pfm_cfg_time_window(rate/1000 * time_window);
        return 0;
    } else {
        printf("get ddr rate failed\n");
        return -1;
    }
}

int pfm_setup_group(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        printf("bad arguments\n");
        return -1;
    }
    uint32_t group = argv[1].u;
    uint32_t ret = pfm_enable_group(group);
    if (ret) {
        printf("invalied group\n");
        return -1;
    }
    return 0;
}

int pfm_channel_configure(int argc, const cmd_args *argv)
{
    if (argc != 6) {
        printf("bad arguments\n");
        return -1;
    }
    pfm_observer_t observer = {0};
    uint32_t channel = argv[1].u;
    observer.master0 = argv[2].u;
    observer.msk0 = argv[3].u;
    observer.master1 = argv[4].u;
    observer.msk1 = argv[5].u;
    uint32_t ret = pfm_master_observer(channel, &observer);
    if (ret) {
        printf("failed to config channel\n");
        return -1;
    }
    return 0;
}

pfm_observer_t def_observer[16] = {
    {3,  0x7f,  4, 0x7f},  /* AP */
    {56, 0x78, 56, 0x78},  /* GPU1 */
    {64, 0x78, 64, 0x78},  /* GPU2 */
    {29, 0x7f, 29, 0x7f},  /* VPU1 */
    {30, 0x7f, 30, 0x7f},  /* VPU2 */
    {23, 0x7f, 23, 0x7f},  /* DP1 */
    {24, 0x7f, 24, 0x7f},  /* DP2 */
    {25, 0x7f, 25, 0x7f},  /* DP3 */
    {19, 0x7f, 19, 0x7f},  /* DC1 */
    {20, 0x7f, 20, 0x7f},  /* DC2 */
    {21, 0x7f, 21, 0x7f},  /* DC3 */
    {22, 0x7f, 22, 0x7f},  /* DC4 */
    {26, 0x7f, 26, 0x7f},  /* DC5 */
    {16, 0x7e, 18, 0x7f},  /* CSI */
    {5,  0x7f,  5, 0x7f},  /* VDSP */
    {0, 0, 0, 0} /* ALL */
};

int pfm_default(int argc, const cmd_args *argv)
{
    cmd_args time_argv[2];
    time_argv[1].u = 200;
    pfm_time_window(2, time_argv);
    pfm_enable_group(16);

    for (uint32_t i = 0; i < 16; i++)
    {
        pfm_master_observer(i, &def_observer[i]);
    }
    pfm_stop_condition_t condition;
    condition.rd_thr = 0xffffffff;
    condition.wr_thr = 0xffffffff;
    condition.rd_bcnt_thr = 0xffffffff;
    condition.wr_bcnt_thr = 0xffffffff;
    for (uint32_t i = 0; i < 8; i++)
    {
        pfm_stop_trigger(i, &condition);
    }
    return 0;
}

int pfm_channel_tigger(int argc, const cmd_args *argv)
{
    if (argc != 6) {
        printf("bad arguments\n");
        return -1;
    }
    pfm_stop_condition_t condition = {0};
    condition.rd_thr = argv[2].u;
    condition.wr_thr = argv[3].u;
    condition.rd_bcnt_thr = argv[4].u;
    condition.wr_bcnt_thr = argv[5].u;
    pfm_stop_trigger(argv[1].u, &condition);
    return 0;
}

int pfm_debug(int argc, const cmd_args *argv)
{
    unsigned long rate;
    pfm_head_t *head = calloc(1, sizeof(pfm_head_t));
    pfm_save_config(head);

    rate = get_ddr_rate();
    if (rate) {
        /* Time window in ms */
        rate = rate/1000;
        head->time_window = readl(PFM_MON_BASE_CNT_CMP)/rate;
    } else {
        printf("failed to get ddr rate\n");
        free(head);
        return -1;
    }
    printf("dump config:\n");
    printf("group_nr:%u\n", head->group_nr);
    printf("time window: %u (ms)\n", head->time_window);
    for(uint32_t i = 0; i < 16; i++)
    {
        printf("channel %u, master0: %u\n", i, head->observer_config[i].master0);
        printf("channel %u, msk0: %x\n", i, head->observer_config[i].msk0);
        printf("channel %u, master1: %u\n", i, head->observer_config[i].master1);
        printf("channel %u, msk1: %x\n", i, head->observer_config[i].msk1);
    }
    for (uint32_t i = 0; i < 8; i++)
    {
        printf("channel %u, rd_thr: %x\n", i, head->condition[i].rd_thr);
        printf("channel %u, wr_thr: %x\n", i, head->condition[i].wr_thr);
        printf("channel %u, rd_bcnt_thr: %x\n", i, head->condition[i].rd_bcnt_thr);
        printf("channel %u, wr_bcnt_thr: %x\n", i, head->condition[i].wr_bcnt_thr);
    }
    printf("mode %u\n", head->mode);
    printf("rounds %u\n", rounds);
    free(head);
    return 0;
}

static void pfm_server(const struct app_descriptor *app, void *args)
{
    pfm_default(0, NULL);
    cmd_args cmd[2] ={0};
    cmd[1].str = "irq";
    pfm_enable(2, cmd);
    printf("start pfm daemon service\n");
}

int pfm_dump(int argc, const cmd_args *argv)
{
    int count = 0, size = 4;
    uint32_t val;
    static unsigned long address, stop;

    if (pfm_is_running()) {
        printf("pfm is running, stop pfm firstly\n");
        return -1;
    }

    address = PFM_RECORD_BUFFER;
    stop = PFM_RECORD_BUFFER + PFM_RECORD_BUFFER_SIZE;
    printf("pfm_dump begin: addr=0x%lx len=%x\n", address, PFM_RECORD_BUFFER_SIZE);
    for ( ; address < stop; address += size ) {
        if (count == 0)
            printf("0x%08lx: ", address);
        val = SWAP_32(*(uint32_t *)address);
        printf("%08x ", val);
        count += size;
        if (count == 16) {
            printf("\n");
            count = 0;
        }
    }
    printf("pfm_dump end\n");
    return 0;
}

int pfm_help(int argc, const cmd_args *argv)
{
    printf("pfm is a ddr performace monitor tools\n");
    printf("usage of pfm:\n");
    printf("1) use pfm_default to configure the pfm\n");
    printf("   or use pfm_trigger/pfm_group/pfm_channel/pfm_window");
    printf(" to make a customized config\n");
    printf("2) use pfm_debug to display the config currently\n");
    printf("3) use pfm_enable to start pfm, parameter shall be irq or dma\n");
    printf("4) use pfm_disable to stop pfm\n");
    printf("5) use pfm_dump to print out the records from pfm pool\n");
    printf("6) parse pfm dump file with host parse tools\n");
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("pfm_help", "ddr performace monitor help command", (console_cmd)&pfm_help)
STATIC_COMMAND("pfm_enable", "enable ddr performace monitor", (console_cmd)&pfm_enable)
STATIC_COMMAND("pfm_disable", "disable ddr performace monitor", (console_cmd)&pfm_disable)
STATIC_COMMAND("pfm_window",  "pfm time window(ms)", (console_cmd)&pfm_time_window)
STATIC_COMMAND("pfm_group",  "enable channel number", (console_cmd)&pfm_setup_group)
STATIC_COMMAND("pfm_channel",  "observer config", (console_cmd)&pfm_channel_configure)
STATIC_COMMAND("pfm_trigger",  "stop condition trigger", (console_cmd)&pfm_channel_tigger)
STATIC_COMMAND("pfm_default",  "pfm default observer", (console_cmd)&pfm_default)
STATIC_COMMAND("pfm_debug",  "pfm debug", (console_cmd)&pfm_debug)
STATIC_COMMAND("pfm_dump", "print out pfm mem pool", (console_cmd)&pfm_dump)
STATIC_COMMAND_END(pfmon);
#endif

APP_START(pfm)
.flags = SDRV_DDR_PFM_AUTO_FLAG,
.entry = pfm_server,
APP_END
