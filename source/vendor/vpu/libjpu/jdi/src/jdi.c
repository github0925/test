/*----------------------------------------------------------------------------
 * File: jdi.c
 * Copyright (c) 2006, Chips & Media.  All rights reserved.
 * Description: jpu sdk osal
 *
 * Revision Histrory:
 *----------------------------------------------------------------------------
 *Version 1.1, 2/12/2020  chentianming<tianming.chen@semidrive.com>
 *                        updata & modify this file
 */

/*************************************Include*********************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#ifdef _KERNEL_
#include <linux/delay.h>
#endif
#include <signal.h>        /* SIGIO */
#include <fcntl.h>         /* fcntl */
#include <pthread.h>
#include <sys/mman.h>      /* mmap */
#include <sys/ioctl.h>     /* fopen/fread */
#include <sys/errno.h>     /* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>

#include "jpu.h"
#include "jdi.h"
#include "jpulog.h"
#include "jputypes.h"
#include "regdefine.h"


/*************************************Macro*********************************/
#define JDI_MUTEX_DEAD_MAGIC_SIZE       4
#define JDI_NUM_LOCK_HANDLES            4
#define JDI_DRAM_PHYSICAL_BASE          0x00
#define JPU_REG_SIZE                    0x300
#define JDI_INSTANCE_POOL_SIZE          sizeof(jpu_instance_pool_t)
#define JDI_INSTANCE_POOL_TOTAL_SIZE    (JDI_INSTANCE_POOL_SIZE + sizeof(MUTEX_HANDLE)*JDI_NUM_LOCK_HANDLES)
#define JDI_DRAM_PHYSICAL_SIZE          (64*1024*1024)
#define JDI_SYSTEM_ENDIAN               JDI_LITTLE_ENDIAN
#define JPU_DEVICE_NAME                 "/dev/jpucoda"

/**********************************Data struct******************************/
typedef pthread_mutex_t    MUTEX_HANDLE;

typedef struct jpudrv_buffer_pool_t {
	jpudrv_buffer_t jdb;
	BOOL            inuse;
} jpudrv_buffer_pool_t;

typedef struct {
	Int32                   jpu_fd;
	jpu_instance_pool_t     *pjip;
	Int32                   task_num; //task of this process
	Int32                   clock_state;
	jpudrv_buffer_t         jdb_register;
	jpudrv_buffer_pool_t    jpu_buffer_pool[MAX_JPU_BUFFER_POOL];
	Int32                   jpu_buffer_pool_count;
	void                    *jpu_mutex;
	void					*jpu_mutex_dead_magic;
} jdi_info_t;

static jdi_info_t s_jdi_info;
static Int32 swap_endian(BYTE *data, Uint32 len, Uint32 endian);

/*****************************Api function define***************************/
/**
 *jdi_get_task_num - get instance count opened
 *@param: void
 *@return: count opened
 *@note:
 */
int jdi_get_task_num(void)
{
	jdi_info_t *jdi = NULL;
	jdi = &s_jdi_info;

	if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) {
		JLOG(ERR, "[jdi-%s] jpu device no open\n", __func__);
		return 0;
	}

	return jdi->task_num;
}

/**
 *jdi_init - init jpu device info, allocate instance memory from kernel
 *space, mmap register address, create mutex
 *@param: void
 *@return: negative error values, 0 on success
 *@note:
 */
int jdi_init(void)
{
	jdi_info_t *jdi = NULL;
	int i;
	jdi = &s_jdi_info;

	if (jdi->jpu_fd != -1 && jdi->jpu_fd != 0x00) {
		jdi_lock();
		jdi->task_num++;
		jdi_unlock();
		return 0;
	}

	jdi->jpu_fd = open(JPU_DEVICE_NAME, O_RDWR);

	if (jdi->jpu_fd < 0) {
		JLOG(ERR, "[jdi-%s] Can't open jpu driver, error=%s\n", __func__, strerror(errno));
		return -1;
	}

	memset(jdi->jpu_buffer_pool, 0x00, sizeof(jpudrv_buffer_pool_t)*MAX_JPU_BUFFER_POOL);

	if (!jdi_get_instance_pool()) {
		JLOG(ERR, "[jdi-%s] fail to create instance pool for saving context\n", __func__);
		return -1;
	}

	if (jdi->pjip->instance_pool_inited == FALSE) {
		Uint32 *pCodecInst;
		pthread_mutexattr_t mutexattr;
		pthread_mutexattr_init(&mutexattr);
		pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
		memset(jdi->jpu_mutex_dead_magic, 0x00, JDI_MUTEX_DEAD_MAGIC_SIZE);
#else
		/*
		 *If a process or a thread is terminated abnormally,
		 * pthread_mutexattr_setrobust_np(attr, PTHREAD_MUTEX_ROBUST_NP) makes
		 * next onwer call pthread_mutex_lock() without deadlock.
		 */
		pthread_mutexattr_setrobust_np(&mutexattr, PTHREAD_MUTEX_ROBUST_NP);
#endif
		pthread_mutex_init((MUTEX_HANDLE *)jdi->jpu_mutex, &mutexattr);

		for (i = 0; i < MAX_NUM_INSTANCE; i++) {
			pCodecInst    = (Uint32 *)jdi->pjip->jpgInstPool[i];
			pCodecInst[1] = i;    // indicate instIndex of CodecInst
			pCodecInst[0] = 0;    // indicate inUse of CodecInst
		}

		jdi->pjip->instance_pool_inited = TRUE;
	}

	if (jdi_lock() < 0) {
		JLOG(ERR, "[jdi-%s] fail to handle lock function\n", __func__);
		return -1;
	}

	if (ioctl(jdi->jpu_fd, JDI_IOCTL_GET_REGISTER_INFO, &jdi->jdb_register) < 0) {
		JLOG(ERR, "[jdi-%s] fail to get host interface register\n", __func__);
		goto ERR_JDI_INIT;
	}

	jdi->jdb_register.virt_addr = (unsigned long)mmap(NULL, jdi->jdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, jdi->jdb_register.phys_addr);

	if (jdi->jdb_register.virt_addr == (unsigned long)MAP_FAILED) {
		JLOG(ERR, "[jdi-%s] fail to map jpu registers\n", __func__);
		goto ERR_JDI_INIT;
	}

	JLOG(INFO, "map jdb_register virt addr=0x%lx, phy addr=0x%lx, size=%d\n",
        jdi->jdb_register.virt_addr, jdi->jdb_register.phys_addr, jdi->jdb_register.size);
	jdi_set_clock_gate(1);
	jdi->task_num++;
	jdi_unlock();
	JLOG(INFO, "success to init driver\n");
	return 0;
ERR_JDI_INIT:
	jdi_unlock();
	jdi_release();
	return -1;
}

/**
 *jdi_release - release jpu device info have been created
 *in jdi_init function
 *@param: void
 *@return: negative error values, 0 on success
 *@note:
 */
int jdi_release(void)
{
	jdi_info_t *jdi = NULL;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) {
		JLOG(ERR, "[jdi-%s] jpu device no open\n", __func__);
		return 0;
	}

	if (jdi_lock() < 0) {
		JLOG(ERR, "[jdi-%s] fail to handle lock function\n", __func__);
		return -1;
	}

	if (jdi->task_num == 0) {
		JLOG(ERR, "[jdi-%s] task_num is 0\n", __func__);
		jdi_unlock();
		return 0;
	}

	jdi->task_num--;

	if (jdi->task_num > 0) {/* means that the opened instance remains */
		jdi_unlock();
		return 0;
	}

	if (jdi->jdb_register.virt_addr) {
		if (munmap((void *)jdi->jdb_register.virt_addr, jdi->jdb_register.size) < 0) { //lint !e511
			JLOG(ERR, "[jdi-%s] failed to munmap\n", __func__);
		}
	}

	memset(&jdi->jdb_register, 0x00, sizeof(jpudrv_buffer_t));
	jdi_unlock();

	if (jdi->jpu_fd != -1 && jdi->jpu_fd != 0x00) {
		if (jdi->pjip != NULL) {
			if (munmap((void *)jdi->pjip, JDI_INSTANCE_POOL_TOTAL_SIZE) < 0)
				JLOG(ERR, "[jdi-%s] failed to munmap\n", __func__);
		}

		close(jdi->jpu_fd);
	}

	jdi_set_clock_gate(0);
	memset(jdi, 0x00, sizeof(jdi_info_t));
	return 0;
}

/**
 *jdi_get_instance_pool - get instance pool
 *@param: void
 *@return: null error values, success jpu_instance_pool_t * point
 *@note:
 */
jpu_instance_pool_t *jdi_get_instance_pool(void)
{
	jdi_info_t *jdi = NULL;
	jpudrv_buffer_t jdb;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return NULL;

	memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

	if (!jdi->pjip) {
		jdb.size = JDI_INSTANCE_POOL_TOTAL_SIZE;

		if (ioctl(jdi->jpu_fd, JDI_IOCTL_GET_INSTANCE_POOL, &jdb) < 0) {
			JLOG(ERR, "[jdi-%s] fail to allocate get instance pool physical space=%d\n", __func__, (int)jdb.size);
			return NULL;
		}

		jdb.virt_addr = (unsigned long)mmap(NULL, jdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, 0);

		if (jdb.virt_addr == (unsigned long)MAP_FAILED) {
			JLOG(ERR, "[jdi-%s] fail to map instance pool phyaddr=0x%lx, size = %d\n", __func__, (int)jdb.phys_addr, (int)jdb.size);
			return NULL;
		}

		jdi->pjip      = (jpu_instance_pool_t *)jdb.virt_addr;
		/**
		 * change the pointer of jpu_mutex to at end pointer of jpu_instance_pool_t
		 * to assign at allocated position.
		 */
		jdi->jpu_mutex = (void *)((unsigned long)jdi->pjip + JDI_INSTANCE_POOL_SIZE);
		jdi->jpu_mutex_dead_magic = (void *)((unsigned long)jdi->pjip + jdb.size -  JDI_NUM_LOCK_HANDLES * JDI_MUTEX_DEAD_MAGIC_SIZE);
		JLOG(INFO, "instance pool physaddr=%p, virtaddr=%p, base=%p, size=%d\n", jdb.phys_addr, jdb.virt_addr, jdb.base, jdb.size);
	}

	return (jpu_instance_pool_t *)jdi->pjip;
}

/**
 *jdi_open_instance - open one instance
 *@param: inst_idx instance id
 *@return: negative error values, 0 on success
 *@note:
 */
int jdi_open_instance(unsigned long inst_idx)
{
	jdi_info_t *jdi = NULL;
	jpudrv_inst_info_t inst_info = {0};
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return -1;

	inst_info.inst_idx = inst_idx;

	if (ioctl(jdi->jpu_fd, JDI_IOCTL_OPEN_INSTANCE, &inst_info) < 0) {
		JLOG(ERR, "[jdi-%s] fail to deliver open instance num inst_idx=%d\n", __func__, (int)inst_idx);
		return -1;
	}

	jdi->pjip->jpu_instance_num = inst_info.inst_open_count;
	return 0;
}

/**
 *jdi_close_instance - close one instance
 *@param: inst_idx  instance id
 *@return: negative error values, 0 on success
 *@note:
 */
int jdi_close_instance(unsigned long inst_idx)
{
	jdi_info_t *jdi = NULL;
	jpudrv_inst_info_t inst_info = {0};
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return -1;

	inst_info.inst_idx = inst_idx;

	if (ioctl(jdi->jpu_fd, JDI_IOCTL_CLOSE_INSTANCE, &inst_info) < 0) {
		JLOG(ERR, "[jdi-%s] fail to deliver open instance num inst_idx=%d\n", __func__, (int)inst_idx);
		return -1;
	}

	jdi->pjip->jpu_instance_num = inst_info.inst_open_count;
	return 0;
}

/**
 *jdi_get_instance_num - get have opened instance num
 *@param: void
 *@return: negative error values, 0 on success
 *@note: max instance num --four
 */
int jdi_get_instance_num(void)
{
	jdi_info_t *jdi = NULL;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return -1;

	return jdi->pjip->jpu_instance_num;
}

/**
 *jdi_hw_reset -reset jpu  device
 *@param: void
 *@return: negative error values, 0 on success
 *@note:
 */
int jdi_hw_reset(void)
{
	jdi_info_t *jdi = NULL;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return -1;

	return ioctl(jdi->jpu_fd, JDI_IOCTL_RESET, 0);
}

static void restore_mutex_in_dead(jdi_info_t *jdi)
{
	int32_t mutex_value;

	if (!jdi->jpu_mutex_dead_magic)
		return;

	memcpy(&mutex_value, jdi->jpu_mutex_dead_magic, JDI_MUTEX_DEAD_MAGIC_SIZE);

	if (mutex_value == (int32_t)0xdead10cc) { // destroy by device driver
		pthread_mutexattr_t mutexattr;
		pthread_mutexattr_init(&mutexattr);
		pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(jdi->jpu_mutex, &mutexattr);
		memset(jdi->jpu_mutex_dead_magic, 0x00, JDI_MUTEX_DEAD_MAGIC_SIZE);
		JLOG(INFO, "[jdi-%s] reinit jpu mutex\n", __func__);
	}
}

/**
 *jdi_lock - lock
 *@param: void
 *@return: negative error values, 0 on success
 *@note:
 */
int jdi_lock(void)
{
	jdi_info_t *jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) {
		JLOG(ERR, "[jdi-%s] jdi handle isn't initialized\n", __func__);
		return -1;
	}

#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
	restore_mutex_in_dead(jdi);
	pthread_mutex_lock((MUTEX_HANDLE *)jdi->jpu_mutex);
#else

	if (pthread_mutex_lock(&jdi->jpu_mutex) != 0) {
		JLOG(ERR, "[jdi-%s] failed to pthread_mutex_locK\n", __func__);
		return -1;
	}

#endif
	return 0;
}

/**
 *jdi_unlock - lock
 *@param: void
 *@return: void
 *@note:
 */
void jdi_unlock(void)
{
	jdi_info_t *jdi = NULL;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd <= 0)
		return;

	pthread_mutex_unlock((MUTEX_HANDLE *)jdi->jpu_mutex);
}

/**
 *jdi_write_register - write data to register
 *@param: addr offset to register base
 *@param: data
 *@return: void
 *@note:
 */
void jdi_write_register(unsigned long addr, unsigned int data)
{
	jdi_info_t *jdi = NULL;
	unsigned long *reg_addr;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return;

	reg_addr = (unsigned long *)(addr + (unsigned long)jdi->jdb_register.virt_addr);
	*(volatile unsigned int *)reg_addr = data;
}

/**
 *jdi_read_register - read data from register
 *@param: addr offset to register base
 *@return: read data
 *@note:
 */
unsigned long jdi_read_register(unsigned long addr)
{
	jdi_info_t *jdi = NULL;
	unsigned long *reg_addr;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return (unsigned int) -1;

	reg_addr = (unsigned long *)(addr + (unsigned long)jdi->jdb_register.virt_addr);
	return *(volatile unsigned int *)reg_addr;
}


void * jdi_mmap_dma_buf(jpu_buffer_t * buf)
{
	jdi_info_t *jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd <= 0)
		return NULL;

	return mmap(NULL, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, buf->phys_addr);
}


void * jdi_get_memory(unsigned long addr)
{
	jdi_info_t *jdi = NULL;
	jpudrv_buffer_t jdb;
	Uint32 offset = 0;
	Uint32 i;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd <= 0)
		return NULL;

	memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

	for (i = 0; i < MAX_JPU_BUFFER_POOL; i++) {

		if (jdi->jpu_buffer_pool[i].inuse == 1) {
			jdb = jdi->jpu_buffer_pool[i].jdb;

			if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
				break;
		}
	}

	if (i == MAX_JPU_BUFFER_POOL) {
		JLOG(ERR, "[jdi-%s] not found address %08x\n", __func__, addr);
		return NULL;
	}

	if (!jdb.size) {
		JLOG(ERR, "[jdi-%s] address 0x%08x is not mapped address\n", __func__, (int)addr);
		return NULL;
	}

	if (jdb.cached) {
		jdb.data_direction = JDI_DMA_FROM_DEVICE;
		if (ioctl(jdi->jpu_fd, JDI_IOCTL_MEMORY_CACHE_REFRESH, &jdb) < 0) {
			JLOG(ERR, "[jdi-%s] fail to refresh cache\n", __func__);
			return -1;
		}
	}

	offset = addr - (unsigned long)jdb.phys_addr;
	return (void *)((unsigned long)jdb.virt_addr + offset);
}

/**
 *jdi_write_memory - write data to memory
 *@param: addr phy address write to
 *@param: data want to
 *@param: len  size
 *@param: endian  big or small
 *@return: negative error values, 0 on success
 *@note:
 */
int jdi_write_memory(unsigned long addr, unsigned char *data, int len, int endian)
{
	jdi_info_t *jdi = NULL;
	jpudrv_buffer_t jdb;
	Uint32 offset = 0;
	Uint32 i;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return -1;

	memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

	for (i = 0; i < MAX_JPU_BUFFER_POOL; i++) {
		if (jdi->jpu_buffer_pool[i].inuse == 1) {
			jdb = jdi->jpu_buffer_pool[i].jdb;

			if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
				break;
		}
	}

	if (i == MAX_JPU_BUFFER_POOL) {
		JLOG(ERR, "[jdi-%s] not found address %08x\n", __func__, addr);
		return -1;
	}

	if (!jdb.size) {
		JLOG(ERR, "[jdi-%s] address 0x%08x is not mapped address\n", __func__, (int)addr);
		return -1;
	}

	offset = addr - (unsigned long)jdb.phys_addr;
	swap_endian(data, len, endian);
	memcpy((void *)((unsigned long)jdb.virt_addr + offset), data, len);

	if (jdb.cached) {
		jdb.data_direction = JDI_DMA_TO_DEVICE;
		if (ioctl(jdi->jpu_fd, JDI_IOCTL_MEMORY_CACHE_REFRESH, &jdb) < 0) {
			JLOG(ERR, "[jdi-%s] fail to refresh cache\n", __func__);
			return -1;
		}
	}

	return len;
}

/**
 *jdi_read_memory - read data from memory
 *@param: addr phy address want to read
 *@param: data write to
 *@param: len  size
 *@param: endian  big or small
 *@return: negative error values, 0 on success
 *@note:
 */
int jdi_read_memory(unsigned long addr, unsigned char *data, int len, int endian)
{
	jdi_info_t *jdi = NULL;
	jpudrv_buffer_t jdb;
	unsigned long offset = 0;
	int i;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return -1;

	memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

	for (i = 0; i < MAX_JPU_BUFFER_POOL; i++) {
		if (jdi->jpu_buffer_pool[i].inuse == 1) {
			jdb = jdi->jpu_buffer_pool[i].jdb;

			if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
				break;
		}
	}

	if (len == 0) {
		JLOG(ERR, "[jdi-%s] read memory len size = 0\n", __func__);
		return 0;
	}

	if (!jdb.size)
		return -1;

	if (jdb.cached) {
		jdb.data_direction = JDI_DMA_FROM_DEVICE;
		if (ioctl(jdi->jpu_fd, JDI_IOCTL_MEMORY_CACHE_REFRESH, &jdb) < 0) {
			JLOG(ERR, "[jdi-%s] fail to refresh cache\n", __func__);
			return -1;
		}
	}

	offset = addr - (unsigned long)jdb.phys_addr;
	memcpy(data, (const void *)((unsigned long)jdb.virt_addr + offset), len);
	swap_endian(data, len,  endian);
	return len;
}

/**
 *jdi_allocate_dma_memory - alloc dma memory
 *@param:vb dma buffer info, size/phy/virtual
 *@return: negative error values, 0 on success
 *@note: memory allocate by dma-interface in kernel space
 */
int jdi_allocate_dma_memory(jpu_buffer_t *vb)
{
	jdi_info_t *jdi = NULL;
	int i;
	jpudrv_buffer_t jdb;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return -1;

	memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));
	jdb.size = vb->size;
	jdb.cached = vb->cached;

	if (ioctl(jdi->jpu_fd, JDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY, &jdb) < 0) {
		JLOG(ERR, "[jdi-%s] fail to alloc size=%d dma memory\n", __func__, vb->size);
		return -1;
	}

	vb->phys_addr = (unsigned long)jdb.phys_addr;
	vb->base = (unsigned long)jdb.base;
	/* map to virtual address */
	jdb.virt_addr = (unsigned long)mmap(NULL, jdb.size, PROT_READ | PROT_WRITE, MAP_SHARED, jdi->jpu_fd, jdb.phys_addr);

	if (jdb.virt_addr == (unsigned long)MAP_FAILED) {
		memset(vb, 0x00, sizeof(jpu_buffer_t));
		return -1;
	}

	vb->virt_addr = jdb.virt_addr;
	jdi_lock();

	for (i = 0; i < MAX_JPU_BUFFER_POOL; i++) {
		if (jdi->jpu_buffer_pool[i].inuse == 0) {
			jdi->jpu_buffer_pool[i].jdb = jdb;
			jdi->jpu_buffer_pool_count++;
			jdi->jpu_buffer_pool[i].inuse = 1;
			break;
		}
	}

	jdi_unlock();
	JLOG(INFO, "alloc dma memory success, physaddr=%p, virtaddr=%p~%p, size=%d\n",
	     vb->phys_addr, vb->virt_addr, vb->virt_addr + vb->size, vb->size);
	return 0;
}

/**
 *jdi_free_dma_memory - free dma memory
 *@param:vb dma buffer info, size/phy/virtual
 *@return: negative error values, 0 on success
 *@note: memory allocate by dma-interface in kernel space
 */
void jdi_free_dma_memory(jpu_buffer_t *vb)
{
	jdi_info_t *jdi = NULL;
	int i;
	jpudrv_buffer_t jdb;
	jdi = &s_jdi_info;

	if (!vb || !jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return;

	if (vb->size == 0)
		return;

	memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));
	jdi_lock();

	for (i = 0; i < MAX_JPU_BUFFER_POOL; i++) {
		if (jdi->jpu_buffer_pool[i].jdb.phys_addr == vb->phys_addr) {
			jdi->jpu_buffer_pool[i].inuse = 0;
			jdi->jpu_buffer_pool_count--;
			jdb = jdi->jpu_buffer_pool[i].jdb;
			break;
		}
	}

	jdi_unlock();

	if (!jdb.size) {
		JLOG(ERR, "[jdi-%s] invalid buffer to free address = 0x%lx\n", __func__, (int)jdb.virt_addr);
		return;
	}

	ioctl(jdi->jpu_fd, JDI_IOCTL_FREE_PHYSICALMEMORY, &jdb);

	if (munmap((void *)jdb.virt_addr, jdb.size) != 0)
		JLOG(ERR, "[jdi-%s] fail to free unmap dma memory virtial address = 0x%lx\n", __func__, (int)jdb.virt_addr);

	memset(vb, 0, sizeof(jpu_buffer_t));
}

int jdi_device_memory_map(jpu_buffer_t *jdb)
{
    int ret = 0;
    jdi_info_t *jdi = &s_jdi_info;
    jpudrv_buffer_t *vb = (jpudrv_buffer_t *)jdb;

    if (!jdi || jdi->jpu_fd < 0) {
        JLOG(INFO, "[JDI-ERR] vpu have not been inited now \n");
        return -1;
    }

	if (!vb) {
		JLOG(INFO, "[JDI-ERR] device memory unmap jdb is NULL \n");
		return -1;
	}

	if ((ret = ioctl(jdi->jpu_fd, JDI_IOCTL_DEVICE_MEMORY_MAP,
                     vb) < 0)) {

        JLOG(INFO, "device memory error value %d\n", ret);
        return -1;
    }

    /* application and openMax may use phy instead dma address */
    vb->phys_addr = vb->dma_addr;
    vb->base = vb->dma_addr;
    JLOG(INFO,
         "[JDI] jdi_device_memory_map, dma_handle %d, base %#llx, dma add %#llx, phy addr=%#llx, size=%d\n",
         vb->buf_handle,
         vb->base,
         vb->dma_addr,
         vb->phys_addr,
         vb->size);
    return ret;
}

void jdi_device_memory_unmap(jpu_buffer_t *jdb)
{
    jdi_info_t *jdi = &s_jdi_info;
    jpudrv_buffer_t *vb = (jpudrv_buffer_t *)jdb;

    if (!jdi || jdi->jpu_fd < 0) {
        JLOG(INFO, "[JDI-ERR] vpu have not been inited now \n");
        return ;
    }

    if (!vb) {
        JLOG(INFO, "[JDI-ERR] device memory unmap jdb is NULL \n");
        return ;
    }

    JLOG(INFO,
        "[JDI] jdi_device_memory_unmap VA 0x%lx, PA 0x%lx, DA 0x%lx, size %u, attachment 0x%lx, sgt 0x%lx\n",
        jdb->virt_addr,
        jdb->phys_addr,
        jdb->dma_addr,
        jdb->size,
        (void *)jdb->attachment,
        (void *)jdb->sgt);

    ioctl(jdi->jpu_fd, JDI_IOCTL_DEVICE_MEMORY_UNMAP, vb);
}

/**
 *jdi_set_clock_gate - enable/disable clock gate
 *@param:enable 1/0
 *@return: negative error values, 0 on success
 *@note: core clock(cclk/aclk)
 */
int jdi_set_clock_gate(int enable)
{
	jdi_info_t *jdi = NULL;
	int ret = 0;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return -1;

	jdi->clock_state = enable;
	ret = ioctl(jdi->jpu_fd, JDI_IOCTL_SET_CLOCK_GATE, &enable);
	return ret;
}

/**
 *jdi_get_clock_gate - get clock state
 *@param:void
 *@return: negative error values, 0 on success
 *@note: core clock(cclk/aclk)
 */
int jdi_get_clock_gate(void)
{
	jdi_info_t *jdi;
	int ret = 0;
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
		return -1;

	ret = jdi->clock_state;
	return ret;
}

/**
 *jdi_wait_inst_ctrl_busy - wait instance state(busy/idea)
 *@param:timeout time
 *@param: addr_flag_reg register address
 *@param: flag
 *@return: negative error values, 0 on success
 *@note:
 */
int jdi_wait_inst_ctrl_busy(int timeout, unsigned int addr_flag_reg, unsigned int flag)
{
	Int64 elapse, cur;
	unsigned int data_flag_reg;
	struct timeval tv;
	int retry_count;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	gettimeofday(&tv, NULL);
	elapse = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	while (1) {
		data_flag_reg = jdi_read_register(addr_flag_reg);

		if (((data_flag_reg >> 4) & 0xf) == flag)
			break;

		gettimeofday(&tv, NULL);
		cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

		if (timeout > 0 && (cur - elapse) > timeout) {
			for (retry_count = 0; retry_count < 10; retry_count++)
				JLOG(ERR, "[jdi-%s] jdi wait instance ctrl timeout, 0x%x=0x%lx\n", __func__, addr_flag_reg, jdi_read_register(addr_flag_reg));

			return -1;
		}
	}

	return 0;
}

/**
 *jdi_wait_interrupt - wait interrupt
 *@param:timeout time
 *@param: addr_int_reason register address
 *@param: instIdx instance id
 *@return: interrupt reason
 *@note:
 */
int jdi_wait_bus_busy(int timeout, unsigned long instIdx)
{
	Int64 elapse, cur;
	Uint32 wresp_flag;
	struct timeval tv;
	int retry_count;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	gettimeofday(&tv, NULL);
	elapse = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	while (1) {
		wresp_flag = JpuReadInstReg(instIdx, MJPEG_WRESP_CHECK_REG);

		if (wresp_flag == 0)
			break;

		gettimeofday(&tv, NULL);
		cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

		if (timeout > 0 && (cur - elapse) > timeout) {
			for (retry_count = 0; retry_count < 10; retry_count++) {
				JLOG(ERR, "[jdi-%s] %s timeout, inst=%d, 0x%x=0x%lx\n", __func__, instIdx, MJPEG_WRESP_CHECK_REG, JpuReadInstReg(instIdx, MJPEG_WRESP_CHECK_REG));
			}

			return -1;
		}
	}

	return 0;
}

/**
 *jdi_wait_interrupt - wait interrupt
 *@param:timeout time
 *@param: addr_int_reason register address
 *@param: instIdx instance id
 *@return: interrupt reason
 *@note:
 */
int jdi_wait_interrupt(int timeout, unsigned long instIdx)
{
	int intr_reason = 0;
	jdi_info_t *jdi;
	int ret;
	jpudrv_intr_info_t intr_info = {0};
	jdi = &s_jdi_info;

	if (!jdi || jdi->jpu_fd <= 0)
		return -1;

	intr_info.timeout     = timeout;
	intr_info.intr_reason = 0;
	intr_info.inst_idx    = instIdx;
	ret = ioctl(jdi->jpu_fd, JDI_IOCTL_WAIT_INTERRUPT, (void *)&intr_info);

	if (ret != 0)
		return -1;

	intr_reason = intr_info.intr_reason;
	return intr_reason;
}

/**
 *jdi_log - jpu device debug log
 *@param:cmd
 *@param: step
 *@param: inst instance id
 *@return: void
 *@note:
 */
void jdi_log(int cmd, int step, int inst)
{
	Int32   i;

	switch (cmd) {
	case JDI_LOG_CMD_PICRUN:
		if (step == 1)    //
			JLOG(INFO, "\n**PIC_RUN INST=%d start\n", inst);
		else
			JLOG(INFO, "\n**PIC_RUN INST=%d  end\n", inst);

		break;

	case JDI_LOG_CMD_INIT:
		if (step == 1)    //
			JLOG(INFO, "\n**INIT INST=%d  start\n", inst);
		else
			JLOG(INFO, "\n**INIT INST=%d  end\n", inst);

		break;

	case JDI_LOG_CMD_RESET:
		if (step == 1)    //
			JLOG(INFO, "\n**RESET INST=%d  start\n", inst);
		else
			JLOG(INFO, "\n**RESET INST=%d  end\n", inst);

		break;

	case JDI_LOG_CMD_PAUSE_INST_CTRL:
		if (step == 1)    //
			JLOG(INFO, "\n**PAUSE INST_CTRL  INST=%d start\n", inst);
		else
			JLOG(INFO, "\n**PAUSE INST_CTRL  INST=%d end\n", inst);

		break;
	}

	for (i = (inst * NPT_REG_SIZE); i <= ((inst * NPT_REG_SIZE) + 0x250); i = i + 16) {
		JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
		     jdi_read_register(i), jdi_read_register(i + 4),
		     jdi_read_register(i + 8), jdi_read_register(i + 0xc));
	}

	JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", NPT_PROC_BASE,
	     jdi_read_register(NPT_PROC_BASE + 0x00), jdi_read_register(NPT_PROC_BASE + 4),
	     jdi_read_register(NPT_PROC_BASE + 8), jdi_read_register(NPT_PROC_BASE + 0xc));
}

static void swap_byte(Uint8 *data, Uint32 len)
{
	Uint8   temp;
	Uint32  i;

	for (i = 0; i < len; i += 2) {
		temp = data[i];
		data[i] = data[i + 1];
		data[i + 1] = temp;
	}
}

static void swap_word(Uint8 *data, Uint32 len)
{
	Uint16  temp;
	Uint16 *ptr = (Uint16 *)data;
	Int32   i, size = len / sizeof(Uint16);

	for (i = 0; i < size; i += 2) {
		temp = ptr[i];
		ptr[i] = ptr[i + 1];
		ptr[i + 1] = temp;
	}
}

static void swap_dword(Uint8 *data, Uint32 len)
{
	Uint32  temp;
	Uint32 *ptr = (Uint32 *)data;
	Int32   i, size = len / sizeof(Uint32);

	for (i = 0; i < size; i += 2) {
		temp = ptr[i];
		ptr[i] = ptr[i + 1];
		ptr[i + 1] = temp;
	}
}

Int32 swap_endian(BYTE *data, Uint32 len, Uint32 endian)
{
	Uint8   endian_mask[8] = {   // endianMask :[2] - 4byte unit swap
		0x00, 0x07, 0x04, 0x03, //              [1] - 2byte unit swap
		0x06, 0x05, 0x02, 0x01  //              [0] - 1byte unit swap
	};
	Uint8   target_endian;
	Uint8   system_endian;
	Uint8   changes;
	BOOL    byte_swap = FALSE, word_swap = FALSE, dword_swap = FALSE;

	if (endian > 7) {
		JLOG(ERR, "[jdi-%s] Invalid endian mode: %d, expected value: 0~7\n", __func__, endian);
		return -1;
	}

	target_endian = endian_mask[endian];
	system_endian = endian_mask[JDI_SYSTEM_ENDIAN];
	changes = target_endian ^ system_endian;
	byte_swap = changes & 0x01 ? TRUE : FALSE;
	word_swap = changes & 0x02 ? TRUE : FALSE;
	dword_swap = changes & 0x04 ? TRUE : FALSE;

	if (byte_swap == TRUE)
		swap_byte(data, len);

	if (word_swap == TRUE)
		swap_word(data, len);

	if (dword_swap == TRUE)
		swap_dword(data, len);

	return changes == 0 ? 0 : 1;
}


