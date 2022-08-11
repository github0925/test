#ifndef __DISPLAY_SHARE_H__
#define __DISPLAY_SHARE_H__

/* Features */
#define CONFIG_DISP_THREAD		(1)
#define CONFIG_VSYNC_THREAD		(0)

#define SHARE_MASK_WIDTH     1920
#define SHARE_MASK_HEIGHT    720

//0:cluster, 1: main screen, 2: sub screen
#define DISPLAY_ID_RPC (0)

#define MAX_BUFFERS 2

#define DISP_MAILBOX_ADDR (0x80)

#define abs(x) (((x) >= 0)? (x) : (-(x)))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define fourcc_code(a, b, c, d) ((u32)(a) | ((u32)(b) << 8) | \
                 ((u32)(c) << 16) | ((u32)(d) << 24))

/* Remote IOC command */
#define DISP_CMD_START				RP_IOWR('d', 0, struct disp_frame_info)
#define DISP_CMD_SET_FRAMEINFO		RP_IOWR('d', 1, struct disp_frame_info)
#define DISP_CMD_SHARING_WITH_MASK	RP_IOWR('d', 2, struct disp_frame_info)
#define DISP_CMD_CLEAR				RP_IOWR('d', 3, struct disp_frame_info)

#define DRM_FORMAT_BIG_ENDIAN	(1<<31) /* format is big endian instead of little endian */
#define DRM_FORMAT_XRGB8888		fourcc_code('X', 'R', '2', '4') /* [31:0] x:R:G:B 8:8:8:8 little endian */
#define DRM_FORMAT_XBGR8888		fourcc_code('X', 'B', '2', '4') /* [31:0] x:B:G:R 8:8:8:8 little endian */
#define DRM_FORMAT_RGBX8888		fourcc_code('R', 'X', '2', '4') /* [31:0] R:G:B:x 8:8:8:8 little endian */
#define DRM_FORMAT_BGRX8888		fourcc_code('B', 'X', '2', '4') /* [31:0] B:G:R:x 8:8:8:8 little endian */

#define DRM_FORMAT_ARGB8888		fourcc_code('A', 'R', '2', '4') /* [31:0] A:R:G:B 8:8:8:8 little endian */
#define DRM_FORMAT_ABGR8888		fourcc_code('A', 'B', '2', '4') /* [31:0] A:B:G:R 8:8:8:8 little endian */
#define DRM_FORMAT_RGBA8888		fourcc_code('R', 'A', '2', '4') /* [31:0] R:G:B:A 8:8:8:8 little endian */
#define DRM_FORMAT_BGRA8888		fourcc_code('B', 'A', '2', '4') /* [31:0] B:G:R:A 8:8:8:8 little endian */

struct dc_share {
    void *g2d;
    uint32_t *mask_base;
    uint32_t *outbufs[MAX_BUFFERS];

    struct sdm_buffer bufs[MAX_BUFFERS];
    struct sdm_post_config post_configs[MAX_BUFFERS];
    struct display_server display_server;
};

int sdm_post_init(struct dc_share *dc_s);
int g2d_handle_init(struct dc_share *dc_s);
void g2d_mask_blending(struct dc_share *dc_s);
int disp_ioctl_set_frameinfo(struct dc_share *dc_s, void *arg);
#endif

