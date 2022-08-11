#ifndef _BA_VSTREAMER_H_
#define _BA_VSTREAMER_H_
#include <kernel/mutex.h>
#include <kernel/event.h>
#include <stdint.h>
#include <queue.h>
#include <sys/types.h>

struct vstreamer_t;

typedef int (*vs_push_t)(struct vstreamer_t* stream,void* item);
typedef int (*vs_pop_t)(struct vstreamer_t* stream,void* item);
typedef void (*vs_proc_t)(void* prod_rsp, void* cons_req);

typedef struct vstreamer_t
{
    QueueHandle_t in;
    QueueHandle_t out;
    uint32_t item_size_in;
    uint32_t item_num_in;
    vs_proc_t proc;
    void* proc_buf;
    vs_push_t push;
    vs_pop_t pop;
    event_t *evt_done; // add to replace timer, using csi
}vstreamer_t;

vstreamer_t* vstream_create(uint32_t item_size_in,uint32_t item_num_in);
void vstream_link(vstreamer_t* producer,vstreamer_t* consumer,vs_proc_t proc, event_t *evt);
void vstream_destroy(vstreamer_t* stream);


#endif
