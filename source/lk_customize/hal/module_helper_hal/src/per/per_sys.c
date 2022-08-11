
#include <per/per_sys.h>
#include <module_helper_hal_internal.h>
/*init*/
static struct res_handle_item sx_s2[] = {
    /*pll cpu1a, pll_cpu1b*/
    {RSTGEN_ID_ISO_CPU1, RST_HOLD},
    {RSTGEN_ID_MODULE_CPU1_SS, RST_RELEASE},
    {CLK_ID_CPU1A_2, 6000000},
    {CLK_ID_CPU1A_2, 1},

    /* pll cpu2*/
    {RSTGEN_ID_MODULE_CPU2_SS, RST_RELEASE},
    {CLK_ID_CPU2_2, 6000000},
    {CLK_ID_CPU2_2, 1},
    /*pll gpu1*/
    {RSTGEN_ID_ISO_GPU1, RST_HOLD},
    {RSTGEN_ID_MODULE_GPU1_SS, RST_RELEASE},
    {RSTGEN_ID_MODULE_GPU1_CORE, RST_RELEASE},
    {CLK_ID_GPU1_2, 1},
    /*pll gpu2*/
    {RSTGEN_ID_MODULE_GPU2_SS, RST_RELEASE},
    {RSTGEN_ID_MODULE_GPU2_CORE, RST_RELEASE},
    {CLK_ID_GPU2_2, 1},
    /*pll vpu*/
    {CLK_ID_VPU_BUS_1, 1},

    /*pll vsn*/
    {CLK_ID_VSN_BUS_1, 6000000},
    {CLK_ID_VSN_BUS_1, 1},

    NULL_RES_HANDLE_ITEM,
};

/*uninit*/
static struct res_handle_item sx_s0[] = {
    {RSTGEN_ID_ISO_CPU1, RST_RELEASE},
    {CLK_ID_VPU_BUS_0, 0},

    NULL_RES_HANDLE_ITEM,
};

struct res_handle_item *per_sys_state_table[MAX_SYS_STATE][MAX_SYS_STATE]
    = {
    /*SYS_S0    SYS_S1  SYS_S2i1    SYS_S2*/
    /*SYS_S0*/{&sx_s0[0], NULL, NULL, &sx_s2[0]},
    /*SYS_S1*/{&sx_s0[0], NULL, NULL, &sx_s2[0]},
    /*SYS_S2i1*/{&sx_s0[0], NULL, NULL, &sx_s2[0]},
    /*SYS_S2*/{&sx_s0[0], NULL, NULL, NULL},
};

static void per_sys_state_init(void)
{
}


