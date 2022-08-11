#include "animation_config.h"
#include "container.h"
#include "data_structure_def.h"
#include <app.h>
#include <lib/console.h>
#include <dcf.h>
#include <event.h>
#include <sdrpc.h>
#include "early_app_cfg.h"

void player_task(token_handle_t token);
void mjpeg_decoder_task(token_handle_t token);
void vpu2disp_proc(void* prod_rsp, void* cons_req);
uint32_t getReplaySts(void);

extern event_t eBAfinished;

void player_over_callback(token_handle_t token,void* para)
{
    USDBG("Player freeze, over.\n");
    event_signal(para,false);
}


void audio_task(token_handle_t token);

void animation_entry(void* token)
{
    event_t video_kick;
    event_init(&video_kick,0,EVENT_FLAG_AUTOUNSIGNAL);

    if(getReplaySts() == 0)
        unified_service_publish(token,ussRun);

    printf("Animation start.\r\n");

    token_handle_t decoder_token = token_create("decoder",(ctx_convert_t)vpu2disp_proc,0,NULL,NULL);
    token_handle_t player_token = token_create("player",NULL,400,player_over_callback,&video_kick);

    token_serialization(2,decoder_token,player_token);
    thread_t* decoder = thread_create("decoder",(thread_start_routine)mjpeg_decoder_task,decoder_token,DEFAULT_PRIORITY,DEFAULT_STACK_SIZE);
    thread_t* player = thread_create("player",(thread_start_routine)player_task,player_token,DEFAULT_PRIORITY,DEFAULT_STACK_SIZE);
#if (TARGET_REFERENCE_X9 == 1)||(FAST_AUDIO_CFG0 == 1)
    if(getReplaySts() == 0)
    {
        thread_t* audio = thread_create("audio",(thread_start_routine)audio_task,NULL,DEFAULT_PRIORITY,DEFAULT_STACK_SIZE);
        thread_detach_and_resume(audio);
    }
#endif
    thread_detach_and_resume(decoder);
    thread_detach_and_resume(player);

    event_wait(&video_kick);

    if(token_getstatus(decoder_token) != TOKEN_FIN)
        token_setstatus(decoder_token,TOKEN_ABNORMAL);
    token_destroy(decoder_token);

    if(token_getstatus(player_token) != TOKEN_FIN)
        token_setstatus(player_token,TOKEN_ABNORMAL);
    token_destroy(player_token);

    event_destroy(&video_kick);

    if(getReplaySts() == 0){
        unified_service_publish(token,ussTerminated);
        event_signal(&eBAfinished,true);
    }

    printf("Animation finish.\r\n");

}

#if 0
void start_animation_video(void* argv)
{
    event_signal(argv,false);
}

void start_animation_audio(void)
{
    printf("NON-IMPL yet.\n");
}


#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("av", "start animation video", (console_cmd)&start_animation_video)
STATIC_COMMAND("au", "start animation audio", (console_cmd)&start_animation_audio)
STATIC_COMMAND_END(animation_sets);
#endif

#endif
