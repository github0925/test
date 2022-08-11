
#include <event.h>
#include <app.h>
#include <lk_wrapper.h>
#include <kernel/mutex.h>
#include <driver.h>
#include <dcf.h>
#include "surface_info.h"

event_t enqueue_event;
event_t receive_event;
bool recv_display = false;
int surface_status = surface_end;
static struct surface_buff msurface;
static int chan;

void surface_receiver_init()
{
    printf("surface_receive init \n");
    event_init(&enqueue_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&receive_event, false, EVENT_FLAG_AUTOUNSIGNAL);
}

void surface_receiver_send(void) {
    if(chan > 0) {
        static char sendbuf[] = "received";
        int ret = posix_write(chan, (char*)sendbuf, sizeof(sendbuf));
    }
}

void surface_receiver_task(token_handle_t token)
{
    chan = posix_open(DEV_DISP_C, O_RDWR);
    if(chan < 0)
    {
        printf("receive char is null \n");
        return;
    }

    int count = 0;

    while(1)
    {
        int recved = 0;
        recved = posix_read(chan, (char*)&msurface.surface,
                   sizeof(struct surface_info));
        //printf("\n i receive data,its phy_addr is %llx,width is %d,height is %d stride is %d recv %d, size %lld, cmd %d\n",msurface.surface.phy_addr,msurface.surface.width,
          //      msurface.surface.height,msurface.surface.stride, recved, msurface.surface.size, msurface.surface.cmd);

        if (recved <= 0) {
            dprintf(INFO, "\n\nFrom received stop msg\n");
            break;
        }

        event_signal(&receive_event,true);
        event_wait(&enqueue_event);
    }

    posix_close(chan);
}

status_t surface_receiver_dequeue(struct surface_buff ** surface, int timeout)
{
    if (timeout <= 0)
    {
        timeout = INFINITE_TIME;
    }
    status_t ret = event_wait_timeout(&receive_event, timeout);
    //printf("surface_receiver_dequeue %d ms\n",current_time());
    *surface = &msurface;
    return ret;
}

void surface_receiver_enqueue(void)
{
    //printf("surface_receiver_enqueue %d ms\n",current_time());
    surface_receiver_send();
    event_signal(&enqueue_event,true);
}
