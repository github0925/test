/*
 * property.c
 *
 * Copyright (c) 2019 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * property library
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <platform.h>
#include <dcf.h>
#include "property.h"

#define CONFIG_HAS_PROPERTY_THREAD  (0)

#if CONFIG_USE_SYS_PROPERTY

#define PROPERTY_WAIT_DEFAULT_TIME (5000)
#define PROPERTY_IN_CALLBACK_BIT        (0x1000)
#define PROPERTY_WAIT_STATUS_BIT        (0x8000)
#define VALID_PROPERTY_ID(id)   \
    ((property_id) >= 0 && (property_id < properties_number))

static struct sys_property_value ss_properties[] = {
    {.val = 0, .id = 0, .flags = SYS_PROP_F_RO,    .owner = 0},
    {.val = 0, .id = 1, .flags = SYS_PROP_F_RO,    .owner = 0},
    {.val = 0, .id = 2, .flags = SYS_PROP_F_RO,    .owner = 0},
    {.val = 0, .id = 3, .flags = SYS_PROP_F_RO,    .owner = 0},
};

static struct sys_property_value *_properties = &ss_properties[0];
static int properties_number;
static mutex_t property_lock;
#if CONFIG_SUPPORT_POSIX
static int property_fd;
#endif
#if CONFIG_HAS_PROPERTY_THREAD
static int property_thread;
#endif

static int property_get_remote(int property_id, int remote, int *val)
{
    rpc_call_request_t request;
    rpc_call_result_t result = {0,};
    int ret = 0;

    DCF_INIT_RPC_REQ1(request, SYS_RPC_REQ_GET_PROPERTY, property_id);
    ret = dcf_call(remote, &request, &result, 100);
    if ((ret < 0) || (0 != result.retcode)) {
        dprintf(0, "[%lld] %s failed (%d) ret=%d %d\n", current_time_hires(),
                __func__, property_id, ret, result.retcode);
        return ret && result.retcode;
    }

    *val = result.result[0];

    return 0;
}

static int property_set_remote(int property_id, int remote, int val)
{
    rpc_call_request_t request;
    rpc_call_result_t result = {0,};
    int ret = 0;

    DCF_INIT_RPC_REQ4(request, SYS_RPC_REQ_SET_PROPERTY, property_id, val, 0, 0);
    ret = dcf_call(remote, &request, &result, 100);
    if ((ret < 0) || (0 != result.retcode)) {
        dprintf(0, "failed to set remote property (%d)\n", property_id);
        return (ret < 0) ? ret : (int)result.retcode;
    }

    return 0;
}

bool system_property_is_local(int property_id)
{
    return (_properties[property_id].owner ==  dcf_get_this_proc());
}

inline static
bool property_is_ro_inited(struct sys_property_value *property)
{
    int flags = property->flags & (SYS_PROP_F_INITD|SYS_PROP_F_RO);
    return (flags == (SYS_PROP_F_INITD|SYS_PROP_F_RO) );
}

int system_property_observe(int property_id, system_on_changed_cb cb, void *data)
{
    struct sys_property_value *property = NULL;
    int ret = 0;

    mutex_acquire(&property_lock);

    if (VALID_PROPERTY_ID(property_id)) {
        property = &_properties[property_id];
        property->callback = cb;
        property->usr_data = data;
        dprintf(1, "observing property(%d)\n", property_id);
    } else {
        ret = ERR_NOT_FOUND;
    }

    if (ret < 0) {
        dprintf(0, "%s property(%d) for reason: %d\n", __func__, property_id, ret);
    }
    mutex_release(&property_lock);

    return ret;
}

static void on_property_changed_locked(int property_id, int old, int val)
{
    struct sys_property_value *property = NULL;

    property = &_properties[property_id];
    if ((old != val) && property->callback) {
        /* avoid nested callback and reenter */
        if ((property->status & PROPERTY_IN_CALLBACK_BIT) == 0) {
            property->status |= PROPERTY_IN_CALLBACK_BIT;
            mutex_release(&property_lock);
            property->callback(property_id, old, val, property->usr_data);
            mutex_acquire(&property_lock);
            property->status &= ~PROPERTY_IN_CALLBACK_BIT;
        } else {
            dprintf(0, "callback reenter would block pid=%d\n", property_id);
        }
    }
    if ((property->status & PROPERTY_WAIT_STATUS_BIT) &&
        (property->expect_val == val)) {

        event_signal(&property->event, false);
    }
}

int system_property_set(int property_id, int val)
{
    struct sys_property_value *property = NULL;
    int ret = 0;
    int old;

    mutex_acquire(&property_lock);

    if (VALID_PROPERTY_ID(property_id)) {
        property = &_properties[property_id];
        /* readonly val can be set only once */
        if (property_is_ro_inited(property)) {
            ret = ERR_NOT_ALLOWED;
            goto set_property_done;
        }

        if (property->owner ==  dcf_get_this_proc()) {
            if (!(property->flags & SYS_PROP_F_INITD))
                property->flags |= SYS_PROP_F_INITD;

            old = property->val;
            property->val = val;

            on_property_changed_locked(property_id, old, val);

        } else {
            /* it's managed by remote domain, to request RPC */
            ret = property_set_remote(property_id, property->owner, val);
        }
    } else {
        ret = ERR_NOT_FOUND;
    }

set_property_done:
    mutex_release(&property_lock);

    if (ret < 0) {
        dprintf(0, "fail to set property(%d) for reason: %d\n", property_id, ret);
    }

    return ret;
}

int system_property_get(int property_id, int *val)
{
    struct sys_property_value *property = NULL;
    int ret = 0;

    if (!val)
        return ERR_INVALID_ARGS;

    mutex_acquire(&property_lock);

    if (VALID_PROPERTY_ID(property_id)) {
        property = &_properties[property_id];
        if (property->owner ==  dcf_get_this_proc()) {
            *val = property->val;
        } else {
            /* it's managed by remote domain, to request by RPC */
            ret = property_get_remote(property_id, property->owner, val);
        }
    } else {
        ret = ERR_NOT_FOUND;
    }

    mutex_release(&property_lock);

    if (ret < 0) {
        dprintf(0, "fail to get property(%d) for reason: %d\n", property_id, ret);
    }

    return ret;
}

static int local_property_wait_timeout(struct sys_property_value *property, int expect, lk_time_t ms)
{
    int ret;

    mutex_acquire(&property_lock);
    event_init(&property->event, 0, EVENT_FLAG_AUTOUNSIGNAL);
    property->status |= PROPERTY_WAIT_STATUS_BIT;
    property->expect_val = expect;
    mutex_release(&property_lock);

    ret = event_wait_timeout(&property->event, ms);
    if (ret < 0) {
        dprintf(0, "wait property(%d) @condition(%d) timeout \n", property->id, expect);
    }

    mutex_acquire(&property_lock);
    property->status &= ~PROPERTY_WAIT_STATUS_BIT;
    event_destroy(&property->event);
    mutex_release(&property_lock);

    return ret;
}

static int remote_property_poll(struct sys_property_value *property, int expect, lk_time_t timeout)
{
    int ret;
    int val;
    lk_time_t complete;

    complete = current_time() + timeout;
    while(1) {
        ret = property_get_remote(property->id, property->owner, &val);
        if (!ret && (val == expect))
            return 0;

        thread_sleep(100);

        if (current_time() >= complete)
            return ERR_TIMED_OUT;
    }

}

int system_property_wait_timeout(int property_id, int expect, lk_time_t ms)
{
    struct sys_property_value *property = NULL;
    int ret = 0;
    int val = 0;

    if (!VALID_PROPERTY_ID(property_id)) {
        dprintf(0, "wait property with bad id (%d)\n", property_id);
        return ERR_NOT_FOUND;
    }
    dprintf(INFO, "[%lld] %s: enter\n", current_time_hires(), __func__);

    ret = system_property_get(property_id, &val);
    if (!ret && (val == expect))
        return NO_ERROR;

    /* no wait, return immediately */
    if (ms == 0)
        return ERR_ALREADY_EXPIRED;

    /* wait on condition */
    property = &_properties[property_id];

    if (property->owner ==  dcf_get_this_proc()) {
        ret = local_property_wait_timeout(property, expect, ms);
    } else {
        ret = remote_property_poll(property, expect, ms);
    }

    dprintf(0, "[%lld] %s: done, ret: %d\n", current_time_hires(), __func__, ret);

    return ret;
}

int system_property_wait_condition(int property_id, int expect)
{
    return system_property_wait_timeout(property_id, expect, PROPERTY_WAIT_DEFAULT_TIME);
}

static rpc_call_result_t rpc_get_property_cb(rpc_server_handle_t hserver, rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    int status = 0;
    int property_id = -1;

    result.ack = request->cmd;

    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        return result;
    }

    property_id = (int) request->param[0];
    if (!VALID_PROPERTY_ID(property_id)) {
        result.retcode = ERR_NOT_VALID;
        return result;
    }

    result.retcode = system_property_get(property_id, &status);
    result.result[0] = status;

    return result;
}

static rpc_call_result_t rpc_set_property_cb(rpc_server_handle_t hserver, rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    int property_id = -1;
    int val = 0;

    result.ack = request->cmd;
    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        return result;
    }

    property_id = (int) request->param[0];
    val = (int) request->param[1];
    if (!VALID_PROPERTY_ID(property_id)) {
        result.retcode = ERR_NOT_VALID;
        return result;
    }

    /* RPC callback only allowed to set local property */
    if (system_property_is_local(property_id)) {
        result.retcode = system_property_set(property_id, val);
    } else {
        result.retcode = ERR_NOT_FOUND;
        dprintf(0, "setprop: %d not allowed\n", property_id);
    }

    return result;
}

#if CONFIG_HAS_PROPERTY_THREAD
int property_task(void *dev_name)
{
    char send_buf[DCF_MSG_MAX_DLEN] = {0,};
    size_t len = DCF_MSG_MAX_DLEN;
    int ret = 0;
    int fd;


    while (1) {
        ret = posix_read(fd, send_buf, len);
        if (ret <= 0) {
            printf("%s: fd %d read err=%d\n", __func__, fd, ret);
            continue;
        }

        if (strncmp("stop", send_buf, 4) == 0) {
            printf("fd %d receive stop message, quit\n", fd, ret);
            printf("closing fd %d\n", fd);
            break;
        }

        ret = posix_write(fd, send_buf, len);
        if (ret < 0) {
            printf("%s: fd %d write err=%d\n", __func__, fd, ret);
            continue;
        }
    }

    posix_close(fd);
    printf("test exit\n");

    return 0;
}
#endif

int start_property_service(struct sys_property_value *property_table, int num)
{
    static rpc_server_impl_t s_myfuncs[] = {
        {SYS_RPC_REQ_SET_PROPERTY, rpc_set_property_cb, IPCC_RPC_NO_FLAGS},
        {SYS_RPC_REQ_GET_PROPERTY, rpc_get_property_cb, IPCC_RPC_NO_FLAGS},
    };
    struct sys_property_value *property_area = NULL;
    int ret = 0;

    if (_properties && properties_number) {
        dprintf(1, "%s already started\n", __func__);
        return 0;
    }

    mutex_init(&property_lock);

    mutex_acquire(&property_lock);

#if CONFIG_SUPPORT_POSIX
    /* failure is expected for the file not exist */
    property_fd = posix_open(DEV_PROPERTY, O_RDWR);
#endif

    if (property_table && num) {
        property_area = malloc(sizeof(struct sys_property_value) * num);
        if (!property_area) {
            dprintf(0, "not enough memory for properties %d\n", num);
            ret = ERR_NO_MEMORY;
            goto fail_and_free;
        }

        memcpy(property_area, property_table, sizeof(struct sys_property_value) * num);
        _properties = property_area;
        properties_number = num;
    }

    start_ipcc_rpc_service(s_myfuncs, ARRAY_SIZE(s_myfuncs));
#if CONFIG_HAS_PROPERTY_THREAD
    property_thread = thread_create("propertyd", property_task,
                            NULL, THREAD_PRI_PROPERTY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(property_thread);
#endif

    mutex_release(&property_lock);
    dprintf(1, "%s started\n", __func__);

    return 0;

fail_and_free:
#if CONFIG_SUPPORT_POSIX
    if (property_fd > 0)
        posix_close(property_fd);
#endif

    mutex_release(&property_lock);

    return ret;
}

#endif //CONFIG_USE_SYS_PROPERTY

