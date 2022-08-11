#include <lk_wrapper.h>
#include "vstreamer.h"

static inline int vstream_push(struct vstreamer_t* stream,void* item);
static inline int vstream_pop(struct vstreamer_t* stream,void* item);


vstreamer_t* vstream_create(uint32_t item_size_in,uint32_t item_num_in)
{
    vstreamer_t* streamer = malloc(sizeof(vstreamer_t));
    ASSERT(streamer);
    streamer->proc = NULL;
    streamer->item_num_in = item_num_in;
    streamer->item_size_in = item_size_in;

    if(item_num_in == 0 || item_size_in == 0)
    {
        streamer->in = NULL;
    }
    else
    {
        streamer->in = xQueueCreate(streamer->item_num_in, streamer->item_size_in);
        ASSERT(streamer->in);
    }
    streamer->out = NULL;
    streamer->push = vstream_push;
    streamer->pop = vstream_pop;

    return streamer;

}

static inline int vstream_push(struct vstreamer_t* stream,void* item)
{
    if(stream->out)
    {
        if(stream->proc)
        {
            stream->proc(item,stream->proc_buf);
            xQueueSend(stream->out,stream->proc_buf,portMAX_DELAY);
        }
        else
        {
            xQueueSend(stream->out,item,portMAX_DELAY);
        }

    }

    return 0;
}

static inline int vstream_pop(struct vstreamer_t* stream,void* item)
{
    if(stream->in)
    {
        xQueueReceive(stream->in,item,portMAX_DELAY);
    }

    return 0;
}

void vstream_link(vstreamer_t* producer,vstreamer_t* consumer,vs_proc_t proc, event_t *evt)
{
    ASSERT(producer && consumer);
    producer->out = consumer->in;
    if (evt) {
        producer->evt_done = evt;
        consumer->evt_done = evt;
    }

    if(proc)
    {
        producer->proc_buf = malloc(consumer->item_size_in);
        ASSERT(producer->proc_buf);
        producer->proc = proc;
    }
    else
    {
        producer->proc = NULL;
    }

}

void vstream_destroy(vstreamer_t* stream)
{
    if(stream->in)
    {
        vQueueDelete(stream->in);
    }
    free(stream->proc_buf);
    free(stream);
}
