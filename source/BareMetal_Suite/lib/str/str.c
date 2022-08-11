#include "str.h"
#ifdef SUPPORT_STR_MODE
#include "soc.h"
#include "scr.h"
#include "helper.h"
#include <debug.h>
#include "lib/sdrv_common_reg.h"
#include "DWC_ddr_umctl2_reg.h"
#include "dwc_ddrphy_top_reg.h"
//#include "storage_device.h"
//#include "storage_dev_ospi.h"
#include "partition_parser.h"
#include "ddr_init_helper.h"
#include <compiler.h>
#define STR_PT_CONFIGS                             \
        PT_LOAD_CONFIG_ITEM(0, 0, 0, ssystem)        \
        PT_LOAD_CONFIG_ITEM(0, 0, 0, hsm_fw)         \
        PT_LOAD_CONFIG_ITEM(SYS_CFG_MEMBASE, SYS_CFG_MEMSIZE, PT_LD_DECD, system_config)  \
        PT_LOAD_CONFIG_ITEM(SAFETY_MEMBASE, SAFETY_MEMSIZE, PT_LD_DECD, safety_os)         \
        PT_LOAD_CONFIG_ITEM(DIL2_MEMBASE, DIL2_MEMSIZE, PT_LD_DECD, dil2)         \
        PT_LOAD_CONFIG_ITEM(VBMETA_MEMBASE, VBMETA_MEMSIZE, PT_LD_DECD, vbmeta)         \

#define PT_LOAD_STR_CONFIGS(name) \
        PT_LOAD_CONFIGS_START(name) \
        STR_PT_CONFIGS                     \
        PT_LOAD_CONFIGS_END

#define STR_PT_CONFIGS_CNT  (sizeof(str_pt_configs) / sizeof(str_pt_configs[0]))
PT_LOAD_STR_CONFIGS(str_pt_configs);
static bool str_flag;
struct pt_load_config *str_get_pt_configs(void)
{
    return str_pt_configs;
}
uint32_t str_get_pt_configs_cnt(void)
{
    return STR_PT_CONFIGS_CNT;
}
bool is_str_enter(void)
{
    uint32_t bootreason = readl(STR_RTC_FLAG);

    if (bootreason == 0x12345678)
        return true;

    return false;
}
bool is_str_resume(void)
{
    return str_flag;
}
void set_str_resume(enum STR_MASTER master, bool flag)
{
    uint32_t base;

    switch (master) {
        case STR_AP1:
            str_flag = flag;
            writel(0, STR_RTC_FLAG);//only ap1 set flag
            base = STR_AP1_TO_SAF;
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return;
    }

    if (flag) {
        writel(0x8765, base);
    }
    else {
        writel(0, base);
        writel(0, base + 4);
    }
}
void str_save_ddr_freq(uint32_t freq)
{
    uint32_t v = readl(STR_SAVED_FREQ);
    v &= 0xffff;
    v |= (freq << 16);
    writel(v, STR_SAVED_FREQ);
}
#undef DDR_W32
#undef DDR_W32_BITS
#undef DDR_POLL_BITS
#define DDR_W32(a, v)   writel(v, a)
#define DDR_R32(a)
#define DDR_W32_BITS(a, shift, w, v)   RMWREG32(a, shift, w, v)
#define DDR_POLL_BITS(a, msk, v, tmt)   \
    do {\
        uint32_t cnt = 0;\
        while((cnt++ < (uint32_t)(tmt)) && ((readl(a) & (msk)) != ((v) & (msk))));\
    } while(0)


#define REG_AP_APB_DWC_DDR_UMCTL2_DBG1  (APB_DDRCTRL_BASE+UMCTL2_REGS_DBG1_OFF)
#define REG_AP_APB_DWC_DDR_UMCTL2_MSTR  (APB_DDRCTRL_BASE+UMCTL2_REGS_MSTR_OFF)
#define REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2 (APB_DDRCTRL_BASE+UMCTL2_REGS_DRAMTMG2_OFF)
#define DRAMTMG2_RD2WR_FIELD_OFFSET 8
#define DRAMTMG2_RD2WR_FIELD_SIZE 6
#define DRAMTMG2_WR2RD_FIELD_OFFSET 0
#define DRAMTMG2_WR2RD_FIELD_SIZE 6
#define REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL   (APB_DDRCTRL_BASE+UMCTL2_REGS_RANKCTL_OFF)
#define RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_OFFSET 26
#define RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_SIZE 1
#define RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_OFFSET 24
#define RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_SIZE 1
#define RANKCTL_DIFF_RANK_WR_GAP_FIELD_OFFSET 8
#define RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE 4
#define RANKCTL_DIFF_RANK_RD_GAP_FIELD_OFFSET 4
#define RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE 4
#define REG_AP_APB_DWC_DDR_UMCTL2_DFITMG1   (APB_DDRCTRL_BASE+UMCTL2_REGS_DFITMG1_OFF)
#define DFITMG1_DFI_T_WRDATA_DELAY_FIELD_OFFSET 16
#define DFITMG1_DFI_T_WRDATA_DELAY_FIELD_SIZE 5
#define DFITMG1_DFI_T_WRDATA_DELAY_FIELD_OFFSET 16
#define DFITMG1_DFI_T_WRDATA_DELAY_FIELD_SIZE 5
#define REG_AP_APB_DWC_DDR_UMCTL2_DFITMG1   (APB_DDRCTRL_BASE+UMCTL2_REGS_DFITMG1_OFF)
#define REG_AP_APB_DWC_DDR_UMCTL2_DBG1  (APB_DDRCTRL_BASE+UMCTL2_REGS_DBG1_OFF)

struct str_info_t {
    uint32_t freq;
    uint32_t reserved[8];
    uint32_t save_csr_index;
    uint32_t restore_csr_index;
    uint32_t save_c_index;
    uint32_t save_i_index;
    uint32_t ddrc_csr[100];
    uint32_t phy_csr[970];
};
static char str_data[9 * DEFAULT_BLK_SZ] __ALIGNED(DEFAULT_BLK_SZ) = {0};
struct str_info_t *str_info = NULL;
// scr_sec, DDR_SS_PWROKIN_AON, [768:768], scr_num - 24, bits - 1
static inline void set_scr_sec_ddr_ss_pwrokin_aon(unsigned int data0)
{
    unsigned int rdata = 0;
    // scr_sec[768:768], start_bit - 0, end_bit - 0, offset - 0x60, data0[0:0]
    rdata = readl(APB_SCR_SAF_BASE + (0x34 << 10));
    writel((rdata & (~(1 << 0))) | ((data0 & 0x1) << 0),
           APB_SCR_SAF_BASE + (0x34 << 10));
}

static inline void ddrc_wr(uint32_t addr, uint32_t v)
{
    writel(v, addr);
}
static void dwc_ddrphy_apb_wr(int phy_addr, int phy_data)
{
    int w_addr = APB_DDRPHY_BASE + phy_addr * 4;
    int w_data = phy_data;
    //printf("WR: phy_addr=%0x, w_data=%0x\n", phy_addr, w_data);
    writel(w_data, w_addr);
}
/*
static void ddrc_timing_reg_restore(void)
{
    //int ddrc_csr_0 =readl(DDRC_CSR_BASE);
    //int ddrc_csr_1 =readl(DDRC_CSR_BASE+4);
    //int rdata;
    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000001);

    writel(str_info->ddrc_csr[59], REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2);
    INFO("Restore DRAMTMG2, data = 0x%x\n", str_info->ddrc_csr[60]);

    writel(str_info->ddrc_csr[56], REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL);
    INFO("Restore RANKCTL, data = 0x%x\n", str_info->ddrc_csr[57]);

    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000000);
}
*/
static void dwc_ddrphy_apb_wr_for_io_retention(int phy_addr, int phy_data)
{
    int w_addr = APB_DDRPHY_BASE + phy_addr * 4;
    int w_data = str_info->phy_csr[str_info->restore_csr_index];
    writel(w_data, w_addr);
    str_info->restore_csr_index++;
}
uint32_t c_index = 0;
static void dwc_ddrc_csr_wr_for_io_retention(int phy_addr, int val)
{
    int w_addr = APB_DDRCTRL_BASE + phy_addr;
    int w_data = str_info->ddrc_csr[c_index];
    writel(w_data, w_addr);
    c_index++;
}
static void restore_ddrc_csr()
{
    //4266
    c_index = 0;
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DBG1_OFF, 0x1);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_PWRCTL_OFF, 0x1);
    DDR_R32(APB_DDRCTRL_BASE + UMCTL2_REGS_STAT_OFF);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_MRCTRL0_OFF, 0x40003030);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_MRCTRL1_OFF, 0x370d5);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_MRCTRL2_OFF, 0xeb74135c);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_MSTR2_OFF, 0x1);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DERATECTL_OFF, 0x0);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_PWRCTL_OFF, 0x20);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_HWLPCTL_OFF, 0xac0002);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ECCCFG0_OFF, 0x53f7f50);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ECCCFG1_OFF, 0x7b2);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ECCCTL_OFF, 0x300);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ECCPOISONADDR0_OFF,
                                     0x1000fc0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ECCPOISONADDR1_OFF,
                                     0x20019294);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_CRCPARCTL0_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_CRCPARCTL1_OFF, 0x1000);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DIMMCTL_OFF, 0x20);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFIMISC_OFF, 0x41);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP0_OFF, 0x18);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP1_OFF, 0x50505);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP2_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP3_OFF, 0x3030300);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP4_OFF, 0x1f1f);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP5_OFF, 0x70f0707);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP6_OFF, 0x7070707);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP7_OFF, 0xf07);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP8_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP9_OFF, 0x7070707);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP10_OFF, 0x7070707);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADDRMAP11_OFF, 0x7);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ODTMAP_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_SCHED_OFF, 0x8b9f00);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_SCHED1_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_PERFHPR1_OFF, 0xf00003f);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_PERFLPR1_OFF, 0xf0003ff);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_PERFWR1_OFF, 0xf0003ff);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DBG0_OFF, 0x0);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DBG1_OFF, 0x0);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DBGCMD_OFF, 0x0);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_SWCTL_OFF, 0x1);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_SWCTLSTATIC_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_OCPARCFG1_OFF, 0x844);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_POISONCFG_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ADVECCINDEX_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ECCPOISONPAT0_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ECCPOISONPAT2_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_REGPARCFG_OFF, 0x3);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_OCCAPCFG_OFF, 0x0);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_OCCAPCFG1_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_MP_PCTRL_0_OFF, 0x1);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_MSTR_OFF, 0x83080020);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DERATEEN_OFF, 0x1415);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DERATEINT_OFF, 0x439b2345);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_PWRTMG_OFF, 0xe1102);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_RFSHCTL0_OFF, 0x218000);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_RFSHCTL1_OFF, 0x19000a);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_RFSHCTL3_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_RFSHTMG_OFF, 0x82012c);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_RFSHTMG1_OFF, 0x610000);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_INIT0_OFF, 0xc0030828);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_INIT1_OFF, 0xd1000a);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_INIT2_OFF, 0x8d05);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_INIT3_OFF, 0x74003f);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_INIT4_OFF, 0xf30008);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_INIT5_OFF, 0x30007);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_INIT6_OFF, 0x64004d);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_INIT7_OFF, 0x40000);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_RANKCTL_OFF, 0xe32f);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG0_OFF, 0x2121482d);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG1_OFF, 0x90941);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG2_OFF, 0x9141c1d);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG3_OFF, 0xf0f006);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG4_OFF, 0x14040914);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG5_OFF, 0x2061111);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG6_OFF, 0x20b000a);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG7_OFF, 0x602);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG8_OFF, 0x1014501);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG9_OFF, 0x21);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG10_OFF, 0xe0007);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG11_OFF, 0x7f01001b);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG12_OFF, 0x20000);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG13_OFF, 0xe100002);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG14_OFF, 0x4e1);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DRAMTMG15_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ZQCTL0_OFF, 0x542d0021);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ZQCTL1_OFF, 0x3600070);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ZQCTL2_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_ODTCFG_OFF, 0x6070e74);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFITMG0_OFF, 0x4a3820e);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFITMG1_OFF, 0xb0303);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFILPCFG0_OFF, 0x3c0a020);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFILPCFG1_OFF, 0x70);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFIUPD0_OFF, 0x60400018);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFIUPD1_OFF, 0x8000b2);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFIUPD2_OFF, 0x80000000);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFITMG2_OFF, 0x230e);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFITMG3_OFF, 0x17);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DBICTL_OFF, 0x7);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_DFIPHYMSTR_OFF, 0x1);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_REGS_OCPARCFG0_OFF, 0xb02003);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_MP_PCCFG_OFF, 0x0);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_MP_PCFGR_0_OFF, 0x2b7);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_MP_PCFGW_0_OFF, 0x40bb);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_OCCAPCFG_OFF, 0x10001);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_OCCAPCFG1_OFF, 0x10001);
    dwc_ddrc_csr_wr_for_io_retention(UMCTL2_MP_PCFGQOS0_0_OFF, 0x1110e00);
    DDR_R32(APB_DDRCTRL_BASE + UMCTL2_REGS_RFSHCTL3_OFF);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DBG1_OFF, 0x0);
    DDR_R32(APB_DDRCTRL_BASE + UMCTL2_REGS_PWRCTL_OFF);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_PWRCTL_OFF, 0x20);
    DDR_R32(APB_DDRCTRL_BASE + UMCTL2_REGS_PWRCTL_OFF);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_PWRCTL_OFF, 0x20);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_SWCTL_OFF, 0x0);
    DDR_POLL_BITS(APB_DDRCTRL_BASE + UMCTL2_REGS_SWSTAT_OFF, 0x1, 0x0, 0x64);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DFIMISC_OFF, 0x40);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DFIMISC_OFF, 0x40);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DFIMISC_OFF, 0x40);
//ddrc_timing_reg_restore();
    soc_assert_reset(DDR_SS);
}
void dwc_ddrphy_phyinit_retention_sequence(void)
{
    dwc_ddrphy_apb_wr(0xd0000, 0x0); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
    dwc_ddrphy_apb_wr(0xc0080, 0x3); // DWC_DDRPHYA_DRTUB0_UcclkHclkEnables
    dwc_ddrphy_apb_wr(0x20089, 0x1); // DWC_DDRPHYA_MASTER0_CalZap
//  Issue register writes to restore registers values.
//  --------------------------------------------------
//  - IMPORTANT -
//  - The register values printed in this txt file are always 0x0. The
//  - user must replace them with actual registers values from the save sequence. The
//  - save sequence can be found in the output.txt file associated with the particular init sequence.
//  - The save sequence issues APB read commands to read and save register values.
//  - refer to the init sequence output file for details.
//  --------------------------------------------------------------------------------
    dwc_ddrphy_apb_wr_for_io_retention(0x1005f,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxSlewRate_b0_p0
//printf("HH3\n");
    dwc_ddrphy_apb_wr_for_io_retention(0x1015f,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxSlewRate_b1_p0
//printf("HH31\n");
    dwc_ddrphy_apb_wr_for_io_retention(0x1105f,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxSlewRate_b0_p0
//printf("HH32\n");
    dwc_ddrphy_apb_wr_for_io_retention(0x1115f,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxSlewRate_b1_p0
//printf("HH33\n");
    dwc_ddrphy_apb_wr_for_io_retention(0x1205f,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxSlewRate_b0_p0
//printf("HH34\n");
    dwc_ddrphy_apb_wr_for_io_retention(0x1215f,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxSlewRate_b1_p0
//printf("HH35\n");
    dwc_ddrphy_apb_wr_for_io_retention(0x1305f,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxSlewRate_b0_p0
//printf("HH36\n");
    dwc_ddrphy_apb_wr_for_io_retention(0x1315f,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxSlewRate_b1_p0
//printf("HH37\n");
    dwc_ddrphy_apb_wr_for_io_retention(0x55,
                                       0x0); // DWC_DDRPHYA_ANIB0_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x1055,
                                       0x0); // DWC_DDRPHYA_ANIB1_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x2055,
                                       0x0); // DWC_DDRPHYA_ANIB2_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x3055,
                                       0x0); // DWC_DDRPHYA_ANIB3_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x4055,
                                       0x0); // DWC_DDRPHYA_ANIB4_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x5055,
                                       0x0); // DWC_DDRPHYA_ANIB5_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x6055,
                                       0x0); // DWC_DDRPHYA_ANIB6_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x7055,
                                       0x0); // DWC_DDRPHYA_ANIB7_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x8055,
                                       0x0); // DWC_DDRPHYA_ANIB8_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x9055,
                                       0x0); // DWC_DDRPHYA_ANIB9_ATxSlewRate
    dwc_ddrphy_apb_wr_for_io_retention(0x200c5,
                                       0x0); // DWC_DDRPHYA_MASTER0_PllCtrl2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x2002e,
                                       0x0); // DWC_DDRPHYA_MASTER0_ARdPtrInitVal_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x90204,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BGPR4_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x20024,
                                       0x0); // DWC_DDRPHYA_MASTER0_DqsPreambleControl_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x2003a,
                                       0x0); // DWC_DDRPHYA_MASTER0_DbyteDllModeCntrl
    dwc_ddrphy_apb_wr_for_io_retention(0x2007d,
                                       0x0); // DWC_DDRPHYA_MASTER0_DllLockParam_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x2007c,
                                       0x0); // DWC_DDRPHYA_MASTER0_DllGainCtl_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x20056,
                                       0x0); // DWC_DDRPHYA_MASTER0_ProcOdtTimeCtl_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1004d,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1014d,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1104d,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1114d,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1204d,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1214d,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1304d,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1314d,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x10049,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x10149,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x11049,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x11149,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x12049,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x12149,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x13049,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x13149,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x43,
                                       0x0); // DWC_DDRPHYA_ANIB0_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x1043,
                                       0x0); // DWC_DDRPHYA_ANIB1_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x2043,
                                       0x0); // DWC_DDRPHYA_ANIB2_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x3043,
                                       0x0); // DWC_DDRPHYA_ANIB3_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x4043,
                                       0x0); // DWC_DDRPHYA_ANIB4_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x5043,
                                       0x0); // DWC_DDRPHYA_ANIB5_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x6043,
                                       0x0); // DWC_DDRPHYA_ANIB6_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x7043,
                                       0x0); // DWC_DDRPHYA_ANIB7_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x8043,
                                       0x0); // DWC_DDRPHYA_ANIB8_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x9043,
                                       0x0); // DWC_DDRPHYA_ANIB9_ATxImpedance
    dwc_ddrphy_apb_wr_for_io_retention(0x20018,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiMode
    dwc_ddrphy_apb_wr_for_io_retention(0x20075,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiCAMode
    dwc_ddrphy_apb_wr_for_io_retention(0x20050,
                                       0x0); // DWC_DDRPHYA_MASTER0_CalDrvStr0
    dwc_ddrphy_apb_wr_for_io_retention(0x2009b,
                                       0x0); // DWC_DDRPHYA_MASTER0_CalVRefs
    dwc_ddrphy_apb_wr_for_io_retention(0x20008,
                                       0x0); // DWC_DDRPHYA_MASTER0_CalUclkInfo_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x20088,
                                       0x0); // DWC_DDRPHYA_MASTER0_CalRate
    dwc_ddrphy_apb_wr_for_io_retention(0x200b2,
                                       0x0); // DWC_DDRPHYA_MASTER0_VrefInGlobal_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x10043,
                                       0x0); // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x10143,
                                       0x0); // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x11043,
                                       0x0); // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x11143,
                                       0x0); // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x12043,
                                       0x0); // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x12143,
                                       0x0); // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x13043,
                                       0x0); // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x13143,
                                       0x0); // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x200fa,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiFreqRatio_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x20019,
                                       0x0); // DWC_DDRPHYA_MASTER0_TristateModeCA_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x200f0,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiFreqXlat0
    dwc_ddrphy_apb_wr_for_io_retention(0x200f1,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiFreqXlat1
    dwc_ddrphy_apb_wr_for_io_retention(0x200f2,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiFreqXlat2
    dwc_ddrphy_apb_wr_for_io_retention(0x200f3,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiFreqXlat3
    dwc_ddrphy_apb_wr_for_io_retention(0x200f4,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiFreqXlat4
    dwc_ddrphy_apb_wr_for_io_retention(0x200f5,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiFreqXlat5
    dwc_ddrphy_apb_wr_for_io_retention(0x200f6,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiFreqXlat6
    dwc_ddrphy_apb_wr_for_io_retention(0x200f7,
                                       0x0); // DWC_DDRPHYA_MASTER0_DfiFreqXlat7
    dwc_ddrphy_apb_wr_for_io_retention(0x1004a,
                                       0x0); // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl1
    dwc_ddrphy_apb_wr_for_io_retention(0x1104a,
                                       0x0); // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl1
    dwc_ddrphy_apb_wr_for_io_retention(0x1204a,
                                       0x0); // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl1
    dwc_ddrphy_apb_wr_for_io_retention(0x1304a,
                                       0x0); // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl1
    dwc_ddrphy_apb_wr_for_io_retention(0x20025,
                                       0x0); // DWC_DDRPHYA_MASTER0_MasterX4Config
    dwc_ddrphy_apb_wr_for_io_retention(0x2002d,
                                       0x0); // DWC_DDRPHYA_MASTER0_DMIPinPresent_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x2002c,
                                       0x0); // DWC_DDRPHYA_MASTER0_Acx4AnibDis
    dwc_ddrphy_apb_wr_for_io_retention(0xd0000,
                                       0x0); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
    dwc_ddrphy_apb_wr_for_io_retention(0x90000,
                                       0x0); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90001,
                                       0x0); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90002,
                                       0x0); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90003,
                                       0x0); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90004,
                                       0x0); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90005,
                                       0x0); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90029,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b0s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9002a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b0s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9002b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b0s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9002c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b1s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9002d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b1s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9002e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b1s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9002f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b2s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90030,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b2s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90031,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b2s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90032,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b3s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90033,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b3s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90034,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b3s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90035,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b4s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90036,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b4s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90037,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b4s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90038,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b5s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90039,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b5s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9003a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b5s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9003b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b6s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9003c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b6s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9003d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b6s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9003e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b7s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9003f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b7s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90040,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b7s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90041,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b8s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90042,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b8s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90043,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b8s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90044,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b9s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90045,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b9s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90046,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b9s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90047,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b10s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90048,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b10s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90049,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b10s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9004a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b11s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9004b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b11s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9004c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b11s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9004d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b12s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9004e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b12s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9004f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b12s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90050,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b13s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90051,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b13s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90052,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b13s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90053,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b14s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90054,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b14s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90055,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b14s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90056,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b15s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90057,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b15s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90058,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b15s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90059,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b16s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9005a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b16s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9005b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b16s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9005c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b17s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9005d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b17s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9005e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b17s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9005f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b18s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90060,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b18s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90061,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b18s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90062,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b19s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90063,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b19s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90064,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b19s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90065,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b20s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90066,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b20s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90067,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b20s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90068,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b21s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90069,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b21s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9006a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b21s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9006b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b22s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9006c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b22s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9006d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b22s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9006e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b23s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9006f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b23s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90070,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b23s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90071,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b24s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90072,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b24s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90073,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b24s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90074,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b25s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90075,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b25s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90076,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b25s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90077,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b26s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90078,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b26s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90079,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b26s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9007a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b27s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9007b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b27s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9007c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b27s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9007d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b28s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9007e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b28s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9007f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b28s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90080,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b29s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90081,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b29s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90082,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b29s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90083,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b30s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90084,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b30s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90085,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b30s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90086,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b31s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90087,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b31s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90088,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b31s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90089,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b32s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9008a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b32s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9008b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b32s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9008c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b33s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9008d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b33s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9008e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b33s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9008f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b34s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90090,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b34s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90091,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b34s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90092,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b35s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90093,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b35s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90094,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b35s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90095,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b36s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90096,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b36s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90097,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b36s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90098,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b37s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90099,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b37s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9009a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b37s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9009b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b38s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9009c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b38s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9009d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b38s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9009e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b39s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9009f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b39s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900a0,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b39s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900a1,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b40s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900a2,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b40s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900a3,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b40s2
    dwc_ddrphy_apb_wr_for_io_retention(0x40000,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x0
    dwc_ddrphy_apb_wr_for_io_retention(0x40020,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x0
    dwc_ddrphy_apb_wr_for_io_retention(0x40040,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x0
    dwc_ddrphy_apb_wr_for_io_retention(0x40060,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x0
    dwc_ddrphy_apb_wr_for_io_retention(0x40001,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x1
    dwc_ddrphy_apb_wr_for_io_retention(0x40021,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x1
    dwc_ddrphy_apb_wr_for_io_retention(0x40041,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x1
    dwc_ddrphy_apb_wr_for_io_retention(0x40061,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x1
    dwc_ddrphy_apb_wr_for_io_retention(0x40002,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x2
    dwc_ddrphy_apb_wr_for_io_retention(0x40022,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x2
    dwc_ddrphy_apb_wr_for_io_retention(0x40042,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x2
    dwc_ddrphy_apb_wr_for_io_retention(0x40062,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x2
    dwc_ddrphy_apb_wr_for_io_retention(0x40003,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x3
    dwc_ddrphy_apb_wr_for_io_retention(0x40023,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x3
    dwc_ddrphy_apb_wr_for_io_retention(0x40043,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x3
    dwc_ddrphy_apb_wr_for_io_retention(0x40063,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x3
    dwc_ddrphy_apb_wr_for_io_retention(0x40004,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x4
    dwc_ddrphy_apb_wr_for_io_retention(0x40024,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x4
    dwc_ddrphy_apb_wr_for_io_retention(0x40044,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x4
    dwc_ddrphy_apb_wr_for_io_retention(0x40064,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x4
    dwc_ddrphy_apb_wr_for_io_retention(0x40005,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x5
    dwc_ddrphy_apb_wr_for_io_retention(0x40025,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x5
    dwc_ddrphy_apb_wr_for_io_retention(0x40045,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x5
    dwc_ddrphy_apb_wr_for_io_retention(0x40065,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x5
    dwc_ddrphy_apb_wr_for_io_retention(0x40006,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x6
    dwc_ddrphy_apb_wr_for_io_retention(0x40026,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x6
    dwc_ddrphy_apb_wr_for_io_retention(0x40046,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x6
    dwc_ddrphy_apb_wr_for_io_retention(0x40066,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x6
    dwc_ddrphy_apb_wr_for_io_retention(0x40007,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x7
    dwc_ddrphy_apb_wr_for_io_retention(0x40027,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x7
    dwc_ddrphy_apb_wr_for_io_retention(0x40047,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x7
    dwc_ddrphy_apb_wr_for_io_retention(0x40067,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x7
    dwc_ddrphy_apb_wr_for_io_retention(0x40008,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x8
    dwc_ddrphy_apb_wr_for_io_retention(0x40028,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x8
    dwc_ddrphy_apb_wr_for_io_retention(0x40048,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x8
    dwc_ddrphy_apb_wr_for_io_retention(0x40068,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x8
    dwc_ddrphy_apb_wr_for_io_retention(0x40009,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x9
    dwc_ddrphy_apb_wr_for_io_retention(0x40029,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x9
    dwc_ddrphy_apb_wr_for_io_retention(0x40049,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x9
    dwc_ddrphy_apb_wr_for_io_retention(0x40069,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x9
    dwc_ddrphy_apb_wr_for_io_retention(0x4000a,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x10
    dwc_ddrphy_apb_wr_for_io_retention(0x4002a,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x10
    dwc_ddrphy_apb_wr_for_io_retention(0x4004a,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x10
    dwc_ddrphy_apb_wr_for_io_retention(0x4006a,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x10
    dwc_ddrphy_apb_wr_for_io_retention(0x4000b,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x11
    dwc_ddrphy_apb_wr_for_io_retention(0x4002b,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x11
    dwc_ddrphy_apb_wr_for_io_retention(0x4004b,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x11
    dwc_ddrphy_apb_wr_for_io_retention(0x4006b,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x11
    dwc_ddrphy_apb_wr_for_io_retention(0x4000c,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x12
    dwc_ddrphy_apb_wr_for_io_retention(0x4002c,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x12
    dwc_ddrphy_apb_wr_for_io_retention(0x4004c,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x12
    dwc_ddrphy_apb_wr_for_io_retention(0x4006c,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x12
    dwc_ddrphy_apb_wr_for_io_retention(0x4000d,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x13
    dwc_ddrphy_apb_wr_for_io_retention(0x4002d,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x13
    dwc_ddrphy_apb_wr_for_io_retention(0x4004d,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x13
    dwc_ddrphy_apb_wr_for_io_retention(0x4006d,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x13
    dwc_ddrphy_apb_wr_for_io_retention(0x4000e,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x14
    dwc_ddrphy_apb_wr_for_io_retention(0x4002e,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x14
    dwc_ddrphy_apb_wr_for_io_retention(0x4004e,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x14
    dwc_ddrphy_apb_wr_for_io_retention(0x4006e,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x14
    dwc_ddrphy_apb_wr_for_io_retention(0x4000f,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x15
    dwc_ddrphy_apb_wr_for_io_retention(0x4002f,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x15
    dwc_ddrphy_apb_wr_for_io_retention(0x4004f,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x15
    dwc_ddrphy_apb_wr_for_io_retention(0x4006f,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x15
    dwc_ddrphy_apb_wr_for_io_retention(0x40010,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x16
    dwc_ddrphy_apb_wr_for_io_retention(0x40030,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x16
    dwc_ddrphy_apb_wr_for_io_retention(0x40050,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x16
    dwc_ddrphy_apb_wr_for_io_retention(0x40070,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x16
    dwc_ddrphy_apb_wr_for_io_retention(0x40011,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x17
    dwc_ddrphy_apb_wr_for_io_retention(0x40031,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x17
    dwc_ddrphy_apb_wr_for_io_retention(0x40051,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x17
    dwc_ddrphy_apb_wr_for_io_retention(0x40071,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x17
    dwc_ddrphy_apb_wr_for_io_retention(0x40012,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x18
    dwc_ddrphy_apb_wr_for_io_retention(0x40032,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x18
    dwc_ddrphy_apb_wr_for_io_retention(0x40052,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x18
    dwc_ddrphy_apb_wr_for_io_retention(0x40072,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x18
    dwc_ddrphy_apb_wr_for_io_retention(0x40013,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x19
    dwc_ddrphy_apb_wr_for_io_retention(0x40033,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x19
    dwc_ddrphy_apb_wr_for_io_retention(0x40053,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x19
    dwc_ddrphy_apb_wr_for_io_retention(0x40073,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x19
    dwc_ddrphy_apb_wr_for_io_retention(0x40014,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x20
    dwc_ddrphy_apb_wr_for_io_retention(0x40034,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x20
    dwc_ddrphy_apb_wr_for_io_retention(0x40054,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x20
    dwc_ddrphy_apb_wr_for_io_retention(0x40074,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x20
    dwc_ddrphy_apb_wr_for_io_retention(0x40015,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x21
    dwc_ddrphy_apb_wr_for_io_retention(0x40035,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x21
    dwc_ddrphy_apb_wr_for_io_retention(0x40055,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x21
    dwc_ddrphy_apb_wr_for_io_retention(0x40075,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x21
    dwc_ddrphy_apb_wr_for_io_retention(0x40016,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x22
    dwc_ddrphy_apb_wr_for_io_retention(0x40036,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x22
    dwc_ddrphy_apb_wr_for_io_retention(0x40056,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x22
    dwc_ddrphy_apb_wr_for_io_retention(0x40076,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x22
    dwc_ddrphy_apb_wr_for_io_retention(0x40017,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x23
    dwc_ddrphy_apb_wr_for_io_retention(0x40037,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x23
    dwc_ddrphy_apb_wr_for_io_retention(0x40057,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x23
    dwc_ddrphy_apb_wr_for_io_retention(0x40077,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x23
    dwc_ddrphy_apb_wr_for_io_retention(0x40018,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x24
    dwc_ddrphy_apb_wr_for_io_retention(0x40038,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x24
    dwc_ddrphy_apb_wr_for_io_retention(0x40058,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x24
    dwc_ddrphy_apb_wr_for_io_retention(0x40078,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x24
    dwc_ddrphy_apb_wr_for_io_retention(0x40019,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x25
    dwc_ddrphy_apb_wr_for_io_retention(0x40039,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x25
    dwc_ddrphy_apb_wr_for_io_retention(0x40059,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x25
    dwc_ddrphy_apb_wr_for_io_retention(0x40079,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x25
    dwc_ddrphy_apb_wr_for_io_retention(0x4001a,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq0x26
    dwc_ddrphy_apb_wr_for_io_retention(0x4003a,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq1x26
    dwc_ddrphy_apb_wr_for_io_retention(0x4005a,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq2x26
    dwc_ddrphy_apb_wr_for_io_retention(0x4007a,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmSeq3x26
    dwc_ddrphy_apb_wr_for_io_retention(0x900a4,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b41s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900a5,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b41s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900a6,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b41s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900a7,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b42s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900a8,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b42s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900a9,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b42s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900aa,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b43s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900ab,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b43s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900ac,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b43s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900ad,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b44s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900ae,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b44s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900af,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b44s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900b0,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b45s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900b1,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b45s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900b2,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b45s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900b3,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b46s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900b4,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b46s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900b5,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b46s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900b6,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b47s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900b7,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b47s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900b8,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b47s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900b9,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b48s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900ba,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b48s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900bb,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b48s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900bc,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b49s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900bd,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b49s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900be,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b49s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900bf,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b50s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900c0,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b50s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900c1,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b50s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900c2,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b51s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900c3,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b51s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900c4,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b51s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900c5,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b52s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900c6,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b52s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900c7,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b52s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900c8,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b53s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900c9,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b53s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900ca,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b53s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900cb,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b54s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900cc,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b54s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900cd,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b54s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900ce,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b55s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900cf,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b55s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900d0,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b55s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900d1,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b56s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900d2,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b56s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900d3,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b56s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900d4,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b57s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900d5,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b57s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900d6,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b57s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900d7,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b58s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900d8,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b58s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900d9,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b58s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900da,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b59s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900db,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b59s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900dc,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b59s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900dd,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b60s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900de,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b60s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900df,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b60s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900e0,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b61s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900e1,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b61s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900e2,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b61s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900e3,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b62s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900e4,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b62s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900e5,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b62s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900e6,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b63s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900e7,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b63s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900e8,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b63s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900e9,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b64s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900ea,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b64s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900eb,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b64s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900ec,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b65s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900ed,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b65s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900ee,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b65s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900ef,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b66s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900f0,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b66s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900f1,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b66s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900f2,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b67s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900f3,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b67s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900f4,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b67s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900f5,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b68s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900f6,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b68s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900f7,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b68s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900f8,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b69s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900f9,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b69s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900fa,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b69s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900fb,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b70s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900fc,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b70s1
    dwc_ddrphy_apb_wr_for_io_retention(0x900fd,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b70s2
    dwc_ddrphy_apb_wr_for_io_retention(0x900fe,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b71s0
    dwc_ddrphy_apb_wr_for_io_retention(0x900ff,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b71s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90100,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b71s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90101,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b72s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90102,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b72s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90103,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b72s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90104,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b73s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90105,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b73s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90106,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b73s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90107,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b74s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90108,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b74s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90109,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b74s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9010a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b75s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9010b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b75s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9010c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b75s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9010d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b76s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9010e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b76s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9010f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b76s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90110,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b77s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90111,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b77s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90112,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b77s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90113,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b78s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90114,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b78s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90115,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b78s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90116,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b79s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90117,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b79s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90118,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b79s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90119,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b80s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9011a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b80s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9011b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b80s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9011c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b81s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9011d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b81s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9011e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b81s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9011f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b82s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90120,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b82s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90121,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b82s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90122,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b83s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90123,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b83s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90124,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b83s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90125,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b84s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90126,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b84s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90127,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b84s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90128,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b85s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90129,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b85s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9012a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b85s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9012b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b86s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9012c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b86s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9012d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b86s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9012e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b87s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9012f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b87s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90130,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b87s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90131,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b88s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90132,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b88s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90133,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b88s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90134,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b89s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90135,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b89s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90136,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b89s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90137,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b90s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90138,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b90s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90139,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b90s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9013a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b91s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9013b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b91s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9013c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b91s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9013d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b92s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9013e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b92s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9013f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b92s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90140,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b93s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90141,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b93s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90142,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b93s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90143,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b94s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90144,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b94s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90145,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b94s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90146,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b95s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90147,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b95s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90148,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b95s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90149,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b96s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9014a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b96s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9014b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b96s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9014c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b97s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9014d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b97s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9014e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b97s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9014f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b98s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90150,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b98s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90151,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b98s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90152,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b99s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90153,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b99s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90154,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b99s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90155,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b100s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90156,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b100s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90157,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b100s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90158,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b101s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90159,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b101s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9015a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b101s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9015b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b102s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9015c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b102s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9015d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b102s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9015e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b103s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9015f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b103s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90160,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b103s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90161,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b104s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90162,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b104s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90163,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b104s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90164,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b105s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90165,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b105s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90166,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b105s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90167,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b106s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90168,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b106s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90169,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b106s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9016a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b107s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9016b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b107s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9016c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b107s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9016d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b108s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9016e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b108s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9016f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b108s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90170,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b109s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90171,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b109s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90172,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b109s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90173,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b110s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90174,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b110s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90175,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b110s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90176,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b111s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90177,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b111s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90178,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b111s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90179,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b112s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9017a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b112s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9017b,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b112s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9017c,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b113s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9017d,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b113s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9017e,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b113s2
    dwc_ddrphy_apb_wr_for_io_retention(0x9017f,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b114s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90180,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b114s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90181,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b114s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90182,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b115s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90183,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b115s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90184,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b115s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90185,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b116s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90186,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b116s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90187,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b116s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90188,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b117s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90189,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b117s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9018a,
                                       0x0); // DWC_DDRPHYA_INITENG0_SequenceReg0b117s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90006,
                                       0x0); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s0
    dwc_ddrphy_apb_wr_for_io_retention(0x90007,
                                       0x0); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s1
    dwc_ddrphy_apb_wr_for_io_retention(0x90008,
                                       0x0); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s2
    dwc_ddrphy_apb_wr_for_io_retention(0x90009,
                                       0x0); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s0
    dwc_ddrphy_apb_wr_for_io_retention(0x9000a,
                                       0x0); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s1
    dwc_ddrphy_apb_wr_for_io_retention(0x9000b,
                                       0x0); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s2
    dwc_ddrphy_apb_wr_for_io_retention(0xd00e7,
                                       0x0); // DWC_DDRPHYA_APBONLY0_SequencerOverride
    dwc_ddrphy_apb_wr_for_io_retention(0x90017,
                                       0x0); // DWC_DDRPHYA_INITENG0_StartVector0b0
    dwc_ddrphy_apb_wr_for_io_retention(0x9001f,
                                       0x0); // DWC_DDRPHYA_INITENG0_StartVector0b8
    dwc_ddrphy_apb_wr_for_io_retention(0x90026,
                                       0x0); // DWC_DDRPHYA_INITENG0_StartVector0b15
    dwc_ddrphy_apb_wr_for_io_retention(0x400d0,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl0
    dwc_ddrphy_apb_wr_for_io_retention(0x400d1,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl1
    dwc_ddrphy_apb_wr_for_io_retention(0x400d2,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl2
    dwc_ddrphy_apb_wr_for_io_retention(0x400d3,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl3
    dwc_ddrphy_apb_wr_for_io_retention(0x400d4,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl4
    dwc_ddrphy_apb_wr_for_io_retention(0x400d5,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl5
    dwc_ddrphy_apb_wr_for_io_retention(0x400d6,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl6
    dwc_ddrphy_apb_wr_for_io_retention(0x400d7,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl7
    dwc_ddrphy_apb_wr_for_io_retention(0x2000b,
                                       0x0); // DWC_DDRPHYA_MASTER0_Seq0BDLY0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x2000c,
                                       0x0); // DWC_DDRPHYA_MASTER0_Seq0BDLY1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x2000d,
                                       0x0); // DWC_DDRPHYA_MASTER0_Seq0BDLY2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x2000e,
                                       0x0); // DWC_DDRPHYA_MASTER0_Seq0BDLY3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x9000c,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag0
    dwc_ddrphy_apb_wr_for_io_retention(0x9000d,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag1
    dwc_ddrphy_apb_wr_for_io_retention(0x9000e,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag2
    dwc_ddrphy_apb_wr_for_io_retention(0x9000f,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag3
    dwc_ddrphy_apb_wr_for_io_retention(0x90010,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag4
    dwc_ddrphy_apb_wr_for_io_retention(0x90011,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag5
    dwc_ddrphy_apb_wr_for_io_retention(0x90012,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag6
    dwc_ddrphy_apb_wr_for_io_retention(0x90013,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag7
    dwc_ddrphy_apb_wr_for_io_retention(0x20010,
                                       0x0); // DWC_DDRPHYA_MASTER0_PPTTrainSetup_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x20011,
                                       0x0); // DWC_DDRPHYA_MASTER0_PPTTrainSetup2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x40080,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmPlayback0x0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x40081,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmPlayback1x0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x40082,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmPlayback0x1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x40083,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmPlayback1x1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x40084,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmPlayback0x2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x40085,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmPlayback1x2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x400fd,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCtrl13
    dwc_ddrphy_apb_wr_for_io_retention(0x10011,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TsmByte1
    dwc_ddrphy_apb_wr_for_io_retention(0x10012,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TsmByte2
    dwc_ddrphy_apb_wr_for_io_retention(0x10013,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TsmByte3
    dwc_ddrphy_apb_wr_for_io_retention(0x10018,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TsmByte5
    dwc_ddrphy_apb_wr_for_io_retention(0x10002,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TrainingParam
    dwc_ddrphy_apb_wr_for_io_retention(0x100b2,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Tsm0_i0
    dwc_ddrphy_apb_wr_for_io_retention(0x101b4,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Tsm2_i1
    dwc_ddrphy_apb_wr_for_io_retention(0x102b4,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Tsm2_i2
    dwc_ddrphy_apb_wr_for_io_retention(0x103b4,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Tsm2_i3
    dwc_ddrphy_apb_wr_for_io_retention(0x104b4,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Tsm2_i4
    dwc_ddrphy_apb_wr_for_io_retention(0x105b4,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Tsm2_i5
    dwc_ddrphy_apb_wr_for_io_retention(0x106b4,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Tsm2_i6
    dwc_ddrphy_apb_wr_for_io_retention(0x107b4,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Tsm2_i7
    dwc_ddrphy_apb_wr_for_io_retention(0x108b4,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Tsm2_i8
    dwc_ddrphy_apb_wr_for_io_retention(0x11011,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TsmByte1
    dwc_ddrphy_apb_wr_for_io_retention(0x11012,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TsmByte2
    dwc_ddrphy_apb_wr_for_io_retention(0x11013,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TsmByte3
    dwc_ddrphy_apb_wr_for_io_retention(0x11018,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TsmByte5
    dwc_ddrphy_apb_wr_for_io_retention(0x11002,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TrainingParam
    dwc_ddrphy_apb_wr_for_io_retention(0x110b2,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Tsm0_i0
    dwc_ddrphy_apb_wr_for_io_retention(0x111b4,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Tsm2_i1
    dwc_ddrphy_apb_wr_for_io_retention(0x112b4,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Tsm2_i2
    dwc_ddrphy_apb_wr_for_io_retention(0x113b4,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Tsm2_i3
    dwc_ddrphy_apb_wr_for_io_retention(0x114b4,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Tsm2_i4
    dwc_ddrphy_apb_wr_for_io_retention(0x115b4,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Tsm2_i5
    dwc_ddrphy_apb_wr_for_io_retention(0x116b4,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Tsm2_i6
    dwc_ddrphy_apb_wr_for_io_retention(0x117b4,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Tsm2_i7
    dwc_ddrphy_apb_wr_for_io_retention(0x118b4,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Tsm2_i8
    dwc_ddrphy_apb_wr_for_io_retention(0x12011,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TsmByte1
    dwc_ddrphy_apb_wr_for_io_retention(0x12012,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TsmByte2
    dwc_ddrphy_apb_wr_for_io_retention(0x12013,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TsmByte3
    dwc_ddrphy_apb_wr_for_io_retention(0x12018,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TsmByte5
    dwc_ddrphy_apb_wr_for_io_retention(0x12002,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TrainingParam
    dwc_ddrphy_apb_wr_for_io_retention(0x120b2,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Tsm0_i0
    dwc_ddrphy_apb_wr_for_io_retention(0x121b4,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Tsm2_i1
    dwc_ddrphy_apb_wr_for_io_retention(0x122b4,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Tsm2_i2
    dwc_ddrphy_apb_wr_for_io_retention(0x123b4,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Tsm2_i3
    dwc_ddrphy_apb_wr_for_io_retention(0x124b4,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Tsm2_i4
    dwc_ddrphy_apb_wr_for_io_retention(0x125b4,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Tsm2_i5
    dwc_ddrphy_apb_wr_for_io_retention(0x126b4,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Tsm2_i6
    dwc_ddrphy_apb_wr_for_io_retention(0x127b4,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Tsm2_i7
    dwc_ddrphy_apb_wr_for_io_retention(0x128b4,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Tsm2_i8
    dwc_ddrphy_apb_wr_for_io_retention(0x13011,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TsmByte1
    dwc_ddrphy_apb_wr_for_io_retention(0x13012,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TsmByte2
    dwc_ddrphy_apb_wr_for_io_retention(0x13013,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TsmByte3
    dwc_ddrphy_apb_wr_for_io_retention(0x13018,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TsmByte5
    dwc_ddrphy_apb_wr_for_io_retention(0x13002,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TrainingParam
    dwc_ddrphy_apb_wr_for_io_retention(0x130b2,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Tsm0_i0
    dwc_ddrphy_apb_wr_for_io_retention(0x131b4,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Tsm2_i1
    dwc_ddrphy_apb_wr_for_io_retention(0x132b4,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Tsm2_i2
    dwc_ddrphy_apb_wr_for_io_retention(0x133b4,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Tsm2_i3
    dwc_ddrphy_apb_wr_for_io_retention(0x134b4,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Tsm2_i4
    dwc_ddrphy_apb_wr_for_io_retention(0x135b4,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Tsm2_i5
    dwc_ddrphy_apb_wr_for_io_retention(0x136b4,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Tsm2_i6
    dwc_ddrphy_apb_wr_for_io_retention(0x137b4,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Tsm2_i7
    dwc_ddrphy_apb_wr_for_io_retention(0x138b4,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Tsm2_i8
    dwc_ddrphy_apb_wr_for_io_retention(0x20089,
                                       0x0); // DWC_DDRPHYA_MASTER0_CalZap
    dwc_ddrphy_apb_wr_for_io_retention(0xc0080,
                                       0x0); // DWC_DDRPHYA_DRTUB0_UcclkHclkEnables
    dwc_ddrphy_apb_wr_for_io_retention(0x200cb,
                                       0x0); // DWC_DDRPHYA_MASTER0_PllCtrl3
    dwc_ddrphy_apb_wr_for_io_retention(0x10068,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x10069,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x10168,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x10169,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x10268,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x10269,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x10368,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x10369,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x10468,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x10469,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x10568,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x10569,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x10668,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x10669,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x10768,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x10769,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x10868,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x10869,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x100aa,
                                       0x0); // DWC_DDRPHYA_DBYTE0_PptCtlStatic
    dwc_ddrphy_apb_wr_for_io_retention(0x10062,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TrainingIncDecDtsmEn_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x10001,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TsmByte0
    dwc_ddrphy_apb_wr_for_io_retention(0x11068,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x11069,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x11168,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x11169,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x11268,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x11269,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x11368,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x11369,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x11468,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x11469,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x11568,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x11569,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x11668,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x11669,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x11768,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x11769,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x11868,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x11869,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x110aa,
                                       0x0); // DWC_DDRPHYA_DBYTE1_PptCtlStatic
    dwc_ddrphy_apb_wr_for_io_retention(0x11062,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TrainingIncDecDtsmEn_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x11001,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TsmByte0
    dwc_ddrphy_apb_wr_for_io_retention(0x12068,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x12069,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x12168,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x12169,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x12268,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x12269,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x12368,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x12369,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x12468,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x12469,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x12568,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x12569,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x12668,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x12669,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x12768,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x12769,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x12868,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x12869,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x120aa,
                                       0x0); // DWC_DDRPHYA_DBYTE2_PptCtlStatic
    dwc_ddrphy_apb_wr_for_io_retention(0x12062,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TrainingIncDecDtsmEn_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x12001,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TsmByte0
    dwc_ddrphy_apb_wr_for_io_retention(0x13068,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x13069,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x13168,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x13169,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x13268,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x13269,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x13368,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x13369,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x13468,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x13469,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x13568,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x13569,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x13668,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x13669,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x13768,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x13769,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x13868,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x13869,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x130aa,
                                       0x0); // DWC_DDRPHYA_DBYTE3_PptCtlStatic
    dwc_ddrphy_apb_wr_for_io_retention(0x13062,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TrainingIncDecDtsmEn_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x13001,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TsmByte0
    dwc_ddrphy_apb_wr_for_io_retention(0x80,
                                       0x0); // DWC_DDRPHYA_ANIB0_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1080,
                                       0x0); // DWC_DDRPHYA_ANIB1_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x2080,
                                       0x0); // DWC_DDRPHYA_ANIB2_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x3080,
                                       0x0); // DWC_DDRPHYA_ANIB3_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x4080,
                                       0x0); // DWC_DDRPHYA_ANIB4_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x5080,
                                       0x0); // DWC_DDRPHYA_ANIB5_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x6080,
                                       0x0); // DWC_DDRPHYA_ANIB6_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x7080,
                                       0x0); // DWC_DDRPHYA_ANIB7_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x8080,
                                       0x0); // DWC_DDRPHYA_ANIB8_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x9080,
                                       0x0); // DWC_DDRPHYA_ANIB9_ATxDly_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x10020,
                                       0x0); // DWC_DDRPHYA_DBYTE0_DFIMRL_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x10080,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x10081,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x100d0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x100d1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1008c,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxClkDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1008d,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxClkDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x10180,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x10181,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x101d0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x101d1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1018c,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxClkDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1018d,
                                       0x0); // DWC_DDRPHYA_DBYTE0_RxClkDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x100c0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x100c1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x101c0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x101c1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x102c0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x102c1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x103c0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x103c1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x104c0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r4_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x104c1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r4_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x105c0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r5_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x105c1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r5_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x106c0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r6_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x106c1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r6_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x107c0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r7_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x107c1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r7_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x108c0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r8_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x108c1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r8_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x100ae,
                                       0x0); // DWC_DDRPHYA_DBYTE0_PptDqsCntInvTrnTg0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x100af,
                                       0x0); // DWC_DDRPHYA_DBYTE0_PptDqsCntInvTrnTg1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x100a0,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Dq0LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x100a1,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Dq1LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x100a2,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Dq2LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x100a3,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Dq3LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x100a4,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Dq4LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x100a5,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Dq5LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x100a6,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Dq6LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x100a7,
                                       0x0); // DWC_DDRPHYA_DBYTE0_Dq7LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x11020,
                                       0x0); // DWC_DDRPHYA_DBYTE1_DFIMRL_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x11080,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x11081,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x110d0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x110d1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1108c,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxClkDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1108d,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxClkDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x11180,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x11181,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x111d0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x111d1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1118c,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxClkDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1118d,
                                       0x0); // DWC_DDRPHYA_DBYTE1_RxClkDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x110c0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x110c1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x111c0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x111c1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x112c0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x112c1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x113c0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x113c1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x114c0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r4_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x114c1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r4_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x115c0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r5_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x115c1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r5_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x116c0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r6_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x116c1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r6_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x117c0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r7_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x117c1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r7_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x118c0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r8_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x118c1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r8_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x110ae,
                                       0x0); // DWC_DDRPHYA_DBYTE1_PptDqsCntInvTrnTg0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x110af,
                                       0x0); // DWC_DDRPHYA_DBYTE1_PptDqsCntInvTrnTg1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x110a0,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Dq0LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x110a1,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Dq1LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x110a2,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Dq2LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x110a3,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Dq3LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x110a4,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Dq4LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x110a5,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Dq5LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x110a6,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Dq6LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x110a7,
                                       0x0); // DWC_DDRPHYA_DBYTE1_Dq7LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x12020,
                                       0x0); // DWC_DDRPHYA_DBYTE2_DFIMRL_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x12080,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x12081,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x120d0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x120d1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1208c,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxClkDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1208d,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxClkDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x12180,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x12181,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x121d0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x121d1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1218c,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxClkDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1218d,
                                       0x0); // DWC_DDRPHYA_DBYTE2_RxClkDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x120c0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x120c1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x121c0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x121c1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x122c0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x122c1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x123c0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x123c1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x124c0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r4_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x124c1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r4_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x125c0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r5_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x125c1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r5_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x126c0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r6_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x126c1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r6_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x127c0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r7_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x127c1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r7_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x128c0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r8_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x128c1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r8_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x120ae,
                                       0x0); // DWC_DDRPHYA_DBYTE2_PptDqsCntInvTrnTg0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x120af,
                                       0x0); // DWC_DDRPHYA_DBYTE2_PptDqsCntInvTrnTg1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x120a0,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Dq0LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x120a1,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Dq1LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x120a2,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Dq2LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x120a3,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Dq3LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x120a4,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Dq4LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x120a5,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Dq5LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x120a6,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Dq6LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x120a7,
                                       0x0); // DWC_DDRPHYA_DBYTE2_Dq7LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x13020,
                                       0x0); // DWC_DDRPHYA_DBYTE3_DFIMRL_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x13080,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x13081,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x130d0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x130d1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1308c,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxClkDlyTg0_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1308d,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxClkDlyTg1_u0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x13180,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x13181,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x131d0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x131d1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1318c,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxClkDlyTg0_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x1318d,
                                       0x0); // DWC_DDRPHYA_DBYTE3_RxClkDlyTg1_u1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x130c0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x130c1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x131c0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x131c1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x132c0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x132c1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x133c0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x133c1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x134c0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r4_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x134c1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r4_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x135c0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r5_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x135c1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r5_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x136c0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r6_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x136c1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r6_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x137c0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r7_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x137c1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r7_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x138c0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r8_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x138c1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r8_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x130ae,
                                       0x0); // DWC_DDRPHYA_DBYTE3_PptDqsCntInvTrnTg0_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x130af,
                                       0x0); // DWC_DDRPHYA_DBYTE3_PptDqsCntInvTrnTg1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x130a0,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Dq0LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x130a1,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Dq1LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x130a2,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Dq2LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x130a3,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Dq3LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x130a4,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Dq4LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x130a5,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Dq5LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x130a6,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Dq6LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x130a7,
                                       0x0); // DWC_DDRPHYA_DBYTE3_Dq7LnSel
    dwc_ddrphy_apb_wr_for_io_retention(0x90201,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BGPR1_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x90202,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BGPR2_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x90203,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BGPR3_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x90205,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BGPR5_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x90206,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BGPR6_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x90207,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BGPR7_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x90208,
                                       0x0); // DWC_DDRPHYA_INITENG0_Seq0BGPR8_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x20020,
                                       0x0); // DWC_DDRPHYA_MASTER0_HwtMRL_p0
    dwc_ddrphy_apb_wr_for_io_retention(0x20077,
                                       0x0); // DWC_DDRPHYA_MASTER0_HwtCAMode
    dwc_ddrphy_apb_wr_for_io_retention(0x20072,
                                       0x0); // DWC_DDRPHYA_MASTER0_HwtLpCsEnA
    dwc_ddrphy_apb_wr_for_io_retention(0x20073,
                                       0x0); // DWC_DDRPHYA_MASTER0_HwtLpCsEnB
    dwc_ddrphy_apb_wr_for_io_retention(0x400c0,
                                       0x0); // DWC_DDRPHYA_ACSM0_AcsmCtrl23
    dwc_ddrphy_apb_wr_for_io_retention(0x10040,
                                       0x0); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x10140,
                                       0x0); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x10240,
                                       0x0); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x10340,
                                       0x0); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x10440,
                                       0x0); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x10540,
                                       0x0); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x10640,
                                       0x0); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x10740,
                                       0x0); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x10840,
                                       0x0); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x11040,
                                       0x0); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x11140,
                                       0x0); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x11240,
                                       0x0); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x11340,
                                       0x0); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x11440,
                                       0x0); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x11540,
                                       0x0); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x11640,
                                       0x0); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x11740,
                                       0x0); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x11840,
                                       0x0); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x12040,
                                       0x0); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x12140,
                                       0x0); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x12240,
                                       0x0); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x12340,
                                       0x0); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x12440,
                                       0x0); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x12540,
                                       0x0); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x12640,
                                       0x0); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x12740,
                                       0x0); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x12840,
                                       0x0); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r8
    dwc_ddrphy_apb_wr_for_io_retention(0x13040,
                                       0x0); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r0
    dwc_ddrphy_apb_wr_for_io_retention(0x13140,
                                       0x0); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r1
    dwc_ddrphy_apb_wr_for_io_retention(0x13240,
                                       0x0); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r2
    dwc_ddrphy_apb_wr_for_io_retention(0x13340,
                                       0x0); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r3
    dwc_ddrphy_apb_wr_for_io_retention(0x13440,
                                       0x0); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r4
    dwc_ddrphy_apb_wr_for_io_retention(0x13540,
                                       0x0); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r5
    dwc_ddrphy_apb_wr_for_io_retention(0x13640,
                                       0x0); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r6
    dwc_ddrphy_apb_wr_for_io_retention(0x13740,
                                       0x0); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r7
    dwc_ddrphy_apb_wr_for_io_retention(0x13840,
                                       0x0); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r8
// [dwc_ddrphy_phyinit_restore_sequence] Write the UcclkHclkEnables CSR to disable the appropriate clocks after all reads done.
// Disabling Ucclk (PMU)
    dwc_ddrphy_apb_wr(0xc0080, 0x2); // DWC_DDRPHYA_DRTUB0_UcclkHclkEnables
// [dwc_ddrphy_phyinit_restore_sequence] Write the MicroContMuxSel CSR to 0x1 to isolate the internal CSRs during mission mode
    dwc_ddrphy_apb_wr(0xd0000, 0x1); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
//
//   After Function Call
//   --------------------------------------------------------------------------
//   To complete the Retention Exit sequence:
//
//   -# Initialize the PHY to Mission Mode through DFI Initialization
//     You may use the same sequence implemented in
//     dwc_ddrphy_phyinit_userCustom_J_enterMissionMode()
//
// [dwc_ddrphy_phyinit_restore_sequence] End of dwc_ddrphy_phyinit_restore_sequence
// [dwc_ddrphy_phyinit_main] End of dwc_ddrphy_phyinit_retention_sequence()
}
void get_str_info(struct str_info_t *t)
{
    int ret;
    //int i;
    uint64_t size = ROUNDUP(sizeof(struct str_info_t), DEFAULT_BLK_SZ);
    //arch_invalidate_cache_range(t, ROUNDUP(size, DEFAULT_BLK_SZ));
    ret = load_partition_force_size_by_name("ddr_ioretention", (uint8_t *)t,
                                            size, 0);

    //ret = load_partition_data("ddr_ioretention", (uint8_t *)t,
    //                             size, 0,0);
    if (!ret) {
        INFO("Opps, DDR_Init_Seq not found\n");
        return;
    }
}
//volatile int test=1;
void ddr_self_exit(void)
{
    unsigned int rdata;
    unsigned int addr;
    unsigned int wdata;
    unsigned int umctl_base;
    unsigned int phy_base;
    //int i=0;
    umctl_base = APB_DDRCTRL_BASE;
    phy_base   = APB_DDRPHY_BASE;
    str_info = (struct str_info_t *)&str_data[0];
    get_str_info(str_info);
    soc_dis_isolation(DDR_SS);
    soc_deassert_reset(DDR_SS);
    //scr_bit_set(APB_SCR_SEC_BASE, L31, SCR_SEC_NOC2DDR_FIFO_WRAP_BYPASS_L31_START_BIT);
#ifdef TGT_safe
    scr_bit_set(APB_SCR_SAF_BASE, RW, SCR_SAF_DDR_SS_PWROKIN_AON_RW_START_BIT);
#else
    scr_bit_set(APB_SCR_SEC_BASE, RW, SCR_SEC_DDR_SS_PWROKIN_AON_RW_START_BIT);
#endif
    soc_config_clk(DDR_SS, str_info->freq);
    //while(test);
// step11: TODO: DDR registers configure??
    addr = APB_DDR_PERF_MON_BASE + 0x80; //???
    rdata = readl(addr);
    //Lixin update, according to Lerry's mail at 2019-02-12
    //ddr phy apb clock gating should be enable before ddr initialization
    //set bit[17:16] and bit[0] to 1
    //wdata = 0x1 | rdata;
    //wdata = rdata | 0x30001;
    //Lixin update, according to Lerry's mail at 2019-02-15
    //set the ddr arbiter timeout value to 'd16, he will update the RTL to set the reset value to 16 at next release
    //wdata = 0x1001;
    //reprogram the register to pre-power removal values for controller
    wdata = 0x31001;
    writel(wdata, addr);
    // kunlun_r1p4 changes: the Pwron signal is moved to scr_saf | scr_sec, bit[0] of 0x80 is not used anymore
    //set_scr_sec_ddr_ss_pwrokin_aon(0x1);
//DBG1 dis_dq:does not de-queue any transaction from the CAM
    addr = umctl_base + 0x00000304;
    wdata = 0x00000001;
    writel(wdata, addr);
//PWRCTL
    addr = umctl_base + 0x00000030;
    rdata = readl(addr);
//enable selfref_sw
    rdata = rdata | 0x20;
    writel(rdata, addr);
    restore_ddrc_csr();
    //restore_dwc_ddrphy_phyinit_C_initPhyConfig();
    dwc_ddrphy_phyinit_retention_sequence();
    //dwc_ddrphy_phyinit_I_loadPIEImage();
    addr = phy_base + 0x200c9 * 4;
    rdata = readl(addr);
    unsigned int ii;

    for (ii = 0; ii < 0x1000; ii = ii + 1) {
        //printf("ZZZZZZ1 ii=%0d\n", ii);
        rdata = readl(addr);
    }

    //SWCTL sw_done
    addr = umctl_base + 0x00000320;
    wdata = 0x00000000;
    writel(wdata, addr);
//DFIMISC
    addr = umctl_base + 0x000001b0;
    wdata = 0x00001070;
    writel(wdata, addr);
    //DFISTAT dfi_init_complete
    addr = umctl_base + 0x000001bc;
    rdata = readl(addr);

    while (rdata != 0x1) {
        rdata = readl(addr);
    }

//DFIMISC
    addr = umctl_base + 0x000001b0;
    wdata = 0x00001071;
    writel(wdata, addr);
    //PWRCTL disable selfref_sw
    addr = umctl_base + 0x00000030;
    rdata = readl(addr);
    rdata = rdata & ~0x20;
    writel(rdata, addr);
    //STAT operating_mode
    addr = umctl_base + 0x00000004;
    rdata = readl(addr);

    while (rdata != 0x1) {
        rdata = readl(addr);
    }

//SWCTL sw_done
    addr = umctl_base + 0x00000320;
    wdata = 0x00000000;
    writel(wdata, addr);
//PCTRL_n port_en
    addr = umctl_base + 0x00000490;
    wdata = 0x00000001;
    writel(wdata, addr);
    //SBRCTL
    addr = umctl_base + 0x00000f24;
    wdata = 0x0000ff11;
    writel(wdata, addr);
//SWCTL sw_done
    addr = umctl_base + 0x00000320;
    wdata = 0x00000001;
    writel(wdata, addr);
}
#else
struct pt_load_config *str_get_pt_configs(void) {return NULL;}
uint32_t str_get_pt_configs_cnt(void) { return 0;}
bool is_str_enter(void) { return false;}
bool is_str_resume(void) {return false;}
void set_str_resume(enum STR_MASTER master, bool flag) {}
void str_save_ddr_freq(uint32_t freq) {}
void ddr_self_exit(void) {}
#endif
