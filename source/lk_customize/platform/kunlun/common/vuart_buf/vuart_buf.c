#include <stdlib.h>
#include <debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <arch/ops.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include "vuart_buf.h"

#define SDRV_LOGBUF_MAGIC       0x20210618
#define SDRV_LOGBUF_BANK_HEADER_SIZE    0x40
#define SDRV_LOGBUF_HEADER_SIZE     0x80

/* bank buffer flag */
#define SDLOG_BANK_BUF_FLAG_OVERRIDE    (1 << 0)

/*
 * to keep the following structure alignment, we need follow
 * points below:
 * - use architecture-independment types (eg: int, char)
 * - use single-byte or four-byte alignment
 */

/* reserved 128 byte to describe the logbuf header */
typedef union logbuf_hdr {
    struct logbuf_header {
        uint32_t magic;
        uint32_t logoff[SDRV_CORE_MAX];
        uint32_t logsize[SDRV_CORE_MAX];
        uint32_t status;
        uint32_t uart_mask;
    } header;
    uint8_t pad[SDRV_LOGBUF_HEADER_SIZE];
} logbuf_hdr_t;

/* reserved 64 byte to describe per bank header */
typedef union logbuf_bk_hdr {
    struct logbuf_bk_header {
        uint32_t tail;      /* read  counter */
        uint32_t head;      /* write counter */
        uint32_t size;      /* bank buffer size */
        uint32_t linepos;       /* pos value corresponding to '\n' */
        uint32_t flag;
    } header;
    uint8_t pad[SDRV_LOGBUF_BANK_HEADER_SIZE];
} logbuf_bk_hdr_t;

void *vuart_buffer_get_bank_hdr(core_type_t ctype)
{
    logbuf_bk_hdr_t *bank_hdr;
#if WITH_KERNEL_VM
    logbuf_hdr_t *hdr = paddr_to_kvaddr(SDRV_LOGBUF_HEADER);
#else
    logbuf_hdr_t *hdr = (logbuf_hdr_t *)SDRV_LOGBUF_HEADER;
#endif

    if (SDRV_LOGBUF_HEADER == 0x0)        /* invalid logbuf header */
        return NULL;

    if ((hdr->header.magic != SDRV_LOGBUF_MAGIC) || (ctype >= SDRV_CORE_MAX))
        return NULL;

#if WITH_KERNEL_VM
    bank_hdr = paddr_to_kvaddr((long)(SDRV_LOGBUF_HEADER +
                                      hdr->header.logoff[ctype]));
#else
    bank_hdr = (logbuf_bk_hdr_t *)((long)(SDRV_LOGBUF_HEADER +
                                          hdr->header.logoff[ctype]));
#endif
    /* the bank buffer size must be a power of two */
    bank_hdr->header.size = hdr->header.logsize[ctype] -
                            SDRV_LOGBUF_BANK_HEADER_SIZE;
    return (void *)bank_hdr;
}

int vuart_buffer_putc(void *hdr, char c)
{
    char *bbuf;
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;
    bbuf = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE);
    bbuf[bk_hdr->header.head] = c;

    if (c == '\n')
        bk_hdr->header.linepos = bk_hdr->header.head;

    bk_hdr->header.head++;

    if (bk_hdr->header.head >= bk_hdr->header.size)
        bk_hdr->header.flag |= SDLOG_BANK_BUF_FLAG_OVERRIDE;

    bk_hdr->header.head = bk_hdr->header.head & (bk_hdr->header.size - 1);
    return 0;
}

/* copy entire ring buffer */
int vuart_buffer_get_last_buf_and_size(void *hdr, char **buf,
                                       uint32_t *len)
{
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;
    uint32_t pos;

    if (bk_hdr->header.flag & SDLOG_BANK_BUF_FLAG_OVERRIDE) {
        *len = bk_hdr->header.size;
        pos = bk_hdr->header.head;
    }
    else {
        *len = bk_hdr->header.head;
        pos = 0;
    }

    *buf = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE + pos);
    return 0;
}

int vuart_buffer_get_new_buf(void *hdr, char *buf, uint32_t len)
{
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;
    char *dsrc = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE);
    uint32_t buf_len = 0;

    while ((bk_hdr->header.tail != bk_hdr->header.head) && (buf_len < len)) {
        buf[buf_len++] = dsrc[bk_hdr->header.tail];
        bk_hdr->header.tail++;
        bk_hdr->header.tail = bk_hdr->header.tail & (bk_hdr->header.size - 1);
    }

    return buf_len;
}

int vuart_buffer_update_tail(void *hdr)
{
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;
    bk_hdr->header.tail = bk_hdr->header.linepos + 1;
    return 0;
}

void vuart_buffer_clear(void *hdr)
{
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;
    char *buf = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE);
    int i = 0;

    do {
        bk_hdr->header.head = 0;
        bk_hdr->header.tail = 0;
        bk_hdr->header.linepos = 0;
        bk_hdr->header.flag = 0;
        memset(buf, 0x0, bk_hdr->header.size);
    } while ((i++ < 1) || (bk_hdr->header.head != 0)
             || (bk_hdr->header.flag != 0));
}

int vuart_buffer_is_enable(void)
{
#if WITH_KERNEL_VM
    logbuf_hdr_t *hdr = paddr_to_kvaddr(SDRV_LOGBUF_HEADER);
#else
    logbuf_hdr_t *hdr = (logbuf_hdr_t *)SDRV_LOGBUF_HEADER;
#endif

    if (hdr->header.magic != SDRV_LOGBUF_MAGIC)
        return false;

    return hdr->header.status;
}
