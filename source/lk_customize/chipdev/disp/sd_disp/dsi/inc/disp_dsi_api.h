/*
* disp_dsi_api.h
*
* Copyright (c) 2019-2020 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 12/23/2019 BI create this file
*/
#ifndef __DISP_DSI_API_H__
#define __DISP_DSI_API_H__
#include <disp_dsi.h>

int mipi_dsih_open(struct dsi_context *ctx);
int mipi_dsih_dpi_video(struct dsi_context *ctx);
int mipi_dsih_gen_wr_cmd(struct dsi_context *ctx, uint8_t vc,
    uint8_t *params, uint16_t param_length);
uint16_t mipi_dsih_gen_rd_cmd(struct dsi_context *ctx, uint8_t vc,
    uint8_t *params, uint16_t param_length, uint8_t bytes_to_read,
    uint8_t *read_buffer);
int mipi_dsih_dcs_rd_cmd(struct dsi_context *ctx, uint8_t vc, uint8_t command,
    uint8_t bytes_to_read, uint8_t *read_buffer);
int mipi_dsih_dcs_wr_cmd(struct dsi_context *ctx, uint8_t vc,
    uint8_t *params, uint16_t param_length);

#endif //__DISP_DSI_API_H__
