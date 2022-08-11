#ifndef __JPU_MIDDLEWARE_API_H__
#define __JPU_MIDDLEWARE_API_H__

#include "drmutils.h"
#include "jpuapifunc.h"

#define MAX_FRAME_BUFFER_NUM 19

typedef void (*fill_input_data)(void *private_ptr, int expect_size, void *input_buffer,
                                int *fill_input_size);
typedef void (*read_output_data)(void *private_ptr, void *output_buffer, int output_size);

typedef enum {
    PACK_FORMAT_NONE, // planar format
    PACK_FORMAT_NV12, // semi-planar format
    PACK_FORMAT_NV21, // semi-planar format
    PACK_FORMAT_YUYV, // packed format
    PACK_FORMAT_YVYU, // packed format
    PACK_FORMAT_UYVY, // packed format
    PACK_FORMAT_VYUY, // packed format
    PACK_FORMAT_AYUV  // packed format
} jpu_pack_format_t;

typedef enum {
    FEED_METHOD_FIXED_SIZE, // read a fixed data every time
    FEED_METHOD_FRAME_SIZE, // read a frame every time
    FEED_METHOD_MAX
} dec_feed_method_t;

typedef struct dec_output_info {
    int pic_width;
    int pic_height;
    FrameFormat pic_format;

    int output_width;  // hw 32-bit align
    int output_height; // hw 32-bit align
    int output_cbcrInterleave;
    int output_packed_format;
    FrameFormat output_format;
    int output_frame_size;

    int bit_depth;
    int current_output_index;
    int decode_finish;

    Uint8 *y_ptr;
    Uint8 *u_ptr;
    Uint8 *v_ptr;
    int y_size;
    int u_size;
    int v_size;
} dec_output_info;

typedef struct enc_input_info {
    // write by user
    int pic_width;
    int pic_height;

    // Source Format (0 : 4:2:0, 1 : 4:2:2, 2 : 4:4:0, 3 : 4:4:4, 4 : 4:0:0)
    FrameFormat frame_format;
    /*
        0-planar, 1-NV12,NV16(CbCr interleave) 2-NV21,NV61(CbCr alternative)
        3-YUYV, 4-UYVY, 5-YVYU, 6-VYUY, 7-YUV packed (444 only)
    */
    int packageFormat;

    // write by jpu
    int frameSize;
    int lumaSize;
    int chromaSize;
    int lumaLineWidth;
    int lumaHeight;
    int chromaLineWidth;
    int chromaHeight;
} enc_input_info;

typedef struct jpu_hw_instance {
    jpu_buffer_t bs_stream;
    fill_input_data fill_buffer_callback;
    read_output_data read_buffer_callback;

    dm_drm_t *drm_device;
    dm_drm_bo_t *bo;

    dec_output_info dec_output;
    dec_feed_method_t dec_feedong_method;
    int jpu_dec_init_success;
    jpu_buffer_t dec_frame_buffer_stream;
    FrameBuffer dec_frameBuf[MAX_FRAME_BUFFER_NUM];

    enc_input_info enc_input;
    int jpu_enc_init_success;
    jpu_buffer_t enc_frame_buffer_stream;
    FrameBuffer enc_frameBuf[MAX_FRAME_BUFFER_NUM];

    JpgInst *handle;
    void *user_private_ptr; // for user
} jpu_hw_instance;

jpu_hw_instance *jpu_hw_init();
int jpu_decompress(jpu_hw_instance *jpu);
int jpu_decompress_to_fd(jpu_hw_instance *jpu, int *yuv_fd);
int jpu_encompress(jpu_hw_instance *jpu);
int jpu_encompress_from_fd(jpu_hw_instance *jpu, int yuv_fd);
int jpu_get_output_buffer(jpu_hw_instance *jpu, unsigned char *output_buffer, int buffer_size);
int jpu_encompress_fill_input_buffer(jpu_hw_instance *jpu, unsigned char **y_ptr,
                                     unsigned char **u_ptr, unsigned char **v_ptr, int rows);
int jpu_hw_release(jpu_hw_instance *jpu);

#endif
