#include <stdio.h>
#include <console.h>
#include <kernel/mutex.h>
#include <sys/types.h>

#include "opt3001.h"

int opt_sample_once(int argc, const cmd_args *argv)
{
    u16 value;
    float result;

    struct opt3001_device *opt = opt3001_init();
    if (!opt) {
        printf("opt device allocation failed");
        return 0;
    }

    value = opt3001_get_id(opt);
    if (value != 0x3001) {
        printf("read wrong id!\n");
        return 0;
    }

    printf("%s() device_id =0x%x\n", __func__, value);
    opt3001_config(opt, OPT3001_CONFIGURATION_CONTINUOUS);
    /*conversion time*/
    thread_sleep(1000);
    result = opt3001_get_result(opt);
    printf("%s() result =%f\n", __func__, result);
    opt3001_deinit(opt);

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("opt3001_sample", "opt sample once", (console_cmd)&opt_sample_once)
STATIC_COMMAND_END(opt);

#endif
