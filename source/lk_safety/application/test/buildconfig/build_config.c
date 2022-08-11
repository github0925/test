#include <lib/console.h>
#include <res_loader.h>
#include <heap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#
#define BUILD_CONFIG_PATH "make_config/make_config.conf"

static int print_build_config(int argc, const cmd_args *argv)
{
    uint8_t *build_config_info = NULL;
    int build_config_size = 0;

    build_config_size = ROUNDUP(res_size(BUILD_CONFIG_PATH), 32);
    build_config_info = (uint8_t *)memalign(32, build_config_size);

    res_load(BUILD_CONFIG_PATH, build_config_info, build_config_size,  0);
    build_config_info[res_size(BUILD_CONFIG_PATH)]=0;
    printf("Image Build Config Meta Data:\r\n%s\r\n",(char *)build_config_info);

    free(build_config_info);
    build_config_info = NULL;

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("build_config", "Print Build config", print_build_config)
STATIC_COMMAND_END(build_config);
#endif
