
#include <per/per_test.h>
#include <module_helper_hal_internal.h>
/*init*/
static struct res_handle_item sx_s2[] = {
//{RSTGEN_ID_ISO_CPU1, RST_HOLD},
    {CLK_ID_VPU_BUS_0, 333000000},
    {CLK_ID_VPU_BUS_0, 1},
    {CLK_ID_CPU1A_2, 200000000},
    {CLK_ID_CPU1A_0, 800000000},
    {CLK_ID_CPU1A_2, 400000000},
    {CLK_ID_CPU1A_0, 0},

    {CLK_ID_CPU1A_2, 0},

//{CLK_ID_VPU_BUS_0, 666000000},
//{CLK_ID_CPU1A_0, 842000000},
//{CLK_ID_MJPEG_0, 100000000},

    NULL_RES_HANDLE_ITEM,
};

/*uninit*/
static struct res_handle_item sx_s0[] = {
    {RSTGEN_ID_ISO_CPU1, RST_RELEASE},
    {CLK_ID_VPU_BUS_0, 0},
    {CLK_ID_CPU1A_2, 0},


    NULL_RES_HANDLE_ITEM,
};

struct res_handle_item
    *per_test_state_table[MAX_TEST_STATE][MAX_TEST_STATE] = {
    /*SYS_S0    SYS_S1  SYS_S2i1    SYS_S2*/
    /*SYS_S0*/{&sx_s0[0], NULL, NULL, &sx_s2[0]},
    /*SYS_S1*/{&sx_s0[0], NULL, NULL, &sx_s2[0]},
    /*SYS_S2i1*/{&sx_s0[0], NULL, NULL, &sx_s2[0]},
    /*SYS_S2*/{&sx_s0[0], NULL, NULL, NULL},
};

static void per_test_state_init(void)
{

}


