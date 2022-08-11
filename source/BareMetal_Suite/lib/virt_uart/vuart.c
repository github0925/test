#include <stdlib.h>
#include <debug.h>
#include <string.h>
#include "vuart.h"

#define SDRV_LOGBUF_MAGIC		0x20210618
#define SDRV_LOGBUF_HEADER_SIZE		0x80
#define SDRV_LOGBUF_BANK_HEADER_SIZE	0x40

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

void virt_uart_logbuf_bank_init(uint32_t base, core_type_t type, uint32_t size, const char *name)
{
	logbuf_hdr_t *hdr = (logbuf_hdr_t *)((long)base);

	if (type == 0)
		hdr->header.logoff[type] = SDRV_LOGBUF_HEADER_SIZE;
	else
		hdr->header.logoff[type] = hdr->header.logoff[type - 1] + hdr->header.logsize[type - 1];
	hdr->header.logsize[type] = SDRV_LOGBUF_BANK_HEADER_SIZE + size;

	DBG("%s: base: 0x%x, size: 0x%x\n", name, base + hdr->header.logoff[type],
	     hdr->header.logsize[type]);
}

void virt_uart_init(uint32_t base, uint32_t size)
{
	logbuf_hdr_t *hdr = (logbuf_hdr_t *)((long)base);
	uint32_t logbuf_end;

	if (base == 0x0)
		return;

	memset((void *)((long)(base)), 0, size);

	/* please do not adjuect the initialization order */
	virt_uart_logbuf_bank_init(base, SAFETY_R5, LOGBUF_BANK_SAFETY_SIZE, "safety");
	virt_uart_logbuf_bank_init(base, SECURE_R5, LOGBUF_BANK_SECURE_SIZE, "secure");
	virt_uart_logbuf_bank_init(base, MPC_R5, LOGBUF_BANK_MP_SIZE, "mpc");
	virt_uart_logbuf_bank_init(base, AP1_A55, LOGBUF_BANK_AP1_SIZE, "ap1");
	virt_uart_logbuf_bank_init(base, AP1_USER, LOGBUF_BANK_AP1_USER_SIZE, "ap1_user");
	virt_uart_logbuf_bank_init(base, AP2_A55, LOGBUF_BANK_AP2_SIZE, "ap2");
	virt_uart_logbuf_bank_init(base, AP2_USER, LOGBUF_BANK_AP2_USER_SIZE, "ap2_user");

	logbuf_end = hdr->header.logoff[SDRV_CORE_MAX - 1] + hdr->header.logsize[SDRV_CORE_MAX - 1];
	if ( logbuf_end > size) {
		WARN("allocated address [0x%lx - 0x%lx) out of reserved range [0x%lx - 0x%lx)\n",
		       base, base + logbuf_end, base, base + size);
		return;
	}

	hdr->header.magic = SDRV_LOGBUF_MAGIC;
	hdr->header.status = SDRV_VIRT_UART_ENABLE;
	hdr->header.uart_mask = DEBUG_UART_ENABLE_MASK;
}
