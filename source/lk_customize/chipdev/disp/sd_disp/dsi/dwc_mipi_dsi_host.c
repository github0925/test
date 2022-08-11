/*
* dwc_mipi_dsi_host.c
*
* Semidrive platform drm driver
*
* Copyright (C) 2019, Semidrive  Communications Inc.
*
* This file is licensed under a dual GPLv2 or X11 license.
*/
#include <disp_general_header.h>
#include <disp_dsi.h>
#include <dsi_reg.h>
#include <reg.h>
#include <disp_drv_log.h>

#define DSI_VESION_1_40A 0x3134302A

#undef BIT
#define BIT(nr) (1U << (nr))

static inline unsigned int reg_value(unsigned int val,
    unsigned int src, unsigned int shift, unsigned int mask)
{
    return (src & ~mask) | ((val << shift) & mask);
}

static inline unsigned int disp_read(unsigned long base, unsigned int reg)
{
    return readl(base + reg);
}

static inline void disp_write(unsigned long base,
        unsigned int reg, unsigned int value)
{
    writel(value, base + reg);

#if 0
    DISPDBG("w(0x%lx, 0x%08x), r(0x%08x)\n", (base + reg),
            value, readl(base + reg));
#endif
}

/**
* Get DSI Host core version
* @param instance pointer to structure holding the DSI Host core information
* @return ascii number of the version
*/
static bool dsi_check_version(struct dsi_context *ctx)
{
    addr_t base = ctx->base;

    /* DWC_mipi_dsi_host_1.140 */
    if (disp_read(base, MIPI_DSI_VERSION) == DSI_VESION_1_40A)
        return true;

    return false;
}

/**
* Modify power status of DSI Host core
* @param instance pointer to structure holding the DSI Host core information
* @param on (1) or off (0)
*/
static void dsi_power_enable(struct dsi_context *ctx, int enable)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_PWR_UP, enable);
}

/**
* Get the power status of the DSI Host core
* @param dev pointer to structure holding the DSI Host core information
* @return power status
*/
static uint8_t dsi_get_power_status(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_PWR_UP)
        & PWR_UP_SHUTDOWNZ_MASK)
        >> PWR_UP_SHUTDOWNZ_SHIFT;

    return val;
}

/* PRESP Time outs */
/**
* configure timeout divisions (so they would have more clock ticks)
* @param instance pointer to structure holding the DSI Host core information
* @param div no of hs cycles before transiting back to LP in
*  (lane_clk / div)
*/
static void dsi_timeout_clock_division(struct dsi_context *ctx,
    uint8_t div)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_CLKMSG_CFG);
    val = reg_value(div, val, TO_CLK_DIVISION_SHIFT, TO_CLK_DIVISION_MASK);

    disp_write(base, MIPI_DSI_CLKMSG_CFG, val);
}

/**
* Write transmission escape timeout
* a safe guard so that the state machine would reset if transmission
* takes too long
* @param instance pointer to structure holding the DSI Host core information
* @param div
*/
static void dsi_tx_escape_division(struct dsi_context *ctx,
    uint8_t division)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_CLKMSG_CFG);
    val = reg_value(division, val,
        TX_ESC_CLK_DIVISION_SHIFT,
        TX_ESC_CLK_DIVISION_MASK);

    disp_write(base, MIPI_DSI_CLKMSG_CFG, val);
}

/**
* Write the DPI video virtual channel destination
* @param instance pointer to structure holding the DSI Host core information
* @param vc virtual channel
*/
static void dsi_video_vcid(struct dsi_context *ctx, uint8_t vc)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_DPI_VCID, vc);
}

/**
 * Get the DPI video virtual channel destination
 * @param dev pointer to structure holding the DSI Host core information
 * @return virtual channel
 */
static uint8_t dsi_get_video_vcid(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_DPI_VCID)
        & DPI_VCID_MASK)
        >> DPI_VCID_SHIFT;

    return val;
}

/**
* Set DPI video color coding
* @param instance pointer to structure holding the DSI Host core information
* @param color_coding enum (configuration and color depth)
* @return error code
*/
static void dsi_dpi_color_coding(struct dsi_context *ctx, int coding)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_DPI_COLOR_CODING);
    val = reg_value(coding, val, DPI_COLOR_CODING_SHIFT, DPI_COLOR_CODING_MASK);

    disp_write(base, MIPI_DSI_DPI_COLOR_CODING, val);
}

/**
 * Get DPI video color coding
 * @param dev pointer to structure holding the DSI Host core information
 * @return color coding enum (configuration and color depth)
 */
static uint8_t dsi_dpi_get_color_coding(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_DPI_COLOR_CODING)
        & DPI_COLOR_CODING_MASK)
        >> DPI_COLOR_CODING_SHIFT;

    return val;
}

/**
 * Get DPI video color depth
 * @param dev pointer to structure holding the DSI Host core information
 * @return number of bits per pixel
 */
static uint8_t dsi_dpi_get_color_depth(struct dsi_context *ctx)
{
	uint8_t color_depth = 0;
	switch (dsi_dpi_get_color_coding(ctx))
	{
		case 0:
		case 1:
		case 2:
			color_depth = 16;
			break;
		case 3:
		case 4:
			color_depth = 18;
			break;
		case 5:
			color_depth = 24;
			break;
		case 6:
			color_depth = 20;
			break;
		case 7:
			color_depth = 24;
			break;
		case 8:
			color_depth = 16;
			break;
		case 9:
			color_depth = 30;
			break;
		case 10:
			color_depth = 36;
			break;
		case 11:
			color_depth = 12;
			break;
		default:
			break;
	}
	return color_depth;
}

/**
 * Get DPI video pixel configuration
 * @param dev pointer to structure holding the DSI Host core information
 * @return pixel configuration
 */
static uint8_t dsi_dpi_get_color_config(struct dsi_context *ctx)
{
	uint8_t color_config = 0;
	switch (dsi_dpi_get_color_coding(ctx)) {
		case 0:
			color_config = 1;
			break;
		case 1:
			color_config = 2;
			break;
		case 2:
			color_config = 3;
			break;
		case 3:
			color_config = 1;
			break;
		case 4:
			color_config = 2;
			break;
		default:
			color_config = 0;
			break;
	}
	return color_config;
}

/**
* Set DPI loosely packetisation video (used only when color depth = 18
* @param instance pointer to structure holding the DSI Host core information
* @param enable
*/
static void dsi_dpi_18_loosely_packet_en(struct dsi_context *ctx,
			int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_DPI_COLOR_CODING);
    val = reg_value(enable, val, LOOSELY18_EN_SHIFT, LOOSELY18_EN_MASK);

    disp_write(base, MIPI_DSI_DPI_COLOR_CODING, val);
}

/*
* Set DPI color mode pin polarity
* @param instance pointer to structure holding the DSI Host core information
* @param active_low (1) or active high (0)
*/
static void dsi_dpi_color_mode_pol(struct dsi_context *ctx,
			int active_low)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_DPI_CFG_POL);
    val = reg_value(active_low, val,
        COLORM_ACTIVE_LOW_SHIFT,
        COLORM_ACTIVE_LOW_MASK);

    disp_write(base, MIPI_DSI_DPI_CFG_POL, val);
}

/*
* Set DPI shut down pin polarity
* @param instance pointer to structure holding the DSI Host core information
* @param active_low (1) or active high (0)
*/
static void dsi_dpi_shut_down_pol(struct dsi_context *ctx,
			int active_low)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_DPI_CFG_POL);
    val = reg_value(active_low, val,
        SHUTD_ACTIVE_LOW_SHIFT,
        SHUTD_ACTIVE_LOW_MASK);

    disp_write(base, MIPI_DSI_DPI_CFG_POL, val);
}

/*
* Set DPI horizontal sync pin polarity
* @param instance pointer to structure holding the DSI Host core information
* @param active_low (1) or active high (0)
*/
static void dsi_dpi_hsync_pol(struct dsi_context *ctx, int active_low)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_DPI_CFG_POL);
    val = reg_value(active_low, val,
        HSYNC_ACTIVE_LOW_SHIFT,
        HSYNC_ACTIVE_LOW_MASK);

    disp_write(base, MIPI_DSI_DPI_CFG_POL, val);
}

/*
* Set DPI vertical sync pin polarity
* @param instance pointer to structure holding the DSI Host core information
* @param active_low (1) or active high (0)
*/
static void dsi_dpi_vsync_pol(struct dsi_context *ctx, int active_low)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_DPI_CFG_POL);
    val = reg_value(active_low, val,
        VSYNC_ACTIVE_LOW_SHIFT,
        VSYNC_ACTIVE_LOW_MASK);

    disp_write(base, MIPI_DSI_DPI_CFG_POL, val);
}

/*
* Set DPI data enable pin polarity
* @param instance pointer to structure holding the DSI Host core information
* @param active_low (1) or active high (0)
*/
static void dsi_dpi_data_en_pol(struct dsi_context *ctx, int active_low)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_DPI_CFG_POL);
    val = reg_value(active_low, val,
        DATAEN_ACTIVE_LOW_SHIFT,
        DATAEN_ACTIVE_LOW_MASK);

    disp_write(base, MIPI_DSI_DPI_CFG_POL, val);
}

/**
* Enable EOTp reception
* @param instance pointer to structure holding the DSI Host core information
* @param enable
*/
static void dsi_eotp_rx_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PCKHDL_CFG);
    val = reg_value(enable, val, EOTP_RX_EN_SHIFT, EOTP_RX_EN_MASK);

    disp_write(base, MIPI_DSI_PCKHDL_CFG, val);
}

/**
* Enable EOTp transmission
* @param instance pointer to structure holding the DSI Host core information
* @param enable
*/
static void dsi_eotp_tx_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PCKHDL_CFG);
    val = reg_value(enable, val, EOTP_TX_EN_SHIFT, EOTP_TX_EN_MASK);

    disp_write(base, MIPI_DSI_PCKHDL_CFG, val);
}

/**
* Enable Bus Turn-around request
* @param instance pointer to structure holding the DSI Host core information
* @param enable
*/
static void dsi_bta_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PCKHDL_CFG);
    val = reg_value(enable, val, BTA_EN_SHIFT, BTA_EN_MASK);

    disp_write(base, MIPI_DSI_PCKHDL_CFG, val);
}

/**
* Enable ECC reception, error correction and reporting
* @param instance pointer to structure holding the DSI Host core information
* @param enable
*/
static void dsi_ecc_rx_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PCKHDL_CFG);
    val = reg_value(enable, val, ECC_RX_EN_SHIFT, ECC_RX_EN_MASK);

    disp_write(base, MIPI_DSI_PCKHDL_CFG, val);
}

/**
* Enable CRC reception, error reporting
* @param instance pointer to structure holding the DSI Host core information
* @param enable
*/
static void dsi_crc_rx_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PCKHDL_CFG);
    val = reg_value(enable, val, CRC_RX_EN_SHIFT, CRC_RX_EN_MASK);

    disp_write(base, MIPI_DSI_PCKHDL_CFG, val);
}

/**
* Enable the EoTp transmission in low-Power
* @param instance pointer to structure holding the DSI Host core information
* @param enable
*/
static void dsi_eotp_tx_lp_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PCKHDL_CFG);
    val = reg_value(enable, val, EOTP_TX_LP_EN_SHIFT, EOTP_TX_LP_EN_MASK);

    disp_write(base, MIPI_DSI_PCKHDL_CFG, val);
}

/**
* Configure the read back virtual channel for the generic interface
* @param instance pointer to structure holding the DSI Host core information
* @param vc to listen to on the line
*/
static void dsi_rx_vcid(struct dsi_context *ctx, uint8_t vc)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_GEN_VCID);
    val = reg_value(vc, val, GEN_VCID_RX_SHIFT, GEN_VCID_RX_MASK);

    disp_write(base, MIPI_DSI_GEN_VCID, val);
}

/**
* Enable/disable DPI video mode
* @param instance pointer to structure holding the DSI Host core information
*/
static void dsi_video_mode(struct dsi_context *ctx)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_MODE_CFG, 0);
}

/**
* Enable command mode (Generic interface)
* @param instance pointer to structure holding the DSI Host core information
*/
static void dsi_cmd_mode(struct dsi_context *ctx)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_MODE_CFG, 1);
}

/**
* Get the status of video mode, whether enabled or not in core
* @param dev pointer to structure holding the DSI Host core information
* @return status
*/
static bool dsi_is_cmd_mode(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_MODE_CFG)
        & CMD_VIDEO_MODE_MASK)
        >> CMD_VIDEO_MODE_SHIFT;

    return val;
}

/**
* Set DCS read command packet transmission to transmission type
* @param instance pointer to structure holding the DSI Host core information
* @param no_of_param of command
* @param lp transmit in low power
* @return error code
*/
static void dsi_video_mode_lp_cmd_en(struct dsi_context *ctx,
    int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, LP_CMD_EN_SHIFT, LP_CMD_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
* Enable FRAME BTA ACK
* @param instance pointer to structure holding the DSI Host core information
* @param enable (1) - disable (0)
*/
static void dsi_video_mode_frame_ack_en(struct dsi_context *ctx,
    int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val,
        FRAME_BTA_ACK_EN_SHIFT,
        FRAME_BTA_ACK_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
 * Enable return to low power mode inside horizontal front porch periods when
 *  timing allows
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
static void dsi_video_mode_lp_hfp_en(struct dsi_context *ctx,
    int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, LP_HFP_EN_SHIFT, LP_HFP_EN_SHIFT);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
 * Enable return to low power mode inside horizontal back porch periods when
 *  timing allows
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
static void dsi_video_mode_lp_hbp_en(struct dsi_context *ctx,
    int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, LP_HBP_EN_SHIFT, LP_HBP_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
 * Enable return to low power mode inside vertical active lines periods when
 *  timing allows
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
static void dsi_video_mode_lp_vact_en(struct dsi_context *ctx,
    int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, LP_VACT_EN_SHIFT, LP_VACT_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
 * Enable return to low power mode inside vertical front porch periods when
 *  timing allows
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
static void dsi_video_mode_lp_vfp_en(struct dsi_context *ctx,
    int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, LP_VFP_EN_SHIFT, LP_VFP_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
 * Enable return to low power mode inside vertical back porch periods when
 * timing allows
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
static void dsi_video_mode_lp_vbp_en(struct dsi_context *ctx,
    int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, LP_VBP_EN_SHIFT, LP_VBP_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
 * Enable return to low power mode inside vertical sync periods when
 *  timing allows
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable (1) - disable (0)
 */
static void dsi_video_mode_lp_vsa_en(struct dsi_context *ctx,
    int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, LP_VSA_EN_SHIFT, LP_VSA_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
* Enable return to low power mode inside horizontal front porch periods when
*  timing allows
* @param instance pointer to structure holding the DSI Host core information
* @param enable (1) - disable (0)
*/
static void dsi_dpi_hporch_lp_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, LP_HFP_EN_SHIFT, LP_HFP_EN_MASK);
    val = reg_value(enable, val, LP_HBP_EN_SHIFT, LP_HBP_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
* Enable return to low power mode inside vertical active lines periods when
*  timing allows
* @param instance pointer to structure holding the DSI Host core information
* @param enable (1) - disable (0)
*/
static void dsi_dpi_vporch_lp_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, LP_VACT_EN_SHIFT, LP_VACT_EN_MASK);
    val = reg_value(enable, val, LP_VFP_EN_SHIFT, LP_VFP_EN_MASK);
    val = reg_value(enable, val, LP_VBP_EN_SHIFT, LP_VBP_EN_MASK);
    val = reg_value(enable, val, LP_VSA_EN_SHIFT, LP_VSA_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
* Set DPI video mode type (burst/non-burst - with sync pulses or events)
* @param instance pointer to structure holding the DSI Host core information
* @param type
* @return error code
*/
static void dsi_video_mode_mode_type(struct dsi_context *ctx, int mode)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(mode, val, VID_MODE_TYPE_SHIFT, VID_MODE_TYPE_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
 * Change Pattern orientation
 * @param dev pointer to structure holding the DSI Host core information
 * @param orientation choose between horizontal or vertical pattern
 */
static void dsi_vpg_orientation_act(struct dsi_context *ctx,
    uint8_t orientation)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(orientation, val,
        VPG_ORIENTATION_SHIFT,
        VPG_ORIENTATION_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
 * Change Pattern Type
 * @param dev pointer to structure holding the DSI Host core information
 * @param mode choose between normal or BER pattern
 */
static void dsi_vpg_mode_act(struct dsi_context *ctx, uint8_t mode)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(mode, val, VPG_MODE_SHIFT, VPG_MODE_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
 * Change Video Pattern Generator
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable enable video pattern generator
 */
static void dsi_enable_vpg_act(struct dsi_context *ctx, uint8_t enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_MODE_CFG);
    val = reg_value(enable, val, VPG_EN_SHIFT, VPG_EN_MASK);

    disp_write(base, MIPI_DSI_VID_MODE_CFG, val);
}

/**
* Write video packet size. obligatory for sending video
* @param instance pointer to structure holding the DSI Host core information
* @param size of video packet - containing information
* @return error code
*/
static void dsi_dpi_video_packet_size(struct dsi_context *ctx,
    uint16_t size)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_PKT_SIZE, size);
}

/*
* Write no of chunks to core - taken into consideration only when multi packet
* is enabled
* @param instance pointer to structure holding the DSI Host core information
* @param no of chunks
*/
static void dsi_dpi_chunk_num(struct dsi_context *ctx, uint16_t num)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_NUM_CHUNKS, num);
}

/**
* Write the null packet size - will only be taken into account when null
* packets are enabled.
* @param instance pointer to structure holding the DSI Host core information
* @param size of null packet
* @return error code
*/
static void dsi_dpi_null_packet_size(struct dsi_context *ctx,
    uint16_t size)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_NULL_SIZE, size);
}

/**
* Configure the Horizontal sync time
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle taken to transmit the horizontal sync
*/
static void dsi_dpi_hsync_time(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_HSA_TIME, byte_cycle);
}

/**
* Configure the Horizontal back porch time
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle taken to transmit the horizontal back porch
*/
static void dsi_dpi_hbp_time(struct dsi_context *ctx, uint16_t byte_cycle)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_HBP_TIME, byte_cycle);
}

/*
* Configure the Horizontal Line time
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle taken to transmit the total of the horizontal line
*/
static void dsi_dpi_hline_time(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_HLINE_TIME, byte_cycle);
}

/**
* Configure the vertical sync lines of the video stream
* @param instance pointer to structure holding the DSI Host core information
* @param lines
*/
static void dsi_dpi_vsync(struct dsi_context *ctx, uint16_t lines)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_VSA_LINES, lines);
}

/**
* Configure the vertical back porch lines of the video stream
* @param instance pointer to structure holding the DSI Host core information
* @param lines
*/
static void dsi_dpi_vbp(struct dsi_context *ctx, uint16_t lines)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_VBP_LINES, lines);
}

/**
* Configure the vertical front porch lines of the video stream
* @param instance pointer to structure holding the DSI Host core information
* @param lines
*/
static void dsi_dpi_vfp(struct dsi_context *ctx, uint16_t lines)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_VFP_LINES, lines);
}

/**
* Configure the vertical active lines of the video stream
* @param instance pointer to structure holding the DSI Host core information
* @param lines
*/
static void dsi_dpi_vact(struct dsi_context *ctx, uint16_t lines)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_VID_VACTIVE_LINES, lines);
}

/**
* Enable tear effect acknowledge
* @param instance pointer to structure holding the DSI Host core information
* @param enable (1) - disable (0)
*/
static void dsi_tear_effect_ack_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_CMD_MODE_CFG);
    val = reg_value(enable, val, TEAR_FX_EN_SHIFT, TEAR_FX_EN_MASK);

    disp_write(base, MIPI_DSI_CMD_MODE_CFG, val);
}

/**
* Enable packets acknowledge request after each packet transmission
* @param instance pointer to structure holding the DSI Host core information
* @param enable (1) - disable (0)
*/
static void dsi_cmd_ack_request_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_CMD_MODE_CFG);
    val = reg_value(enable, val, ACK_RQST_EN_SHIFT, ACK_RQST_EN_MASK);

    disp_write(base, MIPI_DSI_CMD_MODE_CFG, val);
}

/**
* Set DCS&Generic command packet transmission to transmission type
* @param instance pointer to structure holding the DSI Host core information
* @param no_of_param of command
* @param lp transmit in low power
* @return error code
*/
static void dsi_cmd_mode_lp_cmd_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_CMD_MODE_CFG);
    val = reg_value(enable, val, GEN_SW_0P_TX_SHIFT, GEN_SW_0P_TX_MASK);
    val = reg_value(enable, val, GEN_SW_1P_TX_SHIFT, GEN_SW_1P_TX_MASK);
    val = reg_value(enable, val, GEN_SW_2P_TX_SHIFT, GEN_SW_2P_TX_MASK);
    val = reg_value(enable, val, GEN_LW_TX_SHIFT, GEN_LW_TX_MASK);
    val = reg_value(enable, val, DCS_SW_0P_TX_SHIFT, DCS_SW_0P_TX_MASK);
    val = reg_value(enable, val, DCS_SW_1P_TX_SHIFT, DCS_SW_1P_TX_MASK);
    val = reg_value(enable, val, DCS_LW_TX_SHIFT, DCS_LW_TX_MASK);
    val = reg_value(enable, val, MAX_RD_PKT_SIZE_SHIFT, MAX_RD_PKT_SIZE_MASK);

    val = reg_value(enable, val, GEN_SR_0P_TX_SHIFT, GEN_SR_0P_TX_MASK);
    val = reg_value(enable, val, GEN_SR_1P_TX_SHIFT, GEN_SR_1P_TX_MASK);
    val = reg_value(enable, val, GEN_SR_2P_TX_SHIFT, GEN_SR_2P_TX_MASK);
    val = reg_value(enable, val, DCS_SR_0P_TX_SHIFT, DCS_SR_0P_TX_MASK);

    disp_write(base, MIPI_DSI_CMD_MODE_CFG, val);
}

/**
* Write command header in the generic interface
* (which also sends DCS commands) as a subset
* @param instance pointer to structure holding the DSI Host core information
* @param vc of destination
* @param packet_type (or type of DCS command)
* @param ls_byte (if DCS, it is the DCS command)
* @param ms_byte (only parameter of short DCS packet)
* @return error code
*/
static void dsi_set_packet_header(struct dsi_context *ctx,
    uint8_t vc,
    uint8_t type,
    uint8_t wc_lsb,
    uint8_t wc_msb)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_GEN_HDR);
    val = reg_value(type, val, GEN_DT_SHIFT, GEN_DT_MASK);
    val = reg_value(vc, val, GEN_VC_SHIFT, GEN_VC_MASK);
    val = reg_value(wc_lsb, val, GEN_WC_LSBYTE_SHIFT, GEN_WC_LSBYTE_MASK);
    val = reg_value(wc_msb, val, GEN_WC_MSBYTE_SHIFT, GEN_WC_MSBYTE_MASK);

    disp_write(base, MIPI_DSI_GEN_HDR, val);
}

/**
* Write the payload of the long packet commands
* @param instance pointer to structure holding the DSI Host core information
* @param payload array of bytes of payload
* @return error code
*/
static void dsi_set_packet_payload(struct dsi_context *ctx,
    uint32_t payload)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_GEN_PLD_DATA, payload);
}

/**
* Write the payload of the long packet commands
* @param instance pointer to structure holding the DSI Host core information
* @param payload pointer to 32-bit array to hold read information
* @return error code
*/
static void dsi_get_rx_payload(struct dsi_context *ctx,
    uint32_t *payload)
{
    addr_t base = ctx->base;

    *payload = disp_read(base, MIPI_DSI_GEN_PLD_DATA);
}


/**
 * Get status of read command
 * @param dev pointer to structure holding the DSI Host core information
 * @return 1 if busy
 */
/*static uint8_t dsi_rd_cmd_busy(struct dsi_context *ctx)
{
	struct dsi_reg *reg = (struct dsi_reg *)ctx->base;
	union _0x74 cmd_pkt_status;

	cmd_pkt_status.val = dsi_read(&reg->CMD_PKT_STATUS);

	return cmd_pkt_status.bits.gen_rd_cmd_busy;
}*/

/**
* Get status of read command
* @param instance pointer to structure holding the DSI Host core information
* @return 1 if busy
*/
static bool dsi_is_bta_returned(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_CMD_PKT_STATUS)
        & GEN_RD_CMD_BUSY_MASK)
        >> GEN_RD_CMD_BUSY_SHIFT;

    return !val;
}

/**
* Get the FULL status of generic read payload fifo
* @param instance pointer to structure holding the DSI Host core information
* @return 1 if fifo full
*/
static bool dsi_is_rx_payload_fifo_full(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_CMD_PKT_STATUS)
        & GEN_PLD_R_FULL_MASK)
        >> GEN_PLD_R_FULL_SHIFT;

    return val;
}

/**
* Get the EMPTY status of generic read payload fifo
* @param instance pointer to structure holding the DSI Host core information
* @return 1 if fifo empty
*/
static bool dsi_is_rx_payload_fifo_empty(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_CMD_PKT_STATUS)
        & GEN_PLD_R_EMPTY_MASK)
        >> GEN_PLD_R_EMPTY_SHIFT;

    return val;
}

/**
* Get the FULL status of generic write payload fifo
* @param instance pointer to structure holding the DSI Host core information
* @return 1 if fifo full
*/
static bool dsi_is_tx_payload_fifo_full(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_CMD_PKT_STATUS)
        & GEN_PLD_W_FULL_MASK)
        >> GEN_PLD_W_FULL_SHIFT;

    return val;
}

/**
* Get the EMPTY status of generic write payload fifo
* @param instance pointer to structure holding the DSI Host core information
* @return 1 if fifo empty
*/
static bool dsi_is_tx_payload_fifo_empty(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_CMD_PKT_STATUS)
        & GEN_PLD_W_EMPTY_MASK)
        >> GEN_PLD_W_EMPTY_SHIFT;

    return val;
}

/**
* Get the FULL status of generic command fifo
* @param instance pointer to structure holding the DSI Host core information
* @return 1 if fifo full
*/
static bool dsi_is_tx_cmd_fifo_full(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_CMD_PKT_STATUS)
        & GEN_CMD_FULL_MASK)
        >> GEN_CMD_FULL_SHIFT;

    return val;
}

/**
* Get the EMPTY status of generic command fifo
* @param instance pointer to structure holding the DSI Host core information
* @return 1 if fifo empty
*/
static bool dsi_is_tx_cmd_fifo_empty(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_CMD_PKT_STATUS)
        & GEN_CMD_EMPTY_MASK)
        >> GEN_CMD_EMPTY_SHIFT;

    return val;
}

/**
* Configure the Low power receive time out
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle (of byte cycles)
*/
static void dsi_lp_rx_timeout(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_TO_CNT_CFG);
    val = reg_value(byte_cycle, val, LPRX_TO_CNT_SHIFT, LPRX_TO_CNT_MASK);

    disp_write(base, MIPI_DSI_TO_CNT_CFG, val);
}

/**
* Configure a high speed transmission time out
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle (byte cycles)
*/
static void dsi_hs_tx_timeout(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_TO_CNT_CFG);
    val = reg_value(byte_cycle, val, HSTX_TO_CNT_SHIFT, HSTX_TO_CNT_MASK);

    disp_write(base, MIPI_DSI_TO_CNT_CFG, val);
}

/**
* Timeout for peripheral between HS data transmission read requests
* @param instance pointer to structure holding the DSI Host core information
* @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
* link still, after sending a high-speed read operation. This period is
* measured in cycles of lanebyteclk
*/
static void dsi_hs_read_presp_timeout(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_HS_RD_TO_CNT, byte_cycle);
}

/**
* Timeout for peripheral (for controller to stay still) after LP data
* transmission read requests
* @param instance pointer to structure holding the DSI Host core information
* @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
* link still, after sending a low power read operation. This period is
* measured in cycles of lanebyteclk
*/
static void dsi_lp_read_presp_timeout(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_LP_RD_TO_CNT, byte_cycle);
}

/**
* Timeout for peripheral (for controller to stay still) after HS data
* transmission write requests
* @param instance pointer to structure holding the DSI Host core information
* @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
* link still, after sending a high-speed write operation. This period is
* measured in cycles of lanebyteclk
*/
static void dsi_hs_write_presp_timeout(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_HS_WR_TO_CNT);
    val = reg_value(byte_cycle, val, HS_WR_TO_CNT_SHIFT, HS_WR_TO_CNT_MASK);

    disp_write(base, MIPI_DSI_HS_WR_TO_CNT, val);
}

/**
* Timeout for peripheral (for controller to stay still) after LP data
* transmission write requests
* @param instance pointer to structure holding the DSI Host core information
* @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
* link still, after sending a low power write operation. This period is
* measured in cycles of lanebyteclk
*/
static void dsi_lp_write_presp_timeout(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_LP_WR_TO_CNT, byte_cycle);
}

/* PRESP Time outs */
/**
* Timeout for peripheral (for controller to stay still) after bus turn around
* @param instance pointer to structure holding the DSI Host core information
* @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
* link still, after sending a BTA operation. This period is
* measured in cycles of lanebyteclk
*/
static void dsi_bta_presp_timeout(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_BTA_TO_CNT, byte_cycle);
}

/**
* Enable the automatic mechanism to stop providing clock in the clock
* lane when time allows
* @param instance pointer to structure holding the DSI Host core information
* @param enable
* @return error code
*/
static void dsi_nc_clk_en(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_LPCLK_CTRL);
    val = reg_value(enable, val,
        AUTO_CLKLANE_CTRL_SHIFT,
        AUTO_CLKLANE_CTRL_MASK);

    disp_write(base, MIPI_DSI_LPCLK_CTRL, val);
}
/**
 * Get the status of the automatic mechanism to stop providing clock in the
 * clock lane when time allows
 * @param dev pointer to structure holding the DSI Host core information
 * @return status 1 (enabled) 0 (disabled)
 */
static uint8_t dsi_nc_clk_status(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_LPCLK_CTRL)
        & AUTO_CLKLANE_CTRL_MASK)
        >> AUTO_CLKLANE_CTRL_SHIFT;

    return val;
}

/**
* Get the error 0 interrupt register status
* @param instance pointer to structure holding the DSI Host core information
* @param mask the mask to be read from the register
* @return error status 0 value
*/
static uint32_t dsi_int0_status(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_INT_ST0);

    if (val & DPHY_ERRORS_0_MASK)
        DISPERR("dphy_err: escape entry error\n");

    if (val & DPHY_ERRORS_1_MASK)
        DISPERR("dphy_err: lp data transmission sync error\n");

    if (val & DPHY_ERRORS_2_MASK)
        DISPERR("dphy_err: control error\n");

    if (val & DPHY_ERRORS_3_MASK)
        DISPERR("dphy_err: LP0 contention error\n");

    if (val & DPHY_ERRORS_4_MASK)
        DISPERR("dphy_err: LP1 contention error\n");

    if (val & ACK_WITH_ERR_0_MASK)
        DISPERR("ack_err: SoT error\n");

    if (val & ACK_WITH_ERR_1_MASK)
        DISPERR("ack_err: SoT Sync error\n");

    if (val & ACK_WITH_ERR_2_MASK)
        DISPERR("ack_err: EoT Sync error\n");

    if (val & ACK_WITH_ERR_3_MASK)
        DISPERR("ack_err: Escape Mode Entry Command error\n");

    if (val & ACK_WITH_ERR_4_MASK)
        DISPERR("ack_err: LP Transmit Sync error\n");

    if (val & ACK_WITH_ERR_5_MASK)
        DISPERR("ack_err: Peripheral Timeout error\n");

    if (val & ACK_WITH_ERR_6_MASK)
        DISPERR("ack_err: False Control error\n");

    if (val & ACK_WITH_ERR_7_MASK)
        DISPERR("ack_err: reserved (specific to device)\n");

    if (val & ACK_WITH_ERR_8_MASK)
        DISPERR("ack_err: ECC error, single-bit (corrected)\n");

    if (val & ACK_WITH_ERR_9_MASK)
        DISPERR("ack_err: ECC error, multi-bit (not corrected)\n");

    if (val & ACK_WITH_ERR_10_MASK)
        DISPERR("ack_err: checksum error (long packet only)\n");

    if (val & ACK_WITH_ERR_11_MASK)
        DISPERR("ack_err: not recognized DSI data type\n");

    if (val & ACK_WITH_ERR_12_MASK)
        DISPERR("ack_err: DSI VC ID Invalid\n");

    if (val & ACK_WITH_ERR_13_MASK)
        DISPERR("ack_err: invalid transmission length\n");

    if (val & ACK_WITH_ERR_14_MASK)
        DISPERR("ack_err: reserved (specific to device)\n");

    if (val & ACK_WITH_ERR_15_MASK)
        DISPERR("ack_err: DSI protocol violation\n");

    return 0;
}

/**
* Get the error 1 interrupt register status
* @param instance pointer to structure holding the DSI Host core information
* @param mask the mask to be read from the register
* @return error status 1 value
*/
static uint32_t dsi_int1_status(struct dsi_context *ctx)
{
    int val;
    uint32_t status = 0;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_INT_ST1);

    if (val & TO_HS_TX_MASK)
        DISPERR("high-speed transmission timeout\n");

    if (val & TO_LP_RX_MASK)
        DISPERR("low-power reception timeout\n");

    if (val & ECC_SINGLE_ERR_MASK)
        DISPERR("ECC single error in a received packet\n");

    if (val & ECC_MILTI_ERR_MASK)
        DISPERR("ECC multiple error in a received packet\n");

    if (val & CRC_ERR_MASK)
        DISPERR("CRC error in the received packet payload\n");

    if (val & PKT_SIZE_ERR_MASK)
        DISPERR("receive packet size error\n");

    if (val & EOPT_ERR_MASK)
        DISPERR("EoTp packet is not received\n");

    if (val & DPI_PLD_WR_ERR_MASK) {
        DISPERR("DPI pixel-fifo is full\n");
        status |= BIT(0);//DSI_INT_STS_NEED_SOFT_RESET;
    }

    if (val & GEN_CMD_WR_ERR_MASK)
        DISPERR("cmd header-fifo is full\n");

    if (val & GEN_PLD_WR_ERR_MASK)
        DISPERR("cmd write-payload-fifo is full\n");

    if (val & GEN_PLD_SEND_ERR_MASK)
        DISPERR("cmd write-payload-fifo is empty\n");

    if (val & GEN_PLD_RD_ERR_MASK)
        DISPERR("cmd read-payload-fifo is empty\n");

    if (val & GEN_PLD_RECEV_ERR_MASK)
        DISPERR("cmd read-payload-fifo is full\n");

    return status;
}

/**
* Configure MASK (hiding) of interrupts coming from error 0 source
* @param instance pointer to structure holding the DSI Host core information
* @param mask to be written to the register
*/
static void dsi_int0_mask(struct dsi_context *ctx, uint32_t mask)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_INT_MSK0, mask);
}

/**
 * Get the ERROR MASK  0 register status
 * @param dev pointer to structure holding the DSI Host core information
 * @param mask the bits to read from the mask register
 */
static uint32_t dsi_int_get_mask_0(struct dsi_context *ctx, uint32_t mask)
{
    addr_t base = ctx->base;

    return disp_read(base, MIPI_DSI_INT_MSK0);
}

/**
* Configure MASK (hiding) of interrupts coming from error 1 source
* @param instance pointer to structure holding the DSI Host core information
* @param mask the mask to be written to the register
*/
static void dsi_int1_mask(struct dsi_context *ctx, uint32_t mask)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_INT_MSK1, mask);
}

/**
 * Get the ERROR MASK  1 register status
 * @param dev pointer to structure holding the DSI Host core information
 * @param mask the bits to read from the mask register
 */
static uint32_t dsi_int_get_mask_1(struct dsi_context *ctx, uint32_t mask)
{
    addr_t base = ctx->base;

    return disp_read(base, MIPI_DSI_INT_MSK1);
}

/**
 * Force Interrupt coming from INT 0
 * @param dev pointer to structure holding the DSI Host core information
 * @param force interrupts to be forced
 */
static void dsi_force_int_0(struct dsi_context *ctx, uint32_t force)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_INT_FORCE0, force);
}

/**
 * Force Interrupt coming from INT 1
 * @param dev pointer to structure holding the DSI Host core information
 * @param force interrupts to be forced
 */
static void dsi_force_int_1(struct dsi_context *ctx, uint32_t force)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_INT_FORCE1, force);
}

/*
static void dsi_auto_ulps_mode(struct dsi_context *ctx, int mode)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, )
	struct dsi_reg *reg = (struct dsi_reg *)ctx->base;
	union _0xE0  auto_ulps_mode;

	auto_ulps_mode.bits.auto_ulps = mode;

	dsi_write(auto_ulps_mode.val, &reg->AUTO_ULPS_MODE);
}

static void dsi_pll_off_in_ulps(struct dsi_context *ctx,
		int pll_off_ulps, int pre_pll_off_req)
{
	struct dsi_reg *reg = (struct dsi_reg *)ctx->base;
	union _0xE0  auto_ulps_mode;

	auto_ulps_mode.bits.pll_off_ulps = pll_off_ulps;
	auto_ulps_mode.bits.pre_pll_off_req = pre_pll_off_req;

	dsi_write(auto_ulps_mode.val, &reg->AUTO_ULPS_MODE);
}

static void dsi_auto_ulps_entry(struct dsi_context *ctx, int delay)
{
	struct dsi_reg *reg = (struct dsi_reg *)ctx->base;

	dsi_write(delay, &reg->AUTO_ULPS_ENTRY_DELAY);
}

static void dsi_auto_ulps_wakeup(struct dsi_context *ctx,
			int twakeup_clk_div, int twakeup_cnt)
{
	struct dsi_reg *reg = (struct dsi_reg *)ctx->base;
	union _0xE8  auto_ulps_wakeup_time;

	auto_ulps_wakeup_time.bits.twakeup_clk_div = twakeup_clk_div;
	auto_ulps_wakeup_time.bits.twakeup_cnt = twakeup_cnt;

	dsi_write(auto_ulps_wakeup_time.val, &reg->AUTO_ULPS_WAKEUP_TIME);
}*/

/**
* Configure how many cycles of byte clock would the PHY module take
* to turn the bus around to start receiving
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle
* @return error code
*/
static void dsi_max_read_time(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    addr_t base = ctx->base;

    disp_write(base, MIPI_DSI_PHY_TMR_RD_CFG, byte_cycle);
}

/**
 * Function to activate shadow registers functionality
 * @param dev pointer to structure holding the DSI Host core information
 * @param activate activate or deactivate shadow registers
 */
static void dsi_activate_shadow_registers(struct dsi_context *ctx,
    uint8_t activate)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_SHADOW_CTRL);
    val = reg_value(activate, val, VID_SHADOW_EN_SHIFT, VID_SHADOW_EN_MASK);

    disp_write(base, MIPI_DSI_VID_SHADOW_CTRL, val);
}

/**
 * Function to read shadow registers functionality state
 * @param dev pointer to structure holding the DSI Host core information
 */
static uint8_t dsi_read_state_shadow_registers(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_SHADOW_CTRL)
        & VID_SHADOW_EN_MASK)
        >> VID_SHADOW_EN_SHIFT;

    return val;
}

/**
 * Request registers change
 * @param dev pointer to structure holding the DSI Host core information
 */
static void dsi_request_registers_change(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_SHADOW_CTRL);
    val = reg_value(0x1, val, VID_SHADOW_REQ_SHIFT, VID_SHADOW_REQ_MASK);

    disp_write(base, MIPI_DSI_VID_SHADOW_CTRL, val);
}

/**
 * Use external pin as control to registers change
 * @param dev pointer to structure holding the DSI Host core information
 * @param external choose between external or internal control
 */
static void dsi_external_pin_registers_change(struct dsi_context *ctx,
    uint8_t external)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_VID_SHADOW_CTRL);
    val = reg_value(external, val,
        VID_SHADOW_PIN_REQ_SHIFT,
        VID_SHADOW_PIN_REQ_MASK);

    disp_write(base, MIPI_DSI_VID_SHADOW_CTRL, val);
}

/**
 * Get the DPI video virtual channel destination
 * @param dev pointer to structure holding the DSI Host core information
 * @return virtual channel
 */
static uint8_t dsi_get_dpi_video_vc_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_DPI_VCID_ACT)
        & DPI_VCID_MASK)
        >> DPI_VCID_SHIFT;

    return val;
}

/**
 * Get loosely packed variant to 18-bit configurations status
 * @param dev pointer to structure holding the DSI Host core information
 * @return loosely status
 */
static uint8_t dsi_get_loosely18_en_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_DPI_COLOR_CODING)
        & LOOSELY18_EN_MASK)
        >> LOOSELY18_EN_SHIFT;

    return val;
}

/**
 * Get DPI video color coding
 * @param dev pointer to structure holding the DSI Host core information
 * @return color coding enum (configuration and color depth)
 */
static uint8_t dsi_get_dpi_color_coding_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_DPI_COLOR_CODING)
        & DPI_COLOR_CODING_MASK)
        >> DPI_COLOR_CODING_SHIFT;

    return val;
}

/**
 * See if the command transmission is in low-power mode.
 * @param dev pointer to structure holding the DSI Host core information
 * @return lpm command transmission
 */
static uint8_t dsi_get_lp_cmd_en_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_MODE_CFG_ACT)
        & ACT_LP_CMD_EN_MASK)
        >> ACT_LP_CMD_EN_SHIFT;

    return val;
}

/**
 * See if there is a request for an acknowledge response at
 * the end of a frame.
 * @param dev pointer to structure holding the DSI Host core information
 * @return  acknowledge response
 */
static uint8_t dsi_get_frame_bta_ack_en_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_MODE_CFG_ACT)
        & ACT_FRAME_BTA_ACK_EN_MASK)
        >> ACT_FRAME_BTA_ACK_EN_SHIFT;

    return val;
}

/**
 * Get the return to low-power inside the Horizontal Front Porch (HFP)
 * period when timing allows.
 * @param dev pointer to structure holding the DSI Host core information
 * @return return to low-power
 */
static uint8_t dsi_get_lp_hfp_en_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_MODE_CFG_ACT)
        & ACT_LP_HFP_EN_MASK)
        >> ACT_LP_HFP_EN_SHIFT;

    return val;
}

/**
 * Get the return to low-power inside the Horizontal Back Porch (HBP) period
 * when timing allows.
 * @param dev pointer to structure holding the DSI Host core information
 * @return return to low-power
 */
static uint8_t dsi_get_lp_hbp_en_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_MODE_CFG_ACT)
        & ACT_LP_HBP_EN_MASK)
        >> ACT_LP_HBP_EN_SHIFT;

    return val;
}

/**
 * Get the return to low-power inside the Vertical Active (VACT) period when
 * timing allows.
 * @param dev pointer to structure holding the DSI Host core information
 * @return return to low-power
 */
static uint8_t dsi_get_lp_vact_en_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_MODE_CFG_ACT)
        & ACT_LP_VACT_EN_MASK)
        >> ACT_LP_VACT_EN_SHIFT;

    return val;
}

/**
 * Get the return to low-power inside the Vertical Front Porch (VFP) period
 * when timing allows.
 * @param dev pointer to structure holding the DSI Host core information
 * @return return to low-power
 */
static uint8_t dsi_get_lp_vfp_en_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_MODE_CFG_ACT)
        & ACT_LP_VFP_EN_MASK)
        >> ACT_LP_VFP_EN_SHIFT;

    return val;
}

/**
 * Get the return to low-power inside the Vertical Back Porch (VBP) period
 * when timing allows.
 * @param dev pointer to structure holding the DSI Host core information
 * @return return to low-power
 */
static uint8_t dsi_get_lp_vbp_en_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_MODE_CFG_ACT)
        & ACT_LP_VBP_EN_MASK)
        >> ACT_LP_VBP_EN_SHIFT;

    return val;
}

/**
 * Get the return to low-power inside the Vertical Sync Time (VSA) period
 * when timing allows.
 * @param dev pointer to structure holding the DSI Host core information
 * @return return to low-power
 */
static uint8_t dsi_get_lp_vsa_en_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_MODE_CFG_ACT)
        & ACT_LP_VSA_EN_MASK)
        >> ACT_LP_VSA_EN_SHIFT;

    return val;
}
/**
 * Get the video mode transmission type
 * @param dev pointer to structure holding the DSI Host core information
 * @return video mode transmission type
 */
static uint8_t dsi_get_vid_mode_type_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_MODE_CFG_ACT)
        & ACT_VID_MODE_TYPE_MASK)
        >> ACT_VID_MODE_TYPE_SHIFT;

    return val;
}

/**
 * Get the number of pixels in a single video packet
 * @param dev pointer to structure holding the DSI Host core information
 * @return video packet size
 */
static uint16_t dsi_get_vid_pkt_size_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_PKT_SIZE_ACT)
        & VID_PKT_SIZE_MASK)
        >> VID_PKT_SIZE_SHIFT;

    return val;
}

/**
 * Get the number of chunks to be transmitted during a Line period
 * (a chunk is pair made of a video packet and a null packet).
 * @param dev pointer to structure holding the DSI Host core information
 * @return num_chunks number of chunks to use
 */
static uint16_t dsi_get_vid_num_chunks_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_NUM_CHUNKS_ACT)
        & VID_NUM_CHUNKS_MASK)
        >> VID_NUM_CHUNKS_SHIFT;

    return val;
}

/**
 * Get the number of bytes inside a null packet
 * @param dev pointer to structure holding the DSI Host core information
 * @return size of null packets
 */
static uint16_t dsi_get_vid_null_size_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_NULL_SIZE_ACT)
        & VID_NULL_SIZE_MASK)
        >> VID_NULL_SIZE_SHIFT;

    return val;
}

/**
 * Get the Horizontal Synchronism Active period in lane byte clock cycles.
 * @param dev pointer to structure holding the DSI Host core information
 * @return video HSA time
 */
static uint16_t dsi_get_vid_hsa_time_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_HSA_TIME_ACT)
        & VID_HSA_TIME_MASK)
        >> VID_HSA_TIME_SHIFT;

    return val;
}

/**
 * Get the Horizontal Back Porch period in lane byte clock cycles.
 * @param dev pointer to structure holding the DSI Host core information
 * @return video HBP time
 */
static uint16_t dsi_get_vid_hbp_time_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_HBP_TIME_ACT)
        & VID_HBP_TIME_MASK)
        >> VID_HBP_TIME_SHIFT;

    return val;
}

/**
 * Get the size of the total line time (HSA+HBP+HACT+HFP)
 * counted in lane byte clock cycles.
 * @param dev pointer to structure holding the DSI Host core information
 * @return overall time for each video line
 */
static uint16_t dsi_get_vid_hline_time_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_HLINE_TIME_ACT)
        & VID_HLINE_TIME_MASK)
        >> VID_HLINE_TIME_SHIFT;

    return val;
}

/**
 * Get the Vertical Synchronism Active period measured in number of horizontal lines.
 * @param dev pointer to structure holding the DSI Host core information
 * @return VSA period
 */
static uint16_t dsi_get_vsa_lines_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_VSA_LINES_ACT)
        & VSA_LINES_MASK)
        >> VSA_LINES_SHIFT;

    return val;
}

/**
 * Get the Vertical Back Porch period measured in number of horizontal lines.
 * @param dev pointer to structure holding the DSI Host core information
 * @return VBP period
 */
static uint16_t dsi_get_vbp_lines_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_VBP_LINES_ACT)
        & VBP_LINES_MASK)
        >> VBP_LINES_SHIFT;

    return val;
}

/**
 * Get the Vertical Front Porch period measured in number of horizontal lines.
 * @param dev pointer to structure holding the DSI Host core information
 * @return VFP period
 */
static uint16_t dsi_get_vfp_lines_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_VFP_LINES_ACT)
        & VFP_LINES_MASK)
        >> VFP_LINES_SHIFT;

    return val;
}

/**
 * Get the Vertical Active period measured in number of horizontal lines
 * @param dev pointer to structure holding the DSI Host core information
 * @return vertical resolution of video.
 */
static uint16_t dsi_get_v_active_lines_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_VID_VACTIVE_LINES_ACT)
        & V_ACTIVE_LINES_MASK)
        >> V_ACTIVE_LINES_SHIFT;

    return val;
}


/**
 * See if the next VSS packet includes 3D control payload
 * @param dev pointer to structure holding the DSI Host core information
 * @return include 3D control payload
 */
static uint8_t dsi_get_send_3d_cfg_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_SDF_3D_ACT)
        & SEND_3D_CFG_MASK)
        >> SEND_3D_CFG_SHIFT;

    return val;
}

/**
 * Get the left/right order:
 * @param dev pointer to structure holding the DSI Host core information
 * @return left/right order:
 */
static uint8_t dsi_get_right_left_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_SDF_3D_ACT)
        & RIGHT_FIRST_MASK)
        >> RIGHT_FIRST_SHIFT;

    return val;
}

/**
 * See if there is a second VSYNC pulse between left and right Images,
 * when 3D image format is frame-based:
 * @param dev pointer to structure holding the DSI Host core information
 * @return second vsync
 */
static uint8_t dsi_get_second_vsync_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_SDF_3D_ACT)
        & SECOND_VSYNC_MASK)
        >> SECOND_VSYNC_SHIFT;

    return val;
}

/**
 * Get the 3D image format
 * @param dev pointer to structure holding the DSI Host core information
 * @return 3D image format
 */
static uint8_t dsi_get_format_3d_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_SDF_3D_ACT)
        & FORMAT_3D_MASK)
        >> FORMAT_3D_SHIFT;

    return val;
}

/**
 * Get the 3D mode on/off and display orientation
 * @param dev pointer to structure holding the DSI Host core information
 * @return 3D mode
 */
static uint8_t dsi_get_mode_3d_act(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_SDF_3D_ACT)
        & MODE_3D_MASK)
        >> MODE_3D_SHIFT;

    return val;
}

/*----------------------------------------------------------------------------*/
/*---------------------------dsi api for dphy---------------------------------*/
/*----------------------------------------------------------------------------*/

/**
* Configure how many cycles of byte clock would the PHY module take
* to switch clock lane from high speed to low power
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle
* @return error code
*/
static void dsi_dphy_clklane_hs2lp_config(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_TMR_LPCLK_CFG);
    val = reg_value(byte_cycle, val,
        PHY_CLKHS2LP_TIME_SHIFT,
        PHY_CLKHS2LP_TIME_MASK);

    disp_write(base, MIPI_DSI_PHY_TMR_LPCLK_CFG, val);
}

/**
* Configure how many cycles of byte clock would the PHY module take
* to switch clock lane from to low power high speed
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle
* @return error code
*/
static void dsi_dphy_clklane_lp2hs_config(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_TMR_LPCLK_CFG);
    val = reg_value(byte_cycle, val,
        PHY_CLKLP2HS_TIME_SHIFT,
        PHY_CLKLP2HS_TIME_MASK);

    disp_write(base, MIPI_DSI_PHY_TMR_LPCLK_CFG, val);
}


/**
* Configure how many cycles of byte clock would the PHY module take
* to switch data lane from high speed to low power
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle
* @return error code
*/
static void dsi_dphy_datalane_hs2lp_config(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_TMR_CFG);
    val = reg_value(byte_cycle, val,
        PHY_HS2LP_TIME_SHIFT,
        PHY_HS2LP_TIME_MASK);

    disp_write(base, MIPI_DSI_PHY_TMR_CFG, val);
}

/**
* Configure how many cycles of byte clock would the PHY module take
* to switch the data lane from to low power high speed
* @param instance pointer to structure holding the DSI Host core information
* @param byte_cycle
* @return error code
*/
static void dsi_dphy_datalane_lp2hs_config(struct dsi_context *ctx,
    uint16_t byte_cycle)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_TMR_CFG);
    val = reg_value(byte_cycle, val,
        PHY_LP2HS_TIME_SHIFT,
        PHY_LP2HS_TIME_MASK);

    disp_write(base, MIPI_DSI_PHY_TMR_CFG, val);
}

/**
* Enable/disable the D-PHY Clock Lane
* @param instance pointer to structure holding the DSI Host core information
* @param enable
*/
static void dsi_dphy_enableclk(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_RSTZ);
    val = reg_value(enable, val, PHY_ENABLECLK_SHIFT, PHY_ENABLECLK_MASK);

    disp_write(base, MIPI_DSI_PHY_RSTZ, val);
}

/*
 * Reset D-PHY module
 *
 * @param instance pointer to structure holding the DSI Host core information
 * module
 * @param reset
 */
static void dsi_dphy_reset(struct dsi_context *ctx, int reset)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_RSTZ);
    val = reg_value(reset, val, PHY_RSTZ_SHIFT, PHY_RSTZ_MASK);

    disp_write(base, MIPI_DSI_PHY_RSTZ, val);
}

/**
 * Power up/down D-PHY module
 *
 * @param instance pointer to structure holding the DSI Host core information
 * module
 * @param powerup (1) shutdown (0)
 */
static void dsi_dphy_shutdown(struct dsi_context *ctx, int powerup)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_RSTZ);
    val = reg_value(powerup, val, PHY_SHUTDOWNZ_SHIFT, PHY_SHUTDOWNZ_MASK);

    disp_write(base, MIPI_DSI_PHY_RSTZ, val);
}
/*
 * Force D-PHY PLL to stay on while in ULPS
 *
 * @param instance pointer to structure holding the DSI Host core information
 * module
 * @param force (1) disable (0)
 * @note To follow the programming model, use wakeup_pll function
 */
static void dsi_dphy_force_pll(struct dsi_context *ctx, int force)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_RSTZ);
    val = reg_value(force, val, PHY_FORCEPLL_SHIFT, PHY_FORCEPLL_MASK);

    disp_write(base, MIPI_DSI_PHY_RSTZ, val);
}
/**
 * Get force D-PHY PLL module
 * @param instance pointer to structure holding the DSI Host core information
 * module
 * @return force value
 */
static int dsi_dphy_get_force_pll(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_PHY_RSTZ)
        & PHY_FORCEPLL_MASK)
        >> PHY_FORCEPLL_SHIFT;

    return val;
}

/**
 * Configure minimum wait period for HS transmission request after a stop state
 *
 * @param instance pointer to structure holding the DSI Host core information
 * @param no_of_byte_cycles [in byte (lane) clock cycles]
 */
static void dsi_dphy_stop_wait_time(struct dsi_context *ctx,
    uint8_t no_of_byte_cycles)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_IF_CFG);
    val = reg_value(no_of_byte_cycles, val,
        PHY_STOP_WAIT_TIME_SHIFT, PHY_STOP_WAIT_TIME_MASK);

    disp_write(base, MIPI_DSI_PHY_IF_CFG, val);
}

/**
* Configure the number of active data lanes
* @param instance pointer to structure holding the DSI Host core information
* @param n_lanes
*/
static void dsi_dphy_n_lanes(struct dsi_context *ctx, uint8_t n_lanes)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_IF_CFG);
    val = reg_value(n_lanes - 1, val, N_LANES_SHIFT, N_LANES_MASK);

    disp_write(base, MIPI_DSI_PHY_IF_CFG, val);
}
/**
 * Get number of currently active lanes
 *
 * @param instance pointer to structure holding the DSI Host core information
 * @return number of active lanes
 */
static uint8_t dsi_dphy_get_n_lanes(struct dsi_context *ctx)
{
    int val;
    addr_t base = ctx->base;

    val = (disp_read(base, MIPI_DSI_PHY_IF_CFG)
        & N_LANES_MASK)
        >> N_LANES_SHIFT;

    return (val + 1);
}

/**
 * Request the PHY module to start transmission of high speed clock.
 * This causes the clock lane to start transmitting DDR clock on the
 * lane interconnect.
 *
 * @param dev pointer to structure which holds information about the d-phy
 * module
 * @param enable
 * @note this function should be called explicitly by user always except for
 * transmitting
 */
static void dsi_dphy_enable_hs_clk(struct dsi_context *ctx, int enable)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_LPCLK_CTRL);
    val = reg_value(enable, val,
        PHY_TXREQUESTCLKHS_SHIFT,
        PHY_TXREQUESTCLKHS_MASK);

    disp_write(base, MIPI_DSI_LPCLK_CTRL, val);
}

/*
 * Get D-PHY PPI status
 *
 * @param dev pointer to structure which holds information about the d-phy
 * module
 * @param mask
 * @return status
 */
static uint32_t dsi_dphy_get_status(struct dsi_context *ctx,
    uint16_t mask)
{
    addr_t base = ctx->base;

    return disp_read(base, MIPI_DSI_PHY_STATUS) & mask;
}

/*
 * @param dev pointer to structure which holds information about the d-phy
 * module
 * @param value
 */
static void dsi_dphy_testclk(struct dsi_context *ctx, uint8_t status)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_TST_CTRL0);
    val = reg_value(status, val, PHY_TESTCLK_SHIFT, PHY_TESTCLK_MASK);

    disp_write(base, MIPI_DSI_PHY_TST_CTRL0, val);
}

/**
 * @param dev pointer to structure which holds information about the d-phy
 * module
 * @param value
 */
static void dsi_dphy_test_clear(struct dsi_context *ctx, int value)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_TST_CTRL0);
    val = reg_value(value, val, PHY_TESTCLR_SHIFT, PHY_TESTCLR_MASK);

    disp_write(base, MIPI_DSI_PHY_TST_CTRL0, val);
}

/**
 * @param dev pointer to structure which holds information about the d-phy
 * module
 * @param test_data
 */
static void dsi_dphy_testdin(struct dsi_context *ctx, uint8_t test_data)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_TST_CTRL1);
    val = reg_value(test_data, val, PHY_TESTDIN_SHIFT, PHY_TESTDIN_MASK);

    disp_write(base, MIPI_DSI_PHY_TST_CTRL1, val);
}

/**
 * @param dev pointer to structure which holds information about the d-phy
 * module
 * @param status
 */
static void dsi_dphy_testen(struct dsi_context *ctx, uint8_t status)
{
    int val;
    addr_t base = ctx->base;

    val = disp_read(base, MIPI_DSI_PHY_TST_CTRL1);
    val = reg_value(status, val, PHY_TESTEN_SHIFT, PHY_TESTEN_MASK);

    disp_write(base, MIPI_DSI_PHY_TST_CTRL1, val);
}

/**
 * Write to D-PHY module (encapsulating the digital interface)
 * @param ctx pointer to structure which holds information about the d-phy
 * module
 * @param address offset inside the D-PHY digital interface
 * @param data array of bytes to be written to D-PHY
 * @param data_length of the data array
 */
static void dsi_dphy_write(struct dsi_context *ctx, uint16_t address,
    uint8_t *data, uint8_t data_length)
{
    unsigned i = 0;

    DISPDBG("TEST CODE: ADDR %X DATA %X\n", address, data[0]);
    if (data != 0) {
        /* set the TESTCLK input high in preparation
         * to latch in the desired test mode */
        dsi_dphy_testclk(ctx, 1);
        /* set the desired test code in the input 8-bit bus
         * TESTDIN[7:0] */
        dsi_dphy_testdin(ctx, (uint8_t)address);
        /* set TESTEN input high  */
        dsi_dphy_testen(ctx, 1);
        /* drive the TESTCLK input low; the falling edge
         * captures the chosen test code into the transceiver
         */
        dsi_dphy_testclk(ctx, 0);
        /* set TESTEN input low to disable further test mode
         * code latching  */
        dsi_dphy_testen(ctx, 0);
        /* start writing MSB first */
        for (i = data_length; i > 0; i--) {
            /* set TESTDIN[7:0] to the desired test data
             * appropriate to the chosen test mode */
            dsi_dphy_testdin(ctx, data[i - 1]);
            /* pulse TESTCLK high to capture this test data
             * into the macrocell; repeat these two steps
             * as necessary */
            dsi_dphy_testclk(ctx, 1);
            dsi_dphy_testclk(ctx, 0);
        }
    }
}


struct dsi_core_ops dwc_mipi_dsi_host_ops = {
    .check_version = dsi_check_version,
    .power_enable = dsi_power_enable,
    .get_power_status = dsi_get_power_status,
    .timeout_clock_division = dsi_timeout_clock_division,
    .tx_escape_division = dsi_tx_escape_division,
    .video_vcid = dsi_video_vcid,
    .get_video_vcid = dsi_get_video_vcid,
    .dpi_color_coding = dsi_dpi_color_coding,
    .dpi_get_color_coding = dsi_dpi_get_color_coding,
    .dpi_get_color_depth = dsi_dpi_get_color_depth,
    .dpi_get_color_config = dsi_dpi_get_color_config,
    .dpi_18_loosely_packet_en = dsi_dpi_18_loosely_packet_en,
    .dpi_color_mode_pol = dsi_dpi_color_mode_pol,
    .dpi_shut_down_pol = dsi_dpi_shut_down_pol,
    .dpi_hsync_pol = dsi_dpi_hsync_pol,
    .dpi_vsync_pol = dsi_dpi_vsync_pol,
    .dpi_data_en_pol = dsi_dpi_data_en_pol,
    .eotp_rx_en = dsi_eotp_rx_en,
    .eotp_tx_en = dsi_eotp_tx_en,
    .bta_en = dsi_bta_en,
    .ecc_rx_en = dsi_ecc_rx_en,
    .crc_rx_en = dsi_crc_rx_en,
    .eotp_tx_lp_en = dsi_eotp_tx_lp_en,
    .rx_vcid = dsi_rx_vcid,
    .video_mode = dsi_video_mode,
    .cmd_mode = dsi_cmd_mode,
    .is_cmd_mode = dsi_is_cmd_mode,
    .video_mode_lp_cmd_en = dsi_video_mode_lp_cmd_en,
    .video_mode_frame_ack_en = dsi_video_mode_frame_ack_en,
    .video_mode_lp_hfp_en = dsi_video_mode_lp_hfp_en,
    .video_mode_lp_hbp_en = dsi_video_mode_lp_hbp_en,
    .video_mode_lp_vact_en = dsi_video_mode_lp_vact_en,
    .video_mode_lp_vfp_en = dsi_video_mode_lp_vfp_en,
    .video_mode_lp_vbp_en = dsi_video_mode_lp_vbp_en,
    .video_mode_lp_vsa_en = dsi_video_mode_lp_vsa_en,
    .dpi_hporch_lp_en = dsi_dpi_hporch_lp_en,
    .dpi_vporch_lp_en = dsi_dpi_vporch_lp_en,
    .video_mode_mode_type= dsi_video_mode_mode_type,
    .vpg_orientation_act = dsi_vpg_orientation_act,
    .vpg_mode_act = dsi_vpg_mode_act,
    .enable_vpg_act = dsi_enable_vpg_act,
    .dpi_video_packet_size = dsi_dpi_video_packet_size,
    .dpi_chunk_num = dsi_dpi_chunk_num,
    .dpi_null_packet_size = dsi_dpi_null_packet_size,
    .dpi_hline_time = dsi_dpi_hline_time,
    .dpi_hbp_time = dsi_dpi_hbp_time,
    .dpi_hsync_time = dsi_dpi_hsync_time,
    .dpi_vsync = dsi_dpi_vsync,
    .dpi_vbp = dsi_dpi_vbp,
    .dpi_vfp = dsi_dpi_vfp,
    .dpi_vact = dsi_dpi_vact,
    .tear_effect_ack_en = dsi_tear_effect_ack_en,
    .cmd_ack_request_en = dsi_cmd_ack_request_en,
    .cmd_mode_lp_cmd_en = dsi_cmd_mode_lp_cmd_en,
    .set_packet_header = dsi_set_packet_header,
    .set_packet_payload = dsi_set_packet_payload,
    .get_rx_payload = dsi_get_rx_payload ,
    .is_bta_returned = dsi_is_bta_returned,
    .is_rx_payload_fifo_full = dsi_is_rx_payload_fifo_full,
    .is_rx_payload_fifo_empty = dsi_is_rx_payload_fifo_empty,
    .is_tx_payload_fifo_full = dsi_is_tx_payload_fifo_full ,
    .is_tx_payload_fifo_empty = dsi_is_tx_payload_fifo_empty,
    .is_tx_cmd_fifo_full = dsi_is_tx_cmd_fifo_full,
    .is_tx_cmd_fifo_empty = dsi_is_tx_cmd_fifo_empty,
    .lp_rx_timeout = dsi_lp_rx_timeout,
    .hs_tx_timeout = dsi_hs_tx_timeout,
    .hs_read_presp_timeout = dsi_hs_read_presp_timeout,
    .lp_read_presp_timeout = dsi_lp_read_presp_timeout,
    .hs_write_presp_timeout = dsi_hs_write_presp_timeout,
    .bta_presp_timeout = dsi_bta_presp_timeout,
    .nc_clk_en = dsi_nc_clk_en ,
    .nc_clk_status = dsi_nc_clk_status,
    .int0_status = dsi_int0_status,
    .int1_status = dsi_int1_status,
    .int0_mask = dsi_int0_mask,
    .int_get_mask_0 = dsi_int_get_mask_0,
    .int1_mask = dsi_int1_mask,
    .int_get_mask_1 = dsi_int_get_mask_1,
    .force_int_0 = dsi_force_int_0,
    .force_int_1 = dsi_force_int_1,
    .max_read_time = dsi_max_read_time,
    .activate_shadow_registers = dsi_activate_shadow_registers,
    .read_state_shadow_registers = dsi_read_state_shadow_registers,
    .request_registers_change = dsi_request_registers_change,
    .external_pin_registers_change = dsi_external_pin_registers_change,
    .get_dpi_video_vc_act = dsi_get_dpi_video_vc_act,
    .get_loosely18_en_act = dsi_get_loosely18_en_act,
    .get_dpi_color_coding_act = dsi_get_dpi_color_coding_act,
    .get_lp_cmd_en_act = dsi_get_lp_cmd_en_act,
    .get_frame_bta_ack_en_act = dsi_get_frame_bta_ack_en_act,
    .get_lp_hfp_en_act = dsi_get_lp_hfp_en_act,
    .get_lp_hbp_en_act = dsi_get_lp_hbp_en_act,
    .get_lp_vact_en_act = dsi_get_lp_vact_en_act,
    .get_lp_vfp_en_act = dsi_get_lp_vfp_en_act,
    .get_lp_vbp_en_act = dsi_get_lp_vbp_en_act,
    .get_lp_vsa_en_act = dsi_get_lp_vsa_en_act ,
    .get_vid_mode_type_act = dsi_get_vid_mode_type_act,
    .get_vid_pkt_size_act = dsi_get_vid_pkt_size_act,
    .get_vid_num_chunks_act = dsi_get_vid_num_chunks_act,
    .get_vid_null_size_act = dsi_get_vid_null_size_act,
    .get_vid_hsa_time_act = dsi_get_vid_hsa_time_act,
    .get_vid_hbp_time_act = dsi_get_vid_hbp_time_act,
    .get_vid_hline_time_act = dsi_get_vid_hline_time_act,
    .get_vsa_lines_act = dsi_get_vsa_lines_act,
    .get_vbp_lines_act = dsi_get_vbp_lines_act,
    .get_vfp_lines_act = dsi_get_vfp_lines_act,
    .get_v_active_lines_act = dsi_get_v_active_lines_act,
    .get_send_3d_cfg_act = dsi_get_send_3d_cfg_act,
    .get_right_left_act = dsi_get_right_left_act,
    .get_second_vsync_act = dsi_get_second_vsync_act,
    .get_format_3d_act = dsi_get_format_3d_act,
    .get_mode_3d_act = dsi_get_mode_3d_act,

    .dphy_clklane_hs2lp_config = dsi_dphy_clklane_hs2lp_config,
    .dphy_clklane_lp2hs_config = dsi_dphy_clklane_lp2hs_config,
    .dphy_datalane_hs2lp_config = dsi_dphy_datalane_hs2lp_config,
    .dphy_datalane_lp2hs_config = dsi_dphy_datalane_lp2hs_config,
    .dphy_enableclk = dsi_dphy_enableclk,
    .dphy_reset = dsi_dphy_reset,
    .dphy_shutdown = dsi_dphy_shutdown,
    .dphy_force_pll = dsi_dphy_force_pll,
    .dphy_get_force_pll = dsi_dphy_get_force_pll,
    .dphy_stop_wait_time = dsi_dphy_stop_wait_time,
    .dphy_n_lanes = dsi_dphy_n_lanes,
    .dphy_enable_hs_clk = dsi_dphy_enable_hs_clk,
    .dphy_get_n_lanes = dsi_dphy_get_n_lanes,
    .dphy_get_status = dsi_dphy_get_status,
    .dphy_test_clear = dsi_dphy_test_clear,
    .dphy_write = dsi_dphy_write,
};

