#ifndef __PRELOADER_CONFIG__H__
#define __PRELOADER_CONFIG__H__

#include <stdint.h>

#ifndef MAX_GPT_NAME_SIZE
#define MAX_GPT_NAME_SIZE 72
#endif

#define FULL_PT_NAME (MAX_GPT_NAME_SIZE*2+2)
#define PT_LD_AP2SEC (1u << 0)
#define PT_LD_PEER   (1u << 1)
#define PT_LD_JUMP   (1u << 2)
#define PT_LD_DECD   (1u << 3)

typedef int ( *complete_load)(void* config, void* arg);

struct pt_load_config {
    uint64_t base;
    uint64_t sz;
    uint32_t flag;
    char pt_name[FULL_PT_NAME];
};

#define PT_LOAD_CONFIGS_START(name)  static struct pt_load_config name[]= {\

/* loader order, load addr, load size, flag, partition name
* */
#define PT_LOAD_CONFIG_ITEM(addr, size, flags, name)             \
    {                                                            \
        .base = addr,                                            \
        .sz = size,                                              \
        .flag = flags,                                           \
        .pt_name = {#name}                                       \
    },

#define PT_LOAD_CONFIGS_END };

#define PT_LOAD_CONFIGS(name) \
        PT_LOAD_CONFIGS_START(name) \
        PT_CONFIGS                     \
        PT_LOAD_CONFIGS_END

#endif


