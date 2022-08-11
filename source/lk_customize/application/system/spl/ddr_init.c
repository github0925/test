/*
 * Author: xx.xx
 * copyright:  xx.xx@semidrive.com
 *
 * 2018 semidrive Inc.
 */
#include "stdio.h"
#include "stdlib.h"
#include <reg.h>
#include <__regs_base.h>
#include "chip_res.h"
#include "ddr_init.h"
#include "lib/reg.h"
#include "rstgen_hal.h"
#include "scr_hal.h"

#define wr(a,v) writel(v,a)
#define rd(a,p) *p=readl(a)

static void reset_module(uint32_t resid)
{
    bool ret = true;
    void *handle = NULL;

    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        return ;
    }

    hal_rstgen_init(handle);
    hal_rstgen_module_reset(handle, resid);
    hal_rstgen_release_handle(handle);
}

static void setup_scr(uint64_t resid, uint32_t v)
{
    scr_handle_t handle;

    handle  = hal_scr_create_handle(resid);

    hal_scr_get(handle);
    hal_scr_set(handle, v);

    hal_scr_delete_handle(handle);
}

u32 ddr_init(void)
{
    unsigned int rdata;
    unsigned int addr;
    unsigned int wdata;
//  unsigned int addr_index;
    unsigned int umctl_base;


    umctl_base = _ioaddr(APB_DDRCTRL_BASE);
//-----------------------------------------------
//   DRAM  WRITE && READ
//-----------------------------------------------
//mr7 read
//
//
// addr = 0x40000000;
//
// for(ii=0;ii<30;ii=ii+1){
//   addr = 0x40000000;
//   rdata = readl(addr);
//   addr = addr + 4;
// }

    //addr = 0x33020000 + 0x80;
    addr = _ioaddr(APB_DDR_PERF_MON_BASE) + 0x80; //???
    rdata = readl(addr);
    //Lixin update, according to Lerry's mail at 2019-02-12
    //ddr phy apb clock gating should be enable before ddr initialization
    //set bit[17:16] and bit[0] to 1
    //wdata = 0x1 | rdata;
    //wdata = rdata | 0x30001;
    //Lixin update, according to Lerry's mail at 2019-02-15
    //set the ddr arbiter timeout value to 'd16, he will update the RTL to set the reset value to 16 at next release
    //wdata = 0x1001;
    wdata = 0x31001;
    writel(wdata, addr);

    // kunlun_r1p4 changes: the Pwron signal is moved to scr_saf | scr_sec, bit[0] of 0x80 is not used anymore
#ifdef SAF_CORE
    //set_scr_saf_ddr_ss_pwrokin_aon(0x1);
    setup_scr(scr_safETY__RW__ddr_ss_pwrokin_aon,0x1);
#else
    //set_scr_sec_ddr_ss_pwrokin_aon(0x1);
    setup_scr(SCR_SEC__RW__ddr_ss_pwrokin_aon,0x1);
#endif

    //umctl_base = 0x33000000;
    //phy_base   = 0x32000000;
    umctl_base = _ioaddr(APB_DDRCTRL_BASE);
    addr = umctl_base + 0x00000304;
    wdata = 0x00000001;
    writel(wdata, addr);
    addr = umctl_base + 0x00000030;
    wdata = 0x00000001;
    writel(wdata, addr);
    addr = umctl_base + 0x00000004;
    rdata = readl(addr);
    addr = umctl_base + 0x00000000;
    wdata = 0x83080020;
    writel(wdata, addr);
    addr = umctl_base + 0x00000010;
    wdata = 0x40003030;
    writel(wdata, addr);
    addr = umctl_base + 0x00000014;
    wdata = 0x000370d5;
    writel(wdata, addr);
    addr = umctl_base + 0x0000001c;
    wdata = 0xeb74135c;
    writel(wdata, addr);
    addr = umctl_base + 0x00000020;
    wdata = 0x00001404;
    writel(wdata, addr);
    addr = umctl_base + 0x00000024;
    wdata = 0x81e48043;
    writel(wdata, addr);
    addr = umctl_base + 0x00000028;
    wdata = 0x00000001;
    writel(wdata, addr);
    addr = umctl_base + 0x0000002c;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000030;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000034;
    wdata = 0x0040d404;
    writel(wdata, addr);
    addr = umctl_base + 0x00000038;
    wdata = 0x00ac0002;
    writel(wdata, addr);
//addr = umctl_base + 0x00000050; wdata = 0x00210000; writel(wdata,addr); //per_bank_refresh
    addr = umctl_base + 0x00000050;
    wdata = 0x00210004;
    writel(wdata, addr);
    addr = umctl_base + 0x00000054;
    wdata = 0x001e006b;
    writel(wdata, addr);
//addr = umctl_base + 0x00000060; wdata = 0x00000001; writel(wdata,addr);//disable autorefresh
    addr = umctl_base + 0x00000060;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000064;
    wdata = 0x00820197;
    writel(wdata, addr);
    addr = umctl_base + 0x00000068;
    wdata = 0x00610000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000070;
    wdata = 0x053f7f50;
    writel(wdata, addr);
    addr = umctl_base + 0x00000074;
    wdata = 0x000007b2;
    writel(wdata, addr);
    addr = umctl_base + 0x0000007c;
    wdata = 0x00000300;
    writel(wdata, addr);
    addr = umctl_base + 0x000000b8;
    wdata = 0x01000fc0;
    writel(wdata, addr);
    addr = umctl_base + 0x000000bc;
    wdata = 0x20019294;
    writel(wdata, addr);
    addr = umctl_base + 0x000000c0;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x000000c4;
    wdata = 0x00001000;
    writel(wdata, addr);
//addr = umctl_base + 0x000000d0; wdata = 0x00020002; writel(wdata,addr);
    addr = umctl_base + 0x000000d0;
    wdata = 0x00040004;
    writel(wdata, addr);
    addr = umctl_base + 0x000000d4;
    wdata = 0x00030002;
    writel(wdata, addr);
    addr = umctl_base + 0x000000d8;
    wdata = 0x0000ac05;
    writel(wdata, addr);
    addr = umctl_base + 0x000000dc;
    wdata = 0x0074003f;
    writel(wdata, addr);
    addr = umctl_base + 0x000000e0;
    wdata = 0x00330000;
    writel(wdata, addr);
    addr = umctl_base + 0x000000e4;
    wdata = 0x0005000c;
    writel(wdata, addr);
    addr = umctl_base + 0x000000e8;
    wdata = 0x0000004d;
    writel(wdata, addr);
    addr = umctl_base + 0x000000ec;
    wdata = 0x0000004d;
    writel(wdata, addr);
    addr = umctl_base + 0x000000f0;
    wdata = 0x00000020;
    writel(wdata, addr);
    addr = umctl_base + 0x000000f4;
    wdata = 0x000004df;
    writel(wdata, addr);
    addr = umctl_base + 0x00000100;
    wdata = 0x2121482d;
    writel(wdata, addr);
//addr = umctl_base + 0x00000104; wdata = 0x00090901; writel(wdata,addr);//tRC high7
    addr = umctl_base + 0x00000104;
    wdata = 0x00090941;
    writel(wdata, addr);
    addr = umctl_base + 0x00000108;
    wdata = 0x09121219;
    writel(wdata, addr);
    addr = umctl_base + 0x0000010c;
    wdata = 0x00f0f006;
    writel(wdata, addr);
    addr = umctl_base + 0x00000110;
    wdata = 0x14040914;
    writel(wdata, addr);
    addr = umctl_base + 0x00000114;
    wdata = 0x02061111;
    writel(wdata, addr);
    addr = umctl_base + 0x00000118;
    wdata = 0x0101000a;
    writel(wdata, addr);
    addr = umctl_base + 0x0000011c;
    wdata = 0x00000602;
    writel(wdata, addr);
    addr = umctl_base + 0x00000120;
    wdata = 0x01010101;
    writel(wdata, addr);
    addr = umctl_base + 0x00000124;
    wdata = 0x00000003;
    writel(wdata, addr);
    addr = umctl_base + 0x00000128;
    wdata = 0x000f020f;
    writel(wdata, addr);
    addr = umctl_base + 0x0000012c;
    wdata = 0x0101001b;
    writel(wdata, addr);
    addr = umctl_base + 0x00000130;
    wdata = 0x00020000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000134;
    wdata = 0x0e100002;
    writel(wdata, addr);
    addr = umctl_base + 0x00000138;
    wdata = 0x0000019f;
    writel(wdata, addr);
    addr = umctl_base + 0x0000013c;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000180;
    wdata = 0xd42f0021;
    writel(wdata, addr);
    addr = umctl_base + 0x00000184;
    wdata = 0x036bda6d;
    writel(wdata, addr);
    addr = umctl_base + 0x00000188;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000190;
    wdata = 0x039f820e;
    writel(wdata, addr);
    addr = umctl_base + 0x00000194;
    wdata = 0x00090202;
    writel(wdata, addr);
    addr = umctl_base + 0x00000198;
    wdata = 0x07a06000;
    writel(wdata, addr);
    addr = umctl_base + 0x0000019c;
    wdata = 0x00000050;
    writel(wdata, addr);
    addr = umctl_base + 0x000001a0;
    wdata = 0xe0400018;
    writel(wdata, addr);
    addr = umctl_base + 0x000001a4;
    wdata = 0x007c009c;
    writel(wdata, addr);
    addr = umctl_base + 0x000001a8;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x000001b0;
    wdata = 0x00000041;
    writel(wdata, addr);
    addr = umctl_base + 0x000001b4;
    wdata = 0x00001f0e;
    writel(wdata, addr);
    addr = umctl_base + 0x000001b8;
    wdata = 0x00000008;
    writel(wdata, addr);
    addr = umctl_base + 0x000001c0;
    wdata = 0x00000001;
    writel(wdata, addr);
    addr = umctl_base + 0x000001c4;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000200;
    wdata = 0x00000018;
    writel(wdata, addr);
//addr = umctl_base + 0x00000204; wdata = 0x00080808; writel(wdata,addr);//bankmap
    addr = umctl_base + 0x00000204;
    wdata = 0x00050505;
    writel(wdata, addr);
    addr = umctl_base + 0x00000208;
    wdata = 0x00000000;
    writel(wdata, addr);
//addr = umctl_base + 0x0000020c; wdata = 0x00080808; writel(wdata,addr); //col map
    addr = umctl_base + 0x0000020c;
    wdata = 0x03030300;
    writel(wdata, addr);
    addr = umctl_base + 0x00000210;
    wdata = 0x00001f1f;
    writel(wdata, addr);
    addr = umctl_base + 0x00000214;
    wdata = 0x070f0707;
    writel(wdata, addr);
    addr = umctl_base + 0x00000218;
    wdata = 0x07070707;
    writel(wdata, addr);
    addr = umctl_base + 0x0000021c;
    wdata = 0x00000f07;
    writel(wdata, addr);
    addr = umctl_base + 0x00000220;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000224;
    wdata = 0x07070707;
    writel(wdata, addr);
    addr = umctl_base + 0x00000228;
    wdata = 0x07070707;
    writel(wdata, addr);
    addr = umctl_base + 0x0000022c;
    wdata = 0x00000007;
    writel(wdata, addr);
    addr = umctl_base + 0x00000240;
    wdata = 0x060a0c7c;
    writel(wdata, addr);
    addr = umctl_base + 0x00000244;
    wdata = 0x00000000;
    writel(wdata, addr);
//addr = umctl_base + 0x00000250; wdata = 0x008b1f00; writel(wdata,addr);//hp_lp
    addr = umctl_base + 0x00000250;
    wdata = 0x008b9f00;
    writel(wdata, addr);
    addr = umctl_base + 0x00000254;
    wdata = 0x00000000;
    writel(wdata, addr);
//addr = umctl_base + 0x0000025c; wdata = 0x0f000001; writel(wdata,addr);//hpr_max_starve
    addr = umctl_base + 0x0000025c;
    wdata = 0x0f00003f;
    writel(wdata, addr);
//addr = umctl_base + 0x00000264; wdata = 0x0f00007f; writel(wdata,addr);//lpr_max_starve
    addr = umctl_base + 0x00000264;
    wdata = 0x0f0003ff;
    writel(wdata, addr);
//addr = umctl_base + 0x0000026c; wdata = 0x0f00007f; writel(wdata,addr);//w_max_starve
    addr = umctl_base + 0x0000026c;
    wdata = 0x0f0003ff;
    writel(wdata, addr);
    addr = umctl_base + 0x00000300;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000304;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x0000030c;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000320;
    wdata = 0x00000001;
    writel(wdata, addr);
    addr = umctl_base + 0x00000328;
    wdata = 0x00000000;
    writel(wdata, addr);
//addr = umctl_base + 0x00000330; wdata = 0x00901022; writel(wdata,addr);
    addr = umctl_base + 0x00000330;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000334;
    wdata = 0x00000844;
    writel(wdata, addr);
    addr = umctl_base + 0x0000036c;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000374;
    wdata = 0x000000e5;
    writel(wdata, addr);
    addr = umctl_base + 0x0000037c;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000384;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x000003c0;
    wdata = 0x00000003;
    writel(wdata, addr);
    addr = umctl_base + 0x000003e0;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x000003e8;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000490;
    wdata = 0x00000001;
    writel(wdata, addr);
    addr = umctl_base + 0x00000494;
    wdata = 0x01110e00;
    writel(wdata, addr); //qos
    addr = umctl_base + 0x00002020;
    wdata = 0x00001404;
    writel(wdata, addr);
    addr = umctl_base + 0x00002024;
    wdata = 0x81e48043;
    writel(wdata, addr);
    addr = umctl_base + 0x00002034;
    wdata = 0x0040d404;
    writel(wdata, addr);
    addr = umctl_base + 0x00002050;
    wdata = 0x00210000;
    writel(wdata, addr);
    addr = umctl_base + 0x00002064;
    wdata = 0x00820197;
    writel(wdata, addr);
    addr = umctl_base + 0x00002068;
    wdata = 0x00610000;
    writel(wdata, addr);
    addr = umctl_base + 0x000020dc;
    wdata = 0x0074003f;
    writel(wdata, addr);
    addr = umctl_base + 0x000020e0;
    wdata = 0x00330000;
    writel(wdata, addr);
    addr = umctl_base + 0x000020e8;
    wdata = 0x0000004d;
    writel(wdata, addr);
    addr = umctl_base + 0x000020ec;
    wdata = 0x0000004d;
    writel(wdata, addr);
    addr = umctl_base + 0x000020f4;
    wdata = 0x000004df;
    writel(wdata, addr);
    addr = umctl_base + 0x00002100;
    wdata = 0x2121482d;
    writel(wdata, addr);
//addr = umctl_base + 0x00002104; wdata = 0x00090901; writel(wdata,addr);//tRC high7
    addr = umctl_base + 0x00002104;
    wdata = 0x00090941;
    writel(wdata, addr);
    addr = umctl_base + 0x00002108;
    wdata = 0x09121219;
    writel(wdata, addr);
    addr = umctl_base + 0x0000210c;
    wdata = 0x00f0f006;
    writel(wdata, addr);
    addr = umctl_base + 0x00002110;
    wdata = 0x14040914;
    writel(wdata, addr);
    addr = umctl_base + 0x00002114;
    wdata = 0x02061111;
    writel(wdata, addr);
    addr = umctl_base + 0x00002118;
    wdata = 0x0101000a;
    writel(wdata, addr);
    addr = umctl_base + 0x0000211c;
    wdata = 0x00000602;
    writel(wdata, addr);
    addr = umctl_base + 0x00002120;
    wdata = 0x01010101;
    writel(wdata, addr);
    addr = umctl_base + 0x00002124;
    wdata = 0x00000003;
    writel(wdata, addr);
    addr = umctl_base + 0x00002128;
    wdata = 0x000f020f;
    writel(wdata, addr);
    addr = umctl_base + 0x0000212c;
    wdata = 0x0101001b;
    writel(wdata, addr);
    addr = umctl_base + 0x00002130;
    wdata = 0x00020000;
    writel(wdata, addr);
    addr = umctl_base + 0x00002134;
    wdata = 0x0e100002;
    writel(wdata, addr);
    addr = umctl_base + 0x00002138;
    wdata = 0x0000019f;
    writel(wdata, addr);
    addr = umctl_base + 0x0000213c;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00002180;
    wdata = 0xd42f0021;
    writel(wdata, addr);
    addr = umctl_base + 0x00002190;
    wdata = 0x039f820e;
    writel(wdata, addr);
    addr = umctl_base + 0x00002194;
    wdata = 0x00090202;
    writel(wdata, addr);
    addr = umctl_base + 0x000021b4;
    wdata = 0x00001f0e;
    writel(wdata, addr);
    addr = umctl_base + 0x000021b8;
    wdata = 0x00000008;
    writel(wdata, addr);
    addr = umctl_base + 0x00002240;
    wdata = 0x060a0c7c;
    writel(wdata, addr);
    addr = umctl_base + 0x00003020;
    wdata = 0x00001404;
    writel(wdata, addr);
    addr = umctl_base + 0x00003024;
    wdata = 0x81e48043;
    writel(wdata, addr);
    addr = umctl_base + 0x00003034;
    wdata = 0x0040d404;
    writel(wdata, addr);
    addr = umctl_base + 0x00003050;
    wdata = 0x00210000;
    writel(wdata, addr);
    addr = umctl_base + 0x00003064;
    wdata = 0x00828197;
    writel(wdata, addr);
    addr = umctl_base + 0x00003068;
    wdata = 0x00610000;
    writel(wdata, addr);
    addr = umctl_base + 0x000030dc;
    wdata = 0x0074003f;
    writel(wdata, addr);
    addr = umctl_base + 0x000030e0;
    wdata = 0x00330000;
    writel(wdata, addr);
    addr = umctl_base + 0x000030e8;
    wdata = 0x0000004d;
    writel(wdata, addr);
    addr = umctl_base + 0x000030ec;
    wdata = 0x0000004d;
    writel(wdata, addr);
    addr = umctl_base + 0x000030f4;
    wdata = 0x000004df;
    writel(wdata, addr);
    addr = umctl_base + 0x00003100;
    wdata = 0x2121482d;
    writel(wdata, addr);
//addr = umctl_base + 0x00003104; wdata = 0x00090901; writel(wdata,addr);//tRC high7
    addr = umctl_base + 0x00003104;
    wdata = 0x00090941;
    writel(wdata, addr);
    addr = umctl_base + 0x00003108;
    wdata = 0x09121219;
    writel(wdata, addr);
    addr = umctl_base + 0x0000310c;
    wdata = 0x00f0f006;
    writel(wdata, addr);
    addr = umctl_base + 0x00003110;
    wdata = 0x14040914;
    writel(wdata, addr);
    addr = umctl_base + 0x00003114;
    wdata = 0x02061111;
    writel(wdata, addr);
    addr = umctl_base + 0x00003118;
    wdata = 0x0101000a;
    writel(wdata, addr);
    addr = umctl_base + 0x0000311c;
    wdata = 0x00000602;
    writel(wdata, addr);
    addr = umctl_base + 0x00003120;
    wdata = 0x01010101;
    writel(wdata, addr);
    addr = umctl_base + 0x00003124;
    wdata = 0x00000003;
    writel(wdata, addr);
    addr = umctl_base + 0x00003128;
    wdata = 0x000f020f;
    writel(wdata, addr);
    addr = umctl_base + 0x0000312c;
    wdata = 0x0101001b;
    writel(wdata, addr);
    addr = umctl_base + 0x00003130;
    wdata = 0x00020000;
    writel(wdata, addr);
    addr = umctl_base + 0x00003134;
    wdata = 0x0e100002;
    writel(wdata, addr);
    addr = umctl_base + 0x00003138;
    wdata = 0x0000019f;
    writel(wdata, addr);
    addr = umctl_base + 0x0000313c;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00003180;
    wdata = 0xd42f0021;
    writel(wdata, addr);
    addr = umctl_base + 0x00003190;
    wdata = 0x039f820e;
    writel(wdata, addr);
    addr = umctl_base + 0x00003194;
    wdata = 0x00090202;
    writel(wdata, addr);
    addr = umctl_base + 0x000031b4;
    wdata = 0x00001f0e;
    writel(wdata, addr);
    addr = umctl_base + 0x000031b8;
    wdata = 0x00000008;
    writel(wdata, addr);
    addr = umctl_base + 0x00003240;
    wdata = 0x060a0c7c;
    writel(wdata, addr);
    addr = umctl_base + 0x00004020;
    wdata = 0x00001404;
    writel(wdata, addr);
    addr = umctl_base + 0x00004024;
    wdata = 0x81e48043;
    writel(wdata, addr);
    addr = umctl_base + 0x00004034;
    wdata = 0x0040d404;
    writel(wdata, addr);
    addr = umctl_base + 0x00004050;
    wdata = 0x00210000;
    writel(wdata, addr);
    addr = umctl_base + 0x00004064;
    wdata = 0x00828197;
    writel(wdata, addr);
    addr = umctl_base + 0x00004068;
    wdata = 0x00610000;
    writel(wdata, addr);
    addr = umctl_base + 0x000040dc;
    wdata = 0x0074003f;
    writel(wdata, addr);
    addr = umctl_base + 0x000040e0;
    wdata = 0x00330000;
    writel(wdata, addr);
    addr = umctl_base + 0x000040e8;
    wdata = 0x0000004d;
    writel(wdata, addr);
    addr = umctl_base + 0x000040ec;
    wdata = 0x0000004d;
    writel(wdata, addr);
    addr = umctl_base + 0x000040f4;
    wdata = 0x000004df;
    writel(wdata, addr);
    addr = umctl_base + 0x00004100;
    wdata = 0x2121482d;
    writel(wdata, addr);
//addr = umctl_base + 0x00004104; wdata = 0x00090901; writel(wdata,addr);//tRC high7
    addr = umctl_base + 0x00004104;
    wdata = 0x00090941;
    writel(wdata, addr);
    addr = umctl_base + 0x00004108;
    wdata = 0x09121219;
    writel(wdata, addr);
    addr = umctl_base + 0x0000410c;
    wdata = 0x00f0f006;
    writel(wdata, addr);
    addr = umctl_base + 0x00004110;
    wdata = 0x14040914;
    writel(wdata, addr);
    addr = umctl_base + 0x00004114;
    wdata = 0x02061111;
    writel(wdata, addr);
    addr = umctl_base + 0x00004118;
    wdata = 0x0101000a;
    writel(wdata, addr);
    addr = umctl_base + 0x0000411c;
    wdata = 0x00000602;
    writel(wdata, addr);
    addr = umctl_base + 0x00004120;
    wdata = 0x01010101;
    writel(wdata, addr);
    addr = umctl_base + 0x00004124;
    wdata = 0x00000003;
    writel(wdata, addr);
    addr = umctl_base + 0x00004128;
    wdata = 0x000f020f;
    writel(wdata, addr);
    addr = umctl_base + 0x0000412c;
    wdata = 0x0101001b;
    writel(wdata, addr);
    addr = umctl_base + 0x00004130;
    wdata = 0x00020000;
    writel(wdata, addr);
    addr = umctl_base + 0x00004134;
    wdata = 0x0e100002;
    writel(wdata, addr);
    addr = umctl_base + 0x00004138;
    wdata = 0x0000019f;
    writel(wdata, addr);
    addr = umctl_base + 0x0000413c;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00004180;
    wdata = 0xd42f0021;
    writel(wdata, addr);
    addr = umctl_base + 0x00004190;
    wdata = 0x039f820e;
    writel(wdata, addr);
    addr = umctl_base + 0x00004194;
    wdata = 0x00090202;
    writel(wdata, addr);
    addr = umctl_base + 0x000041b4;
    wdata = 0x00001f0e;
    writel(wdata, addr);
    addr = umctl_base + 0x000041b8;
    wdata = 0x00000008;
    writel(wdata, addr);
    addr = umctl_base + 0x00004240;
    wdata = 0x060a0c7c;
    writel(wdata, addr);
    addr = umctl_base + 0x00000060;
    rdata = readl(addr);
    addr = umctl_base + 0x00000400;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000330;
    wdata = 0x00901022;
    writel(wdata, addr);
    addr = umctl_base + 0x00000404;
    wdata = 0x0000000f;
    writel(wdata, addr);
    addr = umctl_base + 0x00000404;
    wdata = 0x0000100f;
    writel(wdata, addr);
    addr = umctl_base + 0x00000404;
    wdata = 0x0000100f;
    writel(wdata, addr);
    addr = umctl_base + 0x00000404;
    wdata = 0x0001100f;
    writel(wdata, addr);
    addr = umctl_base + 0x00000408;
    wdata = 0x0000400f;
    writel(wdata, addr);
    addr = umctl_base + 0x00000408;
    wdata = 0x0000500f;
    writel(wdata, addr);
    addr = umctl_base + 0x00000408;
    wdata = 0x0000500f;
    writel(wdata, addr);
    addr = umctl_base + 0x00000408;
    wdata = 0x0000100f;
    writel(wdata, addr);
    addr = umctl_base + 0x00000304;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000030;
    rdata = readl(addr);
    addr = umctl_base + 0x00000030;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000030;
    rdata = readl(addr);
    addr = umctl_base + 0x00000030;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000320;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x000001b0;
    wdata = 0x00000040;
    writel(wdata, addr);
    addr = umctl_base + 0x000001b0;
    wdata = 0x00000040;
    writel(wdata, addr);
    addr = umctl_base + 0x000001b0;
    wdata = 0x00001040;
    writel(wdata, addr);
    addr = umctl_base + 0x000000d0;
    rdata = readl(addr);
    addr = umctl_base + 0x000001c0;
    rdata = readl(addr);
    addr = umctl_base + 0x00000000;
    rdata = readl(addr);
    addr = umctl_base + 0x00000000;
    rdata = readl(addr);
    addr = umctl_base + 0x000000dc;
    rdata = readl(addr);
    addr = umctl_base + 0x000000dc;
    rdata = readl(addr);
    addr = umctl_base + 0x000000e0;
    rdata = readl(addr);
    addr = umctl_base + 0x000000e8;
    rdata = readl(addr);
    addr = umctl_base + 0x000000e8;
    rdata = readl(addr);
    addr = umctl_base + 0x000000e0;
    rdata = readl(addr);
    addr = umctl_base + 0x000000ec;
    rdata = readl(addr);
    addr = umctl_base + 0x000000ec;
    rdata = readl(addr);
    addr = umctl_base + 0x000000d0;
    rdata = readl(addr);
    addr = umctl_base + 0x000001c0;
    rdata = readl(addr);
    addr = umctl_base + 0x00000000;
    rdata = readl(addr);
    addr = umctl_base + 0x00000000;
    rdata = readl(addr);
    addr = umctl_base + 0x000000dc;
    rdata = readl(addr);
    addr = umctl_base + 0x000000dc;
    rdata = readl(addr);
    addr = umctl_base + 0x000000e0;
    rdata = readl(addr);
    addr = umctl_base + 0x000000e8;
    rdata = readl(addr);
    addr = umctl_base + 0x000000e8;
    rdata = readl(addr);
    addr = umctl_base + 0x000000e0;
    rdata = readl(addr);
    addr = umctl_base + 0x000000ec;
    rdata = readl(addr);
    addr = umctl_base + 0x000000ec;
    rdata = readl(addr);
    addr = umctl_base + 0x000000d0;
    rdata = readl(addr);

//rstgen_sec_module_rst(RSTGEN_SEC_MODULE_RST_B_DDR_SS_SW_AXI_RSTN_INDEX,0x1); //ddr_axi_rstn = rstgen_sec.module_rst_b[31]
    reset_module(RES_MODULE_RST_SEC_DDR_SW_2);

    rdata = readl(addr);
    rdata = readl(addr);
    rdata = readl(addr);

//rstgen_sec_module_rst(RSTGEN_SEC_MODULE_RST_B_DDR_SS_SW_DDR_CORE_RSTN_INDEX,0x1); //ddr_core_rstn = rstgen_sec.module_rst_b[32]
    reset_module(RES_MODULE_RST_SEC_DDR_SW_3);
    rdata = readl(addr);
    rdata = readl(addr);
    rdata = readl(addr);
    rdata = readl(addr);
    rdata = readl(addr);
    rdata = readl(addr);
    rdata = readl(addr);
    rdata = readl(addr);

#ifndef EMU
    addr = phy_base + 0x1005f * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxSlewRate_b0_p0
    addr = phy_base + 0x1015f * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxSlewRate_b1_p0
    addr = phy_base + 0x1105f * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxSlewRate_b0_p0
    addr = phy_base + 0x1115f * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxSlewRate_b1_p0
    addr = phy_base + 0x1205f * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxSlewRate_b0_p0
    addr = phy_base + 0x1215f * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxSlewRate_b1_p0
    addr = phy_base + 0x1305f * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxSlewRate_b0_p0
    addr = phy_base + 0x1315f * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxSlewRate_b1_p0
    addr = phy_base + 0x55 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB0_ATxSlewRate
    addr = phy_base + 0x1055 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB1_ATxSlewRate
    addr = phy_base + 0x2055 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB2_ATxSlewRate
    addr = phy_base + 0x3055 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB3_ATxSlewRate
    addr = phy_base + 0x4055 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB4_ATxSlewRate
    addr = phy_base + 0x5055 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB5_ATxSlewRate
    addr = phy_base + 0x6055 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB6_ATxSlewRate
    addr = phy_base + 0x7055 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB7_ATxSlewRate
    addr = phy_base + 0x8055 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB8_ATxSlewRate
    addr = phy_base + 0x9055 * 4;
    wdata = 0x1ff;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB9_ATxSlewRate
    addr = phy_base + 0x200c5 * 4;
    wdata = 0x18;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_PllCtrl2_p0
    addr = phy_base + 0x2002e * 4;
    wdata = 0x2;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_ARdPtrInitVal_p0
    addr = phy_base + 0x90204 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BGPR4_p0
    addr = phy_base + 0x20024 * 4;
    wdata = 0xe3;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DqsPreambleControl_p0
    addr = phy_base + 0x2003a * 4;
    wdata = 0x2;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DbyteDllModeCntrl
    addr = phy_base + 0x20056 * 4;
    wdata = 0x3;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_ProcOdtTimeCtl_p0
    addr = phy_base + 0x1004d * 4;
    wdata = 0x600;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b0_p0
    addr = phy_base + 0x1014d * 4;
    wdata = 0x600;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b1_p0
    addr = phy_base + 0x1104d * 4;
    wdata = 0x600;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b0_p0
    addr = phy_base + 0x1114d * 4;
    wdata = 0x600;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b1_p0
    addr = phy_base + 0x1204d * 4;
    wdata = 0x600;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b0_p0
    addr = phy_base + 0x1214d * 4;
    wdata = 0x600;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b1_p0
    addr = phy_base + 0x1304d * 4;
    wdata = 0x600;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b0_p0
    addr = phy_base + 0x1314d * 4;
    wdata = 0x600;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b1_p0
    addr = phy_base + 0x10049 * 4;
    wdata = 0x61f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b0_p0
    addr = phy_base + 0x10149 * 4;
    wdata = 0x61f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b1_p0
    addr = phy_base + 0x11049 * 4;
    wdata = 0x61f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b0_p0
    addr = phy_base + 0x11149 * 4;
    wdata = 0x61f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b1_p0
    addr = phy_base + 0x12049 * 4;
    wdata = 0x61f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b0_p0
    addr = phy_base + 0x12149 * 4;
    wdata = 0x61f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b1_p0
    addr = phy_base + 0x13049 * 4;
    wdata = 0x61f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b0_p0
    addr = phy_base + 0x13149 * 4;
    wdata = 0x61f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b1_p0
    addr = phy_base + 0x43 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB0_ATxImpedance
    addr = phy_base + 0x1043 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB1_ATxImpedance
    addr = phy_base + 0x2043 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB2_ATxImpedance
    addr = phy_base + 0x3043 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB3_ATxImpedance
    addr = phy_base + 0x4043 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB4_ATxImpedance
    addr = phy_base + 0x5043 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB5_ATxImpedance
    addr = phy_base + 0x6043 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB6_ATxImpedance
    addr = phy_base + 0x7043 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB7_ATxImpedance
    addr = phy_base + 0x8043 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB8_ATxImpedance
    addr = phy_base + 0x9043 * 4;
    wdata = 0x7f;
    writel(wdata, addr); // DWC_DDRPHYA_ANIB9_ATxImpedance
    addr = phy_base + 0x20018 * 4;
    wdata = 0x3;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiMode
    addr = phy_base + 0x20075 * 4;
    wdata = 0x4;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiCAMode
    addr = phy_base + 0x20050 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_CalDrvStr0
    addr = phy_base + 0x20008 * 4;
    wdata = 0x42b;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_CalUclkInfo_p0
    addr = phy_base + 0x20088 * 4;
    wdata = 0x9;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_CalRate
    addr = phy_base + 0x200b2 * 4;
    wdata = 0x104;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_VrefInGlobal_p0
    addr = phy_base + 0x10043 * 4;
    wdata = 0x5a1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b0_p0
    addr = phy_base + 0x10143 * 4;
    wdata = 0x5a1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b1_p0
    addr = phy_base + 0x11043 * 4;
    wdata = 0x5a1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b0_p0
    addr = phy_base + 0x11143 * 4;
    wdata = 0x5a1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b1_p0
    addr = phy_base + 0x12043 * 4;
    wdata = 0x5a1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b0_p0
    addr = phy_base + 0x12143 * 4;
    wdata = 0x5a1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b1_p0
    addr = phy_base + 0x13043 * 4;
    wdata = 0x5a1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b0_p0
    addr = phy_base + 0x13143 * 4;
    wdata = 0x5a1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b1_p0
    addr = phy_base + 0x200fa * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiFreqRatio_p0
    addr = phy_base + 0x20019 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_TristateModeCA_p0
    addr = phy_base + 0x200f0 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiFreqXlat0
    addr = phy_base + 0x200f1 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiFreqXlat1
    addr = phy_base + 0x200f2 * 4;
    wdata = 0x4444;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiFreqXlat2
    addr = phy_base + 0x200f3 * 4;
    wdata = 0x8888;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiFreqXlat3
    addr = phy_base + 0x200f4 * 4;
    wdata = 0x5555;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiFreqXlat4
    addr = phy_base + 0x200f5 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiFreqXlat5
    addr = phy_base + 0x200f6 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiFreqXlat6
    addr = phy_base + 0x200f7 * 4;
    wdata = 0xf000;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DfiFreqXlat7
    addr = phy_base + 0x1004a * 4;
    wdata = 0x500;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl1
    addr = phy_base + 0x1104a * 4;
    wdata = 0x500;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl1
    addr = phy_base + 0x1204a * 4;
    wdata = 0x500;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl1
    addr = phy_base + 0x1304a * 4;
    wdata = 0x500;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl1
    addr = phy_base + 0x20025 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_MasterX4Config
    addr = phy_base + 0x2002d * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DMIPinPresent_p0
    addr = phy_base + 0x10020 * 4;
    wdata = 0x6;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_DFIMRL_p0
    addr = phy_base + 0x11020 * 4;
    wdata = 0x6;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_DFIMRL_p0
    addr = phy_base + 0x12020 * 4;
    wdata = 0x6;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_DFIMRL_p0
    addr = phy_base + 0x13020 * 4;
    wdata = 0x6;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_DFIMRL_p0
    addr = phy_base + 0x20020 * 4;
    wdata = 0x6;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_HwtMRL_p0
    addr = phy_base + 0x100d0 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg0_u0_p0
    addr = phy_base + 0x100d1 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg1_u0_p0
    addr = phy_base + 0x101d0 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg0_u1_p0
    addr = phy_base + 0x101d1 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg1_u1_p0
    addr = phy_base + 0x110d0 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg0_u0_p0
    addr = phy_base + 0x110d1 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg1_u0_p0
    addr = phy_base + 0x111d0 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg0_u1_p0
    addr = phy_base + 0x111d1 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg1_u1_p0
    addr = phy_base + 0x120d0 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg0_u0_p0
    addr = phy_base + 0x120d1 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg1_u0_p0
    addr = phy_base + 0x121d0 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg0_u1_p0
    addr = phy_base + 0x121d1 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg1_u1_p0
    addr = phy_base + 0x130d0 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg0_u0_p0
    addr = phy_base + 0x130d1 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg1_u0_p0
    addr = phy_base + 0x131d0 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg0_u1_p0
    addr = phy_base + 0x131d1 * 4;
    wdata = 0x100;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg1_u1_p0
    addr = phy_base + 0x100c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r0_p0
    addr = phy_base + 0x100c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r0_p0
    addr = phy_base + 0x101c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r1_p0
    addr = phy_base + 0x101c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r1_p0
    addr = phy_base + 0x102c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r2_p0
    addr = phy_base + 0x102c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r2_p0
    addr = phy_base + 0x103c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r3_p0
    addr = phy_base + 0x103c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r3_p0
    addr = phy_base + 0x104c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r4_p0
    addr = phy_base + 0x104c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r4_p0
    addr = phy_base + 0x105c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r5_p0
    addr = phy_base + 0x105c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r5_p0
    addr = phy_base + 0x106c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r6_p0
    addr = phy_base + 0x106c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r6_p0
    addr = phy_base + 0x107c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r7_p0
    addr = phy_base + 0x107c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r7_p0
    addr = phy_base + 0x108c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r8_p0
    addr = phy_base + 0x108c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r8_p0
    addr = phy_base + 0x110c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r0_p0
    addr = phy_base + 0x110c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r0_p0
    addr = phy_base + 0x111c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r1_p0
    addr = phy_base + 0x111c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r1_p0
    addr = phy_base + 0x112c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r2_p0
    addr = phy_base + 0x112c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r2_p0
    addr = phy_base + 0x113c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r3_p0
    addr = phy_base + 0x113c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r3_p0
    addr = phy_base + 0x114c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r4_p0
    addr = phy_base + 0x114c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r4_p0
    addr = phy_base + 0x115c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r5_p0
    addr = phy_base + 0x115c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r5_p0
    addr = phy_base + 0x116c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r6_p0
    addr = phy_base + 0x116c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r6_p0
    addr = phy_base + 0x117c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r7_p0
    addr = phy_base + 0x117c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r7_p0
    addr = phy_base + 0x118c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r8_p0
    addr = phy_base + 0x118c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r8_p0
    addr = phy_base + 0x120c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r0_p0
    addr = phy_base + 0x120c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r0_p0
    addr = phy_base + 0x121c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r1_p0
    addr = phy_base + 0x121c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r1_p0
    addr = phy_base + 0x122c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r2_p0
    addr = phy_base + 0x122c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r2_p0
    addr = phy_base + 0x123c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r3_p0
    addr = phy_base + 0x123c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r3_p0
    addr = phy_base + 0x124c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r4_p0
    addr = phy_base + 0x124c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r4_p0
    addr = phy_base + 0x125c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r5_p0
    addr = phy_base + 0x125c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r5_p0
    addr = phy_base + 0x126c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r6_p0
    addr = phy_base + 0x126c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r6_p0
    addr = phy_base + 0x127c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r7_p0
    addr = phy_base + 0x127c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r7_p0
    addr = phy_base + 0x128c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r8_p0
    addr = phy_base + 0x128c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r8_p0
    addr = phy_base + 0x130c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r0_p0
    addr = phy_base + 0x130c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r0_p0
    addr = phy_base + 0x131c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r1_p0
    addr = phy_base + 0x131c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r1_p0
    addr = phy_base + 0x132c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r2_p0
    addr = phy_base + 0x132c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r2_p0
    addr = phy_base + 0x133c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r3_p0
    addr = phy_base + 0x133c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r3_p0
    addr = phy_base + 0x134c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r4_p0
    addr = phy_base + 0x134c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r4_p0
    addr = phy_base + 0x135c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r5_p0
    addr = phy_base + 0x135c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r5_p0
    addr = phy_base + 0x136c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r6_p0
    addr = phy_base + 0x136c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r6_p0
    addr = phy_base + 0x137c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r7_p0
    addr = phy_base + 0x137c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r7_p0
    addr = phy_base + 0x138c0 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r8_p0
    addr = phy_base + 0x138c1 * 4;
    wdata = 0x4c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r8_p0
    addr = phy_base + 0x10080 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg0_u0_p0
    addr = phy_base + 0x10081 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg1_u0_p0
    addr = phy_base + 0x10180 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg0_u1_p0
    addr = phy_base + 0x10181 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg1_u1_p0
    addr = phy_base + 0x11080 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg0_u0_p0
    addr = phy_base + 0x11081 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg1_u0_p0
    addr = phy_base + 0x11180 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg0_u1_p0
    addr = phy_base + 0x11181 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg1_u1_p0
    addr = phy_base + 0x12080 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg0_u0_p0
    addr = phy_base + 0x12081 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg1_u0_p0
    addr = phy_base + 0x12180 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg0_u1_p0
    addr = phy_base + 0x12181 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg1_u1_p0
    addr = phy_base + 0x13080 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg0_u0_p0
    addr = phy_base + 0x13081 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg1_u0_p0
    addr = phy_base + 0x13180 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg0_u1_p0
    addr = phy_base + 0x13181 * 4;
    wdata = 0x2c4;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg1_u1_p0
    addr = phy_base + 0x90201 * 4;
    wdata = 0x2200;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BGPR1_p0
    addr = phy_base + 0x90202 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BGPR2_p0
    addr = phy_base + 0x90203 * 4;
    wdata = 0x2e00;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BGPR3_p0
    addr = phy_base + 0x20072 * 4;
    wdata = 0x3;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_HwtLpCsEnA
    addr = phy_base + 0x20073 * 4;
    wdata = 0x3;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_HwtLpCsEnB
    addr = phy_base + 0x100ae * 4;
    wdata = 0x1c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_PptDqsCntInvTrnTg0_p0
    addr = phy_base + 0x110ae * 4;
    wdata = 0x1c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_PptDqsCntInvTrnTg0_p0
    addr = phy_base + 0x120ae * 4;
    wdata = 0x1c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_PptDqsCntInvTrnTg0_p0
    addr = phy_base + 0x130ae * 4;
    wdata = 0x1c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_PptDqsCntInvTrnTg0_p0
    addr = phy_base + 0x100af * 4;
    wdata = 0x1c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_PptDqsCntInvTrnTg1_p0
    addr = phy_base + 0x110af * 4;
    wdata = 0x1c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_PptDqsCntInvTrnTg1_p0
    addr = phy_base + 0x120af * 4;
    wdata = 0x1c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_PptDqsCntInvTrnTg1_p0
    addr = phy_base + 0x130af * 4;
    wdata = 0x1c;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_PptDqsCntInvTrnTg1_p0
    addr = phy_base + 0x100aa * 4;
    wdata = 0x703;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_PptCtlStatic
    addr = phy_base + 0x110aa * 4;
    wdata = 0x70f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_PptCtlStatic
    addr = phy_base + 0x120aa * 4;
    wdata = 0x703;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_PptCtlStatic
    addr = phy_base + 0x130aa * 4;
    wdata = 0x70f;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_PptCtlStatic
    addr = phy_base + 0x20077 * 4;
    wdata = 0x34;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_HwtCAMode
    addr = phy_base + 0x2007c * 4;
    wdata = 0x54;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DllGainCtl_p0
    addr = phy_base + 0x2007d * 4;
    wdata = 0x2f2;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DllLockParam_p0
    addr = phy_base + 0x400c0 * 4;
    wdata = 0x10f;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCtrl23
    addr = phy_base + 0x200cb * 4;
    wdata = 0x61f0;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_PllCtrl3
    addr = phy_base + 0x90028 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PhyInLP3
    addr = phy_base + 0xd0000 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
    addr = phy_base + 0x90000 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s0
    addr = phy_base + 0x90001 * 4;
    wdata = 0x400;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s1
    addr = phy_base + 0x90002 * 4;
    wdata = 0x10e;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s2
    addr = phy_base + 0x90003 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s0
    addr = phy_base + 0x90004 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s1
    addr = phy_base + 0x90005 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s2
    addr = phy_base + 0x90029 * 4;
    wdata = 0xb;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b0s0
    addr = phy_base + 0x9002a * 4;
    wdata = 0x480;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b0s1
    addr = phy_base + 0x9002b * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b0s2
    addr = phy_base + 0x9002c * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b1s0
    addr = phy_base + 0x9002d * 4;
    wdata = 0x448;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b1s1
    addr = phy_base + 0x9002e * 4;
    wdata = 0x139;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b1s2
    addr = phy_base + 0x9002f * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b2s0
    addr = phy_base + 0x90030 * 4;
    wdata = 0x478;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b2s1
    addr = phy_base + 0x90031 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b2s2
    addr = phy_base + 0x90032 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b3s0
    addr = phy_base + 0x90033 * 4;
    wdata = 0xe8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b3s1
    addr = phy_base + 0x90034 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b3s2
    addr = phy_base + 0x90035 * 4;
    wdata = 0x2;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b4s0
    addr = phy_base + 0x90036 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b4s1
    addr = phy_base + 0x90037 * 4;
    wdata = 0x139;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b4s2
    addr = phy_base + 0x90038 * 4;
    wdata = 0xb;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b5s0
    addr = phy_base + 0x90039 * 4;
    wdata = 0x7c0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b5s1
    addr = phy_base + 0x9003a * 4;
    wdata = 0x139;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b5s2
    addr = phy_base + 0x9003b * 4;
    wdata = 0x44;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b6s0
    addr = phy_base + 0x9003c * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b6s1
    addr = phy_base + 0x9003d * 4;
    wdata = 0x159;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b6s2
    addr = phy_base + 0x9003e * 4;
    wdata = 0x14f;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b7s0
    addr = phy_base + 0x9003f * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b7s1
    addr = phy_base + 0x90040 * 4;
    wdata = 0x159;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b7s2
    addr = phy_base + 0x90041 * 4;
    wdata = 0x47;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b8s0
    addr = phy_base + 0x90042 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b8s1
    addr = phy_base + 0x90043 * 4;
    wdata = 0x149;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b8s2
    addr = phy_base + 0x90044 * 4;
    wdata = 0x4f;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b9s0
    addr = phy_base + 0x90045 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b9s1
    addr = phy_base + 0x90046 * 4;
    wdata = 0x179;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b9s2
    addr = phy_base + 0x90047 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b10s0
    addr = phy_base + 0x90048 * 4;
    wdata = 0xe0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b10s1
    addr = phy_base + 0x90049 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b10s2
    addr = phy_base + 0x9004a * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b11s0
    addr = phy_base + 0x9004b * 4;
    wdata = 0x7c8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b11s1
    addr = phy_base + 0x9004c * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b11s2
    addr = phy_base + 0x9004d * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b12s0
    addr = phy_base + 0x9004e * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b12s1
    addr = phy_base + 0x9004f * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b12s2
    addr = phy_base + 0x90050 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b13s0
    addr = phy_base + 0x90051 * 4;
    wdata = 0x45a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b13s1
    addr = phy_base + 0x90052 * 4;
    wdata = 0x9;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b13s2
    addr = phy_base + 0x90053 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b14s0
    addr = phy_base + 0x90054 * 4;
    wdata = 0x448;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b14s1
    addr = phy_base + 0x90055 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b14s2
    addr = phy_base + 0x90056 * 4;
    wdata = 0x40;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b15s0
    addr = phy_base + 0x90057 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b15s1
    addr = phy_base + 0x90058 * 4;
    wdata = 0x179;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b15s2
    addr = phy_base + 0x90059 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b16s0
    addr = phy_base + 0x9005a * 4;
    wdata = 0x618;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b16s1
    addr = phy_base + 0x9005b * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b16s2
    addr = phy_base + 0x9005c * 4;
    wdata = 0x40c0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b17s0
    addr = phy_base + 0x9005d * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b17s1
    addr = phy_base + 0x9005e * 4;
    wdata = 0x149;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b17s2
    addr = phy_base + 0x9005f * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b18s0
    addr = phy_base + 0x90060 * 4;
    wdata = 0x4;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b18s1
    addr = phy_base + 0x90061 * 4;
    wdata = 0x48;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b18s2
    addr = phy_base + 0x90062 * 4;
    wdata = 0x4040;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b19s0
    addr = phy_base + 0x90063 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b19s1
    addr = phy_base + 0x90064 * 4;
    wdata = 0x149;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b19s2
    addr = phy_base + 0x90065 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b20s0
    addr = phy_base + 0x90066 * 4;
    wdata = 0x4;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b20s1
    addr = phy_base + 0x90067 * 4;
    wdata = 0x48;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b20s2
    addr = phy_base + 0x90068 * 4;
    wdata = 0x40;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b21s0
    addr = phy_base + 0x90069 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b21s1
    addr = phy_base + 0x9006a * 4;
    wdata = 0x149;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b21s2
    addr = phy_base + 0x9006b * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b22s0
    addr = phy_base + 0x9006c * 4;
    wdata = 0x4;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b22s1
    addr = phy_base + 0x9006d * 4;
    wdata = 0x18;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b22s2
    addr = phy_base + 0x9006e * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b23s0
    addr = phy_base + 0x9006f * 4;
    wdata = 0x4;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b23s1
    addr = phy_base + 0x90070 * 4;
    wdata = 0x78;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b23s2
    addr = phy_base + 0x90071 * 4;
    wdata = 0x549;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b24s0
    addr = phy_base + 0x90072 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b24s1
    addr = phy_base + 0x90073 * 4;
    wdata = 0x159;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b24s2
    addr = phy_base + 0x90074 * 4;
    wdata = 0xd49;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b25s0
    addr = phy_base + 0x90075 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b25s1
    addr = phy_base + 0x90076 * 4;
    wdata = 0x159;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b25s2
    addr = phy_base + 0x90077 * 4;
    wdata = 0x94a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b26s0
    addr = phy_base + 0x90078 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b26s1
    addr = phy_base + 0x90079 * 4;
    wdata = 0x159;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b26s2
    addr = phy_base + 0x9007a * 4;
    wdata = 0x441;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b27s0
    addr = phy_base + 0x9007b * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b27s1
    addr = phy_base + 0x9007c * 4;
    wdata = 0x149;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b27s2
    addr = phy_base + 0x9007d * 4;
    wdata = 0x42;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b28s0
    addr = phy_base + 0x9007e * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b28s1
    addr = phy_base + 0x9007f * 4;
    wdata = 0x149;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b28s2
    addr = phy_base + 0x90080 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b29s0
    addr = phy_base + 0x90081 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b29s1
    addr = phy_base + 0x90082 * 4;
    wdata = 0x149;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b29s2
    addr = phy_base + 0x90083 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b30s0
    addr = phy_base + 0x90084 * 4;
    wdata = 0xe0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b30s1
    addr = phy_base + 0x90085 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b30s2
    addr = phy_base + 0x90086 * 4;
    wdata = 0xa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b31s0
    addr = phy_base + 0x90087 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b31s1
    addr = phy_base + 0x90088 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b31s2
    addr = phy_base + 0x90089 * 4;
    wdata = 0x9;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b32s0
    addr = phy_base + 0x9008a * 4;
    wdata = 0x3c0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b32s1
    addr = phy_base + 0x9008b * 4;
    wdata = 0x149;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b32s2
    addr = phy_base + 0x9008c * 4;
    wdata = 0x9;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b33s0
    addr = phy_base + 0x9008d * 4;
    wdata = 0x3c0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b33s1
    addr = phy_base + 0x9008e * 4;
    wdata = 0x159;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b33s2
    addr = phy_base + 0x9008f * 4;
    wdata = 0x18;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b34s0
    addr = phy_base + 0x90090 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b34s1
    addr = phy_base + 0x90091 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b34s2
    addr = phy_base + 0x90092 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b35s0
    addr = phy_base + 0x90093 * 4;
    wdata = 0x3c0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b35s1
    addr = phy_base + 0x90094 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b35s2
    addr = phy_base + 0x90095 * 4;
    wdata = 0x18;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b36s0
    addr = phy_base + 0x90096 * 4;
    wdata = 0x4;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b36s1
    addr = phy_base + 0x90097 * 4;
    wdata = 0x48;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b36s2
    addr = phy_base + 0x90098 * 4;
    wdata = 0x18;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b37s0
    addr = phy_base + 0x90099 * 4;
    wdata = 0x4;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b37s1
    addr = phy_base + 0x9009a * 4;
    wdata = 0x58;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b37s2
    addr = phy_base + 0x9009b * 4;
    wdata = 0xa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b38s0
    addr = phy_base + 0x9009c * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b38s1
    addr = phy_base + 0x9009d * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b38s2
    addr = phy_base + 0x9009e * 4;
    wdata = 0x2;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b39s0
    addr = phy_base + 0x9009f * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b39s1
    addr = phy_base + 0x900a0 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b39s2
    addr = phy_base + 0x900a1 * 4;
    wdata = 0x5;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b40s0
    addr = phy_base + 0x900a2 * 4;
    wdata = 0x7c0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b40s1
    addr = phy_base + 0x900a3 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b40s2
    addr = phy_base + 0x40000 * 4;
    wdata = 0x811;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x0
    addr = phy_base + 0x40020 * 4;
    wdata = 0x880;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x0
    addr = phy_base + 0x40040 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x0
    addr = phy_base + 0x40060 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x0
    addr = phy_base + 0x40001 * 4;
    wdata = 0x4008;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x1
    addr = phy_base + 0x40021 * 4;
    wdata = 0x83;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x1
    addr = phy_base + 0x40041 * 4;
    wdata = 0x4f;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x1
    addr = phy_base + 0x40061 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x1
    addr = phy_base + 0x40002 * 4;
    wdata = 0x4040;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x2
    addr = phy_base + 0x40022 * 4;
    wdata = 0x83;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x2
    addr = phy_base + 0x40042 * 4;
    wdata = 0x51;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x2
    addr = phy_base + 0x40062 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x2
    addr = phy_base + 0x40003 * 4;
    wdata = 0x811;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x3
    addr = phy_base + 0x40023 * 4;
    wdata = 0x880;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x3
    addr = phy_base + 0x40043 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x3
    addr = phy_base + 0x40063 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x3
    addr = phy_base + 0x40004 * 4;
    wdata = 0x720;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x4
    addr = phy_base + 0x40024 * 4;
    wdata = 0xf;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x4
    addr = phy_base + 0x40044 * 4;
    wdata = 0x1740;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x4
    addr = phy_base + 0x40064 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x4
    addr = phy_base + 0x40005 * 4;
    wdata = 0x16;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x5
    addr = phy_base + 0x40025 * 4;
    wdata = 0x83;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x5
    addr = phy_base + 0x40045 * 4;
    wdata = 0x4b;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x5
    addr = phy_base + 0x40065 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x5
    addr = phy_base + 0x40006 * 4;
    wdata = 0x716;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x6
    addr = phy_base + 0x40026 * 4;
    wdata = 0xf;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x6
    addr = phy_base + 0x40046 * 4;
    wdata = 0x2001;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x6
    addr = phy_base + 0x40066 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x6
    addr = phy_base + 0x40007 * 4;
    wdata = 0x716;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x7
    addr = phy_base + 0x40027 * 4;
    wdata = 0xf;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x7
    addr = phy_base + 0x40047 * 4;
    wdata = 0x2800;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x7
    addr = phy_base + 0x40067 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x7
    addr = phy_base + 0x40008 * 4;
    wdata = 0x716;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x8
    addr = phy_base + 0x40028 * 4;
    wdata = 0xf;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x8
    addr = phy_base + 0x40048 * 4;
    wdata = 0xf00;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x8
    addr = phy_base + 0x40068 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x8
    addr = phy_base + 0x40009 * 4;
    wdata = 0x720;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x9
    addr = phy_base + 0x40029 * 4;
    wdata = 0xf;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x9
    addr = phy_base + 0x40049 * 4;
    wdata = 0x1400;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x9
    addr = phy_base + 0x40069 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x9
    addr = phy_base + 0x4000a * 4;
    wdata = 0xe08;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x10
    addr = phy_base + 0x4002a * 4;
    wdata = 0xc15;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x10
    addr = phy_base + 0x4004a * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x10
    addr = phy_base + 0x4006a * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x10
    addr = phy_base + 0x4000b * 4;
    wdata = 0x623;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x11
    addr = phy_base + 0x4002b * 4;
    wdata = 0x15;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x11
    addr = phy_base + 0x4004b * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x11
    addr = phy_base + 0x4006b * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x11
    addr = phy_base + 0x4000c * 4;
    wdata = 0x4028;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x12
    addr = phy_base + 0x4002c * 4;
    wdata = 0x80;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x12
    addr = phy_base + 0x4004c * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x12
    addr = phy_base + 0x4006c * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x12
    addr = phy_base + 0x4000d * 4;
    wdata = 0xe08;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x13
    addr = phy_base + 0x4002d * 4;
    wdata = 0xc1a;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x13
    addr = phy_base + 0x4004d * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x13
    addr = phy_base + 0x4006d * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x13
    addr = phy_base + 0x4000e * 4;
    wdata = 0x623;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x14
    addr = phy_base + 0x4002e * 4;
    wdata = 0x1a;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x14
    addr = phy_base + 0x4004e * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x14
    addr = phy_base + 0x4006e * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x14
    addr = phy_base + 0x4000f * 4;
    wdata = 0x4040;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x15
    addr = phy_base + 0x4002f * 4;
    wdata = 0x80;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x15
    addr = phy_base + 0x4004f * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x15
    addr = phy_base + 0x4006f * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x15
    addr = phy_base + 0x40010 * 4;
    wdata = 0x2604;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x16
    addr = phy_base + 0x40030 * 4;
    wdata = 0x15;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x16
    addr = phy_base + 0x40050 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x16
    addr = phy_base + 0x40070 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x16
    addr = phy_base + 0x40011 * 4;
    wdata = 0x708;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x17
    addr = phy_base + 0x40031 * 4;
    wdata = 0x5;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x17
    addr = phy_base + 0x40051 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x17
    addr = phy_base + 0x40071 * 4;
    wdata = 0x2002;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x17
    addr = phy_base + 0x40012 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x18
    addr = phy_base + 0x40032 * 4;
    wdata = 0x80;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x18
    addr = phy_base + 0x40052 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x18
    addr = phy_base + 0x40072 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x18
    addr = phy_base + 0x40013 * 4;
    wdata = 0x2604;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x19
    addr = phy_base + 0x40033 * 4;
    wdata = 0x1a;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x19
    addr = phy_base + 0x40053 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x19
    addr = phy_base + 0x40073 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x19
    addr = phy_base + 0x40014 * 4;
    wdata = 0x708;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x20
    addr = phy_base + 0x40034 * 4;
    wdata = 0xa;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x20
    addr = phy_base + 0x40054 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x20
    addr = phy_base + 0x40074 * 4;
    wdata = 0x2002;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x20
    addr = phy_base + 0x40015 * 4;
    wdata = 0x4040;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x21
    addr = phy_base + 0x40035 * 4;
    wdata = 0x80;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x21
    addr = phy_base + 0x40055 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x21
    addr = phy_base + 0x40075 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x21
    addr = phy_base + 0x40016 * 4;
    wdata = 0x60a;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x22
    addr = phy_base + 0x40036 * 4;
    wdata = 0x15;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x22
    addr = phy_base + 0x40056 * 4;
    wdata = 0x1200;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x22
    addr = phy_base + 0x40076 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x22
    addr = phy_base + 0x40017 * 4;
    wdata = 0x61a;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x23
    addr = phy_base + 0x40037 * 4;
    wdata = 0x15;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x23
    addr = phy_base + 0x40057 * 4;
    wdata = 0x1300;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x23
    addr = phy_base + 0x40077 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x23
    addr = phy_base + 0x40018 * 4;
    wdata = 0x60a;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x24
    addr = phy_base + 0x40038 * 4;
    wdata = 0x1a;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x24
    addr = phy_base + 0x40058 * 4;
    wdata = 0x1200;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x24
    addr = phy_base + 0x40078 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x24
    addr = phy_base + 0x40019 * 4;
    wdata = 0x642;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x25
    addr = phy_base + 0x40039 * 4;
    wdata = 0x1a;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x25
    addr = phy_base + 0x40059 * 4;
    wdata = 0x1300;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x25
    addr = phy_base + 0x40079 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x25
    addr = phy_base + 0x4001a * 4;
    wdata = 0x4808;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq0x26
    addr = phy_base + 0x4003a * 4;
    wdata = 0x880;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq1x26
    addr = phy_base + 0x4005a * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq2x26
    addr = phy_base + 0x4007a * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmSeq3x26
    addr = phy_base + 0x900a4 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b41s0
    addr = phy_base + 0x900a5 * 4;
    wdata = 0x790;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b41s1
    addr = phy_base + 0x900a6 * 4;
    wdata = 0x11a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b41s2
    addr = phy_base + 0x900a7 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b42s0
    addr = phy_base + 0x900a8 * 4;
    wdata = 0x7aa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b42s1
    addr = phy_base + 0x900a9 * 4;
    wdata = 0x2a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b42s2
    addr = phy_base + 0x900aa * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b43s0
    addr = phy_base + 0x900ab * 4;
    wdata = 0x7b2;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b43s1
    addr = phy_base + 0x900ac * 4;
    wdata = 0x2a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b43s2
    addr = phy_base + 0x900ad * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b44s0
    addr = phy_base + 0x900ae * 4;
    wdata = 0x7c8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b44s1
    addr = phy_base + 0x900af * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b44s2
    addr = phy_base + 0x900b0 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b45s0
    addr = phy_base + 0x900b1 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b45s1
    addr = phy_base + 0x900b2 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b45s2
    addr = phy_base + 0x900b3 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b46s0
    addr = phy_base + 0x900b4 * 4;
    wdata = 0x2a8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b46s1
    addr = phy_base + 0x900b5 * 4;
    wdata = 0x129;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b46s2
    addr = phy_base + 0x900b6 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b47s0
    addr = phy_base + 0x900b7 * 4;
    wdata = 0x370;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b47s1
    addr = phy_base + 0x900b8 * 4;
    wdata = 0x129;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b47s2
    addr = phy_base + 0x900b9 * 4;
    wdata = 0xa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b48s0
    addr = phy_base + 0x900ba * 4;
    wdata = 0x3c8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b48s1
    addr = phy_base + 0x900bb * 4;
    wdata = 0x1a9;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b48s2
    addr = phy_base + 0x900bc * 4;
    wdata = 0xc;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b49s0
    addr = phy_base + 0x900bd * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b49s1
    addr = phy_base + 0x900be * 4;
    wdata = 0x199;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b49s2
    addr = phy_base + 0x900bf * 4;
    wdata = 0x14;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b50s0
    addr = phy_base + 0x900c0 * 4;
    wdata = 0x790;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b50s1
    addr = phy_base + 0x900c1 * 4;
    wdata = 0x11a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b50s2
    addr = phy_base + 0x900c2 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b51s0
    addr = phy_base + 0x900c3 * 4;
    wdata = 0x4;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b51s1
    addr = phy_base + 0x900c4 * 4;
    wdata = 0x18;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b51s2
    addr = phy_base + 0x900c5 * 4;
    wdata = 0xe;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b52s0
    addr = phy_base + 0x900c6 * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b52s1
    addr = phy_base + 0x900c7 * 4;
    wdata = 0x199;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b52s2
    addr = phy_base + 0x900c8 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b53s0
    addr = phy_base + 0x900c9 * 4;
    wdata = 0x8568;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b53s1
    addr = phy_base + 0x900ca * 4;
    wdata = 0x108;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b53s2
    addr = phy_base + 0x900cb * 4;
    wdata = 0x18;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b54s0
    addr = phy_base + 0x900cc * 4;
    wdata = 0x790;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b54s1
    addr = phy_base + 0x900cd * 4;
    wdata = 0x16a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b54s2
    addr = phy_base + 0x900ce * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b55s0
    addr = phy_base + 0x900cf * 4;
    wdata = 0x1d8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b55s1
    addr = phy_base + 0x900d0 * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b55s2
    addr = phy_base + 0x900d1 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b56s0
    addr = phy_base + 0x900d2 * 4;
    wdata = 0x8558;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b56s1
    addr = phy_base + 0x900d3 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b56s2
    addr = phy_base + 0x900d4 * 4;
    wdata = 0x70;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b57s0
    addr = phy_base + 0x900d5 * 4;
    wdata = 0x788;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b57s1
    addr = phy_base + 0x900d6 * 4;
    wdata = 0x16a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b57s2
    addr = phy_base + 0x900d7 * 4;
    wdata = 0x1ff8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b58s0
    addr = phy_base + 0x900d8 * 4;
    wdata = 0x85a8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b58s1
    addr = phy_base + 0x900d9 * 4;
    wdata = 0x1e8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b58s2
    addr = phy_base + 0x900da * 4;
    wdata = 0x50;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b59s0
    addr = phy_base + 0x900db * 4;
    wdata = 0x798;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b59s1
    addr = phy_base + 0x900dc * 4;
    wdata = 0x16a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b59s2
    addr = phy_base + 0x900dd * 4;
    wdata = 0x60;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b60s0
    addr = phy_base + 0x900de * 4;
    wdata = 0x7a0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b60s1
    addr = phy_base + 0x900df * 4;
    wdata = 0x16a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b60s2
    addr = phy_base + 0x900e0 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b61s0
    addr = phy_base + 0x900e1 * 4;
    wdata = 0x8310;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b61s1
    addr = phy_base + 0x900e2 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b61s2
    addr = phy_base + 0x900e3 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b62s0
    addr = phy_base + 0x900e4 * 4;
    wdata = 0xa310;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b62s1
    addr = phy_base + 0x900e5 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b62s2
    addr = phy_base + 0x900e6 * 4;
    wdata = 0xa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b63s0
    addr = phy_base + 0x900e7 * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b63s1
    addr = phy_base + 0x900e8 * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b63s2
    addr = phy_base + 0x900e9 * 4;
    wdata = 0x6e;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b64s0
    addr = phy_base + 0x900ea * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b64s1
    addr = phy_base + 0x900eb * 4;
    wdata = 0x68;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b64s2
    addr = phy_base + 0x900ec * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b65s0
    addr = phy_base + 0x900ed * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b65s1
    addr = phy_base + 0x900ee * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b65s2
    addr = phy_base + 0x900ef * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b66s0
    addr = phy_base + 0x900f0 * 4;
    wdata = 0x8310;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b66s1
    addr = phy_base + 0x900f1 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b66s2
    addr = phy_base + 0x900f2 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b67s0
    addr = phy_base + 0x900f3 * 4;
    wdata = 0xa310;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b67s1
    addr = phy_base + 0x900f4 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b67s2
    addr = phy_base + 0x900f5 * 4;
    wdata = 0x1ff8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b68s0
    addr = phy_base + 0x900f6 * 4;
    wdata = 0x85a8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b68s1
    addr = phy_base + 0x900f7 * 4;
    wdata = 0x1e8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b68s2
    addr = phy_base + 0x900f8 * 4;
    wdata = 0x68;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b69s0
    addr = phy_base + 0x900f9 * 4;
    wdata = 0x798;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b69s1
    addr = phy_base + 0x900fa * 4;
    wdata = 0x16a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b69s2
    addr = phy_base + 0x900fb * 4;
    wdata = 0x78;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b70s0
    addr = phy_base + 0x900fc * 4;
    wdata = 0x7a0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b70s1
    addr = phy_base + 0x900fd * 4;
    wdata = 0x16a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b70s2
    addr = phy_base + 0x900fe * 4;
    wdata = 0x68;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b71s0
    addr = phy_base + 0x900ff * 4;
    wdata = 0x790;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b71s1
    addr = phy_base + 0x90100 * 4;
    wdata = 0x16a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b71s2
    addr = phy_base + 0x90101 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b72s0
    addr = phy_base + 0x90102 * 4;
    wdata = 0x8b10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b72s1
    addr = phy_base + 0x90103 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b72s2
    addr = phy_base + 0x90104 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b73s0
    addr = phy_base + 0x90105 * 4;
    wdata = 0xab10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b73s1
    addr = phy_base + 0x90106 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b73s2
    addr = phy_base + 0x90107 * 4;
    wdata = 0xa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b74s0
    addr = phy_base + 0x90108 * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b74s1
    addr = phy_base + 0x90109 * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b74s2
    addr = phy_base + 0x9010a * 4;
    wdata = 0x58;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b75s0
    addr = phy_base + 0x9010b * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b75s1
    addr = phy_base + 0x9010c * 4;
    wdata = 0x68;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b75s2
    addr = phy_base + 0x9010d * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b76s0
    addr = phy_base + 0x9010e * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b76s1
    addr = phy_base + 0x9010f * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b76s2
    addr = phy_base + 0x90110 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b77s0
    addr = phy_base + 0x90111 * 4;
    wdata = 0x8b10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b77s1
    addr = phy_base + 0x90112 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b77s2
    addr = phy_base + 0x90113 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b78s0
    addr = phy_base + 0x90114 * 4;
    wdata = 0xab10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b78s1
    addr = phy_base + 0x90115 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b78s2
    addr = phy_base + 0x90116 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b79s0
    addr = phy_base + 0x90117 * 4;
    wdata = 0x1d8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b79s1
    addr = phy_base + 0x90118 * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b79s2
    addr = phy_base + 0x90119 * 4;
    wdata = 0x80;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b80s0
    addr = phy_base + 0x9011a * 4;
    wdata = 0x790;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b80s1
    addr = phy_base + 0x9011b * 4;
    wdata = 0x16a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b80s2
    addr = phy_base + 0x9011c * 4;
    wdata = 0x18;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b81s0
    addr = phy_base + 0x9011d * 4;
    wdata = 0x7aa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b81s1
    addr = phy_base + 0x9011e * 4;
    wdata = 0x6a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b81s2
    addr = phy_base + 0x9011f * 4;
    wdata = 0xa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b82s0
    addr = phy_base + 0x90120 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b82s1
    addr = phy_base + 0x90121 * 4;
    wdata = 0x1e9;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b82s2
    addr = phy_base + 0x90122 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b83s0
    addr = phy_base + 0x90123 * 4;
    wdata = 0x8080;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b83s1
    addr = phy_base + 0x90124 * 4;
    wdata = 0x108;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b83s2
    addr = phy_base + 0x90125 * 4;
    wdata = 0xf;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b84s0
    addr = phy_base + 0x90126 * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b84s1
    addr = phy_base + 0x90127 * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b84s2
    addr = phy_base + 0x90128 * 4;
    wdata = 0xc;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b85s0
    addr = phy_base + 0x90129 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b85s1
    addr = phy_base + 0x9012a * 4;
    wdata = 0x68;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b85s2
    addr = phy_base + 0x9012b * 4;
    wdata = 0x9;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b86s0
    addr = phy_base + 0x9012c * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b86s1
    addr = phy_base + 0x9012d * 4;
    wdata = 0x1a9;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b86s2
    addr = phy_base + 0x9012e * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b87s0
    addr = phy_base + 0x9012f * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b87s1
    addr = phy_base + 0x90130 * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b87s2
    addr = phy_base + 0x90131 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b88s0
    addr = phy_base + 0x90132 * 4;
    wdata = 0x8080;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b88s1
    addr = phy_base + 0x90133 * 4;
    wdata = 0x108;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b88s2
    addr = phy_base + 0x90134 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b89s0
    addr = phy_base + 0x90135 * 4;
    wdata = 0x7aa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b89s1
    addr = phy_base + 0x90136 * 4;
    wdata = 0x6a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b89s2
    addr = phy_base + 0x90137 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b90s0
    addr = phy_base + 0x90138 * 4;
    wdata = 0x8568;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b90s1
    addr = phy_base + 0x90139 * 4;
    wdata = 0x108;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b90s2
    addr = phy_base + 0x9013a * 4;
    wdata = 0xb7;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b91s0
    addr = phy_base + 0x9013b * 4;
    wdata = 0x790;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b91s1
    addr = phy_base + 0x9013c * 4;
    wdata = 0x16a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b91s2
    addr = phy_base + 0x9013d * 4;
    wdata = 0x1f;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b92s0
    addr = phy_base + 0x9013e * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b92s1
    addr = phy_base + 0x9013f * 4;
    wdata = 0x68;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b92s2
    addr = phy_base + 0x90140 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b93s0
    addr = phy_base + 0x90141 * 4;
    wdata = 0x8558;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b93s1
    addr = phy_base + 0x90142 * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b93s2
    addr = phy_base + 0x90143 * 4;
    wdata = 0xf;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b94s0
    addr = phy_base + 0x90144 * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b94s1
    addr = phy_base + 0x90145 * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b94s2
    addr = phy_base + 0x90146 * 4;
    wdata = 0xc;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b95s0
    addr = phy_base + 0x90147 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b95s1
    addr = phy_base + 0x90148 * 4;
    wdata = 0x68;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b95s2
    addr = phy_base + 0x90149 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b96s0
    addr = phy_base + 0x9014a * 4;
    wdata = 0x408;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b96s1
    addr = phy_base + 0x9014b * 4;
    wdata = 0x169;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b96s2
    addr = phy_base + 0x9014c * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b97s0
    addr = phy_base + 0x9014d * 4;
    wdata = 0x8558;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b97s1
    addr = phy_base + 0x9014e * 4;
    wdata = 0x168;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b97s2
    addr = phy_base + 0x9014f * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b98s0
    addr = phy_base + 0x90150 * 4;
    wdata = 0x3c8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b98s1
    addr = phy_base + 0x90151 * 4;
    wdata = 0x1a9;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b98s2
    addr = phy_base + 0x90152 * 4;
    wdata = 0x3;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b99s0
    addr = phy_base + 0x90153 * 4;
    wdata = 0x370;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b99s1
    addr = phy_base + 0x90154 * 4;
    wdata = 0x129;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b99s2
    addr = phy_base + 0x90155 * 4;
    wdata = 0x20;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b100s0
    addr = phy_base + 0x90156 * 4;
    wdata = 0x2aa;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b100s1
    addr = phy_base + 0x90157 * 4;
    wdata = 0x9;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b100s2
    addr = phy_base + 0x90158 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b101s0
    addr = phy_base + 0x90159 * 4;
    wdata = 0x400;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b101s1
    addr = phy_base + 0x9015a * 4;
    wdata = 0x10e;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b101s2
    addr = phy_base + 0x9015b * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b102s0
    addr = phy_base + 0x9015c * 4;
    wdata = 0xe8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b102s1
    addr = phy_base + 0x9015d * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b102s2
    addr = phy_base + 0x9015e * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b103s0
    addr = phy_base + 0x9015f * 4;
    wdata = 0x8140;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b103s1
    addr = phy_base + 0x90160 * 4;
    wdata = 0x10c;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b103s2
    addr = phy_base + 0x90161 * 4;
    wdata = 0x10;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b104s0
    addr = phy_base + 0x90162 * 4;
    wdata = 0x8138;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b104s1
    addr = phy_base + 0x90163 * 4;
    wdata = 0x10c;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b104s2
    addr = phy_base + 0x90164 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b105s0
    addr = phy_base + 0x90165 * 4;
    wdata = 0x7c8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b105s1
    addr = phy_base + 0x90166 * 4;
    wdata = 0x101;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b105s2
    addr = phy_base + 0x90167 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b106s0
    addr = phy_base + 0x90168 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b106s1
    addr = phy_base + 0x90169 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b106s2
    addr = phy_base + 0x9016a * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b107s0
    addr = phy_base + 0x9016b * 4;
    wdata = 0x448;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b107s1
    addr = phy_base + 0x9016c * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b107s2
    addr = phy_base + 0x9016d * 4;
    wdata = 0xf;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b108s0
    addr = phy_base + 0x9016e * 4;
    wdata = 0x7c0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b108s1
    addr = phy_base + 0x9016f * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b108s2
    addr = phy_base + 0x90170 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b109s0
    addr = phy_base + 0x90171 * 4;
    wdata = 0xe8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b109s1
    addr = phy_base + 0x90172 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b109s2
    addr = phy_base + 0x90173 * 4;
    wdata = 0x47;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b110s0
    addr = phy_base + 0x90174 * 4;
    wdata = 0x630;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b110s1
    addr = phy_base + 0x90175 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b110s2
    addr = phy_base + 0x90176 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b111s0
    addr = phy_base + 0x90177 * 4;
    wdata = 0x618;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b111s1
    addr = phy_base + 0x90178 * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b111s2
    addr = phy_base + 0x90179 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b112s0
    addr = phy_base + 0x9017a * 4;
    wdata = 0xe0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b112s1
    addr = phy_base + 0x9017b * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b112s2
    addr = phy_base + 0x9017c * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b113s0
    addr = phy_base + 0x9017d * 4;
    wdata = 0x7c8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b113s1
    addr = phy_base + 0x9017e * 4;
    wdata = 0x109;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b113s2
    addr = phy_base + 0x9017f * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b114s0
    addr = phy_base + 0x90180 * 4;
    wdata = 0x8140;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b114s1
    addr = phy_base + 0x90181 * 4;
    wdata = 0x10c;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b114s2
    addr = phy_base + 0x90182 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b115s0
    addr = phy_base + 0x90183 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b115s1
    addr = phy_base + 0x90184 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b115s2
    addr = phy_base + 0x90185 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b116s0
    addr = phy_base + 0x90186 * 4;
    wdata = 0x4;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b116s1
    addr = phy_base + 0x90187 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b116s2
    addr = phy_base + 0x90188 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b117s0
    addr = phy_base + 0x90189 * 4;
    wdata = 0x7c8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b117s1
    addr = phy_base + 0x9018a * 4;
    wdata = 0x101;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_SequenceReg0b117s2
    addr = phy_base + 0x90006 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s0
    addr = phy_base + 0x90007 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s1
    addr = phy_base + 0x90008 * 4;
    wdata = 0x8;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s2
    addr = phy_base + 0x90009 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s0
    addr = phy_base + 0x9000a * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s1
    addr = phy_base + 0x9000b * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s2
    addr = phy_base + 0xd00e7 * 4;
    wdata = 0x400;
    writel(wdata, addr); // DWC_DDRPHYA_APBONLY0_SequencerOverride
    addr = phy_base + 0x90017 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_StartVector0b0
    addr = phy_base + 0x9001f * 4;
    wdata = 0x29;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_StartVector0b8
    addr = phy_base + 0x90026 * 4;
    wdata = 0x6a;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_StartVector0b15
    addr = phy_base + 0x400d0 * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl0
    addr = phy_base + 0x400d1 * 4;
    wdata = 0x101;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl1
    addr = phy_base + 0x400d2 * 4;
    wdata = 0x105;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl2
    addr = phy_base + 0x400d3 * 4;
    wdata = 0x107;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl3
    addr = phy_base + 0x400d4 * 4;
    wdata = 0x10f;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl4
    addr = phy_base + 0x400d5 * 4;
    wdata = 0x202;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl5
    addr = phy_base + 0x400d6 * 4;
    wdata = 0x20a;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl6
    addr = phy_base + 0x400d7 * 4;
    wdata = 0x20b;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl7
    addr = phy_base + 0x2003a * 4;
    wdata = 0x2;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_DbyteDllModeCntrl
    addr = phy_base + 0x2000b * 4;
    wdata = 0x85;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_Seq0BDLY0_p0
    addr = phy_base + 0x2000c * 4;
    wdata = 0x10a;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_Seq0BDLY1_p0
    addr = phy_base + 0x2000d * 4;
    wdata = 0xa6a;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_Seq0BDLY2_p0
    addr = phy_base + 0x2000e * 4;
    wdata = 0x2c;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_Seq0BDLY3_p0
    addr = phy_base + 0x9000c * 4;
    wdata = 0x0;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag0
    addr = phy_base + 0x9000d * 4;
    wdata = 0x173;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag1
    addr = phy_base + 0x9000e * 4;
    wdata = 0x60;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag2
    addr = phy_base + 0x9000f * 4;
    wdata = 0x6110;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag3
    addr = phy_base + 0x90010 * 4;
    wdata = 0x2152;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag4
    addr = phy_base + 0x90011 * 4;
    wdata = 0xdfbd;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag5
    addr = phy_base + 0x90012 * 4;
    wdata = 0xffff;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag6
    addr = phy_base + 0x90013 * 4;
    wdata = 0x6152;
    writel(wdata, addr); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag7
    addr = phy_base + 0x40080 * 4;
    wdata = 0xe0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmPlayback0x0_p0
    addr = phy_base + 0x40081 * 4;
    wdata = 0x12;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmPlayback1x0_p0
    addr = phy_base + 0x40082 * 4;
    wdata = 0xe0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmPlayback0x1_p0
    addr = phy_base + 0x40083 * 4;
    wdata = 0x12;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmPlayback1x1_p0
    addr = phy_base + 0x40084 * 4;
    wdata = 0xe0;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmPlayback0x2_p0
    addr = phy_base + 0x40085 * 4;
    wdata = 0x12;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmPlayback1x2_p0
    addr = phy_base + 0x400fd * 4;
    wdata = 0xf;
    writel(wdata, addr); // DWC_DDRPHYA_ACSM0_AcsmCtrl13
    addr = phy_base + 0x10011 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TsmByte1
    addr = phy_base + 0x10012 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TsmByte2
    addr = phy_base + 0x10013 * 4;
    wdata = 0x180;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TsmByte3
    addr = phy_base + 0x10018 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TsmByte5
    addr = phy_base + 0x10002 * 4;
    wdata = 0x6209;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_TrainingParam
    addr = phy_base + 0x100b2 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_Tsm0_i0
    addr = phy_base + 0x101b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_Tsm2_i1
    addr = phy_base + 0x102b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_Tsm2_i2
    addr = phy_base + 0x103b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_Tsm2_i3
    addr = phy_base + 0x104b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_Tsm2_i4
    addr = phy_base + 0x105b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_Tsm2_i5
    addr = phy_base + 0x106b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_Tsm2_i6
    addr = phy_base + 0x107b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_Tsm2_i7
    addr = phy_base + 0x108b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE0_Tsm2_i8
    addr = phy_base + 0x11011 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TsmByte1
    addr = phy_base + 0x11012 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TsmByte2
    addr = phy_base + 0x11013 * 4;
    wdata = 0x180;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TsmByte3
    addr = phy_base + 0x11018 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TsmByte5
    addr = phy_base + 0x11002 * 4;
    wdata = 0x6209;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_TrainingParam
    addr = phy_base + 0x110b2 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_Tsm0_i0
    addr = phy_base + 0x111b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_Tsm2_i1
    addr = phy_base + 0x112b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_Tsm2_i2
    addr = phy_base + 0x113b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_Tsm2_i3
    addr = phy_base + 0x114b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_Tsm2_i4
    addr = phy_base + 0x115b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_Tsm2_i5
    addr = phy_base + 0x116b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_Tsm2_i6
    addr = phy_base + 0x117b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_Tsm2_i7
    addr = phy_base + 0x118b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE1_Tsm2_i8
    addr = phy_base + 0x12011 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TsmByte1
    addr = phy_base + 0x12012 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TsmByte2
    addr = phy_base + 0x12013 * 4;
    wdata = 0x180;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TsmByte3
    addr = phy_base + 0x12018 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TsmByte5
    addr = phy_base + 0x12002 * 4;
    wdata = 0x6209;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_TrainingParam
    addr = phy_base + 0x120b2 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_Tsm0_i0
    addr = phy_base + 0x121b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_Tsm2_i1
    addr = phy_base + 0x122b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_Tsm2_i2
    addr = phy_base + 0x123b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_Tsm2_i3
    addr = phy_base + 0x124b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_Tsm2_i4
    addr = phy_base + 0x125b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_Tsm2_i5
    addr = phy_base + 0x126b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_Tsm2_i6
    addr = phy_base + 0x127b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_Tsm2_i7
    addr = phy_base + 0x128b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE2_Tsm2_i8
    addr = phy_base + 0x13011 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TsmByte1
    addr = phy_base + 0x13012 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TsmByte2
    addr = phy_base + 0x13013 * 4;
    wdata = 0x180;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TsmByte3
    addr = phy_base + 0x13018 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TsmByte5
    addr = phy_base + 0x13002 * 4;
    wdata = 0x6209;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_TrainingParam
    addr = phy_base + 0x130b2 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_Tsm0_i0
    addr = phy_base + 0x131b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_Tsm2_i1
    addr = phy_base + 0x132b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_Tsm2_i2
    addr = phy_base + 0x133b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_Tsm2_i3
    addr = phy_base + 0x134b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_Tsm2_i4
    addr = phy_base + 0x135b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_Tsm2_i5
    addr = phy_base + 0x136b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_Tsm2_i6
    addr = phy_base + 0x137b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_Tsm2_i7
    addr = phy_base + 0x138b4 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_DBYTE3_Tsm2_i8
    addr = phy_base + 0x20089 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_CalZap
    addr = phy_base + 0x20088 * 4;
    wdata = 0x19;
    writel(wdata, addr); // DWC_DDRPHYA_MASTER0_CalRate
    addr = phy_base + 0xc0080 * 4;
    wdata = 0x2;
    writel(wdata, addr); // DWC_DDRPHYA_DRTUB0_UcclkHclkEnables
    addr = phy_base + 0xd0000 * 4;
    wdata = 0x1;
    writel(wdata, addr); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
#endif

    addr = umctl_base + 0x000001b0;
    wdata = 0x00001060;
    writel(wdata, addr);
    addr = umctl_base + 0x000001bc;
    rdata = readl(addr);

    while (rdata != 1) {
        rdata = readl(addr);
    }

    addr = umctl_base + 0x000001b0;
    wdata = 0x00001040;
    writel(wdata, addr);
    addr = umctl_base + 0x000001b0;
    wdata = 0x00001041;
    writel(wdata, addr);
    addr = umctl_base + 0x000001b0;
    wdata = 0x00001041;
    writel(wdata, addr);
    addr = umctl_base + 0x00000320;
    wdata = 0x00000001;
    writel(wdata, addr);
    addr = umctl_base + 0x00000324;
    rdata = readl(addr);

    while (rdata != 1) {
        rdata = readl(addr);
    }

    addr = umctl_base + 0x00000004;
    rdata = readl(addr);

    while (rdata != 1) {
        rdata = readl(addr);
    }

    addr = umctl_base + 0x00000030;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000030;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000490;
    wdata = 0x00000001;
    writel(wdata, addr);
    addr = umctl_base + 0x00000f24;
    wdata = 0x0000ff10;
    writel(wdata, addr);
    addr = umctl_base + 0x00000030;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000030;
    wdata = 0x00000000;
    writel(wdata, addr);
    addr = umctl_base + 0x00000020;
    wdata = 0x00001404;
    writel(wdata, addr);
    addr = umctl_base + 0x00002020;
    wdata = 0x00001404;
    writel(wdata, addr);
    addr = umctl_base + 0x00003020;
    wdata = 0x00001404;
    writel(wdata, addr);
    addr = umctl_base + 0x00004020;
    wdata = 0x00001404;
    writel(wdata, addr);
///addr = umctl_base + 0x00000490; wdata = 0x00000000; writel(wdata,addr);
///addr = umctl_base + 0x000003fc; rdata = readl(addr);
///addr = umctl_base + 0x00000f24; wdata = 0x0000ff10; writel(wdata,addr);
///addr = umctl_base + 0x00000f28; rdata = readl(addr);
///addr = umctl_base + 0x00000304; wdata = 0x00000002; writel(wdata,addr);
///addr = umctl_base + 0x00000308; rdata = readl(addr);
///addr = umctl_base + 0x00000060; wdata = 0x00000001; writel(wdata,addr);
///addr = umctl_base + 0x00000018; rdata = readl(addr);




    return 0;

}

