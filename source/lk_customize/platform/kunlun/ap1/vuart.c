#include <stdlib.h>
#include <debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <dev/vuart.h>

static void *logbank_hdr = NULL;
static int is_available;

int vuart_init(void)
{
    void *addr;

    if (logbank_hdr != NULL)
        return 0;

    addr = vuart_buffer_get_bank_hdr(AP1_A55);

    if (!addr) {
        printf("vuart: failed to get log bank header: [%d]\n", AP1_A55);
        return -1;
    }

    logbank_hdr = addr;
    is_available = 1;
    return 0;
}

int vuart_putc(char c)
{
    vuart_buffer_putc(logbank_hdr, c);
    return 0;
}

int vuart_is_available(void)
{
    return is_available && vuart_buffer_is_enable();
}
