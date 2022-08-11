/* draw color bar on the whole frame buffer */
void draw_colorbar(
    uint32_t h,      /* height of the buffer */
    uint32_t w,      /* width of the buffer */
    uint32_t * dst   /* pointer to the destination buffer */
    );

/* fill the whole frame buffer with one color in ARGB format */
void draw_fill(
    uint32_t color,  /* color to be filled, ARGB format */
    uint32_t h,      /* height of the buffer */
    uint32_t w,      /* width of the buffer */
    uint32_t stride, /* stride of the destination buffer */
    uint32_t * dst   /* pointer to the destination buffer */
    );

/* draw background from image source in packed RGB format */
/* the source image can be smaller or larger than the frame buffer  */
void draw_bg(
    uint32_t h,      /* height of the buffer */
    uint32_t w,      /* width of the buffer */
    uint32_t src_h,  /* height of the source buffer */
    uint32_t src_w,  /* width of the source buffer */
    uint8_t * src,   /* pointer to the source buffer */
    uint32_t * dst   /* pointer to the destination buffer */
    );

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
    );

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
    );

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
    );

/* draw needle on the frame buffer */
void draw_needle(
    uint32_t angle,  /* 0-359, > is 0, ^ is 90, < is 180 */
    uint32_t color,  /* 0 means clean, non-zere means xxRRGGBB */
    uint32_t x,      /* x coodinate of the meter (center of the circle) */
    uint32_t y,      /* y coodinate of the meter (center of the circle) */
    uint32_t stride, /* stride of the frame buffer */
    uint8_t * src,   /* pointer to the buffer for all needles */
    uint32_t * dst   /* pointer to the frame buffer */
    );

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
    uint32_t * bg    /* pointer to the background buffer */
    );

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
    );

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
    uint32_t * bg     /* pointer to the background buffer */
    );

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
    );
