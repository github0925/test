/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include "msgsema_unit_reg.h"

#define MB_MSG_REGs_PER_MSG     (3U)
#define MB_MSG_REGs_PER_CPU     (MB_MSG_REGs_PER_MSG * 4)

bool mb_is_lock(module_e m, U8 cpu_id)
{
    msgsema_unit_t *mb = (msgsema_unit_t *)soc_get_module_base(m);
    volatile U32 *cpu_mid_reg = &(mb->cpu0_masterid);

    return ((cpu_mid_reg[cpu_id] & 0x80808080) == 0x80808080);
}

/*
 * map master_ids to cpu_id. Pls note that all the four mid for this 'cpu' as
 * same.
 *
 *  cpu_id      the id of 'cpu'
 *  mid         master id value
 *  lock        lock mid if set
 */
U32 mb_cfg_mid(module_e m, U8 cpu_id, U8 mid, bool lock)
{
    msgsema_unit_t *mb = (msgsema_unit_t *)soc_get_module_base(m);
    volatile U32 *cpu_mid_reg = &(mb->cpu0_masterid);

    U32 val = cpu_mid_reg[cpu_id];

    if (val & 0x80808080) {
        DBG("%s: Opps, cpu_mid_reg[%d] been locked.\n", __FUNCTION__, cpu_id);
        return -1;
    }

    val &= ~(0x7FU | (0x7FU << 8) | (0x7FU << 16) | (0x7FU << 24));
    U32 v = mid & 0x7FU;
    val |= (v | (v << 8) | (v << 16) | (v << 24));

    if (lock) {
        val |= 0x80808080;
    }

    cpu_mid_reg[cpu_id] = val;

    return 0;
}

/* @brief   send a short message (message buffer not used).
 *
 * @para
 *      cpu_to_msk  The cpus the message will be sent to. CPU x if bitx set.
 *      msg_id      the message id will be used. Valid range is 0 ~ 3.
 *      msg         message buffer.
 *      len         in bytes. If not even, one zero will be padded.
 */
U32 mb_tx_smsg(module_e m, U32 cpu_to_msk, U8 msg_id, U8 *msg, U32 len)
{
    msgsema_unit_t *mb = (msgsema_unit_t *)soc_get_module_base(m);
    volatile U32 *tmc_reg = &(mb->tmc0);

    if (len > 8) {
        DBG("Opps, mb_tx_smsg support 8 bytes message at most.\n");
        return -1;
    }

    /* Note: only the receiver can read tms, others only read as 0.
     */

    /* message slot not available yet */
    if (0 != (tmc_reg[msg_id] & BM_TMC0_TMC0_MSG_SEND)) {
        DBG("%s: Opps, can not send by msg ID%d, since previous msg not been fully received yet\n",
            __FUNCTION__, msg_id);
        return -2;
    }

    U64 val = 0ULL;

    for (int i = 0; i < len; i++) {
        val |= ((U64)msg[i] << (i * 8));
    }

    mb->tmh0 = FV_TMH0_MDP(cpu_to_msk) | FV_TMH0_TXMES_LEN((len + 1) / 2)
               | FV_TMH0_MID(msg_id);

    mb->tmh1 = (U32)val;

    if (len > 4) {
        mb->tmh2 = (U32)(val >> 32);
    }

    tmc_reg[msg_id] |= BM_TMC0_TMC0_MSG_SEND;

    return 0;
}

bool mb_is_rx_msg_in(module_e m, U8 cpu_from, U8 msg_id)
{
    msgsema_unit_t *mb = (msgsema_unit_t *)soc_get_module_base(m);

    U32 shift = cpu_from * 4 + msg_id;

    return ((mb->tms & (0x01UL << shift)) != 0);
}

/* @brief   receive short message then ack sender (message buffer not used).
 *
 * @para
 *      cpu_from    the sender.
 *      msg_id      the message id will be used. Valid range is 0 ~ 3.
 *      msg         message buffer.
 *      len         in bytes. inout. len of msg buffer as input, and the bytes
 *                  received as output.
 */
U32 mb_rx_smsg(module_e m, U8 cpu_from, U8 msg_id, U8 *msg, U32 *len)
{
    msgsema_unit_t *mb = (msgsema_unit_t *)soc_get_module_base(m);
    volatile U32 *rx_msg_regs = &(mb->cpu0_msg0_rmh0);
    volatile U32 *rmh = &rx_msg_regs[cpu_from * MB_MSG_REGs_PER_CPU
                                              + msg_id * MB_MSG_REGs_PER_MSG];

    U32 shift = cpu_from * 4 + msg_id;

    if (NULL == msg) {
        return -1;
    }

    while (!(mb->tms & (0x01UL << shift)));

    U32 l = GFV_CPU0_MSG0_RMH0_CPU0_MSG0_LEN(*rmh) * 2;

    if ((l > 8) || (l > *len)) {
        DBG("Opps, too many bytes to receive\n");
        return -2;
    }

    *len = 0;

    rmh++;      /* rmh1 */
    U64 val = *rmh;

    if (l > 4) {
        rmh++;  /* rmh2 */
        val |= (U64)(*rmh) << 32;
    }

    for (int i = 0; i < l; i++) {
        msg[i] = val >> (i * 8);
    }

    /* message acknowledgement */
    mb->rmc |= (0x01UL << shift);

    *len = l;

    return 0;
}

typedef struct {
    volatile U32 sg;
    volatile U32 sgpc;
} sg_reg_t;

uint32_t semphore_get_owner(module_e m, uint32_t id)
{
    msgsema_unit_t *mb = (msgsema_unit_t *)soc_get_module_base(m);
    sg_reg_t *sg_reg = (sg_reg_t *) & (mb->semag0);

    uint32_t v = sg_reg[id].sg;

    return GFV_SEMAG0_SG0LS(v);
}

bool semphore_try_lock(module_e m, uint32_t id)
{
    msgsema_unit_t *mb = (msgsema_unit_t *)soc_get_module_base(m);
    sg_reg_t *sg_reg = (sg_reg_t *) & (mb->semag0);

    sg_reg[id].sg |= BM_SEMAG0_SG0C;

    uint32_t v = sg_reg[id].sg;

    if ((v & BM_SEMAG0_SG0C) && (GFV_SEMAG0_SG0LS(v) == (0x01u << MB_ID_THIS_CPU))) {
        return true;
    } else {
        return false;
    }
}

bool semphore_unlock(module_e m, uint32_t id)
{
    msgsema_unit_t *mb = (msgsema_unit_t *)soc_get_module_base(m);
    sg_reg_t *sg_reg = (sg_reg_t *) & (mb->semag0);

    sg_reg[id].sg &= ~BM_SEMAG0_SG0C;

    return true;
}
