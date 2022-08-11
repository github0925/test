/*
* disp_dsi_api.c
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

#include <disp_dsi.h>
#include <disp_drv_log.h>
#include <disp_general_header.h>

#define DSIH_FIFO_ACTIVE_WAIT   (200)

#define DPHY_DIV_UPPER_LIMIT (8000)
#define DPHY_DIV_LOWER_LIMIT (2000)
#define MIN_OUTPUT_FREQ (80)

static void mipi_dsih_dphy_open(struct dsi_context *ctx)
{
    /*
    struct dsi_core_ops *dsi_ops = ctx->ops;
    dsi_ops->dphy_reset(ctx, 0);
    dsi_ops->dphy_stop_wait_time(ctx, 0x1C);
    dsi_ops->dphy_n_lanes(ctx, 1);
    dsi_ops->dphy_enableclk(ctx, 1);
    dsi_ops->dphy_shutdown(ctx, 1);
    dsi_ops->dphy_reset(ctx, 1);
    */
}

static int wait_dphy_ready(struct dsi_context *ctx)
{
    int32_t count = 0;
    struct dsi_core_ops *dsi_ops = ctx->ops;

    while (!dsi_ops->dphy_get_status(ctx, 0x5))  {
        udelay(1);
        count++;
        if (count > 5000) {
            DISPERR("wait dphy lock&& stopstate timeout\n");
            return -1;
        }
    }

    DISPMSG("dphy lock and stopstate ok\n");
    return 0;
}

static int mipi_dsih_gen_packet_payload(struct dsi_context *ctx,
    uint32_t payload)
{
    struct dsi_core_ops *dsi_ops = ctx->ops;

    if (dsi_ops->is_tx_payload_fifo_full(ctx))
        return -1;

    dsi_ops->set_packet_payload(ctx, payload);

    return 0;
}


int mipi_dsih_dphy_configure(struct dsi_context *ctx,
    uint8_t num_of_lanes, uint32_t output_data_rate)
{
    int ret;
    int flag = 0;
    unsigned long loop_divider = 0; /*(M)*/
    unsigned long input_divider = 0; /*(N)*/
    unsigned long vco_divider = 1; /*(VCO)*/
    unsigned long delta = 0;
    unsigned long tmp_loop_divider = 0;
    unsigned long output_freq = output_data_rate / 2;
    unsigned int step = 0;
    unsigned int reference_freq = 24000; /*[KHz]*/
    uint8_t i;
    uint8_t no_of_bytes = 0;
    uint8_t range_index = 0;
    uint8_t data[4]; /* maximum data for now are 4 bytes per test mode */
    struct dsi_core_ops *dsi_ops = ctx->ops;
    struct
    {
        uint32_t data_rate; /* upper margin of frequency range */
        uint8_t hs_freq;	/* hsfreqrange */
        uint8_t vco_range;  /* vcorange */
    } ranges[] =
        {
            {80, 0x00, 0x3F},
            {90, 0x10, 0x3F},
            {100, 0x20, 0x3F},
            {110, 0x30, 0x39},
            {120, 0x01, 0x39},
            {130, 0x11, 0x39},
            {140, 0x21, 0x39},
            {150, 0x31, 0x39},
            {160, 0x02, 0x39},
            {170, 0x12, 0x2F},
            {180, 0x22, 0x2F},
            {190, 0x32, 0x2F},
            {205, 0x03, 0x2F},
            {220, 0x13, 0x29},
            {235, 0x23, 0x29},
            {250, 0x33, 0x29},
            {275, 0x04, 0x29},
            {300, 0x14, 0x29},
            {325, 0x25, 0x29},
            {350, 0x35, 0x1F},
            {400, 0x05, 0x1F},
            {450, 0x16, 0x19},
            {500, 0x26, 0x19},
            {550, 0x37, 0x19},
            {600, 0x07, 0x19},
            {650, 0x18, 0x19},
            {700, 0x28, 0x0F},
            {750, 0x39, 0x0F},
            {800, 0x09, 0x0F},
            {850, 0x19, 0x0F},
            {900, 0x29, 0x09},
            {950, 0x3A, 0x09},
            {1000, 0x0A, 0x09},
            {1050, 0x1A, 0x09},
            {1100, 0x2A, 0x09},
            {1150, 0x3B, 0x09},
            {1200, 0x0B, 0x09},
            {1250, 0x1B, 0x09},
            {1300, 0x2B, 0x09},
            {1350, 0x3C, 0x03},
            {1400, 0x0C, 0x03},
            {1450, 0x1C, 0x03},
            {1500, 0x2C, 0x03},
            {1550, 0x3D, 0x03},
            {1600, 0x0D, 0x03},
            {1650, 0x1D, 0x03},
            {1700, 0x2E, 0x03},
            {1750, 0x3E, 0x03},
            {1800, 0x0E, 0x03},
            {1850, 0x1E, 0x03},
            {1900, 0x2F, 0x03},
            {1950, 0x3F, 0x03},
            {2000, 0x0F, 0x03},
            {2050, 0x40, 0x03},
            {2100, 0x41, 0x03},
            {2150, 0x42, 0x03},
            {2200, 0x43, 0x03},
            {2250, 0x44, 0x03},
            {2300, 0x45, 0x01},
            {2350, 0x46, 0x01},
            {2400, 0x47, 0x01},
            {2450, 0x48, 0x01},
            {2500, 0x49, 0x01}};

    if (ctx == NULL)
        return -1;

    if (output_freq < MIN_OUTPUT_FREQ)
        return -1;

    for (i = 0; i < ARRAYSIZE(ranges); i++) {
        if ((output_data_rate / 1000) <= ranges[i].data_rate) {
            range_index = i;
            break;
        }
    }

    if (range_index >= ARRAYSIZE(ranges)) {
        DISPERR("Your input output_data_rate=%d is out of our ranges",
            output_data_rate);
        return -1;
    }

    switch (ranges[range_index].vco_range >> 4) {
        case 3:
            vco_divider = 8;
            break;
        case 2:
            vco_divider = 4;
        default:
            vco_divider = 2;
    }

    if (ranges[range_index].data_rate > 640)
        vco_divider = 1;

    output_freq = output_freq * vco_divider;

    loop_divider = (output_freq * (reference_freq / DPHY_DIV_LOWER_LIMIT)) / reference_freq;

    /*Here delta will account for the rounding*/
    delta = (loop_divider * reference_freq) / (reference_freq / DPHY_DIV_LOWER_LIMIT) - output_freq;

    for (input_divider = 1 + reference_freq / DPHY_DIV_UPPER_LIMIT;
        ((reference_freq / input_divider) >= DPHY_DIV_LOWER_LIMIT) && (!flag);
        input_divider++) {
        tmp_loop_divider = (output_freq * input_divider) / reference_freq;
        if ((tmp_loop_divider % 2) == 0) {
            /*if even*/
            if (output_freq == tmp_loop_divider * (reference_freq / input_divider)) {
                /*exact values found*/
                flag = 1;
                loop_divider = tmp_loop_divider;

                delta = output_freq - tmp_loop_divider * (reference_freq / input_divider);

                /*variable was incremented before exiting the loop*/
                input_divider--;
            }

            if ((output_freq - tmp_loop_divider * (reference_freq / input_divider)) < delta) {
                /* values found with smaller delta */
                loop_divider = tmp_loop_divider;

                delta =
                output_freq - (tmp_loop_divider * (reference_freq / input_divider));
                step = 1;
            }
        } else {
            tmp_loop_divider += 1;

            if (output_freq == tmp_loop_divider * (reference_freq / input_divider)) {
                /*exact values found*/
                flag = 1;
                loop_divider = tmp_loop_divider;
                delta = tmp_loop_divider * (reference_freq / input_divider) - output_freq;

                /*variable was incremented before exiting the loop*/
                input_divider--;
            }

            if ((tmp_loop_divider * (reference_freq / input_divider) - output_freq) < delta) {
                /* values found with smaller delta */
                loop_divider = tmp_loop_divider;
                delta = tmp_loop_divider * (reference_freq / input_divider) - output_freq;
                step = 0;
            }
        }
    }

    if (!flag)
        input_divider = step + (loop_divider * reference_freq) / output_freq;

    /*
     * Get the PHY in power down mode
     * (shutdownz = 0) and reset it (rstz = 0) to avoid transient
     * periods in PHY operation during re-configuration procedures
     */
    dsi_ops->dphy_reset(ctx, 0);
    dsi_ops->dphy_enableclk(ctx, 0);
    dsi_ops->dphy_shutdown(ctx, 0);

    dsi_ops->dphy_test_clear(ctx, 1);
    dsi_ops->dphy_test_clear(ctx, 0);

    #if 1  // this is is_g118
    /* PLL Analog Programmability Control */
    data[0] = 0x01;
    dsi_ops->dphy_write(ctx, 0x1F, data, 1);

    /* setup PLL
     * Reserved | pll_vco_cntrl_ovr_en [7]| pll_vco_cntrl_ovr[6:0]*/
    data[0] = (1 << 7) | ranges[range_index].hs_freq;
    dsi_ops->dphy_write(ctx, 0x44, data, 1);

    /* PLL Proportional Charge Pump control*/
    if (ranges[range_index].data_rate >= 1150)
        data[0] = 0x0E;
    else
        data[0] = 0x0D;
    dsi_ops->dphy_write(ctx, 0x0e, data, 1);

    /* PLL Integral Charge Pump control*/
    data[0] = 0x0;
    dsi_ops->dphy_write(ctx, 0x0f, data, 1);

    /* PLL Charge Pump Bias Control*/
    data[0] = 0x10;
    dsi_ops->dphy_write(ctx, 0x1c, data, 1);

    /* PLL GMP Control and Digital Testability*/
    data[0] = 0x10;
    dsi_ops->dphy_write(ctx, 0x13, data, 1);

    /* setup PLL
     * Reserved | pll_vco_cntrl_ovr_en | pll_vco_cntrl_ovr*/
    data[0] = (1 << 6) | (ranges[range_index].vco_range);
    dsi_ops->dphy_write(ctx, 0x12, data, 1);

    data[0] = (0x00 << 6) | (0x01 << 5) | (0x01 << 4);
    if (ranges[range_index].data_rate > 1250)
        data[0] = 0x00;
    dsi_ops->dphy_write(ctx, 0x19, data, 1);

    /* PLL input divider ratio [7:0] */
    data[0] = input_divider - 1;
    dsi_ops->dphy_write(ctx, 0x17, data, 1);
    /* pll loop divider (code 0x18)
     * takes only 2 bytes (10 bits in data) */
    no_of_bytes = 2;
    /* 7 is dependent on no_of_bytes make sure 5 bits
     * only of value are written at a time */
    for (i = 0; i < no_of_bytes; i++)
        data[i] = (uint8_t)((((loop_divider - 2) >> (5 * i)) & 0x1F) | (i << 7));

    /* PLL loop divider ratio -
     * SET no|reserved|feedback divider [7]|[6:5]|[4:0] */
    dsi_ops->dphy_write(ctx, 0x18, data, no_of_bytes);

    /*data[0] = 0b00111110;
    dsi_ops->dphy_write(ctx, 0x10, data, 1);

    data[0] = 0b00111111;
    dsi_ops->dphy_write(ctx, 0x10, data, 1);

    data[0] = 0b00111011;
    dsi_ops->dphy_write(ctx, 0x10, data, 1);

    data[0] = 0b00101011;
    dsi_ops->dphy_write(ctx, 0x10, data, 1);

    data[0] = 0x18;
    dsi_ops->dphy_write(ctx, 0x1d, data, 1);*/
    #endif

    dsi_ops->dphy_n_lanes(ctx, num_of_lanes);
    dsi_ops->dphy_stop_wait_time(ctx, 0x1C);

    dsi_ops->dphy_enableclk(ctx, 1);
    dsi_ops->dphy_shutdown(ctx, 1);
    dsi_ops->dphy_reset(ctx, 1);

    ret = wait_dphy_ready(ctx);
    if (ret < 0) {
        DISPERR("dphy not ready\n");
        return ret;
    }
    return 0;
}

int mipi_dsih_open(struct dsi_context *ctx)
{
    //int ret;
    struct dsi_core_ops *dsi_ops = ctx->ops;

    mipi_dsih_dphy_open(ctx);

    if (!dsi_ops->check_version(ctx)) {
        DISPERR("dsi version error!\n");
        return -1;
    }

    dsi_ops->power_enable(ctx, 0);
    ctx->max_lanes = 4;
    ctx->max_bta_cycles = 4095;

    dsi_ops->dpi_color_mode_pol(ctx, 0);
    dsi_ops->dpi_shut_down_pol(ctx, 0);

    dsi_ops->int0_mask(ctx, 0x0);
    dsi_ops->int1_mask(ctx, 0x0);
    dsi_ops->max_read_time(ctx, ctx->max_bta_cycles);

    /*
     * By default, return to LP during ALL
     * unless otherwise specified
     */
    dsi_ops->dpi_hporch_lp_en(ctx, 1);
    dsi_ops->dpi_vporch_lp_en(ctx, 1);
    /* by default, all commands are sent in LP */
    dsi_ops->cmd_mode_lp_cmd_en(ctx, 1);
    /* by default, RX_VC = 0, NO EOTp, EOTn, BTA, ECC rx and CRC rx */
    dsi_ops->rx_vcid(ctx, 0);
    dsi_ops->eotp_rx_en(ctx, 0);
    dsi_ops->eotp_tx_en(ctx, 0);
    dsi_ops->bta_en(ctx, 0);
    dsi_ops->ecc_rx_en(ctx, 0);
    dsi_ops->crc_rx_en(ctx, 0);

    dsi_ops->video_mode_lp_cmd_en(ctx, 1);
    dsi_ops->power_enable(ctx, 1);
    /* dividing by 6 is aimed for max PHY frequency, 1GHz */
    dsi_ops->tx_escape_division(ctx, 6); /*need change to calc -- billy*/

    //ret = mipi_dsih_dphy_configure(ctx, 0, DEFAULT_BYTE_CLOCK);
    //if (ret < 0) {
    //    DISPERR("dphy configure failed\n");
    //    return -1;
   // }

#if 0
    if (dsi_ops->check_version(ctx)) {
        DISPERR("dsi version error!\n");
        return -1;
    }
    /*dphy pll clksel set*/
    //disp_write(disp_mux);

    /*config Display Pixel Interface*/
    dsi_ops->video_vcid(ctx, 0);
    dsi_ops->dpi_color_coding(ctx, COLOR_CODE_24BIT);

    dsi_ops->dpi_color_mode_pol(ctx, 0);
    dsi_ops->dpi_shut_down_pol(ctx, 0);
    dsi_ops->dpi_hsync_pol(ctx, 0);
    dsi_ops->dpi_vsync_pol(ctx, 0);
    dsi_ops->dpi_data_en_pol(ctx, 0);

    /*clkmrg cfg*/
    dsi_ops->timeout_clock_division(ctx, 1);
    dsi_ops->tx_escape_division(ctx, 7);

    /*Configure Packet handler*/
    dsi_ops->ecc_rx_en(ctx, 1);
    dsi_ops->crc_rx_en(ctx, 1);

    dsi_ops->video_mode(ctx);

    dsi_ops->video_mode_mode_type(ctx, VIDEO_BURST_WITH_SYNC_PULSES);
    dsi_ops->dpi_hporch_lp_en(ctx, 1);
    dsi_ops->dpi_vporch_lp_en(ctx, 1);

    dsi_ops->dpi_video_packet_size(ctx, 320); //frame width??
    dsi_ops->dpi_hsync_time(ctx, 200); //hsync
    dsi_ops->dpi_hbp_time(ctx, 40);//hbp
    dsi_ops->dpi_hline_time(ctx, 2200); //hsync+hbp_hact+hfp
    dsi_ops->dpi_vsync(ctx, 4);//vsync
    dsi_ops->dpi_vbp(ctx, 6);//vbp
    dsi_ops->dpi_vfp(ctx, 1);//vfp
    dsi_ops->dpi_vact(ctx, 5);//vact

    dsi_ops->datalane_hs2lp_config(ctx, 0x26);
    dsi_ops->datalane_lp2hs_config(ctx, 0x60);

    //dsi_ops->phy_n_lanes(ctx, 0x3);

    //dsi_ops->phy_enableclk(ctx, 0x1);

    /*Enabing all interrupts*/
    dsi_ops->int0_mask(ctx, 0xFFFFFFFF);
    dsi_ops->int1_mask(ctx, 0xFFFFFFFF);

    /*Waking up Core*/
    dsi_ops->power_enable(ctx, 0);
    dsi_ops->power_enable(ctx, 1);

/*-------**/
    /*Configuring new PLL parameters*/
    /*dphy shutdown & reset  release*/
    dsi_dphy_configure(ctx, 4, 500000);


    //wait_dphy_lock(1);
    wait_dphy_ready(ctx);
#endif

    return 0;
}

int mipi_dsih_dpi_video(struct dsi_context *ctx)
{
    int error = 0;
    uint32_t remain = 0;
    uint32_t counter = 0;
    uint32_t bytes_left = 0;
    uint32_t hs_timeout = 0;
    uint16_t video_size = 0;
    uint32_t total_bytes = 0;
    uint16_t no_of_chunks = 0;
    uint32_t chunk_overhead = 0;
    uint8_t video_size_step = 0;
    uint32_t bytes_per_chunk = 0;
    uint32_t ratio_clock_xPF = 0; /* holds dpi clock/byte clock times precision factor */
    uint16_t null_packet_size = 0;
    uint16_t bytes_per_pixel_x100 = 0; /* bpp x 100 because it can be 2.25 */
    uint32_t hsync_time = 0;
    struct dsi_core_ops *dsi_ops = ctx->ops;
    struct dsih_dpi_video_t *dpi_video = &ctx->dpi_video;

    dsi_ops->dphy_datalane_hs2lp_config(ctx, dpi_video->data_hs2lp);
    dsi_ops->dphy_datalane_lp2hs_config(ctx, dpi_video->data_lp2hs);
    dsi_ops->dphy_clklane_hs2lp_config(ctx, dpi_video->clk_hs2lp);
    dsi_ops->dphy_clklane_lp2hs_config(ctx, dpi_video->clk_lp2hs);
    dsi_ops->nc_clk_en(ctx, dpi_video->nc_clk_en);

    mipi_dsih_dphy_configure(ctx, dpi_video->n_lanes, dpi_video->phy_freq);

	ratio_clock_xPF =
		(dpi_video->byte_clock * PRECISION_FACTOR) / (dpi_video->pixel_clock);
    video_size = dpi_video->h_active_pixels;

    dsi_ops->video_mode_frame_ack_en(ctx, dpi_video->frame_ack_en);
    if (dpi_video->frame_ack_en) {
        /*
         * if ACK is requested, enable BTA
         * otherwise leave as is
         */
        dsi_ops->bta_en(ctx, 1);
    }
    dsi_ops->video_mode(ctx);

    /*
     * get bytes per pixel and video size
     * step (depending if loosely or not
     */
    switch (dpi_video->color_coding) {
    case COLOR_CODE_16BIT_CONFIG1:
    case COLOR_CODE_16BIT_CONFIG2:
    case COLOR_CODE_16BIT_CONFIG3:
        bytes_per_pixel_x100 = 200;
        //video_size_step = 1;
        break;
    case COLOR_CODE_18BIT_CONFIG1:
    case COLOR_CODE_18BIT_CONFIG2:
        dsi_ops->dpi_18_loosely_packet_en(ctx, dpi_video->is_18_loosely);
        bytes_per_pixel_x100 = 225;
        if (!dpi_video->is_18_loosely) {
        /*18bits per pixel and NOT loosely,packets should be multiples of 4*/
        //video_size_step = 4;
        /*round up active H pixels to a multiple of 4*/
            for (; (video_size % 4) != 0; video_size++) {
                ;
            }
        } else {
            video_size_step = 1;
        }
        break;
    case COLOR_CODE_24BIT:
        bytes_per_pixel_x100 = 300;
        video_size_step = 1;
        break;
    case COLOR_CODE_20BIT_YCC422_LOOSELY:
        bytes_per_pixel_x100 = 250;
        video_size_step = 2;
        /* round up active H pixels to a multiple of 2 */
        if ((video_size % 2) != 0) {
            video_size += 1;
        }
        break;
    case COLOR_CODE_24BIT_YCC422:
        bytes_per_pixel_x100 = 300;
        video_size_step = 2;
        /* round up active H pixels to a multiple of 2 */
        if ((video_size % 2) != 0) {
            video_size += 1;
        }
        break;
    case COLOR_CODE_16BIT_YCC422:
        bytes_per_pixel_x100 = 200;
        video_size_step = 2;
        /* round up active H pixels to a multiple of 2 */
        if ((video_size % 2) != 0) {
            video_size += 1;
        }
        break;
    case COLOR_CODE_30BIT:
        bytes_per_pixel_x100 = 375;
        video_size_step = 2;
        break;
    case COLOR_CODE_36BIT:
        bytes_per_pixel_x100 = 450;
        video_size_step = 2;
        break;
    case COLOR_CODE_12BIT_YCC420:
        bytes_per_pixel_x100 = 150;
        video_size_step = 2;
        /* round up active H pixels to a multiple of 2 */
        if ((video_size % 2) != 0) {
            video_size += 1;
        }
        break;
    case COLOR_CODE_DSC24:
        bytes_per_pixel_x100 = 300;
        video_size_step = 1;
        break;
    default:
        DISPERR("invalid color coding\n");
        return -1;
    }

    dsi_ops->dpi_color_coding(ctx, dpi_video->color_coding);
    dsi_ops->eotp_rx_en(ctx, dpi_video->eotp_rx_en);
    dsi_ops->eotp_tx_en(ctx, dpi_video->eotp_tx_en);
    dsi_ops->video_mode_lp_cmd_en(ctx, dpi_video->dpi_lp_cmd_en);
    dsi_ops->video_mode_mode_type(ctx, dpi_video->burst_mode);

    hsync_time = dpi_video->h_sync_pixels * ratio_clock_xPF / PRECISION_FACTOR;
    if (dpi_video->burst_mode != VIDEO_BURST_WITH_SYNC_PULSES)
        hsync_time += (ratio_clock_xPF -  3 * PRECISION_FACTOR / dpi_video->n_lanes) * dpi_video->h_active_pixels / PRECISION_FACTOR;
    dsi_ops->dpi_hsync_time(ctx, hsync_time); //hsync
    dsi_ops->dpi_hbp_time(ctx, dpi_video->h_back_porch_pixels * ratio_clock_xPF / PRECISION_FACTOR);//hbp
    dsi_ops->dpi_hline_time(ctx, dpi_video->h_total_pixels * ratio_clock_xPF / PRECISION_FACTOR); //hsync+hbp_hact+hfp

    dsi_ops->dpi_vsync(ctx, dpi_video->v_sync_lines);//vsync
    dsi_ops->dpi_vbp(ctx, dpi_video->v_back_porch_lines);//vbp
    dsi_ops->dpi_vfp(ctx, dpi_video->v_front_porch_lines);//vfp
    dsi_ops->dpi_vact(ctx, dpi_video->v_active_lines);//vact

    dsi_ops->dpi_hsync_pol(ctx, dpi_video->h_polarity);
    dsi_ops->dpi_vsync_pol(ctx, dpi_video->v_polarity);
    dsi_ops->dpi_data_en_pol(ctx, dpi_video->data_en_polarity);

    hs_timeout = (dpi_video->h_total_pixels * dpi_video->v_active_lines) +
        (DSIH_PIXEL_TOLERANCE * bytes_per_pixel_x100) / 100;
    for (counter = 0x80; (counter < hs_timeout) && (counter > 2); counter--) {
        if (hs_timeout % counter == 0) {
            dsi_ops->timeout_clock_division(ctx, counter + 1);
            dsi_ops->lp_rx_timeout(ctx, (uint16_t)(hs_timeout / counter));
            dsi_ops->hs_tx_timeout(ctx, (uint16_t)(hs_timeout / counter));
            break;
        }
    }
    dsi_ops->tx_escape_division(ctx, 6);

    /*video packetisation*/
    if (dpi_video->burst_mode == VIDEO_BURST_WITH_SYNC_PULSES) {
        /*BURST*/
        DISPMSG("INFO: burst video\n");
        dsi_ops->dpi_null_packet_size(ctx, 0);
        dsi_ops->dpi_chunk_num(ctx, 1);
        dsi_ops->dpi_video_packet_size(ctx, video_size);

        /*
         * BURST by default returns to LP during
         * ALL empty periods - energy saving
         */
        dsi_ops->dpi_hporch_lp_en(ctx, 1);
        dsi_ops->dpi_vporch_lp_en(ctx, 1);

        DISPMSG("INFO: h line time -> %d\n",
            (uint16_t)dpi_video->h_total_pixels);
        DISPMSG("INFO: video_size -> %d\n", video_size);
    } else {
        /*NON BURST*/
        DISPMSG("INFO: non burst video\n");

        null_packet_size = 0;
        /* Bytes to be sent - first as one chunk */
        bytes_per_chunk = bytes_per_pixel_x100 * dpi_video->h_active_pixels / 100
            + VIDEO_PACKET_OVERHEAD + NULL_PACKET_OVERHEAD;

        total_bytes = bytes_per_pixel_x100 * dpi_video->n_lanes *
            dpi_video->h_total_pixels / 100;

        if (total_bytes >= bytes_per_chunk) {
            chunk_overhead = total_bytes - bytes_per_chunk -
                VIDEO_PACKET_OVERHEAD - NULL_PACKET_OVERHEAD;
            DISPMSG("DSI INFO: overhead %d -> enable multi packets\n",
                chunk_overhead);

            if (!(chunk_overhead > 1)) {
                /* MULTI packets */
                DISPMSG("DSI INFO: multi packets\n");
                DISPMSG("DSI INFO: video_size -> %d\n", video_size);
                DISPMSG("DSI INFO: video_size_step -> %d\n", video_size_step);
                for (video_size = video_size_step;
                    video_size < dpi_video->h_active_pixels;
                    video_size += video_size_step) {
                    DISPMSG("DSI INFO: determine no of chunks\n");
                    DISPMSG("DSI INFO: video_size -> %d\n", video_size);
                    remain = (dpi_video->h_active_pixels * PRECISION_FACTOR /
                        video_size) % PRECISION_FACTOR;
                    DISPMSG("DSI INFO: remain -> %d", remain);
                    if (remain == 0) {
                        no_of_chunks = dpi_video->h_active_pixels / video_size;
                        DISPMSG("DSI INFO: no_of_chunks -> %d", no_of_chunks);
                        bytes_per_chunk =  bytes_per_pixel_x100 *
                            video_size / 100 + VIDEO_PACKET_OVERHEAD;
                        DISPMSG("DSI INFO: bytes_per_chunk -> %d\n",
                            bytes_per_chunk);
                        if (total_bytes >= (bytes_per_chunk * no_of_chunks)) {
                            bytes_left = total_bytes -
                                (bytes_per_chunk * no_of_chunks);
                            DISPMSG("DSI INFO: bytes_left -> %d", bytes_left);
                            break;
                        }
                    }
                }
                if (bytes_left > (NULL_PACKET_OVERHEAD * no_of_chunks)) {
                    null_packet_size = (bytes_left -
                        (NULL_PACKET_OVERHEAD * no_of_chunks)) / no_of_chunks;
                    if (null_packet_size > MAX_NULL_SIZE) {
                        /* avoid register overflow */
                        null_packet_size = MAX_NULL_SIZE;
                    }
                } else {
                    /* no multi packets */
                    no_of_chunks = 1;

                    DISPMSG("DSI INFO: no multi packets\n");
                    DISPMSG("DSI INFO: horizontal line time -> %d\n",
                             (uint16_t)((dpi_video->h_total_pixels *
                             ratio_clock_xPF) / PRECISION_FACTOR));
                    DISPMSG("DSI INFO: video size -> %d\n", video_size);

                    /* video size must be a multiple of 4 when not 18 loosely */
                    for (video_size = dpi_video->h_active_pixels;
                         (video_size % video_size_step) != 0;
                         video_size++)
                        ;
                }
            }
            dsi_ops->dpi_null_packet_size(ctx, null_packet_size);
            dsi_ops->dpi_chunk_num(ctx, no_of_chunks);
            dsi_ops->dpi_video_packet_size(ctx, video_size);
        } else {
            DISPERR("resolution cannot be sent to display through current settings\n");
            error = -1;
        }
    }

    dsi_ops->video_vcid(ctx, dpi_video->virtual_channel);
    dsi_ops->dphy_n_lanes(ctx, dpi_video->n_lanes);

    /*enable high speed clock*/
    //dsi_ops->dphy_enable_hs_clk(ctx, 1);//panel not init yet, could not set to hs

    return error;
}

static int mipi_dsih_gen_wr_packet(struct dsi_context *ctx, uint8_t vc,
    uint8_t data_type, uint8_t *params, uint16_t param_length)
{
    struct dsi_core_ops *dsi_ops = ctx->ops;
    uint32_t temp = 0;
    int timeout = 0;
    int i = 0;
    int j = 0;

    if (ctx == NULL) {
        DISPERR("Null ctx\n");
        return -1;
    }

    if ((params == NULL) && (param_length != 0)) {
        DISPERR("Null params\n");
        return -1;
    }

    if (param_length > 200) {
        DISPERR("param length too large\n");
        return -1;
    }

    if (param_length > 2) {
        /*Long Parcket -- write word count to header and the rest to payload*/
        for (i = 0; i < param_length; i += j) {
            temp = 0;
            for (j = 0; (j < 4) && ((i + j) < param_length); j++) {
                /* temp = (payload[i + 3] << 24) | (payload[i + 2] << 16) |
                (payload[i + 1] << 8) | payload[i]; */
                temp |= params[i + j] << (j * 8) ;
            }
            /*check if tx payload fifo is not full*/
            for (timeout = 0; timeout < DSIH_FIFO_ACTIVE_WAIT; timeout++) {
                if (!mipi_dsih_gen_packet_payload(ctx, temp))
                    break;
                udelay(500 * 1000);
            }

            if (timeout >= DSIH_FIFO_ACTIVE_WAIT) {
                DISPERR("set payload timeout!\n");
                return -1;
            }
        }

    }

    for (timeout = 0; timeout < DSIH_FIFO_ACTIVE_WAIT; timeout++) {
        /*check if tx command fifo is not full*/
        if (dsi_ops->is_tx_cmd_fifo_empty(ctx)) {
            if (param_length == 0) {
                dsi_ops->set_packet_header(ctx, vc, data_type, 0x0, 0x0);
            } else if (param_length == 1) {
                dsi_ops->set_packet_header(ctx, vc, data_type, 0x0, params[0]);
            } else if (param_length == 2) {
                dsi_ops->set_packet_header(ctx, vc, data_type, params[1],
                    params[0]);
            } else {
                dsi_ops->set_packet_header(ctx, vc, data_type,
                    param_length >> 8, param_length & 0xFF);
            }
            break;
        }
    }
    if (timeout >= DSIH_FIFO_ACTIVE_WAIT) {
        DISPERR("set header timeout!\n");
        return -1;
    }

    return 0;
}

int mipi_dsih_gen_wr_cmd(struct dsi_context *ctx, uint8_t vc,
    uint8_t *params, uint16_t param_length)
{
    uint8_t data_type = 0;

    if (ctx == NULL) {
        DISPERR("Null ctx\n");
        return -1;
    }

    switch (param_length) {
    case 0:
        data_type = 0x03;
        break;
    case 1:
        data_type = 0x13;
        break;
    case 2:
        data_type = 0x23;
        break;
    default:
        data_type = 0x29;
        break;
    }

    return mipi_dsih_gen_wr_packet(ctx, vc, data_type, params, param_length);
}

static int mipi_dsih_gen_rd_packet(struct dsi_context *ctx,
    uint8_t vc, uint8_t data_type, uint8_t msb_byte, uint8_t lsb_byte,
    uint8_t bytes_to_read, uint8_t *read_buffer)
{
    int i = 0;
    int timeout;
    int counter = 0;
    int last_count = 0;
    uint32_t temp[1] = {0};
    struct dsi_core_ops *dsi_ops = ctx->ops;

    /*make sure command mode is on*/
    //dsi_ops->cmd_mode(ctx);
    /*make sure receiving is enabled*/
    dsi_ops->bta_en(ctx, 1);
    /*listen to the same virtual channel as the one sent to*/
    dsi_ops->rx_vcid(ctx, vc);

    for (timeout = 0; timeout < DSIH_FIFO_ACTIVE_WAIT; timeout++) {
        if (!dsi_ops->is_tx_cmd_fifo_full(ctx)) {
            dsi_ops->set_packet_header(ctx, vc, data_type, msb_byte, lsb_byte);
            break;
        }
    }
    if (timeout >= DSIH_FIFO_ACTIVE_WAIT) {
        DISPERR("Tx Read command timeout\n");
        return -1;
    }

    /*loop for the number of words to be read*/
    for (timeout = 0; timeout < DSIH_FIFO_ACTIVE_WAIT; timeout++) {
        /*check if command transaction is done*/
        if (dsi_ops->is_bta_returned(ctx)) {
            if (!dsi_ops->is_rx_payload_fifo_empty(ctx)) {
                for (counter = 0; (!dsi_ops->is_rx_payload_fifo_empty(ctx));
                    counter +=4) {
                    dsi_ops->get_rx_payload(ctx, temp);
                    if (counter < bytes_to_read) {
                        for (i = 0; i < 4; i++) {
                            if ((counter + i) < bytes_to_read) {
                                /*put 32 bit temp in 4 bytes of buffer passed by user*/
                                read_buffer[counter + i] = (uint8_t)(temp[0] >> (i * 8));
                                last_count = i + counter;
                            } else {
                                if ((uint8_t)(temp[0] >> (i * 8)) != 0x00)
                                    last_count = i + counter;
                            }
                       }

                    } else {
                        last_count = counter;
                        for (i = 0; i < 4; i++) {
                            if ((uint8_t)(temp[0] >> (i * 8)) != 0x00)
                                last_count = i + counter;
                        }
                    }
                }
                    return last_count + 1;
            } else {
                DISPERR("Rx buffer empty\n");
                return 0;
            }
        }
    }
    DISPERR("Rx command timeout\n");
    return -1;
}

uint16_t mipi_dsih_gen_rd_cmd(struct dsi_context *ctx, uint8_t vc,
    uint8_t *params, uint16_t param_length, uint8_t bytes_to_read,
    uint8_t *read_buffer)
{
    uint8_t data_type = 0;

    if (ctx == NULL) {
        DISPERR("Null ctx\n");
        return -1;
    }

    switch (param_length) {
    case 0:
        data_type = 0x04;
        return mipi_dsih_gen_rd_packet(ctx, vc, data_type, 0x00, 0x00,
            bytes_to_read, read_buffer);
    case 1:
        data_type = 0x14;
        return mipi_dsih_gen_rd_packet(ctx, vc, data_type, 0x00, params[0],
            bytes_to_read, read_buffer);
    case 2:
        data_type = 0x24;
        return mipi_dsih_gen_rd_packet(ctx, vc, data_type, params[1], params[0],
            bytes_to_read, read_buffer);
    default:
        return 0;
    }
}

int mipi_dsih_dcs_rd_cmd(struct dsi_context *ctx, uint8_t vc, uint8_t command,
    uint8_t bytes_to_read, uint8_t *read_buffer)
{
    if (ctx == NULL) {
        DISPERR("Null ctx\n");
        return -1;
    }

    switch (command) {
    case 0xA8:
    case 0xA1:
    case 0x45:
    case 0x3E:
    case 0x2E:
    case 0x0F:
    case 0x0E:
    case 0x0D:
    case 0x0C:
    case 0x0B:
    case 0x0A:
    case 0x08:
    case 0x07:
    case 0x06:
        /*COMMAND_TYPE 0x06 - DCS Read no params refer to DSI spec p.47*/
        return mipi_dsih_gen_rd_packet(ctx, vc, 0x06, 0x00, command,
            bytes_to_read, read_buffer);
    default:
        DISPERR("invalid DCS command 0x%x\n", command);
        return -1;
    }

    return 0;
}

int mipi_dsih_dcs_wr_cmd(struct dsi_context *ctx, uint8_t vc,
    uint8_t *params, uint16_t param_length)
{
    uint8_t packet_type = 0;

    if (ctx == NULL) {
        DISPERR("Null ctx\n");
        return -1;
    }

    switch (param_length) {
    case 1:
        packet_type = 0x05; /* DCS short write no param */
        break;
    case 2:
        packet_type = 0x15; /* DCS short write 1 param */
        break;
    default:
        packet_type = 0x39; /* DCS long write/write_LUT command packet */
        break;
    }

    return mipi_dsih_gen_wr_packet(ctx, vc, packet_type, params, param_length);
}
