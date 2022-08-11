#ifndef __GST_DRM_UTILS_H__
#define __GST_DRM_UTILS_H__

#include <drm/drm.h>
#include <drm/drm_fourcc.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
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
/**********************************************************/
#define MAX_DMABUF_PLAINES 4
#define MAX_BUF_PLANES 4

typedef struct dm_native_array {
    int num;
    int *data;
} dm_native_array_t;

typedef struct dm_handle {
    /**
     * @brief dm hal flag
     */
    int flags;
    /**
     * @brief virtual address
     */
    uint64_t addr;
    /**
     *@brief store attribute values. for example: pid .. etc
     */
    dm_native_array_t attributes;
    /**
     * @brief handled by dm hal, framework should not touch it.
     */
    void *native_hdl;

} dm_handle_t;

typedef struct dm_drm_handle_t {
    dm_handle_t dm_hdl;
    /* file descriptors */
    int prime_fd;
    unsigned int bo_handle;

    int magic;

    int width;
    int height;
    int format;
    int usage;
    int drm_format;
    int data_owner; /* owner of data (for validation) */
    int size;
    int n_planes;
    int buf_fd[MAX_DMABUF_PLAINES];
    int offset[MAX_DMABUF_PLAINES];
    int stride[MAX_DMABUF_PLAINES];
    union {
        struct dm_drm_bo_t *data; /* pointer to struct yalloc_drm_bo_t */
        int64_t __padding;

    } __attribute__((aligned(8)));
} dm_drm_handle_t;

#define DRM_HANDLE_MAGIC 0x12345678
#define MY_PID getpid()

typedef struct dm_drm_bo_t {
    struct dm_drm_t *drm;
    struct dm_drm_handle_t *handle;

    int imported; /* the handle is from a remote proces when true */
    unsigned int refcount;
} dm_drm_bo_t;

typedef struct dm_drm_drv_t {
    /* destroy the driver */
    void (*destroy)(struct dm_drm_drv_t *drv);

    /* allocate or import a bo */
    struct dm_drm_bo_t *(*alloc)(struct dm_drm_t *drv, struct dm_drm_handle_t *handle);

    /* free a bo */
    void (*free)(struct dm_drm_t *drv, struct dm_drm_bo_t *bo);

    /* map a bo for CPU access */
    int (*map)(struct dm_drm_t *drv, struct dm_drm_bo_t *bo, int x, int y, int w, int h,
               void **addr);

    /* unmap a bo */
    void (*unmap)(struct dm_drm_t *drv, struct dm_drm_bo_t *bo);

    struct dm_drm_bo_t *(*import)(struct dm_drm_t *drm, struct dm_drm_handle_t *handle);
} dm_drm_drv_t;

typedef struct dm_drm_t {
    int fd;
    pthread_mutex_t mutex;
    struct dm_drm_drv_t *drv;
    unsigned int refcount;
} dm_drm_t;

struct dm_drm_drv_t *dm_drm_drv_create_for_vendor(int fd);

/**********************************************************/

/**********************************************************/
typedef struct dm_meta_data {
    int fds[MAX_BUF_PLANES];          /** file descriptors of the buffer*/
    int n_planes;                     /** number of planes of the buffer*/
    uint32_t width;                   /** width of the buffer*/
    uint32_t height;                  /** height of the buffer*/
    uint32_t format;                  /** format of the buffer*/
    uint32_t strides[MAX_BUF_PLANES]; /** strides of the buffer*/
    uint32_t offsets[MAX_BUF_PLANES]; /** offsets of the buffer*/
    uint32_t size;                    /** size of the buffer*/
} dm_meta_data;

struct dm_drm_t *dm_drm_create(void);
int dm_drm_destroy(dm_drm_t *drm);
dm_handle_t *get_dm_target(struct dm_drm_bo_t *bo);
struct dm_drm_bo_t *dm_drm_bo_create(struct dm_drm_t *drm, int width, int height, int format,
                                     int usage);
void dm_drm_bo_destroy(struct dm_drm_bo_t *bo);
int dm_drm_bo_lock(struct dm_drm_bo_t *bo, int usage, int x, int y, int w, int h, void **addr);
void dm_drm_bo_unlock(struct dm_drm_bo_t *bo);
struct dm_drm_bo_t *dm_drm_bo_create_from_fd(struct dm_drm_t *drm, struct dm_meta_data *data);

#endif
