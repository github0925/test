#include <lk_wrapper.h>
#include <macros.h>
#include <string.h>
#include <assert.h>
#include "can_proxy_buffer.h"

struct cp_msg_buf_t;

typedef void*  (*cp_msg_buf_create_t)(size_t size);
typedef size_t (*cp_msg_buf_post_t)(void* handle,void* data, size_t len);
typedef size_t (*cp_msg_buf_get_t)(void* handle,void* data, size_t len);


typedef struct cp_msg_buf_t
{
    void* handle;
    size_t size;
    cp_msg_buf_create_t create;
    cp_msg_buf_post_t post;
    cp_msg_buf_post_t get;
}cp_msg_buf_t;


size_t os_msg_buf_post(void* handle, void* data, size_t len);
size_t os_msg_buf_get(void* handle, void* data, size_t len);
void* os_msg_buf_create(size_t buffer_size);


static cp_msg_buf_t interface =
{
    .handle = NULL,
    .size = CP_BUFFER_SIZE,
    .create = os_msg_buf_create,
    .post = os_msg_buf_post,
    .get = os_msg_buf_get,
};

bool can_proxy_buffer_init(void)
{
    interface.handle = interface.create(interface.size);

    if(interface.handle)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* interface frame interface post */
bool cp_if_frame_post(cp_if_frame_t* frame)
{
    return interface.post(interface.handle,frame,cp_if_get_frame_size(frame)) ? true : false;
}

/* interface frame interface get */
size_t cp_if_frame_get(cp_if_frame_t* frame, size_t max_data_len)
{
    if(!interface.get(interface.handle,frame,max_data_len+sizeof(cp_if_frame_t)))
    {
        return 0;
    }
    else
    {
        return frame->len;
    }
}
