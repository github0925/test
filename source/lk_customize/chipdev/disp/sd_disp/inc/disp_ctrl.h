/*
* disp_ctrl.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/
#ifndef __DISP_CTRL_H__
#define __DISP_CTRL_H__
#include <sdm_panel.h>
#include <kernel/event.h>
#include <string.h>
#include <kernel/semaphore.h>
#include <event.h>
#define MAX_DISP_NUM    5

typedef enum {
    PIPE_TYPE_SPIPE,
    PIPE_TYPE_GPIPE,
    PIPE_TYPE_NUM
} pipe_type_t;

typedef enum {
    MASTER_MODE = 0,
    SLAVE_MODE,
} ms_mode_t;

struct ms_info_t {
    int ms_en;
    int ms_mode;
    int start;
    int end;
};

struct dc_device {
    int display_id;
    int dc_idx;
    struct ms_info_t ms_info;
    int mlc_select;
    unsigned int irq;
    unsigned long  base;
    double dsp_clk;
    double ratio;
    unsigned int aflu_time;
    mutex_t mutex_refresh;
    semaphore_t vsync_sema;
    event_t vsync_kick;
    int last_zorder[PIPE_TYPE_NUM];
    int vsync_wait;
};

struct dc_operations {
    int (*init)(struct dc_device *dev, struct sdm_panel *panel);
    int (*reset)(struct dc_device *dev);
    int (*update)(struct dc_device *dev, struct sdm_post_config *post);
    void (*clear_layers)(struct dc_device *dev, u8 mask, u8 z_order);
    void (*triggle)(struct dc_device *dev);
    unsigned int (*check_triggle_status)(struct dc_device *dev);
    void (*vsync_enable)(struct dc_device *dev, bool enable);
    void (*enable)(struct dc_device *dev);
    enum handler_return (*irq_handler)(void *arg);
    int (*csc_set)(struct dc_device *dev, struct sdm_pq_params *pq);
};
int disp_vsync_callback(int display_id);
#endif //__DISP_CTRL_H__
