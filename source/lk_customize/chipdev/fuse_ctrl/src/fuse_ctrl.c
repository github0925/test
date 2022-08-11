/********************************************************
 *	        Copyright(c) 2018	Semidrive 		        *
 ********************************************************/

#include <stdio.h>
#include "fuse_ctrl.h"
#include "fuse_ctrl_reg.h"
#include "lib/reg.h"
#include "__regs_base.h"

//#define DEBUG
#ifdef DEBUG
#define DBG(fmt, args...)   printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#if !defined(FUSE0_OFFSET)
#define FUSE0_OFFSET    0x1000
#endif

#if !defined(BM_SEMA_LOCK_STATUS)
#define BM_SEMA_LOCK_STATUS     BM_SEMA_AP_LOCK
#endif

/* fixme: a shortcut for bringup*/
#define ECC_FUSE_START  8
#define ECC_FUSE_END    167
#define ECC_VAL_START   196
#define RED_VAL_START1   188
#define RED_VAL_START2   236

#define FUSE_CTRL1  1
static addr_t soc_get_module_base(uint32_t module)
{
    addr_t base = 0ul;
    switch (module) {
    case FUSE_CTRL1:
         base = _ioaddr(APB_EFUSEC_BASE);
         break;
    }

    return base;
}

static uint32_t fuse_get_parity_type(uint32_t id)
{
    fuse_pari_type_e t = PARITY_NONE;

    if ((id >= ECC_FUSE_START) && (id <= ECC_FUSE_END)) {
        t = PARITY_ECC;
    } else if ((id < ECC_FUSE_START) ||
               ((id > ECC_FUSE_END) && (id < RED_VAL_START1))) {
        t = PARITY_RED;
    }

    return t;
}

static uint32_t fuse_get_red_pos(uint32_t id)
{
    uint32_t pos = 0;

    if (id < ECC_FUSE_START) {
        pos = RED_VAL_START1 + id;
    } else if ((id > ECC_FUSE_END) && (id < RED_VAL_START1)) {
        pos = RED_VAL_START2 + id - (ECC_FUSE_END + 1);
    }

    return pos;
}

static uint32_t fuse_get_ecc_pos(uint32_t id, uint32_t *wd, uint32_t *shift)
{
    *wd = ECC_VAL_START + (id - ECC_FUSE_START) / 4;
    *shift = ((id - ECC_FUSE_START) % 4) * 8;

    return 0;
}

const static uint8_t ecc_v[32] = {
    0x07, 0x0b, 0x0d, 0x0e, 0x15, 0x16, 0x19, 0x1a,
    0x23, 0x25, 0x2a, 0x2c, 0x31, 0x32, 0x34, 0x38,
    0x13, 0x1c, 0x29, 0x45, 0x46, 0x49, 0x4a, 0x4c,
    0x51, 0x52, 0x54, 0x61, 0x62, 0x64, 0x68, 0x70
};

static uint8_t fuse_gen_ecc(uint32_t v)
{
    uint8_t ecc = 0;
    for (int i =0; i < 32; i++) {
        ecc ^= (v & (0x01u << i)) ? ecc_v[i] : 0;
    }

    return ecc;
}

uint32_t fuse_read(uint32_t index)
{
    addr_t base = soc_get_module_base(FUSE_CTRL1);

    return readl(base + FUSE0_OFFSET + index * 4);
}

uint32_t fuse_sense(uint32_t index, uint32_t *data)
{
    addr_t b = soc_get_module_base(FUSE_CTRL1);

    while(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_LOCK);
    writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    do {
        writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    } while (!(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_STATUS));

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    uint32_t ctrl = readl(b + FUSE_CTRL_OFF);
    uint32_t trig = readl(b + FUSE_TRIG_OFF);

    ctrl &= ~FM_FUSE_CTRL_ADDR;
    ctrl |= FV_FUSE_CTRL_ADDR(index);
    writel(ctrl, b + FUSE_CTRL_OFF);

    trig |= BM_FUSE_TRIG_READ;
    writel(trig, b + FUSE_TRIG_OFF);

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    *data = readl(b + READ_FUSE_DATA_OFF);

    uint32_t res = (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_ERROR) ? -1 : 0;

    writel(0, b + SEMA_LOCK_OFF);

    return res;
}

static uint32_t fuse_program_raw(uint32_t index, uint32_t v)
{
    addr_t b = soc_get_module_base(FUSE_CTRL1);

    DBG("%s: index=%d, v=0x%x\n", __FUNCTION__, index, v);

    while(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_LOCK);
    writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    do {
        writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    } while (!(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_STATUS));

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    uint32_t ctrl = readl(b + FUSE_CTRL_OFF);
    uint32_t trig = readl(b + FUSE_TRIG_OFF);

    ctrl &= ~FM_FUSE_CTRL_ADDR;
    ctrl |= FV_FUSE_CTRL_ADDR(index) | FV_FUSE_CTRL_PROG_KEY(FUSE_PROG_KEY);
    writel(ctrl, b + FUSE_CTRL_OFF);

    writel(v, b + PROG_DATA_OFF);

    trig |= BM_FUSE_TRIG_PROG;
    writel(trig, b + FUSE_TRIG_OFF);

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    uint32_t res = (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_ERROR) ? -1 : 0;

    writel(0, b + SEMA_LOCK_OFF);

    return res;
}

uint32_t fuse_program(uint32_t index, uint32_t v)
{
    uint32_t res = -1;

    do {
        res = fuse_program_raw(index, v);
        if (0 != res) break;

        fuse_pari_type_e type = fuse_get_parity_type(index);
        DBG("fuse_%d is %s\n", index,
            type == PARITY_ECC ? "ECC-ed" :
            type == PARITY_RED ? "Redundant" :
            type == PARITY_NONE ? "no parity" : "invalid parity");
        if (type == PARITY_INVALID) break;

        if (type == PARITY_ECC) {
            uint32_t ecc_wd = 0, ecc_shft = 0;
            fuse_get_ecc_pos(index, &ecc_wd, &ecc_shft);

            uint32_t ecc = fuse_gen_ecc(v);
            uint32_t val = fuse_read(ecc_wd);
            val &= ~(0xffu << ecc_shft);
            val |= ecc << ecc_shft;
            res = fuse_program_raw(ecc_wd, val);
        } else if (type == PARITY_RED) {
            uint32_t red_wd = fuse_get_red_pos(index);
            if (0 != red_wd) {
                res = fuse_program_raw(red_wd, v);
            }
        }
    } while(0);

    return res;
}

uint32_t fuse_reload(void)
{
    addr_t b = soc_get_module_base(FUSE_CTRL1);

    while(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_LOCK);
    writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    do {
        writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    } while (!(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_STATUS));

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    uint32_t v = readl(b + FUSE_TRIG_OFF);
    v |= BM_FUSE_TRIG_LOAD;
    writel(v, b + FUSE_TRIG_OFF);

    while ((readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY)
           || (readl(b + FUSE_TRIG_OFF) & BM_FUSE_TRIG_LOAD));

    uint32_t res = (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_ERROR) ? -1 : 0;

    if (0 != res) {
        DBG("Opps, error found. int_stat = 0x%x\n", readl(b + INT_STA_OFF));
    }

    writel(0, b + SEMA_LOCK_OFF);

    return res;
}

void fuse_set_sticky_bit(fuse_acc_domain_e dom, uint32_t id)
{
    addr_t b = soc_get_module_base(FUSE_CTRL1);

    uint32_t sticky_off = (dom == FUSE_ACC_SAFE ?
                      SW_STICKY_SE0_OFF : SW_STICKY_AP0_OFF) + (id / 32) * 4;

    uint32_t v = readl(b + sticky_off);
    v |= (0x01u << (id % 32));
    writel(v, b + sticky_off);
}

uint32_t fuse_get_bank_lock(fuse_acc_domain_e dom,uint32_t bank)
{
    addr_t b = soc_get_module_base(FUSE_CTRL1);

    uint32_t lock_off = (dom == FUSE_ACC_SAFE ?
                    SE_BANK_LOCK0_OFF: AP_BANK_LOCK0_OFF) + (bank / 8) * 4;

    uint32_t v = readl(b + lock_off);
    v &= (0xFu << ((bank % 8) * 4));
    v = v >> ((bank % 8) * 4);

    uint32_t lock_off_fuse = (dom == FUSE_ACC_SAFE ? FUSE0_OFFSET :
                         (FUSE0_OFFSET + 0x10) ) + (bank / 8) * 4;

    uint32_t vfuse = readl(b + lock_off_fuse);
    vfuse &= (0xFu << ((bank % 8) * 4));
    vfuse = vfuse >> ((bank % 8) * 4);

    return (v | vfuse);
}

void fuse_lock_bank(fuse_acc_domain_e dom, uint32_t bank, uint32_t lock_bits)
{
    addr_t b = soc_get_module_base(FUSE_CTRL1);

    uint32_t lock_off = (dom == FUSE_ACC_SAFE ?
                    SE_BANK_LOCK0_OFF: AP_BANK_LOCK0_OFF) + (bank / 8) * 4;
    uint32_t v = readl(b + lock_off);
    v &= ~(0xFu << ((bank % 8) * 4));
    v |= (lock_bits << ((bank % 8) * 4));
    writel(v, b + lock_off);
}

void fuse_ctl_cfg_timing(uint32_t freq_mhz)
{
    addr_t b = soc_get_module_base(FUSE_CTRL1);
    while(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_LOCK);
    writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    do {
        writel(BM_SEMA_LOCK_LOCK, b + SEMA_LOCK_OFF);
    } while (!(readl(b + SEMA_LOCK_OFF) & BM_SEMA_LOCK_STATUS));

    while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY);

    uint32_t v = readl(b + FUSE_TIMING_1_OFF);
    v &= ~FM_FUSE_TIMING_1_PROG_STRB_CNT;
    v |= FV_FUSE_TIMING_1_PROG_STRB_CNT(freq_mhz * 5 - 1);  /* 5us */
    writel(v, b + FUSE_TIMING_1_OFF);

    writel(0, b + SEMA_LOCK_OFF);
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

int cmd_fuse_program(int argc, const cmd_args *argv)
{
    if (argc != 3) {
        DBG("%s: Opps, invalid paras.\n", __func__);
        return -1;
    }
    uint32_t fuse = argv[1].u;
    uint32_t v = argv[2].u;

    return fuse_program(fuse, v);
}

int cmd_fuse_sense(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        DBG("%s: Opps, invalid paras.\n", __func__);
        return -1;
    }
    uint32_t fuse = argv[1].u;
    uint32_t v = 0;
    fuse_sense(fuse, &v);
    printf("%s: fuse_%d = 0x%x\n", __func__, fuse, v);

    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("fuse_program", "fuse blow", (console_cmd)&cmd_fuse_program)
STATIC_COMMAND("fuse_sense", "fuse blow", (console_cmd)&cmd_fuse_sense)
STATIC_COMMAND_END(fuse);
#endif
