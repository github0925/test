#include <thread.h>
#include <container.h>
#include <app.h>
#include <event.h>
#include "early_app_cfg.h"
#include "dcf.h"

static event_t eRepeat;
event_t eBAfinished;

void animation_entry(void* token);
static uint32_t iReplay = 0;
static uint32_t needReplay = 0;

uint32_t getReplaySts(void)
{
    return iReplay;
}

void ba_state_cb(void* pargs, int uargs,int singal)
{
    printf("ba_state_cb signal=%d\n",singal);
    if (singal == 2) {
        needReplay = 1;
    }
    event_signal(&eRepeat,true);
}

static void ba_replay_entry(const struct app_descriptor *app, void *args)
{

    event_init(&eRepeat,0,false);

    event_init(&eBAfinished,0,false);
    event_wait(&eBAfinished);

    unified_service_subscribe(DMP_ID_BA_STATUS,ussTerminated,
                            ba_state_cb,NULL,0);

    while(1){
        event_wait(&eRepeat);
        event_unsignal(&eRepeat);

        iReplay = 1;
        if (needReplay) {
            thread_t* rplayer = thread_create("ba_replay",(thread_start_routine)animation_entry,NULL,DEFAULT_PRIORITY,1024);
            thread_detach_and_resume(rplayer);
            needReplay = 0;
        }

        //thread_sleep(500);
        unified_service_subscribe(DMP_ID_BA_STATUS,ussTerminated,
                            ba_state_cb,NULL,0);
    }
}

void ba_signal(void)
{
    event_signal(&eRepeat,true);
}


#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("replay","Replay BootAnimation ", (console_cmd)&ba_signal)
STATIC_COMMAND_END(bootanimation);
#endif


APP_START(ba_replay)
.flags = 0,
.entry = ba_replay_entry,
APP_END