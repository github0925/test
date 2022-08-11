
#define EGL_ERROR 1<<0
#define ENV_ERROR 1<<1
#define INIT_FAIL 1<<2
#define DRAW_ERR  1<<3
#define OCL_INIT_ERR   1<<4
#define OCL_CMPT_ERR   1<<5
#define OCL_VRFY_ERR   1<<6

#define ERR_FRAME(x) ((x<<8)|DRAW_ERR)
#define ERR_CLCASE(x,y) ((x<<16)|y)

