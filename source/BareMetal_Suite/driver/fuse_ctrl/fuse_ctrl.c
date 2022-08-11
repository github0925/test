/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include "fuse_ctrl.h"
#include "fuse_ctrl_reg.h"

//#define FUSE_DBG(fmt, args...)  DBG(fmt, ##args)
#define FUSE_DBG(fmt, args...)

#if !defined(FUSE0_OFFSET)
#define FUSE0_OFFSET    0x1000
#endif

#if !defined(BM_SEMA_LOCK_STATUS)
#define BM_SEMA_LOCK_STATUS     BM_SEMA_SE_LOCK
#endif

const static U8 ecc_v[32] = {
    0x07, 0x0b, 0x0d, 0x0e, 0x15, 0x16, 0x19, 0x1a,
    0x23, 0x25, 0x2a, 0x2c, 0x31, 0x32, 0x34, 0x38,
    0x13, 0x1c, 0x29, 0x45, 0x46, 0x49, 0x4a, 0x4c,
    0x51, 0x52, 0x54, 0x61, 0x62, 0x64, 0x68, 0x70
};

static U8 fuse_gen_ecc(U32 v)
{
    U8 ecc = 0;

    for (int i = 0; i < 32; i++) {
        ecc ^= (v & (0x01u << i)) ? ecc_v[i] : 0;
    }

    return ecc;
}

U32 fuse_read(U32 index)
{
    U32 base = soc_get_module_base(FUSE_CTRL1);

    return readl(base + FUSE0_OFFSET + index * 4);
}

U32 fuse_sense(U32 index, uint32_t *data)
{
    U32 b = soc_get_module_base(FUSE_CTRL1);

    while (readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_LOCK);

    writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);

    do {
        writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    } while (!(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_STATUS));

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    U32 ctrl = readl(b + FUSE_CTRL_OFF);
    U32 trig = readl(b + FUSE_TRIG_OFF);

    ctrl &= ~FM_FUSE_CTRL_ADDR;
    ctrl |= FV_FUSE_CTRL_ADDR(index);
    writel(ctrl, b + FUSE_CTRL_OFF);

    trig |= BM_FUSE_TRIG_READ;
    writel(trig, b + FUSE_TRIG_OFF);

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    *data = readl(b + READ_FUSE_DATA_OFF);

    U32 res = (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_ERROR) ? -1 : 0;

    writel(0, b + SEMA_LOCK_OFF);

    return res;
}

static U32 fuse_program_raw(U32 index, U32 v)
{
    U32 b = soc_get_module_base(FUSE_CTRL1);

    FUSE_DBG("%s: index=%d, v=0x%x\n", __FUNCTION__, index, v);

    while (readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_LOCK);

    writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);

    do {
        writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    } while (!(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_STATUS));

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    U32 ctrl = readl(b + FUSE_CTRL_OFF);
    U32 trig = readl(b + FUSE_TRIG_OFF);

    ctrl &= ~FM_FUSE_CTRL_ADDR;
    ctrl |= FV_FUSE_CTRL_ADDR(index) | FV_FUSE_CTRL_PROG_KEY(FUSE_PROG_KEY);
    writel(ctrl, b + FUSE_CTRL_OFF);

    writel(v, b + PROG_DATA_OFF);

    trig |= BM_FUSE_TRIG_PROG;
    writel(trig, b + FUSE_TRIG_OFF);

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    U32 res = (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_ERROR) ? -1 : 0;

    writel(0, b + SEMA_LOCK_OFF);

    return res;
}

U32 fuse_program(U32 index, U32 v)
{
    U32 res = -1;

    do {
        res = fuse_program_raw(index, v);

        if (0 != res) break;

        fuse_pari_type_e type = fuse_get_parity_type(index);
        FUSE_DBG("fuse_%d is %s\n", index,
                 type == PARITY_ECC ? "ECC-ed" :
                 type == PARITY_RED ? "Redundant" :
                 type == PARITY_NONE ? "no parity" : "invalid parity");

        if (type == PARITY_INVALID) break;

        if (type == PARITY_ECC) {
            U32 ecc_wd = 0, ecc_shft = 0;
            fuse_get_ecc_pos(index, &ecc_wd, &ecc_shft);

            U32 ecc = fuse_gen_ecc(v);
            U32 val = fuse_read(ecc_wd);
            val &= ~(0xffu << ecc_shft);
            val |= ecc << ecc_shft;
            res = fuse_program_raw(ecc_wd, val);
        } else if (type == PARITY_RED) {
            U32 red_wd = fuse_get_red_pos(index);

            if (0 != red_wd) {
                res = fuse_program_raw(red_wd, v);
            }
        }
    } while (0);

    return res;
}

U32 fuse_reload(void)
{
    U32 b = soc_get_module_base(FUSE_CTRL1);

    while (readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_LOCK);

    writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);

    do {
        writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    } while (!(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_STATUS));

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    U32 v = readl(b + FUSE_TRIG_OFF);
    v |= BM_FUSE_TRIG_LOAD;
    writel(v, b + FUSE_TRIG_OFF);

    while ((readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY)
           || (readl(b + FUSE_TRIG_OFF) & BM_FUSE_TRIG_LOAD));

    U32 res = (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_ERROR) ? -1 : 0;

    if (0 != res) {
        DBG("Opps, error found. int_stat = 0x%x\n", readl(b + INT_STA_OFF));
    }

    writel(0, b + SEMA_LOCK_OFF);

    return res;
}

void fuse_set_sticky_bit(fuse_acc_domain_e dom, U32 id)
{
    U32 b = soc_get_module_base(FUSE_CTRL1);

    U32 sticky_off = (dom == FUSE_ACC_SAFE ?
                      SW_STICKY_SE0_OFF : SW_STICKY_AP0_OFF) + (id / 32) * 4;

    U32 v = readl(b + sticky_off);
    v |= (0x01u << (id % 32));
    writel(v, b + sticky_off);
}

U32 fuse_get_bank_lock(fuse_acc_domain_e dom, U32 bank)
{
    U32 b = soc_get_module_base(FUSE_CTRL1);

    U32 lock_off = (dom == FUSE_ACC_SAFE ?
                    SE_BANK_LOCK0_OFF : AP_BANK_LOCK0_OFF) + (bank / 8) * 4;

    U32 v = readl(b + lock_off);
    v &= (0xFu << ((bank % 8) * 4));
    v = v >> ((bank % 8) * 4);

    U32 lock_off_fuse = (dom == FUSE_ACC_SAFE ? FUSE0_OFFSET :
                         (FUSE0_OFFSET + 0x10) ) + (bank / 8) * 4;

    U32 vfuse = readl(b + lock_off_fuse);
    vfuse &= (0xFu << ((bank % 8) * 4));
    vfuse = vfuse >> ((bank % 8) * 4);

    return (v | vfuse);
}

void fuse_lock_bank(fuse_acc_domain_e dom, U32 bank, U32 lock_bits)
{
    U32 b = soc_get_module_base(FUSE_CTRL1);

    U32 lock_off = (dom == FUSE_ACC_SAFE ?
                    SE_BANK_LOCK0_OFF : AP_BANK_LOCK0_OFF) + (bank / 8) * 4;
    U32 v = readl(b + lock_off);
    v &= ~(0xFu << ((bank % 8) * 4));
    v |= (lock_bits << ((bank % 8) * 4));
    writel(v, b + lock_off);
}

void fuse_ctl_cfg_timing(U32 freq_mhz)
{
    U32 b = soc_get_module_base(FUSE_CTRL1);

    while (readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_LOCK);

    writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);

    do {
        writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    } while (!(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_STATUS));

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    U32 v = readl(b + FUSE_TIMING_1_OFF);
    v &= ~FM_FUSE_TIMING_1_PROG_STRB_CNT;
    v |= FV_FUSE_TIMING_1_PROG_STRB_CNT(freq_mhz * 5 - 1);  /* 5us */
    writel(v, b + FUSE_TIMING_1_OFF);

    writel(0, b + SEMA_LOCK_OFF);
}

#include "shell/shell.h"

uint32_t cmd_fuse_program(uint32_t argc, char *argv[])
{
    if (argc != 3) {
        DBG("%s: Opps, invalid paras.\n", __func__);
        return -1;
    }

    uint32_t fuse = strtoul(argv[1], NULL, 0);
    uint32_t v = strtoul(argv[2], NULL, 0);

    return fuse_program(fuse, v);
}

uint32_t cmd_fuse_sense(uint32_t argc, char *argv[])
{
    if (argc != 2) {
        DBG("%s: Opps, invalid paras.\n", __func__);
        return -1;
    }

    uint32_t fuse = strtoul(argv[1], NULL, 0);;
    uint32_t v = 0;
    fuse_sense(fuse, &v);
    DBG("%s: fuse_%d = 0x%x\n", __func__, fuse, v);

    return 0;
}

SHELL_CMD("fuse_blow", cmd_fuse_program, "Usage: fuse_blow fuse value")
SHELL_CMD("fuse_sense", cmd_fuse_sense, "Usage: fuse_sense fuse")
