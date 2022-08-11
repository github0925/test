/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: domain communication high level api for secure
*
*/
#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <platform/debug.h>
#include "dcf.h"

enum dcf_file_state {
    DCF_FS_NOT_INITED,
    DCF_FS_OPENED,
    DCF_FS_CLOSED,
};

#if defined(CONFIG_SUPPORT_POSIX) && (CONFIG_SUPPORT_POSIX == 1)
static int ext_fd_start = DCF_MAX_FILES;
static struct dcf_file *dynamic_files;
extern struct dcf_file dcf_files[DCF_MAX_FILES];

static int find_dcf_fd(const char *name)
{
    int i;

    for (i = 0; i < DCF_MAX_FILES; i++) {
        if (0 == strncmp(dcf_files[i].file_name, name, DCF_NAM_MAX_LEN)) {
            return i;
        }
    }

    return -1;
}

static struct dcf_file *find_dcf_file(const char *name)
{
    int i;

    for (i = 0; i < DCF_MAX_FILES; i++) {
        if (0 == strncmp(dcf_files[i].file_name, name, DCF_NAM_MAX_LEN)) {
            return &dcf_files[i];
        }
    }

    return NULL;
}

struct dcf_file *dcf_get_file(int fd)
{
    if (fd >= DCF_MAX_FILES && !dynamic_files)
        return NULL;

    /* check static files */
    if (fd < DCF_MAX_FILES) {
        if (strlen(dcf_files[fd].file_name))
            return &dcf_files[fd];
        else
            return NULL;
    }

    /* TODO: dynamic files */
    ext_fd_start++;

    return NULL;
}

void dcf_file_msg_cb(struct ipcc_channel *rpchan, struct dcf_message *mssg, int len)
{

}

int dcf_file_poll(struct pollfd *pf)
{
    struct dcf_file *file;
    int mask = 0;
    int flags = 0;

    file = dcf_get_file(pf->fd);
    if (!file) {
        dprintf(0, "invalid fd %d\n", pf->fd);
        return -EBADF;
    }

    if (file->f_state != DCF_FS_OPENED)
        return -ENOENT;

    flags = file->f_flags & O_ACCMODE;
    if (flags == O_RDONLY || flags == O_RDWR) {
        /* Always writeable */
        if (pf->events & POLLOUT)
            mask |= POLLOUT;

        if (pf->events & POLLIN) {
            if (ipcc_channel_inq_avail(file->rpchan) > 0) {
                mask |= POLLIN;
            }
        }
    }

    return mask;
}

void dcf_file_event_cb(void *ctx, ipcc_event_t e, u32 data)
{
    struct dcf_file *file;
    int fd = (int)ctx;

    file = dcf_get_file(fd);
    if (!file) {
        dprintf(0, "This is NULL file\n");
        return;
    }

    switch(e) {
    case IPCC_EVENT_RECVED:
        file->rcvevent++;
        dcf_handle_pollevent(fd);
        break;
    default:
        break;
    }
}

int dcf_open(const char *name, unsigned int flags)
{
    struct dcf_file *file;
    int fd;

    fd = find_dcf_fd(name);
    file = dcf_get_file(fd);
    if (!file) {
        dprintf(0, "file %s not found\n", name);
        return -ENOENT;
    }

    if (file->rpchan) {
        file->refcount++;
        dprintf(0, "file %s already opened\n", name);
        return fd;
    }

    file->ipdev = ipcc_device_gethandle(file->remote_processor, 5000);
    if (!file->ipdev)
        return -ENODEV;

    file->rpchan = ipcc_channel_create(file->ipdev, file->rpmsg_endpoint, file->ns_name, true);
    if (!file->rpchan) {
        dprintf(0, "%s already opened\n", name);
        return -EIO;
    }

    ipcc_channel_register_event_cb(file->rpchan, dcf_file_event_cb, (void*)fd);

    ipcc_channel_start(file->rpchan, NULL);
    file->f_state = DCF_FS_OPENED;
    file->f_flags = flags;
    file->refcount++;

    return fd;
}

int dcf_close(int fd)
{
    struct dcf_file *file;

    file = dcf_get_file(fd);
    if (!file) {
        dprintf(0, "fd=%d not exist\n", fd);
        return -EBADF;
    }

    if (file->f_state != DCF_FS_OPENED) {
        dprintf(0, "fd=%d is not open\n", fd);
        return -ENOENT;
    }

    if (--file->refcount)
        return 0;

    /* TODO: check any read/write activity on this */
    if (file->rpchan) {
        ipcc_channel_stop(file->rpchan);
        ipcc_channel_destroy(file->rpchan);
        file->rpchan = NULL;
        file->ipdev = NULL;
    }
    file->f_state = DCF_FS_CLOSED;
    file->f_flags = 0;

    return 0;
}

int dcf_read(int fd, void *mem, size_t len)
{
    struct dcf_file *file;
#if CONFIG_HAS_DCF_MSGHDR
    char msg_buf[DCF_MSG_MAX_LEN];
    struct dcf_message *msg = (struct dcf_message *)msg_buf;
#else
    char *msg = (void *)mem;
#endif
    int size = len;
    int block = DCF_BLOCK;
    int ret = 0;

    if (len > DCF_MSG_MAX_DLEN) {
        dprintf(0, "fd=%d len is too big\n", fd);
        return -EFBIG;
    }

    file = dcf_get_file(fd);
    if (!file) {
        dprintf(0, "fd=%d not exist\n", fd);
        return -EBADF;
    }

    ASSERT(file->f_state == DCF_FS_OPENED);

    if (file->f_flags & O_NONBLOCK)
        block = 0;

    ret = ipcc_channel_recvfrom(file->rpchan, NULL, msg, &size, block);
    if (ret == 0) {
#if CONFIG_HAS_DCF_MSGHDR
        size -= DCF_MSG_HLEN;
        memcpy(mem, msg->data, size);
#endif
        return size;
    }

    return ret;
}

int dcf_write(int fd, void *mem, size_t len)
{
    struct dcf_file *file;
#if CONFIG_HAS_DCF_MSGHDR
    char msg_buf[DCF_MSG_MAX_LEN];
    struct dcf_message *msg = (struct dcf_message *)msg_buf;
#else
    char *msg = (void *)mem;
#endif
    int size = len;
    int block = DCF_BLOCK;
    int ret = 0;

    if (len > DCF_MSG_MAX_DLEN) {
        dprintf(0, "fd=%d len is too big\n", fd);
        return -EFBIG;
    }

    file = dcf_get_file(fd);
    if (!file) {
        dprintf(0, "fd=%d not exist\n", fd);
        return -EBADF;
    }

    if (file->f_state != DCF_FS_OPENED)
        return -ENOENT;

#if CONFIG_HAS_DCF_MSGHDR
    DCF_MSG_INIT_HDR(msg, COMM_MSG_CORE, len, DCF_MSGF_STD);
    memcpy(msg->data, mem, len);
    size += DCF_MSG_HLEN;
#endif

    if (file->f_flags & O_NONBLOCK)
        block = 0;

    ret = ipcc_channel_sendto(file->rpchan, file->rpmsg_endpoint, msg, size, block);

    return (ret == 0) ? (int)len : ret;
}

int dcf_fcntl(int fd, int cmd, int val)
{
    return 0;
}

void dcf_select_init(void);

int dcf_file_init(void)
{
    dcf_select_init();

    return 0;
}

void dcf_file_list(int argc)
{
    int i;

    for (i = 0; i < DCF_MAX_FILES; i++) {
        if (strlen(dcf_files[i].file_name)) {
            printf("%s\n", dcf_files[i].file_name);
        }
    }
}

#endif //if CONFIG_SUPPORT_POSIX

