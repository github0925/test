/*----------------------------------------------------------------------------
 *File: jdi.h
 *
 *Copyright (c) 2006, Chips & Media.  All rights reserved.
 *----------------------------------------------------------------------------
 *Version 1.1, 2/12/2019  chentianming<tianming.chen@semidrive.com>
 *                         updata & modify this file
 */

#ifndef _JDI_HPI_H_
#define _JDI_HPI_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "jpuconfig.h"
#include "jputypes.h"

#define MAX_JPU_BUFFER_POOL 32
#define JpuWriteInstReg( INST_IDX, ADDR, DATA )     jdi_write_register( ((unsigned long)INST_IDX*NPT_REG_SIZE)+ADDR, DATA ) // system register write with instance index
#define JpuReadInstReg( INST_IDX, ADDR )            jdi_read_register( ((unsigned long)INST_IDX*NPT_REG_SIZE)+ADDR ) // system register write with instance index
#define JpuWriteReg( ADDR, DATA )                   jdi_write_register( ADDR, DATA ) // system register write
#define JpuReadReg( ADDR )                          jdi_read_register( ADDR )           // system register write
#define JpuWriteMem( ADDR, DATA, LEN, ENDIAN )      jdi_write_memory( ADDR, DATA, LEN, ENDIAN ) // system memory write
#define JpuReadMem( ADDR, DATA, LEN, ENDIAN )       jdi_read_memory( ADDR, DATA, LEN, ENDIAN ) // system memory write

typedef struct jpu_buffer_t {
	uint32_t size;
	uint64_t phys_addr;
	uint64_t base;					/* kernel logical address in use kernel */
	uint64_t virt_addr;				/* virtual user space address */
	uint64_t dma_addr;
	uint64_t buf_handle;
	uint64_t attachment;
	uint64_t sgt;
	uint64_t data_direction;
	uint64_t cached;
	uint64_t pages;
} jpu_buffer_t;


typedef struct jpu_instance_pool_t {
	unsigned char   jpgInstPool[MAX_NUM_INSTANCE][MAX_INST_HANDLE_SIZE];
	uint32_t        jpu_instance_num;
	BOOL            instance_pool_inited;
	void           *instPendingInst[MAX_NUM_INSTANCE];
} jpu_instance_pool_t;

typedef enum {
	JDI_LITTLE_ENDIAN = 0,
	JDI_BIG_ENDIAN,
	JDI_32BIT_LITTLE_ENDIAN,
	JDI_32BIT_BIG_ENDIAN,
} EndianMode;

typedef enum {
	JDI_LOG_CMD_PICRUN  = 0,
	JDI_LOG_CMD_INIT  = 1,
	JDI_LOG_CMD_RESET  = 2,
	JDI_LOG_CMD_PAUSE_INST_CTRL = 3,
	JDI_LOG_CMD_MAX
} jdi_log_cmd;

enum {
	JDI_UN_CACHED_BUFFER = 0,
	JDI_CACHED_BUFFER = 1,
};

enum {
	JDI_DMA_BIDIRECTIONAL = 0,
	JDI_DMA_TO_DEVICE = 1,
	JDI_DMA_FROM_DEVICE = 2,
	JDI_DMA_NONE = 3,
};

#if defined (__cplusplus)
extern "C" {
#endif
int jdi_probe(void);
/* @brief It returns the number of task using JDI.
 */
int jdi_get_task_num(void);
int jdi_init(void);
int jdi_release(void);    //this function may be called only at system off.
jpu_instance_pool_t *jdi_get_instance_pool(void);
int jdi_allocate_dma_memory(jpu_buffer_t *vb);
void jdi_free_dma_memory(jpu_buffer_t *vb);

int jdi_wait_interrupt(int timeout, unsigned long instIdx);
int jdi_hw_reset();
int jdi_wait_inst_ctrl_busy(int timeout, unsigned int addr_flag_reg, unsigned int flag);
int jdi_wait_bus_busy(int timeout, unsigned long instIdx);
int jdi_set_clock_gate(int enable);
int jdi_get_clock_gate();

int jdi_open_instance(unsigned long instIdx);
int jdi_close_instance(unsigned long instIdx);
int jdi_get_instance_num();

void jdi_write_register(unsigned long addr, unsigned int data);
unsigned long jdi_read_register(unsigned long addr);

void * jdi_get_memory(unsigned long addr);
int jdi_write_memory(unsigned long addr, unsigned char *data, int len, int endian);
int jdi_read_memory(unsigned long addr, unsigned char *data, int len, int endian);
int jdi_device_memory_map(jpu_buffer_t *jdb);
void jdi_device_memory_unmap(jpu_buffer_t *jdb);
void * jdi_mmap_dma_buf(jpu_buffer_t * buf);

int jdi_lock();
void jdi_unlock();
void jdi_log(int cmd, int step, int inst);

#if defined (__cplusplus)
}
#endif

#endif
