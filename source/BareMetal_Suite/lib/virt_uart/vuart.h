#ifndef _SDRV_VIRT_UART_H_
#define _SDRV_VIRT_UART_H_

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

/* the following bank buffer size must be a power of two */
#ifndef LOGBUF_BANK_SAFETY_SIZE
#define LOGBUF_BANK_SAFETY_SIZE		0x20000
#endif

#ifndef LOGBUF_BANK_SECURE_SIZE
#define LOGBUF_BANK_SECURE_SIZE		0x20000
#endif

#ifndef LOGBUF_BANK_MP_SIZE
#define LOGBUF_BANK_MP_SIZE		0x20000
#endif

#ifndef LOGBUF_BANK_AP1_SIZE
#define LOGBUF_BANK_AP1_SIZE		0x40000
#endif

#ifndef LOGBUF_BANK_AP1_USER_SIZE
#define LOGBUF_BANK_AP1_USER_SIZE	0x40000
#endif

#ifndef LOGBUF_BANK_AP2_SIZE
#define LOGBUF_BANK_AP2_SIZE		0x40000
#endif

#ifndef LOGBUF_BANK_AP2_USER_SIZE
#define LOGBUF_BANK_AP2_USER_SIZE	0x40000
#endif

/* enable/disable the virtual uart */
#define SDRV_VIRT_UART_ENABLE		true

#define BITX(x)                         (1 << (x))
#define SAFETY_UART_MASK                BITX(SAFETY_R5)
#define SECURE_UART_MASK                BITX(SECURE_R5)
#define MP_UART_MASK                    BITX(MPC_R5)
#define AP1_UART_MASK                   BITX(AP1_A55)
#define AP2_UART_MASK                   BITX(AP2_A55)

/* mark the cores that need to use the physical uart port */
#define DEBUG_UART_ENABLE_MASK          (SAFETY_UART_MASK | AP1_UART_MASK)

void virt_uart_init(uint32_t base, uint32_t size);

#endif /* _SDRV_VIRT_UART_H_ */
