#ifndef __PRELOADER_CONFIG__H__
#define __PRELOADER_CONFIG__H__

#include <lk/macros.h>
#include <malloc.h>
#include "partition_parser.h"
#include "lib/reg.h"
#include "libavb.h"
#include "res.h"
#include "cpu_hal.h"

#define BOOT_DEVICE_USER_DATA 0
#ifndef MAX_GPT_NAME_SIZE
#define MAX_GPT_NAME_SIZE 72
#endif

#define FULL_PT_NAME (MAX_GPT_NAME_SIZE*2+2)

#define PT_AP2SEC_F   (1U << 0)
#define PT_KICK_F     (1U << 1)
#define PT_SAVE_F     (1U << 2)
#define PT_BAK_LOW_F  (1U << 3)
#define PT_BAK_HIGH_F (3U << 3)

#define PT_BAK_MASK       (PT_BAK_HIGH_F)
#define PT_BAK_POS(flags) (PT_BAK_MASK & flags)

typedef int ( *prepare_load)(void* config,void* arg);
typedef int ( *complete_load)(void* config, void* arg);

enum pt_type {
    PT_BL = 0x0,
    PT_SML,
    PT_TOS,
    PT_DTB,
    PT_HYPERVISOR,
    PT_USER,
    PT_TYPE_MAX
};

struct pt_load_config {
    enum pt_type type;
    uint32_t load_order;
    bool load_complete;
    uint64_t load_addr;
    uint64_t load_size;
    sd_cpu_id cpu_id;
    uint32_t flags;
    char pt_name[FULL_PT_NAME];
    prepare_load prepare;
    complete_load complete;
    void * pre_arg;
    void * post_arg;
    bool last_config;
};

#define PT_LOAD_CONFIGS_START(name)  static struct pt_load_config name[]={

/* pt_type, loader order, load addr, load size, running on which cpu,
 * needs to kick cpu, needs to save addr, needs to convert addr, partition name
* */
#define PT_LOAD_CONFIG_ITEM(pt_type, order, addr, size, cpu, flag, name)   \
    [order]={                                                   \
        .type = pt_type,                                        \
        .load_order = order,                                    \
        .load_complete = false,                                 \
        .load_addr = addr,                                      \
        .load_size = size,                                      \
        .cpu_id = cpu,                                          \
        .flags = flag,                                          \
        .pt_name = {#name},                                     \
        .prepare = NULL,                                        \
        .complete = NULL,                                       \
        .pre_arg  = NULL,                                       \
        .post_arg = NULL,                                       \
        .last_config = false,                                   \
    },                                                          \

#define PT_LOAD_CONFIGS_END  };

#define PT_LOAD_CONFIGS(name) \
        PT_LOAD_CONFIGS_START(name) \
        CONFIGS                     \
        PT_LOAD_CONFIGS_END

#if 0
static inline int register_load_prepare(struct pt_load_config *configs,
                    uint32_t count, uint32_t type, prepare_load prepare, void* arg)
{
    for (uint32_t i = 0; i < count; i++) {
        if (type == (uint32_t)configs[i].type) {
            configs[i].prepare = prepare;
            configs[i].pre_arg = arg;
            return 0;
        }
    }

    return -1;
}

static inline int register_load_complete(struct pt_load_config *configs,
        uint32_t count, uint32_t type, complete_load complete, void * arg)
{
    for (uint32_t i = 0; i < count; i++) {
        if (type == (uint32_t)configs[i].type) {
            configs[i].complete = complete;
            configs[i].post_arg = arg;
            return 0;
        }
    }

    return -1;
}

static struct pt_load_config *get_load_config(struct pt_load_config
        *configs, uint32_t count,
        uint32_t pt_type)
{
    for (uint32_t i = 0; i < count; i++) {
        if (configs[i].type == pt_type) {
            return &configs[i];
        }
    }

    return NULL;
}
#endif

static inline int register_load_complete_for_all(struct pt_load_config *configs,
        uint32_t count, complete_load complete, void * arg)
{
    for (uint32_t i = 0; i < count; i++) {
            configs[i].complete = complete;
            configs[i].post_arg = arg;
    }
    return 0;
}

static inline int register_load_prepare_for_all(struct pt_load_config *configs,
        uint32_t count, prepare_load prepare, void * arg)
{
    for (uint32_t i = 0; i < count; i++) {
            configs[i].prepare = prepare;
            configs[i].pre_arg = arg;
    }
    return 0;
}

static int load_all_partition(struct pt_load_config *configs,
                              uint32_t count, partition_device_t *ptdev)
{
    storage_device_t *storage     = NULL;
    const char *ptname            = NULL;
    unsigned long long ptn        = 0;
    unsigned long long pt_size    = 0;
    vaddr_t load_addr             = 0;
    struct pt_load_config *config = NULL;
    uint32_t switch_pt_num        = BOOT_DEVICE_USER_DATA;
    AvbFooter footer = {0};
    uint32_t bk_sz;
    uint8_t * block_buf = NULL;

    dprintf(INFO, "%s %d E\n", __func__, __LINE__);

    if (!ptdev || !ptdev->storage) {
        dprintf(CRITICAL, "%s %d\n", __func__, __LINE__);
        return -1;
    }

    storage = ptdev->storage;

    if (storage->switch_part)
        storage->switch_part(storage, switch_pt_num);

    bk_sz = storage->get_block_size(storage);
    block_buf = memalign(bk_sz, bk_sz);
    config = configs;

    for (uint32_t i = 0; i < count; i++, config++) {
        ptname   = config->pt_name;
        ptn      = ptdev_get_offset(ptdev, ptname);
        pt_size  = ptdev_get_size(ptdev, ptname);

        if (!ptn) {
            dprintf(CRITICAL, "partition %s no found.\n", ptname);
            continue;
        }

        if (block_buf && !storage->read(storage, ptn + pt_size - bk_sz, block_buf, bk_sz))
        {
            if (avb_footer_validate_and_byteswap((AvbFooter *)(block_buf+bk_sz - sizeof(AvbFooter)), &footer))
                    pt_size = footer.original_image_size;
        }

        pt_size = MIN(pt_size, config->load_size);
        config->load_size = pt_size;
        if (config->prepare && config->prepare((void*)config, config->pre_arg)) {
            dprintf(CRITICAL, "prepare to load partition error. name:%s\n", ptname);
            continue;
        }

        if (config->flags & PT_AP2SEC_F) {
            load_addr = (vaddr_t)_ioaddr(ap2p(config->load_addr));
        }
        else {
            load_addr = (vaddr_t)_ioaddr(config->load_addr);
        }

        if (storage->read(storage, ptn, (uint8_t *)load_addr, round_up(pt_size, bk_sz))){
            panic("load paritition error.name:%s \n", ptname);
        }

        config->load_complete = true;

        if (i == (count - 1))
            config->last_config = true;

        if (config->complete && config->complete((void*)config,configs->post_arg)) {
            panic("complete load partition error.name:%s\n", ptname);
        }
    }

    if (block_buf)
        free(block_buf);

    return 0;
}

static int fork_compatible_configs (sd_cpu_id cpu_array[], uint32_t num, struct pt_load_config *org_config,
                uint32_t org_count, struct pt_load_config **new_configs, uint32_t *new_count)
{
    uint32_t i, m;
    uint32_t match_count = 0;
    struct pt_load_config *local = NULL;
    uint32_t *match_id = calloc(org_count, sizeof(uint32_t));

    for (i = 0; i < org_count; i++) {
        for (m = 0; m < num; m++) {
              if (org_config[i].cpu_id == cpu_array[m]) {
                  match_id[match_count] = i;
                  match_count++;
              }
        }
    }

    if (!match_count)
        return -1;

    local = calloc(match_count, sizeof(*local));
    if (!local)
        return -1;
    for (i = 0; i < match_count; i++) {
        memcpy(local + i, org_config + match_id[i], sizeof(*local));
    }
    *new_configs = local;
    *new_count = match_count;

    free(match_id);
    return 0;
}
#endif


