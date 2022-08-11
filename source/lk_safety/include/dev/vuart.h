#ifndef __DEV_VUART_H__
#define __DEV_VUART_H__
#include <stdbool.h>
#include <sys/types.h>
#include <lib/cbuf.h>
#include "vuart_buf.h"

int vuart_init(void);
int vuart_putc(char c);
int vuart_is_available(void);

#ifdef SUPPORT_VIRT_CONSOLE
int sdshell_wake_thread(void);
cbuf_t *sdshell_get_rx_cbuf(void);
int vuart_getc(int port, bool wait);
void vuart_clear_buf(void);
int vuart_get_last_buf_and_size(char **buf, uint32_t *len);
int vuart_get_new_buf(char *buf, uint32_t len);
int vuart_update_tail(void);
#endif

#endif
