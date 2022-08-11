#include "res_loader.h"
#include <app.h>
#include <thread.h>
#include <event.h>
#include <container.h>
#include <res.h>
#include <sdrpc.h>
#include <dcf.h>
#include <dc_status.h>
#include <early_app_cfg.h>

void unified_servcie_load(void);

static queue_t sched_queue;
static uso_t* uso_sets[MAX_USO_NUM];
bool cluster_should_show_fps;

void unified_service_publish(void* token, usmState state)
{
    uso_t* uso = (uso_t*)token;
    uso->state = state;
    system_property_set(uso->property_id,uso->state);
    queue_send(uso->q,&uso);
}

static inline uso_t* unified_service_find_obj(int id)
{
    for(int i=0;i<MAX_USO_NUM;i++)
    {
        if(uso_sets[i]->property_id == id)
        {
            return uso_sets[i];
        }
    }

    return NULL;
}


static void unified_service_general_callback(int id, int old, int new, void* data)
{
    uso_t* pub = (uso_t*)data;

    uso_cbk_block_t cbk;

    USDBG("%s go into us general callback.\n",pub->name);

    usmState state = new & 0x0000FFFF;
    int signal = (new & 0xFFFF0000) >> 16;

    while(queue_query(&pub->cbk_channel[state],&cbk))
    {
        USDBG("Get callback. GO!\n");
        cbk.callback(cbk.pargs,cbk.uargs,signal);
    }

}

void unified_service_subscribe(
    int id,
    usmState state,
    uso_callback_t cb,
    void* pargs,
    int uargs)
{
    uso_t* pub = unified_service_find_obj(id);

    if(!pub) return;


    uso_cbk_block_t cbk;

    cbk.callback = cb;
    cbk.pargs = pargs;
    cbk.uargs = uargs;

    queue_send(&pub->cbk_channel[state],&cbk);


    return;

}


//make init to be an async invoking
static void async_unified_servcie_init(uso_t* uso)
{
    if(!uso->deps.init_depc)
    {
        uso->q = &sched_queue;

        if(uso->init)
        {
            char init_name[20];
            snprintf(init_name,20,"%s_init",uso->name);
            thread_t* t = thread_create(init_name,(thread_start_routine)uso->init,uso,DEFAULT_PRIORITY,4096);
            thread_detach_and_resume(t);
        }
        else
        {
            unified_service_publish(uso,ussReady);
        }
    }

}

static void async_unified_servcie_entry(uso_t* uso)
{
    if(!uso->deps.entry_depc && uso->state == ussReady)
    {
        uso->q = &sched_queue;

        if(uso->entry)
        {
            char entry_name[20];
            snprintf(entry_name,20,"%s_entry",uso->name);
            thread_t* t = thread_create(entry_name,(thread_start_routine)uso->entry,uso,DEFAULT_PRIORITY,4096);
            thread_detach_and_resume(t);
        }
        else
        {
            unified_service_publish(uso,ussTerminated);
        }
    }

}


static int unified_service_deps_validated(uso_t* child,uso_t* parent,uso_deps_meta_t** meta)
{
    for(int i=0;i<MAX_DEPS_PER_USO;i++)
    {
        if(child->deps.init[i].parent == parent)
        {
            *meta = &child->deps.init[i];
            return 0;
        }

        if(child->deps.entry[i].parent == parent)
        {
            *meta = &child->deps.entry[i];
            return 1;
        }
    }

    *meta = NULL;

    return -1;
}


int unified_servcie_deps_chain_append(
    uso_t* uso,
    uso_t* dep_uso,
    usmState toggle_state,
    int phase)
{

    for(int i=0;i<sizeof(uso_sets)/sizeof(uso_sets[0]);i++)
    {
        if(!uso_sets[i])
        {
            uso_sets[i] = uso;
            for(int i=0;i<ussMAX;i++)
            {
                queue_create(&uso->cbk_channel[i],MAX_DEPS_PER_USO,sizeof(uso_cbk_block_t));
            }
            system_property_observe(uso->property_id,unified_service_general_callback,uso);

            break;
        }
        else if(uso_sets[i] == uso)
        {
            break;
        }
    }



    if(!dep_uso)
    {
        return 0;
    }

    for(int i=0;i<MAX_DEPS_PER_USO;i++)
    {
        if(!dep_uso->child[i])
        {
            dep_uso->child[i] = uso;
            dep_uso->children++;
            break;
        }
    }

    if(!phase) //init phase
    {
        for(int i=0;i<MAX_DEPS_PER_USO;i++)
        {
            if(!uso->deps.init[i].parent)
            {
                uso->deps.init[i].parent = dep_uso;
                uso->deps.init[i].toggle_state = toggle_state;
                uso->deps.init_depc++;
                break;
            }
        }

    }
    else
    {
        for(int i=0;i<MAX_DEPS_PER_USO;i++)
        {
            if(!uso->deps.entry[i].parent)
            {
                uso->deps.entry[i].parent = dep_uso;
                uso->deps.entry[i].toggle_state = toggle_state;
                uso->deps.entry_depc++;
                break;
            }
        }

    }

    // USDBG("---------------\n");
    // USDBG("uso %s appended.\n",uso->name);
    // USDBG("init: %p\n",uso->init);
    // USDBG("Entry: %p\n",uso->entry);
    // if(uso->deps.init.parent)
    // {
    //     USDBG("init deps:%p",uso->deps.init.parent);
    //     USDBG(" %s changed on state:%d\n",uso->deps.init.parent->name,uso->deps.init.state_for_hook);
    // }
    // if(uso->deps.entry.parent)
    // {
    //     USDBG("entry deps:%p",uso->deps.entry.parent);
    //     USDBG(" %s changed on state:%d\n",uso->deps.entry.parent->name,uso->deps.entry.state_for_hook);
    // }

    return 0;
}

static void unified_servcie_spawn(uso_t* uso)
{
    USDBG("---------------\n");
    USDBG("service %s spawn child.\n",uso->name);

    uso_deps_meta_t* meta = NULL;

    if(uso->children)
    {
        USDBG("children:%d\n",uso->children);

        uso_t* child = NULL;

        for(int i=0;i<MAX_DEPS_PER_USO;i++)
        {
            if(!uso->child[i]) continue;

            child = uso->child[i];

            USDBG("child:%s\n",child->name);

            int phase = unified_service_deps_validated(child,uso,&meta);

            ASSERT(meta);

            if(meta->toggle_state == uso->state)
            {
                meta->parent = NULL;
                meta->toggle_state = ussNone;

                if(!phase)
                {
                    child->deps.init_depc--;
                    async_unified_servcie_init(child);
                }
                else
                {
                    child->deps.entry_depc--;
                    async_unified_servcie_entry(child);
                }

                uso->child[i] = NULL;
                uso->children--;
            }

        }
    }

}

static void unified_servcie_recycle(uso_t* uso)
{
    USDBG("Called unimplemented app recycle function.\n");
}

static void unified_servcie_manager(void)
{
    queue_create(&sched_queue,10,sizeof(void*));

    unified_servcie_load();

    for(int i=0;i<MAX_USO_NUM;i++)
    {
        async_unified_servcie_init(uso_sets[i]);
    }

    uso_t* uso = NULL;

    while(1)
    {
        queue_recv(&sched_queue,&uso);

        USDBG("uso %s state:%d\n",uso->name,uso->state);

        unified_servcie_spawn(uso);

        switch(uso->state)
        {
        case ussInit:
            // USDBG("uso %s start init.\n",uso->name);
        break;

        case ussReady:
            async_unified_servcie_entry(uso);
            // USDBG("uso %s init done.\n",uso->name);
        break;

        case ussRun:
            // USDBG("uso %s launch.\n",uso->name);
        break;

        case ussTerminated:
            unified_servcie_recycle(uso);
            // USDBG("uso %s fin.\n",uso->name);
        break;

        default:
            USDBG("Unexpected state %d from uso %s\n",uso->name);
        break;
        }

        uso = NULL;
    }
}

APP_START(USM)
 .entry = (app_entry)unified_servcie_manager,
APP_END