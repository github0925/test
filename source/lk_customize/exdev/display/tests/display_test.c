
#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <malloc.h>

#include "disp_data_type.h"

#include <sdm_display.h>
#include <arch/ops.h>


#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#if WITH_KERNEL_VM
#define v2p(va)    (paddr_t)(vaddr_to_paddr(va))
#define p2v(pa)    (vaddr_t)(paddr_to_kvaddr(pa))
#else
#define v2p(va)    (paddr_t)(va)
#define p2v(pa)    (vaddr_t)(pa)
#endif
/*struct disp_xxx_data {
    unsigned char panel_index;
};*/

static unsigned int sd_get_color_format(void)
{
    unsigned int format;

    format = COLOR_ARGB8888;

    return format;
}

static unsigned char sd_get_bpp_by_format(unsigned int format)
{
    unsigned char bpp = 0;

    switch (format) {
        case COLOR_RGB565:
            bpp = 2;
            break;
        case COLOR_ARGB8888:
            bpp = 4;
            break;
        default:
            LOGD("Not support this format\n");
            break;
    }
    return bpp;
}



#if defined(WITH_LIB_CONSOLE)

//#if LK_DEBUGLEVEL > 1
#if 1

#include <lib/console.h>
static int cmd_display_test(int argc, const cmd_args *argv);
STATIC_COMMAND_START
STATIC_COMMAND("display_test", "sdm display test", &cmd_display_test)
STATIC_COMMAND_END(display_test);
typedef void (*func_t)(void);

typedef struct
{
    int x1;
    int y1;
    int x2;
    int y2;
} lv_area_t;



int sdm_display_test(sdm_display_t *sdm, uint8_t *buf_bottom, uint8_t *buf_top, uint8_t opa,
	lv_area_t *src, lv_area_t *dest){
	LOGD("sdm 0\n");
    struct sdm_post_config post;
    int i;
    int ret;
    int n_bufs;
    uint8_t *bufs[] = {buf_bottom, buf_top};

	if (!sdm) {
		LOGE("display handle is invalid\n");
		return -1;
	}
	LOGD("sdm 1\n");

    memset(&post, 0, sizeof(struct sdm_post_config));
    if (!buf_bottom) {
        LOGE("Error: buf1 is null\n");
        return -3;
    }
    n_bufs = 1;
    if (buf_top) {
        n_bufs ++;
    }
	LOGD("sdm 2\n");
    post.bufs = (struct sdm_buffer*) malloc(sizeof(struct sdm_buffer) * n_bufs);
    if (!post.bufs) {
        LOGE("Error: malloc sdm_buffer failed\n");
        return -2;
    }
	LOGD("sdm 3\n");

    post.n_bufs = n_bufs;
    for (i = 0; i < n_bufs; i++) {
        struct sdm_buffer *buf = &post.bufs[i];
        buf->addr[0] = (unsigned long)v2p((void *)bufs[i]);
		LOGD("addr: %p --- (v2p) %ld\n", (void *)bufs[i], buf->addr[0]);
        if (i == 1)
            buf->alpha = opa;
        else
            buf->alpha = 0xff;
        buf->alpha_en = 1;
        buf->ckey = 0;
        buf->ckey_en = 0;
        buf->fmt = sd_get_color_format();
        buf->layer_en = 1;
        buf->src.x = src->x1;
        buf->src.y = src->y1;
        buf->src.w = src->x2 - src->x1;
        buf->src.h = src->y2 - src->y1;

        buf->start.x = src->x1;
        buf->start.y = src->y1;
        buf->start.w = buf->src.w;
        buf->start.h = buf->src.h;
        buf->src_stride[0] = buf->start.w * sd_get_bpp_by_format(buf->fmt);

        // dc do not support scaling.
        buf->dst.x = dest->x1;
        buf->dst.y = dest->y1;
        buf->dst.w = dest->x2 - dest->x1;
        buf->dst.h = dest->y2 - dest->y1;
        buf->z_order = i;
    }
	LOGD("sdm 4\n");

    ret = sdm_post(sdm->handle, &post);
    if (ret) {
        LOGD("post failed: %d\n", ret);
    }
	LOGD("sdm 5\n");
    free(post.bufs);
	LOGD("sdm 6\n");
    return 0;

}
sdm_display_t *g_displays[DISPLAY_TYPE_MAX];
#include <ext_data.h>
#define TEST_WIDTH 240
#define TEST_HEIGHT 320

int test_display_by_id(int id) {
	LOGD("START 0\n");
	lv_area_t source_area = {
		.x1 = 0,
		.y1 = 0,
		.x2 = TEST_WIDTH,
		.y2 = TEST_HEIGHT,
	};

	lv_area_t area = {
		.x1 = 100,
		.y1 = 100,
	};
	area.x2 = area.x1 + TEST_WIDTH;
	area.y2 = area.y1 + TEST_HEIGHT;

	int w = area.x2 - area.x1;
	int h = area.y2 - area.y1;
	size_t sz = w * sd_get_bpp_by_format(COLOR_ARGB8888) * h;
	(void)sz;
	static uint32_t buf3_1[TEST_WIDTH * TEST_HEIGHT] EXT_SECTION(LVGL);			/*A screen sized buffer*/
	static uint32_t buf3_2[TEST_WIDTH * TEST_HEIGHT] EXT_SECTION(LVGL); 		   /*An other screen sized buffer*/
	//(void)buf3_2;
	LOGD("START 1\n");

	LOGD("BUF3_1:%p\n",buf3_1);
	LOGD("BUF3_2:%p\n",buf3_2);

	int i,j = 0;
	(void)j;
	static int inited = 0;
	if (inited == 0) {
	for (i = 0; i < TEST_HEIGHT; i++) {
				//	LOGD("memset %d\n", i);
		for (j = 0; j < TEST_WIDTH; j++) {
			buf3_1[i * TEST_WIDTH + j] = 0xffffffff;
			buf3_2[i * TEST_WIDTH + j] = 0x0000ffff;
		}
	}

	inited = 1;
	}
	//arch_clean_cache_range((addr_t)buf3_1, TEST_WIDTH *4);
	//arch_clean_cache_range((addr_t)buf3_2, TEST_WIDTH *4);
	LOGD("START 2\n");

	uint8_t *buf;
	switch(id) {
		case 0:
			buf = (uint8_t *) buf3_1;
		break;
		case 1:
			buf = (uint8_t *) buf3_2;
		break;
		default:
			return -1;
	}
	LOGD("START 3\n");
	(void)buf;
	(void)source_area;
		sdm_display_test(g_displays[id], (uint8_t *)buf, (uint8_t *)NULL, 0xff, &source_area, &area);
	LOGD("START 4\n");

	return 0;
}

static int cmd_display_test(int argc, const cmd_args *argv) {
    int id = 0;

	int num_display = list_length(sdm_get_display_list());

	if (argc  == 1)
		LOGD("got %d displays connected!\n", num_display);

	if (argc == 2)
		id = atoi(argv[1].str);
	struct list_node *head = sdm_get_display_list();
	sdm_display_t *sdm;

	int n = 0;
	list_for_every_entry(head, sdm, sdm_display_t, node) {

		LOGD("disp->id, disp->handle->display_id (%d, %d)\n",
			sdm->id, sdm->handle->display_id);
		if (n >= DISPLAY_TYPE_MAX) {
			LOGE("error num of display: %d\n", n);
			break;
		}
		g_displays[n++] = sdm;
	}
	int i=0;
	for (i=0;i< 10000;i++) {
		test_display_by_id(i%2);
	}

	(void)id;
    return 0;
}

#endif
#endif
