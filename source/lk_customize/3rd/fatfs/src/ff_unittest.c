#include <stdlib.h>
#include <lib/console.h>
#include <app.h>
#include <debug.h>
#include <sdunittest.h>
#include "ff.h"

static  FATFS fatfs;

static bool ff_test_mount(int argc, const cmd_args *argv)
{
    dprintf(CRITICAL, "ff mount test start!\n");
    FRESULT rc;

    rc = f_mount(&fatfs, argv[1].str, 1);
    if(rc)
    {
        printf("ERROR: f_mount returned %d\r\n",rc);
        return false;
    }
    printf("f_mount success\r\n");

    dprintf(CRITICAL, "ff mount test end!\n");;
    return true;
}

static bool ff_test_read(int argc, const cmd_args *argv)
{
    dprintf(CRITICAL, "ff read test start!\n");
    FIL fil;
    FRESULT rc;
    UINT br;
    uint32_t offset = atoui(argv[2].str);
    uint8_t *buf = memalign(512, 512 * 16);

    rc=f_open(&fil, argv[1].str, FA_READ);
    if(rc)
    {
        printf("ERROR:f_open returned %d\r\n",rc);
        return false;
    }
    rc = f_lseek(&fil, offset);
    if(rc)
    {
        printf("ERROR:f_lseek returned %d\r\n",rc);
        return false;
    }
    rc = f_read(&fil, (void*)buf, 512 * 16, &br);
    if(rc)
    {
        printf("ERROR:f_read returned %d\r\n",rc);
        return false;
    }
    rc = f_close(&fil);
    if(rc)
    {
        printf("ERROR:f_open returned %d\r\n",rc);
        return false;
    }

    printf("the read string is: %s\n", buf);
    dprintf(CRITICAL, "ff read test end!\n");
    return true;
}

static bool ff_test_write(int argc, const cmd_args *argv)
{
    dprintf(CRITICAL, "ff write test start!\n");
    FIL fil;
    FRESULT rc;
    UINT br;
    uint32_t offset = atoui(argv[2].str);
    const char *buf = "fat: helloworld\n";

    rc=f_open(&fil, argv[1].str, FA_CREATE_ALWAYS);
    if(rc)
    {
        printf("ERROR:f_open returned %d\r\n",rc);
        return false;
    }

    rc = f_close(&fil);
    if(rc)
    {
        printf("ERROR:f_open returned %d\r\n",rc);
        return false;
    }

    rc=f_open(&fil, argv[1].str, FA_WRITE);
    if(rc)
    {
        printf("ERROR:f_open returned %d\r\n",rc);
        return false;
    }

    rc = f_lseek(&fil, offset);
    if(rc)
    {
        printf("ERROR:f_lseek returned %d\r\n",rc);
        return false;
    }
    rc = f_write(&fil, (void*)buf, strlen(buf) + 1, &br);
    if(rc)
    {
        printf("ERROR:f_write returned %d\r\n",rc);
        return false;
    }
    rc = f_close(&fil);
    if(rc)
    {
        printf("ERROR:f_open returned %d\r\n",rc);
        return false;
    }

    printf("the write string is: %s\n", buf);
    dprintf(CRITICAL, "ff write test end!\n");
    return true;
}

static int ff_read_thread(void *arg)
{
    cmd_args argv[3] = {};

    argv[1].str = arg;
    argv[2].str = "0";

    ff_test_read(3, argv);

    return 0;
}

static int ff_write_thread(void *arg)
{
    cmd_args argv[3] = {};

    argv[1].str = arg;
    argv[2].str = "0";

    ff_test_write(3, argv);

    return 0;
}

static bool ff_test_task(int argc, const cmd_args *argv)
{
    thread_t *read_thread =
        thread_create("ff_read_thread", ff_read_thread, (void*)argv[1].str,
                      DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach(read_thread);
    thread_resume(read_thread);
    thread_t *write_thread =
        thread_create("ff_write_thread", ff_write_thread, (void*)argv[1].str,
                      DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach(write_thread);
    thread_resume(write_thread);

    return true;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("ff_test_mount", "ff mount unittest case",
               (console_cmd)&ff_test_mount)
STATIC_COMMAND("ff_test_read", "ff read file unittest case",
               (console_cmd)&ff_test_read)
STATIC_COMMAND("ff_test_write", "ff write file unittest case",
               (console_cmd)&ff_test_write)
STATIC_COMMAND("ff_test_task", "ff concurrency unittest case",
               (console_cmd)&ff_test_task)
STATIC_COMMAND_END(ff_test);
#endif


APP_START(ff_test)
.flags = 0,
APP_END
