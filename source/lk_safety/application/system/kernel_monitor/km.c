#include <lib/console.h>
#include <stdio.h>
#include <heap.h>
#include <string.h>
#include <app.h>
#include <lk_wrapper.h>


void sdrvDumpTaskInfo( void );
void sdvrTaskStateInfo(void);

struct km_options_t;
typedef int(*km_options_cb_t)(int,const cmd_args*,struct km_options_t*);

struct km_options_t
{
    const char* pattern;
    const char* help;
    km_options_cb_t opt_cb;
    struct km_options_t* subopt;
};

int km_heap_dump_cb(int argc, const cmd_args* argv,struct km_options_t* me)
{
    heap_info();

    return 0;
}


int km_task_dump_cb(int argc, const cmd_args* argv,struct km_options_t* me)
{
    if(argc < 1)
    {
        sdvrTaskStateInfo();
    }
    else
    {
        return -1;
    }


    return 0;
}

struct km_options_t heap =
{
    "heap",
    "dump heap debug info & list.",
    km_heap_dump_cb,
    NULL,
};

struct km_options_t task =
{
    "task",
    "dump task runtime info. args:\nnone - show info\n(sr) [s] - set sample rate in s",
    km_task_dump_cb,
    NULL,
};

struct km_options_t* kmopt[] =
{
    &heap,
    &task,
};

int kernel_monitor(int argc, const cmd_args *argv)
{
    if(argc < 2)
    {
        printf("km [options]\noptions:\n");
        for(uint32_t i=0;i<sizeof(kmopt)/sizeof(kmopt[0]);i++)
        {
            printf("--------------------------\n");
            printf("%s : %s\n",kmopt[i]->pattern,kmopt[i]->help);
        }
    }
    else
    {
        uint32_t i = 0;

        for(i=0;i<sizeof(kmopt)/sizeof(kmopt[0]);i++)
        {
            if(!strcmp(argv[1].str,kmopt[i]->pattern))
            {
                if(kmopt[i]->opt_cb(argc-2,(const cmd_args*)argv+2,kmopt[i]))
                {
                    printf("Invalid args of %s\n",kmopt[i]->pattern);
                    printf("--------------------------\n");
                    printf("%s : %s\n",kmopt[i]->pattern,kmopt[i]->help);
                }

                break;
            }
        }

        if(i==sizeof(kmopt)/sizeof(kmopt[0]))
        {
            printf("KM:Invalid cmd:%s\n",argv[1].str);
        }

    }

    return 0;

}

int kernel_top_task(int argc, const cmd_args *argv)
{
    sdrvDumpTaskInfo();

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("km", "kernel monitor tools", (console_cmd)&kernel_monitor)
STATIC_COMMAND("top", "dump task stats", (console_cmd)&kernel_top_task)
STATIC_COMMAND_END(kernel_monitor);
#endif

