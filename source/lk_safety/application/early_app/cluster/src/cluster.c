#include <app.h>
#include <err.h>
#include <lk_wrapper.h>
#include <console.h>
#include <thread.h>
#include <event.h>
#include "container.h"
#include <sdrpc.h>
#include <sdm_display.h>
#include <early_app_common.h>
#include <disp_data_type.h>
#include <string.h>
#include <malloc.h>
#include <heap.h>
#include <spi_nor_hal.h>
#include <cluster_draw.h>
#include <cluster_ui_parameter.h>
#include <cluster_ui.h>
#include <dcf_common.h>
#include <surface_info.h>
#include "lv_demo_cluster.h"
#include "lvgl_gui.h"
#include "storage_device.h"
#define FG_COUNT 2
#define BG_COUNT 1
#define IMG_ADDR_ON_NORFLASH (0x03407000)
#define RESOURCE_PATH   "early_app/cluster/cluster.bin"

struct spi_nor_test_cfg
{
    uint32_t id;
    struct spi_nor_cfg config;
};

static struct spi_nor_test_cfg spi_nor_cfg_data =
{
    .id = RES_OSPI_REG_OSPI1,
    .config =
    {
        .cs = SPI_NOR_CS0,
        .bus_clk = SPI_NOR_CLK_50MHZ,
    },
};

bool res_load(const char* res_name,void* to, uint32_t size, uint32_t offset);
void cluster_property_update(bool remote, uint16_t val);

void read_img_from_norflash(uint8_t * buffer, uint32_t addr, uint32_t length)
{
    int ret = 0;
    void *handle;

    dprintf(CRITICAL, " spi_nor creat handle ...\n");
    ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data.id);
    dprintf(CRITICAL, " spi_nor creat handle result: %s\n", ret ? "PASS" : "FAILED");

    if (!ret)
    return;

    ((struct spi_nor_handle *)handle)->config = &spi_nor_cfg_data.config;
    dprintf(CRITICAL, " spi_nor init spi_nor device ...\n");
    ret = hal_spi_nor_init(handle);
    dprintf(CRITICAL, " spi_nor init spi_nor device result: %s\n", ret ? "FAILED" : "PASS");

    if (ret)
    return;

    hal_spi_nor_read((struct spi_nor_handle *)handle, (spi_nor_address_t)addr, buffer, length);
}

static enum handler_return fps_timer_handler(struct timer *t, lk_time_t now, void *arg)
{
    event_t* evt = arg;
    event_signal(evt,true);

    return 0;
}

static int disp_flags = 0;

void cluster_entry(token_handle_t token)
{
#if !ENABLE_LVGL_CLUSTER
    uint32_t as = 0;
    as = current_time();
    printf("Cluster start at %d ms.\n",as);

//replace into pre-init phase.
#if !FUNC_SAFE_CLUSTER
    cluster_property_update(0,1);
#endif


    struct cluster_ui ui;
    struct list_node *head = sdm_get_display_list();

    sdm_display_t *sdm;
    struct sdm_buffer bufs[2];
    struct sdm_post_config post;
    struct sdm_buffer *buf;

    uint8_t * frame_buffer;
    uint8_t * img_source;
    uint8_t * img_source_bg;
    uint8_t * img_source_number;
    uint8_t * img_source_needle;
    uint8_t * img_source_nav;
    uint32_t alpha;
    int i,j;

    memset(&bufs, 0, sizeof(struct sdm_buffer) * 2);
    memset(&post, 0, sizeof(struct sdm_post_config));

    /* allocate bg and fg buffer */
    frame_buffer = memalign(0x1000, CLUSTER_BUF_SIZE * BYTE_PER_PIXEL * (FG_COUNT + BG_COUNT));
    if (frame_buffer == NULL) {
        printf("malloc frame buffer failed\n");
        return;
    }

    /* allocate source image buffer */
    img_source = memalign(0x1000, ROUNDUP(IMG_TOTAL_SIZE,32));
    if (img_source == NULL) {
        printf("malloc image buffer failed\n");
        return;
    }
#if (CLUSTER_NAV_EN==1)
#if !FUNC_SAFE_CLUSTER
    surface_receiver_init();
    token_handle_t surface_recv_token = token_create("surface",NULL,400,NULL,NULL);
    thread_t* surfacethread = thread_create("surface",(thread_start_routine)surface_receiver_task,surface_recv_token,DEFAULT_PRIORITY,DEFAULT_STACK_SIZE);
    thread_detach_and_resume(surfacethread);
#endif
#endif

    /* buffer for each image source */
    img_source_bg = img_source;
    img_source_needle = img_source + IMG_BG_SIZE;
    img_source_number = img_source + IMG_BG_SIZE + IMG_NEEDLE_SIZE;
    img_source_nav = img_source + IMG_BG_SIZE + IMG_NEEDLE_SIZE + IMG_NUMBER_SIZE;

    /* load source image from flash into external memory */
    printf("Read source images from flash\n");
    res_load(RESOURCE_PATH,img_source_bg, ROUNDUP(IMG_TOTAL_SIZE,32),0);
    printf("Read source images from flash - Done\n");

    /* pass buffer address to cluster ui */
    ui.fg[0].buf = (uint32_t *) frame_buffer;
    ui.fg[1].buf = (uint32_t *) frame_buffer + CLUSTER_BUF_SIZE;
    ui.bg[0].buf = (uint32_t *) frame_buffer + CLUSTER_BUF_SIZE * 2;
    ui.bg[1].buf = (uint32_t *) frame_buffer + CLUSTER_BUF_SIZE * 2; //FIXME after getting more memory
    ui.bg[2].buf = (uint32_t *) frame_buffer + CLUSTER_BUF_SIZE * 2; //FIXME after getting more memory
    ui.res.needles_src = img_source_needle;
    ui.res.bg_src = img_source_bg;
    ui.res.numbers_src = img_source_number;
    ui.res.nav_src = img_source_nav;

    /* configure display buffer */
    post.n_bufs = 2;
    post.bufs = &bufs[0];

    /* configure first buffer */
    buf = &post.bufs[0];
    buf->alpha = 0xff;
    buf->alpha_en = 1;
    buf->ckey = 0;
    buf->ckey_en = 0;
    buf->fmt = COLOR_ARGB8888;
    buf->layer_en = 1;
    buf->src_stride[0] = CLUSTER_WIDTH*4;
    buf->src.x = 0;
    buf->src.y = 0;
    buf->src.w = CLUSTER_WIDTH;
    buf->src.h = CLUSTER_HEIGHT;
    buf->start.x = 0;
    buf->start.y = 0;
    buf->start.w = CLUSTER_WIDTH;
    buf->start.h = CLUSTER_HEIGHT;
    buf->dst.x = 0;
    buf->dst.y = 0;
    buf->dst.w = CLUSTER_WIDTH;
    buf->dst.h = CLUSTER_HEIGHT;
    buf->z_order = 1;
    buf->layer = 1;

    /* configure second buffer */
    buf = &post.bufs[1];
    buf->alpha = 0xff;
    buf->alpha_en = 1;
    buf->ckey = 0;
    buf->ckey_en = 0;
    buf->fmt = COLOR_ABGR8888;
    buf->layer_en = 1;
    buf->src_stride[0] = CLUSTER_WIDTH*4;
    buf->src.x = 0;
    buf->src.y = 0;
    buf->src.w = CLUSTER_WIDTH;
    buf->src.h = CLUSTER_HEIGHT;
    buf->start.x = 0;
    buf->start.y = 0;
    buf->start.w = CLUSTER_WIDTH;
    buf->start.h = CLUSTER_HEIGHT;
    buf->dst.x = 0;
    buf->dst.y = 0;
    buf->dst.w = CLUSTER_WIDTH;
    buf->dst.h = CLUSTER_HEIGHT;
    buf->layer = 0;
    buf->z_order = 0;

    /* cluster on the 3rd display */
    list_for_every_entry(head, sdm, sdm_display_t, node) {
        if (sdm->handle->display_id == CLUSTER)
            break;
    }

    int cluster_status = 0;

    /* initialize fg and bg */
    cluster_bg_init(&ui,0);
    cluster_fg_init(&ui,59.9,0,0);
    buf = &post.bufs[0];
    buf->addr[0] = (unsigned long)cluster_ui_get_fg(&ui);
    buf = &post.bufs[1];
    buf->addr[0] = (unsigned long)cluster_ui_get_bg(&ui);
    /* enable display */
    LOGD("disp->id, disp->handle->display_id (%d, %d)\n",
             sdm->id, sdm->handle->display_id);
    printf("get sdm screen:%16s in cluster app\n",sdm->handle->name);
    //above should be integrated into init phase.


    if(disp_flags)
    {
        sdm_post(sdm->handle, &post);
    }
    else
    {
        sdm_clear_display(sdm->handle);
    }

    printf("Startup Time = %d ms\n",current_time()-as);

    timer_t fps_timer;
    event_t fps_timer_signal;

    event_init(&fps_timer_signal, false, EVENT_FLAG_AUTOUNSIGNAL);
    timer_initialize(&fps_timer);
    timer_set_periodic(&fps_timer, 7, fps_timer_handler, &fps_timer_signal);

    float fps = 59.9;
    float rpm = 0;
    float kmph = 0;
    float rpm_step = 0;
    float kmph_step = 0;
    uint32_t nav_en = 0;
    uint32_t mode = 0;
    float kmph_max_step = (KMPH_MAX/ANGLE_RANGE);
    float rpm_max_step = (RPM_MAX/ANGLE_RANGE);
    float kmph_target = 0;

    int frame=0;
    uint32_t ts = 0;
    uint32_t ps = 0;
    uint32_t bg_alpha = 0xff;
    //uint32_t nav_alpha = 0xff;
    uint32_t bg_off = 0;
    struct surface_buff* surfacebuff = NULL;
    int status = -1; //for start info end
    status_t receive_status = NO_ERROR;

    struct cluster_ui ui_save = ui;

    //ts = current_time();

    while(1)
    {
        //event_wait(&fps_timer_signal);

        //ps = current_time();

        /* update fg */
        cluster_fg_update(&ui,fps,kmph,rpm);
        /* update buffer into to display frame work */
        buf = &post.bufs[0];
        buf->addr[0] = (unsigned long)cluster_ui_get_fg(&ui);
#if (CLUSTER_NAV_EN==1)
        if((bg_off==0) && (nav_en==1))
        {
            /* change fg to embedded alpha so bg can be displayed */
            buf = &post.bufs[0];
            if(buf->alpha_en==1)
            {
                buf->alpha_en=0;
            }

            /* reduce bg alpha so the default bg disappears */
            buf = &post.bufs[1];
            bg_alpha = buf->alpha;
            if(bg_alpha>16)
            {
                bg_alpha = bg_alpha - 4;
            }
            else
            {
                bg_alpha = 0;
                bg_off = 1;
                //nav_alpha = 16;
            }
            buf->alpha = bg_alpha;
        }
        if((bg_off==1) && (nav_en==1))
        {
            receive_status = surface_receiver_dequeue(&surfacebuff, 1);
            if (receive_status == NO_ERROR) {
                status = surfacebuff->surface.cmd;
                if (surfacebuff != NULL && surfacebuff->surface.cmd == surface_info) {
                    if(surfacebuff->surface.phy_addr != 0) {
                        cluster_bg_update(&ui,1,(uint32_t *)((uint32_t)(surfacebuff->surface.phy_addr-0x10000000)),1,0);
                        buf = &post.bufs[1];
                        //for alpha gradual change
                        /*if(nav_alpha+4<0xff)
                        {
                            nav_alpha = nav_alpha + 4;
                        }
                        else
                        {
                            nav_alpha = 0xff;
                        }*/
                        /* change display buffer setup based on image from nav system */
                        buf->alpha = 0xff;//nav_alpha;
                        buf->src_stride[0] = surfacebuff->surface.stride*4;
                        buf->dst.x = NAV_POS_X;
                        buf->dst.y = NAV_POS_Y;

                        buf->addr[0] = (unsigned long)cluster_ui_get_bg(&ui);
                        buf->layer_en = 1;
                    }
                } else {
                    buf = &post.bufs[1];
                    buf->alpha = 0xff;
                    buf->layer_en = 1;
                }
                buf = &post.bufs[0];
                buf->alpha_en = 0;
            } else {
                //Prevent splash screen
                if (status == surface_info || status == surface_start)
                {
                    buf = &post.bufs[1];
                    buf->alpha = 0xff;
                    buf->layer_en = 1;
                }
                else {
                    buf = &post.bufs[1];
                    buf->alpha = 0;
                    buf->addr[0] = (unsigned long)cluster_ui_get_bg(&ui_save);//add
                    buf->layer_en = 0;

                    //buf = &post.bufs[0];
                    //buf->alpha_en=1;
                }
            }
        }
#endif

        if(disp_flags)
        {
            sdm_post(sdm->handle, &post);
        }
        else
        {
            sdm_clear_display(sdm->handle);
        }
        //printf("sdm_post Time = %d ms\n",current_time()-ps);
#if (CLUSTER_NAV_EN==1)
        if (receive_status == NO_ERROR) {
            surface_receiver_enqueue();
        }
#endif

        frame++;
        if(frame==30)
        {
            fps = frame * 1000.0 / (current_time()-ts);
            ts = current_time();
            frame = 0;
        }

        /* Demo code */
        if(mode==0) // self test cycle 0
        {
            kmph = kmph + kmph_max_step;
            rpm = rpm + rpm_max_step;
            if((kmph>120) || (rpm>8000))
            {
            kmph = 120;
            rpm = 8000;
            mode = 1;
            }
        }
        else if(mode==1) // self test cycle 1
        {
            kmph = kmph - kmph_max_step;
            rpm = rpm - rpm_max_step;
            if((kmph<0) || (rpm<0))
            {
            kmph = 0;
            rpm = 0;
            mode = 2;
            kmph_target = 60 + current_time()%60;
            }
        }
        else if(mode==2) // acc mode
        {
        #if (CLUSTER_NAV_EN==1)
                nav_en = 1;
        #endif
                if(rpm >6000)
                {
                rpm_step = rpm_max_step*2;
                }
                else if(rpm >4000)
                {
                    rpm_step = rpm_max_step*(rpm/4000);
                }
                else
                {
                    rpm_step = rpm_max_step;
                }
            rpm = rpm + rpm_step;
            if(rpm>7500)
            {
            rpm = 7500;
            }

            kmph = kmph + kmph_max_step;
            if(kmph>kmph_target)
            {
            kmph = kmph_target;
            kmph_target = 60 - current_time()%50;
            mode = 3;
            }
        }
        else if(mode==3) // break mode
        {
            rpm_step = rpm / 2000 * rpm_max_step;
            rpm = rpm - rpm_step;
            if(rpm<1000)
            {
            rpm = 1000;
            }

            kmph = kmph - kmph_max_step;
            if(kmph<kmph_target)
            {
            kmph = kmph_target;
            kmph_target = 60 + current_time()%60;
            mode = 2;
            }
        }
    }

    free(frame_buffer);
    free(img_source);
    printf("Cluster finish.\n");
#else
    //printf("Cluster start \n");
    //TaskHandle_t task = xTaskGetCurrentTaskHandle();

    lv_cluster_init();

    lv_cluster_start();

    lv_cluster_stop();

    lv_cluster_deinit();
#endif
}

void cluster_resume(void)
{
    disp_flags = 1;
}

void cluster_halt(void)
{
    disp_flags = 0;
}


#include <sys_diagnosis.h>

void cluster_property_set_worker(void* p1, uint32_t p2)
{
    system_property_set(DMP_ID_CLUSTER_STATUS,p2);
    printf("set cluster to 0x%x\n",p2);
}

void cluster_changed_cb(int property_id, int old, int new, void* d)
{
    (void)d;
    if(property_id != DMP_ID_CLUSTER_STATUS)
    {
        printf("Unexpected property changed trigger %d\n",property_id);
        return;
    }

    uint16_t* pstate;

    pstate = (uint16_t*)&new;

    printf("get cluster property:0x%x - 0x%x\n",old,new);
    //changed by other domain?
    if(pstate[1])
    {
        //clear
        pstate[1] = 0;
        pstate[0] = 0;

        xTimerPendFunctionCall(cluster_property_set_worker,NULL,*pstate,portMAX_DELAY);

        return;
    }

    if(pstate[0])
    {
        cluster_resume();
    }
    else
    {
        cluster_halt();
    }

}

void cluster_property_update(bool remote, uint16_t val)
{
    int v;
    system_property_get(DMP_ID_CLUSTER_STATUS,&v);
    if(remote)
    {
        v &= 0xFFFF0000;
        v |= (val << 16);
    }
    else
    {
        v &= 0x0000FFFF;
        v |= val;
    }

    system_property_set(DMP_ID_CLUSTER_STATUS,v);
}

int cluster_backup_callback(int signal,void* args)
{
    cluster_property_update(1,1);//will be set to 0x4 by ap2
    return 0;
}

void start_cluster(void)
{
    static bool flag = 0;
    if(flag == 0)
    {
        cluster_property_update(0,1);
        flag = 1;
    }
    else
    {
        cluster_property_update(1,1);
        flag = 0;
    }

}

SYSD_CALL(cluster)
{
    system_property_observe(DMP_ID_CLUSTER_STATUS,cluster_changed_cb,NULL);
    sysd_register_handler(cluster_backup_callback,NULL,1,WDT6____ovflow_int);
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("cluster", "start cluster", (console_cmd)&start_cluster)
STATIC_COMMAND_END(cluster_sets);
#endif
