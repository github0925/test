#ifndef __CLASS_FASTBOOT_H__
#define __CLASS_FASTBOOT_H__

#include <kernel/event.h>
#include "hal_usb.h"

#define STATE_OFFLINE    0
#define STATE_COMMAND    1
#define STATE_COMPLETE   2
#define STATE_ERROR      3

#define MAX_RSP_SIZE            64

/* allocate a buffer on the stack aligned and padded to the cpu's cache line size */
#define STACKBUF_DMA_ALIGN(var, size) \
    uint8_t var[ROUNDUP(size, CACHE_LINE)] __ALIGNED(CACHE_LINE);

typedef status_t (*usb_online_cb)(void *);
typedef status_t (*usb_offline_cb)(void *);

typedef struct {
	usb_online_cb  online;
	usb_offline_cb offline;
} fastboot_usb_callback;

struct fb_priv {
	void *download_base;
	uint32_t download_max;
	uint32_t download_size;

	event_t usb_online;
	event_t usb_write_done;
	event_t usb_read_done;

	int usb_write_status;
	int usb_read_status;

	uint32_t actual_read_len;
	uint32_t actual_write_len;

	uint32_t read_ep;
	uint32_t write_ep;
};

typedef struct {
	uint32_t state;
	fastboot_usb_callback cb;
	void (*fb_init)(void *args);
	void (*fb_stop)(void *args);
	status_t (*usb_read)(void *handle, void *buf, unsigned len);
	status_t (*usb_write)(void *handle, void *buf, unsigned len);
	struct fb_priv priv;
	usb_t usb;
} fastboot_t;

void fastboot_init(void *args);
void fastboot_stop(void *args);
int fastboot_handler(void *arg);

void fastboot_okay(fastboot_t *fb, const char *info);
void fastboot_fail(fastboot_t *fb, const char *reason);
void fastboot_info(fastboot_t *fb, const char *reason);

void fastboot_register_cmd(const char *prefix,
    void (*handle)(fastboot_t *fb, const char *arg, void *data, unsigned sz));
void fastboot_register_var(const char *name, const char *value);

int fastboot_usb_read(void *handle, void *buf, unsigned len);
int fastboot_usb_write(void *handle, void *buf, unsigned len);

status_t fastboot_online(void *arg);
status_t fastboot_offline(void *arg);

void cmd_flash(fastboot_t *fb, const char *arg, void *data, unsigned sz);
void cmd_getvar(fastboot_t *fb, const char *arg, void *data, unsigned sz);
void cmd_download(fastboot_t *fb, const char *arg, void *data, unsigned sz);

#endif
