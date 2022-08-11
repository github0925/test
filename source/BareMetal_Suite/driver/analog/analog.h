/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __ANALOG_H__
#define __ANALOG_H__

#include <common_hdr.h>
#include <soc.h>

typedef enum {
    SEL_OSC,
    SEL_XTAL1,
    SEL_XTAL2,
} osc_out_sel_e;

typedef enum {
    FSRC_SAF,
    FSRC_SEC,
    FSRC_HIS,
    FSRC_HPI,
    FSRC_DDR,
} fsrefclk_sel_e;

BOOL ana_is_xtal_ready(U32 base);
void ana_osc24_output_sel(U32 fsrc, osc_out_sel_e sel);
osc_out_sel_e ana_osc24_get_output_sel(U32 fsrc);
void ana_xtal_set_xgain(U32 base, U32 xgain);

/* 10,000 clk (at most) needed for the xtal to be locked */
#define XTAL_LOCK_TIMEOUT_us    4200

#endif  /* __ANALOG_H__ */
