/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 *******************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#include <common_hdr.h>
#include <compiler.h>
#include <soc.h>
#include <assert.h>
#include <debug.h>
#include <dw_umctl2.h>
#include <wchar.h>
#include "partition_parser.h"
#define DDR_FW_BUF_SZ (DEFAULT_BLK_SZ * 2)

extern const char *find_streaming_string(uint32_t fw, uint32_t id);

extern uint8_t ddrphy_fw_lpddr4x_dmem[];
extern uint8_t ddrphy_fw_lpddr4x_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4x_imem[];
extern uint8_t ddrphy_fw_lpddr4x_imem_end[];
extern uint8_t ddrphy_fw_lpddr4x_2d_dmem[];
extern uint8_t ddrphy_fw_lpddr4x_2d_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4x_2d_imem[];
extern uint8_t ddrphy_fw_lpddr4x_2d_imem_end[];

extern uint8_t ddrphy_fw_lpddr4_dmem[];
extern uint8_t ddrphy_fw_lpddr4_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4_imem[];
extern uint8_t ddrphy_fw_lpddr4_imem_end[];
extern uint8_t ddrphy_fw_lpddr4_2d_dmem[];
extern uint8_t ddrphy_fw_lpddr4_2d_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4_2d_imem[];
extern uint8_t ddrphy_fw_lpddr4_2d_imem_end[];

extern uint8_t ddrphy_fw_ddr4_dmem[];
extern uint8_t ddrphy_fw_ddr4_dmem_end[];
extern uint8_t ddrphy_fw_ddr4_imem[];
extern uint8_t ddrphy_fw_ddr4_imem_end[];
extern uint8_t ddrphy_fw_ddr4_2d_dmem[];
extern uint8_t ddrphy_fw_ddr4_2d_dmem_end[];
extern uint8_t ddrphy_fw_ddr4_2d_imem[];
extern uint8_t ddrphy_fw_ddr4_2d_imem_end[];

extern uint8_t ddrphy_fw_ddr3_dmem[];
extern uint8_t ddrphy_fw_ddr3_dmem_end[];
extern uint8_t ddrphy_fw_ddr3_imem[];
extern uint8_t ddrphy_fw_ddr3_imem_end[];

uint8_t  __attribute__((section("ddr_fw_sec"))) __attribute__((
            used)) ddrphy_fw_dummy;//do not del this dummy

#ifdef DDR_DIAG
extern uint8_t ddrphy_fw_lpddr4x_diag_dmem[];
extern uint8_t ddrphy_fw_lpddr4x_diag_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4x_diag_imem[];
extern uint8_t ddrphy_fw_lpddr4x_diag_imem_end[];
extern uint8_t ddrphy_fw_lpddr4_diag_dmem[];
extern uint8_t ddrphy_fw_lpddr4_diag_dmem_end[];
extern uint8_t ddrphy_fw_lpddr4_diag_imem[];
extern uint8_t ddrphy_fw_lpddr4_diag_imem_end[];
extern uint8_t ddrphy_fw_ddr4_diag_dmem[];
extern uint8_t ddrphy_fw_ddr4_diag_dmem_end[];
extern uint8_t ddrphy_fw_ddr4_diag_imem[];
extern uint8_t ddrphy_fw_ddr4_diag_imem_end[];
uint8_t  __attribute__((section("ddr_diag_sec"))) __attribute__((
            used)) ddrphy_diag_dummy;//do not del this dummy
#endif

extern void *ddrphy_apb_memcpy(uint32_t *to, uint16_t *from, size_t sz);

static uint32_t ddr_get_data_field(uint32_t data, uint32_t offset, uint32_t width)
{
    return ((data << (32 - offset - width)) >> (32 - width));
}

static uint32_t ddrc_rd_bits(uint32_t addr, uint32_t offset, uint32_t width)
{
    uint32_t v = readl(addr);
    return ddr_get_data_field(v, offset, width);
}

static inline void ddrc_wr_bits(uint32_t addr, uint32_t offset, uint32_t width, uint32_t v)
{
    RMWREG32(addr, offset, width, v);
}

static inline void ddrc_wr(uint32_t addr, uint32_t v)
{
    writel(v, addr);
}

static unsigned char max_abs_4(signed char A, signed char B, signed char C, signed char D)
{
    unsigned char A1;
    unsigned char B1;
    unsigned char C1;
    unsigned char D1;
    unsigned char max;

    A1 = (A > 0) ? A : (-1 * A);
    B1 = (B > 0) ? B : (-1 * B);
    C1 = (C > 0) ? C : (-1 * C);
    D1 = (D > 0) ? D : (-1 * D);

    max = A1;
    max = (B1 >= max) ? B1 : max;
    max = (C1 >= max) ? C1 : max;
    max = (D1 >= max) ? D1 : max;

    return max;
}

static int32_t max_abs_4int(int A, int B, int C, int D)
{
    int max;

    max = A;
    max = (B >= max) ? B : max;
    max = (C >= max) ? C : max;
    max = (D >= max) ? D : max;

    return max;
}

static unsigned char max_abs_2(signed char A, signed char B)
{
    unsigned char A1;
    unsigned char B1;
    unsigned char max;

    A1 = (A > 0) ? A : (-1 * A);
    B1 = (B > 0) ? B : (-1 * B);

    max = A1;
    max = (B1 >= max) ? B1 : max;

    return max;
}

static unsigned char max2(unsigned char A, unsigned char B)
{
    unsigned char max;

    max = A;
    max = (B >= max) ? B : max;

    return max;
}

static int dwc_ddrphy_apb_rd(int phy_addr)
{
    return readl(APB_DDRPHY_BASE + phy_addr * 4);
}

static void dwc_ddrphy_apb_wr(int phy_addr, int phy_data)
{
    int w_addr = APB_DDRPHY_BASE + phy_addr * 4;
    int w_data = phy_data;
    writel(w_data, w_addr);
}

static int dwc_ddrphy_get_mail(int mode)
{
    int mail;

    while ((0x1 & dwc_ddrphy_apb_rd(0x000d0004)) != 0); //UctWriteProtShadow

    mail = dwc_ddrphy_apb_rd(0x000d0032);

    if (mode == 32) {
        mail |= (dwc_ddrphy_apb_rd(0x000d0034) << 16);
    }

    dwc_ddrphy_apb_wr(0x000d0031, 0);

    while ((0x1 & dwc_ddrphy_apb_rd(0x000d0004)) == 0) {;} //UctWriteProtShadow

    dwc_ddrphy_apb_wr(0x000d0031, 1);
    return mail;
}

#if defined(DEBUG_ENABLE) && defined(DDR_DEBUG)
static const char *ddrphy_decode_major_message(int major_info)
{
    return (major_info == 0x00 ? "End of initialization" :
            major_info == 0x01 ? "End of fine write leveling" :
            major_info == 0x02 ? "End of read enable training" :
            major_info == 0x03 ? "End of read delay center optimization" :
            major_info == 0x04 ? "End of write dealy center optimization" :
            major_info == 0x05 ? "End of 2D read delay/voltage center optimization" :
            major_info == 0x06 ? "End of 2D write delay/voltage center optimization" :
            major_info == 0x07 ? "Training has run successfully" :
            major_info == 0x08 ? "Streaming message" :
            major_info == 0x09 ? "End of max read latency training" :
            major_info == 0x0a ? "End of read dq deskew training" :
            major_info == 0x0b ? "Reserved" :
            major_info == 0x0c ? "End of all DB training" :
            major_info == 0x0d ? "End of CA training" :
            major_info == 0xfd ? "End of MPR read delay center optimization" :
            major_info == 0xfe ? "End of Write leveling coarse delay" :
            major_info == 0xff ? "Training has failed" : "NOT defined major message");
}
#endif

static volatile int para[32];
static void parse_streaming_msg(uint32_t fw)
{
    int string_id = dwc_ddrphy_get_mail(32);
    int para_N = string_id & 0xffff;

    if (para_N > 32) {
        para_N = 32;
    }

    for (int i = 0; i < para_N; i++) {
        para[i] = dwc_ddrphy_get_mail(32);
    }

#if defined(DDR_DEBUG)
    const char *fmt = find_streaming_string(fw, string_id);

    switch (para_N) {
    case 0:
        DDR_DBG(fmt);
        break;

    case 1:
        DDR_DBG(fmt, para[0]);
        break;

    case 2:
        DDR_DBG(fmt, para[0], para[1]);
        break;

    case 3:
        DDR_DBG(fmt, para[0], para[1], para[2]);
        break;

    case 4:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3]);
        break;

    case 5:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4]);
        break;

    case 6:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5]);
        break;

    case 7:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6]);
        break;

    case 8:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7]);
        break;

    case 9:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                para[8]);
        break;

    case 10:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                para[8], para[9]);
        break;

    case 11:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                para[8], para[9], para[10]);
        break;

    case 12:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                para[8], para[9], para[10], para[11]);
        break;

    case 31:
        DDR_DBG(fmt, para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7],
                para[8], para[9], para[10], para[11], para[12], para[13], para[14], para[15],
                para[16], para[17], para[18], para[19], para[20], para[21], para[22], para[23],
                para[24], para[25], para[26], para[27], para[28], para[29], para[30]);
        break;

    default:
        DDR_DBG("%s: %d paras, not supported yet\n", __FUNCTION__, para_N);
        break;
    }

#endif  /* defined(DDR_DEBUG) */
}

uint32_t dwc_ddrphy_fw_monitor_loop(uint32_t fw)
{
    int major_info;

    DDR_DBG("Entering fw mointor loop...\n");

    while (1) {
        major_info = dwc_ddrphy_get_mail(16);

        if (0x08u == major_info) {  /* Streaming message */
            parse_streaming_msg(fw);
        } else {
            DDR_DBG("ddrphy fw mail:0x%x:%s\n", major_info,
                    ddrphy_decode_major_message(major_info));
        }

        if ( (major_info == 0x07) || (major_info == 0xff)) {
            break;
        }
    }

    if (major_info == 0xff) {
        DBG("%s: Opps: DDR Training Failed.\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

void *ddrphy_apb_memcpy(uint32_t *to, uint16_t *from, size_t sz)
{
    uint32_t v = 0;
    void *p = (void *)to;

    //assert((NULL != to) && (NULL != from) && (sz % 2 == 0));

    while (sz) {
        v = *from;
        *to = (uint16_t)v;
        from++;
        to++;
        sz -= 2;
    }

    return p;
}

/* variables to save training result then update to ctrl regs */
static signed char CDD_ChA_RR_1_0;
static signed char CDD_ChA_RR_0_1;
static signed char CDD_ChA_RW_1_1;
static signed char CDD_ChA_RW_1_0;
static signed char CDD_ChA_RW_0_1;
static signed char CDD_ChA_RW_0_0;
static signed char CDD_ChA_WR_1_1;
static signed char CDD_ChA_WR_1_0;
static signed char CDD_ChA_WR_0_1;
static signed char CDD_ChA_WR_0_0;
static signed char CDD_ChA_WW_1_0;
static signed char CDD_ChA_WW_0_1;

static signed char CDD_ChB_RR_1_0;
static signed char CDD_ChB_RR_0_1;
static signed char CDD_ChB_RW_1_1;
static signed char CDD_ChB_RW_1_0;
static signed char CDD_ChB_RW_0_1;
static signed char CDD_ChB_RW_0_0;
static signed char CDD_ChB_WR_1_1;
static signed char CDD_ChB_WR_1_0;
static signed char CDD_ChB_WR_0_1;
static signed char CDD_ChB_WR_0_0;
static signed char CDD_ChB_WW_1_0;
static signed char CDD_ChB_WW_0_1;

/* variables for DDR4 */
static signed char CDD_RR_3_2;
static signed char CDD_RR_3_1;
static signed char CDD_RR_3_0;
static signed char CDD_RR_2_3;
static signed char CDD_RR_2_1;
static signed char CDD_RR_2_0;
static signed char CDD_RR_1_3;
static signed char CDD_RR_1_2;
static signed char CDD_RR_1_0;
static signed char CDD_RR_0_3;
static signed char CDD_RR_0_2;
static signed char CDD_RR_0_1;
static signed char CDD_WW_3_2;
static signed char CDD_WW_3_1;
static signed char CDD_WW_3_0;
static signed char CDD_WW_2_3;
static signed char CDD_WW_2_1;
static signed char CDD_WW_2_0;
static signed char CDD_WW_1_3;
static signed char CDD_WW_1_2;
static signed char CDD_WW_1_0;
static signed char CDD_WW_0_3;
static signed char CDD_WW_0_2;
static signed char CDD_WW_0_1;
static signed char CDD_RW_3_3;
static signed char CDD_RW_3_2;
static signed char CDD_RW_3_1;
static signed char CDD_RW_3_0;
static signed char CDD_RW_2_3;
static signed char CDD_RW_2_2;
static signed char CDD_RW_2_1;
static signed char CDD_RW_2_0;
static signed char CDD_RW_1_3;
static signed char CDD_RW_1_2;
static signed char CDD_RW_1_1;
static signed char CDD_RW_1_0;
static signed char CDD_RW_0_3;
static signed char CDD_RW_0_2;
static signed char CDD_RW_0_1;
static signed char CDD_RW_0_0;
static signed char CDD_WR_3_3;
static signed char CDD_WR_3_2;
static signed char CDD_WR_3_1;
static signed char CDD_WR_3_0;
static signed char CDD_WR_2_3;
static signed char CDD_WR_2_2;
static signed char CDD_WR_2_1;
static signed char CDD_WR_2_0;
static signed char CDD_WR_1_3;
static signed char CDD_WR_1_2;
static signed char CDD_WR_1_1;
static signed char CDD_WR_1_0;
static signed char CDD_WR_0_3;
static signed char CDD_WR_0_2;
static signed char CDD_WR_0_1;
static signed char CDD_WR_0_0;

static void _read_1d_message_block_outputs_(void)
{
    int rdata;

    //CHANNEL A
    rdata = dwc_ddrphy_apb_rd(0x54013);
    CDD_ChA_RR_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_RR_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54014);
    CDD_ChA_RW_1_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_RW_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54015);
    CDD_ChA_RW_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_RW_0_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54016);
    CDD_ChA_WR_1_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_WR_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54017);
    CDD_ChA_WR_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_WR_0_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54018);
    CDD_ChA_WW_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChA_WW_0_1 = ddr_get_data_field(rdata, 8, 8);

    //CHANNEL B
    rdata = dwc_ddrphy_apb_rd(0x5402c);
    CDD_ChB_RR_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402d);
    CDD_ChB_RR_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_RW_1_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402e);
    CDD_ChB_RW_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_RW_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402f);
    CDD_ChB_RW_0_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_WR_1_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54030);
    CDD_ChB_WR_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_WR_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54031);
    CDD_ChB_WR_0_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_ChB_WW_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54032);
    CDD_ChB_WW_0_1 = ddr_get_data_field(rdata, 0, 8);

    DDR_DBG("CDD_ChA_RR_1_0 = %d\n", CDD_ChA_RR_1_0);
    DDR_DBG("CDD_ChA_RR_0_1 = %d\n", CDD_ChA_RR_0_1);
    DDR_DBG("CDD_ChA_RW_1_1 = %d\n", CDD_ChA_RW_1_1);
    DDR_DBG("CDD_ChA_RW_1_0 = %d\n", CDD_ChA_RW_1_0);
    DDR_DBG("CDD_ChA_RW_0_1 = %d\n", CDD_ChA_RW_0_1);
    DDR_DBG("CDD_ChA_RW_0_0 = %d\n", CDD_ChA_RW_0_0);
    DDR_DBG("CDD_ChA_WR_1_1 = %d\n", CDD_ChA_WR_1_1);
    DDR_DBG("CDD_ChA_WR_1_0 = %d\n", CDD_ChA_WR_1_0);
    DDR_DBG("CDD_ChA_WR_0_1 = %d\n", CDD_ChA_WR_0_1);
    DDR_DBG("CDD_ChA_WR_0_0 = %d\n", CDD_ChA_WR_0_0);
    DDR_DBG("CDD_ChA_WW_1_0 = %d\n", CDD_ChA_WW_1_0);
    DDR_DBG("CDD_ChA_WW_0_1 = %d\n", CDD_ChA_WW_0_1);

    DDR_DBG("CDD_ChB_RR_1_0 = %d\n", CDD_ChB_RR_1_0);
    DDR_DBG("CDD_ChB_RR_0_1 = %d\n", CDD_ChB_RR_0_1);
    DDR_DBG("CDD_ChB_RW_1_1 = %d\n", CDD_ChB_RW_1_1);
    DDR_DBG("CDD_ChB_RW_1_0 = %d\n", CDD_ChB_RW_1_0);
    DDR_DBG("CDD_ChB_RW_0_1 = %d\n", CDD_ChB_RW_0_1);
    DDR_DBG("CDD_ChB_RW_0_0 = %d\n", CDD_ChB_RW_0_0);
    DDR_DBG("CDD_ChB_WR_1_1 = %d\n", CDD_ChB_WR_1_1);
    DDR_DBG("CDD_ChB_WR_1_0 = %d\n", CDD_ChB_WR_1_0);
    DDR_DBG("CDD_ChB_WR_0_1 = %d\n", CDD_ChB_WR_0_1);
    DDR_DBG("CDD_ChB_WR_0_0 = %d\n", CDD_ChB_WR_0_0);
    DDR_DBG("CDD_ChB_WW_1_0 = %d\n", CDD_ChB_WW_1_0);
    DDR_DBG("CDD_ChB_WW_0_1 = %d\n", CDD_ChB_WW_0_1);
}

static bool is_ddr4 = false;

static void _read_1d_message_block_outputs_ddr4_(void)
{
    int rdata;

    rdata = dwc_ddrphy_apb_rd(0x54012);
    CDD_RR_3_2 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54013);
    CDD_RR_3_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_3_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54014);
    CDD_RR_2_3 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_2_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54015);
    CDD_RR_2_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_1_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54016);
    CDD_RR_1_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54017);
    CDD_RR_0_3 = ddr_get_data_field(rdata, 0, 8);
    CDD_RR_0_2 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54018);
    CDD_RR_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_3_2 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54019);
    CDD_WW_3_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_3_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401a);
    CDD_WW_2_3 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_2_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401b);
    CDD_WW_2_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_1_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401c);
    CDD_WW_1_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_1_0 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401d);
    CDD_WW_0_3 = ddr_get_data_field(rdata, 0, 8);
    CDD_WW_0_2 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401e);
    CDD_WW_0_1 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_3_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5401f);
    CDD_RW_3_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_3_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54020);
    CDD_RW_3_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_2_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54021);
    CDD_RW_2_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_2_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54022);
    CDD_RW_2_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_1_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54023);
    CDD_RW_1_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_1_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54024);
    CDD_RW_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_0_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54025);
    CDD_RW_0_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_RW_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54026);
    CDD_RW_0_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_3_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54027);
    CDD_WR_3_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_3_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54028);
    CDD_WR_3_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_2_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x54029);
    CDD_WR_2_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_2_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402a);
    CDD_WR_2_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_1_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402b);
    CDD_WR_1_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_1_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402c);
    CDD_WR_1_0 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_0_3 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402d);
    CDD_WR_0_2 = ddr_get_data_field(rdata, 0, 8);
    CDD_WR_0_1 = ddr_get_data_field(rdata, 8, 8);

    rdata = dwc_ddrphy_apb_rd(0x5402e);
    CDD_WR_0_0 = ddr_get_data_field(rdata, 0, 8);

    DDR_DBG("CDD_RR_3_2= %d\n", CDD_RR_3_2);
    DDR_DBG("CDD_RR_3_1= %d\n", CDD_RR_3_1);
    DDR_DBG("CDD_RR_3_0= %d\n", CDD_RR_3_0);
    DDR_DBG("CDD_RR_2_3= %d\n", CDD_RR_2_3);
    DDR_DBG("CDD_RR_2_1= %d\n", CDD_RR_2_1);
    DDR_DBG("CDD_RR_2_0= %d\n", CDD_RR_2_0);
    DDR_DBG("CDD_RR_1_3= %d\n", CDD_RR_1_3);
    DDR_DBG("CDD_RR_1_2= %d\n", CDD_RR_1_2);
    DDR_DBG("CDD_RR_1_0= %d\n", CDD_RR_1_0);
    DDR_DBG("CDD_RR_0_3= %d\n", CDD_RR_0_3);
    DDR_DBG("CDD_RR_0_2= %d\n", CDD_RR_0_2);
    DDR_DBG("CDD_RR_0_1= %d\n", CDD_RR_0_1);
    DDR_DBG("CDD_WW_3_2= %d\n", CDD_WW_3_2);
    DDR_DBG("CDD_WW_3_1= %d\n", CDD_WW_3_1);
    DDR_DBG("CDD_WW_3_0= %d\n", CDD_WW_3_0);
    DDR_DBG("CDD_WW_2_3= %d\n", CDD_WW_2_3);
    DDR_DBG("CDD_WW_2_1= %d\n", CDD_WW_2_1);
    DDR_DBG("CDD_WW_2_0= %d\n", CDD_WW_2_0);
    DDR_DBG("CDD_WW_1_3= %d\n", CDD_WW_1_3);
    DDR_DBG("CDD_WW_1_2= %d\n", CDD_WW_1_2);
    DDR_DBG("CDD_WW_1_0= %d\n", CDD_WW_1_0);
    DDR_DBG("CDD_WW_0_3= %d\n", CDD_WW_0_3);
    DDR_DBG("CDD_WW_0_2= %d\n", CDD_WW_0_2);
    DDR_DBG("CDD_WW_0_1= %d\n", CDD_WW_0_1);
    DDR_DBG("CDD_RW_3_3= %d\n", CDD_RW_3_3);
    DDR_DBG("CDD_RW_3_2= %d\n", CDD_RW_3_2);
    DDR_DBG("CDD_RW_3_1= %d\n", CDD_RW_3_1);
    DDR_DBG("CDD_RW_3_0= %d\n", CDD_RW_3_0);
    DDR_DBG("CDD_RW_2_3= %d\n", CDD_RW_2_3);
    DDR_DBG("CDD_RW_2_2= %d\n", CDD_RW_2_2);
    DDR_DBG("CDD_RW_2_1= %d\n", CDD_RW_2_1);
    DDR_DBG("CDD_RW_2_0= %d\n", CDD_RW_2_0);
    DDR_DBG("CDD_RW_1_3= %d\n", CDD_RW_1_3);
    DDR_DBG("CDD_RW_1_2= %d\n", CDD_RW_1_2);
    DDR_DBG("CDD_RW_1_1= %d\n", CDD_RW_1_1);
    DDR_DBG("CDD_RW_1_0= %d\n", CDD_RW_1_0);
    DDR_DBG("CDD_RW_0_3= %d\n", CDD_RW_0_3);
    DDR_DBG("CDD_RW_0_2= %d\n", CDD_RW_0_2);
    DDR_DBG("CDD_RW_0_1= %d\n", CDD_RW_0_1);
    DDR_DBG("CDD_RW_0_0= %d\n", CDD_RW_0_0);
    DDR_DBG("CDD_WR_3_3= %d\n", CDD_WR_3_3);
    DDR_DBG("CDD_WR_3_2= %d\n", CDD_WR_3_2);
    DDR_DBG("CDD_WR_3_1= %d\n", CDD_WR_3_1);
    DDR_DBG("CDD_WR_3_0= %d\n", CDD_WR_3_0);
    DDR_DBG("CDD_WR_2_3= %d\n", CDD_WR_2_3);
    DDR_DBG("CDD_WR_2_2= %d\n", CDD_WR_2_2);
    DDR_DBG("CDD_WR_2_1= %d\n", CDD_WR_2_1);
    DDR_DBG("CDD_WR_2_0= %d\n", CDD_WR_2_0);
    DDR_DBG("CDD_WR_1_3= %d\n", CDD_WR_1_3);
    DDR_DBG("CDD_WR_1_2= %d\n", CDD_WR_1_2);
    DDR_DBG("CDD_WR_1_1= %d\n", CDD_WR_1_1);
    DDR_DBG("CDD_WR_1_0= %d\n", CDD_WR_1_0);
    DDR_DBG("CDD_WR_0_3= %d\n", CDD_WR_0_3);
    DDR_DBG("CDD_WR_0_2= %d\n", CDD_WR_0_2);
    DDR_DBG("CDD_WR_0_1= %d\n", CDD_WR_0_1);
    DDR_DBG("CDD_WR_0_0= %d\n", CDD_WR_0_0);
}

void _dwc_ddrphy_phyinit_userCustom_H_readMsgBlock_ (int Train2D)
{
    if (is_ddr4) {
        _read_1d_message_block_outputs_ddr4_();
    } else {
        _read_1d_message_block_outputs_();
    }
}

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

static void timing_reg_update_after_training_lpddr4(void)
{
    unsigned char CDD_ChA_RW;
    unsigned char CDD_ChB_RW;
    unsigned char CDD_ChA_WW;
    unsigned char CDD_ChB_WW;
    unsigned char CDD_ChA_RR;
    unsigned char CDD_ChB_RR;

    int rd2wr_old               ;
    int wr2rd_old               ;
    int diff_rank_wr_gap_old    ;
    int diff_rank_wr_gap_msb_old;
    int diff_rank_rd_gap_old    ;
    int diff_rank_rd_gap_msb_old;

    int rd2wr_new               ;
    int wr2rd_new               ;
    int diff_rank_wr_gap_new    ;
    int diff_rank_wr_gap_msb_new;
    int diff_rank_rd_gap_new    ;
    int diff_rank_rd_gap_msb_new;

    int TxDqsDlyTg0_lower_nibble;
    int TxDqsDlyTg0_upper_nibble;
    int TxDqsDlyTg1_lower_nibble;
    int TxDqsDlyTg1_upper_nibble;
    int TxDqsDly;
    int corse_delay;
#if defined(DDR_DEBUG)
    int fine_delay;
#endif
    int wrdata_delay_old;
    int wrdata_delay_new;

    CDD_ChA_RW = max_abs_4(CDD_ChA_RW_1_1, CDD_ChA_RW_1_0, CDD_ChA_RW_0_1, CDD_ChA_RW_0_0);
    CDD_ChB_RW = max_abs_4(CDD_ChB_RW_1_1, CDD_ChB_RW_1_0, CDD_ChB_RW_0_1, CDD_ChB_RW_0_0);

    CDD_ChA_WW = max_abs_2(CDD_ChA_WW_1_0, CDD_ChA_WW_0_1);
    CDD_ChB_WW = max_abs_2(CDD_ChB_WW_1_0, CDD_ChB_WW_0_1);

    CDD_ChA_RR = max_abs_2(CDD_ChA_RR_1_0, CDD_ChA_RR_0_1);
    CDD_ChB_RR = max_abs_2(CDD_ChB_RR_1_0, CDD_ChB_RR_0_1);

    DDR_DBG("CDD_ChA_RW = %d\n", CDD_ChA_RW);
    DDR_DBG("CDD_ChB_RW = %d\n", CDD_ChB_RW);
    DDR_DBG("CDD_ChA_WW = %d\n", CDD_ChA_WW);
    DDR_DBG("CDD_ChB_WW = %d\n", CDD_ChB_WW);
    DDR_DBG("CDD_ChA_RR = %d\n", CDD_ChA_RR);
    DDR_DBG("CDD_ChB_RR = %d\n", CDD_ChB_RR);

    rd2wr_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_RD2WR_FIELD_OFFSET,
                             DRAMTMG2_RD2WR_FIELD_SIZE);
    wr2rd_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_WR2RD_FIELD_OFFSET,
                             DRAMTMG2_WR2RD_FIELD_SIZE);
    diff_rank_wr_gap_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL,
                                        RANKCTL_DIFF_RANK_WR_GAP_FIELD_OFFSET, RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE);
    diff_rank_wr_gap_msb_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL,
                                            RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_OFFSET, RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_SIZE);
    diff_rank_rd_gap_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL,
                                        RANKCTL_DIFF_RANK_RD_GAP_FIELD_OFFSET, RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE);
    diff_rank_rd_gap_msb_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL,
                                            RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_OFFSET, RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_SIZE);

    DDR_DBG("rd2wr_old                = %d\n", rd2wr_old               );
    DDR_DBG("wr2rd_old                = %d\n", wr2rd_old               );
    DDR_DBG("diff_rank_wr_gap_old     = %d\n", diff_rank_wr_gap_old    );
    DDR_DBG("diff_rank_wr_gap_msb_old = %d\n", diff_rank_wr_gap_msb_old);
    DDR_DBG("diff_rank_rd_gap_old     = %d\n", diff_rank_rd_gap_old    );
    DDR_DBG("diff_rank_rd_gap_msb_old = %d\n", diff_rank_rd_gap_msb_old);

    rd2wr_new = 4 + rd2wr_old + (max2(CDD_ChA_RW, CDD_ChB_RW) + 1) / 2;
    wr2rd_new = 4 + wr2rd_old + (max2(CDD_ChA_WW, CDD_ChB_WW) + 1) / 2;

    DDR_DBG("11 rd2wr_new                = %d\n", rd2wr_new               );
    DDR_DBG("11 wr2rd_new                = %d\n", wr2rd_new               );

    diff_rank_wr_gap_new = (diff_rank_wr_gap_msb_old << RANKCTL_DIFF_RANK_WR_GAP_FIELD_OFFSET) +
                           diff_rank_wr_gap_old + (max2(CDD_ChA_WW, CDD_ChB_WW) + 1) / 2;
    DDR_DBG("11 diff_rank_wr_gap_new     = %d\n", diff_rank_wr_gap_new    );

    if (diff_rank_wr_gap_new <= 0xF) {
        diff_rank_wr_gap_new = diff_rank_wr_gap_new;
        diff_rank_wr_gap_msb_new = 0;
    } else {
        diff_rank_wr_gap_new     = ddr_get_data_field(diff_rank_wr_gap_new, 0,
                                   RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE);
        diff_rank_wr_gap_msb_new = ddr_get_data_field(diff_rank_wr_gap_new,
                                   RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE, RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_SIZE);
    }

    diff_rank_rd_gap_new = (diff_rank_rd_gap_msb_old << RANKCTL_DIFF_RANK_RD_GAP_FIELD_OFFSET) +
                           diff_rank_rd_gap_old + (max2(CDD_ChA_RR, CDD_ChB_RR) + 1) / 2;
    DDR_DBG("11 diff_rank_rd_gap_new     = %d\n", diff_rank_rd_gap_new);

    if (diff_rank_rd_gap_new <= 0xF) {
        diff_rank_rd_gap_new = diff_rank_rd_gap_new;
        diff_rank_rd_gap_msb_new = 0;
    } else {
        diff_rank_rd_gap_new     = ddr_get_data_field(diff_rank_rd_gap_new, 0,
                                   RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE);
        diff_rank_rd_gap_msb_new = ddr_get_data_field(diff_rank_rd_gap_new,
                                   RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE, RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_SIZE);
    }

    DDR_DBG("rd2wr_new                = %d\n", rd2wr_new               );
    DDR_DBG("wr2rd_new                = %d\n", wr2rd_new               );
    DDR_DBG("diff_rank_wr_gap_new     = %d\n", diff_rank_wr_gap_new    );
    DDR_DBG("diff_rank_wr_gap_msb_new = %d\n", diff_rank_wr_gap_msb_new);
    DDR_DBG("diff_rank_rd_gap_new     = %d\n", diff_rank_rd_gap_new    );
    DDR_DBG("diff_rank_rd_gap_msb_new = %d\n", diff_rank_rd_gap_msb_new);

    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000001);
    //ddrc_rd(REG_AP_APB_DWC_DDR_UMCTL2_MSTR);
    //ddrc_rd(REG_AP_APB_DWC_DDR_UMCTL2_MSTR2);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_RD2WR_FIELD_OFFSET,
                 DRAMTMG2_RD2WR_FIELD_SIZE, rd2wr_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_WR2RD_FIELD_OFFSET,
                 DRAMTMG2_WR2RD_FIELD_SIZE, wr2rd_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_WR_GAP_FIELD_OFFSET,
                 RANKCTL_DIFF_RANK_WR_GAP_FIELD_SIZE, diff_rank_wr_gap_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_OFFSET,
                 RANKCTL_DIFF_RANK_WR_GAP_MSB_FIELD_SIZE, diff_rank_wr_gap_msb_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_RD_GAP_FIELD_OFFSET,
                 RANKCTL_DIFF_RANK_RD_GAP_FIELD_SIZE, diff_rank_rd_gap_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_RANKCTL, RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_OFFSET,
                 RANKCTL_DIFF_RANK_RD_GAP_MSB_FIELD_SIZE, diff_rank_rd_gap_msb_new);

    //TxDqsDlyTg0-1
    TxDqsDlyTg0_lower_nibble = dwc_ddrphy_apb_rd(0x0100D0);
    TxDqsDlyTg0_upper_nibble = dwc_ddrphy_apb_rd(0x0101D0);
    TxDqsDlyTg1_lower_nibble = dwc_ddrphy_apb_rd(0x0100D1);
    TxDqsDlyTg1_upper_nibble = dwc_ddrphy_apb_rd(0x0101D1);

    TxDqsDly = max_abs_4int(TxDqsDlyTg0_lower_nibble, TxDqsDlyTg0_upper_nibble,
                            TxDqsDlyTg1_lower_nibble, TxDqsDlyTg1_upper_nibble);

    corse_delay = ddr_get_data_field(TxDqsDly, 0, 5);
#if defined(DDR_DEBUG)
    fine_delay  = ddr_get_data_field(TxDqsDly, 6, 4);
#endif
    DDR_DBG("TxDqsDlyTg0_lower_nibble = %d\n", TxDqsDlyTg0_lower_nibble);
    DDR_DBG("TxDqsDlyTg0_upper_nibble = %d\n", TxDqsDlyTg0_upper_nibble);
    DDR_DBG("TxDqsDlyTg1_lower_nibble = %d\n", TxDqsDlyTg1_lower_nibble);
    DDR_DBG("TxDqsDlyTg1_upper_nibble = %d\n", TxDqsDlyTg1_upper_nibble);
    DDR_DBG("corse_delay = %d\n", corse_delay);
    DDR_DBG("fine_delay  = %d\n", fine_delay);

    wrdata_delay_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_DFITMG1,
                                    DFITMG1_DFI_T_WRDATA_DELAY_FIELD_OFFSET, DFITMG1_DFI_T_WRDATA_DELAY_FIELD_SIZE);
    wrdata_delay_new = wrdata_delay_old + ((corse_delay + 1) / 4); //1UI=1/2TCK
    DDR_DBG("wrdata_delay_old = %d\n", wrdata_delay_old);
    DDR_DBG("wrdata_delay_new = %d\n", wrdata_delay_new);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_DFITMG1, DFITMG1_DFI_T_WRDATA_DELAY_FIELD_OFFSET,
                 DFITMG1_DFI_T_WRDATA_DELAY_FIELD_SIZE, wrdata_delay_new);
    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000000);
}

static void timing_reg_update_after_training_ddr4(void)
{
    unsigned char CDD_RW;
    int rd2wr_old ;
    int rd2wr_new ;

    CDD_RW = (CDD_RW_0_0 > 0) ? CDD_RW_0_0 : (-1 * CDD_RW_0_0);

    DDR_DBG("CDD_RW = 0x%x\n", CDD_RW);

    rd2wr_old = ddrc_rd_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_RD2WR_FIELD_OFFSET,
                             DRAMTMG2_RD2WR_FIELD_SIZE);

    DDR_DBG("rd2wr_old = %d\n", rd2wr_old);

    rd2wr_new = rd2wr_old + (CDD_RW + 1) / 2;

    DDR_DBG("rd2wr_new  = %d\n", rd2wr_new);

    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000001);
    ddrc_wr_bits(REG_AP_APB_DWC_DDR_UMCTL2_DRAMTMG2, DRAMTMG2_RD2WR_FIELD_OFFSET,
                 DRAMTMG2_RD2WR_FIELD_SIZE, rd2wr_new);
    ddrc_wr(REG_AP_APB_DWC_DDR_UMCTL2_DBG1, 0x00000000);
}

void _timing_reg_update_after_training_(void)
{
    if (is_ddr4) {
        timing_reg_update_after_training_ddr4();
    } else {
        timing_reg_update_after_training_lpddr4();
    }
}

int load_ddr_training_fw(uint32_t mem, ddrphy_fw_e fw)
{
#ifdef DDR_DEBUG
    static char *DBGstrings[MAXCCM] = {
        "ICCM",
        "DCCM",
        "DIAGI",
        "DIAGD"
    };
#endif
    uint16_t *p_fw = NULL;
    uint32_t sz = 0;
    uint32_t *to = NULL;


    DDR_DBG("going to load train FW(%s/%dd)...\n",
            DBGstrings[mem], (fw % 2) + 1);

    if ((fw == DDRPHY_FW_DDR4_1D) || (fw == DDRPHY_FW_DDR4_2D)) {
        is_ddr4 = true;
    }

#if defined(CFG_DDR_FW_IN_FLASH)
    const char *name = NULL;

    if (mem == DCCM) {
        name = DDRPHY_FW_LPDDR4X_1D == fw ? "lpddr4x_pmu_train_dmem.bin" :
               DDRPHY_FW_LPDDR4_1D == fw ? "lpddr4_pmu_train_dmem.bin" :
               DDRPHY_FW_DDR4_1D == fw ?  "ddr4_pmu_train_dmem.bin" :
               DDRPHY_FW_DDR3_1D == fw ?  "ddr3_pmu_train_dmem.bin" :
               DDRPHY_FW_LPDDR4X_2D == fw ? "lpddr4x_2d_pmu_train_dmem.bin" :
               DDRPHY_FW_LPDDR4_2D == fw ? "lpddr4_2d_pmu_train_dmem.bin" :
               DDRPHY_FW_DDR4_2D == fw ? "ddr4_2d_pmu_train_dmem.bin" : NULL;
        to = (uint32_t *)(APB_DDRPHY_BASE + 0x54000 * 4);
    } else if (mem == ICCM) {
        name = DDRPHY_FW_LPDDR4X_1D == fw ? "lpddr4x_pmu_train_imem.bin" :
               DDRPHY_FW_LPDDR4_1D == fw ? "lpddr4_pmu_train_imem.bin" :
               DDRPHY_FW_DDR4_1D == fw ?  "ddr4_pmu_train_imem.bin" :
               DDRPHY_FW_DDR3_1D == fw ?  "ddr3_pmu_train_imem.bin" :
               DDRPHY_FW_LPDDR4X_2D == fw ? "lpddr4x_2d_pmu_train_imem.bin" :
               DDRPHY_FW_LPDDR4_2D == fw ? "lpddr4_2d_pmu_train_imem.bin" :
               DDRPHY_FW_DDR4_2D == fw ? "ddr4_2d_pmu_train_imem.bin" : NULL;
        to = (uint32_t *)(APB_DDRPHY_BASE + 0x50000 * 4);
    }

    const uint8_t * base = NULL;

    if (NULL != name){
        base = get_partition_addr_by_name("ddr_fw");
        if(!base){
            /* Here, it means that the boot device is eMMC.
            * It cann't get data by accessing address directly.
            * */
            return load_ddr_training_fw_piecewise(name, "ddr_fw", to, NULL, true);
        }

        if (NULL != base ) {
            const hdr_entry_t *hdr = (hdr_entry_t *)base;

            for (int i = 0; i < DDR_FW_ENTRIES_CNT_MAX; i++, hdr++) {
                if (0 == mini_strncmp_s(name, hdr->name, sizeof(hdr->name))) {
                    p_fw = (uint16_t *) (base + hdr->offset);
                    sz = hdr->size;
                    DBG("fw: %s found at 0x%p (%d bytes)\n", name, p_fw, sz);
                    break;
                }
            }
        }
    }

#else

    if (mem == DCCM) {
        p_fw =  DDRPHY_FW_LPDDR4X_1D == fw ? (uint16_t *)ddrphy_fw_lpddr4x_dmem :
                DDRPHY_FW_LPDDR4_1D == fw ? (uint16_t *)ddrphy_fw_lpddr4_dmem :
                DDRPHY_FW_DDR4_1D == fw ? (uint16_t *)ddrphy_fw_ddr4_dmem :
                DDRPHY_FW_DDR3_1D == fw ? (uint16_t *)ddrphy_fw_ddr3_dmem :
                DDRPHY_FW_LPDDR4X_2D == fw ? (uint16_t *)ddrphy_fw_lpddr4x_2d_dmem :
                DDRPHY_FW_LPDDR4_2D == fw ? (uint16_t *)ddrphy_fw_lpddr4_2d_dmem :
                DDRPHY_FW_DDR4_2D == fw ? (uint16_t *)ddrphy_fw_ddr4_2d_dmem : NULL;
        sz =
            DDRPHY_FW_LPDDR4X_1D == fw ? (ddrphy_fw_lpddr4x_dmem_end - ddrphy_fw_lpddr4x_dmem) :
            DDRPHY_FW_LPDDR4_1D == fw ? (ddrphy_fw_lpddr4_dmem_end - ddrphy_fw_lpddr4_dmem) :
            DDRPHY_FW_DDR4_1D == fw ? (ddrphy_fw_ddr4_dmem_end - ddrphy_fw_ddr4_dmem) :
            DDRPHY_FW_DDR3_1D == fw ? (ddrphy_fw_ddr3_dmem_end - ddrphy_fw_ddr3_dmem) :
            DDRPHY_FW_LPDDR4X_2D == fw ? (ddrphy_fw_lpddr4x_2d_dmem_end - ddrphy_fw_lpddr4x_2d_dmem) :
            DDRPHY_FW_LPDDR4_2D == fw ? (ddrphy_fw_lpddr4_2d_dmem_end - ddrphy_fw_lpddr4_2d_dmem) :
            DDRPHY_FW_DDR4_2D == fw ? (ddrphy_fw_ddr4_2d_dmem_end - ddrphy_fw_ddr4_2d_dmem) : 0;
        to = (uint32_t *)(APB_DDRPHY_BASE + 0x54000 * 4);
    } else if (mem == ICCM) {
        p_fw =  DDRPHY_FW_LPDDR4X_1D == fw ? (uint16_t *)ddrphy_fw_lpddr4x_imem :
                DDRPHY_FW_LPDDR4_1D == fw ? (uint16_t *)ddrphy_fw_lpddr4_imem :
                DDRPHY_FW_DDR4_1D == fw ? (uint16_t *)ddrphy_fw_ddr4_imem :
                DDRPHY_FW_DDR3_1D == fw ? (uint16_t *)ddrphy_fw_ddr3_imem :
                DDRPHY_FW_LPDDR4X_2D == fw ? (uint16_t *)ddrphy_fw_lpddr4x_2d_imem :
                DDRPHY_FW_LPDDR4_2D == fw ? (uint16_t *)ddrphy_fw_lpddr4_2d_imem :
                DDRPHY_FW_DDR4_2D == fw ? (uint16_t *)ddrphy_fw_ddr4_2d_imem : NULL;
        sz =
            DDRPHY_FW_LPDDR4X_1D == fw ? (ddrphy_fw_lpddr4x_imem_end - ddrphy_fw_lpddr4x_imem) :
            DDRPHY_FW_LPDDR4_1D == fw ? (ddrphy_fw_lpddr4_imem_end - ddrphy_fw_lpddr4_imem) :
            DDRPHY_FW_DDR4_1D == fw ? (ddrphy_fw_ddr4_imem_end - ddrphy_fw_ddr4_imem) :
            DDRPHY_FW_DDR3_1D == fw ? (ddrphy_fw_ddr3_imem_end - ddrphy_fw_ddr3_imem) :
            DDRPHY_FW_LPDDR4X_2D == fw ? (ddrphy_fw_lpddr4x_2d_imem_end - ddrphy_fw_lpddr4x_2d_imem) :
            DDRPHY_FW_LPDDR4_2D == fw ? (ddrphy_fw_lpddr4_2d_imem_end - ddrphy_fw_lpddr4_2d_imem) :
            DDRPHY_FW_DDR4_2D == fw ? (ddrphy_fw_ddr4_2d_imem_end - ddrphy_fw_ddr4_2d_imem) : 0;
        to = (uint32_t *)(APB_DDRPHY_BASE + 0x50000 * 4);
#ifdef DDR_DIAG
    } else if(mem == DIAGI) {
        p_fw = DDRPHY_FW_LPDDR4X_DIAG == fw? (uint16_t *)ddrphy_fw_lpddr4x_diag_imem :
               DDRPHY_FW_LPDDR4_DIAG == fw? (uint16_t *)ddrphy_fw_lpddr4_diag_imem :
               DDRPHY_FW_DDR4_DIAG == fw? (uint16_t *)ddrphy_fw_ddr4_diag_imem : NULL;
        sz = 
            DDRPHY_FW_LPDDR4X_DIAG == fw? (ddrphy_fw_lpddr4x_diag_imem_end - ddrphy_fw_lpddr4x_diag_imem) :
            DDRPHY_FW_LPDDR4_DIAG == fw? (ddrphy_fw_lpddr4_diag_imem_end - ddrphy_fw_lpddr4_diag_imem) :
            DDRPHY_FW_DDR4_DIAG == fw? (ddrphy_fw_ddr4_diag_imem_end - ddrphy_fw_ddr4_diag_imem) : 0;
        to = (uint32_t *)(APB_DDRPHY_BASE + 0x50000 * 4);
    } else if(DIAGD) {
        p_fw = DDRPHY_FW_LPDDR4X_DIAG == fw? (uint16_t *)ddrphy_fw_lpddr4x_diag_dmem :
               DDRPHY_FW_LPDDR4_DIAG == fw? (uint16_t *)ddrphy_fw_lpddr4_diag_dmem :
               DDRPHY_FW_DDR4_DIAG == fw? (uint16_t *)ddrphy_fw_ddr4_diag_dmem : NULL;
        sz = 
            DDRPHY_FW_LPDDR4X_DIAG == fw? (ddrphy_fw_lpddr4x_diag_dmem_end - ddrphy_fw_lpddr4x_diag_dmem) :
            DDRPHY_FW_LPDDR4_DIAG == fw? (ddrphy_fw_lpddr4_diag_dmem_end - ddrphy_fw_lpddr4_diag_dmem) :
            DDRPHY_FW_DDR4_DIAG == fw? (ddrphy_fw_ddr4_diag_dmem_end - ddrphy_fw_ddr4_diag_dmem) : 0;
        to = (uint32_t *)(APB_DDRPHY_BASE + 0x54200 * 4);
#endif
    }
#endif

    if (sz && (NULL != p_fw)) {
        ddrphy_apb_memcpy(to, p_fw, sz);
    } else {
        return -1;
    }

    return 0;
}

int load_ddr_training_fw_piecewise(const char *name, const char *pt_name,
                                   uint32_t *to, uint64_t * entry_sz,
                                   bool is_phy_apb)
{
    uint8_t *temp_ptr = (uint8_t *)to;
    uint32_t cpy_cnt = 0;
    uint64_t sz = 0, rd_sz = 0, offset = 0, unaligned = 0, remain_sz = 0;
    uint8_t ddr_fw_data[DDR_FW_BUF_SZ] __ALIGNED(DEFAULT_BLK_SZ) = {0};
    const hdr_entry_t *hdr = (hdr_entry_t *)ddr_fw_data;

    rd_sz = load_partition_by_name(pt_name, ddr_fw_data,
                                   ROUNDUP(sizeof(hdr_entry_t) * DDR_FW_ENTRIES_CNT_MAX, DEFAULT_BLK_SZ), 0);
    if (!rd_sz){
        INFO("fail to load ddr fw entry!\n");
        return -1;
    }

    for (int i = 0; i < DDR_FW_ENTRIES_CNT_MAX; i++, hdr++) {
        if (0 == mini_strncmp_s(name, hdr->name, sizeof(hdr->name))) {
            offset = hdr->offset;
            sz = hdr->size;
            DBG("fw: %s found at offset %llu (%d bytes)\n", name, offset, sz);
            break;
        }
    }

    if (!sz){
        INFO("cann't find ddr fw entry!\n");
        return -1;
    }

    if (!is_phy_apb)
        sz = ROUNDUP(sz, 16);

    if (entry_sz)
        *entry_sz = sz;

    unaligned = offset - ROUNDDOWN(offset, DEFAULT_BLK_SZ);
    offset = ROUNDDOWN(offset, DEFAULT_BLK_SZ);

    remain_sz = sz + unaligned;
    do{
        memset(ddr_fw_data, 0x0, sizeof(ddr_fw_data));
        rd_sz = load_partition_by_name(pt_name, ddr_fw_data,
                           DDR_FW_BUF_SZ, offset);
        if(!rd_sz)
        {
            INFO("fail to load ddr fw\n");
            return -1;
        }

        cpy_cnt = MIN(remain_sz, DDR_FW_BUF_SZ) - unaligned;
        if (is_phy_apb)
        {
            ddrphy_apb_memcpy(to, (uint16_t *)(ddr_fw_data + unaligned), cpy_cnt);
            to += cpy_cnt / sizeof(uint16_t);
        }else{
            mini_memcpy_s(temp_ptr, (void *)(ddr_fw_data + unaligned), cpy_cnt);
            temp_ptr += cpy_cnt;
        }

        remain_sz -= (remain_sz > DDR_FW_BUF_SZ) ? DDR_FW_BUF_SZ : remain_sz;
        offset += DDR_FW_BUF_SZ;
        unaligned = 0;

    }while(remain_sz);

    return 0;
}

#ifdef DDR_DIAG
#define TEST_SIMPLE_WRITE_READ (4)
#define TEST_TX_EYE (5)
#define TEST_RX_EYE (6)

static uint8_t diag_flag_type;
static uint8_t diag_flag_rank;
static uint8_t diag_flag_byte;
static uint8_t diag_flag_lane;
static char *type_string;


/*use static variables to avoid too much para transmit*/
static struct diag_msgblk {
    uint8_t	DiagTestNum;
    uint8_t	DiagSubTest;
    uint8_t	DiagPrbs;
    uint8_t	DiagRank;
    uint8_t	DiagChannel;
    uint8_t	DiagRepeatCount;
    uint8_t	DiagLoopCount;
    uint8_t	DiagByte;
    uint8_t	DiagLane;
    uint8_t	DiagVrefInc;
    uint8_t	DiagReserved0A;
    uint8_t	DiagXCount;
    uint16_t	DiagAddrLow;
    uint16_t	DiagAddrHigh;
    uint16_t	DiagPatternLow;
    uint16_t	DiagPatternHigh;
    uint8_t	DiagMisc0;
    uint8_t	DiagReserved15;
    uint16_t	DiagReturnData;
} __PACKED DIAG_MSGBLK;

static void parse_diag_flag(int diag_flag)
{
    diag_flag_type = (diag_flag>>0) & 0xff;
    diag_flag_rank = (diag_flag>>8) & 0xff;
    diag_flag_byte = (diag_flag>>16) & 0xff;
    diag_flag_lane = (diag_flag>>24) & 0xff;
}

static void cfg_diag_msgblk()
{
    //can we use memcpy as we now big/little end
    dwc_ddrphy_apb_wr(0x54200, DIAG_MSGBLK.DiagSubTest << 8 | DIAG_MSGBLK.DiagTestNum);
    dwc_ddrphy_apb_wr(0x54201, DIAG_MSGBLK.DiagRank << 8 | DIAG_MSGBLK.DiagPrbs);
    dwc_ddrphy_apb_wr(0x54202, DIAG_MSGBLK.DiagRepeatCount << 8 | DIAG_MSGBLK.DiagChannel);
    dwc_ddrphy_apb_wr(0x54203, DIAG_MSGBLK.DiagByte << 8 | DIAG_MSGBLK.DiagLoopCount);
    dwc_ddrphy_apb_wr(0x54204, DIAG_MSGBLK.DiagVrefInc << 8 | DIAG_MSGBLK.DiagLane);
    dwc_ddrphy_apb_wr(0x54205, DIAG_MSGBLK.DiagXCount << 8);
    dwc_ddrphy_apb_wr(0x54206, DIAG_MSGBLK.DiagAddrLow);
    dwc_ddrphy_apb_wr(0x54207, DIAG_MSGBLK.DiagAddrHigh);
    dwc_ddrphy_apb_wr(0x54208, DIAG_MSGBLK.DiagPatternLow);
    dwc_ddrphy_apb_wr(0x54209, DIAG_MSGBLK.DiagPatternHigh);
    dwc_ddrphy_apb_wr(0x5420a, DIAG_MSGBLK.DiagMisc0);

    //print all para
    INFO("%s DiagTestNum: 0x%x\n", type_string, DIAG_MSGBLK.DiagTestNum);
    INFO("%s DiagSubTest: 0x%x\n", type_string, DIAG_MSGBLK.DiagSubTest);
    INFO("%s DiagPrbs: 0x%x\n", type_string, DIAG_MSGBLK.DiagPrbs);
    INFO("%s DiagRank: 0x%x\n", type_string, DIAG_MSGBLK.DiagRank);
    INFO("%s DiagChannel: 0x%x\n", type_string, DIAG_MSGBLK.DiagChannel);
    INFO("%s DiagRepeatCount: 0x%x\n", type_string, DIAG_MSGBLK.DiagRepeatCount);
    INFO("%s DiagLoopCount: 0x%x\n", type_string, DIAG_MSGBLK.DiagLoopCount);
    INFO("%s DiagByte: 0x%x\n", type_string, DIAG_MSGBLK.DiagByte);
    INFO("%s DiagLane: 0x%x\n", type_string, DIAG_MSGBLK.DiagLane);
    INFO("%s DiagVrefInc: 0x%x\n", type_string, DIAG_MSGBLK.DiagVrefInc);
    INFO("%s DiagXCount: 0x%x\n", type_string, DIAG_MSGBLK.DiagXCount);
    INFO("%s DiagAddrLow: 0x%x\n", type_string, DIAG_MSGBLK.DiagAddrLow);
    INFO("%s DiagAddrHigh: 0x%x\n", type_string, DIAG_MSGBLK.DiagAddrHigh);
    INFO("%s DiagPatternLow: 0x%x\n", type_string, DIAG_MSGBLK.DiagPatternLow);
    INFO("%s DiagPatternHigh: 0x%x\n", type_string, DIAG_MSGBLK.DiagPatternHigh);
    INFO("%s DiagMisc0: 0x%x\n", type_string, DIAG_MSGBLK.DiagMisc0);
}

static void __run_diag()
{
    int result;

    dwc_ddrphy_apb_wr(0x0C0033, 0x1); //UctWriteProt=0x1
    dwc_ddrphy_apb_wr(0x0D0031, 0x1); //DctWriteProt
    dwc_ddrphy_apb_wr(0x0C0032, 0x0); //UctWriteOnly
    dwc_ddrphy_apb_wr(0xd0000, 0x1); //MicroContMuxSel
    dwc_ddrphy_apb_wr(0xd0099, 0x9); //MicroReset
    dwc_ddrphy_apb_wr(0xd0099, 0x1); //MicroReset
    dwc_ddrphy_apb_wr(0xd0099, 0x0); //MicroReset
    do {
        result = dwc_ddrphy_apb_rd(0xd0032);
    }while (result == 0);
    if (result != 0x07)
       FATAL("Diag test abnormal exit.\n"); 
    dwc_ddrphy_apb_wr(0xd0099, 0x1); //MicroReset
    dwc_ddrphy_apb_wr(0xd0000, 0x0); //MicroContMuxSel
}

static void diag_preset_wr()
{
    DIAG_MSGBLK.DiagTestNum = 4;
    DIAG_MSGBLK.DiagPrbs = 1;
    DIAG_MSGBLK.DiagRepeatCount = 0;
    DIAG_MSGBLK.DiagLoopCount = 10;
    DIAG_MSGBLK.DiagXCount = 0;
    DIAG_MSGBLK.DiagByte = 0;
    DIAG_MSGBLK.DiagLane = 0;
    DIAG_MSGBLK.DiagAddrLow = 0;
    DIAG_MSGBLK.DiagAddrHigh = 0;
    DIAG_MSGBLK.DiagPatternLow = 0;
    DIAG_MSGBLK.DiagPatternHigh = 0;
    DIAG_MSGBLK.DiagVrefInc = 0;
}
static void diag_preset_tx()
{
    DIAG_MSGBLK.DiagTestNum = 5; 
    DIAG_MSGBLK.DiagPrbs = 1;
    DIAG_MSGBLK.DiagRepeatCount = 30;
    DIAG_MSGBLK.DiagLoopCount = 255;
    DIAG_MSGBLK.DiagXCount = 20;
    DIAG_MSGBLK.DiagAddrLow = 0;
    DIAG_MSGBLK.DiagAddrHigh = 0;
    DIAG_MSGBLK.DiagPatternLow = 0;
    DIAG_MSGBLK.DiagPatternHigh = 0;
    DIAG_MSGBLK.DiagVrefInc =1;
}
static void diag_preset_rx()
{
    DIAG_MSGBLK.DiagTestNum = 6; 
    DIAG_MSGBLK.DiagPrbs = 1;
    DIAG_MSGBLK.DiagRepeatCount = 30;
    DIAG_MSGBLK.DiagLoopCount = 255;
    DIAG_MSGBLK.DiagXCount = 20;
    DIAG_MSGBLK.DiagAddrLow = 0;
    DIAG_MSGBLK.DiagAddrHigh = 0;
    DIAG_MSGBLK.DiagPatternLow = 0;
    DIAG_MSGBLK.DiagPatternHigh = 0;
    DIAG_MSGBLK.DiagVrefInc =1;
}
static void get_wr_result()
{
    int i = 0;
    int addr = 0x5420c;
    int byte, data;
    while(i < 36) {
        data = dwc_ddrphy_apb_rd(addr);
        byte = ddr_get_data_field(data, 0, 8);
        if ((unlikely(i%9) == 8)) {//do not care dbi
            INFO("\n");
        } else if(byte == 0) {
            INFO("O ");
        } else {
            INFO("X ");
        }
        i++;
        byte = ddr_get_data_field(data, 8, 8);
        if ((unlikely(i%9) == 8)) {//do not care dbi
            INFO("\n");
        } else if(byte == 0) {
            INFO("O ");
        } else {
            INFO("X ");
        }
        i++;
        addr++;
    }
}
static void get_tx_result()
{
    int nvref,ndelay;
    int ivref,idelay;
    int trained_vref,trained_delay;
    int data;
    int byte;
    int addr = 0x5420b;
    bool need_read;

    data = dwc_ddrphy_apb_rd(addr);
    ndelay = ddr_get_data_field(data, 0, 8);
    nvref = ddr_get_data_field(data, 8, 8);
    addr++;
    data = dwc_ddrphy_apb_rd(addr);
    trained_vref = ddr_get_data_field(data, 0, 8);
    addr++;
    trained_delay = dwc_ddrphy_apb_rd(addr);
    addr++;
    need_read = true;

    INFO("\ntrained vref: %d   trained_delay:%d \n", trained_vref, trained_delay);
    INFO("     ");
    for (idelay = 0; idelay < ndelay; idelay++) {
        INFO(" %3d ", trained_delay + idelay - ndelay/2);
    }
    INFO("\n");
    for(ivref = 0; ivref < nvref; ivref++) {
        INFO("%3d: ", ivref);
        for (idelay = 0; idelay < ndelay; idelay++) {
            if (need_read) {
                data = dwc_ddrphy_apb_rd(addr);
                byte =  ddr_get_data_field(data, 0, 8); 
            } else {
                byte =  ddr_get_data_field(data, 8, 8);
                addr++;
            }
            need_read = !need_read;
            if (unlikely((ivref == trained_vref) && (idelay == ndelay/2))) {
                INFO("   + ");
            } else {
                INFO(" %3d ", byte);
            }
        }
        INFO("\n");
    } 
}
static void get_rx_result()
{
    int nvref,ndelay;
    int ivref,idelay;
    int trained_vref,trained_vref1,trained_delay;
    int data;
    int byte;
    int addr = 0x5420b;
    bool need_read;

    data = dwc_ddrphy_apb_rd(addr);
    ndelay = ddr_get_data_field(data, 0, 8);
    nvref = ddr_get_data_field(data, 8, 8);
    addr++;
    data = dwc_ddrphy_apb_rd(addr);
    trained_vref = ddr_get_data_field(data, 0, 8);
    trained_vref1 = ddr_get_data_field(data, 8, 8);
    addr++;
    trained_delay = dwc_ddrphy_apb_rd(addr);
    addr++;
    need_read = true;

    INFO("\ntrained vref: %d %d   trained_delay:%d \n", trained_vref, trained_vref1, trained_delay);
    INFO("     ");
    for (idelay = 0; idelay < ndelay; idelay++) {
        INFO(" %3d ", idelay - 6);
    }
    INFO("\n");
    for(ivref = 0; ivref < nvref; ivref++) {
        INFO("%3d: ", ivref);
        for (idelay = 0; idelay < ndelay; idelay++) {
            if (need_read) {
                data = dwc_ddrphy_apb_rd(addr);
                byte =  ddr_get_data_field(data, 0, 8); 
            } else {
                byte =  ddr_get_data_field(data, 8, 8);
                addr++;
            }
            need_read = !need_read;
            if (unlikely((ivref == trained_vref) && (idelay == trained_delay+6))) {
                INFO("   + ");
            } else {
                INFO(" %3d ", byte);
            }
        }
        INFO("\n");
    } 

}
static void run_wr_test()
{
    diag_preset_wr();
    type_string = "RW TEST:";
    INFO("\n\ndiag wr test begin\n");
    for(DIAG_MSGBLK.DiagRank=0; DIAG_MSGBLK.DiagRank<2; DIAG_MSGBLK.DiagRank++) {
        if (!(diag_flag_rank & (1 << DIAG_MSGBLK.DiagRank)))
            continue;
        INFO("\n\n%s RK_NUM=%0d\n", type_string, DIAG_MSGBLK.DiagRank);
        cfg_diag_msgblk();
         __run_diag();
        get_wr_result();
    }
    INFO("\n\ndiag wr test end\n");
}
static void run_tx_eye()
{
    diag_preset_tx();
    type_string = "TX EYE:";
    INFO("\n\ndiag tx test begin\n");
    for(DIAG_MSGBLK.DiagRank=0; DIAG_MSGBLK.DiagRank<2; DIAG_MSGBLK.DiagRank++) {
        if (!(diag_flag_rank & (1 << DIAG_MSGBLK.DiagRank)))
            continue;
        for(DIAG_MSGBLK.DiagByte=0; DIAG_MSGBLK.DiagByte<4; DIAG_MSGBLK.DiagByte++) {
            if (!(diag_flag_byte & (1 << DIAG_MSGBLK.DiagByte)))
                continue;
            for(DIAG_MSGBLK.DiagLane=0; DIAG_MSGBLK.DiagLane<8; DIAG_MSGBLK.DiagLane++) {
                if (!(diag_flag_lane & (1 << DIAG_MSGBLK.DiagLane)))
                    continue;
                INFO("\n\n%s RK_NUM=%0d, DB_NUM=%0d, DQ_NUM=%0d\n",
                    type_string, DIAG_MSGBLK.DiagRank, DIAG_MSGBLK.DiagByte, DIAG_MSGBLK.DiagLane);
                cfg_diag_msgblk();
                __run_diag();
                get_tx_result();
            }
        }
    }
    INFO("\n\ndiag tx test end\n");
}
static void run_rx_eye()
{
    diag_preset_rx();
    type_string = "RX EYE:";
    INFO("\n\ndiag rx test begin\n");
    for(DIAG_MSGBLK.DiagRank=0; DIAG_MSGBLK.DiagRank<2; DIAG_MSGBLK.DiagRank++) {
        if (!(diag_flag_rank & (1 << DIAG_MSGBLK.DiagRank)))
            continue;
        for(DIAG_MSGBLK.DiagByte=0; DIAG_MSGBLK.DiagByte<4; DIAG_MSGBLK.DiagByte++) {
            if (!(diag_flag_byte & (1 << DIAG_MSGBLK.DiagByte)))
                continue;
            for(DIAG_MSGBLK.DiagLane=0; DIAG_MSGBLK.DiagLane<8; DIAG_MSGBLK.DiagLane++) {
                if (!(diag_flag_lane & (1 << DIAG_MSGBLK.DiagLane)))
                    continue;
                INFO("\n\n%s RK_NUM=%0d, DB_NUM=%0d, DQ_NUM=%0d\n",
                    type_string, DIAG_MSGBLK.DiagRank, DIAG_MSGBLK.DiagByte, DIAG_MSGBLK.DiagLane);
                cfg_diag_msgblk();
                __run_diag();
                get_rx_result();
            }
        }
    }
    INFO("\n\ndiag rx test end\n");
}
void run_ddr_diag(int diag_flag)
{
    parse_diag_flag(diag_flag);
    if (diag_flag & 1<<TEST_SIMPLE_WRITE_READ)
        run_wr_test();
    if (diag_flag & 1<<TEST_TX_EYE)
        run_tx_eye();
    if (diag_flag & 1<<TEST_RX_EYE)
        run_rx_eye();
}
#else
void run_ddr_diag(int diag_flag) {}
#endif

#ifdef __cplusplus
}
#endif
