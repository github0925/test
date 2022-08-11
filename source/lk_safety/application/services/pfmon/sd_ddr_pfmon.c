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
#include <thread.h>
#include <platform/interrupts.h>
#include <heap.h>
#include <string.h>
#include "sd_ddr_pfmon.h"
#include <app.h>

#define PFM_RECORD_MASTER_ID_NUM_MAX 16
#define PFM_MASTER_ID_NUM_MAX 128
#define PFM_MASTER_NAME_LEN_MAX 16

#define PFM_SEC_DIV_WINDOW_TIME 2 // window time 500ms, 1s/500ms = 2
#define PFM_WINDOW_TIME_MS 500 //window time 500ms

static uint32_t pfm_serial_print = 0;
static uint32_t pfm_serial_print_time = 0; //sec

#if PFM_RECORD_ON_DDR
static uint32_t pfm_dump_addr = 0;
static int pfm_dump_size = 0;
static int pfm_mem_full = 0;
static uint32_t rounds;
#endif
static int pfm_mem_auto_dump = 0;


struct pfm_record_info {
    uint32_t rd_cnt;
    uint32_t wr_cnt;
    uint32_t rd_bcntl;
    uint32_t wr_bcntl;
};

struct pfm_out_print_group_info {
    struct pfm_record_info rec_inf[PFM_SEC_DIV_WINDOW_TIME];
};

struct pfm_out_print_info {
    uint32_t is_fill;
    uint32_t write_index;
    struct pfm_out_print_group_info group_inf[PFM_RECORD_MASTER_ID_NUM_MAX];
};

struct pfm_out_print_info out_info[2];

static struct pfm_record_irq *current;

pfm_observer_t def_observer[PFM_RECORD_MASTER_ID_NUM_MAX] = {
    {AP1,  0x7f,  AP2, 0x7f},  /* AP 3,4*/
    {GPU1_OS1, 0x78, GPU1_OS1, 0x78},  /* GPU1 56*/
    {GPU2_OS1, 0x78, GPU2_OS1, 0x78},  /* GPU2 64*/
    {VPU1, 0x7f, VPU1, 0x7f},  /* VPU1 29*/
    {VPU2, 0x7f, VPU2, 0x7f},  /* VPU2 30*/
    {DP1, 0x7f, DP1, 0x7f},  /* DP1 23*/
    {DP2, 0x7f, DP2, 0x7f},  /* DP2 24*/
    {DP3, 0x7f, DP3, 0x7f},  /* DP3 25*/
    {DC1, 0x7f, DC1, 0x7f},  /* DC1 19*/
    {DC2, 0x7f, DC2, 0x7f},  /* DC2 20*/
    {DC3, 0x7f, DC3, 0x7f},  /* DC3 21*/
    {DC4, 0x7f, DC4, 0x7f},  /* DC4 22*/
    {DC5, 0x7f, DC5, 0x7f},  /* DC5 26*/
    {CSI1, 0x7e, CSI3, 0x7f},  /* CSI 16,18*/
    {VDSP,  0x7f,  VDSP, 0x7f},  /* VDSP 5*/
    {0, 0, 0, 0} /* ALL */
};

static uint32_t pfm_enable_group(uint32_t nr)
{
    if (nr > PFM_RECORD_MASTER_ID_NUM_MAX)
        return -1;
    writel(BIT_MASK(nr), PFM_MON_GROUP);
    return 0;
}

static uint32_t pfm_group_nr(void)
{
    int i = 0;
    unsigned long ret = readl(PFM_MON_GROUP) & 0xffff;
    for (i = 15; i >= 0; i--)
    {
        if (bitmap_test(&ret, i))
	{
	    return i + 1;
	}
    }
    return 0;
}

/* Time in ms */
static uint32_t pfm_cfg_time_window(uint32_t time)
{
    writel(time_window_per_ms * time, PFM_MON_BASE_CNT_CMP);
    return 0;
}

static uint32_t pfm_master_observer(uint32_t group, pfm_observer_t *observer)
{
    if ((group > PFM_RECORD_MASTER_ID_NUM_MAX) || (observer == NULL))
        return -1;
    writel((observer->master0)<<6, PFM_MON_AXI_ID0(group));
    writel((observer->msk0)<<6, PFM_MON_AXI_ID0_MSK(group));

    writel((observer->master1)<<6, PFM_MON_AXI_ID1(group));
    writel((observer->msk1)<<6, PFM_MON_AXI_ID1_MSK(group));

    return 0;
}

static uint32_t pfm_stop_trigger(uint32_t group, pfm_stop_condition_t *condition)
{
    if ((group > PFM_RECORD_MASTER_ID_NUM_MAX) || (condition == NULL))
        return -1;
    writel(condition->rd_thr, PFM_MON_AXI_RD_THR(group));
    writel(condition->wr_thr, PFM_MON_AXI_WR_THR(group));

    writel(condition->rd_bcnt_thr, PFM_MON_AXI_RD_BCNT_THR(group));
    writel(condition->wr_bcnt_thr, PFM_MON_AXI_WR_BCNT_THR(group));

    return 0;
}

static uint32_t pfm_transaction_record(uint32_t nr_group, struct pfm_record_irq *record)
{
    uint32_t i = 0;
    struct pfm_out_print_info * info_temp = NULL;
    uint32_t write_index;

    if(out_info[0].is_fill == 0){
        info_temp = &out_info[0];
    }else if(out_info[1].is_fill == 0){
        info_temp = &out_info[1];
    }else{
        info_temp = NULL;
    }

    if (nr_group > 16){
        return -1;
    }

    arch_invalidate_cache_range(PFM_MON_AXI_RD_CNT(0), nr_group * 24);

#if PFM_RECORD_ON_DDR
    if (record == NULL){
        return -1;
    }
    if(info_temp != NULL){

        write_index = info_temp->write_index;

        for (i = 0; i < nr_group; i++)
        {
            record[i].rd_cnt = readl(PFM_MON_AXI_RD_CNT(i));
            info_temp->group_inf[i].rec_inf[write_index].rd_cnt = record[i].rd_cnt;
            record[i].wr_cnt = readl(PFM_MON_AXI_WR_CNT(i));
            info_temp->group_inf[i].rec_inf[write_index].wr_cnt = record[i].wr_cnt;
            record[i].rd_bcntl = readl(PFM_MON_AXI_RD_BCNTL(i));
            info_temp->group_inf[i].rec_inf[write_index].rd_bcntl = record[i].rd_bcntl;
            record[i].rd_bcnth = readl(PFM_MON_AXI_RD_BCNTH(i));
            record[i].wr_bcntl = readl(PFM_MON_AXI_WR_BCNTL(i));
            info_temp->group_inf[i].rec_inf[write_index].wr_bcntl = record[i].wr_bcntl;
            record[i].wr_bcnth = readl(PFM_MON_AXI_WR_BCNTH(i));
        }

        info_temp->write_index = ((info_temp->write_index) + 1)%PFM_SEC_DIV_WINDOW_TIME;
        if(info_temp->write_index == 0){
            info_temp->is_fill = 1;
        }

    }else{
        for (i = 0; i < nr_group; i++)
        {
            record[i].rd_cnt = readl(PFM_MON_AXI_RD_CNT(i));
            record[i].wr_cnt = readl(PFM_MON_AXI_WR_CNT(i));
            record[i].rd_bcntl = readl(PFM_MON_AXI_RD_BCNTL(i));
            record[i].rd_bcnth = readl(PFM_MON_AXI_RD_BCNTH(i));
            record[i].wr_bcntl = readl(PFM_MON_AXI_WR_BCNTL(i));
            record[i].wr_bcnth = readl(PFM_MON_AXI_WR_BCNTH(i));
        }
    }
#else
    if(info_temp != NULL){

        write_index = info_temp->write_index;

        for (i = 0; i < nr_group; i++)
        {
            info_temp->group_inf[i].rec_inf[write_index].rd_cnt = readl(PFM_MON_AXI_RD_CNT(i));
            info_temp->group_inf[i].rec_inf[write_index].wr_cnt = readl(PFM_MON_AXI_WR_CNT(i));
            info_temp->group_inf[i].rec_inf[write_index].rd_bcntl = readl(PFM_MON_AXI_RD_BCNTL(i));
            info_temp->group_inf[i].rec_inf[write_index].wr_bcntl = readl(PFM_MON_AXI_WR_BCNTL(i));
        }

        info_temp->write_index = ((info_temp->write_index) + 1)%PFM_SEC_DIV_WINDOW_TIME;
        if(info_temp->write_index == 0){
            info_temp->is_fill = 1;
        }

    }
#endif
    return 0;
}

/* Enable pfm */
static void pfm_enable_counter(void)
{
    /* Clear base counter */
    writel(2, PFM_MON_BASE_CNT_CTL);
    /* Enable base counter */
    writel(1, PFM_MON_BASE_CNT_CTL);
}

static void pfm_disable_counter(void)
{
    writel(0, PFM_MON_BASE_CNT_CTL);
}

static int pfm_timeout_irq(void)
{
	return !!(readl(PFM_MON_MISC_ST) & 0x10);
}

static void pfm_enable_irq_mode(void)
{
    /* Enable base counter timeout irq && auto restart */
    uint32_t ret = readl(PFM_MON_AXI_CNT_CTL1);
    ret |= 2;
    writel(ret, PFM_MON_AXI_CNT_CTL1);
    /* Enable timeout irq  */
    ret = readl(PFM_MON_MISC_INT_CTL);
    ret |= 1;
    writel(ret, PFM_MON_MISC_INT_CTL);
}

static void pfm_enable_dma_mode(void)
{
    /* Enable AXI threshold irq, auto restart */
    writel(1, PFM_MON_AXI_CNT_CTL1);
    /* Disable base counter timeout irq */
    uint32_t ret = readl(PFM_MON_MISC_INT_CTL);
    ret &= 0;
    writel(ret, PFM_MON_MISC_INT_CTL);
}
#if PFM_RECORD_ON_DDR
static void pfm_save_config(pfm_head_t *pfm_head)
{
    pfm_head->magic = PFM_RECORD_MAGIC;
    pfm_head->time_window = readl(PFM_MON_BASE_CNT_CMP)/time_window_per_ms;
    pfm_head->group_nr = pfm_group_nr();
    for (uint32_t i = 0; i < pfm_head->group_nr; i++)
    {
        pfm_head->observer_config[i].master0 = readl(PFM_MON_AXI_ID0(i)) >> 6;
        pfm_head->observer_config[i].msk0 = readl(PFM_MON_AXI_ID0_MSK(i)) >> 6;
        pfm_head->observer_config[i].master1 = readl(PFM_MON_AXI_ID1(i)) >> 6;
        pfm_head->observer_config[i].msk1 = readl(PFM_MON_AXI_ID1_MSK(i))  >> 6;
    }

    for (uint32_t i = 0; i < pfm_head->group_nr && i < 8; i++)
    {
        pfm_head->condition[i].rd_thr = readl(PFM_MON_AXI_RD_THR(i));
        pfm_head->condition[i].wr_thr = readl(PFM_MON_AXI_WR_THR(i));
        pfm_head->condition[i].rd_bcnt_thr = readl(PFM_MON_AXI_RD_BCNT_THR(i));
        pfm_head->condition[i].wr_bcnt_thr = readl(PFM_MON_AXI_WR_BCNT_THR(i));
    }

    pfm_head->mode = readl(PFM_MON_MISC_INT_CTL) & 1;
    pfm_head->reach_thr = 0;
}

void pfm_dump_mem(const char* info, const void* content, uint32_t content_len)
{
    printf("%s: \n", info);
    uint32_t i = 0;
    uint32_t left_num = 0;

    left_num = content_len % 4;

    if(left_num != 0){
        printf("input content_len must 4 bytes rund \n");
        return;
    }

    for (i =0 ; i < content_len; ) {
        printf("%02x", *((uint8_t*)(content) + 1 + i));
        printf("%02x", *((uint8_t*)(content) + i));
        printf("%02x", *((uint8_t*)(content) + 3 + i));
        printf("%02x", *((uint8_t*)(content) + 2 + i));
        printf("\n");
        i = i + 4;
    }
}
#endif
static enum handler_return pfm_irq_handler(void *arg)
{
    uint32_t group;
    //struct pfm_record_irq *record, *base;
    if (pfm_timeout_irq()) {
        group = pfm_group_nr();
        if (group == 0)
        {
            printf("no channel enabled\n");
            return INT_NO_RESCHEDULE;
        }
#if PFM_RECORD_ON_DDR
        if (!current) {
            printf("no pfm record buffer specified\n");
            return INT_NO_RESCHEDULE;
        }
        if(pfm_mem_full == 0){
            pfm_transaction_record(group, current);
        }

        current += group;
        rounds++;

	    if ((uint32_t)current >= (PFM_RECORD_BUFFER + PFM_RECORD_BUFFER_SIZE)) {
            pfm_mem_full = 1;
            current = (struct pfm_record_irq *)(PFM_RECORD_BUFFER + PFM_MON_HEAD_OFFSET);
	    }
#else
        pfm_transaction_record(group, current);
#endif
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
        printf("run pfm irq mode\n");
#if PFM_RECORD_ON_DDR
        current = (struct pfm_record_irq *)(PFM_RECORD_BUFFER + PFM_MON_HEAD_OFFSET);
        rounds = 0;
        pfm_head_t *head = (pfm_head_t *)(PFM_RECORD_BUFFER);
        head->start_time = 0;
        pfm_enable_irq_mode();
        pfm_register_irq_handle();
        pfm_enable_counter();
#else
        pfm_enable_irq_mode();
        pfm_register_irq_handle();
        pfm_enable_counter();
#endif
        printf("run PFM via irq mode\n");
        return 0;
    } else if (!strcmp(argv[1].str, "dma")) {
        pfm_enable_dma_mode();
        pfm_enable_counter();
        return 0;
    } else if(!strcmp(argv[1].str, "auto_dump")){
        pfm_mem_auto_dump = 1;
    }

    printf("usage: pfm_enable irq/dma \n");
    return -1;
}

int pfm_disable_internal(void)
{
    pfm_disable_counter();
#if PFM_RECORD_ON_DDR
    pfm_head_t *head = (pfm_head_t *)(PFM_RECORD_BUFFER);
    pfm_save_config(head);
    head->rounds = rounds;
    head->pool_size = PFM_RECORD_BUFFER_SIZE;
    head->record_offset = PFM_MON_HEAD_OFFSET;
    arch_clean_cache_range(PFM_RECORD_BUFFER, PFM_RECORD_BUFFER_SIZE);
#endif
    return 0;
}

int pfm_disable(int argc, const cmd_args *argv)
{
    int ret = 0;
    ret = pfm_disable_internal();
    return ret;
}

int pfm_time_window(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        printf("bad arguments\n");
        return -1;
    }
    uint32_t time_window = argv[1].u;
    pfm_cfg_time_window(time_window);
    return 0;
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

int pfm_default(int argc, const cmd_args *argv)
{
    pfm_cfg_time_window(PFM_WINDOW_TIME_MS);
    //pfm_cfg_time_window(1);
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
#if PFM_RECORD_ON_DDR
int pfm_debug(int argc, const cmd_args *argv)
{
    pfm_head_t *head = calloc(1, sizeof(pfm_head_t));
    pfm_save_config(head);
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


int pfm_dump(int argc, const cmd_args *argv)
{
    int dump_size = 0;

    if (argc != 3) {
        printf("input argv error\n");
        return -1;
    }

    dump_size = argv[2].u;

    if((dump_size > PFM_RECORD_BUFFER_SIZE)||(dump_size < 0)){
        printf("input dump len error\n");
        return -2;
    }else{
        pfm_dump_size = dump_size;
    }

    pfm_dump_addr = argv[1].u;

    return 0;
}
#endif

int pfm_print(int argc, const cmd_args *argv)
{

    if (argc == 2) {
        pfm_serial_print = argv[1].u;
        pfm_serial_print_time = 0xffffffff;
    }else if(argc == 3){
        pfm_serial_print = argv[1].u;
        pfm_serial_print_time = argv[2].u;
    }else{
        printf("input argv error\n");
        return -1;
    }

    return 0;
}

void pfm_print_name_line(void){
    printf("[ap1/2] [gpu1] [gpu2] [vpu1] [vpu2] [dp1] [dp2] [dp3] [dc1] [dc2] [dc3] [dc4] [dc5] [csi1/3] [vdsp] [all]\n");
}

void pfm_print_value_line(void){
    uint32_t i = 0;
    struct pfm_out_print_info * info_temp = NULL;
    int rd_val = 0;
    int wr_val = 0;

    if(out_info[0].is_fill == 1){
        info_temp = &out_info[0];
    }else if(out_info[1].is_fill == 1){
        info_temp = &out_info[1];
    }else{
        info_temp = NULL;
    }

    if(info_temp != NULL){

        for(i = 0; i < PFM_RECORD_MASTER_ID_NUM_MAX; i++)
        {
            rd_val = ((info_temp->group_inf[i].rec_inf[0].rd_bcntl)>>20)+((info_temp->group_inf[i].rec_inf[1].rd_bcntl)>>20);
            wr_val = ((info_temp->group_inf[i].rec_inf[0].wr_bcntl)>>20)+((info_temp->group_inf[i].rec_inf[1].wr_bcntl)>>20);
            printf("%d M %d M : ", rd_val, wr_val);
        }
        printf("\n");
        info_temp->write_index = 0;
        info_temp->is_fill = 0;
    }
}

static void pfm_server(const struct app_descriptor *app, void *args)
{
    uint32_t print_name = 0;
    cmd_args cmd[2] ={0};

    cmd[1].str = "irq";

    thread_sleep(3000); //wait for ddr init done
    pfm_default(0, NULL);
    pfm_enable(2, cmd);

    printf("start pfm daemon service\n");

    while(1){
#if PFM_RECORD_ON_DDR
        if(pfm_dump_size != 0){
            printf("input dump addr is =%x, len is =%d\n", pfm_dump_addr, pfm_dump_size);
            if(pfm_dump_addr == 0){
                pfm_dump_mem("begin dump", (void *)PFM_RECORD_BUFFER, pfm_dump_size);
            }else{
                pfm_dump_mem("begin dump", (void *)pfm_dump_addr, pfm_dump_size);
            }
            pfm_dump_size = 0;
        }else if((pfm_mem_full != 0)&&(pfm_mem_auto_dump == 1)){
            pfm_disable_internal();
            pfm_dump_mem("pfm mem is full, begin dump", (void *)PFM_RECORD_BUFFER, PFM_RECORD_BUFFER_SIZE);
            pfm_mem_full = 0;
            pfm_enable(2, cmd);
        }
#endif
        if(pfm_serial_print == 1)
        {
            if(print_name == 0){
                printf("\n");
                //print name line
                pfm_print_name_line();
                print_name = 1;
            }

           //print value
           if(pfm_serial_print_time != 0){
                pfm_print_value_line();
                pfm_serial_print_time--;
           }
        }else{
            print_name = 0;
        }

        thread_sleep(500);
    }
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("pfm_enable", "pfm_enable type; enable perf monitor,type : irq, dma, auto_dump", (console_cmd)&pfm_enable)
STATIC_COMMAND("pfm_disable", "disable perf monitor", (console_cmd)&pfm_disable)
STATIC_COMMAND("pfm_window",  "pfm time window(ms)", (console_cmd)&pfm_time_window)
STATIC_COMMAND("pfm_group",  "ebable channel nomber", (console_cmd)&pfm_setup_group)
STATIC_COMMAND("pfm_channal",  "observer config pfm_channal cid mid0 msk0 mid1 msk1;", (console_cmd)&pfm_channel_configure)
STATIC_COMMAND("pfm_trigger",  "stop condition trigger", (console_cmd)&pfm_channel_tigger)
STATIC_COMMAND("pfm_default",  "pfm default observer", (console_cmd)&pfm_default)
#if PFM_RECORD_ON_DDR
STATIC_COMMAND("pfm_debug",  "pfm debug", (console_cmd)&pfm_debug)
STATIC_COMMAND("pfm_dump",  "pfm_dump addr size: pfm_dump 0 16 will dump from PFM_RECORD_BUFFER_START_ADDR 16 bytes", (console_cmd)&pfm_dump)
#endif
STATIC_COMMAND("pfm_print",  "pfm_print 1 60; print on 60s", (console_cmd)&pfm_print)
STATIC_COMMAND_END(pfmon);
#endif

APP_START(pfm)
.flags = 0,
.entry = pfm_server,
.stack_size = 1024,
.flags = APP_FLAG_CUSTOM_STACK_SIZE,
APP_END
