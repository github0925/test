/*
* disp_panel.c
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
#include <disp_drv_log.h>
#include <sdm_panel.h>
#include <disp_dsi.h>
#include <disp_dsi_api.h>
#include <disp_general_header.h>

extern struct dsi_context dsi_ctx[2];

/*int lcm_init_power(void)
{
    return 0
}

int lcm_probe(void)
{
    return 0
}*/

int mipi_lcm_init(int dsi_index, struct sdm_panel *panel)
{
    struct dsi_cmd_desc *cmds = panel->mipi->init_data;
    struct dsi_context *ctx = &dsi_ctx[dsi_index];
    struct dsi_core_ops *dsi_ops = ctx->ops;
    uint32_t size = ARRAYSIZE(cmds);
    uint16_t len;
    uint32_t i;

    if (ctx == NULL)
        return -1;

    if (cmds == NULL)
        goto skip_send_cmd;

    dsi_ops->cmd_mode(ctx);

    for (i = 0; i < size; i++) {
        len = (cmds->wc_h << 8) | cmds->wc_l;
        if (panel->mipi->use_dcs)
            mipi_dsih_dcs_wr_cmd(ctx, ctx->dpi_video.virtual_channel,
                cmds->playload, len);
        else
            mipi_dsih_gen_wr_cmd(ctx, ctx->dpi_video.virtual_channel,
                cmds->playload, len);

        if (cmds->wait)
            udelay(cmds->wait * 1000);

        cmds++;
    }

skip_send_cmd:
    dsi_ops->dphy_enable_hs_clk(ctx, 1);

    dsi_ops->power_enable(ctx, 0);
    udelay(100);
    dsi_ops->power_enable(ctx, 1);
    udelay(10*1000);

    dsi_ops->video_mode(ctx);
    //dsi_ops->int0_status(ctx);
    //dsi_ops->int1_status(ctx);

    return 0;
}

