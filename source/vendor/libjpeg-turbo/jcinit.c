/*
 * jcinit.c
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains initialization logic for the JPEG compressor.
 * This routine is in charge of selecting the modules to be executed and
 * making an initialization call to each one.
 *
 * Logically, this code belongs in jcmaster.c.  It's split out because
 * linking this routine implies linking the entire compression library.
 * For a transcoding-only application, we want to be able to use jcmaster.c
 * without linking in the whole library.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#ifdef WITH_JPU_HW
void read_output_buffer (void * private_ptr, void * output_buffer, int output_size)
{
  int write_size = 0;
  int offset = 0;
  j_compress_ptr cinfo = (j_compress_ptr)private_ptr;

  while (output_size > 0) {
    write_size = (cinfo->dest->free_in_buffer < output_size)
                     ? cinfo->dest->free_in_buffer
                     : output_size;
    memcpy(cinfo->dest->next_output_byte, output_buffer + offset, write_size);

    offset += write_size;
    cinfo->dest->free_in_buffer -= write_size;
    output_size -= write_size;
    cinfo->dest->next_output_byte = cinfo->dest->next_output_byte + write_size;

    if (cinfo->dest->free_in_buffer == 0)
      cinfo->dest->empty_output_buffer(cinfo);
  }

  // static FILE * fp = NULL;
  // if (fp == NULL)
  //     fp = fopen("read_output_buffer.jpg", "wb");
  // fwrite(output_buffer, 1, output_size, fp);
  return;
}
#endif

/*
 * Master selection of compression modules.
 * This is done once at the start of processing an image.  We determine
 * which modules will be used and give them appropriate initialization calls.
 */

GLOBAL(void)
jinit_compress_master(j_compress_ptr cinfo)
{
  /* Initialize master control (includes parameter checking/processing) */
  jinit_c_master_control(cinfo, FALSE /* full compression */);

#ifdef WITH_JPU_HW
  if (cinfo->hw_encompress)
  {
    if (cinfo->arith_code)
    {
      cinfo->hw_encompress = FALSE;
      WARNMS(cinfo, JWRN_HW_ARITH);
    }

    if (cinfo->jpeg_color_space != JCS_YCbCr)
    {
      cinfo->hw_encompress = FALSE;
      WARNMS(cinfo, JWRN_HW_COLORSPACE);
    }
  }
#endif

  /* Preprocessing */
  if (!cinfo->raw_data_in)
  {
#ifdef WITH_JPU_HW
    if (cinfo->hw_encompress)
    {
      if (cinfo->max_h_samp_factor != 1 ||
          cinfo->max_v_samp_factor != 1)
      {
        //jpu input buffer should be yuv444,
        //so that libjpeg does not need to do downsampling
        cinfo->hw_encompress = FALSE;
        WARNMS(cinfo, JWRN_HW_SAMP_FACTOR);
      }

      if (cinfo->hw_encompress)
      {
        if (cinfo->jpu)
          jpu_hw_release(cinfo->jpu);

        cinfo->jpu = jpu_hw_init();

        if (!cinfo->jpu)
        {
          cinfo->hw_encompress = FALSE;
          WARNMS(cinfo, JWRN_HW_INIT);
        }
        else
        {
          cinfo->jpu->user_private_ptr = cinfo;
          cinfo->jpu->fill_buffer_callback = NULL;
          cinfo->jpu->read_buffer_callback = read_output_buffer;  //This architecture may need to be changed
          cinfo->jpu->enc_input.pic_width = cinfo->image_width;   //227;
          cinfo->jpu->enc_input.pic_height = cinfo->image_height; //149;
          cinfo->jpu->enc_input.frame_format = FORMAT_444;        // FORMAT_444;
          cinfo->jpu->enc_input.packageFormat = 0;
        }
      }
    }
#endif

    jinit_color_converter(cinfo);
    jinit_downsampler(cinfo);
    jinit_c_prep_controller(cinfo, FALSE /* never need full buffer here */);
  }
#ifdef WITH_JPU_HW
  else
  {
    if (cinfo->hw_encompress)
    {
      if (cinfo->input_components != 3)
      {
        cinfo->hw_encompress = FALSE;
        WARNMS(cinfo, JWRN_HW_SAMP_FACTOR);
      }
      else if (cinfo->comp_info[0].h_samp_factor != 2 ||
               cinfo->comp_info[0].v_samp_factor != 2 ||
               cinfo->comp_info[1].h_samp_factor != 1 ||
               cinfo->comp_info[1].v_samp_factor != 1 ||
               cinfo->comp_info[2].h_samp_factor != 1 ||
               cinfo->comp_info[2].v_samp_factor != 1)
      {
        cinfo->hw_encompress = FALSE;
        WARNMS(cinfo, JWRN_HW_SAMP_FACTOR);
      }

      if (cinfo->hw_encompress)
      {
        if (cinfo->jpu)
          jpu_hw_release(cinfo->jpu);

        cinfo->jpu = jpu_hw_init();

        if (!cinfo->jpu)
        {
          cinfo->hw_encompress = FALSE;
          WARNMS(cinfo, JWRN_HW_INIT);
        }
        else
        {
          cinfo->jpu->user_private_ptr = cinfo;
          cinfo->jpu->fill_buffer_callback = NULL;
          cinfo->jpu->read_buffer_callback = read_output_buffer;  //This architecture may need to be changed
          cinfo->jpu->enc_input.pic_width = cinfo->image_width;   //227;
          cinfo->jpu->enc_input.pic_height = cinfo->image_height; //149;
          cinfo->jpu->enc_input.frame_format = FORMAT_420;        // FORMAT_420;
          cinfo->jpu->enc_input.packageFormat = 0;
        }
      }
    }
  }
#endif

  /* Forward DCT */
  jinit_forward_dct(cinfo);
  /* Entropy encoding: either Huffman or arithmetic coding. */
  if (cinfo->arith_code) {
#ifdef C_ARITH_CODING_SUPPORTED
    jinit_arith_encoder(cinfo);
#else
    ERREXIT(cinfo, JERR_ARITH_NOTIMPL);
#endif
  } else {
    if (cinfo->progressive_mode) {
#ifdef C_PROGRESSIVE_SUPPORTED
      jinit_phuff_encoder(cinfo);
#else
      ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
    } else
      jinit_huff_encoder(cinfo);
  }

  /* Need a full-image coefficient buffer in any multi-pass mode. */
  jinit_c_coef_controller(cinfo, (boolean)(cinfo->num_scans > 1 ||
                                           cinfo->optimize_coding));
  jinit_c_main_controller(cinfo, FALSE /* never need full buffer here */);

  jinit_marker_writer(cinfo);

  /* We can now tell the memory manager to allocate virtual arrays. */
  (*cinfo->mem->realize_virt_arrays) ((j_common_ptr)cinfo);

  /* Write the datastream header (SOI) immediately.
   * Frame and scan headers are postponed till later.
   * This lets application insert special markers after the SOI.
   */
#ifdef WITH_JPU_HW
  if (cinfo->hw_encompress) {
  } else
#endif
  { (*cinfo->marker->write_file_header)(cinfo); }
}
