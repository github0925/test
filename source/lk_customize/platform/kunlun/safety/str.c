#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sys/types.h>
#include <reg.h>
#if DDR_ENTER_SELF
#include <lib/reg.h>
#include "chipdev/ddr/dw_umctl2/inc/DWC_ddr_umctl2_reg.h"
#include "chipdev/ddr/dw_umctl2/inc/dwc_ddrphy_top_reg.h"
#include <pmu_hal.h>
#include "spi_nor_hal.h"
#include "__regs_base.h"
#include "target_res.h"
#include "partition_parser.h"
#include <arch/ops.h>

#define REG_AP_APB_DWC_DDR_UMCTL2_DBG1  (APB_DDRCTRL_BASE+UMCTL2_REGS_DBG1_OFF)
#define REG_AP_APB_DWC_DDR_UMCTL2_MSTR  (APB_DDRCTRL_BASE+UMCTL2_REGS_MSTR_OFF)
#define REG_AP_APB_DWC_DDR_UMCTL2_MSTR2  (APB_DDRCTRL_BASE+UMCTL2_REGS_MSTR2_OFF)
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
#define APB_DDRPHY_BASE (0xf2000000u)
#define STR_SAVED_FREQ  0xf841b000
#define DDR_W32(a, v)  writel(v, a)
#define DDR_R32(a)

struct str_info_t {
    uint32_t freq;
    uint32_t reserved[8];
    uint32_t save_csr_index;
    uint32_t restore_csr_index;
    uint32_t save_c_index;
    uint32_t save_i_index;
    uint32_t ddrc_csr[100];
    uint32_t phy_csr[970];
} str_info = {0};
struct str_info_t temp;

static void save_str_info(struct str_info_t *t)
{
    int ret = 0;
    void *handle;
    struct spi_nor_handle *spi_nor;
    uint32_t addr;
    partition_device_t  *ptdev = NULL;
    storage_device_t *storage = NULL;
    struct spi_nor_test_cfg {
        uint32_t id;
        struct spi_nor_cfg config;
    };
    struct spi_nor_test_cfg spi_nor_cfg_data = {
        .id = RES_OSPI_REG_OSPI1,
        .config =
        {
            .cs = SPI_NOR_CS0,
            .bus_clk = SPI_NOR_CLK_50MHZ,
            .octal_ddr_en = 0,
        }
    };
    storage = setup_storage_dev(OSPI, RES_OSPI_REG_OSPI1,
                                &spi_nor_cfg_data.config);
    ptdev = ptdev_setup(storage, storage->get_erase_group_size(storage) * 2);
    ptdev_read_table(ptdev);
    addr = ptdev_get_offset(ptdev, "ddr_ioretention");

    if (storage)
        storage->release(storage);

    printf("ddr addr 0x%x\n", addr);
    ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data.id);

    if (!ret)
        return;

    ((struct spi_nor_handle *)handle)->config = &spi_nor_cfg_data.config;
    ret = hal_spi_nor_init(handle);

    if (ret) {
        hal_spi_nor_release_handle(&handle);
        return;
    }

    spi_nor = (struct spi_nor_handle *)handle;
    //addr=get_partition_addr_by_name("ddr_ioretention");
    //return;
    hal_spi_nor_read((struct spi_nor_handle *)handle, (spi_nor_address_t)addr,
                     (uint8_t *)&temp, ROUNDUP(sizeof(struct str_info_t), spi_nor->block_size));
    printf("temp.ddrc_csr[0] 0x%x size 0x%x\n", temp.ddrc_csr[0],
           ROUNDUP(sizeof(struct str_info_t), spi_nor->block_size));
#if 0
    int i = 0;

    for (i = 0; i < 100; i++)
        printf("ddrc_csr[%d]=0x%08x\n", i, t->ddrc_csr[i]);

#endif

    if (temp.ddrc_csr[0] == 0xffffffff || temp.ddrc_csr[0] == 0) {
        int i;

        for (i = 0; i < 8; i++)
            t->reserved[i] = temp.reserved[i];

        arch_clean_cache_range((addr_t)t, ROUNDUP(sizeof(struct str_info_t),
                               spi_nor->block_size));
        hal_spi_nor_erase((struct spi_nor_handle *)handle, (spi_nor_address_t)addr,
                          ROUNDUP(sizeof(struct str_info_t), spi_nor->block_size));
        hal_spi_nor_write((struct spi_nor_handle *)handle, (spi_nor_address_t)addr,
                          (uint8_t *)t, ROUNDUP(sizeof(struct str_info_t), spi_nor->block_size));
    }
    else {
        printf("have save the data at last time, ignore\n");
    }

    hal_spi_nor_release_handle(&handle);
}
// scr_sec, DDR_SS_PWROKIN_AON, [416:416]
static void set_scr_saf_ddr_ss_pwrokin_aon(unsigned int data0)
{
    unsigned int rdata = 0;
    // scr_saf[416:416], start_bit - 0, end_bit - 0, offset - 0x34, data0[0:0]
    rdata = readl(APB_SCR_SAF_BASE + (0x34 << 10));
    writel((rdata & (~(1 << 0))) | ((data0 & 0x1) << 0),
           APB_SCR_SAF_BASE + (0x34 << 10));
}

static void ddrc_wr(uint32_t addr, uint32_t v)
{
    writel(v, addr);
}
static void ddrc_timing_reg_save(void)
{
    int rdata;
    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000001);
    rdata = readl(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2);
    str_info.ddrc_csr[0] = rdata;
    rdata = readl(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL);
    str_info.ddrc_csr[1] = rdata;
    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000000);
}
static int dwc_ddrphy_apb_rd(int phy_addr)
{
    return readl(APB_DDRPHY_BASE + phy_addr * 4);
}

static void dwc_ddrphy_apb_wr(int phy_addr, int phy_data)
{
    int w_addr = APB_DDRPHY_BASE + phy_addr * 4;
    int w_data = phy_data;
    //printf("WR: phy_addr=%0x, w_data=%0x\n", phy_addr, w_data);
    writel(w_data, w_addr);
}
//static volatile int test=1;
static void dwc_ddrphy_apb_rd_for_io_retention(int phy_addr)
{
    int rdata = 0;
    rdata = readl(APB_DDRPHY_BASE + phy_addr * 4);
    str_info.phy_csr[str_info.save_csr_index] = rdata;
    str_info.save_csr_index++;
}
/*
static void dwc_ddrphy_c_rd_for_io_retention(int phy_addr)
{
    int rdata = 0;
    rdata = readl(APB_DDRPHY_BASE + phy_addr);
    printf("0x%x:0x%x\n", (APB_DDRPHY_BASE + phy_addr), rdata);
    str_info.c_config[str_info.save_c_index] = rdata;
    str_info.save_c_index++;
}
void save_dwc_ddrphy_phyinit_C_initPhyConfig(void) {
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_TXSLEWRATE_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_TXSLEWRATE_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_TXSLEWRATE_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_TXSLEWRATE_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_TXSLEWRATE_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_TXSLEWRATE_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_TXSLEWRATE_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_TXSLEWRATE_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQ0LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQ1LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQ2LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQ3LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQ4LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQ5LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQ6LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQ7LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQ0LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQ1LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQ2LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQ3LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQ4LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQ5LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQ6LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQ7LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQ0LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQ1LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQ2LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQ3LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQ4LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQ5LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQ6LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQ7LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQ0LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQ1LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQ2LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQ3LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQ4LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQ5LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQ6LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQ7LNSEL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB0_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB1_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB2_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB3_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB4_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB5_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB6_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB7_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB8_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB9_ATXSLEWRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_PLLCTRL2_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_ARDPTRINITVAL_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_INITENG0_SEQ0BGPR4_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DQSPREAMBLECONTROL_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DBYTEDLLMODECNTRL_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DLLLOCKPARAM_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DLLGAINCTL_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_PROCODTTIMECTL_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_TXODTDRVSTREN_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_TXODTDRVSTREN_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_TXODTDRVSTREN_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_TXODTDRVSTREN_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_TXODTDRVSTREN_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_TXODTDRVSTREN_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_TXODTDRVSTREN_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_TXODTDRVSTREN_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_TXIMPEDANCECTRL1_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_TXIMPEDANCECTRL1_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_TXIMPEDANCECTRL1_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_TXIMPEDANCECTRL1_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_TXIMPEDANCECTRL1_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_TXIMPEDANCECTRL1_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_TXIMPEDANCECTRL1_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_TXIMPEDANCECTRL1_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB0_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB1_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB2_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB3_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB4_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB5_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB6_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB7_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB8_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_ANIB9_ATXIMPEDANCE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIMODE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFICAMODE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_CALDRVSTR0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_CALVREFS_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_CALUCLKINFO_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_CALRATE_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_VREFINGLOBAL_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQDQSRCVCNTRL_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE0_DQDQSRCVCNTRL_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQDQSRCVCNTRL_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE1_DQDQSRCVCNTRL_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQDQSRCVCNTRL_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE2_DQDQSRCVCNTRL_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQDQSRCVCNTRL_B0_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_DBYTE3_DQDQSRCVCNTRL_B1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIFREQRATIO_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_TRISTATEMODECA_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIFREQXLAT0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIFREQXLAT1_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIFREQXLAT2_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIFREQXLAT3_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIFREQXLAT4_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIFREQXLAT5_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIFREQXLAT6_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DFIFREQXLAT7_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_MASTERX4CONFIG_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_DMIPINPRESENT_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_ACX4ANIBDIS_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_PLLCTRL1_P0_OFF);
    dwc_ddrphy_c_rd_for_io_retention(DWC_DDRPHYA_MASTER0_PLLTESTMODE_P0_OFF);
}
*/
static void dwc_ddrphy_apb_wr_for_io_retention(int phy_addr, int phy_data)
{
    int w_addr = APB_DDRPHY_BASE + phy_addr * 4;
    int w_data = str_info.phy_csr[str_info.restore_csr_index];
    writel(w_data, w_addr);
    str_info.restore_csr_index++;
}
static void dwc_ddrphy_phyinit_userCustom_saveRetRegs(void)
{
//  printf("dwc_ddrphy_phyinit_userCustom_saveRetRegs start...\n");
// // [dwc_ddrphy_phyinit_userCustom_saveRetRegs] start of dwc_ddrphy_phyinit_userCustom_saveRetRegs()
//
// //##############################################################
// //
// // Customer Save Retention Registers
// //
// // This function can be used to implement saving of PHY registers to be
// // restored on retention exit. the following list of register reads can
// // be used to compile the exact list of registers.
// //
// //##############################################################
//
    dwc_ddrphy_apb_wr(0xd0000, 0x0); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
    dwc_ddrphy_apb_wr(0xc0080, 0x3); // DWC_DDRPHYA_DRTUB0_UcclkHclkEnables
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1005f); // DWC_DDRPHYA_DBYTE0_TxSlewRate_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1015f); // DWC_DDRPHYA_DBYTE0_TxSlewRate_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1105f); // DWC_DDRPHYA_DBYTE1_TxSlewRate_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1115f); // DWC_DDRPHYA_DBYTE1_TxSlewRate_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1205f); // DWC_DDRPHYA_DBYTE2_TxSlewRate_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1215f); // DWC_DDRPHYA_DBYTE2_TxSlewRate_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1305f); // DWC_DDRPHYA_DBYTE3_TxSlewRate_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1315f); // DWC_DDRPHYA_DBYTE3_TxSlewRate_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x55); // DWC_DDRPHYA_ANIB0_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1055); // DWC_DDRPHYA_ANIB1_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2055); // DWC_DDRPHYA_ANIB2_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x3055); // DWC_DDRPHYA_ANIB3_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4055); // DWC_DDRPHYA_ANIB4_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x5055); // DWC_DDRPHYA_ANIB5_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x6055); // DWC_DDRPHYA_ANIB6_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x7055); // DWC_DDRPHYA_ANIB7_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x8055); // DWC_DDRPHYA_ANIB8_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9055); // DWC_DDRPHYA_ANIB9_ATxSlewRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200c5); // DWC_DDRPHYA_MASTER0_PllCtrl2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2002e); // DWC_DDRPHYA_MASTER0_ARdPtrInitVal_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90204); // DWC_DDRPHYA_INITENG0_Seq0BGPR4_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20024); // DWC_DDRPHYA_MASTER0_DqsPreambleControl_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2003a); // DWC_DDRPHYA_MASTER0_DbyteDllModeCntrl
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2007d); // DWC_DDRPHYA_MASTER0_DllLockParam_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2007c); // DWC_DDRPHYA_MASTER0_DllGainCtl_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20056); // DWC_DDRPHYA_MASTER0_ProcOdtTimeCtl_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1004d); // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1014d); // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1104d); // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1114d); // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1204d); // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1214d); // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1304d); // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1314d); // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10049); // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10149); // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11049); // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11149); // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12049); // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12149); // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13049); // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13149); // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x43); // DWC_DDRPHYA_ANIB0_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1043); // DWC_DDRPHYA_ANIB1_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2043); // DWC_DDRPHYA_ANIB2_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(
        0x3043); // DWC_DDRPHYA_ANIB3_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4043); // DWC_DDRPHYA_ANIB4_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(
        0x5043); // DWC_DDRPHYA_ANIB5_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(
        0x6043); // DWC_DDRPHYA_ANIB6_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(
        0x7043); // DWC_DDRPHYA_ANIB7_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(
        0x8043); // DWC_DDRPHYA_ANIB8_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9043); // DWC_DDRPHYA_ANIB9_ATxImpedance
    dwc_ddrphy_apb_rd_for_io_retention(0x20018); // DWC_DDRPHYA_MASTER0_DfiMode
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20075); // DWC_DDRPHYA_MASTER0_DfiCAMode
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20050); // DWC_DDRPHYA_MASTER0_CalDrvStr0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2009b); // DWC_DDRPHYA_MASTER0_CalVRefs
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20008); // DWC_DDRPHYA_MASTER0_CalUclkInfo_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x20088); // DWC_DDRPHYA_MASTER0_CalRate
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200b2); // DWC_DDRPHYA_MASTER0_VrefInGlobal_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10043); // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10143); // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11043); // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11143); // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12043); // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12143); // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13043); // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13143); // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200fa); // DWC_DDRPHYA_MASTER0_DfiFreqRatio_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20019); // DWC_DDRPHYA_MASTER0_TristateModeCA_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200f0); // DWC_DDRPHYA_MASTER0_DfiFreqXlat0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200f1); // DWC_DDRPHYA_MASTER0_DfiFreqXlat1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200f2); // DWC_DDRPHYA_MASTER0_DfiFreqXlat2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200f3); // DWC_DDRPHYA_MASTER0_DfiFreqXlat3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200f4); // DWC_DDRPHYA_MASTER0_DfiFreqXlat4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200f5); // DWC_DDRPHYA_MASTER0_DfiFreqXlat5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200f6); // DWC_DDRPHYA_MASTER0_DfiFreqXlat6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200f7); // DWC_DDRPHYA_MASTER0_DfiFreqXlat7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1004a); // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1104a); // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1204a); // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1304a); // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20025); // DWC_DDRPHYA_MASTER0_MasterX4Config
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2002d); // DWC_DDRPHYA_MASTER0_DMIPinPresent_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2002c); // DWC_DDRPHYA_MASTER0_Acx4AnibDis
    dwc_ddrphy_apb_rd_for_io_retention(
        0xd0000); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90000); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90001); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90002); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90003); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90004); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90005); // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90029); // DWC_DDRPHYA_INITENG0_SequenceReg0b0s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9002a); // DWC_DDRPHYA_INITENG0_SequenceReg0b0s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9002b); // DWC_DDRPHYA_INITENG0_SequenceReg0b0s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9002c); // DWC_DDRPHYA_INITENG0_SequenceReg0b1s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9002d); // DWC_DDRPHYA_INITENG0_SequenceReg0b1s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9002e); // DWC_DDRPHYA_INITENG0_SequenceReg0b1s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9002f); // DWC_DDRPHYA_INITENG0_SequenceReg0b2s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90030); // DWC_DDRPHYA_INITENG0_SequenceReg0b2s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90031); // DWC_DDRPHYA_INITENG0_SequenceReg0b2s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90032); // DWC_DDRPHYA_INITENG0_SequenceReg0b3s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90033); // DWC_DDRPHYA_INITENG0_SequenceReg0b3s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90034); // DWC_DDRPHYA_INITENG0_SequenceReg0b3s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90035); // DWC_DDRPHYA_INITENG0_SequenceReg0b4s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90036); // DWC_DDRPHYA_INITENG0_SequenceReg0b4s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90037); // DWC_DDRPHYA_INITENG0_SequenceReg0b4s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90038); // DWC_DDRPHYA_INITENG0_SequenceReg0b5s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90039); // DWC_DDRPHYA_INITENG0_SequenceReg0b5s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9003a); // DWC_DDRPHYA_INITENG0_SequenceReg0b5s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9003b); // DWC_DDRPHYA_INITENG0_SequenceReg0b6s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9003c); // DWC_DDRPHYA_INITENG0_SequenceReg0b6s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9003d); // DWC_DDRPHYA_INITENG0_SequenceReg0b6s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9003e); // DWC_DDRPHYA_INITENG0_SequenceReg0b7s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9003f); // DWC_DDRPHYA_INITENG0_SequenceReg0b7s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90040); // DWC_DDRPHYA_INITENG0_SequenceReg0b7s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90041); // DWC_DDRPHYA_INITENG0_SequenceReg0b8s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90042); // DWC_DDRPHYA_INITENG0_SequenceReg0b8s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90043); // DWC_DDRPHYA_INITENG0_SequenceReg0b8s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90044); // DWC_DDRPHYA_INITENG0_SequenceReg0b9s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90045); // DWC_DDRPHYA_INITENG0_SequenceReg0b9s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90046); // DWC_DDRPHYA_INITENG0_SequenceReg0b9s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90047); // DWC_DDRPHYA_INITENG0_SequenceReg0b10s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90048); // DWC_DDRPHYA_INITENG0_SequenceReg0b10s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90049); // DWC_DDRPHYA_INITENG0_SequenceReg0b10s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9004a); // DWC_DDRPHYA_INITENG0_SequenceReg0b11s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9004b); // DWC_DDRPHYA_INITENG0_SequenceReg0b11s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9004c); // DWC_DDRPHYA_INITENG0_SequenceReg0b11s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9004d); // DWC_DDRPHYA_INITENG0_SequenceReg0b12s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9004e); // DWC_DDRPHYA_INITENG0_SequenceReg0b12s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9004f); // DWC_DDRPHYA_INITENG0_SequenceReg0b12s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90050); // DWC_DDRPHYA_INITENG0_SequenceReg0b13s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90051); // DWC_DDRPHYA_INITENG0_SequenceReg0b13s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90052); // DWC_DDRPHYA_INITENG0_SequenceReg0b13s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90053); // DWC_DDRPHYA_INITENG0_SequenceReg0b14s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90054); // DWC_DDRPHYA_INITENG0_SequenceReg0b14s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90055); // DWC_DDRPHYA_INITENG0_SequenceReg0b14s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90056); // DWC_DDRPHYA_INITENG0_SequenceReg0b15s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90057); // DWC_DDRPHYA_INITENG0_SequenceReg0b15s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90058); // DWC_DDRPHYA_INITENG0_SequenceReg0b15s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90059); // DWC_DDRPHYA_INITENG0_SequenceReg0b16s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9005a); // DWC_DDRPHYA_INITENG0_SequenceReg0b16s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9005b); // DWC_DDRPHYA_INITENG0_SequenceReg0b16s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9005c); // DWC_DDRPHYA_INITENG0_SequenceReg0b17s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9005d); // DWC_DDRPHYA_INITENG0_SequenceReg0b17s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9005e); // DWC_DDRPHYA_INITENG0_SequenceReg0b17s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9005f); // DWC_DDRPHYA_INITENG0_SequenceReg0b18s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90060); // DWC_DDRPHYA_INITENG0_SequenceReg0b18s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90061); // DWC_DDRPHYA_INITENG0_SequenceReg0b18s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90062); // DWC_DDRPHYA_INITENG0_SequenceReg0b19s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90063); // DWC_DDRPHYA_INITENG0_SequenceReg0b19s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90064); // DWC_DDRPHYA_INITENG0_SequenceReg0b19s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90065); // DWC_DDRPHYA_INITENG0_SequenceReg0b20s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90066); // DWC_DDRPHYA_INITENG0_SequenceReg0b20s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90067); // DWC_DDRPHYA_INITENG0_SequenceReg0b20s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90068); // DWC_DDRPHYA_INITENG0_SequenceReg0b21s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90069); // DWC_DDRPHYA_INITENG0_SequenceReg0b21s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9006a); // DWC_DDRPHYA_INITENG0_SequenceReg0b21s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9006b); // DWC_DDRPHYA_INITENG0_SequenceReg0b22s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9006c); // DWC_DDRPHYA_INITENG0_SequenceReg0b22s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9006d); // DWC_DDRPHYA_INITENG0_SequenceReg0b22s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9006e); // DWC_DDRPHYA_INITENG0_SequenceReg0b23s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9006f); // DWC_DDRPHYA_INITENG0_SequenceReg0b23s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90070); // DWC_DDRPHYA_INITENG0_SequenceReg0b23s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90071); // DWC_DDRPHYA_INITENG0_SequenceReg0b24s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90072); // DWC_DDRPHYA_INITENG0_SequenceReg0b24s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90073); // DWC_DDRPHYA_INITENG0_SequenceReg0b24s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90074); // DWC_DDRPHYA_INITENG0_SequenceReg0b25s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90075); // DWC_DDRPHYA_INITENG0_SequenceReg0b25s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90076); // DWC_DDRPHYA_INITENG0_SequenceReg0b25s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90077); // DWC_DDRPHYA_INITENG0_SequenceReg0b26s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90078); // DWC_DDRPHYA_INITENG0_SequenceReg0b26s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90079); // DWC_DDRPHYA_INITENG0_SequenceReg0b26s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9007a); // DWC_DDRPHYA_INITENG0_SequenceReg0b27s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9007b); // DWC_DDRPHYA_INITENG0_SequenceReg0b27s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9007c); // DWC_DDRPHYA_INITENG0_SequenceReg0b27s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9007d); // DWC_DDRPHYA_INITENG0_SequenceReg0b28s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9007e); // DWC_DDRPHYA_INITENG0_SequenceReg0b28s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9007f); // DWC_DDRPHYA_INITENG0_SequenceReg0b28s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90080); // DWC_DDRPHYA_INITENG0_SequenceReg0b29s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90081); // DWC_DDRPHYA_INITENG0_SequenceReg0b29s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90082); // DWC_DDRPHYA_INITENG0_SequenceReg0b29s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90083); // DWC_DDRPHYA_INITENG0_SequenceReg0b30s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90084); // DWC_DDRPHYA_INITENG0_SequenceReg0b30s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90085); // DWC_DDRPHYA_INITENG0_SequenceReg0b30s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90086); // DWC_DDRPHYA_INITENG0_SequenceReg0b31s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90087); // DWC_DDRPHYA_INITENG0_SequenceReg0b31s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90088); // DWC_DDRPHYA_INITENG0_SequenceReg0b31s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90089); // DWC_DDRPHYA_INITENG0_SequenceReg0b32s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9008a); // DWC_DDRPHYA_INITENG0_SequenceReg0b32s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9008b); // DWC_DDRPHYA_INITENG0_SequenceReg0b32s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9008c); // DWC_DDRPHYA_INITENG0_SequenceReg0b33s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9008d); // DWC_DDRPHYA_INITENG0_SequenceReg0b33s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9008e); // DWC_DDRPHYA_INITENG0_SequenceReg0b33s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9008f); // DWC_DDRPHYA_INITENG0_SequenceReg0b34s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90090); // DWC_DDRPHYA_INITENG0_SequenceReg0b34s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90091); // DWC_DDRPHYA_INITENG0_SequenceReg0b34s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90092); // DWC_DDRPHYA_INITENG0_SequenceReg0b35s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90093); // DWC_DDRPHYA_INITENG0_SequenceReg0b35s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90094); // DWC_DDRPHYA_INITENG0_SequenceReg0b35s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90095); // DWC_DDRPHYA_INITENG0_SequenceReg0b36s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90096); // DWC_DDRPHYA_INITENG0_SequenceReg0b36s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90097); // DWC_DDRPHYA_INITENG0_SequenceReg0b36s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90098); // DWC_DDRPHYA_INITENG0_SequenceReg0b37s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90099); // DWC_DDRPHYA_INITENG0_SequenceReg0b37s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9009a); // DWC_DDRPHYA_INITENG0_SequenceReg0b37s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9009b); // DWC_DDRPHYA_INITENG0_SequenceReg0b38s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9009c); // DWC_DDRPHYA_INITENG0_SequenceReg0b38s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9009d); // DWC_DDRPHYA_INITENG0_SequenceReg0b38s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9009e); // DWC_DDRPHYA_INITENG0_SequenceReg0b39s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9009f); // DWC_DDRPHYA_INITENG0_SequenceReg0b39s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a0); // DWC_DDRPHYA_INITENG0_SequenceReg0b39s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a1); // DWC_DDRPHYA_INITENG0_SequenceReg0b40s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a2); // DWC_DDRPHYA_INITENG0_SequenceReg0b40s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a3); // DWC_DDRPHYA_INITENG0_SequenceReg0b40s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40000); // DWC_DDRPHYA_ACSM0_AcsmSeq0x0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40020); // DWC_DDRPHYA_ACSM0_AcsmSeq1x0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40040); // DWC_DDRPHYA_ACSM0_AcsmSeq2x0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40060); // DWC_DDRPHYA_ACSM0_AcsmSeq3x0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40001); // DWC_DDRPHYA_ACSM0_AcsmSeq0x1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40021); // DWC_DDRPHYA_ACSM0_AcsmSeq1x1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40041); // DWC_DDRPHYA_ACSM0_AcsmSeq2x1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40061); // DWC_DDRPHYA_ACSM0_AcsmSeq3x1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40002); // DWC_DDRPHYA_ACSM0_AcsmSeq0x2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40022); // DWC_DDRPHYA_ACSM0_AcsmSeq1x2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40042); // DWC_DDRPHYA_ACSM0_AcsmSeq2x2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40062); // DWC_DDRPHYA_ACSM0_AcsmSeq3x2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40003); // DWC_DDRPHYA_ACSM0_AcsmSeq0x3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40023); // DWC_DDRPHYA_ACSM0_AcsmSeq1x3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40043); // DWC_DDRPHYA_ACSM0_AcsmSeq2x3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40063); // DWC_DDRPHYA_ACSM0_AcsmSeq3x3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40004); // DWC_DDRPHYA_ACSM0_AcsmSeq0x4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40024); // DWC_DDRPHYA_ACSM0_AcsmSeq1x4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40044); // DWC_DDRPHYA_ACSM0_AcsmSeq2x4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40064); // DWC_DDRPHYA_ACSM0_AcsmSeq3x4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40005); // DWC_DDRPHYA_ACSM0_AcsmSeq0x5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40025); // DWC_DDRPHYA_ACSM0_AcsmSeq1x5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40045); // DWC_DDRPHYA_ACSM0_AcsmSeq2x5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40065); // DWC_DDRPHYA_ACSM0_AcsmSeq3x5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40006); // DWC_DDRPHYA_ACSM0_AcsmSeq0x6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40026); // DWC_DDRPHYA_ACSM0_AcsmSeq1x6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40046); // DWC_DDRPHYA_ACSM0_AcsmSeq2x6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40066); // DWC_DDRPHYA_ACSM0_AcsmSeq3x6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40007); // DWC_DDRPHYA_ACSM0_AcsmSeq0x7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40027); // DWC_DDRPHYA_ACSM0_AcsmSeq1x7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40047); // DWC_DDRPHYA_ACSM0_AcsmSeq2x7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40067); // DWC_DDRPHYA_ACSM0_AcsmSeq3x7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40008); // DWC_DDRPHYA_ACSM0_AcsmSeq0x8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40028); // DWC_DDRPHYA_ACSM0_AcsmSeq1x8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40048); // DWC_DDRPHYA_ACSM0_AcsmSeq2x8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40068); // DWC_DDRPHYA_ACSM0_AcsmSeq3x8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40009); // DWC_DDRPHYA_ACSM0_AcsmSeq0x9
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40029); // DWC_DDRPHYA_ACSM0_AcsmSeq1x9
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40049); // DWC_DDRPHYA_ACSM0_AcsmSeq2x9
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40069); // DWC_DDRPHYA_ACSM0_AcsmSeq3x9
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4000a); // DWC_DDRPHYA_ACSM0_AcsmSeq0x10
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4002a); // DWC_DDRPHYA_ACSM0_AcsmSeq1x10
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4004a); // DWC_DDRPHYA_ACSM0_AcsmSeq2x10
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4006a); // DWC_DDRPHYA_ACSM0_AcsmSeq3x10
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4000b); // DWC_DDRPHYA_ACSM0_AcsmSeq0x11
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4002b); // DWC_DDRPHYA_ACSM0_AcsmSeq1x11
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4004b); // DWC_DDRPHYA_ACSM0_AcsmSeq2x11
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4006b); // DWC_DDRPHYA_ACSM0_AcsmSeq3x11
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4000c); // DWC_DDRPHYA_ACSM0_AcsmSeq0x12
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4002c); // DWC_DDRPHYA_ACSM0_AcsmSeq1x12
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4004c); // DWC_DDRPHYA_ACSM0_AcsmSeq2x12
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4006c); // DWC_DDRPHYA_ACSM0_AcsmSeq3x12
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4000d); // DWC_DDRPHYA_ACSM0_AcsmSeq0x13
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4002d); // DWC_DDRPHYA_ACSM0_AcsmSeq1x13
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4004d); // DWC_DDRPHYA_ACSM0_AcsmSeq2x13
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4006d); // DWC_DDRPHYA_ACSM0_AcsmSeq3x13
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4000e); // DWC_DDRPHYA_ACSM0_AcsmSeq0x14
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4002e); // DWC_DDRPHYA_ACSM0_AcsmSeq1x14
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4004e); // DWC_DDRPHYA_ACSM0_AcsmSeq2x14
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4006e); // DWC_DDRPHYA_ACSM0_AcsmSeq3x14
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4000f); // DWC_DDRPHYA_ACSM0_AcsmSeq0x15
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4002f); // DWC_DDRPHYA_ACSM0_AcsmSeq1x15
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4004f); // DWC_DDRPHYA_ACSM0_AcsmSeq2x15
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4006f); // DWC_DDRPHYA_ACSM0_AcsmSeq3x15
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40010); // DWC_DDRPHYA_ACSM0_AcsmSeq0x16
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40030); // DWC_DDRPHYA_ACSM0_AcsmSeq1x16
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40050); // DWC_DDRPHYA_ACSM0_AcsmSeq2x16
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40070); // DWC_DDRPHYA_ACSM0_AcsmSeq3x16
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40011); // DWC_DDRPHYA_ACSM0_AcsmSeq0x17
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40031); // DWC_DDRPHYA_ACSM0_AcsmSeq1x17
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40051); // DWC_DDRPHYA_ACSM0_AcsmSeq2x17
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40071); // DWC_DDRPHYA_ACSM0_AcsmSeq3x17
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40012); // DWC_DDRPHYA_ACSM0_AcsmSeq0x18
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40032); // DWC_DDRPHYA_ACSM0_AcsmSeq1x18
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40052); // DWC_DDRPHYA_ACSM0_AcsmSeq2x18
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40072); // DWC_DDRPHYA_ACSM0_AcsmSeq3x18
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40013); // DWC_DDRPHYA_ACSM0_AcsmSeq0x19
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40033); // DWC_DDRPHYA_ACSM0_AcsmSeq1x19
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40053); // DWC_DDRPHYA_ACSM0_AcsmSeq2x19
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40073); // DWC_DDRPHYA_ACSM0_AcsmSeq3x19
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40014); // DWC_DDRPHYA_ACSM0_AcsmSeq0x20
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40034); // DWC_DDRPHYA_ACSM0_AcsmSeq1x20
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40054); // DWC_DDRPHYA_ACSM0_AcsmSeq2x20
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40074); // DWC_DDRPHYA_ACSM0_AcsmSeq3x20
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40015); // DWC_DDRPHYA_ACSM0_AcsmSeq0x21
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40035); // DWC_DDRPHYA_ACSM0_AcsmSeq1x21
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40055); // DWC_DDRPHYA_ACSM0_AcsmSeq2x21
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40075); // DWC_DDRPHYA_ACSM0_AcsmSeq3x21
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40016); // DWC_DDRPHYA_ACSM0_AcsmSeq0x22
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40036); // DWC_DDRPHYA_ACSM0_AcsmSeq1x22
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40056); // DWC_DDRPHYA_ACSM0_AcsmSeq2x22
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40076); // DWC_DDRPHYA_ACSM0_AcsmSeq3x22
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40017); // DWC_DDRPHYA_ACSM0_AcsmSeq0x23
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40037); // DWC_DDRPHYA_ACSM0_AcsmSeq1x23
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40057); // DWC_DDRPHYA_ACSM0_AcsmSeq2x23
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40077); // DWC_DDRPHYA_ACSM0_AcsmSeq3x23
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40018); // DWC_DDRPHYA_ACSM0_AcsmSeq0x24
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40038); // DWC_DDRPHYA_ACSM0_AcsmSeq1x24
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40058); // DWC_DDRPHYA_ACSM0_AcsmSeq2x24
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40078); // DWC_DDRPHYA_ACSM0_AcsmSeq3x24
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40019); // DWC_DDRPHYA_ACSM0_AcsmSeq0x25
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40039); // DWC_DDRPHYA_ACSM0_AcsmSeq1x25
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40059); // DWC_DDRPHYA_ACSM0_AcsmSeq2x25
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40079); // DWC_DDRPHYA_ACSM0_AcsmSeq3x25
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4001a); // DWC_DDRPHYA_ACSM0_AcsmSeq0x26
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4003a); // DWC_DDRPHYA_ACSM0_AcsmSeq1x26
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4005a); // DWC_DDRPHYA_ACSM0_AcsmSeq2x26
    dwc_ddrphy_apb_rd_for_io_retention(
        0x4007a); // DWC_DDRPHYA_ACSM0_AcsmSeq3x26
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a4); // DWC_DDRPHYA_INITENG0_SequenceReg0b41s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a5); // DWC_DDRPHYA_INITENG0_SequenceReg0b41s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a6); // DWC_DDRPHYA_INITENG0_SequenceReg0b41s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a7); // DWC_DDRPHYA_INITENG0_SequenceReg0b42s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a8); // DWC_DDRPHYA_INITENG0_SequenceReg0b42s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900a9); // DWC_DDRPHYA_INITENG0_SequenceReg0b42s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900aa); // DWC_DDRPHYA_INITENG0_SequenceReg0b43s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ab); // DWC_DDRPHYA_INITENG0_SequenceReg0b43s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ac); // DWC_DDRPHYA_INITENG0_SequenceReg0b43s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ad); // DWC_DDRPHYA_INITENG0_SequenceReg0b44s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ae); // DWC_DDRPHYA_INITENG0_SequenceReg0b44s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900af); // DWC_DDRPHYA_INITENG0_SequenceReg0b44s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b0); // DWC_DDRPHYA_INITENG0_SequenceReg0b45s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b1); // DWC_DDRPHYA_INITENG0_SequenceReg0b45s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b2); // DWC_DDRPHYA_INITENG0_SequenceReg0b45s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b3); // DWC_DDRPHYA_INITENG0_SequenceReg0b46s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b4); // DWC_DDRPHYA_INITENG0_SequenceReg0b46s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b5); // DWC_DDRPHYA_INITENG0_SequenceReg0b46s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b6); // DWC_DDRPHYA_INITENG0_SequenceReg0b47s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b7); // DWC_DDRPHYA_INITENG0_SequenceReg0b47s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b8); // DWC_DDRPHYA_INITENG0_SequenceReg0b47s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900b9); // DWC_DDRPHYA_INITENG0_SequenceReg0b48s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ba); // DWC_DDRPHYA_INITENG0_SequenceReg0b48s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900bb); // DWC_DDRPHYA_INITENG0_SequenceReg0b48s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900bc); // DWC_DDRPHYA_INITENG0_SequenceReg0b49s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900bd); // DWC_DDRPHYA_INITENG0_SequenceReg0b49s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900be); // DWC_DDRPHYA_INITENG0_SequenceReg0b49s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900bf); // DWC_DDRPHYA_INITENG0_SequenceReg0b50s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c0); // DWC_DDRPHYA_INITENG0_SequenceReg0b50s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c1); // DWC_DDRPHYA_INITENG0_SequenceReg0b50s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c2); // DWC_DDRPHYA_INITENG0_SequenceReg0b51s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c3); // DWC_DDRPHYA_INITENG0_SequenceReg0b51s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c4); // DWC_DDRPHYA_INITENG0_SequenceReg0b51s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c5); // DWC_DDRPHYA_INITENG0_SequenceReg0b52s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c6); // DWC_DDRPHYA_INITENG0_SequenceReg0b52s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c7); // DWC_DDRPHYA_INITENG0_SequenceReg0b52s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c8); // DWC_DDRPHYA_INITENG0_SequenceReg0b53s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900c9); // DWC_DDRPHYA_INITENG0_SequenceReg0b53s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ca); // DWC_DDRPHYA_INITENG0_SequenceReg0b53s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900cb); // DWC_DDRPHYA_INITENG0_SequenceReg0b54s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900cc); // DWC_DDRPHYA_INITENG0_SequenceReg0b54s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900cd); // DWC_DDRPHYA_INITENG0_SequenceReg0b54s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ce); // DWC_DDRPHYA_INITENG0_SequenceReg0b55s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900cf); // DWC_DDRPHYA_INITENG0_SequenceReg0b55s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d0); // DWC_DDRPHYA_INITENG0_SequenceReg0b55s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d1); // DWC_DDRPHYA_INITENG0_SequenceReg0b56s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d2); // DWC_DDRPHYA_INITENG0_SequenceReg0b56s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d3); // DWC_DDRPHYA_INITENG0_SequenceReg0b56s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d4); // DWC_DDRPHYA_INITENG0_SequenceReg0b57s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d5); // DWC_DDRPHYA_INITENG0_SequenceReg0b57s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d6); // DWC_DDRPHYA_INITENG0_SequenceReg0b57s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d7); // DWC_DDRPHYA_INITENG0_SequenceReg0b58s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d8); // DWC_DDRPHYA_INITENG0_SequenceReg0b58s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900d9); // DWC_DDRPHYA_INITENG0_SequenceReg0b58s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900da); // DWC_DDRPHYA_INITENG0_SequenceReg0b59s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900db); // DWC_DDRPHYA_INITENG0_SequenceReg0b59s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900dc); // DWC_DDRPHYA_INITENG0_SequenceReg0b59s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900dd); // DWC_DDRPHYA_INITENG0_SequenceReg0b60s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900de); // DWC_DDRPHYA_INITENG0_SequenceReg0b60s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900df); // DWC_DDRPHYA_INITENG0_SequenceReg0b60s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e0); // DWC_DDRPHYA_INITENG0_SequenceReg0b61s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e1); // DWC_DDRPHYA_INITENG0_SequenceReg0b61s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e2); // DWC_DDRPHYA_INITENG0_SequenceReg0b61s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e3); // DWC_DDRPHYA_INITENG0_SequenceReg0b62s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e4); // DWC_DDRPHYA_INITENG0_SequenceReg0b62s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e5); // DWC_DDRPHYA_INITENG0_SequenceReg0b62s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e6); // DWC_DDRPHYA_INITENG0_SequenceReg0b63s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e7); // DWC_DDRPHYA_INITENG0_SequenceReg0b63s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e8); // DWC_DDRPHYA_INITENG0_SequenceReg0b63s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900e9); // DWC_DDRPHYA_INITENG0_SequenceReg0b64s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ea); // DWC_DDRPHYA_INITENG0_SequenceReg0b64s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900eb); // DWC_DDRPHYA_INITENG0_SequenceReg0b64s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ec); // DWC_DDRPHYA_INITENG0_SequenceReg0b65s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ed); // DWC_DDRPHYA_INITENG0_SequenceReg0b65s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ee); // DWC_DDRPHYA_INITENG0_SequenceReg0b65s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ef); // DWC_DDRPHYA_INITENG0_SequenceReg0b66s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f0); // DWC_DDRPHYA_INITENG0_SequenceReg0b66s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f1); // DWC_DDRPHYA_INITENG0_SequenceReg0b66s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f2); // DWC_DDRPHYA_INITENG0_SequenceReg0b67s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f3); // DWC_DDRPHYA_INITENG0_SequenceReg0b67s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f4); // DWC_DDRPHYA_INITENG0_SequenceReg0b67s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f5); // DWC_DDRPHYA_INITENG0_SequenceReg0b68s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f6); // DWC_DDRPHYA_INITENG0_SequenceReg0b68s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f7); // DWC_DDRPHYA_INITENG0_SequenceReg0b68s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f8); // DWC_DDRPHYA_INITENG0_SequenceReg0b69s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900f9); // DWC_DDRPHYA_INITENG0_SequenceReg0b69s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900fa); // DWC_DDRPHYA_INITENG0_SequenceReg0b69s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900fb); // DWC_DDRPHYA_INITENG0_SequenceReg0b70s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900fc); // DWC_DDRPHYA_INITENG0_SequenceReg0b70s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900fd); // DWC_DDRPHYA_INITENG0_SequenceReg0b70s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900fe); // DWC_DDRPHYA_INITENG0_SequenceReg0b71s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x900ff); // DWC_DDRPHYA_INITENG0_SequenceReg0b71s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90100); // DWC_DDRPHYA_INITENG0_SequenceReg0b71s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90101); // DWC_DDRPHYA_INITENG0_SequenceReg0b72s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90102); // DWC_DDRPHYA_INITENG0_SequenceReg0b72s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90103); // DWC_DDRPHYA_INITENG0_SequenceReg0b72s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90104); // DWC_DDRPHYA_INITENG0_SequenceReg0b73s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90105); // DWC_DDRPHYA_INITENG0_SequenceReg0b73s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90106); // DWC_DDRPHYA_INITENG0_SequenceReg0b73s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90107); // DWC_DDRPHYA_INITENG0_SequenceReg0b74s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90108); // DWC_DDRPHYA_INITENG0_SequenceReg0b74s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90109); // DWC_DDRPHYA_INITENG0_SequenceReg0b74s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9010a); // DWC_DDRPHYA_INITENG0_SequenceReg0b75s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9010b); // DWC_DDRPHYA_INITENG0_SequenceReg0b75s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9010c); // DWC_DDRPHYA_INITENG0_SequenceReg0b75s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9010d); // DWC_DDRPHYA_INITENG0_SequenceReg0b76s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9010e); // DWC_DDRPHYA_INITENG0_SequenceReg0b76s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9010f); // DWC_DDRPHYA_INITENG0_SequenceReg0b76s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90110); // DWC_DDRPHYA_INITENG0_SequenceReg0b77s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90111); // DWC_DDRPHYA_INITENG0_SequenceReg0b77s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90112); // DWC_DDRPHYA_INITENG0_SequenceReg0b77s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90113); // DWC_DDRPHYA_INITENG0_SequenceReg0b78s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90114); // DWC_DDRPHYA_INITENG0_SequenceReg0b78s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90115); // DWC_DDRPHYA_INITENG0_SequenceReg0b78s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90116); // DWC_DDRPHYA_INITENG0_SequenceReg0b79s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90117); // DWC_DDRPHYA_INITENG0_SequenceReg0b79s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90118); // DWC_DDRPHYA_INITENG0_SequenceReg0b79s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90119); // DWC_DDRPHYA_INITENG0_SequenceReg0b80s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9011a); // DWC_DDRPHYA_INITENG0_SequenceReg0b80s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9011b); // DWC_DDRPHYA_INITENG0_SequenceReg0b80s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9011c); // DWC_DDRPHYA_INITENG0_SequenceReg0b81s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9011d); // DWC_DDRPHYA_INITENG0_SequenceReg0b81s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9011e); // DWC_DDRPHYA_INITENG0_SequenceReg0b81s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9011f); // DWC_DDRPHYA_INITENG0_SequenceReg0b82s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90120); // DWC_DDRPHYA_INITENG0_SequenceReg0b82s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90121); // DWC_DDRPHYA_INITENG0_SequenceReg0b82s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90122); // DWC_DDRPHYA_INITENG0_SequenceReg0b83s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90123); // DWC_DDRPHYA_INITENG0_SequenceReg0b83s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90124); // DWC_DDRPHYA_INITENG0_SequenceReg0b83s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90125); // DWC_DDRPHYA_INITENG0_SequenceReg0b84s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90126); // DWC_DDRPHYA_INITENG0_SequenceReg0b84s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90127); // DWC_DDRPHYA_INITENG0_SequenceReg0b84s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90128); // DWC_DDRPHYA_INITENG0_SequenceReg0b85s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90129); // DWC_DDRPHYA_INITENG0_SequenceReg0b85s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9012a); // DWC_DDRPHYA_INITENG0_SequenceReg0b85s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9012b); // DWC_DDRPHYA_INITENG0_SequenceReg0b86s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9012c); // DWC_DDRPHYA_INITENG0_SequenceReg0b86s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9012d); // DWC_DDRPHYA_INITENG0_SequenceReg0b86s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9012e); // DWC_DDRPHYA_INITENG0_SequenceReg0b87s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9012f); // DWC_DDRPHYA_INITENG0_SequenceReg0b87s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90130); // DWC_DDRPHYA_INITENG0_SequenceReg0b87s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90131); // DWC_DDRPHYA_INITENG0_SequenceReg0b88s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90132); // DWC_DDRPHYA_INITENG0_SequenceReg0b88s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90133); // DWC_DDRPHYA_INITENG0_SequenceReg0b88s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90134); // DWC_DDRPHYA_INITENG0_SequenceReg0b89s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90135); // DWC_DDRPHYA_INITENG0_SequenceReg0b89s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90136); // DWC_DDRPHYA_INITENG0_SequenceReg0b89s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90137); // DWC_DDRPHYA_INITENG0_SequenceReg0b90s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90138); // DWC_DDRPHYA_INITENG0_SequenceReg0b90s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90139); // DWC_DDRPHYA_INITENG0_SequenceReg0b90s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9013a); // DWC_DDRPHYA_INITENG0_SequenceReg0b91s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9013b); // DWC_DDRPHYA_INITENG0_SequenceReg0b91s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9013c); // DWC_DDRPHYA_INITENG0_SequenceReg0b91s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9013d); // DWC_DDRPHYA_INITENG0_SequenceReg0b92s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9013e); // DWC_DDRPHYA_INITENG0_SequenceReg0b92s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9013f); // DWC_DDRPHYA_INITENG0_SequenceReg0b92s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90140); // DWC_DDRPHYA_INITENG0_SequenceReg0b93s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90141); // DWC_DDRPHYA_INITENG0_SequenceReg0b93s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90142); // DWC_DDRPHYA_INITENG0_SequenceReg0b93s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90143); // DWC_DDRPHYA_INITENG0_SequenceReg0b94s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90144); // DWC_DDRPHYA_INITENG0_SequenceReg0b94s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90145); // DWC_DDRPHYA_INITENG0_SequenceReg0b94s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90146); // DWC_DDRPHYA_INITENG0_SequenceReg0b95s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90147); // DWC_DDRPHYA_INITENG0_SequenceReg0b95s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90148); // DWC_DDRPHYA_INITENG0_SequenceReg0b95s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90149); // DWC_DDRPHYA_INITENG0_SequenceReg0b96s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9014a); // DWC_DDRPHYA_INITENG0_SequenceReg0b96s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9014b); // DWC_DDRPHYA_INITENG0_SequenceReg0b96s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9014c); // DWC_DDRPHYA_INITENG0_SequenceReg0b97s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9014d); // DWC_DDRPHYA_INITENG0_SequenceReg0b97s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9014e); // DWC_DDRPHYA_INITENG0_SequenceReg0b97s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9014f); // DWC_DDRPHYA_INITENG0_SequenceReg0b98s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90150); // DWC_DDRPHYA_INITENG0_SequenceReg0b98s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90151); // DWC_DDRPHYA_INITENG0_SequenceReg0b98s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90152); // DWC_DDRPHYA_INITENG0_SequenceReg0b99s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90153); // DWC_DDRPHYA_INITENG0_SequenceReg0b99s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90154); // DWC_DDRPHYA_INITENG0_SequenceReg0b99s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90155); // DWC_DDRPHYA_INITENG0_SequenceReg0b100s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90156); // DWC_DDRPHYA_INITENG0_SequenceReg0b100s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90157); // DWC_DDRPHYA_INITENG0_SequenceReg0b100s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90158); // DWC_DDRPHYA_INITENG0_SequenceReg0b101s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90159); // DWC_DDRPHYA_INITENG0_SequenceReg0b101s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9015a); // DWC_DDRPHYA_INITENG0_SequenceReg0b101s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9015b); // DWC_DDRPHYA_INITENG0_SequenceReg0b102s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9015c); // DWC_DDRPHYA_INITENG0_SequenceReg0b102s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9015d); // DWC_DDRPHYA_INITENG0_SequenceReg0b102s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9015e); // DWC_DDRPHYA_INITENG0_SequenceReg0b103s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9015f); // DWC_DDRPHYA_INITENG0_SequenceReg0b103s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90160); // DWC_DDRPHYA_INITENG0_SequenceReg0b103s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90161); // DWC_DDRPHYA_INITENG0_SequenceReg0b104s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90162); // DWC_DDRPHYA_INITENG0_SequenceReg0b104s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90163); // DWC_DDRPHYA_INITENG0_SequenceReg0b104s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90164); // DWC_DDRPHYA_INITENG0_SequenceReg0b105s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90165); // DWC_DDRPHYA_INITENG0_SequenceReg0b105s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90166); // DWC_DDRPHYA_INITENG0_SequenceReg0b105s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90167); // DWC_DDRPHYA_INITENG0_SequenceReg0b106s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90168); // DWC_DDRPHYA_INITENG0_SequenceReg0b106s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90169); // DWC_DDRPHYA_INITENG0_SequenceReg0b106s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9016a); // DWC_DDRPHYA_INITENG0_SequenceReg0b107s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9016b); // DWC_DDRPHYA_INITENG0_SequenceReg0b107s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9016c); // DWC_DDRPHYA_INITENG0_SequenceReg0b107s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9016d); // DWC_DDRPHYA_INITENG0_SequenceReg0b108s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9016e); // DWC_DDRPHYA_INITENG0_SequenceReg0b108s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9016f); // DWC_DDRPHYA_INITENG0_SequenceReg0b108s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90170); // DWC_DDRPHYA_INITENG0_SequenceReg0b109s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90171); // DWC_DDRPHYA_INITENG0_SequenceReg0b109s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90172); // DWC_DDRPHYA_INITENG0_SequenceReg0b109s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90173); // DWC_DDRPHYA_INITENG0_SequenceReg0b110s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90174); // DWC_DDRPHYA_INITENG0_SequenceReg0b110s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90175); // DWC_DDRPHYA_INITENG0_SequenceReg0b110s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90176); // DWC_DDRPHYA_INITENG0_SequenceReg0b111s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90177); // DWC_DDRPHYA_INITENG0_SequenceReg0b111s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90178); // DWC_DDRPHYA_INITENG0_SequenceReg0b111s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90179); // DWC_DDRPHYA_INITENG0_SequenceReg0b112s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9017a); // DWC_DDRPHYA_INITENG0_SequenceReg0b112s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9017b); // DWC_DDRPHYA_INITENG0_SequenceReg0b112s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9017c); // DWC_DDRPHYA_INITENG0_SequenceReg0b113s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9017d); // DWC_DDRPHYA_INITENG0_SequenceReg0b113s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9017e); // DWC_DDRPHYA_INITENG0_SequenceReg0b113s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9017f); // DWC_DDRPHYA_INITENG0_SequenceReg0b114s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90180); // DWC_DDRPHYA_INITENG0_SequenceReg0b114s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90181); // DWC_DDRPHYA_INITENG0_SequenceReg0b114s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90182); // DWC_DDRPHYA_INITENG0_SequenceReg0b115s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90183); // DWC_DDRPHYA_INITENG0_SequenceReg0b115s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90184); // DWC_DDRPHYA_INITENG0_SequenceReg0b115s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90185); // DWC_DDRPHYA_INITENG0_SequenceReg0b116s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90186); // DWC_DDRPHYA_INITENG0_SequenceReg0b116s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90187); // DWC_DDRPHYA_INITENG0_SequenceReg0b116s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90188); // DWC_DDRPHYA_INITENG0_SequenceReg0b117s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90189); // DWC_DDRPHYA_INITENG0_SequenceReg0b117s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9018a); // DWC_DDRPHYA_INITENG0_SequenceReg0b117s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90006); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90007); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90008); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90009); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9000a); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9000b); // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s2
    dwc_ddrphy_apb_rd_for_io_retention(
        0xd00e7); // DWC_DDRPHYA_APBONLY0_SequencerOverride
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90017); // DWC_DDRPHYA_INITENG0_StartVector0b0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9001f); // DWC_DDRPHYA_INITENG0_StartVector0b8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90026); // DWC_DDRPHYA_INITENG0_StartVector0b15
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400d0); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400d1); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400d2); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400d3); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400d4); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400d5); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400d6); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400d7); // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2000b); // DWC_DDRPHYA_MASTER0_Seq0BDLY0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2000c); // DWC_DDRPHYA_MASTER0_Seq0BDLY1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2000d); // DWC_DDRPHYA_MASTER0_Seq0BDLY2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x2000e); // DWC_DDRPHYA_MASTER0_Seq0BDLY3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9000c); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9000d); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9000e); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x9000f); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90010); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90011); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90012); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90013); // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20010); // DWC_DDRPHYA_MASTER0_PPTTrainSetup_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20011); // DWC_DDRPHYA_MASTER0_PPTTrainSetup2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40080); // DWC_DDRPHYA_ACSM0_AcsmPlayback0x0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40081); // DWC_DDRPHYA_ACSM0_AcsmPlayback1x0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40082); // DWC_DDRPHYA_ACSM0_AcsmPlayback0x1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40083); // DWC_DDRPHYA_ACSM0_AcsmPlayback1x1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40084); // DWC_DDRPHYA_ACSM0_AcsmPlayback0x2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x40085); // DWC_DDRPHYA_ACSM0_AcsmPlayback1x2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400fd); // DWC_DDRPHYA_ACSM0_AcsmCtrl13
    dwc_ddrphy_apb_rd_for_io_retention(0x10011); // DWC_DDRPHYA_DBYTE0_TsmByte1
    dwc_ddrphy_apb_rd_for_io_retention(0x10012); // DWC_DDRPHYA_DBYTE0_TsmByte2
    dwc_ddrphy_apb_rd_for_io_retention(0x10013); // DWC_DDRPHYA_DBYTE0_TsmByte3
    dwc_ddrphy_apb_rd_for_io_retention(0x10018); // DWC_DDRPHYA_DBYTE0_TsmByte5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10002); // DWC_DDRPHYA_DBYTE0_TrainingParam
    dwc_ddrphy_apb_rd_for_io_retention(0x100b2); // DWC_DDRPHYA_DBYTE0_Tsm0_i0
    dwc_ddrphy_apb_rd_for_io_retention(0x101b4); // DWC_DDRPHYA_DBYTE0_Tsm2_i1
    dwc_ddrphy_apb_rd_for_io_retention(0x102b4); // DWC_DDRPHYA_DBYTE0_Tsm2_i2
    dwc_ddrphy_apb_rd_for_io_retention(0x103b4); // DWC_DDRPHYA_DBYTE0_Tsm2_i3
    dwc_ddrphy_apb_rd_for_io_retention(0x104b4); // DWC_DDRPHYA_DBYTE0_Tsm2_i4
    dwc_ddrphy_apb_rd_for_io_retention(0x105b4); // DWC_DDRPHYA_DBYTE0_Tsm2_i5
    dwc_ddrphy_apb_rd_for_io_retention(0x106b4); // DWC_DDRPHYA_DBYTE0_Tsm2_i6
    dwc_ddrphy_apb_rd_for_io_retention(0x107b4); // DWC_DDRPHYA_DBYTE0_Tsm2_i7
    dwc_ddrphy_apb_rd_for_io_retention(0x108b4); // DWC_DDRPHYA_DBYTE0_Tsm2_i8
    dwc_ddrphy_apb_rd_for_io_retention(0x11011); // DWC_DDRPHYA_DBYTE1_TsmByte1
    dwc_ddrphy_apb_rd_for_io_retention(0x11012); // DWC_DDRPHYA_DBYTE1_TsmByte2
    dwc_ddrphy_apb_rd_for_io_retention(0x11013); // DWC_DDRPHYA_DBYTE1_TsmByte3
    dwc_ddrphy_apb_rd_for_io_retention(0x11018); // DWC_DDRPHYA_DBYTE1_TsmByte5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11002); // DWC_DDRPHYA_DBYTE1_TrainingParam
    dwc_ddrphy_apb_rd_for_io_retention(0x110b2); // DWC_DDRPHYA_DBYTE1_Tsm0_i0
    dwc_ddrphy_apb_rd_for_io_retention(0x111b4); // DWC_DDRPHYA_DBYTE1_Tsm2_i1
    dwc_ddrphy_apb_rd_for_io_retention(0x112b4); // DWC_DDRPHYA_DBYTE1_Tsm2_i2
    dwc_ddrphy_apb_rd_for_io_retention(0x113b4); // DWC_DDRPHYA_DBYTE1_Tsm2_i3
    dwc_ddrphy_apb_rd_for_io_retention(0x114b4); // DWC_DDRPHYA_DBYTE1_Tsm2_i4
    dwc_ddrphy_apb_rd_for_io_retention(0x115b4); // DWC_DDRPHYA_DBYTE1_Tsm2_i5
    dwc_ddrphy_apb_rd_for_io_retention(0x116b4); // DWC_DDRPHYA_DBYTE1_Tsm2_i6
    dwc_ddrphy_apb_rd_for_io_retention(0x117b4); // DWC_DDRPHYA_DBYTE1_Tsm2_i7
    dwc_ddrphy_apb_rd_for_io_retention(0x118b4); // DWC_DDRPHYA_DBYTE1_Tsm2_i8
    dwc_ddrphy_apb_rd_for_io_retention(0x12011); // DWC_DDRPHYA_DBYTE2_TsmByte1
    dwc_ddrphy_apb_rd_for_io_retention(0x12012); // DWC_DDRPHYA_DBYTE2_TsmByte2
    dwc_ddrphy_apb_rd_for_io_retention(0x12013); // DWC_DDRPHYA_DBYTE2_TsmByte3
    dwc_ddrphy_apb_rd_for_io_retention(0x12018); // DWC_DDRPHYA_DBYTE2_TsmByte5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12002); // DWC_DDRPHYA_DBYTE2_TrainingParam
    dwc_ddrphy_apb_rd_for_io_retention(0x120b2); // DWC_DDRPHYA_DBYTE2_Tsm0_i0
    dwc_ddrphy_apb_rd_for_io_retention(0x121b4); // DWC_DDRPHYA_DBYTE2_Tsm2_i1
    dwc_ddrphy_apb_rd_for_io_retention(0x122b4); // DWC_DDRPHYA_DBYTE2_Tsm2_i2
    dwc_ddrphy_apb_rd_for_io_retention(0x123b4); // DWC_DDRPHYA_DBYTE2_Tsm2_i3
    dwc_ddrphy_apb_rd_for_io_retention(0x124b4); // DWC_DDRPHYA_DBYTE2_Tsm2_i4
    dwc_ddrphy_apb_rd_for_io_retention(0x125b4); // DWC_DDRPHYA_DBYTE2_Tsm2_i5
    dwc_ddrphy_apb_rd_for_io_retention(0x126b4); // DWC_DDRPHYA_DBYTE2_Tsm2_i6
    dwc_ddrphy_apb_rd_for_io_retention(0x127b4); // DWC_DDRPHYA_DBYTE2_Tsm2_i7
    dwc_ddrphy_apb_rd_for_io_retention(0x128b4); // DWC_DDRPHYA_DBYTE2_Tsm2_i8
    dwc_ddrphy_apb_rd_for_io_retention(0x13011); // DWC_DDRPHYA_DBYTE3_TsmByte1
    dwc_ddrphy_apb_rd_for_io_retention(0x13012); // DWC_DDRPHYA_DBYTE3_TsmByte2
    dwc_ddrphy_apb_rd_for_io_retention(0x13013); // DWC_DDRPHYA_DBYTE3_TsmByte3
    dwc_ddrphy_apb_rd_for_io_retention(0x13018); // DWC_DDRPHYA_DBYTE3_TsmByte5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13002); // DWC_DDRPHYA_DBYTE3_TrainingParam
    dwc_ddrphy_apb_rd_for_io_retention(0x130b2); // DWC_DDRPHYA_DBYTE3_Tsm0_i0
    dwc_ddrphy_apb_rd_for_io_retention(0x131b4); // DWC_DDRPHYA_DBYTE3_Tsm2_i1
    dwc_ddrphy_apb_rd_for_io_retention(0x132b4); // DWC_DDRPHYA_DBYTE3_Tsm2_i2
    dwc_ddrphy_apb_rd_for_io_retention(0x133b4); // DWC_DDRPHYA_DBYTE3_Tsm2_i3
    dwc_ddrphy_apb_rd_for_io_retention(0x134b4); // DWC_DDRPHYA_DBYTE3_Tsm2_i4
    dwc_ddrphy_apb_rd_for_io_retention(0x135b4); // DWC_DDRPHYA_DBYTE3_Tsm2_i5
    dwc_ddrphy_apb_rd_for_io_retention(0x136b4); // DWC_DDRPHYA_DBYTE3_Tsm2_i6
    dwc_ddrphy_apb_rd_for_io_retention(0x137b4); // DWC_DDRPHYA_DBYTE3_Tsm2_i7
    dwc_ddrphy_apb_rd_for_io_retention(0x138b4); // DWC_DDRPHYA_DBYTE3_Tsm2_i8
    dwc_ddrphy_apb_rd_for_io_retention(0x20089); // DWC_DDRPHYA_MASTER0_CalZap
    dwc_ddrphy_apb_rd_for_io_retention(
        0xc0080); // DWC_DDRPHYA_DRTUB0_UcclkHclkEnables
    dwc_ddrphy_apb_rd_for_io_retention(
        0x200cb); // DWC_DDRPHYA_MASTER0_PllCtrl3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10068); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10069); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10168); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10169); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10268); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10269); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10368); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10369); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10468); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10469); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10568); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10569); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10668); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10669); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10768); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10769); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10868); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg0_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10869); // DWC_DDRPHYA_DBYTE0_RxPBDlyTg1_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x100aa); // DWC_DDRPHYA_DBYTE0_PptCtlStatic
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10062); // DWC_DDRPHYA_DBYTE0_TrainingIncDecDtsmEn_r0
    dwc_ddrphy_apb_rd_for_io_retention(0x10001); // DWC_DDRPHYA_DBYTE0_TsmByte0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11068); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11069); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11168); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11169); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11268); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11269); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11368); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11369); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11468); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11469); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11568); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11569); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11668); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11669); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11768); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11769); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11868); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg0_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11869); // DWC_DDRPHYA_DBYTE1_RxPBDlyTg1_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x110aa); // DWC_DDRPHYA_DBYTE1_PptCtlStatic
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11062); // DWC_DDRPHYA_DBYTE1_TrainingIncDecDtsmEn_r0
    dwc_ddrphy_apb_rd_for_io_retention(0x11001); // DWC_DDRPHYA_DBYTE1_TsmByte0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12068); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12069); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12168); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12169); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12268); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12269); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12368); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12369); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12468); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12469); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12568); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12569); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12668); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12669); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12768); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12769); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12868); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg0_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12869); // DWC_DDRPHYA_DBYTE2_RxPBDlyTg1_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x120aa); // DWC_DDRPHYA_DBYTE2_PptCtlStatic
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12062); // DWC_DDRPHYA_DBYTE2_TrainingIncDecDtsmEn_r0
    dwc_ddrphy_apb_rd_for_io_retention(0x12001); // DWC_DDRPHYA_DBYTE2_TsmByte0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13068); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13069); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13168); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13169); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13268); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13269); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13368); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13369); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13468); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13469); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13568); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13569); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13668); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13669); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13768); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13769); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13868); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg0_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13869); // DWC_DDRPHYA_DBYTE3_RxPBDlyTg1_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x130aa); // DWC_DDRPHYA_DBYTE3_PptCtlStatic
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13062); // DWC_DDRPHYA_DBYTE3_TrainingIncDecDtsmEn_r0
    dwc_ddrphy_apb_rd_for_io_retention(0x13001); // DWC_DDRPHYA_DBYTE3_TsmByte0
    dwc_ddrphy_apb_rd_for_io_retention(0x80); // DWC_DDRPHYA_ANIB0_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x1080); // DWC_DDRPHYA_ANIB1_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x2080); // DWC_DDRPHYA_ANIB2_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x3080); // DWC_DDRPHYA_ANIB3_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x4080); // DWC_DDRPHYA_ANIB4_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x5080); // DWC_DDRPHYA_ANIB5_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x6080); // DWC_DDRPHYA_ANIB6_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x7080); // DWC_DDRPHYA_ANIB7_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x8080); // DWC_DDRPHYA_ANIB8_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x9080); // DWC_DDRPHYA_ANIB9_ATxDly_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10020); // DWC_DDRPHYA_DBYTE0_DFIMRL_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10080); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10081); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x100d0); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x100d1); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1008c); // DWC_DDRPHYA_DBYTE0_RxClkDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1008d); // DWC_DDRPHYA_DBYTE0_RxClkDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10180); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10181); // DWC_DDRPHYA_DBYTE0_RxEnDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x101d0); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x101d1); // DWC_DDRPHYA_DBYTE0_TxDqsDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1018c); // DWC_DDRPHYA_DBYTE0_RxClkDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1018d); // DWC_DDRPHYA_DBYTE0_RxClkDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x100c0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x100c1); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x101c0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x101c1); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x102c0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x102c1); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x103c0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x103c1); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x104c0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r4_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x104c1); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r4_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x105c0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r5_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x105c1); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r5_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x106c0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r6_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x106c1); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r6_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x107c0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r7_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x107c1); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r7_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x108c0); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg0_r8_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x108c1); // DWC_DDRPHYA_DBYTE0_TxDqDlyTg1_r8_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x100ae); // DWC_DDRPHYA_DBYTE0_PptDqsCntInvTrnTg0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x100af); // DWC_DDRPHYA_DBYTE0_PptDqsCntInvTrnTg1_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x100a0); // DWC_DDRPHYA_DBYTE0_Dq0LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x100a1); // DWC_DDRPHYA_DBYTE0_Dq1LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x100a2); // DWC_DDRPHYA_DBYTE0_Dq2LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x100a3); // DWC_DDRPHYA_DBYTE0_Dq3LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x100a4); // DWC_DDRPHYA_DBYTE0_Dq4LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x100a5); // DWC_DDRPHYA_DBYTE0_Dq5LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x100a6); // DWC_DDRPHYA_DBYTE0_Dq6LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x100a7); // DWC_DDRPHYA_DBYTE0_Dq7LnSel
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11020); // DWC_DDRPHYA_DBYTE1_DFIMRL_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11080); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11081); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x110d0); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x110d1); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1108c); // DWC_DDRPHYA_DBYTE1_RxClkDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1108d); // DWC_DDRPHYA_DBYTE1_RxClkDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11180); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11181); // DWC_DDRPHYA_DBYTE1_RxEnDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x111d0); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x111d1); // DWC_DDRPHYA_DBYTE1_TxDqsDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1118c); // DWC_DDRPHYA_DBYTE1_RxClkDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1118d); // DWC_DDRPHYA_DBYTE1_RxClkDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x110c0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x110c1); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x111c0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x111c1); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x112c0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x112c1); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x113c0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x113c1); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x114c0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r4_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x114c1); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r4_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x115c0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r5_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x115c1); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r5_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x116c0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r6_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x116c1); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r6_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x117c0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r7_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x117c1); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r7_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x118c0); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg0_r8_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x118c1); // DWC_DDRPHYA_DBYTE1_TxDqDlyTg1_r8_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x110ae); // DWC_DDRPHYA_DBYTE1_PptDqsCntInvTrnTg0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x110af); // DWC_DDRPHYA_DBYTE1_PptDqsCntInvTrnTg1_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x110a0); // DWC_DDRPHYA_DBYTE1_Dq0LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x110a1); // DWC_DDRPHYA_DBYTE1_Dq1LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x110a2); // DWC_DDRPHYA_DBYTE1_Dq2LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x110a3); // DWC_DDRPHYA_DBYTE1_Dq3LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x110a4); // DWC_DDRPHYA_DBYTE1_Dq4LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x110a5); // DWC_DDRPHYA_DBYTE1_Dq5LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x110a6); // DWC_DDRPHYA_DBYTE1_Dq6LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x110a7); // DWC_DDRPHYA_DBYTE1_Dq7LnSel
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12020); // DWC_DDRPHYA_DBYTE2_DFIMRL_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12080); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12081); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x120d0); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x120d1); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1208c); // DWC_DDRPHYA_DBYTE2_RxClkDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1208d); // DWC_DDRPHYA_DBYTE2_RxClkDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12180); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12181); // DWC_DDRPHYA_DBYTE2_RxEnDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x121d0); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x121d1); // DWC_DDRPHYA_DBYTE2_TxDqsDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1218c); // DWC_DDRPHYA_DBYTE2_RxClkDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1218d); // DWC_DDRPHYA_DBYTE2_RxClkDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x120c0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x120c1); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x121c0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x121c1); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x122c0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x122c1); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x123c0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x123c1); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x124c0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r4_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x124c1); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r4_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x125c0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r5_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x125c1); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r5_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x126c0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r6_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x126c1); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r6_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x127c0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r7_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x127c1); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r7_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x128c0); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg0_r8_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x128c1); // DWC_DDRPHYA_DBYTE2_TxDqDlyTg1_r8_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x120ae); // DWC_DDRPHYA_DBYTE2_PptDqsCntInvTrnTg0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x120af); // DWC_DDRPHYA_DBYTE2_PptDqsCntInvTrnTg1_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x120a0); // DWC_DDRPHYA_DBYTE2_Dq0LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x120a1); // DWC_DDRPHYA_DBYTE2_Dq1LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x120a2); // DWC_DDRPHYA_DBYTE2_Dq2LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x120a3); // DWC_DDRPHYA_DBYTE2_Dq3LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x120a4); // DWC_DDRPHYA_DBYTE2_Dq4LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x120a5); // DWC_DDRPHYA_DBYTE2_Dq5LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x120a6); // DWC_DDRPHYA_DBYTE2_Dq6LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x120a7); // DWC_DDRPHYA_DBYTE2_Dq7LnSel
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13020); // DWC_DDRPHYA_DBYTE3_DFIMRL_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13080); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13081); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x130d0); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x130d1); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1308c); // DWC_DDRPHYA_DBYTE3_RxClkDlyTg0_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1308d); // DWC_DDRPHYA_DBYTE3_RxClkDlyTg1_u0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13180); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13181); // DWC_DDRPHYA_DBYTE3_RxEnDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x131d0); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x131d1); // DWC_DDRPHYA_DBYTE3_TxDqsDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1318c); // DWC_DDRPHYA_DBYTE3_RxClkDlyTg0_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x1318d); // DWC_DDRPHYA_DBYTE3_RxClkDlyTg1_u1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x130c0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x130c1); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x131c0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x131c1); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x132c0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x132c1); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x133c0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x133c1); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x134c0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r4_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x134c1); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r4_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x135c0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r5_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x135c1); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r5_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x136c0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r6_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x136c1); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r6_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x137c0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r7_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x137c1); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r7_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x138c0); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg0_r8_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x138c1); // DWC_DDRPHYA_DBYTE3_TxDqDlyTg1_r8_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x130ae); // DWC_DDRPHYA_DBYTE3_PptDqsCntInvTrnTg0_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x130af); // DWC_DDRPHYA_DBYTE3_PptDqsCntInvTrnTg1_p0
    dwc_ddrphy_apb_rd_for_io_retention(0x130a0); // DWC_DDRPHYA_DBYTE3_Dq0LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x130a1); // DWC_DDRPHYA_DBYTE3_Dq1LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x130a2); // DWC_DDRPHYA_DBYTE3_Dq2LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x130a3); // DWC_DDRPHYA_DBYTE3_Dq3LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x130a4); // DWC_DDRPHYA_DBYTE3_Dq4LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x130a5); // DWC_DDRPHYA_DBYTE3_Dq5LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x130a6); // DWC_DDRPHYA_DBYTE3_Dq6LnSel
    dwc_ddrphy_apb_rd_for_io_retention(0x130a7); // DWC_DDRPHYA_DBYTE3_Dq7LnSel
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90201); // DWC_DDRPHYA_INITENG0_Seq0BGPR1_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90202); // DWC_DDRPHYA_INITENG0_Seq0BGPR2_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90203); // DWC_DDRPHYA_INITENG0_Seq0BGPR3_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90205); // DWC_DDRPHYA_INITENG0_Seq0BGPR5_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90206); // DWC_DDRPHYA_INITENG0_Seq0BGPR6_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90207); // DWC_DDRPHYA_INITENG0_Seq0BGPR7_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x90208); // DWC_DDRPHYA_INITENG0_Seq0BGPR8_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20020); // DWC_DDRPHYA_MASTER0_HwtMRL_p0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20077); // DWC_DDRPHYA_MASTER0_HwtCAMode
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20072); // DWC_DDRPHYA_MASTER0_HwtLpCsEnA
    dwc_ddrphy_apb_rd_for_io_retention(
        0x20073); // DWC_DDRPHYA_MASTER0_HwtLpCsEnB
    dwc_ddrphy_apb_rd_for_io_retention(
        0x400c0); // DWC_DDRPHYA_ACSM0_AcsmCtrl23
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10040); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10140); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10240); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10340); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10440); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10540); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10640); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10740); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x10840); // DWC_DDRPHYA_DBYTE0_VrefDAC0_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11040); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11140); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11240); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11340); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11440); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11540); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11640); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11740); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x11840); // DWC_DDRPHYA_DBYTE1_VrefDAC0_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12040); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12140); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12240); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12340); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12440); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12540); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12640); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12740); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x12840); // DWC_DDRPHYA_DBYTE2_VrefDAC0_r8
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13040); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r0
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13140); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r1
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13240); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r2
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13340); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r3
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13440); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r4
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13540); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r5
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13640); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r6
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13740); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r7
    dwc_ddrphy_apb_rd_for_io_retention(
        0x13840); // DWC_DDRPHYA_DBYTE3_VrefDAC0_r8
// // Disabling Ucclk (PMU)
    dwc_ddrphy_apb_wr(0xc0080, 0x2); // DWC_DDRPHYA_DRTUB0_UcclkHclkEnables
    dwc_ddrphy_apb_wr(0xd0000, 0x1); // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
// // [dwc_ddrphy_phyinit_userCustom_saveRetRegs] End of dwc_ddrphy_phyinit_userCustom_saveRetRegs()
//printf("dwc_ddrphy_phyinit_userCustom_saveRetRegs end.\n");
}
int c_index = 0;
static void dwc_ddrc_csr_rd_for_io_retention(int phy_addr, int val)
{
    str_info.ddrc_csr[c_index] = readl(APB_DDRCTRL_BASE + phy_addr);
    c_index++;
//ASSERT(c_index<=str_info.save_c_index);
}
static void save_ddrc_csr(void)
{
    //4266
    c_index = 0;
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DBG1_OFF, 0x1);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_PWRCTL_OFF, 0x1);
//DDR_R32(APB_DDRCTRL_BASE+UMCTL2_REGS_STAT_OFF);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_MRCTRL0_OFF, 0x40003030);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_MRCTRL1_OFF, 0x370d5);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_MRCTRL2_OFF, 0xeb74135c);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_MSTR2_OFF, 0x1);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_DERATECTL_OFF, 0x0);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_PWRCTL_OFF, 0x20);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_HWLPCTL_OFF, 0xac0002);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ECCCFG0_OFF, 0x53f7f50);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ECCCFG1_OFF, 0x7b2);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ECCCTL_OFF, 0x300);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ECCPOISONADDR0_OFF,
                                     0x1000fc0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ECCPOISONADDR1_OFF,
                                     0x20019294);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_CRCPARCTL0_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_CRCPARCTL1_OFF, 0x1000);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DIMMCTL_OFF, 0x20);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFIMISC_OFF, 0x41);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP0_OFF, 0x18);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP1_OFF, 0x50505);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP2_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP3_OFF, 0x3030300);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP4_OFF, 0x1f1f);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP5_OFF, 0x70f0707);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP6_OFF, 0x7070707);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP7_OFF, 0xf07);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP8_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP9_OFF, 0x7070707);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP10_OFF, 0x7070707);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADDRMAP11_OFF, 0x7);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ODTMAP_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_SCHED_OFF, 0x8b9f00);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_SCHED1_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_PERFHPR1_OFF, 0xf00003f);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_PERFLPR1_OFF, 0xf0003ff);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_PERFWR1_OFF, 0xf0003ff);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_DBG0_OFF, 0x0);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DBG1_OFF, 0x0);
    DDR_W32(APB_DDRCTRL_BASE + UMCTL2_REGS_DBGCMD_OFF, 0x0);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_SWCTL_OFF, 0x1);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_SWCTLSTATIC_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_OCPARCFG1_OFF, 0x844);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_POISONCFG_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ADVECCINDEX_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ECCPOISONPAT0_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ECCPOISONPAT2_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_REGPARCFG_OFF, 0x3);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_OCCAPCFG_OFF, 0x0);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_OCCAPCFG1_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_MP_PCTRL_0_OFF, 0x1);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_MSTR_OFF, 0x83080020);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DERATEEN_OFF, 0x1415);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DERATEINT_OFF, 0x439b2345);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_PWRTMG_OFF, 0xe1102);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_RFSHCTL0_OFF, 0x218000);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_RFSHCTL1_OFF, 0x19000a);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_RFSHCTL3_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_RFSHTMG_OFF, 0x82012c);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_RFSHTMG1_OFF, 0x610000);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_INIT0_OFF, 0xc0030828);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_INIT1_OFF, 0xd1000a);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_INIT2_OFF, 0x8d05);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_INIT3_OFF, 0x74003f);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_INIT4_OFF, 0xf30008);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_INIT5_OFF, 0x30007);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_INIT6_OFF, 0x64004d);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_INIT7_OFF, 0x40000);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_RANKCTL_OFF, 0xe32f);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG0_OFF, 0x2121482d);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG1_OFF, 0x90941);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG2_OFF, 0x9141c1d);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG3_OFF, 0xf0f006);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG4_OFF, 0x14040914);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG5_OFF, 0x2061111);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG6_OFF, 0x20b000a);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG7_OFF, 0x602);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG8_OFF, 0x1014501);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG9_OFF, 0x21);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG10_OFF, 0xe0007);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG11_OFF, 0x7f01001b);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG12_OFF, 0x20000);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG13_OFF, 0xe100002);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG14_OFF, 0x4e1);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DRAMTMG15_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ZQCTL0_OFF, 0x542d0021);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ZQCTL1_OFF, 0x3600070);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ZQCTL2_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_ODTCFG_OFF, 0x6070e74);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFITMG0_OFF, 0x4a3820e);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFITMG1_OFF, 0xb0303);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFILPCFG0_OFF, 0x3c0a020);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFILPCFG1_OFF, 0x70);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFIUPD0_OFF, 0x60400018);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFIUPD1_OFF, 0x8000b2);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFIUPD2_OFF, 0x80000000);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFITMG2_OFF, 0x230e);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFITMG3_OFF, 0x17);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DBICTL_OFF, 0x7);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_DFIPHYMSTR_OFF, 0x1);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_REGS_OCPARCFG0_OFF, 0xb02003);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_MP_PCCFG_OFF, 0x0);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_MP_PCFGR_0_OFF, 0x2b7);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_MP_PCFGW_0_OFF, 0x40bb);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_OCCAPCFG_OFF, 0x10001);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_OCCAPCFG1_OFF, 0x10001);
    dwc_ddrc_csr_rd_for_io_retention(UMCTL2_MP_PCFGQOS0_0_OFF, 0x1110e00);
//DDR_R32(APB_DDRCTRL_BASE+UMCTL2_REGS_RFSHCTL3_OFF);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_DBG1_OFF, 0x0);
//DDR_R32(APB_DDRCTRL_BASE+UMCTL2_REGS_PWRCTL_OFF);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_PWRCTL_OFF, 0x20);
//DDR_R32(APB_DDRCTRL_BASE+UMCTL2_REGS_PWRCTL_OFF);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_PWRCTL_OFF, 0x20);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_SWCTL_OFF, 0x0);
//DDR_POLL_BITS(APB_DDRCTRL_BASE+UMCTL2_REGS_SWSTAT_OFF, 0x1, 0x0, 0x64);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_DFIMISC_OFF, 0x40);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_DFIMISC_OFF, 0x40);
//DDR_W32(APB_DDRCTRL_BASE+UMCTL2_REGS_DFIMISC_OFF, 0x40);
}
static void ddr_enter_self(void)
{
    unsigned int rdata;
    unsigned int addr;
    unsigned int wdata;
//  unsigned int addr_index;
    unsigned int umctl_base;
    //unsigned int phy_base;
    //int count=1000;
    str_info.save_csr_index = 0;
    str_info.save_c_index = 0;
    str_info.restore_csr_index = 0;
    //ddrc_timing_reg_save();
    save_ddrc_csr();
    //save_dwc_ddrphy_phyinit_C_initPhyConfig();
    umctl_base = APB_DDRCTRL_BASE;
    //phy_base   = APB_DDRPHY_BASE;
    //sys_tb_ctrl_fsdbDumpon();
    //save_ret_csrs();
    dwc_ddrphy_phyinit_userCustom_saveRetRegs();
    // step1: TODO: DDR PHY IO retention sequence step a/b
    //SWCTL:sw_done enable quasi-dynamic programming
    addr = umctl_base + 0x00000320;
    wdata = 0x00000000;
    writel(wdata, addr);
    //PCTRL_n port_en enables AXI port n
    addr = umctl_base + 0x00000490;
    wdata = 0x00000000;
    writel(wdata, addr);
    //PSTAT port status register [31:0]->wr_port_busy_15...0,rd_port_busy_15...0
    addr = umctl_base + 0x000003fc;
    rdata = readl(addr);

    while (rdata != 0) {
        rdata = readl(addr);
    }

    //SBRCTL:scrubber control register
    addr = umctl_base + 0x00000f24;
    wdata = 0x0000ff10;
    writel(wdata, addr);
    //SBRSTAT:scrubber status register
    addr = umctl_base + 0x00000f28;
    rdata = readl(addr);

    while (rdata != 0) {
        rdata = readl(addr);
    }

    //why two times
    addr = umctl_base + 0x00000f28;
    rdata = readl(addr);

    while (rdata != 0) {
        rdata = readl(addr);
    }

    //PWRCTL
    addr = umctl_base + 0x00000030;
    rdata = readl(addr);
    //selfref_sw=1
    rdata = rdata | 0x20;
    writel(rdata, addr);
    //STAT
    addr = umctl_base + 0x000004;
    rdata = readl(addr);

    //10 entry self_refresh power down
    while ((rdata & 0x220) != 0x220) {
        rdata = readl(addr);
    }

    //DFIMISC
    addr = umctl_base + 0x000001b0;
    wdata = 0x00001f50;
    writel(wdata, addr);
    addr = umctl_base + 0x000001b0;
    wdata = 0x00001f70;
    writel(wdata, addr);
    //DFISTAT
    addr = umctl_base + 0x000001bc;
    rdata = readl(addr);

    while (rdata != 0x0) {
        rdata = readl(addr);
    }

    //SWCTL:sw_done
    addr = umctl_base + 0x00000320;
    wdata = 0x00000001;
    writel(wdata, addr);
    //PWRCTL
    addr = umctl_base + 0x00000030;
    wdata = 0x0000002a;
    writel(wdata, addr);
    //STAT
    addr = umctl_base + 0x00000004;
    rdata = readl(addr);

    while (rdata != 0x223) {
        rdata = readl(addr);
    }

    //SWCTL:sw_done
    addr = umctl_base + 0x00000320;
    wdata = 0x00000000;
    writel(wdata, addr);
    //DFIMISC
    addr = umctl_base + 0x000001b0;
    wdata = 0x00001f50;
    writel(wdata, addr);
    //DFISTAT
    addr = umctl_base + 0x000001bc;
    rdata = readl(addr);

    while (rdata != 0x1) {
        rdata = readl(addr);
    }

    //SWCTL:sw_done
    addr = umctl_base + 0x00000320;
    wdata = 0x00000001;
    writel(wdata, addr);
    set_scr_saf_ddr_ss_pwrokin_aon(0x0);
    /*
       rstgen_sec_iso_b(DDR_TO_AP_ISO_B_IDX,0x0);

       rstgen_sec_module_rst(RSTGEN_SEC_MODULE_RST_B_DDR_SS_SW_DDR_CORE_RSTN_INDEX,0x0);
       rstgen_sec_module_rst(RSTGEN_SEC_MODULE_RST_B_DDR_SS_SW_AXI_RSTN_INDEX,0x0);
       rstgen_sec_module_rst(RSTGEN_SEC_MODULE_RST_B_DDR_SS_SW_APB_RSTN_INDEX,0x0);
       rstgen_sec_module_rst(RSTGEN_SEC_MODULE_RST_B_DDR_SS_RST_N_INDEX,0x0);
      */
}
void str_enter(void)
{
    uint32_t base = 0xF1850000;
    ddr_enter_self();
#if SUPPORT_BOARDINFO
    //config_eeprom_pin(0);
    //set_save_csr_index((int*)&save_csr_index);
    //set_restore_csr_index((int*)&restore_csr_index);
    //for(i=0;i<10;i++)
    //  printf("ddrc_csr[%d]=0x%x\n", i, ddrc_csr[i]);
    //set_ddrc_csr((void*)&ddrc_csr[0]);
    //config_eeprom_pin(1);
    //save_csr_index_test = get_save_csr_index();
    //printf("save_csr_index 0x%x\n", save_csr_index);
    //get_ddrc_csr((void*)&ddrc_csr_test[0]);
    //printf("phy_csr[]={\n");
    //for(i=0;i<(int)save_csr_index;i++) {
    //        printf("0x%08x,",phy_csr[i]);
    //  if((i % 3) == 0)
    //      printf("\n");
    //}
    //printf("};\n");
    //while(1);
    //while(test);
    //set_phy_csr((int*)PHY_CSR_BASE);
#endif
    str_info.freq = ((readl(STR_SAVED_FREQ) & 0xffff0000) >> 16);
    save_str_info(&str_info);
    RMWREG32(base + PMU_CTRL_0, 8, 1, 0x0);
    spin(1000);
    RMWREG32(base + PMU_CTRL_0, 8, 1, 0x1);

    while (1);
}
#endif //DDR_ENTER_SELF

#if !DDR_ENTER_SELF && !STATIC_HANDOVER
#include "str.h"
#if SUPPORT_STR_MODE
static uint32_t vote_flag = 0;
bool is_str_enter(enum STR_MASTER master)
{
    uint32_t v;
    uint32_t base;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return false;
    }

    v = readl(base);

    if ((v & 0xffff) == 0x1234)
        return true;

    return false;
}
bool is_str_resume(enum STR_MASTER master)
{
    uint32_t v;
    uint32_t base;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return false;
    }

    v = readl(base);

    if ((v & 0xffff) == 0x8765)
        return true;

    return false;
}
void clr_str_flag(enum STR_MASTER master)
{
    uint32_t base;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            writel(0x0, STR_RTC_FLAG);//AP1 set rtc flag only
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return;
    }

    writel(0, base);
}
void set_str_flag_to_rtc(enum STR_MASTER master)
{
    uint32_t v;
    uint32_t base;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            writel(0x12345678, STR_RTC_FLAG);//AP1 set rtc flag only
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return;
    }

    v = readl(base);
    v &= 0xffff0000;
    v |= 0x8765;
    writel(v, base);
}
uint32_t get_str_resume_entry(enum STR_MASTER master)
{
    uint32_t base;

    switch (master) {
        case STR_AP1:
            base = STR_AP1_TO_SAF;
            break;

        case STR_AP2:
            base = STR_AP2_TO_SAF;
            break;

        default:
            return 0;
    }

    return readl(base + 0x4);
}
void str_vote(enum STR_MASTER master)
{
    vote_flag |= 1 << master;
}
bool check_str_all(void)
{
    return (vote_flag == STR_VOTE_ALL);
}
void str_clr_vote(void)
{
    vote_flag = 0;
}
#else
bool is_str_enter(enum STR_MASTER master) {return false;}
bool is_str_resume(enum STR_MASTER master) {return false;}
uint32_t get_str_resume_entry(enum STR_MASTER master) {return 0;}
void clr_str_flag(enum STR_MASTER master) {}
void set_str_flag_to_rtc(enum STR_MASTER master) {}
void str_vote(enum STR_MASTER master) {}
bool check_str_all(void) {return false;}
void str_clr_vote(void) {}
#endif
#endif
