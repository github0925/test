/*
 * sdriv_drm_dsi.h
 *
 * Semidrive platform drm driver
 *
 * Copyright (C) 2019, Semidrive  Communications Inc.
 *
 * This file is licensed under a dual GPLv2 or X11 license.
 */
#ifndef __SDRIV_DRM_DSIH__
#define __SDRIV_DRM_DSIH__
#include <stdbool.h>
#include <sys/types.h>

#define DEFAULT_BYTE_CLOCK    (432000)
#define DSIH_PIXEL_TOLERANCE  (2)
#define VIDEO_PACKET_OVERHEAD  6 /* HEADER (4 bytes) + CRC (2 bytes) */
#define NULL_PACKET_OVERHEAD   6  /* HEADER (4 bytes) + CRC (2 bytes) */

#define PRECISION_FACTOR      (1000)
#define MAX_NULL_SIZE         (1023)

enum dsi_work_mode {
	DSI_MODE_CMD = 0,
	DSI_MODE_VIDEO
};

enum video_burst_mode {
	VIDEO_NON_BURST_WITH_SYNC_PULSES = 0,
	VIDEO_NON_BURST_WITH_SYNC_EVENTS,
	VIDEO_BURST_WITH_SYNC_PULSES
};

typedef enum {
    COLOR_CODE_16BIT_CONFIG1 = 0,
    COLOR_CODE_16BIT_CONFIG2 = 1,
    COLOR_CODE_16BIT_CONFIG3 = 2,
    COLOR_CODE_18BIT_CONFIG1 = 3,
    COLOR_CODE_18BIT_CONFIG2 = 4,
    COLOR_CODE_24BIT = 5,
    COLOR_CODE_20BIT_YCC422_LOOSELY = 6,
    COLOR_CODE_24BIT_YCC422 = 7,
    COLOR_CODE_16BIT_YCC422 = 8,
    COLOR_CODE_30BIT = 9,
    COLOR_CODE_36BIT = 10,
    COLOR_CODE_12BIT_YCC420 = 11,
    COLOR_CODE_DSC24 = 12,
    COLOR_CODE_MAX
} dsih_color_coding_t;

struct dsih_dpi_video_t {
	/** Virtual channel number to send this video stream */
	uint8_t virtual_channel;
	/** Number of lanes used to send current video */
	uint8_t n_lanes;
	/** Video mode, whether burst with sync pulses, or packets with either sync pulses or events */
	uint8_t burst_mode;
	/** Maximum number of byte clock cycles needed by the PHY to transition
	 * the data lanes from high speed to low power - REQUIRED */
	uint16_t data_hs2lp;
	/** Maximum number of byte clock cycles needed by the PHY to transition
	 * the data lanes from low power to high speed - REQUIRED */
	uint16_t data_lp2hs;
	/** Maximum number of byte clock cycles needed by the PHY to transition
	 * the clock lane from high speed to low power - REQUIRED */
	uint16_t clk_hs2lp;
	/** Maximum number of byte clock cycles needed by the PHY to transition
	 * the clock lane from low power to high speed - REQUIRED */
	uint16_t clk_lp2hs;
	/** Enable non coninuous clock for energy saving
	 * - Clock lane will go to LS while not transmitting video */
	int nc_clk_en;
	/** Enable receiving of ack packets */
	int frame_ack_en;
	/** Byte (lane) clock [KHz] */
	unsigned long byte_clock;
	/** Pixel (DPI) Clock [KHz]*/
	unsigned long pixel_clock;
    /**Dphy output frequency*/
    uint32_t phy_freq;
	/** Colour coding - BPP and Pixel configuration */
	dsih_color_coding_t color_coding;
	/** Is 18-bit loosely packets (valid only when BPP == 18) */
	int is_18_loosely;
	/** Data enable signal (dpidaten) whether it is active high or low */
	int data_en_polarity;
	/** Horizontal synchronisation signal (dpihsync) whether it is active high or low */
	int h_polarity;
	/** Horizontal resolution or Active Pixels */
	uint16_t h_active_pixels;	/* hadr */
	/** Horizontal Sync Pixels - min 4 for best performance */
	uint16_t h_sync_pixels;
	/** Horizontal back porch pixels */
	uint16_t h_back_porch_pixels;	/* hbp */
	/** Total Horizontal pixels */
	uint16_t h_total_pixels;	/* h_total */
	/** Vertical synchronisation signal (dpivsync) whether it is active high or low */
	int v_polarity;
	/** Vertical active lines (resolution) */
	uint16_t v_active_lines;	/* vadr */
	/** Vertical sync lines */
	uint16_t v_sync_lines;
	/** Vertical back porch lines */
	uint16_t v_back_porch_lines;	/* vbp */
	/** Vertical front porch lines */
	uint16_t v_front_porch_lines;	/* vbp */
	/** Total no of vertical lines */
	uint16_t v_total_lines;	/* v_total */
	/** When set to 1, this bit enables the EoTp reception */
	int eotp_rx_en;
	/** When set to 1, this bit enables the EoTp transmission */
	int eotp_tx_en;
	/** This register configures the number of chunks to use */
	int no_of_chunks;
	/** This register configures the size of null packets */
	uint16_t null_packet_size;
	/** */
	int dpi_lp_cmd_en;
	/** Diplay type*/
	int display_type;

	uint16_t hline;

};

struct dsi_context {
	addr_t base;
	int irq;
	uint32_t int0_mask;
	uint32_t int1_mask;
	uint8_t id;

	bool is_inited;

    uint8_t max_lanes;
    uint16_t max_bta_cycles;
    struct dsih_dpi_video_t dpi_video;

    struct dsi_core_ops *ops;
};

struct dsi_core_ops {
    bool (*check_version)(struct dsi_context *ctx);
    void (*power_enable)(struct dsi_context *ctx, int enable);
    uint8_t (*get_power_status)(struct dsi_context *ctx);
    void (*timeout_clock_division)(struct dsi_context *ctx, uint8_t div);
    void (*tx_escape_division)(struct dsi_context *ctx, uint8_t division);
    void (*video_vcid)(struct dsi_context *ctx, uint8_t vc);
    uint8_t (*get_video_vcid)(struct dsi_context *ctx);
    void (*dpi_color_coding)(struct dsi_context *ctx, int coding);
    uint8_t (*dpi_get_color_coding)(struct dsi_context *ctx);
    uint8_t (*dpi_get_color_depth)(struct dsi_context *ctx);
    uint8_t (*dpi_get_color_config)(struct dsi_context *ctx);
    void (*dpi_18_loosely_packet_en)(struct dsi_context *ctx, int enable);
    void (*dpi_color_mode_pol)(struct dsi_context *ctx, int active_low);
    void (*dpi_shut_down_pol)(struct dsi_context *ctx, int active_low);
    void (*dpi_hsync_pol)(struct dsi_context *ctx, int active_low);
    void (*dpi_vsync_pol)(struct dsi_context *ctx, int active_low);
    void (*dpi_data_en_pol)(struct dsi_context *ctx, int active_low);
    void (*eotp_rx_en)(struct dsi_context *ctx, int enable);
    void (*eotp_tx_en)(struct dsi_context *ctx, int enable);
    void (*bta_en)(struct dsi_context *ctx, int enable);
    void (*ecc_rx_en)(struct dsi_context *ctx, int enable);
    void (*crc_rx_en)(struct dsi_context *ctx, int enable);
    void (*eotp_tx_lp_en)(struct dsi_context *ctx, int enable);
    void (*rx_vcid)(struct dsi_context *ctx, uint8_t vc);
    void (*video_mode)(struct dsi_context *ctx);
    void (*cmd_mode)(struct dsi_context *ctx);
    bool (*is_cmd_mode)(struct dsi_context *ctx);
    void (*video_mode_lp_cmd_en)(struct dsi_context *ctx, int enable);
    void (*video_mode_frame_ack_en)(struct dsi_context *ctx, int enable);
    void (*video_mode_lp_hfp_en)(struct dsi_context *ctx, int enable);
    void (*video_mode_lp_hbp_en)(struct dsi_context *ctx, int enable);
    void (*video_mode_lp_vact_en)(struct dsi_context *ctx, int enable);
    void (*video_mode_lp_vfp_en)(struct dsi_context *ctx, int enable);
    void (*video_mode_lp_vbp_en)(struct dsi_context *ctx, int enable);
    void (*video_mode_lp_vsa_en)(struct dsi_context *ctx, int enable);
    void (*dpi_hporch_lp_en)(struct dsi_context *ctx, int enable);
    void (*dpi_vporch_lp_en)(struct dsi_context *ctx, int enable);
    void (*video_mode_mode_type)(struct dsi_context *ctx, int mode);
    void (*vpg_orientation_act)(struct dsi_context *ctx,
        uint8_t orientation);
    void (*vpg_mode_act)(struct dsi_context *ctx, uint8_t mode);
    void (*enable_vpg_act)(struct dsi_context *ctx, uint8_t enable);
    void (*dpi_video_packet_size)(struct dsi_context *ctx, uint16_t size);
    void (*dpi_chunk_num)(struct dsi_context *ctx, uint16_t num);
    void (*dpi_null_packet_size)(struct dsi_context *ctx, uint16_t size);
    void (*dpi_hline_time)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*dpi_hbp_time)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*dpi_hsync_time)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*dpi_vsync)(struct dsi_context *ctx, uint16_t lines);
    void (*dpi_vbp)(struct dsi_context *ctx, uint16_t lines);
    void (*dpi_vfp)(struct dsi_context *ctx, uint16_t lines);
    void (*dpi_vact)(struct dsi_context *ctx, uint16_t lines);
    void (*tear_effect_ack_en)(struct dsi_context *ctx, int enable);
    void (*cmd_ack_request_en)(struct dsi_context *ctx, int enable);
    void (*cmd_mode_lp_cmd_en)(struct dsi_context *ctx, int enable);
    void (*set_packet_header)(struct dsi_context *ctx,
        uint8_t vc, uint8_t type, uint8_t wc_lsb, uint8_t wc_msb);
    void (*set_packet_payload)(struct dsi_context *ctx, uint32_t payload);
    void (*get_rx_payload)(struct dsi_context *ctx, uint32_t *payload);
    bool (*is_bta_returned)(struct dsi_context *ctx);
    bool (*is_rx_payload_fifo_full)(struct dsi_context *ctx);
    bool (*is_rx_payload_fifo_empty)(struct dsi_context *ctx);
    bool (*is_tx_payload_fifo_full)(struct dsi_context *ctx);
    bool (*is_tx_payload_fifo_empty)(struct dsi_context *ctx);
    bool (*is_tx_cmd_fifo_full)(struct dsi_context *ctx);
    bool (*is_tx_cmd_fifo_empty)(struct dsi_context *ctx);
    void (*lp_rx_timeout)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*hs_tx_timeout)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*hs_read_presp_timeout)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*lp_read_presp_timeout)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*hs_write_presp_timeout)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*lp_write_presp_timeout)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*bta_presp_timeout)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*nc_clk_en)(struct dsi_context *ctx, int enable);
    uint8_t (*nc_clk_status)(struct dsi_context *ctx);
    uint32_t (*int0_status)(struct dsi_context *ctx);
    uint32_t (*int1_status)(struct dsi_context *ctx);
    void (*int0_mask)(struct dsi_context *ctx, uint32_t mask);
    uint32_t (*int_get_mask_0)(struct dsi_context *ctx, uint32_t mask);
    void (*int1_mask)(struct dsi_context *ctx, uint32_t mask);
    uint32_t (*int_get_mask_1)(struct dsi_context *ctx, uint32_t mask);
    void (*force_int_0)(struct dsi_context *ctx, uint32_t force);
    void (*force_int_1)(struct dsi_context *ctx, uint32_t force);
    void (*max_read_time)(struct dsi_context *ctx, uint16_t byte_cycle);
    void (*activate_shadow_registers)(struct dsi_context *ctx,
        uint8_t activate);
    uint8_t (*read_state_shadow_registers)(struct dsi_context *ctx);
    void (*request_registers_change)(struct dsi_context *ctx);
    void (*external_pin_registers_change)(struct dsi_context *ctx,
        uint8_t external);
    uint8_t (*get_dpi_video_vc_act)(struct dsi_context *ctx);
    uint8_t (*get_loosely18_en_act)(struct dsi_context *ctx);
    uint8_t (*get_dpi_color_coding_act)(struct dsi_context *ctx);
    uint8_t (*get_lp_cmd_en_act)(struct dsi_context *ctx);
    uint8_t (*get_frame_bta_ack_en_act)(struct dsi_context *ctx);
    uint8_t (*get_lp_hfp_en_act)(struct dsi_context *ctx);
    uint8_t (*get_lp_hbp_en_act)(struct dsi_context *ctx);
    uint8_t (*get_lp_vact_en_act)(struct dsi_context *ctx);
    uint8_t (*get_lp_vfp_en_act)(struct dsi_context *ctx);
    uint8_t (*get_lp_vbp_en_act)(struct dsi_context *ctx);
    uint8_t (*get_lp_vsa_en_act)(struct dsi_context *ctx);
    uint8_t (*get_vid_mode_type_act)(struct dsi_context *ctx);
    uint16_t (*get_vid_pkt_size_act)(struct dsi_context *ctx);
    uint16_t (*get_vid_num_chunks_act)(struct dsi_context *ctx);
    uint16_t (*get_vid_null_size_act)(struct dsi_context *ctx);
    uint16_t (*get_vid_hsa_time_act)(struct dsi_context *ctx);
    uint16_t (*get_vid_hbp_time_act)(struct dsi_context *ctx);
    uint16_t (*get_vid_hline_time_act)(struct dsi_context *ctx);
    uint16_t (*get_vsa_lines_act)(struct dsi_context *ctx);
    uint16_t (*get_vbp_lines_act)(struct dsi_context *ctx);
    uint16_t (*get_vfp_lines_act)(struct dsi_context *ctx);
    uint16_t (*get_v_active_lines_act)(struct dsi_context *ctx);
    uint8_t (*get_send_3d_cfg_act)(struct dsi_context *ctx);
    uint8_t (*get_right_left_act)(struct dsi_context *ctx);
    uint8_t (*get_second_vsync_act)(struct dsi_context *ctx);
    uint8_t (*get_format_3d_act)(struct dsi_context *ctx);
    uint8_t (*get_mode_3d_act)(struct dsi_context *ctx);

    void (*dphy_clklane_hs2lp_config)(struct dsi_context *ctx,
        uint16_t byte_cycle);
    void (*dphy_clklane_lp2hs_config)(struct dsi_context *ctx,
        uint16_t byte_cycle);
    void (*dphy_datalane_hs2lp_config)(struct dsi_context *ctx,
        uint16_t byte_cycle);
    void (*dphy_datalane_lp2hs_config)(struct dsi_context *ctx,
        uint16_t byte_cycle);
    void (*dphy_enableclk)(struct dsi_context *ctx, int enable);
    void (*dphy_reset)(struct dsi_context *ctx, int reset);
    void (*dphy_shutdown)(struct dsi_context *ctx, int powerup);
    void (*dphy_force_pll)(struct dsi_context *ctx, int force);
    int (*dphy_get_force_pll)(struct dsi_context *ctx);
    void (*dphy_stop_wait_time)(struct dsi_context *ctx,
        uint8_t no_of_byte_cycles);
    void (*dphy_n_lanes)(struct dsi_context *ctx, uint8_t n_lanes);
    uint8_t (*dphy_get_n_lanes)(struct dsi_context *ctx);
    void (*dphy_enable_hs_clk)(struct dsi_context *ctx, int enable);
    uint32_t (*dphy_get_status)(struct dsi_context *ctx, uint16_t mask);
    void (*dphy_test_clear)(struct dsi_context *ctx, int value);
    void (*dphy_write)(struct dsi_context *ctx, uint16_t address,
        uint8_t *data, uint8_t data_length);
};

#endif //__SDRIV_DRM_DSIH__
