/*
* app_dma.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement the app to auto jump to kernel.
*
* Revision History:
* -----------------
* 001, 10/17/2019 shaoyi create this file
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>

#include "dma_cases.h"
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
static void dma_app_init(const struct app_descriptor *app)
{
    printf("dma app init completed.\n");
    init_dma_cases();
}

//static dmac_src_t g_dmac_app_resource = dmac_res_def;

void usage_string(void)
{
    printf("Usage: dma [-case <caseid>]\n");
    printf("           [-list \n");
    printf("           [-curr_cfg]  \n");
    printf("Sample:     \n");
    printf("        'dma -case 1' run case 1. \n");
    printf("        'dma -list '  list all samples. \n");
    printf("        'dma -help' list available subcommands and some concept. \n");
}
int dma_main(int argc, const cmd_args *argv)
{
    int caseid = 0;
    for (int i = 1; i < argc; i++)
    {
#ifdef DMA_DEBUG_PRINT
        printf("\t%d: str '%s', i %ld, u %#lx, b %d\n", i, argv[i].str, argv[i].i, argv[i].u, argv[i].b);
#endif
        if (!strcmp(argv[i].str, "-help"))
        {
            usage_string();
            return 0;
        }
        else if (!strcmp(argv[i].str, "-case") && i + 1 < argc)
        {
            caseid = argv[++i].i;
            printf("case:%d %s\n", caseid, dma_sample_cases[caseid].name);
            run_case(caseid);
            return 0;
        }
        else if (!strcmp(argv[i].str, "-list"))
        {
            for (int i = 0; i < SAMPLE_CASES_NUM; i++)
            {
                printf("%s %-20d %s \n", case_type_str[dma_sample_cases[i].tst_type], i, dma_sample_cases[i].name);
            }
            return 0;
        }
        else if (!strcmp(argv[i].str, "-init"))
        {
            init_dma_cases();
        }
    }
    return 0;
}
#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("dma", "dma example cases, e.g dma -help", (console_cmd)&dma_main)
STATIC_COMMAND_END(dma_test);

#endif
//   .init = dma_app_init,
APP_START(dma_example)
    .flags = 0 APP_END
