/*
 *  * Copyright (c)  Semidrive
 *   * Author : huhenglei
 *    *
 *     */

#include <platform/interrupts.h>
#include <platform/debug.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <err.h>
#include <reg.h>

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include "sd_mipi_csi2.h"


#define LOG_LEVEL 4

static const struct phy_freqrange g_phy_freqs[63] = {
    { 0x00, 80, 88},    //80
    { 0x10, 88, 95},    //90
    { 0x20, 95, 105},   //100
    { 0x30, 105, 115},  //110
    { 0x01, 115, 125},  //120
    { 0x11, 125, 135},  //130
    { 0x21, 135, 145},  //140
    { 0x31, 145, 155},  //150
    { 0x02, 155, 165},  //160
    { 0x12, 165, 175},  //170
    { 0x22, 175, 185},  //180
    { 0x32, 185, 198},  //190
    { 0x03, 198, 212},  //205
    { 0x13, 212, 230},  //220
    { 0x23, 230, 240},  //235
    { 0x33, 240, 260},  //250
    { 0x04, 260, 290},  //275
    { 0x14, 290, 310},  //300
    { 0x25, 310, 340},  //325
    { 0x35, 340, 375},  //350
    { 0x05, 375, 425},  //400
    { 0x16, 425, 275},  //450
    { 0x26, 475, 525},  //500
    { 0x37, 525, 575},  //550
    { 0x07, 575, 625},  //600
    { 0x18, 625, 675},  //650
    { 0x28, 675, 725},  //700
    { 0x39, 725, 775},  //750
    { 0x09, 775, 825},  //800
    { 0x19, 825, 875},  //850
    { 0x29, 875, 925},  //900
    { 0x3a, 925, 975},  //950
    { 0x0a, 975, 1025}, //1000
    { 0x1a, 1025, 1075},    //1050
    { 0x2a, 1075, 1125},    //1100
    { 0x3b, 1125, 1175},    //1150
    { 0x0b, 1175, 1225},    //1200
    { 0x1b, 1225, 1275},    //1250
    { 0x2b, 1275, 1325},    //1300
    { 0x3c, 1325, 1375},    //1350
    { 0x0c, 1375, 1425},    //1400
    { 0x1c, 1425, 1475},    //1450
    { 0x2c, 1475, 1525},    //1500
    { 0x3d, 1525, 1575},    //1550
    { 0x0d, 1575, 1625},    //1600
    { 0x1d, 1625, 1675},    //1650
    { 0x2d, 1675, 1725},    //1700
    { 0x3e, 1725, 1775},    //1750
    { 0x0e, 1775, 1825},    //1800
    { 0x1e, 1825, 1875},    //1850
    { 0x2f, 1875, 1925},    //1900
    { 0x3f, 1925, 1975},    //1950
    { 0x0f, 1975, 2025},    //2000
    { 0x40, 2025, 2075},    //2050
    { 0x41, 2075, 2125},    //2100
    { 0x42, 2125, 2175},    //2150
    { 0x43, 2175, 2225},    //2200
    { 0x44, 2225, 2275},    //2250
    { 0x45, 2275, 2325},    //2300
    { 0x46, 2325, 2375},    //2350
    { 0x47, 2375, 2425},    //2400
    { 0x48, 2425, 2475},    //2450
    { 0x49, 2475, 2500},    //2500
};


bool check_version(mipi_csi_device *dev)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    dprintf(LOG_LEVEL, "%s(): 0x%x.\n", __func__, reg->version);

    if (reg->version != 0x3133302A) {
        printf("mipi_csi2 version error.\n");
        return false;
    }
    else
        return true;
}

void phy_enable(mipi_csi_device *dev, bool enable)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;

    if (enable) {
        reg->phy_shutdownz = 0x1;
        reg->dphy_rstz = 0x1;
    }
    else {
        reg->phy_shutdownz = 0x0;
        reg->dphy_rstz = 0x0;
    }
}

void set_lane_number(mipi_csi_device *dev, int cnt)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;

    switch (cnt) {
        case 1:
            reg->n_lanes = 0;
            break;

        case 2:
            reg->n_lanes = 1;
            break;

        case 3:
            reg->n_lanes = 2;
            break;

        case 4:
        default:
            reg->n_lanes = 3;
            break;
    }
}

void disable_test_mode(mipi_csi_device *dev)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    reg->phy_test_ctrl0 = 0;
}


void test_mode_enable(mipi_csi_device *dev, bool enable)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;

    if (enable == true) {
        reg->phy_test_ctrl0 = 1; //0-1-0, test reset, h reset active
        reg->phy_test_ctrl0 = 0;
    }
}

void test_mode_config_phy(mipi_csi_device *dev)
{
    //mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)io_base;
    //
}

void test_mode_write(mipi_csi_device *dev, int phy_index)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;

    if (phy_index == 0) {
        reg->phy_test_ctrl1 = 1 << 16 | 0x33 << 0; //enable, in addr

        reg->phy_test_ctrl0 |= 1 << 1; //en=h, write addr on clk falling
        reg->phy_test_ctrl0 &= ~(1 << 1);

        reg->phy_test_ctrl1 = 0 << 16 | 0x66 << 0; //enable, in data

        reg->phy_test_ctrl0 |= 1 << 1;
        reg->phy_test_ctrl0 &= ~(1 << 1); //en=l, write data on clk rising
    }
}

void test_mode_read(mipi_csi_device *dev, int phy_index)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;

    if (phy_index == 0) {
        reg->phy_test_ctrl1 = 1 << 16 | 0x33; //enable, in addr

        reg->phy_test_ctrl0 |= 1 << 1; //clk
        reg->phy_test_ctrl0 &= ~(1 << 1);

        uint32_t val = reg->phy_test_ctrl1;

        reg->phy_test_ctrl1 &= ~(1 << 16);
        dprintf(LOG_LEVEL, "%s(): val =0x%x.\n", __func__, val);
    }
}

void inner_resetn(mipi_csi_device *dev, bool reset)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;

    if (reset)
        reg->csi2_resetn = 0;
    else
        reg->csi2_resetn = 1;
}

void get_phy_status(mipi_csi_device *dev, u32 *power_state,
                    u32 *stop_state)
{

    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    dprintf(LOG_LEVEL, "reg->phy_rx=0x%x, reg->phy_stopstate=0x%x.\n",
            reg->phy_rx, reg->phy_stopstate);
    *power_state = reg->phy_rx;
    *stop_state = reg->phy_stopstate;
}

void set_ipi_mode(mipi_csi_device *dev)
{
    uint32_t val;
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    val = 0x1 << 16;
    val |= 0x1 << 24;
    reg->ipi_mode = val;
    reg->ipi2_info.ipi_mode = val;
    reg->ipi3_info.ipi_mode = val;
    reg->ipi4_info.ipi_mode = val;
}

void set_ipi_vcid(mipi_csi_device *dev)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    reg->ipi_vcid = 0;
    reg->ipi2_info.ipi_vcid = 1;
    reg->ipi3_info.ipi_vcid = 2;
    reg->ipi4_info.ipi_vcid = 3;
}

void set_ipi_data_type(mipi_csi_device *dev, uint8_t type)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    reg->ipi_data_type = type;
    reg->ipi2_info.ipi_data_type = type;
    reg->ipi3_info.ipi_data_type = type;
    reg->ipi4_info.ipi_data_type = type;
}


void set_ipi_hsa(mipi_csi_device *dev, uint32_t val)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    reg->ipi_hsa_time = val;
    reg->ipi2_info.ipi_hsa_time = val;
    reg->ipi3_info.ipi_hsa_time = val;
    reg->ipi4_info.ipi_hsa_time = val;
}

void set_ipi_hbp(mipi_csi_device *dev, uint32_t val)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    reg->ipi_hbp_time = val;
    reg->ipi2_info.ipi_hbp_time = val;
    reg->ipi3_info.ipi_hbp_time = val;
    reg->ipi4_info.ipi_hbp_time = val;
}

void set_ipi_hsd(mipi_csi_device *dev, uint32_t val)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    reg->ipi_hsd_time = val;
    reg->ipi2_info.ipi_hsd_time = val;
    reg->ipi3_info.ipi_hsd_time = val;
    reg->ipi4_info.ipi_hsd_time = val;
}

void set_ipi_adv_features(mipi_csi_device *dev, uint32_t val)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    reg->ipi_adv_features = val;
    reg->ipi2_info.ipi_adv_features = val;
    reg->ipi3_info.ipi_adv_features = val;
    reg->ipi4_info.ipi_adv_features = val;
}

static void set_mipi_csi_freqrange(u8 id, u8 val)
{
    addr_t paddr, vaddr;

#if ARCH_ARM64

    if (id == 0) {
        paddr = 0x30d50000 + 0x60;
    }
    else if (id == 1) {
        paddr = 0x30d50000 + 0x68;
    }
    else {
        printf("wrong mipi csi %d\n", id);
        return ;
    }

#else

    if (id == 0) {
        paddr = 0xf0d50000 + 0x60;
    }
    else if (id == 1) {
        paddr = 0xf0d50000 + 0x68;
    }
    else {
        printf("wrong mipi csi %d\n", id);
        return ;
    }

#endif
#if WITH_KERNEL_VM
    vaddr = (vaddr_t)paddr_to_kvaddr(paddr);
#else
    vaddr = paddr;
#endif
    *((u32 *)vaddr) = val;
    printf("%s: 0x%lx=0x%x\n", __func__, vaddr, val);
}


void dump_register(mipi_csi_device *dev)
{
    mipi_csi2_reg_type *reg = (mipi_csi2_reg_type *)dev->reg_base;
    dprintf(LOG_LEVEL, "version=0x%x\n", reg->version);
    dprintf(LOG_LEVEL, "n_lanes=0x%x\n", reg->n_lanes);

    dprintf(LOG_LEVEL, "phy_rx=0x%x\n", reg->phy_rx);
    dprintf(LOG_LEVEL, "phy_stopstate=0x%x\n", reg->phy_stopstate);

    dprintf(LOG_LEVEL, "ipi_mode=0x%x\n", reg->ipi_mode);
    dprintf(LOG_LEVEL, "ipi_vcid=0x%x\n", reg->ipi_vcid);
    dprintf(LOG_LEVEL, "ipi_data_type=0x%x\n", reg->ipi_data_type);
    dprintf(LOG_LEVEL, "ipi_hsa_time=0x%x\n", reg->ipi_hsa_time);
    dprintf(LOG_LEVEL, "ipi_hbp_time=0x%x\n", reg->ipi_hbp_time);
    dprintf(LOG_LEVEL, "ipi_hsd_time=0x%x\n", reg->ipi_hsd_time);
    dprintf(LOG_LEVEL, "ipi_hline_time=0x%x\n", reg->ipi_hline_time);

    dprintf(LOG_LEVEL, "ipi2_mode=0x%x\n", reg->ipi2_info.ipi_mode);
    dprintf(LOG_LEVEL, "ipi2_vcid=0x%x\n", reg->ipi2_info.ipi_vcid);
    dprintf(LOG_LEVEL, "ipi2_data_type=0x%x\n", reg->ipi2_info.ipi_data_type);
    dprintf(LOG_LEVEL, "ipi2_hsa_time=0x%x\n", reg->ipi2_info.ipi_hsa_time);
    dprintf(LOG_LEVEL, "ipi2_hbp_time=0x%x\n", reg->ipi2_info.ipi_hbp_time);
    dprintf(LOG_LEVEL, "ipi2_hsd_time=0x%x\n", reg->ipi2_info.ipi_hsd_time);

}

void sd_mipi_csi2_init_cfg (mipi_csi_device *dev)
{

    dprintf(LOG_LEVEL, "%s() io_base=0x%lx.\n", __func__, dev->reg_base);

    check_version(dev);
    set_lane_number(dev, 4);
    disable_test_mode(dev);

    set_ipi_mode(dev);
    set_ipi_vcid(dev);
    set_ipi_data_type(dev, DT_YUV422_8);
    //set_ipi_hsa(dev, 10);
    //set_ipi_hbp(dev, 20);
    //set_ipi_hsd(dev, 0x60);

    set_ipi_adv_features(dev, 0x1030000);

    //dump_register(dev);
    dprintf(LOG_LEVEL, "%s() end.\n", __func__);
}


status_t sd_mipi_csi2_set_hline_time(mipi_csi_device *dev, uint32_t hsa,
                                     uint32_t hbp, uint32_t hsd)
{
    dprintf(LOG_LEVEL, "%s(): 0x%x, 0x%x, 0x%x.\n", __func__, hsa, hbp, hsd);
    set_ipi_hsa(dev, hsa);
    set_ipi_hbp(dev, hbp);
    set_ipi_hsd(dev, hsd);
    return NO_ERROR;
}


status_t sd_mipi_csi2_set_phy_freq(mipi_csi_device *dev, uint32_t freq,
                                   uint32_t lanes)
{
    int phyrate;
    int i;
    phyrate = freq * lanes * 2;

    if ((phyrate < g_phy_freqs[0].range_l)
            || (phyrate > g_phy_freqs[62].range_h)) {
        printf("err phy freq\n");
        return -1;
    }

    for (i = 0; i < 63; i++) {
        if ((phyrate >= g_phy_freqs[i].range_l)
                && (phyrate < g_phy_freqs[i].range_h))
            break;
    }

    dprintf(LOG_LEVEL, "%s(): phyrate=%d, index=0x%x, dev->freq_index=0x%x\n",
            __func__, phyrate, g_phy_freqs[i].index, dev->freq_index);

    if (dev->freq_index != g_phy_freqs[i].index) {
        dev->freq_index = g_phy_freqs[i].index;
        set_mipi_csi_freqrange(dev->host_id, dev->freq_index);
        spin(1000);
    }

    return NO_ERROR;
}


void sd_mipi_csi2_start (mipi_csi_device *dev)
{
    u32 val1, val2;
    dprintf(0, "%s(): id=%d:\n", __func__, dev->host_id);

    if (!dev->enabled) {
        phy_enable(dev, true);
        inner_resetn(dev, false);
        spin(1000);
        dev->enabled = true;
    }

    get_phy_status(dev, &val1, &val2);
}

void sd_mipi_csi2_stop (mipi_csi_device *dev)
{
    dprintf(0, "%s(): id=%d\n", __func__, dev->host_id);

    phy_enable(dev, false);
    inner_resetn(dev, true);
    dev->enabled = false;
}



mipi_csi_device *sd_mipi_csi2_host_init(uint32_t id, addr_t addr)
{
    dprintf(LOG_LEVEL, "%s(): id=%d, addr=0x%lx.\n", __func__, id, addr);

    mipi_csi_device *dev;
    dev = malloc(sizeof(*dev));

    if (!dev)
        return NULL;

    memset(dev, 0, sizeof(*dev));

#if WITH_KERNEL_VM
    dev->reg_base = (vaddr_t)paddr_to_kvaddr(addr);
#else
    dev->reg_base = addr;
#endif
    dev->host_id = id;
    dev->enabled = false;
    return dev;
}

