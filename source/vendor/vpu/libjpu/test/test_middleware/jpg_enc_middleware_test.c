#include "drmutils.h"
#include "jpu_middleware_api.h"
#include "jpuapi.h"
#include "jpuapifunc.h"
#include "jpulog.h"
#include "main_helper.h"
#include <getopt.h>

double GetNowMs();

typedef struct user_private {
    FILE *input_fp;
    int input_size;
    int read_input_over;

    FILE *output_fp;
} user_private;

void enc_fill_input_buffer(void *private_ptr, int expect_size, void *input_buffer, int *input_size)
{
    int ret = 0;
    user_private *private_data = (user_private *)private_ptr;

    ret = fread(input_buffer, 1, expect_size, private_data->input_fp);
    *input_size = ret;

    private_data->input_size -= ret;

    if (private_data->input_size <= 0)
        private_data->read_input_over = 1;

    return;
}

void read_output_buffer(void *private_ptr, void *output_buffer, int output_size)
{
    user_private *private_data = (user_private *)private_ptr;
    fwrite(output_buffer, 1, output_size, private_data->output_fp);
    return;
}

int get_drm_format_from_jpu_format(FrameFormat frame_format, jpu_pack_format_t packageFormat)
{
    int ret = -1;

    switch (frame_format) {
    case FORMAT_420:
        ret = DRM_FORMAT_YUV420;
        break;
    case FORMAT_422:
        if (packageFormat == PACK_FORMAT_NONE)
            ret = DRM_FORMAT_YUV422;
        else
            ret = DRM_FORMAT_YUYV;
        break;
    case FORMAT_440:
        ret = DRM_FORMAT_YUV422;
        break;
    case FORMAT_444:
        if (packageFormat == PACK_FORMAT_AYUV)
            ret = DRM_FORMAT_AYUV;
        else
            ret = DRM_FORMAT_YUV444;
        break;
    case FORMAT_400:
        ret = DRM_FORMAT_R8;
        break;
    default:
        break;
    }

    return ret;
}

BOOL TestEncoder(EncConfigParam *param)
{
    int ret = 0;
    jpu_hw_instance *jpu = NULL;
    user_private private_data;

    double ts_start = 0.0;
    double ts_end = 0.0;

    memset(&private_data, 0, sizeof(private_data));

    private_data.input_fp = fopen(param->yuvFileName, "rb");
    if (!private_data.input_fp) {
        perror("file isn't exist\n");
        ret = FALSE;
        goto DONE;
    }

    fseek(private_data.input_fp, 0, SEEK_END);
    private_data.input_size = ftell(private_data.input_fp);
    fseek(private_data.input_fp, 0, SEEK_SET);

    private_data.output_fp = fopen("output.jpg", "wb");
    if (!private_data.output_fp) {
        perror("fopen output.jpg error\n");
        ret = FALSE;
        goto DONE;
    }

    jpu = jpu_hw_init();
    if (!jpu) {
        perror("jpu hw init failed\n");
        ret = FALSE;
        goto DONE;
    }

    jpu->user_private_ptr = &private_data;
    jpu->fill_buffer_callback = enc_fill_input_buffer;
    jpu->read_buffer_callback = read_output_buffer;
    jpu->enc_input.pic_width = param->picWidth;          // 227;
    jpu->enc_input.pic_height = param->picHeight;        // 149;
    jpu->enc_input.frame_format = param->frameFormat;    // FORMAT_444;
    jpu->enc_input.packageFormat = param->packageFormat; // 0;

    if (!param->useDMABuf) {
        // encode
        while (!private_data.read_input_over) {
            ts_start = GetNowMs();
            ret = jpu_encompress(jpu);
            if (ret < 0) {
                perror("jpu encompress failed\n");
                break;
            }
        }
    } else {
        // encode from dma fd
        jpu->fill_buffer_callback = NULL;
        {
            void *buffer_ptr = NULL;
            dm_drm_t *drm_device = dm_drm_create();

            // for this drm test, the picwidth need 8-byte alignment
            dm_drm_bo_t *bo = dm_drm_bo_create(
                drm_device, param->picWidth, param->picHeight,
                get_drm_format_from_jpu_format(param->frameFormat, param->packageFormat), 0);

            dm_drm_bo_lock(bo, 0, 0, 0, param->picWidth, param->picHeight, &buffer_ptr);

            while (!private_data.read_input_over) {
                if (private_data.input_size < bo->handle->size) {
                    perror("The size of the input file does not match the parameters\n");
                    goto DMA_TEST_DONE;
                }

                ret = fread(buffer_ptr, 1, bo->handle->size, private_data.input_fp);
                private_data.input_size -= ret;

                if (private_data.input_size <= 0)
                    private_data.read_input_over = 1;

                ts_start = GetNowMs();
                ret = jpu_encompress_from_fd(jpu, bo->handle->prime_fd);
                if (ret < 0) {
                    perror("jpu encompress failed\n");
                    break;
                }
            }

        DMA_TEST_DONE:
            if (bo) {
                dm_drm_bo_unlock(bo);
                dm_drm_bo_destroy(bo);
            }

            if (drm_device)
                dm_drm_destroy(drm_device);
        }
    }

    ts_end = GetNowMs();

    printf("Time consuming:%f ms\n", ts_end - ts_start);

    // test jpu_encompress_fill_input_buffer api
    // {
    //     unsigned char *input_y = (unsigned char *)malloc(param->picWidth * param->picHeight);
    //     unsigned char *input_u = (unsigned char *)malloc(param->picWidth * param->picHeight);
    //     unsigned char *input_v = (unsigned char *)malloc(param->picWidth * param->picHeight);

    //     fread(input_y, 1, param->picWidth * param->picHeight, private_data.input_fp);
    //     fread(input_u, 1, param->picWidth * param->picHeight, private_data.input_fp);
    //     fread(input_v, 1, param->picWidth * param->picHeight, private_data.input_fp);

    //     for (int i = 0; i < param->picHeight; i++) {
    //         unsigned char *current_y = input_y + i * param->picWidth;
    //         unsigned char *current_u = input_u + i * param->picWidth;
    //         unsigned char *current_v = input_v + i * param->picWidth;

    //         ret = jpu_encompress_fill_input_buffer(jpu, &current_y, &current_u, &current_v, 1);

    //         if (ret < 0 && ret != -2) {
    //             perror("jpu_encompress_fill_input_buffer error\n");
    //             break;
    //         }
    //     }

    //     free(input_y);
    //     free(input_u);
    //     free(input_v);
    // }

DONE:
    if (private_data.input_fp)
        fclose(private_data.input_fp);

    if (private_data.output_fp)
        fclose(private_data.output_fp);

    if (jpu)
        jpu_hw_release(jpu);
    return ret;
}
