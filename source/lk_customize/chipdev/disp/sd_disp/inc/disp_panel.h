/*
* disp_panel.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 10/21/2019 BI create this file
*/

#ifndef __DISP_PANEL_H__
#define __DISP_PANEL_H__

#include <sdm_panel.h>

int mipi_lcm_init(int dsi_index, struct sdm_panel *panel);

#endif //__DISP_PANEL_H__
