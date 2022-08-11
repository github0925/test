/********************************************************
 *	        Copyright(c) 2020	Semidrive 		        *
 *******************************************************/

#include <debug.h>
#include <reg.h>
#include "lib/reg.h"
#include <chip_res.h>
#include "ddr_init_helper.h"

//#define LocalDBG(fmt, args...)   printf(fmt, ##args)
#define LocalDBG(fmt, args...)

extern void reset_module(uint32_t resid);

int32_t run_ddr_init_seq(const ddr_act_u *act)
{
    for (; act->call.act != ACT_END; act++) {
        if (ACT_WR == act->call.act) {
            LocalDBG("writel(0x%x, 0x%x)\n", act->wr.val, act->wr.addr);
            writel(act->wr.val, act->wr.addr);
        } else if (ACT_WR_BITS == act->call.act) {
            LocalDBG("RMWREG32(0x%x, 0x%x, 0x%x, 0x%x)\n",
                    act->wrbits.addr, act->wrbits.shift, act->wrbits.width, act->wrbits.val);
            RMWREG32(act->wrbits.addr, act->wrbits.shift, act->wrbits.width, act->wrbits.val);
        } else if (ACT_POLL_BITS == act->call.act) {
            uint32_t cnt = 0;
            uint32_t tmt = (((uint32_t)(act->poll.tmt_h)) << 16) | ((uint32_t)(act->poll.tmt));
            uint32_t a = act->poll.addr;
            uint32_t v = act->poll.val;
            uint32_t msk = act->poll.msk;
            LocalDBG("poll(0x%x, 0x%x, 0x%x, %d)\n", a, msk, v, tmt);
            while ((cnt++ < (uint32_t)(tmt)) && ((readl(a) & (msk)) != ((v) & (msk))));
        } else if (ACT_FUNC_CALL == act->call.act) {
            if (act->call.func == FUNC_RESET) {
                LocalDBG("FUNC_RESET(%d)\n", act->call.para[0]);
#if defined(DDR_SQUEEZER)
                rg_module_reset(APB_RSTGEN_SEC_BASE, act->call.para[0], 1);
#else
                if (DDR_AXI_RSTN == act->call.para[0]) {
                    reset_module(RES_MODULE_RST_SEC_DDR_SW_2);
                } else if (DDR_CORE_RSTN == act->call.para[0]) {
                    reset_module(RES_MODULE_RST_SEC_DDR_SW_3);
                }
#endif
            } else if (act->call.func == FUNC_LOADFW) {
                LocalDBG("FUNC_LOADFW(%d,%d)\n", act->call.para[0], act->call.para[1]);
                load_ddr_training_fw(act->call.para[0], act->call.para[1]);
            } else if (act->call.func == FUNC_FW_MON) {
                LocalDBG("FUNC_FW_MON...\n");
                if (0 != dwc_ddrphy_fw_monitor_loop(act->call.para[0])) {
                    return -1;
                }
            } else if (act->call.func == FUNC_GET_TRAIN_PARA) {
                LocalDBG("FUNC_GET_TRAIN_PARA()\n");
                _dwc_ddrphy_phyinit_userCustom_H_readMsgBlock_(0);
            } else if (act->call.func == FUNC_USE_TRAIN_PARA) {
                LocalDBG("FUNC_USE_TRAIN_PARA\n");
                _timing_reg_update_after_training_();
            } else if (act->call.func == FUNC_CFG_CLK) {
                uint32_t v = act->call.para[0];
                const char *s = v == FREQ_DDR_4266 ? "DDR_4266" :
                            v == FREQ_DDR_2133 ? "DDR_2133" :
                            v == FREQ_DDR_3200 ? "DDR_3200" :
							v == FREQ_DDR_2400 ? "DDR_2400" :
                            v == FREQ_DDR_1600 ? "DDR_1600" :
                            "Invalid Data Rate";
                printf("DDR Data Rate: %s\n", s);
#if defined(DDR_SQUEEZER)
                soc_config_clk(DDR_SS, act->call.para[0]);
#endif
            }else if (act->call.func == FUNC_PRINT_SEQ_VERSION) {
                printf("DDR Sequence Version: 0x%08x\n", act->call.para[0]);
            }
        }
    }

    return 0;
}
