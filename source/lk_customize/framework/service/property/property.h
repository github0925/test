/*
 * property.h
 *
 * Copyright (c) 2019 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * property library heafile
 *
 * Revision History:
 * -----------------
 */

#ifndef _SYS_PROPERTY_H_
#define _SYS_PROPERTY_H_

#include <kernel/wait.h>
#include <kernel/event.h>

/* property flags
 * INITD: inited for local property
 * LOCAL: maintained in this domain, orelse in other domain specified by rproc
 * RO:    readonly once initialized for first time
 * WO:    writeonly
 */
#define SYS_PROP_F_INITD           (1)
#define SYS_PROP_F_WR              (2)
#define SYS_PROP_F_RO              (4)
#define SYS_PROP_F_WO              (8)

typedef void (*system_on_changed_cb)(int, int, int, void*);

/*
 * Here defines property area
 * TODO: use string based dictionary
 */
struct sys_property_value {
    int val;
    int id;
    int flags;
    int owner;

    system_on_changed_cb callback;
    void *usr_data;

    event_t event;
    int expect_val;
    int status;
};

int system_property_set(int property_id, int val);
int system_property_get(int property_id, int *val);
int system_property_wait_condition(int property_id, int expect);
int system_property_wait_timeout(int property_id, int expect, lk_time_t ms);

int system_property_observe(int property_id, system_on_changed_cb cb, void *data);
bool system_property_is_local(int property_id);
int start_property_service(struct sys_property_value *property_table, int num);

#endif //_SYS_PROPERTY_H_
