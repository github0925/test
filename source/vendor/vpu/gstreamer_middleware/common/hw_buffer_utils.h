#ifndef __GST_DRM_UTILS_H__
#define __GST_DRM_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <drm/drm.h>
#include <drm/drm_fourcc.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <minigbm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include <gst/video/video.h>

/**********************************************************/
#define MAX_DMABUF_PLANES 4
#define ALIGN(x, mask) (((x) + ((mask)-1)) & ~((mask)-1))

typedef struct hw_buffer_t {
    int width;
    int height;
    int format;
    int size;
    int n_planes;
    int fds[MAX_DMABUF_PLANES];
    int offsets[MAX_DMABUF_PLANES];
    int strides[MAX_DMABUF_PLANES];
    uint64_t modifiers[MAX_DMABUF_PLANES];
    void *mapped_vaddrs[MAX_DMABUF_PLANES];
    union {
        void *data;
        int64_t __padding;
    } __attribute__((aligned(8)));
} hw_buffer_t;

struct hw_buffer_t *hw_buffer_create(int width, int height, int format);
void hw_buffer_destroy(struct hw_buffer_t *bo);
int hw_buffer_map(struct hw_buffer_t *bo);
int hw_buffer_unmap(struct hw_buffer_t *bo);

/**********************************************************/
GstVideoFormat gst_video_format_from_drm(guint32 drmfmt);
guint32 gst_drm_format_from_video(GstVideoFormat fmt);
guint32 gst_drm_bpp_from_drm(guint32 drmfmt);
guint32 gst_drm_height_from_drm(guint32 drmfmt, guint32 height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
