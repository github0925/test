#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdm_display.h>
#include <lib/reg.h>
#include <string.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <kernel/spinlock.h>

#include <app.h>
#include <res.h>
#include <trace.h>

#include <chip_res.h>

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

#define DISPLAY_NUM    5

static double strtof(const char *str)
{
    double s = 0.0;
    double d = 10.0;
    bool nflag = false;

    while (*str == ' ')
        str++;

    if (*str == '-') {
        nflag = true;
        str++;
    }

    if (!((*str >= '0') && (*str <= '9')))
        return s;

    while ((*str >= '0') && (*str <= '9') && (*str != '.')) {
        s = s * 10.0 + *str - '0';
        str++;
    }

    if (*str == '.')
        str++;

    while ((*str >= '0') && (*str <= '9'))
    {
        s = s + (*str - '0') / d;
        d *= 10.0;
        str++;
    }

    return s * (nflag ? -1.0 : 1.0);
}

static int pq_set(int argc, const cmd_args *argv)
{
    #define PI  3.14159

    display_handle *handle;
    struct sdm_pq_params pq;
    int id = 0;

    pq.contrast = 1; //k
    pq.luminance = 0;//l
    pq.saturation = 1;//s
    pq.chroma = 0;//h

    if (argc < 3) {
        LOGE("pq test\n");
        LOGE("pq [id] [-k] [-l] [-s] [-h] \n");
        LOGE("   -k: contrast\n");
        LOGE("   -l: luminance\n");
        LOGE("   -s: saturation\n");
        LOGE("   -h: chroma\n");
        return 0;
    }

    id = argv[1].u;
    if ((id > (DISPLAY_NUM - 1)) || (id < 0)) {
        id = 0;
        LOGE("id is illegal, using default 0.\n");
    }

    for (int i = 0; i < (argc - 2) / 2; i++) {
        if (!strcmp(argv[2 + 2 * i].str, "-k"))
            pq.contrast = strtof(argv[2 + 2 * i + 1].str);

        if (!strcmp(argv[2 + 2 * i].str, "-l"))
            pq.luminance = strtof(argv[2 + 2 * i + 1].str);

        if (!strcmp(argv[2 + 2 * i].str, "-s"))
            pq.saturation = strtof(argv[2 + 2 * i + 1].str);

        if (!strcmp(argv[2 + 2 * i].str, "-h"))
            pq.chroma = strtof(argv[2 + 2 * i + 1].str) * PI / 180;
    }

    printf("k:%f, l:%f, s:%f, h:%f\n", pq.contrast, pq.luminance, pq.saturation, pq.chroma);
    handle = hal_get_display_handle(id);
    if (!handle) {
        LOGE("display:%d is invalid\n", id);
        return -1;
    }

    sdm_pq_set(handle, &pq);
    return 0;
}
STATIC_COMMAND_START
STATIC_COMMAND("pq", "pq", &pq_set)
STATIC_COMMAND_END(pq);

#endif
