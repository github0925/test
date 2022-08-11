#include "drmutils.h"
#include "jpu_middleware_api.h"
#include "main_helper.h"
#include "time.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

double GetNowMs()
{
    double curr = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    curr = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000.0;
    curr /= 1000.0;

    return curr;
}

void dec_fill_input_buffer(void *private_ptr, int expect_size, void *input_buffer,
                           int *fill_input_size)
{
    int ret = 0;
    FILE *input_fp = (FILE *)private_ptr;

    ret = fread(input_buffer, 1, expect_size, input_fp);
    *fill_input_size = ret;

    return;
}

BOOL TestDecoder(DecConfigParam *param)
{
    int ret = 0;
    unsigned char *output_buffer = NULL;
    jpu_hw_instance *jpu = NULL;

    double ts_start = 0.0;
    double ts_end = 0.0;

    FILE *input_fp = fopen(param->bitstreamFileName, "rb");

    if (!input_fp) {
        perror("file isn't exist\n");
        ret = FALSE;
        goto DONE;
    }

    // init
    jpu = jpu_hw_init();
    if (!jpu) {
        perror("jpu hw init failed\n");
        ret = FALSE;
        goto DONE;
    }

    jpu->user_private_ptr = input_fp;
    jpu->fill_buffer_callback = dec_fill_input_buffer;
    jpu->dec_feedong_method = FEED_METHOD_FIXED_SIZE; // defalut set to FEEDING_METHOD_FIXED_SIZE
    jpu->dec_output.output_cbcrInterleave = param->cbcrInterleave; // defalut set to CBCR_SEPARATED
    jpu->dec_output.output_packed_format = param->packedFormat; // defalut set to PACKED_FORMAT_NONE
    // Only valid when output_packed_format is set to PACKED_FORMAT_NONE
    jpu->dec_output.output_format = param->subsample; // defalut set to FORMAT_444

    ts_start = GetNowMs();

    // decode
    while (!jpu->dec_output.decode_finish) {
        FILE *output_fp;
        int output_fd = 0;
        static int once = 0;

        if (once == 0) {
            once = 1;
            // output_buffer = (unsigned char *)malloc(sizeof(unsigned char) *
            // jpu->dec_output.output_frame_size);
            output_fp = fopen("./output.yuv", "wb");
            fclose(output_fp);
        }

        if (param->useDMABuf) {
            ret = jpu_decompress_to_fd(jpu, &output_fd);
            if (ret < 0) {
                perror("jpu decompress failed\n");
                break;
            }

            if (jpu->dec_output.current_output_index >= 0) {
                printf("[%s][%d] pic_width:%d pic_height:%d pic_format:%d output_width:%d "
                       "output_height:%d output_cbcrInterleave:%d "
                       "output_packed_format:%d output_format_size:%d jpu->dec_output.bit_depth:%d "
                       "y_size:%d u_size:%d v_size:%d\n",
                       __func__, __LINE__, jpu->dec_output.pic_width, jpu->dec_output.pic_height,
                       jpu->dec_output.pic_format, jpu->dec_output.output_width,
                       jpu->dec_output.output_height, jpu->dec_output.output_cbcrInterleave,
                       jpu->dec_output.output_packed_format, jpu->dec_output.output_frame_size,
                       jpu->dec_output.bit_depth, jpu->dec_output.y_size, jpu->dec_output.u_size,
                       jpu->dec_output.v_size);
                printf("dec output fd:%d\n", output_fd);

                void *output_buffer = NULL;
                dm_drm_bo_lock(jpu->bo, 0, 0, 0, jpu->dec_output.output_width,
                               jpu->dec_output.output_height, &output_buffer);

                output_fp = fopen("output.yuv", "ab");
                fwrite(output_buffer, 1, jpu->dec_output.output_frame_size, output_fp);
                fclose(output_fp);

                dm_drm_bo_unlock(jpu->bo);
            }
        } else {
            ret = jpu_decompress(jpu);
            if (ret < 0) {
                perror("jpu decompress failed\n");
                break;
            }

            if (jpu->dec_output.current_output_index >= 0) {
                printf("[%s][%d] pic_width:%d pic_height:%d pic_format:%d output_width:%d "
                       "output_height:%d output_cbcrInterleave:%d "
                       "output_packed_format:%d output_format_size:%d jpu->dec_output.bit_depth:%d "
                       "y_size:%d u_size:%d v_size:%d\n",
                       __func__, __LINE__, jpu->dec_output.pic_width, jpu->dec_output.pic_height,
                       jpu->dec_output.pic_format, jpu->dec_output.output_width,
                       jpu->dec_output.output_height, jpu->dec_output.output_cbcrInterleave,
                       jpu->dec_output.output_packed_format, jpu->dec_output.output_frame_size,
                       jpu->dec_output.bit_depth, jpu->dec_output.y_size, jpu->dec_output.u_size,
                       jpu->dec_output.v_size);

                // 1.get output buffer method one
                // jpu_get_output_buffer(jpu, output_buffer, jpu->dec_output.output_frame_size);

                // {
                //     output_fp = fopen("./output.yuv", "ab");
                //     fwrite(output_buffer, 1, jpu->dec_output.output_frame_size, output_fp);
                //     fclose(output_fp);
                // }

                // 2.get output buffer method two
                if (jpu->dec_output.y_ptr) {
                    output_fp = fopen("./output.yuv", "ab");
                    fwrite(jpu->dec_output.y_ptr, 1, jpu->dec_output.y_size, output_fp);
                    fclose(output_fp);
                }
                if (jpu->dec_output.u_ptr) {
                    output_fp = fopen("./output.yuv", "ab");
                    fwrite(jpu->dec_output.u_ptr, 1, jpu->dec_output.u_size, output_fp);
                    fclose(output_fp);
                }
                if (jpu->dec_output.v_ptr) {
                    output_fp = fopen("./output.yuv", "ab");
                    fwrite(jpu->dec_output.v_ptr, 1, jpu->dec_output.v_size, output_fp);
                    fclose(output_fp);
                }
            }
        }
    }
    ts_end = GetNowMs();

    printf("Time consuming:%f ms\n", ts_end - ts_start);

DONE:
    if (jpu)
        jpu_hw_release(jpu);

    if (output_buffer)
        free(output_buffer);

    if (input_fp)
        fclose(input_fp);

    return ret;
}
