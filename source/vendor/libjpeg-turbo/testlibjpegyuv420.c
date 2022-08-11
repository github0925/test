#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <jerror.h>

#define QUALITY 80
#define OUTPUT_FILE_NAME "output_yuv420.jpg"

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

//decompress from jpeg
int read_jpeg_yuv420p_memory(char* filename, int is_use_hw_codec)
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
    //jpeg_stdio_src(&cinfo, infile);
    for (int i = 0; i < 2; i++)
    {
      fseek(infile, 0, SEEK_END);
      int buffer_size = ftell(infile);
      fseek(infile, 0, SEEK_SET);
      unsigned char *input_buff = malloc(buffer_size);
      fread(input_buff, 1, buffer_size, infile);

      jpeg_mem_src(&cinfo, input_buff, buffer_size);

      /* Step 3: read file parameters with jpeg_read_header() */
      jpeg_read_header(&cinfo, TRUE); //will set defalut params

      /* Step 4: set parameters for decompression */
      //cinfo.out_color_space = JCS_RGB;  //JCS_YCnCr
      //The input parameters here are obtained from the jpeg head.
      //The width and height of the output are the same as the input.
      //The format and stride also use the default values, so nothing is set.
      cinfo.out_color_space = JCS_YCbCr;
      cinfo.raw_data_out = TRUE;
#ifdef WITH_JPU_HW
      if (is_use_hw_codec)
        cinfo.hw_decompress = TRUE;
#endif

      /* Step 5: Start decompressor */
      jpeg_start_decompress(&cinfo);
      printf("[%s][%d] \n", __func__, __LINE__);
      printf("cinfo.output_width:%d  cinfo.output_height:%d "
             "cinfo.image_width:%d cinfo.image_height:%d\n",
             cinfo.output_width, cinfo.output_height,
             cinfo.image_width, cinfo.image_height);

      if (cinfo.comp_info[0].h_samp_factor != 2 ||
          cinfo.comp_info[0].v_samp_factor != 2 ||
          cinfo.comp_info[1].h_samp_factor != 1 ||
          cinfo.comp_info[1].v_samp_factor != 1 ||
          cinfo.comp_info[2].h_samp_factor != 1 ||
          cinfo.comp_info[2].v_samp_factor != 1)
      {
        printf("only test for yuv420 output\n");
        printf("only test for yuv420 output cinfo.comp_info[0].h_samp_factor:%d "
               "cinfo.comp_info[0].v_samp_factor:%d cinfo.comp_info[1].h_samp_factor:%d "
               "cinfo.comp_info[1].v_samp_factor:%d cinfo.comp_info[2].h_samp_factor:%d "
               "cinfo.comp_info[2].v_samp_factor:%d\n",
               cinfo.comp_info[0].h_samp_factor, cinfo.comp_info[0].v_samp_factor,
               cinfo.comp_info[1].h_samp_factor, cinfo.comp_info[1].v_samp_factor,
               cinfo.comp_info[2].h_samp_factor, cinfo.comp_info[2].v_samp_factor);
        exit(1);
      }

      unsigned char *output_buffer = malloc(cinfo.output_width * cinfo.output_height * 3 / 2);
      unsigned char *output_y = output_buffer;
      unsigned char *output_u = output_buffer + cinfo.output_width * cinfo.output_height;
      unsigned char *output_v = output_buffer + cinfo.output_width * cinfo.output_height * 5 / 4;
      unsigned char *last_addr = output_buffer + cinfo.output_width * cinfo.output_height * 3 / 2;

      unsigned char *dec_scratch = malloc(cinfo.output_width);

      unsigned char **line[3];
      unsigned char *y[4 * DCTSIZE] = {
          NULL,
      };
      unsigned char *u[4 * DCTSIZE] = {
          NULL,
      };
      unsigned char *v[4 * DCTSIZE] = {
          NULL,
      };

      line[0] = y;
      line[1] = u;
      line[2] = v;

      double dec_start_ts = 0.0;
      double dec_end_ts = 0.0;
      dec_start_ts = GetNowMs();

      for (int i = 0; i < cinfo.output_height; i += cinfo.comp_info[0].v_samp_factor * DCTSIZE)
      {
        for (int j = 0; j < (cinfo.comp_info[0].v_samp_factor * DCTSIZE); j++)
        {
          //Y
          line[0][j] = output_y + (i + j) * cinfo.output_width;
          if (line[0][j] >= output_u)
          {
            line[0][j] = dec_scratch;
          }
          //U
          if (j < cinfo.comp_info[1].v_samp_factor * DCTSIZE)
          {
            line[1][j] = output_u + (i / 2 + j) * (cinfo.output_width / 2);
            if (line[1][j] >= output_v)
            {
              line[1][j] = dec_scratch;
            }
          }
          //V
          if (j < cinfo.comp_info[2].v_samp_factor * DCTSIZE)
          {
            line[2][j] = output_v + (i / 2 + j) * (cinfo.output_width / 2);
            if (line[2][j] >= last_addr)
            {
              line[2][j] = dec_scratch;
            }
          }
        }
        jpeg_read_raw_data(&cinfo, line, cinfo.comp_info[0].v_samp_factor * DCTSIZE);
      }

      dec_end_ts = GetNowMs();

      printf("[%s][%d] output size:%dx%d, image size:%dx%d decode time=%.1fms\n",
             __func__, __LINE__, cinfo.output_width, cinfo.output_height, cinfo.image_width, cinfo.image_height,
             dec_end_ts - dec_start_ts);

      FILE *fp = fopen("./output.RAW", "wb");
      fwrite(output_buffer, 1, cinfo.output_width * cinfo.output_height * 3 / 2, fp);
      fclose(fp);
      free(output_buffer);
      free(dec_scratch);

      if (input_buff)
        free(input_buff);

      /* Step 7: Finish decompression */
      jpeg_finish_decompress(&cinfo);
    }
    /* Step 8: Release JPEG decompression object */
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);
    return 1;
}


/* put_jpeg_yuv420p_memory converts an input image in the YUV420P format into a jpeg image and puts
 * it in a memory buffer.
 * - input_image is the image in YUV420P format.
 * - width and height are the dimensions of the image
 * Returns buffer size of jpeg image
 */
static int put_jpeg_yuv420p_memory(unsigned char *input_image, int width, int height, int is_use_hw_codec)
{
    int i, j, jpeg_image_size;

    FILE *outfile;
    JSAMPROW y[16], cb[16], cr[16]; // y[2][5] = color sample of row 2 and pixel column 5; (one plane)
    JSAMPARRAY data[3];             // t[0][2][5] = color sample 0 of row 2 and column 5

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    data[0] = y;
    data[1] = cb;
    data[2] = cr;

    cinfo.err = jpeg_std_error(&jerr); // errors get written to stderr

    jpeg_create_compress(&cinfo);

    for (int test_count = 0; test_count < 2; test_count++)
    {
      if ((outfile = fopen(OUTPUT_FILE_NAME, "wb")) == NULL)
      {
        fprintf(stderr, "can't open %s\n", OUTPUT_FILE_NAME);
        exit(1);
      }

      jpeg_stdio_dest(&cinfo, outfile);

      cinfo.image_width = width;
      cinfo.image_height = height;
      cinfo.input_components = 3;
#ifdef WITH_JPU_HW
      if (is_use_hw_codec)
        cinfo.hw_encompress = TRUE;
#endif
      jpeg_set_defaults(&cinfo);

      jpeg_set_colorspace(&cinfo, JCS_YCbCr);

      cinfo.raw_data_in = TRUE; // supply downsampled data
#if JPEG_LIB_VERSION >= 70
      cinfo.do_fancy_downsampling = FALSE; // fix segfaulst with v7
#endif
      cinfo.comp_info[0].h_samp_factor = 2;
      cinfo.comp_info[0].v_samp_factor = 2;
      cinfo.comp_info[1].h_samp_factor = 1;
      cinfo.comp_info[1].v_samp_factor = 1;
      cinfo.comp_info[2].h_samp_factor = 1;
      cinfo.comp_info[2].v_samp_factor = 1;

      jpeg_set_quality(&cinfo, QUALITY, TRUE);
      cinfo.dct_method = JDCT_FASTEST;

      jpeg_start_compress(&cinfo, TRUE);

      double dec_start_ts = 0.0;
      double dec_end_ts = 0.0;
      dec_start_ts = GetNowMs();

      for (j = 0; j < height; j += 16)
      {
        for (i = 0; i < 16; i++)
        {
          y[i] = input_image + width * (i + j);
          if (i % 2 == 0)
          {
            cb[i / 2] = input_image + width * height + width / 2 * ((i + j) / 2);
            cr[i / 2] = input_image + width * height + width * height / 4 + width / 2 * ((i + j) / 2);
          }
        }
        jpeg_write_raw_data(&cinfo, data, 16);
      }

      dec_end_ts = GetNowMs();
      printf("[%s][%d]  encode time=%.1fms\n",
             __func__, __LINE__, dec_end_ts - dec_start_ts);

      jpeg_finish_compress(&cinfo);
      fclose(outfile);
    }
    jpeg_destroy_compress(&cinfo);

    return jpeg_image_size;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        #include <sys/types.h>
        printf("at least three parameters\n"
            "\tfor decoder, such as: testlibjpeg output.jpg 1(is use hw codec?)\n"
            "\tfor encoder, such as: testlibjpeg output.yuv 1280(width) 720(height) 1(is use hw codec?)\n");
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
        read_jpeg_yuv420p_memory(argv[1], is_use_hw_codec);
    } else {
        printf("file: %s is yuv file\n", argv[1]);

        if (argc < 5)
        {
          printf("for encode, need 5 paramertes, "
            "such as: testlibjpeg output.yuv 227(width) 149(height) 0\n");
          return 0;
        }

        int width = atoi(argv[2]);
        int height = atoi(argv[3]);
        int is_use_hw_codec = atoi(argv[4]);
        unsigned char * in_buffer = (unsigned char *)malloc(width * height * 3 / 2);
        {
            FILE *fp = fopen(argv[1], "r");
            if (fp == NULL)
            {
                fprintf(stderr, "can't open %s\n", argv[1]);
                exit(1);
            }
            fread(in_buffer, width * height * 3 / 2, 1, fp);
            fclose(fp);
        }
        put_jpeg_yuv420p_memory(in_buffer, width, height, is_use_hw_codec);
        free(in_buffer);
    }

    printf("test over\n");
    return 0;
}