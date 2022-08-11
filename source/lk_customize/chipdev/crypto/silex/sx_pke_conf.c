/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <string.h>
#include <platform.h>

#include <sx_errors.h>
#include <ce_reg.h>
#include <sx_dma.h>
#include <sx_pke_conf.h>

#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#if RSA_PERFORMANCE_TEST
static uint64_t time_slice = 0;
#endif

#if PK_CM_ENABLED
struct sx_rng pk_rng = {
    .param = NULL,
    .get_rand_blk = NULL
};

void dma_set_rng(struct sx_rng rng)
{
    pk_rng = rng;
}
#endif

/** @brief This function starts the given PK
 *  @param vce_id      vce index
 *  @return The start bit of the CommandReg of the given BA414EP struct has been set
 *  to appropriate value.
 */
static void pke_start(uint32_t vce_id)
{
    uint32_t value = reg_value(0x1, 0, CE_PKE_GO_SHIFT, CE_PKE_GO_MASK);
#if AUTO_OUTPUT_BY_CE
    value = reg_value(0x1, value, CE_PKE_CLRMEMAFTEROP_SHIFT, CE_PKE_CLRMEMAFTEROP_MASK);
#else
    value = reg_value(0x0, value, CE_PKE_CLRMEMAFTEROP_SHIFT, CE_PKE_CLRMEMAFTEROP_MASK);
#endif
    value = reg_value(0x0, value, CE_PKE_CLRMEMAFTEROP_SHIFT, CE_PKE_CLRMEMAFTEROP_MASK);
    writel(value, _ioaddr(REG_PKE_CTRL_CE_(vce_id)));
}

/**
 * @brief Function is used to get the full contents of the DMA status register
 * @param vce_id      vce index
 * @return the contents oft he status register as uint32_t.
 */
static uint32_t pke_get_status(uint32_t vce_id)
{
    uint32_t err = readl(_ioaddr(REG_PK_STATUSREG_CE_(vce_id))) & 0x3FF0; //get low 14 bits

    switch (err) {
        case 0:
            return CRYPTOLIB_SUCCESS;

        case CE_PK_NOTQUADRATICRESIDUE_MASK:
            return CRYPTOLIB_PK_NOTQUADRATICRESIDUE;

        case CE_PK_COMPOSITE_MASK:
            return CRYPTOLIB_PK_COMPOSITE;

        case CE_PK_NOTINVERTIBLE_MASK:
            return CRYPTOLIB_PK_NOTINVERTIBLE;

        case CE_PK_PARAM_AB_NOTVALID_MASK:
            return CRYPTOLIB_PK_PARAM_AB_NOTVALID;

        case CE_PK_SIGNATURE_NOTVALID_MASK:
            return CRYPTOLIB_PK_SIGNATURE_NOTVALID;

        case CE_PK_NOTIMPLEMENTED_MASK:
            return CRYPTOLIB_PK_NOTIMPLEMENTED;

        case CE_PK_PARAM_N_NOTVALID_MASK:
            return CRYPTOLIB_PK_N_NOTVALID;

        case CE_PK_COUPLE_NOTVALID_MASK:
            return CRYPTOLIB_PK_COUPLE_NOTVALID;

        case CE_PK_POINT_PX_ATINFINITY_MASK:
            return CRYPTOLIB_PK_POINT_PX_ATINFINITY;

        case CE_PK_POINT_PX_NOTONCURVE_MASK:
            return CRYPTOLIB_PK_POINT_PX_NOTONCURVE;

        case CE_PK_FAIL_ADDRESS_MASK:
            return CRYPTOLIB_PK_FAIL_ADDRESS;

        default:
            return err;
    }

    return err;
}

void wait_irq_fct(uint32_t vce_id)
{
    event_wait(&g_ce_signal[vce_id]);
    LTRACEF("wait irq end in pke\n");
}

/** @brief: Function tells if a given Public Key is Busy or not (checking its status register)
 *  @param vce_id      vce index
 *  @return 1 when given pub key is busy, 0 otherwise
 */
#if WAIT_PK_WITH_REGISTER_POLLING
static int pke_is_busy(uint32_t vce_id)
{
    return readl(_ioaddr(REG_PK_STATUSREG_CE_(vce_id))) & 0x10000;
}
#endif

/**
 * @brief Function is used to wait for an interrupt, and read & return the status register
 * @param vce_id      vce index
 * @return the contents of the status register as uint32_t.
 */
static uint32_t pke_wait_status(uint32_t vce_id)
{
#if WAIT_PK_WITH_REGISTER_POLLING
    int i = 0;

    while (pke_is_busy(vce_id)) {
        if (5 == i % 1000) {
            LTRACEF("times: %d, PKE is busy.\n", i);
        }

        i++;
    }

#else
    LTRACEF("wait irq in pke\n");
    wait_irq_fct(vce_id);
#endif

#if AUTO_OUTPUT_BY_CE
    //clear pk result ctrl, must use this if auto output
    writel(0x0, _ioaddr(REG_PK_RESULTS_CTRL_CE_(vce_id)));
#endif
    return pke_get_status(vce_id);
}


void pke_set_config(uint32_t vce_id,
                    uint32_t PtrA,
                    uint32_t PtrB,
                    uint32_t PtrC,
                    uint32_t PtrN)
{
    uint32_t value = reg_value(PtrA, 0, CE_PK_OPPTRA_SHIFT, CE_PK_OPPTRA_MASK);
    value = reg_value(PtrB, value, CE_PK_OPPTRB_SHIFT, CE_PK_OPPTRB_MASK);
    value = reg_value(PtrC, value, CE_PK_OPPTRC_SHIFT, CE_PK_OPPTRC_MASK);
    value = reg_value(PtrN, value, CE_PK_OPPTRN_SHIFT, CE_PK_OPPTRN_MASK);

    writel(value, _ioaddr(REG_PK_POINTERREG_CE_(vce_id)));
}

void pke_set_command(uint32_t vce_id,
                     uint32_t op,
                     uint32_t operandsize,
                     uint32_t swap,
                     uint32_t curve_flags)
{
    uint32_t value = 0x80000000;  //PK_CALCR2
    uint32_t NumberOfBytes;

    if (operandsize > 0) {
        NumberOfBytes = operandsize - 1;
    }
    else {
        NumberOfBytes = 0;
    }

    // Data ram is erased automatically after reset in PK engine.
    // Wait until erasing is finished before writing any data
    // (this routine is called before any data transfer)
#if WAIT_PK_WITH_REGISTER_POLLING

    while (pke_is_busy(vce_id));

#else
    //PK_WAITIRQ_FCT();
#endif

    value = reg_value(op, value, CE_PK_TYPE_SHIFT, CE_PK_TYPE_MASK);
    value = reg_value(NumberOfBytes, value, CE_PK_SIZE_SHIFT, CE_PK_SIZE_MASK);

#if PK_CM_ENABLED

    // Counter-Measures for the Public Key
    if (BA414EP_IS_OP_WITH_SECRET_ECC(op)) {
        // ECC operation
        value = reg_value(BA414EP_CMD_RANDPR(PK_CM_RANDPROJ_ECC), value, CE_PK_RANDPROJ_SHIFT, CE_PK_RANDPROJ_MASK);
        value = reg_value(BA414EP_CMD_RANDKE(PK_CM_RANDKE_ECC), value, CE_PK_RANDKE_SHIFT, CE_PK_RANDKE_MASK);
    }
    else if (BA414EP_IS_OP_WITH_SECRET_MOD(op)) {
        // Modular operations
        value = reg_value(BA414EP_CMD_RANDPR(PK_CM_RANDPROJ_MOD), value, CE_PK_RANDPROJ_SHIFT, CE_PK_RANDPROJ_MASK);
        value = reg_value(BA414EP_CMD_RANDKE(PK_CM_RANDKE_MOD), value, CE_PK_RANDKE_SHIFT, CE_PK_RANDKE_MASK);
    }

#endif
    value = reg_value(swap, value, CE_PK_SWAP_SHIFT, CE_PK_SWAP_MASK);
    //value = reg_value(curve_flags, value, CE_PK_SELCURVE_SHIFT, CE_PK_SELCURVE_MASK);
    value |= curve_flags;

    writel(value, _ioaddr(REG_PK_COMMANDREG_CE_(vce_id)));
}

void pke_set_dst_param(uint32_t vce_id,
                       uint32_t operandsize,
                       uint32_t operandsel,
                       addr_t dst_addr,
                       ce_addr_type_t dst_type)
{
    uint32_t value;

    value = reg_value(operandsize, 0, CE_PKE_RESULTS_OP_LEN_SHIFT, CE_PKE_RESULTS_OP_LEN_MASK);
    value = reg_value(operandsel, value, CE_PKE_RESULTS_OP_SELECT_SHIFT, CE_PKE_RESULTS_OP_SELECT_MASK);

    writel(value, _ioaddr(REG_PK_RESULTS_CTRL_CE_(vce_id)));

    writel(addr_switch_to_ce(vce_id, dst_type, (_paddr((void*)dst_addr))) & 0xffffffff, _ioaddr(REG_PK_RESULTS_DST_ADDR_CE_(vce_id)));

    //TODO: should be compatible with 64 bits
    value = reg_value(0/*(uint32_t)(dst_addr >> 32)*/, 0, CE_PKE_RESULTS_DST_ADDR_H_SHIFT, CE_PKE_RESULTS_DST_ADDR_H_MASK);
    value = reg_value(switch_addr_type(dst_type), value, CE_PKE_RESULTS_DST_TYPE_SHIFT, CE_PKE_RESULTS_DST_TYPE_MASK);

    writel(value, _ioaddr(REG_PK_RESULTS_DST_ADDR_H_CE_(vce_id)));
}

uint32_t pke_start_wait_status(uint32_t vce_id)
{
    uint32_t res;

#if RSA_PERFORMANCE_TEST
    uint64_t cur_time = current_time_hires();
#endif

    pke_start(vce_id);
    res = pke_wait_status(vce_id);

#if RSA_PERFORMANCE_TEST
    time_slice += current_time_hires() - cur_time;
#endif

    return res;
}

void pke_reset(uint32_t vce_id)
{
    writel(0x1 << CE_PKE_SOFT_RST_SHIFT, _ioaddr(REG_PKE_CTRL_CE_(vce_id)));
}

void pke_load_curve(uint32_t vce_id,
                    block_t curve,
                    uint32_t size,
                    uint32_t byte_swap,
                    uint32_t gen)
{
    uint32_t i;

    block_t param;
    param.addr  = curve.addr;
    param.len   = size;
    param.addr_type = curve.addr_type;

    /* Load ECC parameters */
    for (i = 0; i * size < curve.len; i++) {
        if (gen || (i != 2 && i != 3)) {
            if (!byte_swap) {
                mem2CryptoRAM_rev(vce_id, param, size, i, true);
            }
            else {
                mem2CryptoRAM(vce_id, param, size, i, false);
            }
        }

        param.addr += param.len;
    }
}

#if RSA_PERFORMANCE_TEST
uint64_t get_rsa_time_slice(void)
{
    return time_slice;
}
void clear_rsa_time_slice(void)
{
    time_slice = 0;
}
#endif
