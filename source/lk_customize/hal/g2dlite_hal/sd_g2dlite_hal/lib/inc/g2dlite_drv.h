/*
* g2dlite_drv.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 04/27/2020 BI create this file
*/
#ifndef __G2DLITE_DRV__
#define __G2DLITE_DRV__
int sd_g2dlite_init(struct g2dlite_instance *handle);
int sd_g2dlite_update(struct g2dlite_instance *handle, struct g2dlite_input *input);
int sd_g2dlite_rotaion(struct g2dlite_instance *handle, struct g2dlite_input *input);
int sd_g2dlite_fill_rect(struct g2dlite_instance *handle, struct g2dlite_bg_cfg *bgcfg,
        struct g2dlite_output_cfg *output);
int sd_g2dlite_fastcopy(struct g2dlite_instance *handle, struct fcopy_t *in, struct fcopy_t *out);
int sd_g2dlite_clut_setting(struct g2dlite_instance *handle, char *clut_table);
#endif //__G2DLITE_DRV__
