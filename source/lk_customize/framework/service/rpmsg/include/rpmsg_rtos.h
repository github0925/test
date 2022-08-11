#ifndef __RPMSG_RTOS_H__
#define __RPMSG_RTOS_H__

#include <dcf.h>
#include <rpmsg_lite.h>
#include <rpmsg_queue.h>

struct rpmsg_channel;
typedef void(*rpmsg_msg_handler)(struct rpmsg_channel *, struct dcf_message *mssg, int len);

struct rpmsg_device {
    rpmsg_dev_config_t config;
    void        *shm_virt_base;
    thread_t    *main_thread;

    struct list_node channels;
    mutex_t lock;

    /* rpmsg lite resources */
    struct rpmsg_lite_instance *rl_instance;
    bool        ready;
};

struct rpmsg_channel {

    char name[DCF_NAM_MAX_LEN];
    int rproc;
    int addr;
    u32 mtu;
    dcf_state_t state;
    event_t initialized;
    thread_t *looper;
    struct list_node node;

    struct rpmsg_device *parent;
    struct rpmsg_lite_instance *rpmsg_dev;
    struct rpmsg_lite_endpoint *endpoint;

    u32         rx_bytes;
    u32         tx_bytes;
    u32         rx_cnt;
    u32         tx_cnt;
    u32         err_cnt;
    u32         drop_cnt;

    rpmsg_queue_handle  msg_queue;
    rpmsg_msg_handler msg_handler;
};

#if defined(CONFIG_RPMSG_SERVICE) && (CONFIG_RPMSG_SERVICE == 1)

void rpmsg_rtos_init(void);
void start_rpmsg_service(void);
void show_rpmsg_service(void);
void rpmsg_device_probe(struct rpmsg_dev_config *cfg);

struct rpmsg_channel *
rpmsg_channel_create(int rproc, int dst, const char *name);
int rpmsg_channel_set_mtu(struct rpmsg_channel *rpchn, unsigned int mtu);
int rpmsg_channel_start(struct rpmsg_channel *rpchn, rpmsg_msg_handler handler);
void rpmsg_channel_destroy(struct rpmsg_channel *rpchn);
status_t rpmsg_channel_stop(struct rpmsg_channel *rpchn);
void rpmsg_channel_listall(struct rpmsg_device *dev);

status_t rpmsg_channel_sendmsg(struct rpmsg_channel *rpchn,
                    struct dcf_message *msg, int len, lk_time_t timeout);

status_t rpmsg_channel_recvmsg(struct rpmsg_channel *rpchn,
                struct dcf_message *msg, int msglen, lk_time_t timeout);

status_t rpmsg_channel_recvfrom(struct rpmsg_channel *rpchn,
                     unsigned long *src,
                     char *data,
                     int maxlen,
                     int *len,
                     unsigned long timeout);

status_t rpmsg_channel_sendto(struct rpmsg_channel *rpchn, unsigned long dst,
                    char *data,
                    unsigned long size,
                    unsigned long timeout);

void *rpmsg_channel_alloc_tx_buf(struct rpmsg_channel *rpchn,
                    unsigned long *size,
                    unsigned long timeout);

status_t rpmsg_channel_sendto_nocopy(struct rpmsg_channel *rpchn, unsigned long dst,
                    char *data,
                    unsigned long size);

#else
inline static void rpmsg_rtos_init(void)
{
}

inline static void start_rpmsg_service(void)
{
}

inline static struct rpmsg_lite_instance *
rpmsg_get_instance_timed(int remote, lk_time_t ms)
{
    return NULL;
}
#endif

#endif //__RPMSG_RTOS_H__
