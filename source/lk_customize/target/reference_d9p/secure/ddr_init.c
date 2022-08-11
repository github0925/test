/********************************************************
 *	        Copyright(c) 2020	Semidrive 		        *
 *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <reg.h>
#include <__regs_base.h>
#include "chip_res.h"
#include "lib/reg.h"
#include "rstgen_hal.h"
#include "scr_hal.h"
#if DDR_INIT_N_TRAINING
#include "ddr_init_helper.h"
#endif

void reset_module(uint32_t resid)
{
    bool ret = true;
    void *handle = NULL;

    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        return ;
    }

    hal_rstgen_init(handle);
    hal_rstgen_module_reset(handle, resid);
    hal_rstgen_release_handle(handle);
}

static void setup_scr(uint64_t resid, uint32_t v)
{
    scr_handle_t handle;

    handle  = hal_scr_create_handle(resid);

    hal_scr_get(handle);
    hal_scr_set(handle, v);

    hal_scr_delete_handle(handle);
}

#if CFG_PARSE_RUN_DDR_INIT_SEQ
#define DDR_INIT_SEQ_ITEM_NUM_MAX   1024
ddr_act_u ddr_init_seq[DDR_INIT_SEQ_ITEM_NUM_MAX];
const uint32_t ddr_init_seq_sz_max = sizeof(ddr_init_seq);
#endif
uint32_t ddr_init(void)
{
#if defined(SAF_CORE)
    setup_scr(scr_safETY__RW__ddr_ss_pwrokin_aon,0x1);
#else
    setup_scr(SCR_SEC__RW__ddr_ss_pwrokin_aon,0x1);
#endif

#if CFG_PARSE_RUN_DDR_INIT_SEQ
    if (0 != run_ddr_init_seq(&ddr_init_seq[0])) {
        return -1;
    }
#else
#ifdef DDR_SCRIPT_PATH
    #include DDR_SCRIPT_PATH
#else
    dprintf(CRITICAL, "No ddr script found!\n");
    return 1;
#endif
#endif  /* CFG_PARSE_RUN_DDR_INIT_SEQ */

	return 0;
}
