/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <app.h>
#include <trace.h>
#include "dw_pcie.h"

static struct kunlun_pcie pcie1_ep;

static void pcie1_ep_v9_entry(const struct app_descriptor *app, void *args)
{
    kunlun_pcie1_ep_mode_init(&pcie1_ep);
    kunlun_pcie1_ep_v9_cfg(&pcie1_ep);
    printf("V9 EP init done!\n");
}

APP_START(pcie_ep_entry)
.flags = 0,
.entry = pcie1_ep_v9_entry,
APP_END
