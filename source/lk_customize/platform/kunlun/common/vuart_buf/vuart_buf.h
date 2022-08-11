#ifndef _SDRV_VIRT_UART_COMMON_H_
#define _SDRV_VIRT_UART_COMMON_H_

#ifndef VIRT_UART_MEMBASE
#define SDRV_LOGBUF_HEADER      0x0  /* invalid address */
#define SDRV_LOGBUF_SIZE        0x200000
#else
#define SDRV_LOGBUF_HEADER      VIRT_UART_MEMBASE
#define SDRV_LOGBUF_SIZE        VIRT_UART_MEMSIZE
#endif

typedef enum {
    SAFETY_R5 = 0,
    SECURE_R5,
    MPC_R5,
    AP1_A55,
    AP1_USER,
    AP2_A55,
    AP2_USER,
    SDRV_CORE_MAX
} core_type_t;

void *vuart_buffer_get_bank_hdr(core_type_t ctype);
int vuart_buffer_putc(void *hdr, char c);
int vuart_buffer_get_last_buf_and_size(void *hdr, char **buf,
                                       uint32_t *len);
int vuart_buffer_get_new_buf(void *hdr, char *buf, uint32_t len);
int vuart_buffer_update_tail(void *hdr);
void vuart_buffer_clear(void *hdr);
int vuart_buffer_is_enable(void);

#endif /* _SDRV_VIRT_UART_COMMON_H_ */
