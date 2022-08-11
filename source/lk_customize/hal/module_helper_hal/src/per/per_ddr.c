
#include <per/per_ddr.h>
#include <module_helper_hal_internal.h>
/*init*/
#define COMMON_INIT \
    {RSTGEN_ID_ISO_DDR, RST_RELEASE},  \
    {RSTGEN_ID_MODULE_DDR_SS, RST_RELEASE}, \
    {RSTGEN_ID_MODULE_DDR_SW_1, RST_RELEASE},   \
    {CLK_ID_DDR_0, 24000000},  \
    {CLK_ID_DDR_0, 1},  \
    {CLK_ID_DDR_1, 12000000},  \
    {CLK_ID_DDR_1, 1},  \
    {CLK_ID_DDR_2, 6000000},    \
    {CLK_ID_DDR_2, 1}, \
    {CLK_ID_DDR_3, 24000000},  \
    {CLK_ID_DDR_3, 1},


static struct res_handle_item s0_s200[] = {
    COMMON_INIT
    {CLK_ID_DDR_0, 200000000},
    NULL_RES_HANDLE_ITEM,
};
static struct res_handle_item sx_s200[] = {
    {CLK_ID_DDR_0, 200000000},
    NULL_RES_HANDLE_ITEM,
};
static struct res_handle_item s0_s400[] = {
    COMMON_INIT
    {CLK_ID_DDR_0, 400000000},
    NULL_RES_HANDLE_ITEM,
};
static struct res_handle_item sx_s400[] = {
    {CLK_ID_DDR_0, 400000000},
    NULL_RES_HANDLE_ITEM,
};
static struct res_handle_item s0_s532[] = {
    COMMON_INIT
    {CLK_ID_DDR_0, 532000000},
    NULL_RES_HANDLE_ITEM,
};
static struct res_handle_item sx_s532[] = {
    {CLK_ID_DDR_0, 532000000},
    NULL_RES_HANDLE_ITEM,
};
static struct res_handle_item s0_s600[] = {
    COMMON_INIT
    {CLK_ID_DDR_0, 600000000},
    NULL_RES_HANDLE_ITEM,
};
static struct res_handle_item sx_s600[] = {
    {CLK_ID_DDR_0, 600000000},
    NULL_RES_HANDLE_ITEM,
};

static struct res_handle_item s0_s800[] = {
    COMMON_INIT
    {CLK_ID_DDR_0, 800000000},
    NULL_RES_HANDLE_ITEM,
};
static struct res_handle_item sx_s800[] = {
    {CLK_ID_DDR_0, 800000000},
    NULL_RES_HANDLE_ITEM,
};

static struct res_handle_item s0_s1066[] = {
    COMMON_INIT
    {CLK_ID_DDR_0, 1066000000},
    NULL_RES_HANDLE_ITEM,
};
static struct res_handle_item sx_s1066[] = {
    {CLK_ID_DDR_0, 1066000000},
    NULL_RES_HANDLE_ITEM,
};

/*uninit*/
static struct res_handle_item sx_s0[] = {
    {CLK_ID_DDR_0, 0},
    {CLK_ID_DDR_1, 0},
    {RSTGEN_ID_MODULE_DDR_SW_1, RST_HOLD},
    {RSTGEN_ID_MODULE_DDR_SS, RST_HOLD},
    {RSTGEN_ID_ISO_DDR, RST_RELEASE},
    NULL_RES_HANDLE_ITEM,
};

struct res_handle_item
    *per_ddr_state_table[MAX_DDR_STATE][MAX_DDR_STATE] = {
    /*S0 S1 200m, 400M, 532M,800M, 1066M*/
    /*SYS_S0*/{&sx_s0[0], NULL, &s0_s200[0], &s0_s400[0], &s0_s532[0], &s0_s600[0], &s0_s800[0], &s0_s1066[0]},
    /*SYS_S1*/{&sx_s0[0], NULL, &s0_s200[0], &s0_s400[0], &s0_s532[0], &s0_s600[0], &s0_s800[0], &s0_s1066[0]},
    /*200M*/{&sx_s0[0], NULL, NULL, &sx_s400[0], &sx_s532[0], &sx_s600[0], &sx_s800[0], &sx_s1066[0]},
    /*400M*/{&sx_s0[0], NULL, &sx_s200[0], NULL, &sx_s532[0], &sx_s600[0], &sx_s800[0], &sx_s1066[0]},
    /*532M*/{&sx_s0[0], NULL, &sx_s200[0], &sx_s400[0], NULL, &sx_s600[0], &sx_s800[0], &sx_s1066[0]},
    /*600M*/{&sx_s0[0], NULL, &sx_s200[0], &sx_s400[0], &sx_s532[0], NULL, &sx_s800[0], &sx_s1066[0]},
    /*800M*/{&sx_s0[0], NULL, &sx_s200[0], &sx_s400[0], &sx_s532[0], &sx_s600[0], NULL, &sx_s1066[0]},
    /*1066M*/{&sx_s0[0], NULL, &sx_s200[0], &sx_s400[0], &sx_s532[0], &sx_s600[0], &sx_s800[0], NULL},
};

const char *ddr_get_state_name(int state)
{
    switch (state) {
        case DDR_S0:
            return "s0";

        case DDR_S1:
            return "s1";

        case DDR_200M:
            return "200M";

        case DDR_400M:
            return "400M";

        case DDR_532M:
            return "532M";

        case DDR_600M:
            return "600M";

        case DDR_800M:
            return "800M";

        case DDR_1066M:
            return "1066M";

        default:
            return "unknown state";
    }
}

static void per_ddr_state_init(void)
{
}


