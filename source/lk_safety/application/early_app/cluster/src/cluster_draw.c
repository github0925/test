#include <lk_wrapper.h>
#include "cluster_draw.h"
#include "cluster_ui_parameter.h"
#include "string.h"

extern void arch_clean_cache_range(addr_t start, size_t len);

/* draw grayscale image on to frame buffer */
/* support color change, vflip and hflip */
/* when the pixel is 0, its alpha is forced to 0 */
/* when the pixel is not 0, its alpha is forced to 0xff */
/* do not use 0 value if the pixel needs to be displayed */
void draw_image(
    uint32_t color,  /* 0 means clean, non-zere means xRGB */
    uint32_t h,      /* height of the image */
    uint32_t w,      /* width of the image */
    uint32_t hflip,  /* 0 - no flip, 1 - hflip */
    uint32_t vflip,  /* 0 - no flip, 1 - vflip */
    uint32_t src_stride, /* stride of the source buffer */
    uint32_t dst_stride, /* stride of the destination buffer */
    uint8_t * src,   /* pointer to the source buffer */
    uint32_t * dst   /* pointer to the destination buffer */
    )
{
    uint32_t a,r,g,b;
    uint32_t r_color,g_color,b_color;
    uint32_t i,j;
    uint8_t grayscale;

    r_color = (color & 0x00ff0000) >> 16;
    g_color = (color & 0x0000ff00) >> 8;
    b_color = (color & 0x000000ff);

    for(i=0;i<h;i++)
    {
        for(j=0;j<w;j++)
        {
            if(color != 0)
            {
                if((hflip==0) && (vflip==0))
                {
                    grayscale = * (src + j + i*src_stride);
                }
                else if((hflip==1) && (vflip==0))
                {
                    grayscale = * (src + (w-1-j) + i*src_stride);
                }
                else if((hflip==1) && (vflip==1))
                {
                    grayscale = * (src + (w-1-j) + (h-i-1)*src_stride);
                }
                else
                {
                    grayscale = * (src + j + (h-i-1)*src_stride);
                }
                if(grayscale == 0x00)
                {
                    a = 0;
                    r = 0;
                    g = 0;
                    b = 0;
                }
                else
                {
                    a = 0xff;
                    r = (grayscale * r_color) >> 8;
                    g = (grayscale * g_color) >> 8;
                    b = (grayscale * b_color) >> 8;
                }
            }
            else
            {
                a=0;
                r=0;
                g=0;
                b=0;
            }

            * (dst + j + i*dst_stride) = b | (g<<8) | (r<<16) | (a<<24);
        }
        arch_clean_cache_range((addr_t)(dst + i*dst_stride), w*4);
    }
}

void draw_image_with_bg(
    uint32_t color,  /* 0 means clean, non-zere means xRGB */
    uint32_t h,      /* height of the image */
    uint32_t w,      /* width of the image */
    uint32_t hflip,  /* 0 - no flip, 1 - hflip */
    uint32_t vflip,  /* 0 - no flip, 1 - vflip */
    uint32_t src_stride, /* stride of the source buffer */
    uint32_t dst_stride, /* stride of the destination buffer */
    uint32_t bg_stride, /* stride of the bg buffer */
    uint8_t * src,   /* pointer to the source buffer */
    uint32_t * dst,   /* pointer to the destination buffer */
    uint32_t * bg   /* pointer to the bg buffer */
    )
{
    uint32_t a,r,g,b;
    uint32_t r_color,g_color,b_color;
    uint32_t i,j;
    uint8_t grayscale;
    uint8_t a_bg,r_bg,g_bg,b_bg;
    uint32_t argb_bg;
    uint32_t * bg_ptr;

    r_color = (color & 0x00ff0000) >> 16;
    g_color = (color & 0x0000ff00) >> 8;
    b_color = (color & 0x000000ff);

    for(i=0;i<h;i++)
    {

        for(j=0;j<w;j++)
        {
            bg_ptr = bg + j + i*bg_stride;
            argb_bg = * bg_ptr;

            a_bg = (argb_bg & 0xff000000)>>24;
            r_bg = (argb_bg & 0x00ff0000)>>16;
            g_bg = (argb_bg & 0x0000ff00)>>8;
            b_bg = (argb_bg & 0x000000ff)>>0;

            if(color != 0)
            {
                if((hflip==0) && (vflip==0))
                {
                    grayscale = * (src + j + i*src_stride);
                }
                else if((hflip==1) && (vflip==0))
                {
                    grayscale = * (src + (w-1-j) + i*src_stride);
                }
                else if((hflip==1) && (vflip==1))
                {
                    grayscale = * (src + (w-1-j) + (h-i-1)*src_stride);
                }
                else
                {
                    grayscale = * (src + j + (h-i-1)*src_stride);
                }
                if(grayscale == 0x00)
                {
                    a = a_bg;
                    r = r_bg;
                    g = g_bg;
                    b = b_bg;
                }
                else
                {
                    a = 0xff;
                    r = (grayscale * r_color) >> 8;
                    g = (grayscale * g_color) >> 8;
                    b = (grayscale * b_color) >> 8;
                }
            }
            else
            {
                a = a_bg;
                r = r_bg;
                g = g_bg;
                b = b_bg;
            }

            * (dst + j + i*dst_stride) = b | (g<<8) | (r<<16) | (a<<24);
        }
        arch_clean_cache_range((addr_t)(dst + i*dst_stride), w*4);
    }
}

/* draw 0-9 numbers on the screen */
void draw_number(
    uint32_t n,      /* number 0 - 9 */
    uint32_t color,  /* 0 means clean, non-zere means ARGB */
    uint32_t x,      /* x offset of the number on destination buffer */
    uint32_t y,      /* y offset of the number on destination buffer */
    uint32_t h,      /* height of the number */
    uint32_t w,      /* width of the number */
    uint32_t src_stride, /* stride of the buffer for numbers */
    uint32_t dst_stride, /* stride of the destination buffer */
    uint8_t * src,   /* pointer to the source buffer for numbers */
    uint32_t * dst   /* pointer to the destination buffer */
    )
{
    uint8_t * num_src;
    uint32_t * num_dst;

    num_src = src + n*w;
    num_dst = dst + x + y*dst_stride;

    draw_image(color, h, w, 0, 0, src_stride, dst_stride, num_src, num_dst);
}

/* draw 0-9 numbers on the screen */
void draw_number_with_bg(
    uint32_t n,      /* number 0 - 9 */
    uint32_t color,  /* 0 means clean, non-zere means ARGB */
    uint32_t x,      /* x offset of the number on destination buffer */
    uint32_t y,      /* y offset of the number on destination buffer */
    uint32_t h,      /* height of the number */
    uint32_t w,      /* width of the number */
    uint32_t src_stride, /* stride of the buffer for numbers */
    uint32_t dst_stride, /* stride of the destination buffer */
    uint32_t bg_stride,  /* stride of the background buffer */
    uint8_t * src,   /* pointer to the source buffer for numbers */
    uint32_t * dst,   /* pointer to the destination buffer */
    uint32_t * bg     /* pointer to the background buffer */
    )
{
    uint8_t * num_src;
    uint32_t * num_dst;
    uint32_t * num_bg;

    num_src = src + n*w;
    num_dst = dst + x + y*dst_stride;
    num_bg = bg + x + y*bg_stride;

    draw_image_with_bg(color, h, w, 0, 0, src_stride, dst_stride, bg_stride, num_src, num_dst, num_bg);
}


/* draw background from image source in packed RGB format */
/* the source image can be smaller or larger than the frame buffer  */
void draw_bg(
    uint32_t h,      /* height of the buffer */
    uint32_t w,      /* width of the buffer */
    uint32_t src_h,  /* height of the source buffer */
    uint32_t src_w,  /* width of the source buffer */
    uint8_t * src,   /* pointer to the source buffer */
    uint32_t * dst   /* pointer to the destination buffer */
    )
{
    uint32_t r,g,b;
    uint32_t i,j;
    uint32_t h_offset, w_offset;

    for(j=0;j<h;j++)
        for(i=0;i<w;i++)
        {
            h_offset = j % src_h;
            w_offset = i % src_w;
            r = * (src + 3 * (h_offset*src_w + w_offset));
            g = * (src + 3 * (h_offset*src_w + w_offset) + 1);
            b = * (src + 3 * (h_offset*src_w + w_offset) + 2);
            * dst = 0xff000000 | b | (g<<8) | (r<<16);
            dst ++;
        }

    arch_clean_cache_range((addr_t)dst, h*w*4);
}

/* change alpha of one ARGB image and write to frame buffer */
void draw_alpha(
    uint32_t h,      /* height of the image */
    uint32_t w,      /* width of the image */
    uint32_t src_stride, /* stride of the source buffer */
    uint32_t alpha_stride,  /* stride of the alpha buffer */
    uint32_t dst_stride, /* stride of the destination buffer */
    uint32_t * src,   /* pointer to the source buffer */
    uint8_t * alpha,   /* pointer to the alpha buffer */
    uint32_t * dst   /* pointer to the destination buffer */
    )
{
    int i,j;
    uint32_t pixel_src;
    uint8_t pixel_alpha;
    uint32_t pixel_dst;

    for(j=0;j<h;j++)
    {
        for(i=0;i<w;i++)
        {
            pixel_alpha = * (alpha + i + j*alpha_stride);
            if(pixel_alpha<128)
            {
                pixel_alpha = 0xff;
            }
            else
            {
                pixel_alpha = 0x00;
            }

            if((j<8)||(j>h-1-8))
            {
                pixel_alpha = 0xff;
            }
            if(pixel_alpha == 0x00)
            {
                if(i<192)
                {
                    pixel_alpha = 0xff - i*4/3;
                }

                if(i>w-1-192)
                {
                    pixel_alpha = 0xff - (w-1-i)*4/3;
                }
            }
            pixel_src = * (src + i + j*src_stride);
            pixel_dst = (((uint32_t) pixel_alpha) << 24 ) | (pixel_src & 0xffffff);

            * (dst+i+j*dst_stride) = pixel_dst;
            //printf("SRC:%x, A:%x, DST:%x\n",pixel_src,pixel_alpha,pixel_dst);
        }

        arch_clean_cache_range((addr_t)(dst+j*dst_stride), w*4);
    }
}
/* overlay one ARGB image onto a background and write to frame buffer */
void draw_overlay(
    uint32_t alpha,  /* 0 mean copy, 1 means using embedded alpha from src */
    uint32_t h,      /* height of the image */
    uint32_t w,      /* width of the image */
    uint32_t x,      /* x offset of the image on background buffer */
    uint32_t y,      /* y offset of the image on background buffer */
    uint32_t x_bg,   /* x offset of the image on destination buffer */
    uint32_t y_bg,   /* y offset of the image on destination buffer */
    uint32_t src_stride, /* stride of the source buffer */
    uint32_t bg_stride,  /* stride of the background buffer */
    uint32_t dst_stride, /* stride of the destination buffer */
    uint32_t * src,   /* pointer to the source buffer */
    uint32_t * bg,   /* pointer to the background buffer */
    uint32_t * dst   /* pointer to the destination buffer */
    )
{
    int i,j;
    uint32_t pixel_src;
    uint32_t pixel_dst = 0x00;
    uint32_t a_src = 0xff,r_src,g_src,b_src;

    for(j=0;j<h;j++)
    {
        memcpy(dst+x+(j+y)*dst_stride,src+j*src_stride,src_stride*4);
        arch_clean_cache_range((addr_t)(dst+j*dst_stride), src_stride*4);
    }
}

/* fill the whole frame buffer with one color in ARGB format */
void draw_fill(
    uint32_t color,  /* color to be filled, ARGB format */
    uint32_t h,      /* height of the buffer */
    uint32_t w,      /* width of the buffer */
    uint32_t stride, /* stride of the destination buffer */
    uint32_t * dst   /* pointer to the destination buffer */
    )
{
    int i,j;
    for(j=0;j<h;j++)
    {
        for(i=0;i<w;i++)
        {
            * (dst+i+j*stride) = color;
        }

        arch_clean_cache_range((addr_t)(dst+j*stride), w*4);
    }
}

/* draw color bar on the whole frame buffer */
void draw_colorbar(
    uint32_t h,      /* height of the buffer */
    uint32_t w,      /* width of the buffer */
    uint32_t * dst   /* pointer to the destination buffer */
    )
{
    uint32_t r,g,b;
    uint32_t i,j;

    for(j=0;j<w;j++)
        for(i=0;i<h;i++)
        {
            if(j<w/6) /*red*/
            {
                r = 255;
                g = 0;
                b = 0;
            }
            else if(j<w/3) /*green*/
            {
                r = 0;
                g = 255;
                b = 0;
            }
            else if(j<w/2) /*blue*/
            {
                r = 0;
                g = 0;
                b = 255;
            }
            else if(j<w/6*4) /*cyan*/
            {
                r = 0;
                g = 255;
                b = 255;
            }
            else if(j<w/6*5) /*yellow*/
            {
                r = 255;
                g = 255;
                b = 0;
            }
            else /*magenta*/
            {
                r = 255;
                g = 0;
                b = 255;
            }

            * dst = b | (g<<8) | (r<<16) | (0xff<<24);
            dst ++;
        }

    arch_clean_cache_range((addr_t)(dst-h*w*4), h*w*4);
}

/* draw needle on the frame buffer */
void draw_needle(
    uint32_t angle,  /* 0-359, > is 0, ^ is 90, < is 180 */
    uint32_t color,  /* 0 means clean, non-zere means xxRRGGBB */
    uint32_t x,      /* x coodinate of the meter (center of the circle) */
    uint32_t y,      /* y coodinate of the meter (center of the circle) */
    uint32_t stride, /* stride of the frame buffer */
    uint8_t * src,   /* pointer to the buffer for all needles */
    uint32_t * dst   /* pointer to the frame buffer */
    )
{
    uint32_t h = NEEDLE_HEIGHT;      /* height of the rectangle area of the needle */
    uint32_t w = NEEDLE_WIDTH;       /* width of the rectange area of the needle */
    uint32_t c_x = NEEDLE_X_OFFSET;  /* x offset of the needle center */
    uint32_t c_y = NEEDLE_Y_OFFSET;  /* y offset of the needle center */

    uint32_t a,r,g,b;
    uint32_t r_color,g_color,b_color;
    uint32_t i,j;
    uint8_t grayscale;

    /* pick the right needle based on angle */

    uint32_t needle_index;
    uint32_t vflip;
    uint32_t hflip;
    uint8_t * new_src;
    uint32_t * new_dst;

    if(angle<=90)
    {
        needle_index = angle;
        vflip = 0;
        hflip = 0;
        new_dst = dst + (x-c_x) + (y-c_y)*stride;
    }
    else if(angle<=180)
    {
        needle_index = 180-angle;
        vflip = 0;
        hflip = 1;
        new_dst = dst + (x+c_x-w) + (y-c_y)*stride;
    }
    else if(angle<=270)
    {
        needle_index = angle-180;
        vflip = 1;
        hflip = 1;
        new_dst = dst + (x+c_x-w) + (y+c_y-h)*stride;
    }
    else
    {
        needle_index = 360-angle;
        vflip = 1;
        hflip = 0;
        new_dst = dst + (x-c_x) + (y+c_y-h)*stride;
    }
    /* needles are stored from 90 to 1 in memory */
    new_src = src + h * w * (90-needle_index);

    /* copy the needle to frame buffer */
    draw_image(color, h, w, hflip, vflip, w, stride, new_src, new_dst);

}

/* draw needle on the frame buffer with background*/
void draw_needle_with_bg(
    uint32_t angle,  /* 0-359, > is 0, ^ is 90, < is 180 */
    uint32_t color,  /* 0 means clean, non-zere means xxRRGGBB */
    uint32_t x,      /* x coodinate of the meter (center of the circle) */
    uint32_t y,      /* y coodinate of the meter (center of the circle) */
    uint32_t stride, /* stride of the frame buffer */
    uint32_t bg_stride, /* stride of the background buffer */
    uint8_t * src,   /* pointer to the buffer for all needles */
    uint32_t * dst,  /* pointer to the frame buffer */
    uint32_t * bg     /* pointer to the background buffer */
    )
{
    uint32_t h = NEEDLE_HEIGHT;      /* height of the rectangle area of the needle */
    uint32_t w = NEEDLE_WIDTH;       /* width of the rectange area of the needle */
    uint32_t c_x = NEEDLE_X_OFFSET;  /* x offset of the needle center */
    uint32_t c_y = NEEDLE_Y_OFFSET;  /* y offset of the needle center */

    uint32_t a,r,g,b;
    uint32_t r_color,g_color,b_color;
    uint32_t i,j;
    uint8_t grayscale;

    /* pick the right needle based on angle */

    uint32_t needle_index;
    uint32_t vflip;
    uint32_t hflip;
    uint8_t * new_src;
    uint32_t * new_bg;
    uint32_t * new_dst;

    if(angle<=90)
    {
        needle_index = angle;
        vflip = 0;
        hflip = 0;
        new_dst = dst + (x-c_x) + (y-c_y)*stride;
    }
    else if(angle<=180)
    {
        needle_index = 180-angle;
        vflip = 0;
        hflip = 1;
        new_dst = dst + (x+c_x-w) + (y-c_y)*stride;
    }
    else if(angle<=270)
    {
        needle_index = angle-180;
        vflip = 1;
        hflip = 1;
        new_dst = dst + (x+c_x-w) + (y+c_y-h)*stride;
    }
    else
    {
        needle_index = 360-angle;
        vflip = 1;
        hflip = 0;
        new_dst = dst + (x-c_x) + (y+c_y-h)*stride;
    }
    /* needles are stored from 90 to 1 in memory */
    new_src = src + h * w * (90-needle_index);

    new_bg = bg + (new_dst - dst);

    /* copy the needle to frame buffer */
    draw_image_with_bg(color, h, w, hflip, vflip, w, stride, bg_stride, new_src, new_dst, new_bg);

}

/* draw fps number on the frame buffer */
/* support fps from 0.0 to 999.9 */
void draw_fps(
    float fps,       /* fps number */
    uint32_t color,  /* 0 means clean, non-zere means xRGB */
    uint32_t x,      /* x offset of the fps on destination buffer */
    uint32_t y,      /* y offset of the fps on destination buffer */
    uint32_t stride, /* stride of the frame buffer */
    uint8_t * src,   /* pointer to the buffer for all numbers */
    uint32_t * dst   /* pointer to the frame buffer */
    )
{
    uint32_t n_100, n_10, n_1, n_0p1;
    uint32_t n_100_color, n_10_color;
    uint32_t h, w, s;
    uint32_t fps_10x;

    h = NUM_HEIGHT;  /* height of the number */
    w = NUM_WIDTH;   /* width of the number */
    s = NUM_STRIDE;  /* stride of the buffer for numbers */

    fps_10x = (uint32_t)(fps*10);
    n_100 = fps_10x/1000;
    n_10 = (fps_10x%1000)/100;
    n_1 = (fps_10x%100)/10;
    n_0p1 = fps_10x%10;

    /* change color to hide the leading 0 */
    if(n_100!=0)
    {
        n_100_color = color;
        n_10_color = color;
    }
    else if(n_10!=0)
    {
        n_100_color = 0;
        n_10_color = color;
    }
    else
    {
        n_100_color = 0;
        n_10_color = 0;
    }

    /* draw numbers on to buffer */
    draw_number(n_100, n_100_color, x, y, h, w, s, stride, src, dst);
    draw_number(n_10, n_10_color, x+w, y, h, w, s, stride, src, dst);
    draw_number(n_1, color, x+2*w, y, h, w, s, stride, src, dst);
    draw_number(n_0p1, color, x+3*w+w/4, y, h, w, s, stride, src, dst);

    /* fill a rectangle as the dot */
    draw_fill(0xff000000|color, h/8, h/8, stride, dst+(x+3*w)+(y+h-h/4)*stride);
}


/* draw fps number on the frame buffer with background */
/* support fps from 0.0 to 999.9 */
void draw_fps_with_bg(
    float fps,       /* fps number */
    uint32_t color,  /* 0 means clean, non-zere means xRGB */
    uint32_t x,      /* x offset of the fps on destination buffer */
    uint32_t y,      /* y offset of the fps on destination buffer */
    uint32_t stride, /* stride of the frame buffer */
    uint32_t bg_stride, /* stride of the background buffer */
    uint8_t * src,   /* pointer to the buffer for all numbers */
    uint32_t * dst,  /* pointer to the frame buffer */
    uint32_t * bg    /* pointer to the background buffer */
    )
{
    uint32_t n_100, n_10, n_1, n_0p1;
    uint32_t n_100_color, n_10_color;
    uint32_t h, w, s;
    uint32_t fps_10x;

    h = NUM_HEIGHT;  /* height of the number */
    w = NUM_WIDTH;   /* width of the number */
    s = NUM_STRIDE;  /* stride of the buffer for numbers */

    fps_10x = (uint32_t)(fps*10);
    n_100 = fps_10x/1000;
    n_10 = (fps_10x%1000)/100;
    n_1 = (fps_10x%100)/10;
    n_0p1 = fps_10x%10;

    /* change color to hide the leading 0 */
    if(n_100!=0)
    {
        n_100_color = color;
        n_10_color = color;
    }
    else if(n_10!=0)
    {
        n_100_color = 0;
        n_10_color = color;
    }
    else
    {
        n_100_color = 0;
        n_10_color = 0;
    }

    /* draw numbers on to buffer */
    draw_number_with_bg(n_100, n_100_color, x, y, h, w, s, stride, bg_stride, src, dst, bg);
    draw_number_with_bg(n_10, n_10_color, x+w, y, h, w, s, stride, bg_stride, src, dst, bg);
    draw_number_with_bg(n_1, color, x+2*w, y, h, w, s, stride, bg_stride, src, dst, bg);
    draw_number_with_bg(n_0p1, color, x+3*w+w/4, y, h, w, s, stride, bg_stride, src, dst, bg);

    /* fill a rectangle as the dot */
    draw_fill(0xff000000|color, h/8, h/8, stride, dst+(x+3*w)+(y+h-h/4)*stride);
}

