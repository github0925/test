/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <debug.h>
#include <string.h>
#include <app.h>
#include <lib/console.h>

#include <reg.h>
#include <res.h>
#include <chip_res.h>
#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0

/* ce key ctrl, use 16 bits, bit value = 1 mean the ce can access the key
*_________________________________________________________________________________________________________
*|-----15-----|-----14-----|-----13-----|-----12-----|-----11-----|-----10-----|-----09-----|-----08 ----|
*|          reserve        |  VLK LOCK  |  VLK LOCK  |            |            | EFUSE GPK3 | EFUSE GPK2 |
*_________________________________________________________________________________________________________
*|-----07-----|-----06-----|-----05-----|-----04-----|-----03-----|-----02-----|-----01-----|-----00-----|
*| EFUSE GPK1 | EFUSE GPK0 | EFUSE SEK2 | EFUSE SEK1 | EFUSE SEK0 |EFUSE ROTPK1|EFUSE ROTPK0|     HUK    |
*/
typedef enum key_enable_offset {
    SAFE_SS_GLUE_OR2_NET_HUK_HLOCK_SE_PROD_Z = 0,
    EFUSEC_ROTPK0_HLOCK_SE,
    EFUSEC_ROTPK1_HLOCK_SE,
    EFUSEC_SEK0_HLOCK_SE,
    EFUSEC_SEK1_HLOCK_SE,
    EFUSEC_SEK2_HLOCK_SE,
    EFUSEC_GP_KEY0_HLOCK_SE,
    EFUSEC_GP_KEY1_HLOCK_SE,
    EFUSEC_GP_KEY2_HLOCK_SE,
    EFUSEC_GP_KEY3_HLOCK_SE,
    SAFE_SS_GLUE_OR2_CE1_KEY_PERCTRL_KEY_HLOCK_10_Z,
    SAFE_SS_GLUE_OR2_CE1_KEY_PERCTRL_KEY_HLOCK_11_Z,
    I_SEC_STORAGE1_VLK_HLOCK_SAFETY_LOCK,
    I_SEC_STORAGE2_VLK_HLOCK_SAFETY_LOCK,
} key_enable_offset_t;

void L16_ce1key_enable_init(paddr_t phy_addr, int32_t offset_index)
{
    unsigned int data = 0;

    LTRACEF("enter phy_addr = 0x%x, offset_index = %d\n", (uint32_t)phy_addr, offset_index);
    // scr_saf, CE1_KEY_PERCTRL_SCR_KEY_ENABLE_15_0, [143:128], scr_num - 4, bits - 16
    // scr_saf[143:128], start_bit - 0, end_bit - 15, offset - 0x2100[15:0]
    // huk rotpk1 sek0 can access
    data = (1 << SAFE_SS_GLUE_OR2_NET_HUK_HLOCK_SE_PROD_Z) | (1 << EFUSEC_ROTPK1_HLOCK_SE) | (1 << EFUSEC_SEK0_HLOCK_SE);
    writel(data, phy_addr + (offset_index << 12));

    if (LOCAL_TRACE) {
        data = readl(phy_addr + (offset_index << 12));
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr + (offset_index << 12)), data);
    }
}

void L16_ce2key_enable_init(paddr_t phy_addr, int32_t offset_index)
{
    unsigned int data = 0;
    paddr_t phy_addr_temp = 0;

    phy_addr_temp = phy_addr + (offset_index << 12);

    LTRACEF("enter phy_addr_temp = 0x%x\n", (uint32_t)phy_addr_temp);
    // scr_sec, CE2_KEY_PERCTRL_SCR_KEY_ENABLE_15_0, [15:0], scr_num - 0, bits - 16
    // scr_sec[15:0], start_bit - 0, end_bit - 15, offset - 0x200, data0[15:0]
    // huk rotpk0 sek1 can access
    data = (1 << SAFE_SS_GLUE_OR2_NET_HUK_HLOCK_SE_PROD_Z) | (1 << EFUSEC_ROTPK0_HLOCK_SE) | (1 << EFUSEC_SEK1_HLOCK_SE);
    writel(data, phy_addr_temp);

    if (LOCAL_TRACE) {
        data = readl(phy_addr_temp);
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)phy_addr_temp, data);
    }

    // scr_sec, CE2_KEY_PERCTRL_SCR_KEY_ENABLE_31_16, [47:32], scr_num - 1, bits - 16
    // scr_sec[47:32], start_bit - 0, end_bit - 15, offset - 0x204, data0[15:0]
    // huk sek2 can access
    data = (1 << SAFE_SS_GLUE_OR2_NET_HUK_HLOCK_SE_PROD_Z) | (1 << EFUSEC_SEK2_HLOCK_SE);
    writel(data, phy_addr_temp + (0x4 << 10));

    if (LOCAL_TRACE) {
        data = readl(phy_addr_temp + (0x4 << 10));
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr_temp + (0x4 << 10)), data);
    }

    // scr_sec, CE2_KEY_PERCTRL_SCR_KEY_ENABLE_47_32, [79:64], scr_num - 2, bits - 16
    // scr_sec[79:64], start_bit - 0, end_bit - 15, offset - 0x208, data0[15:0]
    // huk rotpk0 gpkey0 gpkey1 can access
    data = (1 << SAFE_SS_GLUE_OR2_NET_HUK_HLOCK_SE_PROD_Z) | (1 << EFUSEC_ROTPK0_HLOCK_SE) | (1 << EFUSEC_GP_KEY0_HLOCK_SE) | (1 << EFUSEC_GP_KEY1_HLOCK_SE);
    writel(data, phy_addr_temp + (0x8 << 10));

    if (LOCAL_TRACE) {
        data = readl(phy_addr_temp + (0x8 << 10));
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr_temp + (0x8 << 10)), data);
    }

    // scr_sec, CE2_KEY_PERCTRL_SCR_KEY_ENABLE_63_48, [111:96], scr_num - 3, bits - 16
    // scr_sec[111:96], start_bit - 0, end_bit - 15, offset - 0x20c, data0[15:0]
    // huk gpkey2 gpkey3 can access
    data = (1 << SAFE_SS_GLUE_OR2_NET_HUK_HLOCK_SE_PROD_Z) | (1 << EFUSEC_GP_KEY2_HLOCK_SE) | (1 << EFUSEC_GP_KEY3_HLOCK_SE);
    writel(data, phy_addr_temp + (0xc << 10));

    if (LOCAL_TRACE) {
        data = readl(phy_addr_temp + (0xc << 10));
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr_temp + (0xc << 10)), data);
    }

    // scr_sec, CE2_KEY_PERCTRL_SCR_KEY_ENABLE_79_64, [143:128], scr_num - 4, bits - 16
    // scr_sec[143:128], start_bit - 0, end_bit - 15, offset - 0x210, data0[15:0]
    data = 0;
    writel(data, phy_addr_temp + (0x10 << 10));

    if (LOCAL_TRACE) {
        data = readl(phy_addr_temp + (0x10 << 10));
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr_temp + (0x10 << 10)), data);
    }

    // scr_sec, CE2_KEY_PERCTRL_SCR_KEY_ENABLE_95_80, [175:160], scr_num - 5, bits - 16
    // scr_sec[175:160], start_bit - 0, end_bit - 15, offset - 0x214, data0[15:0]
    data = 0;
    writel(data, phy_addr_temp + (0x14 << 10));

    if (LOCAL_TRACE) {
        data = readl(phy_addr_temp + (0x14 << 10));
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr_temp + (0x14 << 10)), data);
    }

    // scr_sec, CE2_KEY_PERCTRL_SCR_KEY_ENABLE_111_96, [207:192], scr_num - 6, bits - 16
    // scr_sec[207:192], start_bit - 0, end_bit - 15, offset - 0x2180[15:0]
    data = 0;
    writel(data, phy_addr_temp + (0x18 << 10));

    if (LOCAL_TRACE) {
        data = readl(phy_addr_temp + (0x18 << 10));
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr_temp + (0x18 << 10)), data);
    }

    // scr_sec, CE2_KEY_PERCTRL_SCR_KEY_ENABLE_127_112, [239:224], scr_num - 7, bits - 16
    // scr_sec[239:224], start_bit - 0, end_bit - 15, offset - 0x21c0[15:0]
    data = 0;
    writel(data, phy_addr_temp + (0x1c << 10));

    if (LOCAL_TRACE) {
        data = readl(phy_addr_temp + (0x1c << 10));
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr_temp + (0x1c << 10)), data);
    }
}

int ce_key_ctrl(void)
{

    int ret = 0;
    paddr_t phy_addr = 0;
    int32_t resid = 0;
    int32_t offset_index = 0;

    resid = RES_SCR_L16_SEC_CE2_KEY_PERCTRL_SCR_KEY_ENABLE_15_0;
    ret = res_get_info_by_id(resid, &phy_addr, &offset_index);

    if (ret == -1) {
        LTRACEF("ce2 resouce error\n");
    }
    else {
        L16_ce2key_enable_init(phy_addr, offset_index);
    }

    return ret;
}

/* hardcode scr init by following Z1 configuration when system boot */
#define SCR_SEC_BASE 0xF8200000
void src_init_hc(void)
{
    //debug enable
    //set_scr_sec_cpu1_dbgen(1)/set_scr_sec_cpu1_niden(1)/set_scr_sec_cpu1_spiden(1)/set_scr_sec_cpu1_spniden(1); (778, 0xF0000)
    uint32_t rval = readl(SCR_SEC_BASE + (266 << 12));
    rval = (rval & (~0xF0000)) | 0xF0000;
    writel(rval, SCR_SEC_BASE + (266 << 12));

    //set_scr_sec_cr5_sec_dbgen(1)/set_scr_sec_cr5_sec_niden(1); (782, 0x3)
    rval = readl(SCR_SEC_BASE + (270 << 12));
    rval = (rval & (~0x3)) | 0x3;
    writel(rval, SCR_SEC_BASE + (270 << 12));

    //set_scr_sec_cssys_dbgen(1)/set_scr_sec_cssys_niden(1)/set_scr_sec_cssys_spiden(1)/set_scr_sec_cssys_spniden(1);    (786, 0xF)
    rval = readl(SCR_SEC_BASE + (274 << 12));
    rval = (rval & (~0xF)) | 0xF;
    writel(rval, SCR_SEC_BASE + (274 << 12));

#if VIRTUALIZATION_ENABLE
#if VIRTUALIZATION_EXT
    //bypass gpu2
    writel(0x0240, SCR_SEC_BASE + (0x274 <<10));
#else
    //all enable (bypass TBU14/15, other enable 0x1800)
    writel(0x0200, SCR_SEC_BASE + (0x274 <<10));
    dprintf(INFO, "tbu0-tbu15 bypass status 0x%x\n", readl(SCR_SEC_BASE + (0x274 <<10)));
#endif
#endif
}
/* hardcode scr init end */

int scr_init(void)
{
    int ret = 0;

    //ce key setting
    ret = ce_key_ctrl();
    if (ret) {
        dprintf(CRITICAL, "ce_key_ctrl fail.\n");
        return ret;
    }

    // other setting
    src_init_hc();

    return ret;
}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("scr_init", "initial scr config", (console_cmd)&scr_init)
STATIC_COMMAND_END(scr_init);

#endif

APP_START(scr_init)
.flags = 0
APP_END
