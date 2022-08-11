/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <mini_libc/mini_libc.h>
#include <service.h>
#include <soc.h>
#include <arch.h>
#include "uart/uart.h"
#include "shell/shell.h"
#include "memtester/memtester.h"
#include <rstgen/rstgen.h>
#include "DWC_ddr_umctl2_reg.h"
#include "driver/bt_dev/include/usb_if.h"
#include "driver/bt_dev/usb/class/ezusb/ezusb.h"


#if defined(CFG_RUN_DDR_INIT_SEQ)
#include "ddr/dw_umctl2/inc/ddr_init_helper.h"
#endif
#if WITH_SMP
#include "arch/spinlock.h"
#endif
#include "app_para.h"
#if defined(BOARD)
#include "board.h"
#endif
#ifdef RUNNING_IN_MEMORY
#include "ddr_map.h"
#endif
#include "ddr_pfm/ddr_pfmon.h"



#ifdef DDR_DIAG
#define APP_PARA_START  0x1f7fe0u
#else
#define AP_IMG_START    0x1B0000u
#define APP_PARA_START  0x1a7fe0u
#endif
#define USB_ENUMERATE_TIME_OUT  (1000*1000*5)
#define USB_READY_TIME_OUT  (1000*1000*10)
#define USB_HOST_READY_MSG_LEN  7
#if defined(CFG_RUN_DDR_INIT_SEQ)
extern const ddr_act_u ddr_init_seq[];
extern int32_t run_ddr_init_seq(const ddr_act_u *act);
#endif

#if WITH_SMP
static spin_lock_t lock = SPIN_LOCK_INITIAL_VALUE;
#endif
uint32_t cores_online = 1;
uint32_t cores_done = 0;
int usb_if_ok = 0;
/* workaround: if .data section is empty, objcopy not work properly */
char *prod_str = "DDR_Squeezer";
#if defined(TGT_ap)
const char *cpu_str = "AP";
#else
const char *cpu_str = "Sec";
#endif
__attribute__((weak)) const char *board_str = "BOARD_x9_ref";
#if defined(TGT_ap)
#define MAX_CPU_NUM     6
static mem_range_t range[MAX_CPU_NUM];
uint32_t loops = 0;
uint32_t cmd_memtester(uint32_t argc, char *argv[]);
#endif

module_e tty = TTY_UART;
#ifndef AP_TTY_UART
#define AP_TTY_UART TTY_UART
#endif
int __main(int argc, char *argv[])
{
#ifdef  SHELL_USE_USB
    U8 host_data[16];
#endif
    uint32_t cpu_id = arch_curr_cpu_num();

    if (0u == cpu_id) {
#if defined(TGT_sec)
        // remap the vector for r5 core
        sec_soc_vector_init();
#endif
        tmr_enable();
    }

#if defined(TGT_sec)
    module_e tty_ap = AP_TTY_UART;
#endif

    if (0u == cpu_id) {
        app_para_t *para = (app_para_t *)(uintptr_t)APP_PARA_START;
#if defined(TGT_sec)

        if ((para->tag == APP_PARA_TAG) && (0 != (para->tty & 0xffu))) {
            tty = UART1 + (para->tty & 0xffu) - 1;
        }

        soc_deassert_reset(tty);
        soc_pin_cfg(tty, NULL);
        soc_config_clk(tty, UART_FREQ1);

        if ((para->tag == APP_PARA_TAG) && (0 != (((para->tty) >> 8) & 0xffu))) {
            tty_ap =  UART1 + (((para->tty) >> 8) & 0xffu) - 1;
        }

        if (tty_ap != tty) {
            soc_deassert_reset(tty_ap);
            soc_pin_cfg(tty_ap, NULL);
            soc_config_clk(tty_ap, UART_FREQ1);
        }

#else

        if ((para->tag == APP_PARA_TAG) && (0 != (((para->tty) >> 8) & 0xffu))) {
             tty = UART1 + ((para->tty >> 8) & 0xffu) - 1;
        }

#endif
        uart_cfg_t uart_cfg;
        memclr(&uart_cfg, sizeof(uart_cfg));
        uart_cfg.parity = UART_PARITY_NONE;
        uart_cfg.stop = STOP_1BIT;
        uart_cfg.baud_rate = 115200u;
        uart_init(tty, &uart_cfg);
#ifdef  SHELL_USE_USB
        usb_dev_init();
        /* wait for windows enumeration finish */
        U64 tk_tmot = tmr_tick() + SOC_us_TO_TICK(USB_ENUMERATE_TIME_OUT);
        do{
            udelay(1);
        }while(!ezusb_is_online()&&(tmr_tick() < tk_tmot));  
        if (tmr_tick() < tk_tmot) {
            usb_if_ok = 1;
        }
        /* wait for host ready */
        tk_tmot = tmr_tick() + SOC_us_TO_TICK(USB_READY_TIME_OUT);
        mini_memset_s((void*)&host_data[0],'\0',sizeof(host_data));
        if(usb_if_ok){
            do{ 
                if(USB_HOST_READY_MSG_LEN == usb_recv(host_data,USB_HOST_READY_MSG_LEN)){
                    if(!strncmp((const char*)host_data,"host ok",USB_HOST_READY_MSG_LEN)){
                        usb_if_ok = 2;
                        break;
                    }
                }
            }while(tmr_tick() < tk_tmot);
        }
#endif
#if defined(TGT_sec)

        if (tty_ap != tty) {
            /* init A core chosen uart if */
            soc_pin_cfg(tty_ap, NULL);
            uart_init(tty_ap, &uart_cfg);
        }

#endif
        DBG("\n\n%s: %s, built on %s at %s\n\n", cpu_str, prod_str, __DATE__, __TIME__);
    }

#if defined(BOARD)
    DBG("Board: %s\n", board_str);
    board_setup(0, 0, 0, 0);
#endif

#if defined(TGT_sec)
#if defined(CFG_RUN_DDR_INIT_SEQ)
    if ((0 != ddr_init_seq[0].call.act) && (0xffu != ddr_init_seq[0].call.act)) {
        DBG("To run DDR init sequence...\n");
        run_ddr_init_seq(&ddr_init_seq[0]);
    }
#endif  /* defined(CFG_RUN_DDR_INIT_SEQ) */
#ifdef DDR_DIAG
    while(1);//diag do not run init compeletly
#else
#ifdef RUNNING_IN_MEMORY
    // the image copy to ddr rui.wang
    mini_memcpy_s(( void *)DDR_SQUEESER_IMAGE_START_SEC,( void *)AP_IMG_START,DDR_SQUEESER_IMAGE_SIZE);
    arch_clean_invalidate_cache_range(( void *)DDR_SQUEESER_IMAGE_START_SEC,DDR_SQUEESER_IMAGE_SIZE);
    if(!mini_memcmp_s(( void *)DDR_SQUEESER_IMAGE_START_SEC,( void *)AP_IMG_START,DDR_SQUEESER_IMAGE_SIZE))
    {
       DBG("Image load to ddr Pass\n"); 
    }else{
       DBG("Image load to ddr Fail\n"); 
    }
#endif
/* disable firewall so safety can access IRAM2/3/4 */
#define MAC_GLB_CTL (0xf0bc0000u)
    uint32_t v = readl(MAC_GLB_CTL);
    v &= ~0x01u;
    writel(v, MAC_GLB_CTL);
    uint32_t cores = fuse_get_cores();
    
#ifdef  RUNNING_IN_MEMORY
    if ((0 != readl(DDR_SQUEESER_IMAGE_START_SEC)) && (0xffffffffu != readl(DDR_SQUEESER_IMAGE_START_SEC)) && (cores >= 1)) {
#else
    if ((0 != readl(AP_IMG_START)) && (0xffffffffu != readl(AP_IMG_START)) && (cores >= 1)) {
#endif
        if (1 == cores >> 8) {
            DBG("Kick AP2...\n");
#ifdef  SHELL_USE_USB
            usb_dev_deinit();
#endif
            udelay(20 * 1000); /* wait a while unitl all infor flush into tty uart */
            soc_kick_cpu(CPU_AP2);
#ifdef  RUNNING_IN_MEMORY
            soc_start_cpu_core(CPU_AP2, 0, DDR_SQUEESER_IMAGE_START_AP);
#else
            soc_start_cpu_core(CPU_AP2, 0, AP_IMG_START);
#endif
        } else {
            DBG("Kick AP1...\n");
#ifdef  SHELL_USE_USB
            usb_dev_deinit();
#endif
            udelay(20 * 1000); /* wait a while unitl all infor flush into tty uart */
            soc_kick_cpu(CPU_AP1);
#ifdef  RUNNING_IN_MEMORY
            soc_start_cpu_core(CPU_AP1, 0, DDR_SQUEESER_IMAGE_START_AP);
#else
            soc_start_cpu_core(CPU_AP1, 0, AP_IMG_START);
#endif
        }
    }else{
        DBG("AP Image illegal can't Kick AP\n");
    }
 
#if  (CFG_SEC_RUNNING_DDR_PFM_LOG == 1)
        /* USB if is used by A cores,so mast disable log only form uart output */
        usb_if_ok = 0;
        if(tty_ap == tty) {
            DBG("UART tty_ap == tty,log will confusion.\n")
        }
        ddr_profiling_init();
        while (1) {
            if(!pfm_poll_handler()){
                print_channel_record(15);
            }
        }
#else
    dsb();
    isb();
    while (1) {
        __asm volatile("wfi": : : "memory");
    }
#endif
#endif  /* DDR_DIAG */

#else   /* TGT_ap */

    if (0u == cpu_id) {
        init_xlat_table();
        enable_mmu();
#define DDR_SZ_8GB
#if defined(CFG_AUTORUN)
        char *argv[] = {"memtester", "0x40000000",
#if defined(DDR_SZ_8GB)
                        "0x200000000",
#else
                        "0x100000000",
#endif
                        "loop=100"
                       };
        cmd_memtester(4, argv);
#else
#ifndef  SHELL_USE_USB
        shell_loop(tty);
#else  
        shell_loop_usb();
#endif
#endif
    } else {
        enable_mmu();
        memtester_main((void *)&range[cpu_id], loops);
        #if WITH_SMP
        arch_spin_lock(&lock);
        #endif
        cores_done |= (0x01 << cpu_id);
        #if WITH_SMP
        arch_spin_unlock(&lock);
        #endif
        while (1) {
            __asm volatile("wfi": : : "memory");
        }
    }

#endif  /* defined(TGT_sec) */

    return 0;
}

#if defined(TGT_ap)
#if defined(RUNNING_IN_MEMORY)
#define IMAGE_RANGG_SIZE  DDR_SQUEESER_IMAGE_SIZE + 0x20000
#else
#define IMAGE_RANGG_SIZE  0
#endif
uint32_t cmd_memtester(uint32_t argc, char *argv[])
{
    if ((argc < 3) || (argc > 5)) {
        DBG("Usage: memtester start size [loop=m] [core=n]\n");
        return 0;
    }

 #define IS_VALID_DDR_RANGE(x, sz)   (((x) >= (DDR_MEMORY_BASE + IMAGE_RANGG_SIZE )) \
                                         && (((uintptr_t)(-1) - (x)) > (sz))\
                                         && ((x) + (sz) < 0x800000000ull))

    unsigned long long start = strtoull(argv[1], NULL, 0);
    unsigned long long size = strtoull(argv[2], NULL, 0);
    unsigned long long raw_start = start ,raw_size =  size;
    uint32_t n = fuse_get_cores();

    if (0 == n) {
        return -3;
    }

    if (1 == n >> 8) {  /* AP2 */
        n = 1;
    }

    uint32_t cores = n;
    loops = 1;

    for (int i = 3; i < argc; i++) {
        if (0 == strncmp(argv[i], "loop=", 5)) {
            loops = strtoul(argv[i] + 5, NULL, 0);
        } else if (0 == strncmp(argv[i], "core=", 5)) {
            cores = strtoul(argv[i] + 5, NULL, 0);
        }
    }

    cores = MIN(cores, n);

    if (size < 0x1000) {
        DBG("Opps, size shall be not less than 4KB.\n");
        return -1;
    }

    #ifdef RUNNING_IN_MEMORY
    if(start&0xFFF){
        DBG("start[%0p] is not  Align to 4K.\n", start);
        start = ROUNDUP(start, 0x1000);
    }
    else if (size & 0xFFF){
        DBG("size[%0p] is not  be Align to 4K.\n", size);
        size = ROUNDUP(size, 0x1000);
    }
    else if( start < ( DDR_MEMORY_BASE + IMAGE_RANGG_SIZE ) ){
        DBG("region[%p--%p] Stored code.\n",start,start + size);
        return -3;
    }
    #endif

    if (!IS_VALID_DDR_RANGE(start, size)) {
        DBG("Opps, ddr range shall be in[%p-%p]\n", DDR_MEMORY_BASE, 0x800000000ull);
        return -2;
    }

    DBG("To run memtester on {%p - %p}...\n", start, start + size);

    /* It has to be WB_NOWA for DDR ECC enabled scenario. Otherwise, cache line fill
     * happens before DDR memories be ECC initialized. */
    /* We only map the test range to Normal to avoid CA55 prefetch from non-inited ECC area */
    #ifdef RUNNING_IN_MEMORY
    if( ( ( IMAGE_RANGG_SIZE + DDR_MEMORY_BASE ) <= start && start < 0x80000000 ) ){
        uint64_t start_temp = remmap_l2_l3_range(start,size,ATTR_ID(NORMAL_WB_NOWA) | ATTR_SH_OUTER | ATTR_nG | ATTR_AF | ATTR_XN | ATTR_AP_RW);
        size -= start_temp - start;
        start =  start_temp;
    }
    #endif
    mmap_level1_range(1, start, size,
                      ATTR_ID(NORMAL_WB_NOWA) | ATTR_SH_OUTER | ATTR_nG | ATTR_AF | ATTR_XN | ATTR_AP_RW);
    /* For ECC enabled scenario, DDR range need to be write in 64bit manner to initialize ECC */
    memclr((void *)raw_start, ROUNDUP(raw_size, CACHE_LINE));
    arch_clean_invalidate_cache_range((const void *)raw_start, ROUNDUP(raw_size, CACHE_LINE));

    unsigned v = 0;
    soc_get_rand((uint8_t *)&v, sizeof(v));
    srand(v);

    for (int i = 0; i < cores; i++) {
        range[i].start = (addr_t)raw_start + ROUNDDOWN(raw_size / n, CACHE_LINE) * i;
        range[i].sz = (size_t)ROUNDDOWN(raw_size / n, CACHE_LINE);
    }

    range[n - 1].sz = raw_size - (ROUNDDOWN(raw_size / n, CACHE_LINE) * (n - 1));
    arch_clean_invalidate_cache_range((const void *)&range[0], ROUNDUP(sizeof(range)*(n-1),CACHE_LINE));

    for (int i = 1; i < cores; i++) {
        DBG("To start core%d\n", i);
        #ifdef  RUNNING_IN_MEMORY
            soc_start_cpu_core(CPU_AP1, i, DDR_SQUEESER_IMAGE_START_AP);
        #else
            soc_start_cpu_core(CPU_AP1, i, AP_IMG_START);
        #endif
        cores_online++;
        __asm volatile("sev" : : : "memory");
    }

    arch_clean_invalidate_cache_range((const void *)&cores_online, sizeof(uint32_t));

    memtester_main((void *)&range[0], loops);
    #if WITH_SMP
    arch_spin_lock(&lock);
    #endif
    cores_done |= 0x01;
    #if WITH_SMP
    arch_spin_unlock(&lock);
    #endif

    while (cores_done != ((0x01u << cores_online) - 1));
    
    DBG("cores_done = 0x%x\n", cores_done);
    udelay(100);

#if defined(TODO_FIX_STOP_CORE_BUT_HANG_ISSUE)

    for (int i = 1; i < cores; i++) {
        DBG("To stop core%d\n", i);
        #if WITH_SMP
        arch_spin_lock(&lock);
        #endif
        soc_stop_secondary_core(CPU_AP1, i);
        udelay(1000);
        cores_done &= ~(0x01 << i);
        cores_online--;
        #if WITH_SMP
        arch_spin_unlock(&lock);
        #endif
    }

#endif

    return 0;
}

SHELL_CMD("memtester", cmd_memtester, "Usage: memtester start size [loop=m] [core=n]")

uint32_t cmd_systemreset(uint32_t argc, char *argv[])
{
    if ((argc > 1)) {
        DBG("Usage: systemreset");
        return 0;
    }
    
    DBG("System will reset.....\n");
    udelay(20 * 1000); /* wait a while unitl all infor flush into tty uart */
    rg_glb_reset_en(APB_RSTGEN_SEC_BASE, 1);    // self sw reset enable
    rg_glb_self_reset(APB_RSTGEN_SEC_BASE, 1);
    rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_0_INDEX, 1);

    return 0;
}

SHELL_CMD("systemreset", cmd_systemreset, "Usage: systemreset")

/*
    The PIN signal is not mapped according to the register bit sequence, this interface performs conversion
*/
#if 0
const static  uint8_t swap_tab[8] = {5,7,4,6,0,2,1,3};
static uint8_t pin_swapswap(uint8_t raw_vel)
{
    uint8_t ret = 0;
    for(int i=0;i<8;i++)
    {
        if(raw_vel  & (0x01<< swap_tab[i])){
            ret |= 0x01 << i;
        }
    }
    return ret;
}
#endif

uint32_t cmd_ddrmrr(uint32_t argc, char *argv[])
{
    uint32_t  mr_num=0,rank=0;

    if ((argc < 3)) {
        DBG("Opps, params error \nUsage: ddrmrr  mr=x rank=0-3\n");
        return 0;
    }
    if (0 == strncmp(argv[1], "mr=", 3)) {
        mr_num = strtoul(argv[1] + 3, NULL, 0);
    } 
    if (0 == strncmp(argv[2], "rank=", 5)) {
        rank  = strtoul(argv[2] + 5, NULL, 0);
    }

    if (rank > 3) {
        DBG("Opps, param error.\n\nUsage: ddrmrr  mr=x rank=0-3\n",mr_num);
        return 0;
    }
    unsigned int  time_out = 5000;
    // exception
    writel(1,APB_DDRCTRL_BASE+0x00020060); 
    //Poll MRSTAT.mr_wr_busy until it is 0
    //0 mr_wr_busy : The Soc might initiate a MR write operation only if this signal is low
    do{
        udelay(1);
    }while((readl(APB_DDRCTRL_BASE+UMCTL2_REGS_MRSTAT_OFF) & BM_MRSTAT_MR_WR_BUSY) && time_out--);
    if(!time_out){
        DBG("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }

    writel(((mr_num & 0xFF)<<8),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL1_OFF);
	writel((0<<31)+(0<<30)+((mr_num&0xF)<<12)+(rank<<4)+(1<<0),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL0_OFF); 
	writel((1<<31)+(0<<30)+((mr_num&0xF)<<12)+(rank<<4)+(1<<0),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL0_OFF); 
    //Poll MRSTAT.mr_wr_busy until it is 0
    //0 mr_wr_busy : The Soc might initiate a MR write operation only if this signal is low
    time_out = 5000;
    do{
        udelay(1);
    }while((!(readl(APB_DDRCTRL_BASE+UMCTL2_REGS_MRSTAT_OFF)&BM_MRSTAT_MR_WR_BUSY)) && time_out--);
    if(!time_out){
        DBG("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }
    // poll avild 
    time_out = 5000;
    do{
        udelay(1);
    }while(((readl(APB_DDRCTRL_BASE+0x00020060) & 0x8) != 0x8) && time_out--);
    if(!time_out){
        DBG("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }
    uint8_t mr_data = (readl(APB_DDRCTRL_BASE+0x00020064)>>8) & 0xFF;
    //mr_data = pin_swapswap(mr_data);
    DBG("The MR%d rank%d is:%02x.\n",mr_num,rank,mr_data);

    return 0;
}

SHELL_CMD("ddrmrr", cmd_ddrmrr, "Usage: ddrmrr  mr=x rank=0-3\n")

uint32_t cmd_ddrmrw(uint32_t argc, char *argv[])
{

    uint32_t  mr_num=0,rank=0,data=0;

    if ((argc < 4)) {
        DBG("Opps, params error \nUsage: ddrmrw  mr=x rank=0-3 vel=0-255\n");
        return 0;
    }
    if (0 == strncmp(argv[1], "mr=", 3)) {
        mr_num = strtoul(argv[1] + 3, NULL, 0);
    } 
    if (0 == strncmp(argv[2], "rank=", 5)) {
        rank  = strtoul(argv[2] + 5, NULL, 0);
    }
    if (0 == strncmp(argv[3], "vel=", 4)) {
        data  = strtoul(argv[3] + 4, NULL, 0);
    }

    if (rank > 3 || data&(~0xFF)) {
        DBG("Opps, param error.\n\nUsage: ddrmrr  mr=x rank=0-3 vel=0-255\n",mr_num);
        return 0;
    }

    //Poll MRSTAT.mr_wr_busy until it is 0
    //0 mr_wr_busy : The Soc might initiate a MR write operation only if this signal is low
    uint32_t  time_out = 5000;

    do{
        udelay(1);
    }while((readl(APB_DDRCTRL_BASE+UMCTL2_REGS_MRSTAT_OFF) & BM_MRSTAT_MR_WR_BUSY) && time_out--);
    if(!time_out){
        DBG("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }

    writel(((mr_num & 0xFF)<<8)+ (data & 0xFF),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL1_OFF);
    writel((0<<31)+(0<<30)+((mr_num & 0xF)<<12)+(rank<<4)+(0<<0),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL0_OFF); 
    writel((1<<31)+(0<<30)+((mr_num & 0xF)<<12)+(rank<<4)+(0<<0),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL0_OFF); 
    //Poll MRSTAT.mr_wr_busy until it is 0
    //0 mr_wr_busy : The Soc might initiate a MR write operation only if this signal is low
    time_out = 5000;
    do{
        udelay(1);
    }while((readl(APB_DDRCTRL_BASE+UMCTL2_REGS_MRSTAT_OFF) & BM_MRSTAT_MR_WR_BUSY) && time_out--);
    if(!time_out){
        DBG("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }
    DBG("MRW: mr_index=%d, mr_value=%02x\n", mr_num, data);
    return 0;
}

SHELL_CMD("ddrmrw", cmd_ddrmrw, "Usage: ddrmrw  mr=x rank=0-3 vel=0-255\n")
#endif
