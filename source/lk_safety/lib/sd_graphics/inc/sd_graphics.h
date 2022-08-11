#ifndef SD_GRAPHICS_H
#define SD_GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <lk_wrapper.h>

#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif


#define ABS_A(x) (((x) >= 0)? (x) : (-(x)))

#define SD_SWAP_POINTS(A,B)	(*(A))=(*(A))^(*(B));\
								(*(B))=(*(A))^(*(B));\
								(*(A))=(*(A))^(*(B));

#define SD_GET_X(Point)	(Point.x*65536+Point.xd)
#define SD_GET_Y(Point)	(Point.y*65536+Point.yd)

#define SD_MAX_POP_NUM	(2048*16)

#define SD_DIS_SIZE 2


typedef struct
{
	unsigned short 		w;
	unsigned short		h;
}SD_SIZE_T;

typedef struct
{
	short 		x;
	short		y;
}SD_POINT_T;

typedef struct
{
	short 		x;
	short		y;
	unsigned short 		xd;
	unsigned short		yd;
}SD1_POINT_T;

typedef struct
{
	short 		x;
	short		y;
	unsigned short 		op;
}SD_OP_POINT_T;

typedef struct Sd_Verge_t {
	void *nex;
	int ux;
	int uy;
	int dx;
	int dy;
	int cx;
	int tx;
	short count;
	short lx;
	bool valid;
}SD_VERGE_T;

void SD_rotation(const SD_SIZE_T dst_s,
						const SD_SIZE_T src_s,
						SD_POINT_T dst_cter,
						SD_POINT_T src_cter,
						int32_t angle,
						uint32_t *src_ptr,
						uint32_t *dst_ptr,
						int32_t zaxis,
						SD1_POINT_T *point1_ptr);

#endif /*SD_GRAPHICS_H*/