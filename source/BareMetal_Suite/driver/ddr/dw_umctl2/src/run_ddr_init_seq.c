/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 *******************************************************/

#include <common_hdr.h>
#include <debug.h>
#include "ddr_init_helper.h"
#include "srv_timer/srv_timer.h"
#include "rstgen/rstgen.h"
#include "scr/scr.h"
#include "dw_umctl2.h"

//#define LocalDBG(fmt, args...)   DBG(fmt, ##args)
#define LocalDBG(fmt, args...)

extern const ddr_act_u ddr_init_seq[];

#include <gpt.h>

static uint32_t gpt_strcmp(const char *s1, const char *s2)
{
    assert((NULL != s1) && (NULL != s2));

    uint32_t res = 0;

    while ((*s1 != '\0') && (*s2 != '\0')) {
        res |= (*s1 != *s2);
        s1 = s1 + 1;
        s2 = s2 + 2;
    }

    return res | (*s1 != '\0') | (*s2 != '\0');
}

static bool is_valid_ddr_reg(uint32_t a)
{
    return ((a >= APB_DDRPHY_BASE) && (a <= (APB_DDRPHY_BASE + 20u * 1024u * 1024u)));
}

int gpt_partion_search(const char *name, uint32_t *base, uint32_t *len)
{
    int ret = -1;

    for (int i = 0; i < GPT_PARTITION_NUM; i++) {
        if (0 == gpt_strcmp(name, (const char *)GPT_ENTRY_NAME_ADDR(i))) {
            *base = GPT_PARTITION_ADDR(i);
            *len = GPT_PARTITION_SIZE(i);
            ret = 0;
            break;
        }
    }

    return ret;
}

int32_t run_ddr_init_seq(const ddr_act_u *act)
{
    soc_dis_isolation(DDR_SS);

    soc_deassert_reset(DDR_SS);

    scr_bit_set(APB_SCR_SEC_BASE, L31, SCR_SEC_NOC2DDR_FIFO_WRAP_BYPASS_L31_START_BIT);
#ifdef TGT_safe
    scr_bit_set(APB_SCR_SAF_BASE, RW, SCR_SAF_DDR_SS_PWROKIN_AON_RW_START_BIT);
#else
    scr_bit_set(APB_SCR_SEC_BASE, RW, SCR_SEC_DDR_SS_PWROKIN_AON_RW_START_BIT);
#endif
    for (; act->call.act != ACT_END; act++) {
        if (ACT_WR == act->call.act) {
            LocalDBG("writel(0x%x, 0x%x)\n", act->wr.val, act->wr.addr);

            if (!is_valid_ddr_reg(act->wr.addr)) {
                return -1;
            }

            writel(act->wr.val, act->wr.addr);
        } else if (ACT_WR_BITS == act->call.act) {
            LocalDBG("RMWREG32(0x%x, 0x%x, 0x%x, 0x%x)\n",
                     act->wrbits.addr, act->wrbits.shift, act->wrbits.width, act->wrbits.val);

            if (!is_valid_ddr_reg(act->wrbits.addr)) {
                return -1;
            }

            RMWREG32(act->wrbits.addr, act->wrbits.shift, act->wrbits.width, act->wrbits.val);
        } else if (ACT_POLL_BITS == act->call.act) {
            uint32_t cnt = 0;
            uint32_t tmt = (((uint32_t)(act->poll.tmt_h)) << 16) | ((uint32_t)(act->poll.tmt));
            uint32_t a = act->poll.addr;
            uint32_t v = act->poll.val;
            uint32_t msk = act->poll.msk;
            LocalDBG("poll(0x%x, 0x%x, 0x%x, %d)\n", a, msk, v, tmt);

            if (!is_valid_ddr_reg(act->poll.addr)) {
                return -1;
            }

            while ((cnt++ < (uint32_t)(tmt)) && ((readl(a) & (msk)) != ((v) & (msk))));
        } else if (ACT_FUNC_CALL == act->call.act) {
            if (act->call.func == FUNC_RESET) {
                LocalDBG("FUNC_RESET(%d)\n", act->call.para[0]);
                rg_module_reset(APB_RSTGEN_SEC_BASE, act->call.para[0], 1);
            } else if (act->call.func == FUNC_LOADFW) {
                LocalDBG("FUNC_LOADFW(%d,%d)\n", act->call.para[0], act->call.para[1]);

                if (0 != load_ddr_training_fw(act->call.para[0], act->call.para[1])) {
                    return -1;
                }
            } else if (act->call.func == FUNC_FW_MON) {
                LocalDBG("FUNC_FW_MON...\n");

                if (0 != dwc_ddrphy_fw_monitor_loop(act->call.para[0])) {
                    return -2;
                }
            } else if (act->call.func == FUNC_GET_TRAIN_PARA) {
                LocalDBG("FUNC_GET_TRAIN_PARA()\n");
                _dwc_ddrphy_phyinit_userCustom_H_readMsgBlock_(0);
            } else if (act->call.func == FUNC_USE_TRAIN_PARA) {
                LocalDBG("FUNC_USE_TRAIN_PARA\n");
                _timing_reg_update_after_training_();
            } else if (act->call.func == FUNC_CFG_CLK) {
                uint32_t v = act->call.para[0];
                uint32_t dr = (v == FREQ_DDR_4266 ? 4266 :
                               v == FREQ_DDR_2133 ? 2133 :
                               v == FREQ_DDR_3200 ? 3200 :
							   v == FREQ_DDR_2400 ? 2400 :
                               v == FREQ_DDR_1600 ? 1600 :
                               v == FREQ_DDR_800 ? 800 :
                               v);
                INFO("DDR Data Rate: %d\n", dr);
                soc_config_clk(DDR_SS, act->call.para[0]);
            } else if (act->call.func == FUNC_PRINT_SEQ_VERSION) {
                DBG("DDR Sequence Version: 0x%08x\n", (unsigned int)act->call.para[0]);
            }else if (act->call.func == FUNC_UDELAY) {
                udelay((unsigned int)act->call.para[0]);
                DBG("DDR Delay %dus\n",act->call.para[0])
                LocalDBG("FUNC_UDELAY...\n");
            } else if (act->call.func == FUNC_DIAG) {
                run_ddr_diag(act->call.para[0]);
            }
        }
    }

    return 0;
}
