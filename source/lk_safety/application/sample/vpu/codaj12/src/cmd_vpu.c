/*
 * Copyright (c) 2008-2014 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/console.h>
#include <kernel/thread.h>

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include "vpu_hal.h"
#include "jpulog.h"


/**
 * brief
 *
 * param
 * return
 * note
 */
void codaj12_init(void)
{

    int ret = 0;
    Uint32  apiVersion, hwRevison, hwProductId;
    struct vpu_instance  *instance = NULL;

    if (!(hal_vpu_create_handle((void **)&instance, RES_MJPEG_MJPEG))) {
        JLOG(ERR, "Err: create resource instance error \n");
        return;
    }

    ret = hal_vpu_init(instance);

    if (ret != JPG_RET_SUCCESS && ret != JPG_RET_CALLED_BEFORE) {
        JLOG(ERR, "Err: JPU_Init failed Error code is 0x%x \n", ret );
        return;
    }

    hal_vpu_get_version(instance, &apiVersion, &hwRevison, &hwProductId);
    JLOG(INFO, "Codaj12  version api-version = 0x%x, hw_revision=%d, hw_product=0x%x\n", apiVersion, hwRevison, hwProductId);

    hal_vpu_deinit(instance);
    hal_vpu_release_handle(instance);
    instance = NULL;

}

int main_dec(int argc, char **argv);

/**
 * brief
 *
 * param
 * return
 * note
 */
int do_codaj12_dec(int argc, const cmd_args *argv)
{
    int i = 0;
    char *buf[20] = {0};
    JLOG(INFO, "do codaj12 _dec start, input param ... \n");

    for (i = 0; i < argc; i++) {
        buf[i] = (char *)argv[i].str;
        JLOG(INFO, "Param %d: %s\n", i, buf[i]);
    }

    main_dec(argc, buf);
    JLOG(INFO, "do_codaj12_dec end\n");
    return 0;
}


#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("codaj12_dec", "a sample app ", (console_cmd)&do_codaj12_dec)
STATIC_COMMAND("codaj12_init", "a sample app ", (console_cmd)&codaj12_init)
STATIC_COMMAND_END(codaj12sample);

#endif

APP_START(vpu)
.flags = 0
         APP_END
