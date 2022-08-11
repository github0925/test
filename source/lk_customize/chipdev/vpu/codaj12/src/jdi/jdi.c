//------------------------------------------------------------------------------
// File: jdi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#include "jdi.h"
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
#elif defined(linux) || defined(__linux) || defined(ANDROID)
#else

#include <stdio.h>
#include <stdlib.h>
#include "jpulog.h"
#include "regdefine.h"

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <reg.h>
#include <arch/ops.h>
#include <__regs_base.h>
#include <kernel/mutex.h>
#include <heap.h>



#define JPU_BIT_REG_SIZE        0x300
#define JPU_BIT_REG_BASE        APB_MJPEG_BASE

#define JDI_DRAM_PHYSICAL_BASE  0x00
#define JDI_DRAM_PHYSICAL_SIZE  (128*1024*1024)
#define JDI_SYSTEM_ENDIAN   JDI_LITTLE_ENDIAN
unsigned long g_jdi_virt_addr;

typedef struct jpu_buffer_t jpudrv_buffer_t;

typedef struct jpu_buffer_pool_t {
    jpudrv_buffer_t jdb;
    int inuse;
} jpu_buffer_pool_t;

static int s_jpu_fd;
static jpu_instance_pool_t *s_pjip;
static jpu_instance_pool_t s_jip;
static int s_task_num;
static int s_clock_state;
static jpudrv_buffer_t s_jdb_video_memory;
static jpudrv_buffer_t s_jdb_register;
static jpu_buffer_pool_t s_jpu_buffer_pool[MAX_JPU_BUFFER_POOL];
static int s_jpu_buffer_pool_count;

static Int32 swap_endian(BYTE *data, Uint32 len, Uint32 endian);
/* Transform address from cpu address space to peripheral address space */
paddr_t p2ap(paddr_t pa);
/* Transform address from peripheral address space to cpu address space */
paddr_t ap2p(paddr_t va);

static mutex_t jdi_mutex;

/*
 * brief  Create jdi lock for jdi oper
 * Note
 */
static void jdi_lock_create(mutex_t *mutex)
{
    mutex_init(mutex);
}

/*
 * brief  Destroy jdi lock in release fuction
 * Note
 */
static void jdi_lock_destroy(mutex_t *mutex)
{
    mutex_destroy(mutex);
}


/* @return number of task
 */
int jdi_get_task_num()
{
    if (s_jpu_fd == -1 || s_jpu_fd == 0x00) {
        return 0;
    }

    return s_task_num;
}


int jdi_init(unsigned long phy_base, unsigned int reg_range)
{
    int ret;

    if (s_jpu_fd != -1 && s_jpu_fd != 0x00) {
        s_task_num++;
        return 0;
    }

    s_jpu_fd = 1;

	/*
	 * Get codaj12 hardware handle for get register info
	 */

    s_jdb_register.size = (unsigned int)(reg_range);
    s_jdb_register.phys_addr = (unsigned long)(phy_base);

    #if WITH_KERNEL_VM
        s_jdb_register.virt_addr = (unsigned long)paddr_to_kvaddr(s_jdb_register.phys_addr);
    #else
        s_jdb_register.virt_addr = (unsigned long)s_jdb_register.phys_addr;
    #endif

    memset((void *)&s_jpu_buffer_pool, 0x00, sizeof(jpu_buffer_pool_t)*MAX_JPU_BUFFER_POOL);
    s_jpu_buffer_pool_count = 0;


    if (!(s_pjip = jdi_get_instance_pool())) {
        JLOG(ERR, "[jdi] fail to create instance pool for saving context \n");
        goto ERR_JDI_INIT;
    }

    s_jdb_video_memory.size = VPU_CODAJ12_DRV_MEM_SIZE;
    //VMEM_PAGE_SIZE aligned
    s_jdb_video_memory.phys_addr = (unsigned long)(p2ap((paddr_t)memalign(VMEM_PAGE_SIZE,s_jdb_video_memory.size)));

#if WITH_KERNEL_VM
    s_jdb_video_memory.virt_addr = (unsigned long)paddr_to_kvaddr(ap2p(s_jdb_video_memory.phys_addr));
#else
    s_jdb_video_memory.virt_addr = (unsigned long)ap2p(s_jdb_video_memory.phys_addr);
#endif
    s_jdb_video_memory.base = s_jdb_video_memory.phys_addr;

    JLOG(TRACE,"jdb memory phys_addr: %p,  virt_addr:%p,  size:0x%x, base %p \n", s_jdb_video_memory.phys_addr, s_jdb_video_memory.virt_addr, s_jdb_video_memory.size, s_jdb_video_memory.base);

    if (!s_pjip->instance_pool_inited) {
        memset(&s_pjip->vmem, 0x00, sizeof(jpeg_mm_t));
        ret = jmem_init(&s_pjip->vmem, (unsigned long)s_jdb_video_memory.phys_addr, s_jdb_video_memory.size);
        if (ret < 0) {
            JLOG(ERR, "[JDI] fail to init jpu memory management logic\n");
            goto ERR_JDI_INIT;
        }
    }

    /* ctreat mutex for jdi oper */
    jdi_lock_create(&jdi_mutex);

    jdi_set_clock_gate(1);
    s_task_num++;
    JLOG(TRACE, "[jdi] success to init driver reg addr %p size:  0x%x\n", s_jdb_register.virt_addr, s_jdb_register.size);
    return s_jpu_fd;

ERR_JDI_INIT:

    jdi_release();
    return -1;
}

int jdi_release()
{

    if (s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return 0;

    if (jdi_lock() < 0) {
        JLOG(ERR, "[jdi] fail to handle lock function\n");
        return -1;
    }

    if (s_task_num > 1) { // means that the opened instance remains
        s_task_num--;
        jdi_unlock();
        return 0;
    }

    s_task_num--;

    jmem_exit(&s_pjip->vmem);

    memset(&s_jdb_register, 0x00, sizeof(jpudrv_buffer_t));

    if (s_jpu_fd != -1 && s_jpu_fd != 0x00)
        s_jpu_fd = -1;

    s_pjip = NULL;

    jdi_unlock();

    jdi_lock_destroy(&jdi_mutex);
    jdi_set_clock_gate(0);

    free((void*)ap2p(s_jdb_video_memory.phys_addr));

    return 0;
}


jpu_instance_pool_t *jdi_get_instance_pool()
{
    if (!s_pjip) {
        s_pjip = &s_jip;

        memset(s_pjip, 0x00, sizeof(jpu_instance_pool_t));
    }

    return (jpu_instance_pool_t *)s_pjip;
}

int jdi_open_instance(unsigned long instIdx)
{
    if (!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return -1;

    s_pjip->jpu_instance_num++;

    return 0;
}

int jdi_close_instance(unsigned long instIdx)
{
    if (!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return -1;

    s_pjip->jpu_instance_num--;

    return 0;
}


int jdi_get_instance_num()
{
    if (!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return -1;

    return s_pjip->jpu_instance_num;
}

int jdi_hw_reset()
{
    if (!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return -1;

    // to do any action for hw reset
    return 0;
}



int jdi_lock()
{
    mutex_acquire(&jdi_mutex);
    return 0;
}

void jdi_unlock()
{
    mutex_release(&jdi_mutex);
}

void jdi_write_register(unsigned long addr, unsigned int data)
{
    unsigned long *reg_addr;

    if (!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return;

    reg_addr = (unsigned long *)(addr + (unsigned long)s_jdb_register.virt_addr);
    writel(data, reg_addr);

#if 0
    JLOG(INFO,"w(0x%lx, 0x%08x), r(0x%08x)\n",
            reg_addr,
            data,
            readl(reg_addr));
#endif

}

unsigned long jdi_read_register(unsigned long addr)
{
    unsigned long *reg_addr;

    reg_addr = (unsigned long *)(addr + (unsigned long)s_jdb_register.virt_addr);
    //return *(volatile unsigned long *)reg_addr;

    return readl(reg_addr);
}

int jdi_write_memory(unsigned long addr, unsigned char *data, int len, int endian)
{
    jpudrv_buffer_t jdb = {0, };
    unsigned long offset;
    int i;

    if (!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return -1;

    for (i=0; i<MAX_JPU_BUFFER_POOL; i++) {
        if (s_jpu_buffer_pool[i].inuse == 1) {
            jdb = s_jpu_buffer_pool[i].jdb;
            if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
                break;
        }
    }

    if (!jdb.size) {
        JLOG(ERR, "JDIW:address 0x%08x is not mapped address!!!\n", addr);
        return -1;
    }

    offset = addr - (unsigned long)jdb.phys_addr;
    swap_endian(data, len, endian);
    //arch_clean_cache_range((addr_t)data, len);
    memcpy((void *)((unsigned long)jdb.virt_addr+offset), data, len);
    arch_clean_cache_range((addr_t)((unsigned long)jdb.virt_addr+offset), len);

    JLOG(TRACE,"JDI write DMA mem from %p:[vpu view @ 0x%x - 0x%x | cpu view @ 0x%x - 0x%x]\n",data,addr,addr+len,jdb.virt_addr,jdb.virt_addr+len);

    return len;
}

int jdi_read_memory(unsigned long addr, unsigned char *data, int len, int endian)
{
    jpudrv_buffer_t jdb = {0};
    unsigned long offset;
    int i;

    if (!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return -1;

    for (i=0; i<MAX_JPU_BUFFER_POOL; i++) {
        if (s_jpu_buffer_pool[i].inuse == 1) {
            jdb = s_jpu_buffer_pool[i].jdb;
            if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size))
                JLOG(TRACE, "GET JDB\n");
                break;
        }
    }

    if (!jdb.size)
    {
        JLOG(ERR, "JDIR:address 0x%08x is not mapped address!!!\n", addr);
        return -1;
    }


 //   JLOG(TRACE, "addrd %d, addr %p , data %p ，jdb.phys_addr %p, jdb.base %p, jdb.virt_addr %p,  jdb.size 0x%x \n",addr, addr, data, jdb.phys_addr, jdb.base, jdb.virt_addr,jdb.size);
    offset = addr - (unsigned long)jdb.phys_addr;
    arch_invalidate_cache_range((addr_t)((unsigned long)jdb.virt_addr+offset), len);
    memcpy(data, (const void *)((unsigned long)jdb.virt_addr+offset), len);
    swap_endian(data, (Uint32)len,  (Uint32)endian);

    JLOG(TRACE,"JDI read DMA mem to %p:[vpu view @ 0x%x - 0x%x | cpu view @ 0x%x - 0x%x]\n",data,addr,addr+len,jdb.virt_addr,jdb.virt_addr+len);

    return len;
}

int jdi_allocate_dma_memory(jpu_buffer_t *vb)
{
    int i;
    unsigned long offset;
    jpudrv_buffer_t jdb = {0, };

    if (!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return -1;

    jdb.size = vb->size;
    jdb.phys_addr = (unsigned long)jmem_alloc(&s_pjip->vmem, jdb.size, 0);

    if (jdb.phys_addr == (unsigned long)-1)
        return -1; // not enough memory

    offset = (unsigned long)(jdb.phys_addr - s_jdb_video_memory.phys_addr);
    jdb.base = (unsigned long )s_jdb_video_memory.base + offset;
    //jdb.virt_addr = jdb.phys_addr;
#if WITH_KERNEL_VM
    jdb.virt_addr = (unsigned long)paddr_to_kvaddr(ap2p(jdb.phys_addr));
#else
    jdb.virt_addr = ap2p(jdb.phys_addr);
#endif

    vb->phys_addr = (unsigned long)jdb.phys_addr;
    vb->base = (unsigned long)jdb.base;
    vb->virt_addr = jdb.virt_addr;


    for (i=0; i<MAX_JPU_BUFFER_POOL; i++) {
        if (s_jpu_buffer_pool[i].inuse == 0) {
            s_jpu_buffer_pool[i].jdb = jdb;
            s_jpu_buffer_pool_count++;
            s_jpu_buffer_pool[i].inuse = 1;
            break;
        }
    }

    JLOG(TRACE,"JDI allocate DMA mem:[vpu view @ 0x%x - 0x%x cpu view @ 0x%x - 0x%x]\n",vb->phys_addr,vb->phys_addr+vb->size,vb->virt_addr,vb->virt_addr+vb->size);

    return 0;
}

void jdi_free_dma_memory(jpu_buffer_t *vb)
{
    int i;
    jpudrv_buffer_t jdb = {0, };

    if (!s_pjip || s_jpu_fd == -1 || s_jpu_fd == 0x00)
        return ;

    if (vb->size == 0)
        return ;

    for (i=0; i<MAX_JPU_BUFFER_POOL; i++) {
        if (s_jpu_buffer_pool[i].jdb.phys_addr == vb->phys_addr) {
            s_jpu_buffer_pool[i].inuse = 0;
            s_jpu_buffer_pool_count--;
            jdb = s_jpu_buffer_pool[i].jdb;
            break;
        }
    }

    if (!jdb.size) {
        JLOG(ERR, "[JDI] invalid buffer to free address = 0x%x\n", (int)jdb.virt_addr);
        return ;
    }

    jmem_free(&s_pjip->vmem, (unsigned long)jdb.phys_addr, 0);
    memset(vb, 0, sizeof(jpu_buffer_t));
}


int jdi_set_clock_gate(int enable)
{
    s_clock_state = enable;
    return 0;
}


int jdi_get_clock_gate()
{

    return s_clock_state;
}

int jdi_wait_interrupt(int timeout, unsigned int addr_int_reason, unsigned long instIdx)
{
    volatile int count = 0;
    int intr_reason = 0;
    while (1) {
        intr_reason = jdi_read_register(addr_int_reason);
        if (intr_reason)
            break;

        count ++;
        if (count >(timeout*1000*200)) { //200 cycle as 1 us
            count = 0;
            JLOG(ERR,"%s [%x]%ld, [%x]%ld, [%x]%ld\n", __FUNCTION__, MJPEG_INST_CTRL_START_REG, jdi_read_register(MJPEG_INST_CTRL_START_REG),
                   MJPEG_INST_CTRL_STATUS_REG, jdi_read_register(MJPEG_INST_CTRL_STATUS_REG), MJPEG_PIC_STATUS_REG, jdi_read_register(MJPEG_PIC_STATUS_REG));
            return -1;
        }
    }

    return intr_reason;
}

int jdi_wait_inst_ctrl_busy(int timeout, unsigned int addr_flag_reg, unsigned int flag)
{
    unsigned int data_flag_reg;
    JLOG(TRACE,"%s:%d addr:%x, %x\n", __FUNCTION__, __LINE__, addr_flag_reg, jdi_read_register(addr_flag_reg));

    while (1) {
        data_flag_reg = jdi_read_register(addr_flag_reg);

        if (((data_flag_reg >> 4)&0xf) == flag) {
            break;
        }

        //if (timeout > 0 && (cur - elapse) > timeout) {
        //  JLOG(ERR, "[jDI] jdi_wait_inst_ctrl_busy timeout, 0x%x=0x%lx\n", addr_flag_reg, jdi_read_register(addr_flag_reg));
        //  return -1;
        //}
    }
    return 0;
}

void jdi_log(int cmd, int step, int inst)
{
    switch (cmd) {
        case JDI_LOG_CMD_PICRUN:
            if (step == 1)  //
                JLOG(INFO, "\n**PIC_RUN start\n");
            else
                JLOG(INFO, "\n**PIC_RUN end \n");
            break;
    }
#if 1
    int i;
    for (i=0; i<=0x238; i=i+16) {
        JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
             jdi_read_register(i), jdi_read_register(i+4),
             jdi_read_register(i+8), jdi_read_register(i+0xc));
    }
#endif
}

static void SwapByte(Uint8 *data, Uint32 len)
{
    Uint8   temp;
    Uint32  i;

    for (i=0; i<len; i+=2) {
        temp      = data[i];
        data[i]   = data[i+1];
        data[i+1] = temp;
    }
}

static void SwapWord(Uint8 *data, Uint32 len)
{
    Uint16  temp;
    Uint16 *ptr = (Uint16 *)data;
    Int32   i, size = len/sizeof(Uint16);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}

static void SwapDword(Uint8 *data, Uint32 len)
{
    Uint32  temp;
    Uint32 *ptr = (Uint32 *)data;
    Int32   i, size = len/sizeof(Uint32);

    for (i=0; i<size; i+=2) {
        temp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = temp;
    }
}


static Int32 swap_endian(BYTE *data, Uint32 len, Uint32 endian)
{
    Uint8   endianMask[8] = {   // endianMask : [2] - 4byte unit swap
        0x00, 0x07, 0x04, 0x03, //              [1] - 2byte unit swap
        0x06, 0x05, 0x02, 0x01  //              [0] - 1byte unit swap
    };
    Uint8   targetEndian;
    Uint8   systemEndian;
    Uint8   changes;
    BOOL    byteSwap=FALSE, wordSwap=FALSE, dwordSwap=FALSE;

    if (endian > 7) {
        JLOG(ERR, "Invalid endian mode: %d, expected value: 0~7\n", endian);
        return -1;
    }

    targetEndian = endianMask[endian];
    systemEndian = endianMask[JDI_SYSTEM_ENDIAN];
    changes      = targetEndian ^ systemEndian;
    byteSwap     = changes & 0x01 ? TRUE : FALSE;
    wordSwap     = changes & 0x02 ? TRUE : FALSE;
    dwordSwap    = changes & 0x04 ? TRUE : FALSE;

    if (byteSwap == TRUE)  SwapByte(data, len);
    if (wordSwap == TRUE)  SwapWord(data, len);
    if (dwordSwap == TRUE) SwapDword(data, len);

    return changes == 0 ? 0 : 1;
}


#ifdef LIB_C_STUB

/*
* newlib_stubs.c
* the bellow code is just to build ref-code. customers will removed the bellow code because they need a library which is related to the system library such as newlibc
*/
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>


#ifndef STDOUT_USART
#define STDOUT_USART 0
#endif

#ifndef STDERR_USART
#define STDERR_USART 0
#endif

#ifndef STDIN_USART
#define STDIN_USART 0
#endif

#undef errno
extern int errno;

/*
environ
A pointer to a list of environment variables and their values.
For a minimal environment, this empty list is adequate:
*/
char *__env[1] = { 0 };
char **environ = __env;

int _write(int file, char *ptr, int len);

void _exit(int status)
{
    _write(1, "exit", 4);
    while (1) {
        ;
    }
}

int _open(int file)
{
    return -1;
}
int _close(int file)
{
    return -1;
}
/*
execve
Transfer control to a new process. Minimal implementation (for a system without processes):
*/
int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}
/*
fork
Create a new process. Minimal implementation (for a system without processes):
*/

int _fork()
{
    errno = EAGAIN;
    return -1;
}
/*
fstat
Status of an open file. For consistency with other minimal implementations in these examples,
all files are regarded as character special devices.
The `sys/stat.h' header file required is distributed in the `include' subdirectory for this C library.
*/
int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

/*
getpid
Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
*/

int _getpid()
{
    return 1;
}

/*
isatty
Query whether output stream is a terminal. For consistency with the other minimal implementations,
*/
int _isatty(int file)
{
    switch (file) {
        case STDOUT_FILENO:
        case STDERR_FILENO:
        case STDIN_FILENO:
            return 1;
        default:
            //errno = ENOTTY;
            errno = EBADF;
            return 0;
    }
}


/*
kill
Send a signal. Minimal implementation:
*/
int _kill(int pid, int sig)
{
    errno = EINVAL;
    return (-1);
}

/*
link
Establish a new name for an existing file. Minimal implementation:
*/

int _link(char *old, char *new)
{
    errno = EMLINK;
    return -1;
}

/*
lseek
Set position in a file. Minimal implementation:
*/
int _lseek(int file, int ptr, int dir)
{
    return 0;
}

static char *__get_MSP(void)
{
    JLOG(ERR, "<%s:%d> Need to implement it\n", __FUNCTION__, __LINE__);
    return NULL;
}

/*
sbrk
Increase program data space.
Malloc and related functions depend on this
*/
caddr_t _sbrk(int incr)
{

    // extern char _ebss; // Defined by the linker
    char _ebss;
    static char *heap_end;
    char *prev_heap_end;

    JLOG(ERR, "<%s:%d> check _ebss to implement it\n", __FUNCTION__, __LINE__);
    if (heap_end == 0) {
        heap_end = &_ebss;
    }
    prev_heap_end = heap_end;

    char *stack = (char *) __get_MSP();
    if (heap_end + incr >  stack) {
        _write (STDERR_FILENO, "Heap and stack collision\n", 25);
        errno = ENOMEM;
        return  (caddr_t) -1;
        //abort ();
    }

    heap_end += incr;
    return (caddr_t) prev_heap_end;

}

/*
read
Read a character to a file. `libc' subroutines will use this system routine for input from all files, including stdin
Returns -1 on error or blocks until the number of characters have been read.
*/


int _read(int file, char *ptr, int len)
{
    int n;
    int num = 0;
    switch (file) {
        case STDIN_FILENO:
            for (n = 0; n < len; n++) {
                char c=0;
#if   STDIN_USART == 1
                while ((USART1->SR & USART_FLAG_RXNE) == (uint16_t)RESET) {}
                c = (char)(USART1->DR & (uint16_t)0x01FF);
#elif STDIN_USART == 2
                while ((USART2->SR & USART_FLAG_RXNE) == (uint16_t) RESET) {}
                c = (char) (USART2->DR & (uint16_t) 0x01FF);
#elif STDIN_USART == 3
                while ((USART3->SR & USART_FLAG_RXNE) == (uint16_t)RESET) {}
                c = (char)(USART3->DR & (uint16_t)0x01FF);
#endif
                *ptr++ = c;
                num++;
            }
            break;
        default:
            errno = EBADF;
            return -1;
    }
    return num;
}

/*
stat
Status of a file (by name). Minimal implementation:
int    _EXFUN(stat,( const char *__path, struct stat *__sbuf ));
*/

int _stat(const char *filepath, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

/*
times
Timing information for current process. Minimal implementation:
*/

/*
clock_t _times(struct tms *buf)
{
    return -1;
}
*/

/*
unlink
Remove a file's directory entry. Minimal implementation:
*/
int _unlink(char *name)
{
    errno = ENOENT;
    return -1;
}

/*
wait
Wait for a child process. Minimal implementation:
*/
int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}

/*
write
Write a character to a file. `libc' subroutines will use this system routine for output to all files, including stdout
Returns -1 on error or number of bytes sent
*/
int _write(int file, char *ptr, int len)
{
    int n;
    switch (file) {
        case STDOUT_FILENO: /*stdout*/
            for (n = 0; n < len; n++) {
#if STDOUT_USART == 1
                while ((USART1->SR & USART_FLAG_TC) == (uint16_t)RESET) {}
                USART1->DR = (*ptr++ & (uint16_t)0x01FF);
#elif  STDOUT_USART == 2
                while ((USART2->SR & USART_FLAG_TC) == (uint16_t) RESET) {
                }
                USART2->DR = (*ptr++ & (uint16_t) 0x01FF);
#elif  STDOUT_USART == 3
                while ((USART3->SR & USART_FLAG_TC) == (uint16_t)RESET) {}
                USART3->DR = (*ptr++ & (uint16_t)0x01FF);
#endif
            }
            break;
        case STDERR_FILENO: /* stderr */
            for (n = 0; n < len; n++) {
#if STDERR_USART == 1
                while ((USART1->SR & USART_FLAG_TC) == (uint16_t)RESET) {}
                USART1->DR = (*ptr++ & (uint16_t)0x01FF);
#elif  STDERR_USART == 2
                while ((USART2->SR & USART_FLAG_TC) == (uint16_t) RESET) {
                }
                USART2->DR = (*ptr++ & (uint16_t) 0x01FF);
#elif  STDERR_USART == 3
                while ((USART3->SR & USART_FLAG_TC) == (uint16_t)RESET) {}
                USART3->DR = (*ptr++ & (uint16_t)0x01FF);
#endif
            }
            break;
        default:
            errno = EBADF;
            return -1;
    }
    return len;
}

int _gettimeofday( struct timeval *tv, void *tzvp )
{
    Uint64 t = 0;//__your_system_time_function_here__();  // get uptime in nanoseconds
    tv->tv_sec = t / 1000000000;  // convert to seconds
    tv->tv_usec = ( t % 1000000000 ) / 1000;  // get remaining microseconds

    JLOG(WARN, "<%s:%d> NEED TO IMPLEMENT %s\n", __FUNCTION__, __LINE__, __FUNCTION__);
    return 0;
}
#endif

#endif
