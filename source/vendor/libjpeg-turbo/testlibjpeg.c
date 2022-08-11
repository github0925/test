#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#include "jpeglib.h"
#include "jerror.h"

struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

double GetNowMs()
{
    double curr = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    curr = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000.0;
    curr /= 1000.0;

    return curr;
}

void my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr)cinfo->err;

    (*cinfo->err->output_message) (cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

//compress to jpeg
void write_jpeg_file(JSAMPLE * image_buffer, char *filename, int quality,
    int image_width, int image_height, int is_use_hw_codec)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *outfile;
    JSAMPROW row_pointer[1]; //pointer to JSAMPLE row[s]
    int row_stride;
    unsigned char * output_buffer = NULL;
    int output_size = 0x200000; //2M

    /* Step 1: allocate and initialize JPEG compression object */
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    if ((outfile = fopen(filename, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", filename);
        exit(1);
    }

    //Associate some operations of writing files to cinfo->dest,
    //compress and write to io file internally by libjpeg
    jpeg_stdio_dest(&cinfo, outfile);

    //test for write to mem
    // {
    //     output_buffer = (unsigned char *)malloc(output_size); //2M
    //     jpeg_mem_dest(&cinfo, &output_buffer, &output_size);
    // }

    /* Step 3: set parameters for compression */
    cinfo.image_width = image_width;
    cinfo.image_height = image_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB; //input size and format
#ifdef WITH_JPU_HW
    if (is_use_hw_codec)
        cinfo.hw_encompress = TRUE;
#endif

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    /* Step 4: Start compressor */
    jpeg_start_compress(&cinfo, TRUE);

    /* Step 5: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */
    row_stride = image_width * 3;  //a raw data size

    double dec_start_ts = 0.0;
    double dec_end_ts = 0.0;
    dec_start_ts = GetNowMs();
    while (cinfo.next_scanline < cinfo.image_height) {
        //The starting address of each line of raw data
        row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    dec_end_ts = GetNowMs();
    printf("[%s][%d] encode time=%.1fms\n",
        __func__, __LINE__, dec_end_ts - dec_start_ts);

    /* Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);
    fclose(outfile);

    if (output_buffer)
    {
        FILE * fp = fopen("output_from_mem.jpg", "wb");
        fwrite(output_buffer, 1, output_size, fp);
        fclose(fp);
        free(output_buffer);
    }

    /* Step 7: release JPEG compression object */
    jpeg_destroy_compress(&cinfo);
}


//write decompress data to file
int put_scanline_someplace(JSAMPROW buffer, int row_stride)
{
    static int once = 0;
    if (!once)
    {
        once = 1;
        FILE *fp = fopen("./output.RAW", "wb");
        fclose(fp);
    }

    FILE * fp = fopen("./output.RAW", "ab");
    fwrite(buffer, row_stride, 1 , fp);
    fclose(fp);

    return 0;
}

//decompress from jpeg
int read_jpeg_file(char* filename, int is_use_hw_codec)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE * infile; //src file
    JSAMPARRAY buffer;  //output row buffer
    int row_stride;		/* physical row width in output buffer */

    if ((infile = fopen(filename, "rb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", filename);
        return 0;
    }

    /* Step 1: allocate and initialize JPEG decompression object */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;  //overloaded error exit process

    if (setjmp(jerr.setjmp_buffer))
    {
        //setjmp is to get the variables defined by the current context
        fprintf(stderr, "enter set jmp, decompress error\n");
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return 0;
    }

    jpeg_create_decompress(&cinfo);  //cinfo->global_state = DSTATE_START; //

    /* Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(&cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */
    jpeg_read_header(&cinfo, TRUE); //will set defalut params

    /* Step 4: set parameters for decompression */
    //cinfo.out_color_space = JCS_RGB;  //JCS_YCnCr
    //The input parameters here are obtained from the jpeg head.
    //The width and height of the output are the same as the input.
    //The format and stride also use the default values, so nothing is set.
#ifdef WITH_JPU_HW
    if (is_use_hw_codec)
        cinfo.hw_decompress = TRUE;
#endif

    /* Step 5: Start decompressor */
    jpeg_start_decompress(&cinfo);

    //output_components is generally equal to out_color_space, color components in pixel units
    row_stride = cinfo.output_width * cinfo.output_components;
    //Allocate a buffer of the size of row stride
    buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    double dec_start_ts = 0.0;
    double dec_end_ts = 0.0;
    dec_start_ts = GetNowMs();
    while (cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        put_scanline_someplace(buffer[0], row_stride);
    }
    dec_end_ts = GetNowMs();

    printf("[%s][%d] output size:%dx%d, image size:%dx%d decode time=%.1fms\n",
        __func__, __LINE__, cinfo.output_width, cinfo.output_height, cinfo.image_width, cinfo.image_height,
        dec_end_ts - dec_start_ts);

    /* Step 7: Finish decompression */
    jpeg_finish_decompress(&cinfo);

    /* Step 8: Release JPEG decompression object */
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);
    return 1;
}

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        #include <sys/types.h>
        printf("at least three parameters\n"
            "\tfor decoder, such as: testlibjpeg output.jpg 1(is use hw codec?)\n"
            "\tfor encoder, such as: testlibjpeg output.RGB 227(width) 149(height) 1(is use hw codec?)\n");
        return 0;
    }

    if (strlen(argv[1]) < 4)
    {
        printf("invalid input file named:%s\n", argv[1]);
        return 0;
    }

    printf("libjpeg version:%d\n", JPEG_LIB_VERSION);

    if (!strncmp(argv[1] + strlen(argv[1]) - 4, ".jpg", 4))
    {
        printf("file: %s is jpg file\n", argv[1]);

        int is_use_hw_codec = atoi(argv[2]);
        read_jpeg_file(argv[1], is_use_hw_codec);
    } else {
        printf("file: %s is rgb file\n", argv[1]);

        if (argc < 5)
        {
          printf("for encode, need 5 paramertes, "
            "such as: testlibjpeg output.RGB 227(width) 149(height) 0\n");
          return 0;
        }

        int width = atoi(argv[2]);
        int height = atoi(argv[3]);
        int is_use_hw_codec = atoi(argv[4]);
        unsigned char * in_buffer = (unsigned char *)malloc(width * height * 3);
        {
            FILE *fp = fopen(argv[1], "r");
            if (fp == NULL)
            {
                fprintf(stderr, "can't open %s\n", argv[1]);
                exit(1);
            }
            fread(in_buffer, width * height * 3, 1, fp);
            fclose(fp);
        }
        write_jpeg_file(in_buffer, "output.jpg", 90, width, height, is_use_hw_codec);
        free(in_buffer);
    }

    printf("test over\n");
    return 0;
}