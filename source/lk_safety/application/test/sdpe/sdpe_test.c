/*
* sdpe_test.c
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* Description: sdpe_test samplecode.
*
* Revision History:
* -----------------
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <kernel/thread.h>
#include <storage_device.h>
#include "spi_nor_hal.h"
#include "partition_parser.h"

#undef ERPC_TYPE_DEFINITIONS
#ifdef SUPPORT_SDPE_RPC
#include "sdpe_ctrl_service.h"
#else
#include "sdpe_ctrl.h"
#endif
#include "chip_res.h"
#include "image_cfg.h"

static struct spi_nor_cfg g_ospi_cfg = {
    .cs = SPI_NOR_CS0,
    .bus_clk = SPI_NOR_CLK_25MHZ,
    .octal_ddr_en = 0,
};

static void load_route_table(vaddr_t dst, const char *partition, size_t size)
{
    struct storage_device *storage;
    struct partition_device *ptdev;
    uint64_t ptn;
    int ret;

    dprintf(ALWAYS, "Loading routing table from partition %s\n", partition);

    storage = setup_storage_dev(OSPI, RES_OSPI_REG_OSPI1, &g_ospi_cfg);

    if (storage == NULL) {
        dprintf(ALWAYS, "setup_storage_dev error!\n");
        return;
    }

    ptdev = ptdev_setup(storage, storage->get_erase_group_size(storage) * 2);

    if (ptdev == NULL) {
        dprintf(ALWAYS, "ptdev_setup error!\n");
        goto out_with_storage;
    }

    ret = ptdev_read_table(ptdev);

    if (ret) {
        dprintf(ALWAYS, "ptdev_read_table error!\n");
        goto out_with_ptdev;
    }

    ptn = ptdev_get_offset(ptdev, partition);

    if (!ptn) {
        dprintf(ALWAYS, "ptdev_get_offset error!\n");
        goto out_with_ptdev;
    }

    /* Cache is flushed by flash driver */
    storage->read(storage, ptn, (uint8_t *)dst, size);

out_with_ptdev:
    ptdev_destroy(ptdev);

out_with_storage:
    storage_dev_destroy(storage);
}

static int sdpe_start_routing_test(int argc, const cmd_args *argv)
{
    const char *partition = "routing-table";

    if (argc > 1) {
        partition = argv[1].str;
    }

    /* before loading route table, must stop routing first */
    sdpe_stop_routing();
    load_route_table(ROUTE_TAB_MEMBASE, partition, ROUTE_TAB_MEMSIZE);
    sdpe_start_routing((uint32_t)ROUTE_TAB_MEMBASE, ROUTE_TAB_MEMSIZE);

    printf("%s() Result: %d\n", __func__, 0);
    return 0;
}

static int sdpe_start_stop_routing_test(int argc, const cmd_args *argv)
{
    const char *partition[2] = {"routing-table", "misc"};
    int count = 100;

    if (argc > 3) {
        count = argv[1].i;
        partition[0] = argv[2].str;
        partition[1] = argv[3].str;
    }

    /* before loading route table, must stop routing first */
    sdpe_stop_routing();

    while (count--) {
        load_route_table(ROUTE_TAB_MEMBASE, partition[count % 2], ROUTE_TAB_MEMSIZE);
        sdpe_start_routing((uint32_t)ROUTE_TAB_MEMBASE, ROUTE_TAB_MEMSIZE);
        thread_sleep(500);
        sdpe_stop_routing();
        thread_sleep(500);
    }

    printf("%s() Result: %d\n", __func__, 0);
    return 0;
}

static int sdpe_stop_routing_test(int argc, const cmd_args *argv)
{
    sdpe_stop_routing();
    printf("%s() Result: %d\n", __func__, 0);

    return 0;
}

static int get_input(void)
{
    int ret = 0;
    int temp = 0;

    while (true) {
        int temp = getchar();

        if (temp == 0xD) {
            /* Enter */
            printf("\n");
            break;
        }

        if (temp == 0x8) {
            /* Backspace */
            ret /= 10;
            printf("\b \b");
            continue;
        }
        if ((temp < 0x30) || (temp > 0x39)) {
            printf("Please input number\n");
            continue;
        }

        printf("%d", temp - 0x30);

        ret *= 10;
        ret += (temp - 0x30);
    }

    return ret;
}

static void sdpe_monitor_node_lost_event(bool en)
{
    if (en) {
        struct ecu_node_pdu_group {
            uint32_t node_id;
            uint8_t port_id;
            uint8_t bus_id;
            uint8_t frame_nr;
            uint8_t rcvry_cnfrm_cnt;
            uint32_t interval;
            uint32_t frame_id[30];
        } cfg;

        printf("Input node id:\n");
        cfg.node_id = get_input();
        printf("Input port id:\n");
        cfg.port_id = get_input();
        printf("Input bus id:\n");
        cfg.bus_id = get_input();
        printf("Input frame number:\n");
        cfg.frame_nr = get_input();
        while (cfg.frame_nr > 30U) {
            printf("Frame number must not be more than 30, "
                    "re-input frame number:\n");
            cfg.frame_nr = get_input();
        }
        printf("Input recovery confirmation counter:\n");
        cfg.rcvry_cnfrm_cnt = get_input();
        printf("Input interval:\n");
        cfg.interval = get_input();
        for (size_t i = 0; i < cfg.frame_nr; i++) {
            printf("Input frame ID [%d]:\n", i);
            cfg.frame_id[i] = get_input();
        }

        sdpe_monitor_event(0U, 1U, sizeof(cfg), (void *)&cfg);
    }
    else {
        printf("Input node id:\n");
        uint32_t node_id = get_input();

        sdpe_monitor_event(0U, 0U, sizeof(node_id), (void *)&node_id);
    }
}

static void sdpe_monitor_msg_timeout_event(bool en)
{
    if (en) {
        struct msg_pdu_group {
            uint32_t msg_id;
            uint8_t port_id;
            uint8_t bus_id;
            uint8_t frame_nr;
            uint32_t interval;
            uint32_t frame_id[30];
        } msg_grp;

        printf("Input message id:\n");
        msg_grp.msg_id = get_input();
        printf("Input port id:\n");
        msg_grp.port_id = get_input();
        printf("Input bus id:\n");
        msg_grp.bus_id = get_input();
        printf("Input frame number:\n");
        msg_grp.frame_nr = get_input();
        while (msg_grp.frame_nr > 30U) {
            printf("Frame number must not be more than 30, "
                    "re-input frame number:\n");
            msg_grp.frame_nr = get_input();
        }
        printf("Input interval:\n");
        msg_grp.interval = get_input();
        for (size_t i = 0; i < msg_grp.frame_nr; i++) {
            printf("Input frame ID [%d]:\n", i);
            msg_grp.frame_id[i] = get_input();
        }

        sdpe_monitor_event(2U, 1U, sizeof(msg_grp), (void *)&msg_grp);
    }
    else {
        printf("Input message id:\n");
        uint32_t msg_id = get_input();

        sdpe_monitor_event(2U, 0U, sizeof(msg_id), (void *)&msg_id);
    }
}

static void sdpe_monitor_pkt_ovf_event(bool en)
{
    if (en) {
        struct pkt_info {
            uint32_t pdu_id;
            uint8_t port_id;
            uint8_t bus_id;
            uint32_t frame_id;

            uint32_t interval;
            uint32_t pdu_nr;
        } pkt;

        printf("Input pdu id:\n");
        pkt.pdu_id = get_input();
        printf("Input port id:\n");
        pkt.port_id = get_input();
        printf("Input bus id:\n");
        pkt.bus_id = get_input();
        printf("Input frame ID:\n");
        pkt.frame_id = get_input();
        printf("Input interval:\n");
        pkt.interval = get_input();
        printf("Input threshold:\n");
        pkt.pdu_nr = get_input();

        sdpe_monitor_event(8U, 1U, sizeof(pkt), (void *)&pkt);
    }
    else {
        printf("Input pdu id:\n");
        uint32_t pdu_id = get_input();

        sdpe_monitor_event(8U, 0U, sizeof(pdu_id), (void *)&pdu_id);
    }
}

static void sdpe_bus_mirror(bool en)
{
    struct bus {
        uint8_t port_id;
        uint8_t bus_id;
    } tgt_bus;

    struct blacklist {
        uint32_t start;
        uint32_t end;
    };

    struct mirror_info {
        struct bus src;
        struct bus tgt;
        uint32_t blacklist_size;
        struct blacklist item[5];
    } mirror;

    if (en) {
        printf("Input source port id:\n");
        mirror.src.port_id = get_input();
        printf("Input source bus id:\n");
        mirror.src.bus_id = get_input();
        printf("Input target port id:\n");
        mirror.tgt.port_id = get_input();
        printf("Input target bus id:\n");
        mirror.tgt.bus_id = get_input();
        printf("Input blacklist size:\n");
        mirror.blacklist_size = get_input();
        while (mirror.blacklist_size > 5U) {
            printf("Blacklist size must not be more than 5, "
                    "re-input blacklist size:\n");
            mirror.blacklist_size = get_input();
        }
        for (size_t i = 0; i < mirror.blacklist_size; i++) {
            printf("Input blacklist item %d start id:\n", i);
            mirror.item[i].start = get_input();
            printf("Input blacklist item %d end id:\n", i);
            mirror.item[i].end = get_input();
        }

        sdpe_monitor_event(10U, 1U, sizeof(mirror),
                            (void *)&mirror);
    }
    else {
        printf("Input target port id:\n");
        tgt_bus.port_id = get_input();
        printf("Input target bus id:\n");
        tgt_bus.bus_id = get_input();

        sdpe_monitor_event(10U, 0U, sizeof(tgt_bus), (void *)&tgt_bus);
    }
}

static void sdpe_monitor_live_cnt_event(bool en)
{
    if (en) {
        struct live_cnt_cfg {
            uint32_t pdu_id;
            uint32_t frame_id;
            uint8_t port_id;
            uint8_t bus_id;
            uint8_t live_cnt_byte;
            uint8_t live_cnt_bit;
        } live_cnt;

        printf("Input pdu id:\n");
        live_cnt.pdu_id = get_input();
        printf("Input port id:\n");
        live_cnt.port_id = get_input();
        printf("Input bus id:\n");
        live_cnt.bus_id = get_input();
        printf("Input frame id:\n");
        live_cnt.frame_id = get_input();
        printf("Input live counter byte position:\n");
        live_cnt.live_cnt_byte = get_input();
        printf("Input live counter bit position:\n");
        live_cnt.live_cnt_bit = get_input();

        sdpe_monitor_event(5U, 1U, sizeof(live_cnt), (void *)&live_cnt);
    }
    else {
        printf("Input pdu id:\n");
        uint32_t pdu_id = get_input();

        sdpe_monitor_event(5U, 0U, sizeof(pdu_id), (void *)&pdu_id);
    }
}

static void sdpe_monitor_checksum_err_event(bool en)
{
    if (en) {
        struct cs_cfg {
            uint32_t pdu_id;
            uint32_t frame_id;
            uint8_t port_id;
            uint8_t bus_id;
            uint8_t cs_byte;
        } cs;

        printf("Input pdu id:\n");
        cs.pdu_id = get_input();
        printf("Input port id:\n");
        cs.port_id = get_input();
        printf("Input bus id:\n");
        cs.bus_id = get_input();
        printf("Input frame id:\n");
        cs.frame_id = get_input();
        printf("Input checksum byte position:\n");
        cs.cs_byte = get_input();

        sdpe_monitor_event(6U, 1U, sizeof(cs), (void *)&cs);
    }
    else {
        printf("Input pdu id:\n");
        uint32_t pdu_id = get_input();

        sdpe_monitor_event(6U, 0U, sizeof(pdu_id), (void *)&pdu_id);
    }
}

static void sdpe_monitor_invasion_event(bool en)
{
    uint32_t port_id;

    printf("Input port id:\n");
    port_id = get_input();

    sdpe_monitor_event(7U, en, sizeof(port_id), (void *)&port_id);
}

static int sdpe_monitor_event_test(int argc, const cmd_args *argv)
{
    if (argc < 3) {
        printf("Paramter: <event id> <enable>\n");
        return -1;
    }

    uint32_t event_id = argv[1].u;
    uint8_t en = argv[2].u;

    if (event_id == 0U) {
        sdpe_monitor_node_lost_event(en);
    }
    else if (event_id == 2U) {
        sdpe_monitor_msg_timeout_event(en);
    }
    else if (event_id == 5U) {
        sdpe_monitor_live_cnt_event(en);
    }
    else if (event_id == 6U) {
        sdpe_monitor_checksum_err_event(en);
    }
    else if (event_id == 7U) {
        sdpe_monitor_invasion_event(en);
    }
    else if (event_id == 8U) {
        sdpe_monitor_pkt_ovf_event(en);
    }
    else if (event_id == 10U) {
        sdpe_bus_mirror(en);
    }

    return 0;
}


static void sdpe_usage(void)
{
    printf("sdpe start_routing");
    printf("\t:sdpe start routing test\n");

    printf("sdpe stop_routing");
    printf("\t:sdpe stop routing test\n");

    printf("sdpe monitor_event");
    printf("\t:sdpe monitor event test [event_id] [enable] [arg]\n");
}

int do_sdpe_cmd(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        sdpe_usage();
        return 0;
    }

    if (!strcmp(argv[1].str, "start_routing")) {
        sdpe_start_routing_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "stop_routing")) {
        sdpe_stop_routing_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "monitor_event")) {
        sdpe_monitor_event_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "start_stop_routing")) {
        sdpe_start_stop_routing_test(argc-1, &argv[1]);
    }
    else
    {
        printf("error cmd\n");
    }

    return 0;
}

#include <lib/console.h>

STATIC_COMMAND_START
STATIC_COMMAND("sdpe", "sdpe test", (console_cmd)&do_sdpe_cmd)
STATIC_COMMAND_END(cmd_sdpe);

APP_START(cmd_sdpe)
.flags = 0,
APP_END
