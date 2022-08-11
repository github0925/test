/*
 * system_configs_parse.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include "stdio.h"
#include "debug.h"

#include "system_configs_parse.h"

#if !SUPPORT_FAST_BOOT
#define LOG_LEVEL ALWAYS
#else
#define LOG_LEVEL INFO
#endif
//config initialized status
static bool init_done = false;
static uint32_t config_count = 0;
static addr_t addr_module_header = 0;

int configs_check(void)
{
    return CONFIG_NO_ERROR;
}

void configs_info(config_header_t *config_header)
{
    //config_info
    addr_t addr_header = config_header->config_offset + CONFIGS_BASE;
    module_header_t *module_header = (module_header_t*)addr_header;
    addr_t addr_info = module_header->config_offset + CONFIGS_BASE;
    config_info_t *config_info = (config_info_t*)(addr_info);

    dprintf(LOG_LEVEL, "configs info: \n\tversion: \t%d\n\tconfig count: \t%d\n\tmagic number: \t%d\n"
            "\tbuilder: \t%s\n\tbuild time: \t%s\n\tcomments: \t%s\n%d module types as followings:\n",
            config_header->version, config_header->config_count - 1, config_header->magic,
            config_info->username, config_info->build_time, config_info->comments, config_header->config_count - 1);

    addr_header += MODULE_HEADER_SIZE; //skip config_info
    for (uint32_t i = 1; i < config_count; i++) {
        module_header = (module_header_t*)addr_header;
        dprintf(LOG_LEVEL, "\tmodule type: %d\n", module_header->module_type);
        addr_header += MODULE_HEADER_SIZE;
    }
}

int configs_init(void)
{
    if (init_done) return CONFIG_NO_ERROR;

    int result = configs_check();
    if (result) {
        return result;
    }

    config_header_t *config_header = (config_header_t*)CONFIGS_BASE;
    config_count = config_header->config_count;
    addr_module_header = config_header->config_offset + CONFIGS_BASE;
    init_done = true;
    configs_info(config_header);

    dprintf(INFO, "config_head addr: 0x%lx, config_count: %d\n", (addr_t)config_header, config_count);

    return result;
}

int get_config_info(config_module_type_t module_type, addr_t * addr_base)
{
#if !SYS_CFG_VALID
    return CONFIG_MODULE_NOT_FOUND;
#endif

    int result = CONFIG_NO_ERROR;
    addr_t addr_header = 0;
    module_header_t *module_header;
    uint32_t i = 0;

    if (!init_done) {
        result = configs_init();
        if (CONFIG_NO_ERROR != result) {
            return result;
        }
    }
    addr_header = addr_module_header;
    for (; i < config_count; i++) {
        module_header = (module_header_t*)addr_header;

        if (module_type == module_header->module_type) {
            *addr_base = module_header->config_offset + CONFIGS_BASE;
            break;
        }

        addr_header += MODULE_HEADER_SIZE;
    }

    if (i >= config_count) {
        return CONFIG_MODULE_NOT_FOUND;
    }

    return result;
}
