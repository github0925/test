#include <lk_wrapper.h>
#include "cluster_draw.h"
#include "cluster_ui_parameter.h"
#include "cluster_ui.h"

uint32_t cluster_kmph_to_angle(float kmph)
{
    uint32_t angle_offset;

    if(kmph < KMPH_MIN)
        kmph = KMPH_MIN;
    else if(kmph >KMPH_MAX)
        kmph = KMPH_MAX;

    angle_offset = (uint32_t) (kmph*ANGLE_RANGE/(KMPH_MAX-KMPH_MIN));

    if(angle_offset <= MIN_ANGLE)
        return MIN_ANGLE-angle_offset;
    else
        return 360+MIN_ANGLE-angle_offset;
}

uint32_t cluster_rpm_to_angle(float rpm)
{
    uint32_t angle_offset;

    if(rpm < RPM_MIN)
        rpm = RPM_MIN;
    else if(rpm > RPM_MAX)
        rpm = RPM_MAX;

    angle_offset = (uint32_t) (rpm*ANGLE_RANGE/(RPM_MAX-RPM_MIN));

    if(angle_offset <= MIN_ANGLE)
        return MIN_ANGLE-angle_offset;
    else
        return 360+MIN_ANGLE-angle_offset;
}

bool cluster_angle_check(uint32_t a, uint32_t b)
{
    if(((a<=90) && (b<=90)) ||
       (((a>90)&&(a<=180)) && ((b>90)&&(b<=180))) ||
       (((a>180)&&(a<270)) && ((b>180)&&(b<270))) ||
       (((a>=270)&&(a<360)) && ((b>=270)&&(b<360)))
      )
        return 0;
    else
        return 1;

}
void cluster_bg_init(struct cluster_ui * ui, uint32_t nav_en)
{
    draw_bg(CLUSTER_HEIGHT, CLUSTER_WIDTH, CLUSTER_HEIGHT, CLUSTER_WIDTH,
            ui->res.bg_src, ui->bg[0].buf);
    draw_alpha(NAV_HEIGHT, NAV_WIDTH, CLUSTER_WIDTH, NAV_WIDTH, CLUSTER_WIDTH,
               ui->bg[0].buf+NAV_POS_X+NAV_POS_Y*CLUSTER_WIDTH, ui->res.nav_src, ui->bg[0].buf+NAV_POS_X+NAV_POS_Y*CLUSTER_WIDTH);
#if (CLUSTER_BG_NAV_ONLY==1)
#else
    draw_bg(CLUSTER_HEIGHT, CLUSTER_WIDTH, CLUSTER_HEIGHT, CLUSTER_WIDTH,
            ui->res.bg_src, ui->bg[1].buf);
    draw_bg(CLUSTER_HEIGHT, CLUSTER_WIDTH, CLUSTER_HEIGHT, CLUSTER_WIDTH,
            ui->res.bg_src, ui->bg[2].buf);
#endif

    ui->nav_en = nav_en;

    ui->static_bg = &ui->bg[0];
    ui->current_bg = &ui->bg[1];
    ui->next_bg = &ui->bg[2];
}

void cluster_fg_init(struct cluster_ui * ui, float fps, float kmph, float rpm)
{
    struct cluster_fg * fg;
    int i;

    for(i=0;i<2;i++)
    {
        fg = &ui->fg[i];

        fg->fps = fps;
        fg->kmph = kmph;
        fg->rpm = rpm;
#if (CLUSTER_BG_NAV_ONLY==1)
        draw_bg(CLUSTER_HEIGHT, CLUSTER_WIDTH, CLUSTER_HEIGHT, CLUSTER_WIDTH,
                ui->res.bg_src, fg->buf);
        draw_alpha(NAV_HEIGHT, NAV_WIDTH, CLUSTER_WIDTH, NAV_WIDTH, CLUSTER_WIDTH,
                   fg->buf+NAV_POS_X+NAV_POS_Y*CLUSTER_WIDTH, ui->res.nav_src, fg->buf+NAV_POS_X+NAV_POS_Y*CLUSTER_WIDTH);
        draw_fps_with_bg(fps, FPS_COLOR, FPS_POS_X, FPS_POS_Y, CLUSTER_WIDTH, CLUSTER_WIDTH, ui->res.numbers_src, fg->buf, ui->bg[0].buf);
        /*
        draw_needle_with_bg(cluster_kmph_to_angle(kmph),NEEDLE_R_COLOR,NEEDLE_R_POS_X,NEEDLE_R_POS_Y,CLUSTER_WIDTH,CLUSTER_WIDTH,
                    ui->res.needles_src,fg->buf,ui->res.bg_src);
        draw_needle_with_bg(cluster_rpm_to_angle(rpm),NEEDLE_L_COLOR,NEEDLE_L_POS_X,NEEDLE_L_POS_Y,CLUSTER_WIDTH,CLUSTER_WIDTH,
                    ui->res.needles_src,fg->buf,ui->res.bg_src);
                    */
        draw_needle_with_bg(cluster_kmph_to_angle(kmph),NEEDLE_R_COLOR,NEEDLE_R_POS_X,NEEDLE_R_POS_Y,CLUSTER_WIDTH,CLUSTER_WIDTH,
                    ui->res.needles_src,fg->buf,ui->bg[0].buf);
        draw_needle_with_bg(cluster_rpm_to_angle(rpm),NEEDLE_L_COLOR,NEEDLE_L_POS_X,NEEDLE_L_POS_Y,CLUSTER_WIDTH,CLUSTER_WIDTH,
                    ui->res.needles_src,fg->buf,ui->bg[0].buf);
#else
        draw_fill(CLEAR_COLOR, CLUSTER_HEIGHT, CLUSTER_WIDTH, CLUSTER_WIDTH, fg->buf);
        draw_fps(fps, FPS_COLOR, FPS_POS_X, FPS_POS_Y, CLUSTER_WIDTH, ui->res.numbers_src, fg->buf);
        draw_needle(cluster_kmph_to_angle(kmph),NEEDLE_R_COLOR,NEEDLE_R_POS_X,NEEDLE_R_POS_Y,CLUSTER_WIDTH,
                    ui->res.needles_src,fg->buf);
        draw_needle(cluster_rpm_to_angle(rpm),NEEDLE_L_COLOR,NEEDLE_L_POS_X,NEEDLE_L_POS_Y,CLUSTER_WIDTH,
                    ui->res.needles_src,fg->buf);
#endif
    }
    ui->current_fg = &ui->fg[0];
    ui->next_fg = &ui->fg[1];
}

void cluster_bg_update(struct cluster_ui * ui, uint32_t nav_en, uint32_t * nav_source, uint32_t div, uint32_t index)
{
    struct cluster_bg * current_bg;
    struct cluster_bg * next_bg;
    struct cluster_bg * static_bg;

    current_bg = ui->current_bg;
    next_bg = ui->next_bg;
    static_bg = ui->static_bg;

    if(nav_en)
    {
#if (CLUSTER_BG_NAV_ONLY==1)
        next_bg->buf = nav_source;
        static_bg = static_bg;
#else
        draw_overlay(1,
                NAV_HEIGHT/div,
                NAV_WIDTH,
                NAV_POS_X + NAV_HEIGHT/div*index*CLUSTER_WIDTH,
                NAV_POS_Y,
                NAV_POS_X + NAV_HEIGHT/div*index*CLUSTER_WIDTH,
                NAV_POS_Y,
                NAV_WIDTH,
                CLUSTER_WIDTH,
                CLUSTER_WIDTH,
                nav_source + NAV_HEIGHT/div*index*NAV_WIDTH,
                static_bg->buf,
                next_bg->buf);
#endif
        ui->nav_en = 1;
        ui->current_bg = next_bg;
        ui->next_bg = current_bg;
    }
    else
    {
        ui->nav_en = 0;
    }
}

void cluster_fg_update(struct cluster_ui * ui, float fps, float kmph, float rpm)
{
    uint32_t fps_current;
    uint32_t fps_next;
    uint32_t kmph_current_angle;
    uint32_t rpm_current_angle;
    uint32_t kmph_next_angle;
    uint32_t rpm_next_angle;
    struct cluster_fg * current_fg;
    struct cluster_fg * next_fg;
    int32_t ps = current_time();
    current_fg = ui->current_fg;
    next_fg = ui->next_fg;

    fps_current = (uint32_t) (current_fg->fps * 10);
    kmph_current_angle = cluster_kmph_to_angle(current_fg->kmph);
    rpm_current_angle = cluster_rpm_to_angle(current_fg->rpm);
    fps_next = (uint32_t) (fps * 10);
    kmph_next_angle = cluster_kmph_to_angle(kmph);
    rpm_next_angle = cluster_rpm_to_angle(rpm);
    if((fps_current == fps_next) &&
       (kmph_current_angle == kmph_next_angle) &&
       (rpm_current_angle == rpm_next_angle))
    {
        /* no need to update fg */
        return;
    }
    else
    {
        /* update fg, in this case, check against next fg */
        fps_current = (uint32_t) (next_fg->fps * 10);
        kmph_current_angle = cluster_kmph_to_angle(next_fg->kmph);
        rpm_current_angle = cluster_rpm_to_angle(next_fg->rpm);
        if(fps_current != fps_next)
        {
#if (CLUSTER_BG_NAV_ONLY==0)
            draw_fps(fps, FPS_COLOR, FPS_POS_X, FPS_POS_Y, CLUSTER_WIDTH, ui->res.numbers_src, next_fg->buf);
#else
            draw_fps_with_bg(fps, FPS_COLOR, FPS_POS_X, FPS_POS_Y, CLUSTER_WIDTH, CLUSTER_WIDTH, ui->res.numbers_src, next_fg->buf, ui->bg[0].buf);
#endif
        }
        if(kmph_current_angle != kmph_next_angle)
        {
            if(cluster_angle_check(kmph_current_angle, kmph_next_angle))
            {
#if (CLUSTER_BG_NAV_ONLY==0)
                draw_needle(kmph_current_angle,CLEAR_COLOR,NEEDLE_R_POS_X,NEEDLE_R_POS_Y,CLUSTER_WIDTH,
                            ui->res.needles_src,next_fg->buf);
#else
                draw_needle_with_bg(kmph_current_angle,CLEAR_COLOR,NEEDLE_R_POS_X,NEEDLE_R_POS_Y,CLUSTER_WIDTH,CLUSTER_WIDTH,
                            ui->res.needles_src,next_fg->buf,ui->bg[0].buf);
#endif
            }
#if (CLUSTER_BG_NAV_ONLY==0)
            draw_needle(kmph_next_angle,NEEDLE_R_COLOR,NEEDLE_R_POS_X,NEEDLE_R_POS_Y,CLUSTER_WIDTH,
                        ui->res.needles_src,next_fg->buf);
#else
            draw_needle_with_bg(kmph_next_angle,NEEDLE_R_COLOR,NEEDLE_R_POS_X,NEEDLE_R_POS_Y,CLUSTER_WIDTH,CLUSTER_WIDTH,
                        ui->res.needles_src,next_fg->buf,ui->bg[0].buf);
#endif
        }
        if(rpm_current_angle != rpm_next_angle)
        {
            if(cluster_angle_check(rpm_current_angle, rpm_next_angle))
            {
#if (CLUSTER_BG_NAV_ONLY==0)
                draw_needle(rpm_current_angle,CLEAR_COLOR,NEEDLE_L_POS_X,NEEDLE_L_POS_Y,CLUSTER_WIDTH,ui->res.needles_src,next_fg->buf);
#else
                draw_needle_with_bg(rpm_current_angle,CLEAR_COLOR,NEEDLE_L_POS_X,NEEDLE_L_POS_Y,CLUSTER_WIDTH,CLUSTER_WIDTH,
                        ui->res.needles_src,next_fg->buf,ui->bg[0].buf);
#endif
            }
#if (CLUSTER_BG_NAV_ONLY==0)
            draw_needle(rpm_next_angle,NEEDLE_L_COLOR,NEEDLE_L_POS_X,NEEDLE_L_POS_Y,CLUSTER_WIDTH,ui->res.needles_src,next_fg->buf);
#else
            draw_needle_with_bg(rpm_next_angle,NEEDLE_L_COLOR,NEEDLE_L_POS_X,NEEDLE_L_POS_Y,CLUSTER_WIDTH,CLUSTER_WIDTH,
                    ui->res.needles_src,next_fg->buf,ui->bg[0].buf);
#endif
        }

        /* update values for next fg */
        next_fg->fps = fps;
        next_fg->kmph = kmph;
        next_fg->rpm = rpm;

        /* swap current fg with next fg */
        ui->next_fg = current_fg;
        ui->current_fg = next_fg;
    }
}

uint32_t * cluster_ui_get_fg(struct cluster_ui * ui)
{
    return ui->current_fg->buf;
}

uint32_t * cluster_ui_get_bg(struct cluster_ui * ui)
{
    if(ui->nav_en)
        return ui->current_bg->buf;
    else
        return ui->static_bg->buf;
}

