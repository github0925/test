#ifndef _BOOTLOADER_QNX_H_
#define _BOOTLOADER_QNX_H_

#include "startup.h"
#include "partition_parser.h"
#include "ab_partition_parser.h"

#ifndef BACKDOOR_DDR
int bootloader_entry_qnx(partition_device_t *ptdev,
                         struct startup_header *startup_hdr);
#else
int bootloader_entry_qnx_backdoor_ddr(void);
#endif

#endif

