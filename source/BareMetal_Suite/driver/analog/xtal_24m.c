/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include "analog.h"
#include "xtal_24m_reg.h"

BOOL ana_is_xtal_ready(U32 base)
{
    xtal_24m_t *xtal = (xtal_24m_t *)(uintptr_t)base;

    return (xtal->xtal_setting & BM_XTAL_SETTING_XTAL_LOCK) ==
           BM_XTAL_SETTING_XTAL_LOCK ;
}

void ana_xtal_set_xgain(U32 base, U32 xgain)
{
    xtal_24m_t *xtal = (xtal_24m_t *)(uintptr_t)base;

    U32 v = xtal->xtal_setting;
    v &= ~FM_XTAL_SETTING_XTAL_GAIN;
    v |= FV_XTAL_SETTING_XTAL_GAIN(xgain);
    xtal->xtal_setting = v;
}
