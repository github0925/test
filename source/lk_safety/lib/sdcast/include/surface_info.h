#ifndef __SURFFACE_INFO_H__
#define __SURFFACE_INFO_H__

#include <app.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <thread.h>
#include <container.h>

#include <app.h>
#include <lib/console.h>
#include <chip_res.h>
#include <task.h>
#include <queue.h>
#include <heap.h>

struct rect_info {
    int left;
    int top;
    int right;
    int bottom;
} __attribute__((__packed__));

struct surface_info {
    uint64_t phy_addr;
    int width;
    int height;
    int stride;
    uint64_t size;
    struct rect_info source;
    struct rect_info display;
    int format;
    int prime_fd;
    int cmd;
} __attribute__((__packed__));

struct surface_buff {
    uint16_t id;
    int status;
    struct surface_info surface;
};

enum buffer_status {
    buffer_status_idle = 0,
    buffer_status_ready,
};

enum surface_status {
    surface_info = 0,
    surface_start = 1,
    surface_end = 2,
};

void surface_receiver_task(token_handle_t token);
void surface_receiver_init(void);
void surface_receiver_enqueue(void);
status_t surface_receiver_dequeue(struct surface_buff ** surface, int timeout);
int surface_receiver_get_status(void);
bool surface_receiver_is_start(void);
#endif
