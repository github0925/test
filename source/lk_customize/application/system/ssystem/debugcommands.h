#ifndef __DEBUGCOMMANDS_H__
#define __DEBUGCOMMANDS_H__

enum clk_type {
    IP_SLICE = 0,
    BUS_SLICE,
    CORE_SLICE
};

typedef struct clk_mapping {
    char name[24];
    enum clk_type type;
    uint32_t res_id;
}clk_mapping_t;

#endif
