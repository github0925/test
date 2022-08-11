#ifndef __DRM_HANDLE_H
#define __DRM_HANDLE_H


#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#define MAX_DMABUF_PLAINES 3
#define MAX_BUF_PLANES  3
#define DRM_HANDLE_MAGIC 0x12345678
#define MY_PID getpid()

typedef struct dm_native_array
{
    int num;
    int *data;
} dm_native_array_t;


typedef struct dm_handle
{
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

struct dm_drm_handle_t
{
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
    union
    {
        struct dm_drm_bo_t *data; /* pointer to struct yalloc_drm_bo_t */
        int64_t __padding;

    } __attribute__((aligned(8)));
};

struct dm_drm_t
{
    int fd;
    pthread_mutex_t mutex;
    struct dm_drm_drv_t *drv;
    unsigned int refcount;
};

struct dm_drm_bo_t
{
    struct dm_drm_t *drm;
    struct dm_drm_handle_t *handle;

    int imported; /* the handle is from a remote proces when true */
    unsigned int refcount;
};

struct dm_drm_drv_t
{
    /* destroy the driver */
    void (*destroy)(struct dm_drm_drv_t *drv);

    /* allocate or import a bo */
    struct dm_drm_bo_t *(*alloc)(struct dm_drm_t *drv,
                                 struct dm_drm_handle_t *handle);

    /* free a bo */
    void (*free)(struct dm_drm_t *drv,
                 struct dm_drm_bo_t *bo);

    /* map a bo for CPU access */
    int (*map)(struct dm_drm_t *drv,
               struct dm_drm_bo_t *bo,
               int x, int y, int w, int h, void **addr);

    /* unmap a bo */
    void (*unmap)(struct dm_drm_t *drv,
                  struct dm_drm_bo_t *bo);

    struct dm_drm_bo_t *(*import)(struct dm_drm_t *drm,
                                  struct dm_drm_handle_t *handle);
};


struct vendor_info
{
    struct dm_drm_drv_t base;

    int fd;
    int gen;

    uint32_t *batch, *cur;
    int capacity, size;
    int exec_blt;
};

#define DRM_DEVICE "/dev/dri/card0"

#define ALIGN(x, mask) (((x) + ((mask)-1)) & ~((mask)-1))


struct dm_drm_drv_t *dm_drm_drv_create_for_vendor(int fd);
struct dm_drm_t *dm_drm_create(void);
void dm_drm_destroy(struct dm_drm_t * drm);
struct dm_drm_bo_t *dm_drm_bo_create(struct dm_drm_t *drm, int width, int height, int format, int usage);
int dm_drm_bo_lock(struct dm_drm_bo_t *bo, int usage, int x, int y, int w, int h, void **addr);
void dm_drm_bo_unlock(struct dm_drm_bo_t *bo);
int create_arg_init(int format, int width, int height, struct drm_mode_create_dumb *create_arg);


#ifdef __cplusplus
}
#endif


#endif

