#include "lvgl.h"
#include "lv_examples.h"
#include "stb_decoder.h"
#include <stdio.h>
#include <res_loader.h>
#include "heap.h"

#define STBI_MALLOC(sz)           malloc(sz)
#define STBI_REALLOC(p,newsz)     STBI_MALLOC(newsz)
#define STBI_FREE(p)              free(p)

#define STBI_NO_HDR
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define LEN_OST 32
#define PNGHEAD 0X474E5089 //header of png file "PNG"

//store png from data, 32Byte size of png + source png
void* pngsrc[PNGNUM]={NULL};

//store data decoderd  by stb_image.h
lv_img_dsc_t decoderdimg_dsc[PNGNUM];
static lv_img_decoder_t *sd_dec = NULL;



void abgr2argb(uint8_t *data, size_t length);


/**
 * Get info about a PNG image
 * @param decoder pointer to the decoder where this function belongs
 * @param src can be file name or pointer to a C array
 * @param header store the info here
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_res_t _decoder_info(lv_img_decoder_t * decoder, const void * src, lv_img_header_t * header)
{
    static int width,  height, channels;
    int res;

    //src is source png
    if(*(uint32_t*)(src+LEN_OST) == PNGHEAD){
        res = stbi_info_from_memory((stbi_uc const *)(src+LEN_OST),256, &width, &height, &channels);
        decoder->user_data = (void*)src;
        }
    else{
        // src decoderd by stb_image.h
        lv_img_header_t imghead = *(lv_img_header_t*)src;
        width = imghead.w;
        height = imghead.h;
        res = 0;
    }

    if (!res) return LV_RES_INV;
    LV_LOG_INFO("decoder_info: read picture: %d x %d - %d\n", width, height, channels);

    header->cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    header->w = width;
    header->h = height;
    header->always_zero = 0;    //Must write 0
    res = LV_RES_OK;
    return res;
}
static const uint8_t *sd_decoder(const char *src,uint32_t length) {
    int width, height, channels;
    int i;
    uint8_t *data;
    uint32_t *p;
    data = stbi_load_from_memory((stbi_uc const *)src, length, &width, &height, &channels, 4);
    
    if (data == NULL) return NULL;

    abgr2argb(data, width * height * channels);

    return (const uint8_t *)data;
}

/**
 * Open a PNG image and return the decided image
 * @param decoder pointer to the decoder where this function belongs
 * @param dsc pointer to a descriptor which describes this decoding session
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_res_t _decoder_open(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{

    /*Decode and store the image. If `dsc->img_data` is `NULL`,
     * the `read_line` function will be called to get the image data line-by-line*/
    LV_LOG_INFO("open file(%d): %s\n", dsc->src_type, dsc->src);

    uint32_t len = *(uint32_t*)decoder->user_data;

    dsc->img_data = sd_decoder(decoder->user_data+LEN_OST, len);
   
    if (!dsc->img_data){
        LV_LOG_INFO("no img data\r\n");
         return LV_RES_INV;
    }
    else
    {
        LV_LOG_INFO("Get img data\r\n");
    }

    /*Change the color format if required. For PNG usually 'Raw' is fine*/
    dsc->header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;

    /*Call a built in decoder function if required.
     * It's not required if`my_png_decoder` opened the image in true color format.*/
//    lv_res_t res = lv_img_decoder_built_in_open(decoder, dsc);
    return LV_RES_OK;
}

/**
 * Decode `len` pixels starting from the given `x`, `y` coordinates and store them in `buf`.
 * Required only if the "open" function can't open the whole decoded pixel array. (dsc->img_data == NULL)
 * @param decoder pointer to the decoder the function associated with
 * @param dsc pointer to decoder descriptor
 * @param x start x coordinate
 * @param y start y coordinate
 * @param len number of pixels to decode
 * @param buf a buffer to store the decoded pixels
 * @return LV_RES_OK: ok; LV_RES_INV: failed
 */
lv_res_t decoder_built_in_read_line(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc, lv_coord_t x,
                                    lv_coord_t y, lv_coord_t len, uint8_t * buf)
{
    /*With PNG it's usually not required*/

    /*Copy `len` pixels from `x` and `y` coordinates in True color format to `buf` */
    return 0;

}

/**
 * Free the allocated resources
 * @param decoder pointer to the decoder where this function belongs
 * @param dsc pointer to a descriptor which describes this decoding session
 */
static void _decoder_close(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{
    /*Free all allocated data*/
    stbi_image_free((void*)dsc->img_data);
    /*Call the built-in close function if the built-in open/read_line was used*/
    lv_img_decoder_built_in_close(decoder, dsc);
}

static int _sd_decoder_init(void) 
{
    if (!sd_dec){
        /*Create a new decoder and register functions */
        lv_img_decoder_t * dec = lv_img_decoder_create();
        lv_img_decoder_set_info_cb(dec, _decoder_info);
        lv_img_decoder_set_open_cb(dec, _decoder_open);
        lv_img_decoder_set_close_cb(dec, _decoder_close);
    }
    return 0;
}


void dump(const uint8_t *data, int offset) {
    for (int i =0 + offset;i < 4 + offset; i++) {
        LV_LOG_INFO("0x%x\n", data[i]);
    }
}

void abgr2argb(uint8_t *data, size_t length) {
    uint32_t *p = (uint32_t *)data;

    register uint32_t temp;
    for (size_t i = 0; i < length / 4; i++) {
        temp = p[i];
        p[i] = (temp & 0xff00ff00) | ((temp & 0xff) << 16) | ((temp & 0xff0000) >> 16);
    }
}

static void _png_eliminate_pic(void* pic)
{
    free(pic);
    pic = NULL;
}

static void _png_load_pic(void)
{
    uint8_t i = 0;
    uint32_t ipnglen = 0;
    uint32_t alignedlen = 0;
    for(i = 0; i<PNGNUM;i++){
        ipnglen = res_size(png_path[i]);
        alignedlen = ROUNDUP(ipnglen+LEN_OST,32);
        pngsrc[i] = memalign(32,alignedlen);
        ASSERT(pngsrc[i]);
        memcpy(pngsrc[i],&ipnglen,4);
        res_load(png_path[i],pngsrc[i]+LEN_OST,alignedlen,0);
    }
}

static void _decoder_allpng(void)
{
    lv_res_t res;
    lv_img_decoder_dsc_t dsc;
    uint8_t i = 0;

    for(i = 0; i<PNGNUM; i++){
        res = lv_img_decoder_open(&dsc, pngsrc[i], LV_COLOR_GRAY);
        memcpy(&decoderdimg_dsc[i].header, &dsc.header, sizeof(lv_img_header_t));
        decoderdimg_dsc[i].data_size = LV_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(dsc.header.w, dsc.header.h);
        decoderdimg_dsc[i].data = malloc(decoderdimg_dsc[i].data_size);
        memcpy((void*)decoderdimg_dsc[i].data,dsc.img_data,decoderdimg_dsc[i].data_size);
        if (res == LV_RES_OK) {
            _png_eliminate_pic(pngsrc[i]);
            lv_img_decoder_close(&dsc);
        }
    }
}


void stb_decoder(void)
{
    //get png from tlvgl_fs_res,then decode all png,free png file;
    //allocate memory for decoded data and free stb memory to avoid malloc in stb_image.h repeatedly 
    //and decode operation repeatedly, allocated memory isn't free now.
    //tlvgl_fs_res.ima should down to OSPI1_FS_RES.
    //1. get png
    _png_load_pic();
    //2. set callback for lvgl
    _sd_decoder_init();
  
    //3. decoder png
    _decoder_allpng();

}