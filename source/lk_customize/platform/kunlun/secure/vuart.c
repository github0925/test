#include <stdlib.h>
#include <debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <arch/ops.h>
#include <dev/vuart.h>

static void *logbank_hdr = NULL;
static bool inclear; /* avoid accessing the buffer when clearing it */
static bool is_available;

int vuart_init(void)
{
    void *addr;

    if (logbank_hdr != NULL)
        return 0;

    addr = vuart_buffer_get_bank_hdr(SECURE_R5);

    if (!addr) {
        printf("vuart: failed to get log bank header: [%d]\n", SECURE_R5);
        return -1;
    }

    logbank_hdr = addr;
    is_available = 1;
    inclear = 0;
    return 0;
}

int vuart_putc(char c)
{
    if (inclear)
        return 0;

    vuart_buffer_putc(logbank_hdr, c);
#ifdef SUPPORT_VIRT_CONSOLE
    sdshell_wake_thread();
#endif
    return 0;
}

int vuart_is_available(void)
{
    return is_available && vuart_buffer_is_enable();
}

#ifdef SUPPORT_VIRT_CONSOLE
int vuart_getc(int port, bool wait)
{
    cbuf_t *rxbuf = sdshell_get_rx_cbuf();
    char c;

    if (cbuf_read_char(rxbuf, &c, wait) == 1) {
        return c;
    }

    return -1;
}

void vuart_clear_buf(void)
{
    inclear = 1;
    vuart_buffer_clear(logbank_hdr);
    inclear = 0;
}

int vuart_get_last_buf_and_size(char **buf, uint32_t *len)
{
    if (inclear)
        return 0;

    return vuart_buffer_get_last_buf_and_size(logbank_hdr, buf, len);
}

int vuart_get_new_buf(char *buf, uint32_t len)
{
    if (inclear)
        return 0;

    return vuart_buffer_get_new_buf(logbank_hdr, buf, len);
}

int vuart_update_tail(void)
{
    return vuart_buffer_update_tail(logbank_hdr);
}
#endif
