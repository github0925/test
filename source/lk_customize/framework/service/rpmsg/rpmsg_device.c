/*
 * Copyright (c) 2020  Semidrive
 *
 */

#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <debug.h>
#include <platform.h>
#include <platform/debug.h>
#include <kernel/semaphore.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include <lk/init.h>
#include <dcf.h>
#include <sys_cnt.h>

#if defined(CONFIG_RPMSG_SERVICE) && (CONFIG_RPMSG_SERVICE == 1)
#include <rpmsg_ns.h>

#define MAX_REMOTE_DEVICE_NUM   (6)
static struct rpmsg_device rpmsg_remote_devices[MAX_REMOTE_DEVICE_NUM];

/* init once mutex */
static mutex_t rpmsg_rtos_mutex;
static bool once_inited = false;

inline static struct rpmsg_device *alloc_rpmsg_device(struct rpmsg_dev_config *cfg)
{
    int i;
    struct rpmsg_device *rdev = NULL;

    if (!cfg || cfg->shm_phys_base == 0)
        return NULL;

    for (i = 0;i < MAX_REMOTE_DEVICE_NUM;i++) {
        rdev = &rpmsg_remote_devices[i];
        if (rdev->config.shm_phys_base == 0) {
            memcpy(&rdev->config, cfg, sizeof(rpmsg_dev_config_t));
            return rdev;
        }
    }

    return NULL;
}

inline static void free_rpmsg_device(struct rpmsg_device *rdev)
{
    if (rdev->config.shm_phys_base != 0) {
        memset(&rdev->config, 0, sizeof(rpmsg_dev_config_t));
    }
}

inline static struct rpmsg_device *find_rpmsg_device_by_remote(int remote)
{
    int i;
    struct rpmsg_device *rdev = NULL;

    for (i = 0;i < MAX_REMOTE_DEVICE_NUM;i++) {
        rdev = &rpmsg_remote_devices[i];
        if (rdev->config.shm_phys_base && (remote == rdev->config.remote_proc)) {
            return rdev;
        }
    }

    return NULL;
}

static void rpmsg_new_ept_cb(unsigned int new_ept,
                                   const char *new_ept_name,
                                   unsigned long flags,
                                   void *user_data)
{
#if LK_DEBUGLEVEL > 0
    const char * op_name = (flags == RL_NS_CREATE) ? "create": "destroy";

    dprintf(INFO, "%s: src: %d name: %s %s\n", __func__, new_ept, new_ept_name, op_name);
#endif
}

int rpmsg_lite_kick_remote(struct rpmsg_lite_instance *rpmsg_lite_dev);
int rpmsg_echo_loop(struct rpmsg_device *rdev);

int rpmsg_device_main(void *data)
{
    struct rpmsg_device *rdev = data;
    int myproc = dcf_get_this_proc();
    int count;

    rdev->shm_virt_base = (void*) p2v(rdev->config.shm_phys_base);

    dprintf(INFO, "Starting rpmsg %s virtio%d->%d @%p %p\n",
            rdev->config.is_master ? "master": "slave", myproc,
            rdev->config.remote_proc, rdev->config.shm_phys_base, rdev->shm_virt_base);

    mutex_init(&rdev->lock);
    if (rdev->config.is_master) {
        rdev->rl_instance = rpmsg_lite_master_init(rdev->shm_virt_base, rdev->config.shm_size,
                        rdev->config.remote_proc, rdev->config.ext.init_flags);

        rpmsg_ns_bind(rdev->rl_instance, rpmsg_new_ept_cb, rdev->rl_instance);

    } else {
        rdev->rl_instance = rpmsg_lite_remote_init(rdev->shm_virt_base,
                        rdev->config.remote_proc, rdev->config.ext.init_flags);
    }

    dprintf(2, "Waiting for RP%d to get ready...\n", rdev->config.remote_proc);

    /* only remote device wait link up */
    if (!rdev->config.is_master) {
        /* wait for 5 sec in startup, then quit loop */
        count = 20;
        while(!rpmsg_lite_is_link_up(rdev->rl_instance))
        {
            count--;
            if (!count) {
                dprintf(ALWAYS, "rpmsg virtio%d->%d timeout, try later!\n",
                                    myproc, rdev->config.remote_proc);
                goto exit_loop1;
            }

            rpmsg_lite_kick_remote(rdev->rl_instance);
            env_sleep_msec(200);
        }
    }

    dprintf(1, "link %d->%d is up\n", myproc, rdev->config.remote_proc);
    rdev->ready = true;

#if CONFIG_RPMSG_TTY
    rpmsg_tty_probe(rdev);
#endif

#if CONFIG_RPMSG_ECHO
    /* rpmsg echo service */
    rpmsg_echo_loop(rdev);
#endif

exit_loop1:

    rpmsg_lite_deinit(rdev->rl_instance);
    rdev->main_thread = NULL;
    free_rpmsg_device(rdev);

    return 0;
}

void rpmsg_device_probe(struct rpmsg_dev_config *cfg)
{
    struct rpmsg_device *rdev;
    char thread_name[16];

    rdev = find_rpmsg_device_by_remote(cfg->remote_proc);
    if (rdev && rdev->main_thread) {
        dprintf(INFO, "rpmsg virtio%d->%d already started \n",
                dcf_get_this_proc(),rdev->config.remote_proc);
        return;
    }

    if (cfg->is_master < 0) {
        dprintf(ALWAYS, "rpmsg mem(%x %x) defined but disabled in dcf_config\n",
                cfg->shm_phys_base, cfg->shm_size);
        return;
    }

    rdev = alloc_rpmsg_device(cfg);
    if (!rdev) {
        dprintf(ALWAYS, "failed to allocate rpmsg device to rproc %d \n", cfg->remote_proc);
        return;
    }

    list_initialize(&rdev->channels);

    sprintf(thread_name, "rpmsg-echod/%d", cfg->remote_proc);
    rdev->main_thread = thread_create(thread_name, rpmsg_device_main, rdev,
                                  THREAD_PRI_RPMSGECHO, CONFIG_ECHO_STACK_SIZE);
    thread_resume(rdev->main_thread);
}

#define RPMSG_GET_RETRY_SPIN    (100)
struct rpmsg_device *rpmsg_device_get_handle(int remote, lk_time_t ms)
{
    struct rpmsg_device *rdev = find_rpmsg_device_by_remote(remote);
    if (!rdev) {
        dprintf(INFO, "rpmsg device to rproc%d not found \n", remote);
        return NULL;
    }

    do {
        if (ms < RPMSG_GET_RETRY_SPIN) {
            return NULL;
        }
        dprintf(SPEW, "rpmsg virtio link %d->%d is not ready, wait.. \n",
                            dcf_get_this_proc(),rdev->config.remote_proc);
        thread_sleep(RPMSG_GET_RETRY_SPIN);
        ms -= RPMSG_GET_RETRY_SPIN;
    } while (!rdev->ready);

    return rdev;
}

int rpmsg_device_add_channel(struct rpmsg_device *dev, struct rpmsg_channel *rpchn)
{
    mutex_acquire(&dev->lock);
    list_add_tail(&dev->channels, &rpchn->node);
    mutex_release(&dev->lock);
    return 0;
}

int rpmsg_device_rm_channel(struct rpmsg_device *dev, struct rpmsg_channel *rpchn)
{
    mutex_acquire(&dev->lock);
    list_delete(&rpchn->node);
    mutex_release(&dev->lock);
    return 0;
}

struct rpmsg_lite_instance *rpmsg_get_instance(int remote)
{
    struct rpmsg_device *rdev = rpmsg_device_get_handle(remote, 0xFFFFFFFF);

    if (rdev) {
        return rdev->rl_instance;
    }
    return NULL;
}

struct rpmsg_lite_instance *rpmsg_get_instance_timed(int remote, lk_time_t ms)
{
    struct rpmsg_device *rdev = rpmsg_device_get_handle(remote, ms);

    if (rdev) {
        return rdev->rl_instance;
    }
    return NULL;
}

/*
 * This funcion is done in dcf_init quickly.
 */
void rpmsg_rtos_init(void)
{
    mutex_init(&rpmsg_rtos_mutex);
    env_init();
}

/*
 * This funcion can be called many times by different modules,
 * but real initilizaion is done once, add mutex to avoid race condition.
 */
void start_rpmsg_service(void)
{
    int i, dev_num;
    struct rpmsg_dev_config *cfg;
    struct rpmsg_device *rdev;

    mutex_acquire(&rpmsg_rtos_mutex);
    if (!once_inited) {
        dev_num = platform_get_rpmsg_config(&cfg);
        for (i = 0;i < dev_num; i++) {
            rpmsg_device_probe(&cfg[i]);
        }
        once_inited = true;
    }
    mutex_release(&rpmsg_rtos_mutex);
}

void reset_rpmsg_service(void)
{
    int i, dev_num;
    struct rpmsg_dev_config *cfg;
    struct rpmsg_device *rdev;

    mutex_acquire(&rpmsg_rtos_mutex);
    dev_num = platform_get_rpmsg_config(&cfg);
    for (i = 0;i < dev_num; i++) {
        rpmsg_device_probe(&cfg[i]);
    }
    mutex_release(&rpmsg_rtos_mutex);
}

void show_rpmsg_service(void)
{
    int i;
    struct rpmsg_device *rdev;

    printf("rpmsg devices:\n");
    printf("\tPhaddr\t\tSize\t\tRproc\tMaster\tSpaceX\n");
    for (i = 0;i < MAX_REMOTE_DEVICE_NUM;i++) {
        rdev = &rpmsg_remote_devices[i];
        if (rdev->config.shm_phys_base) {


            printf("%d:\t0x%x\t0x%x\t%d\t%d\t%d\n", i, rdev->config.shm_phys_base, rdev->config.shm_size,
                rdev->config.remote_proc, rdev->config.is_master, rdev->config.pa_spacex);

            rpmsg_channel_listall(rdev);
            printf("\n\n");
        }
    }
    printf("End of rpmsg devices\n");
}

void rpmsg_service_entry(uint level)
{
    uint32_t t1, t2;

    t1 = syscnt_get_cnt();
    start_rpmsg_service();
    t2 = syscnt_get_cnt();

#if !SUPPORT_FAST_BOOT
    dprintf(0, "%s takes %d us\n", __func__, syscnt_time_lapse(t1, t2));
#else
    t1 = t2 - t1;   /* pass compiling */
#endif
}

/* Make sure the rpmsg init later than dcf init */
#define RPMSG_INIT_LEVEL            (LK_INIT_LEVEL_PLATFORM + 2)

/* uncomment this for auto boot service */
LK_INIT_HOOK(rpmsg_entry, rpmsg_service_entry, RPMSG_INIT_LEVEL)

#endif

