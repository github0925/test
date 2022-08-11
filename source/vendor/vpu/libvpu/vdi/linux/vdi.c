//------------------------------------------------------------------------------
// File: vdi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#if defined(linux) || defined(__linux) || defined(ANDROID)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef  _KERNEL_
#include <linux/delay.h>
#endif
#include <signal.h>         /* SIGIO */
#include <fcntl.h>          /* fcntl */
#include <pthread.h>
#include <sys/mman.h>       /* mmap */
#include <sys/ioctl.h>      /* fopen/fread */
#include <sys/errno.h>      /* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>
#include "vpu.h"
#include "../vdi.h"
#include "../vdi_osal.h"
#include "vpuapifunc.h"
#include "coda9/coda9_regdefine.h"
#include "wave/common/common_regdefine.h"
#include "wave/wave4/wave4_regdefine.h"
#include "wave/coda7q/coda7q_regdefine.h"

#ifdef ANDROID
#include "ion.h"
#include "ion_4.12.h"
#define VPU_MEMORY_USING_ION
#define ION_DEFAULT_HEAP_NAME "linux,cma"
#endif

#define VPU_DEVICE_NAME_WAVE  "/dev/vpuwave"
#define VPU_DEVICE_NAME_CODA  "/dev/vpucoda"

typedef pthread_mutex_t MUTEX_HANDLE;

//#define PAGE_SIZE    (1<<12)
//#define PAGE_MASK    (~(PAGE_SIZE -1))
//#define PAGE_ALIGN(x) ((x + PAGE_SIZE -1) & (PAGE_MASK))

#define SUPPORT_INTERRUPT
#define VPU_BIT_REG_SIZE                 (0x4000*MAX_NUM_VPU_CORE)
#define VDI_SRAM_BASE_ADDR               0xD00000  // if we can know the sram address in SOC directly for vdi layer. it is possible to set in vdi layer without allocation from driver
#define VDI_SRAM_SIZE_NONE               0x2A000   // 168K: all for secondary axi
#define VDI_SRAM_SIZE_SECAXI             0x18000   // 96K: for secondary axi; 72K: for line buffer
#define VDI_SRAM_SIZE_LINEBUFFER         0x6000    // 24K: for secondary axi; 144K: for line buffer
#define VDI_SYSTEM_ENDIAN                VDI_LITTLE_ENDIAN
#define VDI_128BIT_BUS_SYSTEM_ENDIAN     VDI_128BIT_LITTLE_ENDIAN
#define VDI_NUM_LOCK_HANDLES             4
#define VDI_SOC_SRAM_SIZE                0x40000   // 256K
#define VPU_MAX_SRAM_NUM                 2
#define SRAM_INSIDE_INDEX                0
#define SRAM_SOC_INDEX                   1
#define CACHE_DISABLE                    0
#define CACHE_ENABLE                     1
#define DMA_TO_DEVICE                    1
#define DMA_FROM_DEVICE                  2

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define VPU_CORE_BASE_OFFSET 0x4000
#endif

/**
 *Brief vpudrv_buffer_pool_t manage the memory allocated by vpu driver
 *param vdb  description of allocated memory
 *param inuse flag use/unuse
 */
typedef struct vpudrv_buffer_pool_t {
    vpudrv_buffer_t vdb;
    int inuse;
} vpudrv_buffer_pool_t;

/**
 *Brief vdi_info_t the context of vpu in user sapce
 *param core_idx coda/wave
 *param vpu_fd  file handle of vpu
 *param pvivp manage dma memory
 *param task_num current instance num
 *param vdb_register vpu register info
 *param vpu_common_memory memory used only by vpu
 *param ion_fd  ion device fd when using ion
 *param sram  sram[0] inter sram info
              sram[1] soc sram info
 */
typedef struct  {
    unsigned long core_idx;
    unsigned int product_code;
    int vpu_fd;
    vpu_instance_pool_t *pvip;
    int task_num;
    int clock_state;
    vpudrv_buffer_t vdb_register;
    vpu_buffer_t vpu_common_memory;
    vpudrv_buffer_pool_t vpu_buffer_pool[MAX_VPU_BUFFER_POOL];
    int vpu_buffer_pool_count;
    void *vpu_mutex;
    void *vpu_disp_mutex;
    void *vpu_gate_mutex;
    int ion_fd;
    int heap_mask;
    struct sram_info sram[VPU_MAX_SRAM_NUM];
} vdi_info_t;

static vdi_info_t s_vdi_info[MAX_NUM_VPU_CORE];
static pthread_mutex_t s_vdi_mutex[MAX_NUM_VPU_CORE] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

static int swap_endian(unsigned long core_idx, unsigned char *data,
                             int len, int endian);
static int allocate_common_memory(unsigned long core_idx);

#ifdef VPU_MEMORY_USING_ION
static void vdi_ion_get_default_heap(vdi_info_t *vdi, const char *heap_name);
static int vdi_ion_memory_alloc(unsigned long core_idx,
                                            vpudrv_buffer_t *vdb, uint32_t cache_enable);
static void vdi_ion_memory_free(unsigned long core_idx, vpudrv_buffer_t *vdb);
static int vdi_ion_memory_map(unsigned long core_idx, vpudrv_buffer_t *vdb);
static void vdi_ion_memory_unmap(unsigned long core_idx, vpudrv_buffer_t *vdb);
#endif

int vdi_get_memory_phy(int core_idx, vpu_buffer_t *buf)
{
    vdi_info_t *vdi = NULL;
    vdi = &s_vdi_info[core_idx];

    if (!buf) {
        VLOG(ERR, "get memory param error \n");
        return -1;
    }

    if (vdi->vpu_fd <= 0) {
        VLOG(ERR, "vpu device have not open \n");
        return -1;
    }

    if (ioctl(vdi->vpu_fd, VDI_IOCTL_DEVICE_MEMORY_MAP, buf) < 0) {
        VLOG(ERR, "get memory core_idx(%d) \n)", core_idx);
        return -1;
    }

    VLOG(INFO, "GET MEMORY ADD %p, handle : %p\n", buf->phys_addr,
         buf->buf_handle);
    return 0;
}

/**
 *@brief set seconder sram
 *@para  core_idx  vpu id
 *@para  mode  sram mode 00 10 11
 *@return 0 -1
 *@Notes: vpu using two soc-srams
 *        Line-buffer 144K, mode 10
 */
int vdi_set_sram_cfg(unsigned long core_idx, int mode)
{
    vdi_info_t *vdi = NULL;
    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];
    if(mode != MODE_SRAM_LINEBUFFER) {
        VLOG(ERR, "[VDI] set sram cfg error: %d \n", mode);
        return -1;
    }

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00) {
        VLOG(ERR, "[VDI] device core_id %d have not open. sram mode %d \n", core_idx, mode);
        return -1;
    }

    /* wave412 use soc sram */
     if (core_idx == 1) {
         if (!vdi->sram[SRAM_SOC_INDEX].id) {
             VLOG(ERR, "[VDI] invalid sram[SRAM_SOC_INDEX] memory\n");
             return -1;
         }
         VLOG(INFO, "[VDI] core_idx %d , set device sram cfg success \n", core_idx);
         return 0;
     }

    /* coda use inter sram */
    if (!vdi->sram[SRAM_INSIDE_INDEX].id) {
        VLOG(ERR, "[VDI] invalid sram[%d] memory\n", core_idx);
        return -1;
    }

    // only interanl sram need setting scr
    if(0 != ioctl(vdi->vpu_fd, VDI_IOCTL_DEVICE_SRAM_CFG, &mode)) {
        VLOG(ERR, "[VDI] set device sram cfg error \n");
        return -1;
    }

    VLOG(INFO, "[VDI] core_idx %d , set device sram cfg mode %d success \n", core_idx,  mode);
    return 0;
}


int vdi_init(unsigned long core_idx)
{
    vdi_info_t *vdi;
    int i;

    DebugLevel();

    VLOG(ERR, "MAX_NUM_VPU_CORE(%d),  core_idx(%d) \n)", MAX_NUM_VPU_CORE,
         core_idx);

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    pthread_mutex_lock(&s_vdi_mutex[core_idx]);
    vdi = &s_vdi_info[core_idx];

    if (vdi->vpu_fd != -1 && vdi->vpu_fd != 0x00) {
        vdi->task_num++;
        VLOG(INFO, "[VDI] vdi-init, have open task num %d\n", vdi->task_num);
        pthread_mutex_unlock(&s_vdi_mutex[core_idx]);
        return 0;
    }

    if (0 == core_idx)   /* for coda988 device */
        vdi->vpu_fd = open(VPU_DEVICE_NAME_CODA, O_RDWR);

    if (1 == core_idx)   /* for wave412 device */
        vdi->vpu_fd = open(VPU_DEVICE_NAME_WAVE, O_RDWR);

    if (vdi->vpu_fd < 0) {
        VLOG(ERR, "[VDI] Can't open vpu driver. [error=%s]. try to run vdi/linux/driver/load.sh script \n",
             strerror(errno));
        goto ERR_VDI_FINALIZED;
    }

#ifdef VPU_MEMORY_USING_ION
    /* open ion device for memory alloc */
    if (0 > (vdi->ion_fd = ion_open())) {
        VLOG(ERR, "Can not open ion device \n");
        goto ERR_VDI_FINALIZED;
    }

    vdi_ion_get_default_heap(vdi, ION_DEFAULT_HEAP_NAME);
#endif

    memset(&vdi->vpu_buffer_pool, 0x00,
           sizeof(vpudrv_buffer_pool_t)*MAX_VPU_BUFFER_POOL);

    if (!vdi_get_instance_pool(core_idx)) {
        VLOG(INFO, "[VDI] fail to create shared info for saving context \n");
        goto ERR_VDI_RELEASE;
    }

    if (vdi->pvip->instance_pool_inited == FALSE) {
        int *pCodecInst;
        pthread_mutexattr_t mutexattr;
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
#else
        /* If a process or a thread is terminated abnormally,
        * pthread_mutexattr_setrobust_np(attr, PTHREAD_MUTEX_ROBUST_NP) makes
        * next onwer call pthread_mutex_lock() without deadlock.
        */
        pthread_mutexattr_setrobust_np(&mutexattr, PTHREAD_MUTEX_ROBUST_NP);
#endif
        pthread_mutex_init((MUTEX_HANDLE *)vdi->vpu_mutex, &mutexattr);
        pthread_mutex_init((MUTEX_HANDLE *)vdi->vpu_disp_mutex, &mutexattr);
        pthread_mutex_init((MUTEX_HANDLE *)vdi->vpu_gate_mutex, &mutexattr);

        for ( i = 0; i < MAX_NUM_INSTANCE; i++) {
            pCodecInst = (int *)vdi->pvip->codecInstPool[i];
            pCodecInst[1] = i;  // indicate instIndex of CodecInst
            pCodecInst[0] = 0;  // indicate inUse of CodecInst
        }

        vdi->pvip->instance_pool_inited = TRUE;
    }

#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
    if (ioctl(vdi->vpu_fd, VDI_IOCTL_GET_REGISTER_INFO,
              &vdi->vdb_register) < 0) {
        VLOG(ERR, "[VDI] fail to get host interface register\n");
        goto ERR_VDI_RELEASE;
    }

#endif

#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
    vdi->vdb_register.virt_addr = (unsigned long)mmap(NULL,
                                  vdi->vdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                                  vdi->vpu_fd, vdi->vdb_register.phys_addr);
#else
    vdi->vdb_register.size = VPU_BIT_REG_SIZE;
    vdi->vdb_register.virt_addr = (unsigned long)mmap(NULL,
                                  vdi->vdb_register.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                                  vdi->vpu_fd, 0);
#endif

    if ((void *)vdi->vdb_register.virt_addr == MAP_FAILED) {
        VLOG(ERR, "[VDI] fail to map vpu registers \n");
        goto ERR_VDI_RELEASE;
    }

    VLOG(INFO,
         "[VDI] map vdb_register core_idx=%d, virtaddr=0x%lx, size=%d\n",
         core_idx, vdi->vdb_register.virt_addr, vdi->vdb_register.size);

    vdi_set_clock_gate(core_idx, 1);
    vdi->product_code = vdi_read_register(core_idx,
                                          VPU_PRODUCT_CODE_REGISTER);

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        if (vdi_read_register(core_idx,
                              W4_VCPU_CUR_PC) == 0) { // if BIT processor is not running.
            for (i = 0; i < 64; i++)
                vdi_write_register(core_idx, (i * 4) + 0x100, 0x0);
        }
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) { // CODA9XX
        if (vdi_read_register(core_idx,
                              BIT_CUR_PC) == 0) { // if BIT processor is not running.

            VLOG(INFO, "check the coda statuts: step-1.3  \n");

            for (i = 0; i < 64; i++)
                vdi_write_register(core_idx, (i * 4) + 0x100, 0x0);
        }
    }
    else {
        VLOG(ERR, "Init Unknown product id : %08x\n", vdi->product_code);
        goto ERR_VDI_RELEASE;
    }

    if (vdi_lock(core_idx) < 0) {
        VLOG(ERR, "[VDI] fail to handle lock function\n");
        goto ERR_VDI_INIT;
    }

    if(ioctl(vdi->vpu_fd, VDI_IOCTL_DEVICE_GET_SRAM_INFO, &(vdi->sram[0]))) {
        VLOG(ERR, "[VDI] fail to get sram info \n");
        goto ERR_VDI_INIT;
    }

    VLOG(INFO, "[VDI] core_idx %d;  s_sram[0].phy 0x%x s_sram[0].size %x, s_sram[1].phy 0x%x s_sram[1].size %x\n",
         core_idx, vdi->sram[0].phy, vdi->sram[0].size, vdi->sram[1].phy, vdi->sram[1].size);

    if (allocate_common_memory(core_idx) < 0) {
        VLOG(ERR, "[VDI] fail to get vpu common buffer from driver\n");
        goto ERR_VDI_INIT;
    }

    vdi->core_idx = core_idx;
    vdi->task_num++;
    vdi_unlock(core_idx);
    VLOG(INFO,
         "[VDI] success to init driver core_idx (%d), vdi->product_code(0x%x), ion fd %d, task_num %d\n",
         vdi->core_idx,
         vdi->product_code,
         vdi->ion_fd,
         vdi->task_num);

    pthread_mutex_unlock(&s_vdi_mutex[core_idx]);
    return 0;

ERR_VDI_INIT:
    vdi_unlock(core_idx);
ERR_VDI_RELEASE:
    vdi_release(core_idx);
ERR_VDI_FINALIZED:
    pthread_mutex_unlock(&s_vdi_mutex[core_idx]);
    return -1;
}

int vdi_set_bit_firmware_to_pm(unsigned long core_idx,
                               const unsigned short *code)
{
    int i;
    vpu_bit_firmware_info_t bit_firmware_info;
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    VLOG(ERR, "[VDI] 1vdi_set_bit_firmware core=%d\n",
         bit_firmware_info.core_idx);
    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return 0;

    bit_firmware_info.size = sizeof(vpu_bit_firmware_info_t);
    bit_firmware_info.core_idx = core_idx;
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    bit_firmware_info.reg_base_offset = (core_idx * VPU_CORE_BASE_OFFSET);
#else
    bit_firmware_info.reg_base_offset = 0;
#endif

    for (i = 0; i < 512; i++)
        bit_firmware_info.bit_code[i] = code[i];

    VLOG(ERR, "[VDI] vdi_set_bit_firmware core=%d\n",
         bit_firmware_info.core_idx);

    if (write(vdi->vpu_fd, &bit_firmware_info,
              bit_firmware_info.size) < 0) {
        VLOG(ERR, "[VDI] fail to vdi_set_bit_firmware core=%d\n",
             bit_firmware_info.core_idx);
        return -1;
    }

    return 0;
}

int vdi_release(unsigned long core_idx)
{
    int i;
    vpudrv_buffer_t vdb;
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    pthread_mutex_lock(&s_vdi_mutex[core_idx]);
    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00) {
        goto VDI_RELEASE_FINALIZED;
    }

    if (vdi_lock(core_idx) < 0) {
        VLOG(ERR, "[VDI] fail to handle lock function\n");
        pthread_mutex_unlock(&s_vdi_mutex[core_idx]);
        return -1;
    }

    if (vdi->task_num > 1) { // means that the opened instance remains
        vdi->task_num--;
        vdi_unlock(core_idx);
        VLOG(INFO, "[VDI] vdi-release now, have task num %d \n",
             vdi->task_num);

        goto VDI_RELEASE_FINALIZED;
    }

    if (vdi->vdb_register.virt_addr)
        munmap((void *)vdi->vdb_register.virt_addr, vdi->vdb_register.size);

    osal_memset(&vdi->vdb_register, 0x00, sizeof(vpudrv_buffer_t));
    vdb.size = 0;

    /* get common memory information to free virtual address */
    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_common_memory.phys_addr >=
                vdi->vpu_buffer_pool[i].vdb.phys_addr &&
                vdi->vpu_common_memory.phys_addr <
                (vdi->vpu_buffer_pool[i].vdb.phys_addr +
                 vdi->vpu_buffer_pool[i].vdb.size)) {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdi->vpu_buffer_pool_count--;
            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    vdi_unlock(core_idx);

#ifdef COMMON_MEMORY_USING_ION
    /* free common memory */
    vdi_free_dma_memory(core_idx, (vpu_buffer_t *)&vdb);
    memset(&vdi->vpu_common_memory, 0x00, sizeof(vpu_buffer_t));
#else
    if (vdb.size > 0) {
        VLOG(INFO, "[VDI], unmap common memory size %d, phy %p\n", vdb.size,
             (void *)vdb.phys_addr);
        munmap((void *)vdb.virt_addr, vdb.size);
        memset(&vdi->vpu_common_memory, 0x00, sizeof(vpu_buffer_t));
    }

#endif

#ifdef VPU_MEMORY_USING_ION
    /*close ion fd */
    if (vdi->ion_fd > 0) {
        ion_close(vdi->ion_fd);
        vdi->ion_fd = -1;
        VLOG(ERR, "[VDI] ion fd have been release \n");
    }
#endif

    vdi->task_num--;
    vdi_set_clock_gate(core_idx, 0);
    if (vdi->vpu_fd != -1 && vdi->vpu_fd != 0x00) {
        VLOG(INFO, "[VDI] close vpu driver vpu fd %d \n", vdi->vpu_fd );
        VLOG(INFO,"close value %d \n", close(vdi->vpu_fd));
        vdi->vpu_fd = -1;

    }

    memset(vdi, 0x00, sizeof(vdi_info_t));
    VLOG(ERR, "[VDI] vdi_release success task_num %d \n", vdi->task_num);

VDI_RELEASE_FINALIZED:
    pthread_mutex_unlock(&s_vdi_mutex[core_idx]);
    return 0;
}

int vdi_get_common_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memcpy(vb, &vdi->vpu_common_memory, sizeof(vpu_buffer_t));

    return 0;
}

/**
*@Brief: vdi_device_memory_map   Map device memory
        getting dma_addr should mmap memory,
        component can use function to get dma adrress according
        dma_handle.
        dma address attach one master device
*@param: core_idx coda988 or wave412
*@param: vdb description of memory info
*@return 0 success  -1 failure
*/
int vdi_device_memory_map(unsigned long core_idx,
                          vpu_buffer_t *vdb)
{
    int ret = 0;
    vdi_info_t *vdi = &s_vdi_info[core_idx];
    vpudrv_buffer_t *vb = (vpudrv_buffer_t *)vdb;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00
            || (0 > vdi->ion_fd)) {
        VLOG(INFO, "[VDI-ERR] vpu have not been inited now \n");
        return -1;
    }

    if ((ret = ioctl(vdi->vpu_fd, VDI_IOCTL_DEVICE_MEMORY_MAP,
                     vb) < 0)) {

        VLOG(INFO, "device memory error value %d\n", ret);
        return -1;
    }

    /* application and openMax may use phy instead dma address */
    vb->base = vb->phys_addr;
    VLOG(INFO,
         "[VDI] vdi_device_memory_map, dma_handle %d, base %#llx, dma add %#llx, phy addr=%#llx, size=%d\n",
         vb->buf_handle,
         vb->base,
         vb->dma_addr,
         vb->phys_addr,
         vb->size);
    return ret;
}

/**
*@Brief: vdi_device_memory_unmap  unmap device memory
*@param: core_idx coda988 or wave412
*@param: vdb description of memory info
*@return void
*/
void vdi_device_memory_unmap(unsigned long core_idx,
                             vpu_buffer_t *vdb)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];
    vpudrv_buffer_t *vb = (vpudrv_buffer_t *)vdb;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return ;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00 ) {
        VLOG(INFO, "[VDI-ERR] vpu have not been inited now \n");
        return ;
    }

    if (!vb) {
        VLOG(INFO, "[VDI-ERR] device memory unmap vdb is NULL \n");
        return ;
    }

    VLOG(INFO,
        "[VDI] vdi_device_memory_unmap core %d, VA 0x%" PRIx64 ", PA 0x%" PRIx64 ", DA 0x%" PRIx64 ", size %u, attachment 0x%" PRIx64 ", sgt 0x%" PRIx64 "\n",
        core_idx,
        vdb->virt_addr,
        vdb->phys_addr,
        vdb->dma_addr,
        vdb->size,
        (void *)vdb->attachment,
        (void *)vdb->sgt);

    ioctl(vdi->vpu_fd, VDI_IOCTL_DEVICE_MEMORY_UNMAP, vb);
}


#ifdef VPU_MEMORY_USING_ION
/**
 *Brief: query ion property
 *param: core_idx coda988 or wave412
 *param: vdb description of memory allcated by ion
 *return void
 */
static void vdi_ion_get_default_heap(vdi_info_t *vdi, const char *heap_name)
{
    int i = 0, count = 0;
    struct ion_heap_data *buffers = NULL;

    if (0 != ion_query_heap_cnt(vdi->ion_fd, &count)) {
        VLOG(ERR, "[VDI] get heap count error \n");
        return;
    }

    if (NULL == (buffers = (struct ion_heap_data *) malloc(sizeof(
                               struct ion_heap_data) * count)))
        return;

    /*get heap name & id & type */
    if (0 != ion_query_get_heaps(vdi->ion_fd, count, buffers)) {
        VLOG(ERR, "[VDI] get heap count error \n");
        return;
    }

    for (i = 0; i < count; i++) {
        VLOG(INFO, "\n heap info name ----------   id ---------- type -------  .....\n");
        VLOG(INFO, "-------- %s:   0x%x:       0x%x  \n",
             buffers[i].name, buffers[i].heap_id, buffers[i].type);
        if (!strcmp(buffers[i].name, heap_name)) {
            vdi->heap_mask = (1 << buffers[i].heap_id);
            VLOG(ERR, "[VDI] get heap_mask with %d\n", vdi->heap_mask);
            break;
        }
    }

    free(buffers);
    buffers = NULL;
}


/**
*Brief: vdi_ion_memory_alloc allocating dma memory by ion
*param: core_idx coda988 or wave412
*param: vdb description of memory info
*param: cache_enable  1 normal memory(cache)
*                     0 no cache memory
*return 0 success  -1 failure
*/
static int vdi_ion_memory_alloc(unsigned long core_idx,
                                vpudrv_buffer_t *vdb, uint32_t cache_enable)
{
    int ret = 0;
    int page_size = 0;
    vdi_info_t *vdi = &s_vdi_info[core_idx];

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00
            || (0 > vdi->ion_fd)) {
        VLOG(INFO, "[VDI-ERR] vpu have not been inited now \n");
        return -1;
    }

    if(4096  > (page_size = getpagesize())) {
        VLOG(INFO,"[VDI-INFO] Get page size %d\n", page_size );
        page_size = 4096;
    }

    if (0 != (ret = (ion_alloc_fd(vdi->ion_fd, vdb->size, page_size, vdi->heap_mask ? vdi->heap_mask : 0xffff,
                                   cache_enable, &(vdb->buf_handle))))) {
        VLOG(ERR, "[VDI-ERR] ion alloc memory error vaule %d, ion fd %d\n",
             ret, vdi->ion_fd);
        return -1;
    }

    return ret;
}


/**
*Brief: vdi_ion_memory_free free dma memory allocated by ion
*param: core_idx coda988 or wave412
*param: vdb description of memory allcated by ion
*return void
*/
static void vdi_ion_memory_free(unsigned long core_idx,
                                vpudrv_buffer_t *vdb)
{
    int ret = 0;
    vdi_info_t *vdi = NULL;
    vdi = &s_vdi_info[core_idx];

    if (core_idx >= MAX_NUM_VPU_CORE) {
        VLOG(INFO, "[VDI-ERR] vpu core idx error idx %d\n", core_idx);
        return;
    }

    /* free ion memory using standart file ops*/
    if (0 != (ret = close(vdb->buf_handle))) {
        VLOG(INFO, "[VDI-ERR] ion memory free dma handle err value %d\n",
             ret);
    }

    vdb->buf_handle = 0;
}

/**
*Brief: vdi_ion_memory_map map dma memory by ion
*param: core_idx coda988 or wave412
*param: vdb description of memory allcated by ion
*return success 0 failure -1
*/
static int vdi_ion_memory_map(unsigned long core_idx,
                              vpudrv_buffer_t *vdb)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    vdb->virt_addr = (unsigned long)mmap(NULL, vdb->size,
                                         PROT_READ | PROT_WRITE, MAP_SHARED, vdb->buf_handle, 0);

    if (MAP_FAILED == (void *) vdb->virt_addr) {
        VLOG(ERR, "[VDI-ERR] fail to vdi_memory_map error, phy %p,  handle %d \n",
             vdb->phys_addr,  vdb->buf_handle);
        return -1;
    }

/*
    VLOG(INFO, "[VDI] memory map success : VA %p, PA %p, DMA %p",
         vdb->virt_addr, vdb->phys_addr, vdb->dma_addr);
*/
    return 0;
}

/**
 *Brief: vdi_ion_memory_unmap unmap dma memory by ion
 *param: core_idx coda988 or wave412
 *param: vdb description of memory allcated by ion
 *return void
 */
static void vdi_ion_memory_unmap(unsigned long core_idx,
                                 vpudrv_buffer_t *vdb)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];

    if (core_idx >= MAX_NUM_VPU_CORE)
        return ;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return ;

    if (munmap((void *)vdb->virt_addr, vdb->size) != 0) {
        VLOG(ERR, "[VDI-ERR], munmap error \n");
        return;
    }
}
#endif

static int allocate_common_memory(unsigned long core_idx)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];
    vpudrv_buffer_t vdb;
    int i;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));
//    vdb.size = SIZE_COMMON * MAX_NUM_VPU_CORE;

    vdb.size = SIZE_COMMON;

#ifdef COMMON_MEMORY_USING_ION

    if (0 != vdi_allocate_dma_memory(core_idx,
                                     (struct vpu_buffer_t *)&vdb)) {
        return -1;
    }

    if (ioctl(vdi->vpu_fd, VDI_IOCTL_GET_COMMON_MEMORY, &vdb) < 0) {
        VLOG(ERR, "[VDI] fail to vdi_allocate_dma_memory size=%d\n",
             vdb.size);
        return -1;
    }

#else

    if (ioctl(vdi->vpu_fd, VDI_IOCTL_GET_COMMON_MEMORY, &vdb) < 0) {
        VLOG(ERR, "[VDI] fail to vdi_allocate_dma_memory size=%d\n",
             vdb.size);
        return -1;
    }

    VLOG(INFO,
         "[VDI] allocate_common_memory,  physaddr= %p, vdb.size %d\n",
         (void *)vdb.phys_addr, vdb.size);

    /* with 0xfe000 as the offset token, the kernel needs to be consistent */
    vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size,
                                        PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, 0xfe000);

    if ((void *)vdb.virt_addr == MAP_FAILED) {
        VLOG(ERR, "[VDI] fail to map common memory phyaddr=0x%p, size = %d, dma_addr %p\n",
             (void *)vdb.phys_addr, (int)vdb.size, vdb.dma_addr);
        return -1;
    }

#endif
    /*common buffer can not clear until system power off */
//    osal_memset((void *)vdb.virt_addr, 0, vdb.size);

    /* convert os driver buffer type to vpu buffer type */
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
    vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(
                vdb.phys_addr + (core_idx * SIZE_COMMON));
    vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base +
                                        (core_idx * SIZE_COMMON));
    vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(
                vdb.virt_addr + (core_idx * SIZE_COMMON));
#else
    vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
    vdi->pvip->vpu_common_buffer.phys_addr = (unsigned long)(
                vdb.phys_addr);
    vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base);
    vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(
                vdb.virt_addr);
#endif

    osal_memcpy(&vdi->vpu_common_memory, &vdi->pvip->vpu_common_buffer,
                sizeof(vpudrv_buffer_t));

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 0) {
            vdi->vpu_buffer_pool[i].vdb = vdb;
            vdi->vpu_buffer_pool_count++;
            vdi->vpu_buffer_pool[i].inuse = 1;
            break;
        }
    }

    VLOG(INFO,
         "[VDI] vdi_get_common_memory physaddr %p, size %d, virtaddr %p kernel base %p\n",
         (void *)vdi->vpu_common_memory.phys_addr,
         vdi->vpu_common_memory.size,
         (void *)vdi->vpu_common_memory.virt_addr,
         (void *)vdi->vpu_common_memory.base);
    return 0;
}


vpu_instance_pool_t *vdi_get_instance_pool(unsigned long core_idx)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return NULL;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00 )
        return NULL;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    if (!vdi->pvip) {
        vdb.size = sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE) *
                   VDI_NUM_LOCK_HANDLES;
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
        vdb.size  *= MAX_NUM_VPU_CORE;
#endif

        if (ioctl(vdi->vpu_fd, VDI_IOCTL_GET_INSTANCE_POOL, &vdb) < 0) {
            VLOG(ERR, "[VDI] fail to allocate get instance pool physical space=%d\n",
                 (int)vdb.size);
            return NULL;
        }

#ifdef USE_VMALLOC_FOR_INSTANCE_POOL_MEMORY
        vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size,
                                            PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, 0);
#else
        vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size,
                                            PROT_READ | PROT_WRITE, MAP_SHARED, vdi->vpu_fd, vdb.phys_addr);
#endif

        if ((void *)vdb.virt_addr == MAP_FAILED) {
            VLOG(ERR, "[VDI] fail to map instance pool phyaddr=0x%lx, size = %d\n",
                 (int)vdb.phys_addr, (int)vdb.size);
            return NULL;
        }

        //vdi->pvip = (vpu_instance_pool_t *)(vdb.virt_addr + ((sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE)*VDI_NUM_LOCK_HANDLES)));
        vdi->pvip = (vpu_instance_pool_t *)(vdb.virt_addr);
#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
        vdi->pvip = (vpu_instance_pool_t *)(vdb.virt_addr + (core_idx *
                                            (sizeof(vpu_instance_pool_t) + sizeof(MUTEX_HANDLE) *
                                             VDI_NUM_LOCK_HANDLES)))
#endif
        vdi->vpu_mutex = (void *)((unsigned long)vdi->pvip + sizeof(
                                     vpu_instance_pool_t)); //change the pointer of vpu_mutex to at end pointer of vpu_instance_pool_t to assign at allocated position.
        vdi->vpu_disp_mutex = (void *)((unsigned long)vdi->pvip + sizeof(
                                            vpu_instance_pool_t) + sizeof(MUTEX_HANDLE));
        vdi->vpu_gate_mutex = (void *)((unsigned long)vdi->pvip + sizeof(
                                            vpu_instance_pool_t) + sizeof(MUTEX_HANDLE) + sizeof(MUTEX_HANDLE));
        VLOG(INFO,
             "[VDI] instance pool physaddr=0x%p, virtaddr=0x%p, base=0x%p, size=%d\n",
             (void *)vdb.phys_addr, (void *)vdb.virt_addr, (void *)vdb.base,
             (int)vdb.size);
    }

    return (vpu_instance_pool_t *)vdi->pvip;
}

int vdi_open_instance(unsigned long core_idx, unsigned long inst_idx)
{
    vdi_info_t *vdi;
    vpudrv_inst_info_t inst_info;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    inst_info.core_idx = core_idx;
    inst_info.inst_idx = inst_idx;

    if (ioctl(vdi->vpu_fd, VDI_IOCTL_OPEN_INSTANCE, &inst_info) < 0) {
        VLOG(ERR, "[VDI] fail to deliver open instance num inst_idx=%d\n",
             (int)inst_idx);
        return -1;
    }

    vdi->pvip->vpu_instance_num = inst_info.inst_open_count;

    return 0;
}

int vdi_close_instance(unsigned long core_idx, unsigned long inst_idx)
{
    vdi_info_t *vdi;
    vpudrv_inst_info_t inst_info;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    inst_info.core_idx = core_idx;
    inst_info.inst_idx = inst_idx;

    if (ioctl(vdi->vpu_fd, VDI_IOCTL_CLOSE_INSTANCE, &inst_info) < 0) {
        VLOG(ERR, "[VDI] fail to deliver open instance num inst_idx=%d\n",
             (int)inst_idx);
        return -1;
    }

    vdi->pvip->vpu_instance_num = inst_info.inst_open_count;

    return 0;
}

int vdi_get_instance_num(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    return vdi->pvip->vpu_instance_num;
}

int vdi_hw_reset(unsigned long core_idx)
{
    vdi_info_t *vdi = NULL;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    return ioctl(vdi->vpu_fd, VDI_IOCTL_RESET, 0);

}

static void restore_mutex_in_dead(MUTEX_HANDLE *mutex)
{
    int mutex_value;

    if (!mutex)
        return;

#if defined(ANDROID)
//  mutex_value = mutex->value;   // Android O  mutex can not have value member
    mutex_value = mutex->__private[0];
#else
    memcpy(&mutex_value, mutex, sizeof(mutex_value));
#endif

    if (mutex_value == (int)0xdead10cc) { // destroy by device driver
        pthread_mutexattr_t mutexattr;
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(mutex, &mutexattr);
    }
}

int vdi_lock(unsigned long core_idx)
{
    vdi_info_t *vdi;
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
#else
    const int MUTEX_TIMEOUT = 0x7fffffff;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
    restore_mutex_in_dead((MUTEX_HANDLE *)vdi->vpu_mutex);
    pthread_mutex_lock((MUTEX_HANDLE *)vdi->vpu_mutex);
#else

    if (pthread_mutex_lock((MUTEX_HANDLE *)vdi->vpu_mutex) != 0) {
        VLOG(ERR, "%s:%d failed to pthread_mutex_locK\n", __FUNCTION__,
             __LINE__);
        return -1;
    }

#endif

    return 0;
}

int vdi_lock_check(unsigned long core_idx)
{
    vdi_info_t *vdi;
    int ret;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    ret = pthread_mutex_trylock((MUTEX_HANDLE *)vdi->vpu_mutex);

    if (ret == 0) {
        vdi_unlock(core_idx);
        return -1;
    }
    else {
        return 0;
    }
}

void vdi_unlock(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

    pthread_mutex_unlock((MUTEX_HANDLE *)vdi->vpu_mutex);
}

int vdi_disp_lock(unsigned long core_idx)
{
    vdi_info_t *vdi;
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
#else
    const int MUTEX_TIMEOUT = 5000;  // ms
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
    restore_mutex_in_dead((MUTEX_HANDLE *)vdi->vpu_disp_mutex);
    pthread_mutex_lock((MUTEX_HANDLE *)vdi->vpu_disp_mutex);
#else

    if (pthread_mutex_lock((MUTEX_HANDLE *)vdi->vpu_disp_mutex) != 0) {
        VLOG(ERR, "%s:%d failed to pthread_mutex_lock\n", __FUNCTION__,
             __LINE__);
        return -1;
    }

#endif /* ANDROID */

    return 0;
}

void vdi_disp_unlock(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

    pthread_mutex_unlock((MUTEX_HANDLE *)vdi->vpu_disp_mutex);
}

int vdi_gate_lock(unsigned long core_idx)
{
    vdi_info_t *vdi;
#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
#else
    const int MUTEX_TIMEOUT = 5000;  // ms
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

#if defined(ANDROID) || !defined(PTHREAD_MUTEX_ROBUST_NP)
    restore_mutex_in_dead((MUTEX_HANDLE *)vdi->vpu_gate_mutex);
    pthread_mutex_lock((MUTEX_HANDLE *)vdi->vpu_gate_mutex);
#else
    if (pthread_mutex_lock((MUTEX_HANDLE *)vdi->vpu_gate_mutex) != 0) {
        VLOG(ERR, "%s:%d failed to pthread_mutex_lock\n", __FUNCTION__,
             __LINE__);
        return -1;
    }
#endif /* ANDROID */

    return 0;
}

void vdi_gate_unlock(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

    pthread_mutex_unlock((MUTEX_HANDLE *)vdi->vpu_gate_mutex);
}

void vdi_write_register(unsigned long core_idx, unsigned int addr,
                        unsigned int data)
{
    vdi_info_t *vdi;
    unsigned long *reg_addr;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    reg_addr = (unsigned long *)(addr + (unsigned long)
                                 vdi->vdb_register.virt_addr + (core_idx * VPU_CORE_BASE_OFFSET));
#else
    //VLOG(ERR, " no SUPPORT_MULTI_CORE_IN_ONE_DRIVER\n ");
    reg_addr = (unsigned long *)(addr + (unsigned long)
                                 vdi->vdb_register.virt_addr);
#endif
    *(volatile unsigned int *)reg_addr = data;
//   VLOG(ERR," %s:%d: write-offset(0x%x); base-register-address(0x%x); data(0x%x)\n", __FUNCTION__,__LINE__, addr, ((unsigned long)vdi->vdb_register.virt_addr), data);
}

unsigned int vdi_read_register(unsigned long core_idx,
                               unsigned int addr)
{
    vdi_info_t *vdi;
    unsigned long *reg_addr;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return (unsigned int) -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return (unsigned int) -1;


#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    reg_addr = (unsigned long *)(addr + (unsigned long)
                                 vdi->vdb_register.virt_addr + (core_idx * VPU_CORE_BASE_OFFSET));
#else
    reg_addr = (unsigned long *)(addr + (unsigned long)
                                 vdi->vdb_register.virt_addr);
#endif
    //  VLOG(ERR,"%s:%d: read-register-offset(0x%x); base-register-address(0x%x); data(0x%x)\n",__FUNCTION__,__LINE__, addr, ((unsigned long)vdi->vdb_register.virt_addr), (*(volatile unsigned int *)reg_addr));
    return *(volatile unsigned int *)reg_addr;
}

#define FIO_TIMEOUT         100

unsigned int vdi_fio_read_register(unsigned long core_idx,
                                   unsigned int addr)
{
    unsigned int ctrl;
    unsigned int count = 0;
    unsigned int data  = 0xffffffff;

    ctrl  = (addr & 0xffff);
    ctrl |= (0 << 16);  /* read operation */
    vdi_write_register(core_idx, W4_VPU_FIO_CTRL_ADDR, ctrl);
    count = FIO_TIMEOUT;

    while (count--) {
        ctrl = vdi_read_register(core_idx, W4_VPU_FIO_CTRL_ADDR);

        if (ctrl & 0x80000000) {
            data = vdi_read_register(core_idx, W4_VPU_FIO_DATA);
            break;
        }
    }

    return data;
}

void vdi_fio_write_register(unsigned long core_idx, unsigned int addr,
                            unsigned int data)
{
    unsigned int ctrl;

    vdi_write_register(core_idx, W4_VPU_FIO_DATA, data);
    ctrl  = (addr & 0xffff);
    ctrl |= (1 << 16);  /* write operation */
    vdi_write_register(core_idx, W4_VPU_FIO_CTRL_ADDR, ctrl);
}


#define VCORE_DBG_ADDR(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x300
#define VCORE_DBG_DATA(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x304
#define VCORE_DBG_READY(__vCoreIdx)     0x8000+(0x1000*__vCoreIdx) + 0x308

static void UNREFERENCED_FUNCTION write_vce_register(
    unsigned int   core_idx,
    unsigned int   vce_core_idx,
    unsigned int   vce_addr,
    unsigned int   udata
)
{
    int vcpu_reg_addr;

    vdi_fio_write_register(core_idx, VCORE_DBG_READY(vce_core_idx), 0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(core_idx, VCORE_DBG_DATA(vce_core_idx), udata);
    vdi_fio_write_register(core_idx, VCORE_DBG_ADDR(vce_core_idx),
                           (vcpu_reg_addr) & 0x00007FFF);

    while (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) < 0) {
        VLOG(ERR, "failed to write VCE register: 0x%04x\n", vce_addr);
    }
}

Uint32 ReadRegVCE(
    Uint32 core_idx,
    Uint32 vce_core_idx,
    Uint32 vce_addr
    )
{
    int     vcpu_reg_addr;
    int     udata;
    int     vce_core_base = 0x8000 + 0x1000*vce_core_idx;

    SetClockGate(core_idx, 1);
    vdi_fio_write_register(core_idx, VCORE_DBG_READY(vce_core_idx), 0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(core_idx, VCORE_DBG_ADDR(vce_core_idx),vcpu_reg_addr + vce_core_base);

    if (vdi_fio_read_register(core_idx, VCORE_DBG_READY(vce_core_idx)) == 1)
        udata= vdi_fio_read_register(core_idx, VCORE_DBG_DATA(vce_core_idx));
    else {
        VLOG(ERR, "failed 4 to read VCE register: core id %  %d, 0x%04x\n", core_idx, vce_core_idx, vce_addr);
        udata = -2;//-1 can be a valid value
    }

    SetClockGate(core_idx, 0);
    return udata;
}


static unsigned int read_vce_register(
    unsigned int core_idx,
    unsigned int vce_core_idx,
    unsigned int vce_addr
)
{
    int     vcpu_reg_addr;
    int     udata;
    int     vce_core_base = 0x8000 + 0x1000 * vce_core_idx;

    vdi_fio_write_register(core_idx, VCORE_DBG_READY(vce_core_idx), 0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(core_idx, VCORE_DBG_ADDR(vce_core_idx),
                           vcpu_reg_addr + vce_core_base);

    while (TRUE) {
        if (vdi_fio_read_register(core_idx, VCORE_DBG_READY(vce_core_idx)) == 1) {
            udata = vdi_fio_read_register(core_idx, VCORE_DBG_DATA(vce_core_idx));
            break;
        }
    }

    return udata;
}


int vdi_clear_memory(unsigned long core_idx, unsigned int addr,
                     int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
    unsigned long offset;

    int i;
    Uint8  *zero;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 1) {
            vdb = vdi->vpu_buffer_pool[i].vdb;

            if (addr >= vdb.phys_addr && addr < (vdb.phys_addr + vdb.size))
                break;
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address, endian %d !!!\n",
             (int)addr, endian);
        return -1;
    }

    zero = (Uint8 *)osal_malloc(len);
    osal_memset((void *)zero, 0x00, len);

    offset = addr - (unsigned long)vdb.phys_addr;
    osal_memcpy((void *)((unsigned long)vdb.virt_addr + offset), zero,
                len);

    osal_free(zero);

    return len;
}

void vdi_set_sdram(unsigned long coreIdx, unsigned int addr, int len,
                   unsigned char data, int endian)
{
    VLOG(INFO,
         "[VDI] sdram common in ..coredIdx(%d), len(%d), endian(%d). data %d \n",
         coreIdx, len, endian, data);

    vdi_info_t *vdi = &s_vdi_info[coreIdx];
    unsigned char *buf;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return ;

    buf = (unsigned char *)osal_malloc(len);
    memset(buf, 0x00, len);
    vdi_write_memory(coreIdx, addr, buf, len, endian);
    free(buf);
}

int vdi_write_memory(unsigned long core_idx, unsigned int addr,
                     unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
    unsigned long offset;
    int i;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 1) {
            vdb = vdi->vpu_buffer_pool[i].vdb;

            if (addr >= vdb.phys_addr && addr < (vdb.phys_addr + vdb.size)) {
                break;
            }
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
        return -1;
    }


    extern int flagmem;
    offset = addr - (unsigned long)vdb.phys_addr;
    swap_endian(core_idx, data, len, endian);
#if 0

    if (flagmem == 1) {
        VLOG(ERR, "CTM Memory copy o virt(0x%p), +offset(0x%p),  data(%p), len%d()\n",
             vdb.virt_addr,  vdb.virt_addr + offset, data, len);
        flagmem = 0;
        int i = 0;

        for (i = 0; i < len; i++) {
            osal_memcpy((void *)((unsigned long)vdb.virt_addr + offset + i),
                        data + i, 1);
        }
    }
    else {
        //  osal_memcpy((void *)((unsigned long)vdb.virt_addr+offset), data, len);
        int i = 0;

        for (i = 0; i < len; i++) {
            osal_memcpy((void *)((unsigned long)vdb.virt_addr + offset + i),
                        data + i, 1);
        }
    }

#else
    {
        osal_memcpy((void *)((unsigned long)vdb.virt_addr + offset), data,
                    len);
    }
#endif
    return len;
}

/**
*brief refresh normal memory, sync memory and cache data
*para  core_idx  vpu core id
*para  vb  buffer info contain size and dma address
*return 0  success
*        -1 failure
*/
static int vdi_refresh_memory_cache(unsigned long core_idx,vpudrv_buffer_t *vb)
{
    vdi_info_t *vdi = NULL;
    vpudrv_buffer_t vdb = {0};

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00 || !vb) {
        VLOG(ERR, "vdi memory refresh param error \n");
        return -1;
    }

    memcpy(&vdb, vb, sizeof(vpudrv_buffer_t));

    if (0 != ioctl(vdi->vpu_fd, VDI_IOCTL_MEMORY_CACHE_REFRESH, &vdb)) {
        VLOG(INFO, "[VDI], vdi refresh memory cache \n");
        return -1;
    }

    return 0;
}

static int vdi_read_memory_internal(unsigned long core_idx, unsigned int addr,
                     unsigned char *data, int len, int endian, int32_t cache_enable)
{
    vdi_info_t *vdi = NULL;
    vpudrv_buffer_t vdb = {0};
    unsigned long offset = 0;
    int i = 0;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 1) {
            vdb = vdi->vpu_buffer_pool[i].vdb;

            if (addr >= vdb.phys_addr && addr < (vdb.phys_addr + vdb.size))
                break;
        }
    }

    if (!vdb.size)
        return -1;

    /* read normal memory must sync memory and cache */
    if (cache_enable) {
        /* vpu as master write data to memory */
        vdb.data_direction = DMA_FROM_DEVICE;
        if (vdi_refresh_memory_cache(core_idx, &vdb)) {
            VLOG(ERR, "vdi refresh memory cache error \n");
            return -1;
        }
    }

    offset = addr - (unsigned long)vdb.phys_addr;
    osal_memcpy(data, (const void *)((unsigned long)vdb.virt_addr +
                                     offset), len);
    swap_endian(core_idx, data, len,  endian);
    return len;
}

int vdi_read_memory(unsigned long core_idx, unsigned int addr,
                    unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi = NULL;
    int length = 0;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00 || !data)
        return -1;

    length = vdi_read_memory_internal(core_idx, addr, data, len, endian, (int32_t)CACHE_DISABLE);

    return length;
}

/**
 *@brief read memory with cache
 *@param  core_idx  vpu core id
 *@param  addr
 *@param  data dst
 *@param  len read length
 *@param  endain  data endain
 *@return read length
 */
int vdi_read_cache_memory(unsigned long core_idx, unsigned int addr,
                    unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi = NULL;
    int length = 0;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00 || !data)
        return -1;

    length = vdi_read_memory_internal(core_idx, addr, data, len, endian, (int32_t)CACHE_ENABLE);
    return length;
}


static int vdi_allocate_dma_memory_internal(unsigned long core_idx, vpu_buffer_t *vb, uint32_t cache_enable)
{
    vdi_info_t *vdi;
    int i;
    vpudrv_buffer_t vdb;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00 || !vb)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));
    vdb.size = vb->size;

#ifdef VPU_MEMORY_USING_ION

    if (vdi_ion_memory_alloc(core_idx, &vdb, cache_enable) < 0) {
        VLOG(ERR, "[VDI] vpu alloc memory fail\n");
        return -1;
    }

    /*
     * one:map device memory, vpu can access to
     *         memory as mater device
     * two:get dma phy for writing to vpu register
     *     when register framebuffer
     */
    if ((vdi_device_memory_map(core_idx,
                               (struct vpu_buffer_t *) &vdb) < 0)) {

        VLOG(INFO, "device memory map error \n");
        return -1;
    }

    /* map dam buffer--> virt to user space */
    if (vdi_ion_memory_map(core_idx, &vdb) < 0) {
        VLOG(ERR, "vpu map memory fail \n");
        return -1;
    }

    memcpy(vb, &vdb, sizeof(struct vpu_buffer_t));

    /* application and openMax may use phy instend dma address */
    vb->phys_addr = vb->dma_addr;
    vb->base = vb->dma_addr;
    vdb.base = vdb.dma_addr;
    vdb.phys_addr = vdb.dma_addr;
#else

    if (ioctl(vdi->vpu_fd, VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY,
              &vdb) < 0) {
        VLOG(ERR, "[VDI] fail to vdi_allocate_dma_memory size=%d\n",
             vb->size);
        return -1;
    }

    //map to virtual address
    vdb.virt_addr = (unsigned long)mmap(NULL, vdb.size,
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED, vdi->vpu_fd, vdb.dma_addr);

    if ((void *)vdb.virt_addr == MAP_FAILED) {
        memset(vb, 0x00, sizeof(vpu_buffer_t));
        return -1;
    }

    vdb.dma_addr = vdb.phys_addr;
    memcpy(vb, &vdb, sizeof(vpu_buffer_t));

#endif

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 0) {
            vdi->vpu_buffer_pool[i].vdb = vdb;
            vdi->vpu_buffer_pool_count++;
            vdi->vpu_buffer_pool[i].inuse = 1;
            break;
        }
    }

    VLOG(INFO,
         "[VDI] vdi_allocate_dma_memory, dma_handle %d, base %#llx, dma add %#llx, phy addr=%#llx, virt addr=%#llx,size=%d, cache_enable %d \n",
         vb->buf_handle,
         vb->base,
         vb->dma_addr,
         vb->phys_addr,
         vb->virt_addr,
         vb->size,
         cache_enable);
    return 0;
}

/**
 *@brief alloc no cache memory
 *@para  core_idx  vpu core id
 *@para  vb  contains the size allocated
 *@return 0  success
 *        -1 failure
 */
int vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00 || !vb) {
        VLOG(ERR, "vdi alloc normal dma memory param error \n");
        return -1;
    }

    if (0 != vdi_allocate_dma_memory_internal(core_idx, vb, (uint32_t)CACHE_DISABLE)) {
        VLOG(ERR, "vdi alloc normal dma memory failure\n");
        return -1;
    }

    return 0;
}

/**
 *@brief alloc normal memory(cache)
 *@para  core_idx  vpu core id
 *@para  vb  contains the size allocated
 *@return 0  success
 *        -1 failure
 */
int vdi_allocate_dma_cache_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00 || !vb) {
        VLOG(ERR, "vdi alloc cache dma memory param error \n");
        return -1;
    }

    if (0 != vdi_allocate_dma_memory_internal(core_idx, vb, (uint32_t)CACHE_ENABLE)) {
        VLOG(ERR, "vdi alloc cache dma memory failure\n");
        return -1;
    }

    return 0;
}

int vdi_attach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    vpudrv_buffer_t vdb;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    vdb.size = vb->size;
    /* vb phys_addr in kernel */
    vdb.phys_addr = vb->phys_addr;
    /* vb  virt addr in kernel */
    vdb.base = vb->base;
    /* vb virt addr in user space */
    vdb.virt_addr = vb->virt_addr;

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr) {
            vdi->vpu_buffer_pool[i].vdb = vdb;
            vdi->vpu_buffer_pool[i].inuse = 1;
            break;
        }
        else {
            if (vdi->vpu_buffer_pool[i].inuse == 0) {
                vdi->vpu_buffer_pool[i].vdb = vdb;
                vdi->vpu_buffer_pool_count++;
                vdi->vpu_buffer_pool[i].inuse = 1;
                break;
            }
        }
    }

    VLOG(INFO,
         "[VDI] vdi_attach_dma_memory, physaddr=%p, virtaddr=%p, size=%d, index=%d\n",
         vb->phys_addr, vb->virt_addr, vb->size, i);
    return 0;
}

int vdi_dettach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vb || !vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (vb->size == 0)
        return -1;

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr) {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdi->vpu_buffer_pool_count--;
            break;
        }
    }

    return 0;
}

void vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    vpudrv_buffer_t vdb;

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
    core_idx = 0;
#endif

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if (!vb || !vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

    if (vb->size == 0)
        return ;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr) {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdi->vpu_buffer_pool_count--;
            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "[VDI] invalid buffer to free address = %ud\n",
             vdb.virt_addr);
        return ;
    }

#ifdef VPU_MEMORY_USING_ION
    /* unmap device memory */
    vdi_device_memory_unmap(core_idx, (vpu_buffer_t *)&vdb);

    /* unmap user virt  */
    vdi_ion_memory_unmap(core_idx, &vdb);

    /* close dma handle  */
    vdi_ion_memory_free(core_idx, &vdb);
#else
    ioctl(vdi->vpu_fd, VDI_IOCTL_FREE_PHYSICALMEMORY, &vdb);

    if (munmap((void *)vdb.virt_addr, vdb.size) != 0) {
        VLOG(ERR, "[VDI] vpu fail to vdi_free_dma_memory virtial address =%p\n",
             (void *)vdb.virt_addr);
    }

#endif

    osal_memset(vb, 0, sizeof(vpu_buffer_t));
}

int vdi_get_sram_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi = NULL;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vb || !vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(vb, 0x00, sizeof(vpu_buffer_t));

    /**
     * soc sram3
     */
    if(vdi->sram[SRAM_SOC_INDEX].id) {
        vb->phys_addr = vdi->sram[SRAM_SOC_INDEX].phy;
        vb->size = vdi->sram[SRAM_SOC_INDEX].size;
    }

    VLOG(INFO, "core_idx %d sram size %d, sram addr 0x%x\n", core_idx, vb->size, vb->phys_addr);
    return 0;
}


int vdi_set_clock_gate(unsigned long core_idx, int enable)
{
    vdi_info_t *vdi = NULL;
    int ret = 0;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (vdi->product_code == WAVE510_CODE
            || vdi->product_code == WAVE512_CODE) {
        return 0;
    }

    if(vdi->clock_state != enable) {
        vdi->clock_state = enable;
        ret = ioctl(vdi->vpu_fd, VDI_IOCTL_SET_CLOCK_GATE, &enable);
        VLOG(INFO, "[VDI], vdi get clock_state success enable value %d \n", enable);
    }

    return ret;
}

int vdi_get_clock_gate(unsigned long core_idx)
{
    vdi_info_t *vdi = NULL;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    return vdi->clock_state;
}

int vdi_wait_bus_busy(unsigned long core_idx, int timeout,
                      unsigned int gdi_busy_flag)
{
    Int64 elapse, cur;
    struct timeval tv;
    vdi_info_t *vdi;

    vdi = &s_vdi_info[core_idx];
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    elapse = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    while (1) {
        if (vdi->product_code == WAVE420SN_CODE) {
            if ( gdi_busy_flag == W4_FIO_VCORE6_BUS_CHANNEL_SELECTOR) {
                if (vdi_fio_read_register(core_idx,
                                          gdi_busy_flag) == 0xFFFFFFFF) break;
            }
            else {
                if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0) break;
            }
        }
        else if (vdi->product_code == WAVE420L_CODE) {
            if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0)  {
                break;
            }
        }
        else if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
            if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0x738) break;
        }
        else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
            if (vdi_read_register(core_idx, gdi_busy_flag) == 0x77) break;
        }
        else {
            VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
            return -1;
        }

        gettimeofday(&tv, NULL);
        cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        if ((cur - elapse) > timeout) {
            VLOG(ERR, "[VDI] vdi_wait_bus_busy timeout, PC=0x%lx\n",
                 vdi_read_register(core_idx, 0x018));
            return -1;
        }
    }

    return 0;

}

int vdi_wait_vpu_busy(unsigned long core_idx, int timeout,
                      unsigned int addr_bit_busy_flag)
{
    Int64 elapse, cur;
    struct timeval tv;
    Uint32 pc;
    Uint32 code, normalReg = TRUE;


    tv.tv_sec = 0;
    tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    elapse = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    code = vdi_read_register(core_idx,
                             VPU_PRODUCT_CODE_REGISTER); /* read product code */

    if (PRODUCT_CODE_W_SERIES(code)) {
        pc = W4_VCPU_CUR_PC;

        if (addr_bit_busy_flag & 0x8000) normalReg = FALSE;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(code)) {
        pc = BIT_CUR_PC;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", code);
        return -1;
    }

    while (1) {
        if (normalReg == TRUE) {
            if (vdi_read_register(core_idx, addr_bit_busy_flag) == 0) break;
        }
        else {
            if (vdi_fio_read_register(core_idx, addr_bit_busy_flag) == 0) break;
        }

        gettimeofday(&tv, NULL);
        cur = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        if ((cur - elapse) > timeout) {
            Uint32 index;

            for (index = 0; index < 50; index++) {
                VLOG(ERR, "[VDI] vdi_wait_vpu_busy timeout, PC=0x%lx\n",
                     vdi_read_register(core_idx, pc));
            }

            return -1;
        }
    }

    return 0;

}


int vdi_wait_interrupt(unsigned long coreIdx, int timeout,
                       unsigned int addr_bit_int_reason)
{
    int intr_reason = 0;
    int ret;
    vdi_info_t *vdi;
    vpudrv_intr_info_t intr_info;

    if (coreIdx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[coreIdx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

#ifdef SUPPORT_INTERRUPT
    intr_info.timeout     = timeout;
    intr_info.intr_reason = 0;
    ret = ioctl(vdi->vpu_fd, VDI_IOCTL_WAIT_INTERRUPT,
                (void *)&intr_info);

    if (ret != 0) {
        VLOG(ERR, "product id : %08x, addr_bit_int_reason 0x%x, intr_info.intr_reason 0x%x\n",
             vdi->product_code, addr_bit_int_reason, intr_info.intr_reason);
        return -1;
    }

    intr_reason = intr_info.intr_reason;
#else
    struct timeval  tv = {0};
    uint32_t        intrStatusReg;
    uint32_t        pc;
    int32_t         startTime, endTime, elaspedTime;

    UNREFERENCED_PARAMETER(intr_info);

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        pc            = W4_VCPU_CUR_PC;
        intrStatusReg = W4_VPU_VPU_INT_STS;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        pc            = BIT_CUR_PC;
        intrStatusReg = BIT_INT_STS;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    gettimeofday(&tv, NULL);
    startTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    while (TRUE) {
        if (vdi_read_register(coreIdx, intrStatusReg)) {
            if ((intr_reason = vdi_read_register(coreIdx, addr_bit_int_reason)))
                break;
        }

        gettimeofday(&tv, NULL);
        endTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        if ((endTime - startTime) >= timeout) {
            return -1;
        }
    }

#endif

    return intr_reason;
}


static int read_pinfo_buffer(int core_idx, int addr)
{
    int ack;
    int rdata;
#define VDI_LOG_GDI_PINFO_ADDR  (0x1068)
#define VDI_LOG_GDI_PINFO_REQ   (0x1060)
#define VDI_LOG_GDI_PINFO_ACK   (0x1064)
#define VDI_LOG_GDI_PINFO_DATA  (0x106c)
    //------------------------------------------
    // read pinfo - indirect read
    // 1. set read addr     (GDI_PINFO_ADDR)
    // 2. send req          (GDI_PINFO_REQ)
    // 3. wait until ack==1 (GDI_PINFO_ACK)
    // 4. read data         (GDI_PINFO_DATA)
    //------------------------------------------
    vdi_write_register(core_idx, VDI_LOG_GDI_PINFO_ADDR, addr);
    vdi_write_register(core_idx, VDI_LOG_GDI_PINFO_REQ, 1);

    ack = 0;

    while (ack == 0) {
        ack = vdi_read_register(core_idx, VDI_LOG_GDI_PINFO_ACK);
    }

    rdata = vdi_read_register(core_idx, VDI_LOG_GDI_PINFO_DATA);

    //VLOG(INFO,"[READ PINFO] ADDR[%x], DATA[%x]", addr, rdata);
    return rdata;
}


enum {
    VDI_PRODUCT_ID_980,
    VDI_PRODUCT_ID_960
};

static void printf_gdi_info(int core_idx, int num, int reset)
{
    int i;
    int bus_info_addr;
    int tmp;
    int val;
    int productId = 0;

    val = vdi_read_register(core_idx, VPU_PRODUCT_CODE_REGISTER);

    if ((val & 0xff00) == 0x3200) val = 0x3200;

    if (PRODUCT_CODE_W_SERIES(val)) {
        return;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(val)) {
        if (val == CODA960_CODE || val == BODA950_CODE)
            productId = VDI_PRODUCT_ID_960;
        else if (val == CODA980_CODE || val == WAVE320_CODE)
            productId = VDI_PRODUCT_ID_980;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", val);
        return;
    }

    if (productId == VDI_PRODUCT_ID_980)
        VLOG(INFO, "\n**GDI information for GDI_20\n");
    else
        VLOG(INFO, "\n**GDI information for GDI_10\n");

    for (i = 0; i < num; i++) {

#define VDI_LOG_GDI_INFO_CONTROL 0x1400

        if (productId == VDI_PRODUCT_ID_980)
            bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i * (0x20);
        else
            bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i * 0x14;

        if (reset) {
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);

            if (productId == VDI_PRODUCT_ID_980) {
                bus_info_addr += 4;
                vdi_write_register(core_idx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(core_idx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(core_idx, bus_info_addr, 0x00);
            }

        }
        else {
            VLOG(INFO, "index = %02d", i);

            tmp = read_pinfo_buffer(core_idx,
                                    bus_info_addr);   //TiledEn<<20 ,GdiFormat<<17,IntlvCbCr,<<16 GdiYuvBufStride
            VLOG(INFO, " control = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " pic_size = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " y-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " cb-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " cr-top = 0x%08x", tmp);

            if (productId == VDI_PRODUCT_ID_980) {
                bus_info_addr += 4;
                tmp = read_pinfo_buffer(core_idx, bus_info_addr);
                VLOG(INFO, " y-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(core_idx, bus_info_addr);
                VLOG(INFO, " cb-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(core_idx, bus_info_addr);
                VLOG(INFO, " cr-bot = 0x%08x", tmp);
            }

            VLOG(INFO, "\n");
        }
    }
}




void vdi_print_vpu_status(unsigned long coreIdx)
{
    unsigned int product_code;

    product_code = vdi_read_register(coreIdx, VPU_PRODUCT_CODE_REGISTER);

    if (PRODUCT_CODE_W_SERIES(product_code)) {
        int      rd, wr;
        unsigned int    tq, ip, mc, lf;
        unsigned int    avail_cu, avail_tu, avail_tc, avail_lf, avail_ip;
        unsigned int     ctu_fsm, nb_fsm, cabac_fsm, cu_info, mvp_fsm,
                 tc_busy, lf_fsm, bs_data, bbusy, fv;
        unsigned int    reg_val;
        unsigned int    index;
        unsigned int    vcpu_reg[31] = {0,};

        VLOG(INFO,"-------------------------------------------------------------------------------\n");
        VLOG(INFO,"------                            VCPU STATUS                             -----\n");
        VLOG(INFO,"-------------------------------------------------------------------------------\n");
        rd = VpuReadReg(coreIdx, W4_BS_RD_PTR);
        wr = VpuReadReg(coreIdx, W4_BS_WR_PTR);
        VLOG(INFO,"RD_PTR: 0x%08x WR_PTR: 0x%08x BS_OPT: 0x%08x BS_PARAM: 0x%08x\n",
               rd, wr, VpuReadReg(coreIdx, W4_BS_OPTION), VpuReadReg(coreIdx,
                       W4_BS_PARAM));

        // --------- VCPU register Dump
        VLOG(INFO,"[+] VCPU REG Dump\n");

        for (index = 0; index < 25; index++) {
            VpuWriteReg (coreIdx, 0x14, (1 << 9) | (index & 0xff));
            vcpu_reg[index] = VpuReadReg (coreIdx, 0x1c);

            if (index < 16) {
                VLOG(INFO,"0x%08x\t",  vcpu_reg[index]);

                if ((index % 4) == 3) VLOG(INFO,"\n");
            }
            else {
                switch (index) {
                    case 16:
                        VLOG(INFO,"CR0: 0x%08x\t", vcpu_reg[index]);
                        break;

                    case 17:
                        VLOG(INFO,"CR1: 0x%08x\n", vcpu_reg[index]);
                        break;

                    case 18:
                        VLOG(INFO,"ML:  0x%08x\t", vcpu_reg[index]);
                        break;

                    case 19:
                        VLOG(INFO,"MH:  0x%08x\n", vcpu_reg[index]);
                        break;

                    case 21:
                        VLOG(INFO,"LR:  0x%08x\n", vcpu_reg[index]);
                        break;

                    case 22:
                        VLOG(INFO,"PC:  0x%08x\n", vcpu_reg[index]);
                        break;

                    case 23:
                        VLOG(INFO,"SR:  0x%08x\n", vcpu_reg[index]);
                        break;

                    case 24:
                        VLOG(INFO,"SSP: 0x%08x\n", vcpu_reg[index]);
                        break;
                }
            }
        }

        VLOG(INFO,"[-] VCPU REG Dump\n");
        // --------- BIT register Dump
        VLOG(INFO,"[+] BPU REG Dump\n");
        VLOG(INFO,"BITPC = 0x%08x\n", vdi_fio_read_register(coreIdx,
                (W4_REG_BASE + 0x8000 + 0x18)));
        VLOG(INFO,"BIT START=0x%08x, BIT END=0x%08x\n",
               vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x11c)),
               vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x120)) );

        if (product_code == WAVE410_CODE )
            VLOG(INFO,"BIT COMMAND 0x%x\n", vdi_fio_read_register(coreIdx,
                    (W4_REG_BASE + 0x8000 + 0x100)));

        if (product_code == WAVE4102_CODE || product_code == WAVE510_CODE)
            VLOG(INFO,"BIT COMMAND 0x%x\n", vdi_fio_read_register(coreIdx,
                    (W4_REG_BASE + 0x8000 + 0x1FC)));

        VLOG(INFO,"CODE_BASE			%x \n", vdi_fio_read_register(coreIdx,
                (W4_REG_BASE + 0x7000 + 0x18)));
        VLOG(INFO,"VCORE_REINIT_FLAG	%x \n", vdi_fio_read_register(coreIdx,
                (W4_REG_BASE + 0x7000 + 0x0C)));

        // --------- BIT HEVC Status Dump
        ctu_fsm     = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x48));
        nb_fsm      = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x4c));
        cabac_fsm   = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x50));
        cu_info     = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x54));
        mvp_fsm     = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x58));
        tc_busy     = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x5c));
        lf_fsm      = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x60));
        bs_data     = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x64));
        bbusy       = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x68));
        fv          = vdi_fio_read_register(coreIdx,
                                            (W4_REG_BASE + 0x8000 + 0x6C));


        VLOG(INFO,"[DEBUG-BPUHEVC] CTU_X: %4d, CTU_Y: %4d\n",
               vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x40)),
               vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x44)));
        VLOG(INFO,"[DEBUG-BPUHEVC] CTU_FSM>   Main: 0x%02x, FIFO: 0x%1x, NB: 0x%02x, DBK: 0x%1x\n",
               ((ctu_fsm >> 24) & 0xff), ((ctu_fsm >> 16) & 0xff),
               ((ctu_fsm >> 8) & 0xff), (ctu_fsm & 0xff));
        VLOG(INFO,"[DEBUG-BPUHEVC] NB_FSM:	0x%02x\n", nb_fsm & 0xff);
        VLOG(INFO,"[DEBUG-BPUHEVC] CABAC_FSM> SAO: 0x%02x, CU: 0x%02x, PU: 0x%02x, TU: 0x%02x, EOS: 0x%02x\n",
               ((cabac_fsm >> 25) & 0x3f), ((cabac_fsm >> 19) & 0x3f),
               ((cabac_fsm >> 13) & 0x3f), ((cabac_fsm >> 6) & 0x7f),
               (cabac_fsm & 0x3f));
        VLOG(INFO,"[DEBUG-BPUHEVC] CU_INFO value = 0x%04x \n\t\t(l2cb: 0x%1x, cux: %1d, cuy; %1d, pred: %1d, pcm: %1d, wr_done: %1d, par_done: %1d, nbw_done: %1d, dec_run: %1d)\n",
               cu_info,
               ((cu_info >> 16) & 0x3), ((cu_info >> 13) & 0x7),
               ((cu_info >> 10) & 0x7), ((cu_info >> 9) & 0x3),
               ((cu_info >> 8) & 0x1), ((cu_info >> 6) & 0x3),
               ((cu_info >> 4) & 0x3), ((cu_info >> 2) & 0x3), (cu_info & 0x3));
        VLOG(INFO,"[DEBUG-BPUHEVC] MVP_FSM> 0x%02x\n", mvp_fsm & 0xf);
        VLOG(INFO,"[DEBUG-BPUHEVC] TC_BUSY> tc_dec_busy: %1d, tc_fifo_busy: 0x%02x\n",
               ((tc_busy >> 3) & 0x1), (tc_busy & 0x7));
        VLOG(INFO,"[DEBUG-BPUHEVC] LF_FSM>  SAO: 0x%1x, LF: 0x%1x\n",
               ((lf_fsm >> 4) & 0xf), (lf_fsm  & 0xf));
        VLOG(INFO,"[DEBUG-BPUHEVC] BS_DATA> ExpEnd=%1d, bs_valid: 0x%03x, bs_data: 0x%03x\n",
               ((bs_data >> 31) & 0x1), ((bs_data >> 16) & 0xfff),
               (bs_data & 0xfff));
        VLOG(INFO,"[DEBUG-BPUHEVC] BUS_BUSY> mib_wreq_done: %1d, mib_busy: %1d, sdma_bus: %1d\n",
               ((bbusy >> 2) & 0x1), ((bbusy >> 1) & 0x1), (bbusy & 0x1));
        VLOG(INFO,"[DEBUG-BPUHEVC] FIFO_VALID> cu: %1d, tu: %1d, iptu: %1d, lf: %1d, coff: %1d\n\n",
               ((fv >> 4) & 0x1), ((fv >> 3) & 0x1), ((fv >> 2) & 0x1),
               ((fv >> 1) & 0x1), (fv & 0x1));
        VLOG(INFO,"[-] BPU REG Dump\n");



#if 1
                // --------- VCE register Dump
        VLOG(INFO,"[+] VCE REG Dump now coreidx %d \n", coreIdx);
        tq = read_vce_register(coreIdx, 0, 0xd0);
        ip = read_vce_register(coreIdx, 0, 0xd4);
        mc = read_vce_register(coreIdx, 0, 0xd8);
        lf = read_vce_register(coreIdx, 0, 0xdc);
        avail_cu = (read_vce_register(coreIdx, 0,
                                      0x11C) >> 16) - (read_vce_register(coreIdx, 0, 0x110) >> 16);
        avail_tu = (read_vce_register(coreIdx, 0,
                                      0x11C) & 0xFFFF) - (read_vce_register(coreIdx, 0, 0x110) & 0xFFFF);
        avail_tc = (read_vce_register(coreIdx, 0,
                                      0x120) >> 16) - (read_vce_register(coreIdx, 0, 0x114) >> 16);
        avail_lf = (read_vce_register(coreIdx, 0,
                                      0x120) & 0xFFFF) - (read_vce_register(coreIdx, 0, 0x114) & 0xFFFF);
        avail_ip = (read_vce_register(coreIdx, 0,
                                      0x124) >> 16) - (read_vce_register(coreIdx, 0, 0x118) >> 16);
        VLOG(INFO,"       TQ            IP              MC             LF      GDI_EMPTY          ROOM \n");
        VLOG(INFO,"------------------------------------------------------------------------------------------------------------\n");
        VLOG(INFO,"| %d %04d %04d | %d %04d %04d |  %d %04d %04d | %d %04d %04d | 0x%08x | CU(%d) TU(%d) TC(%d) LF(%d) IP(%d)\n",
               (tq >> 22) & 0x07, (tq >> 11) & 0x3ff, tq & 0x3ff,
               (ip >> 22) & 0x07, (ip >> 11) & 0x3ff, ip & 0x3ff,
               (mc >> 22) & 0x07, (mc >> 11) & 0x3ff, mc & 0x3ff,
               (lf >> 22) & 0x07, (lf >> 11) & 0x3ff, lf & 0x3ff,
               vdi_fio_read_register(0, 0x88f4),                      /* GDI empty */
               avail_cu, avail_tu, avail_tc, avail_lf, avail_ip);
        /* CU/TU Queue count */
        reg_val = read_vce_register(coreIdx, 0, 0x12C);
        VLOG(INFO,"[DCIDEBUG] QUEUE COUNT: CU(%5d) TU(%5d) ",
               (reg_val >> 16) & 0xffff, reg_val & 0xffff);
        reg_val = read_vce_register(coreIdx, 0, 0x1A0);
        VLOG(INFO,"TC(%5d) IP(%5d) ", (reg_val >> 16) & 0xffff,
               reg_val & 0xffff);
        reg_val = read_vce_register(coreIdx, 0, 0x1A4);
        VLOG(INFO,"LF(%5d)\n", (reg_val >> 16) & 0xffff);
        VLOG(INFO,"VALID SIGNAL : CU0(%d)  CU1(%d)  CU2(%d) TU(%d) TC(%d) IP(%5d) LF(%5d)\n"
               "               DCI_FALSE_RUN(%d) VCE_RESET(%d) CORE_INIT(%d) SET_RUN_CTU(%d) \n",
               (reg_val >> 6) & 1, (reg_val >> 5) & 1, (reg_val >> 4) & 1,
               (reg_val >> 3) & 1,
               (reg_val >> 2) & 1, (reg_val >> 1) & 1, (reg_val >> 0) & 1,
               (reg_val >> 10) & 1, (reg_val >> 9) & 1, (reg_val >> 8) & 1,
               (reg_val >> 7) & 1);

        VLOG(INFO,"State TQ: 0x%08x IP: 0x%08x MC: 0x%08x LF: 0x%08x\n",
               read_vce_register(coreIdx, 0, 0xd0), read_vce_register(coreIdx, 0, 0xd4),
               read_vce_register(coreIdx, 0, 0xd8), read_vce_register(coreIdx, 0, 0xdc));
        VLOG(INFO,"BWB[1]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n",
               read_vce_register(coreIdx, 0, 0x194), read_vce_register(coreIdx, 0, 0x198));
        VLOG(INFO,"BWB[2]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n",
               read_vce_register(coreIdx, 0, 0x194), read_vce_register(coreIdx, 0, 0x198));
        VLOG(INFO,"DCI INFO\n");
        VLOG(INFO,"READ_CNT_0 : 0x%08x\n", read_vce_register(coreIdx, 0, 0x110));
        VLOG(INFO,"READ_CNT_1 : 0x%08x\n", read_vce_register(coreIdx, 0, 0x114));
        VLOG(INFO,"READ_CNT_2 : 0x%08x\n", read_vce_register(coreIdx, 0, 0x118));
        VLOG(INFO,"WRITE_CNT_0: 0x%08x\n", read_vce_register(coreIdx, 0, 0x11c));
        VLOG(INFO,"WRITE_CNT_1: 0x%08x\n", read_vce_register(coreIdx, 0, 0x120));
        VLOG(INFO,"WRITE_CNT_2: 0x%08x\n", read_vce_register(coreIdx, 0, 0x124));
        reg_val = read_vce_register(coreIdx, 0, 0x128);
        VLOG(INFO,"LF_DEBUG_PT: 0x%08x\n", reg_val & 0xffffffff);
        VLOG(INFO,"cur_main_state %2d, r_lf_pic_deblock_disable %1d, r_lf_pic_sao_disable %1d\n",
               (reg_val >> 16) & 0x1f,
               (reg_val >> 15) & 0x1,
               (reg_val >> 14) & 0x1);
        VLOG(INFO,"para_load_done %1d, i_rdma_ack_wait %1d, i_sao_intl_col_done %1d, i_sao_outbuf_full %1d\n",
               (reg_val >> 13) & 0x1,
               (reg_val >> 12) & 0x1,
               (reg_val >> 11) & 0x1,
               (reg_val >> 10) & 0x1);
        VLOG(INFO,"lf_sub_done %1d, i_wdma_ack_wait %1d, lf_all_sub_done %1d, cur_ycbcr %1d, sub8x8_done %2d\n",
               (reg_val >> 9) & 0x1,
               (reg_val >> 8) & 0x1,
               (reg_val >> 6) & 0x1,
               (reg_val >> 4) & 0x1,
               reg_val & 0xf);
        VLOG(INFO,"[-] VCE REG Dump\n");
        VLOG(INFO,"[-] VCE REG Dump\n");

                VLOG(INFO, "--------------------> FOR HEVC <--------------------\n");
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000050], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000050));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000024], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000024));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000b0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000b0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000140], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000140));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000c0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000c0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000c4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000c4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000070], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000070));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000074], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000074));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000078], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000078));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000007c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000007c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000080], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000080));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f8], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000f8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000084], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000084));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000fc], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000fc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000088], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000088));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000f4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000f0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000054], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000054));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000058], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000058));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001b0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001b4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b8], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001b8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001bc], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001bc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000090], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000090));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000098], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000098));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000040], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000040));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000060], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000060));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000000c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000000c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000010], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000010));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000094], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000094));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a8], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000a8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000ac], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000ac));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000028], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000028));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000001c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000001c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000020], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000020));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000014], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000014));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000018], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000018));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000a0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000a4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000000], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000000));
                VLOG(INFO, "--------------------> FOR VP9  <--------------------\n");
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000068], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000068));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000064], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000064));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000014c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000014c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000148], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000148));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000154], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000154));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000150], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000150));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000168], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000168));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000164], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000164));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000170], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000170));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000016c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000016c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000000], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000000));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000070], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000070));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000074], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000074));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000078], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000078));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000007c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000007c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000080], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000080));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f8], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000f8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000084], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000084));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000fc], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000fc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000088], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000088));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000f4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000f0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000054], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000054));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000058], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000058));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001b0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001b4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b8], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001b8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001bc], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001bc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000024], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000024));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000b0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000b0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000017c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000017c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000140], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000140));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000c0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000c0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000c4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000c4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000090], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000090));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000098], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000098));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000040], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000040));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000004c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000004c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000060], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000060));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000000c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000000c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000010], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000010));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000094], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000094));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000180], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000180));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000184], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000184));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000188], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000188));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001d0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001d0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001d4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001d4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001d8], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001d8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001dc], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001dc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001e0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001e0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001e4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001e4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001e8], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001e8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001ec], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001ec));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001f0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001f0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001f4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001f4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001f8], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000001f8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000028], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000028));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000001c], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x0000001c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000020], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000020));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000014], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000014));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000018], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000018));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a8], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000a8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000ac], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000ac));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a0], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000a0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a4], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x000000a4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000000], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000000));

        VLOG(INFO,"-------------------------------------------------------------------------------\n");

#else
        // --------- VCE register Dump
        VLOG(INFO,"[+] VCE REG Dump\n");
        tq = read_vce_register(0, 0, 0xd0);
        ip = read_vce_register(0, 0, 0xd4);
        mc = read_vce_register(0, 0, 0xd8);
        lf = read_vce_register(0, 0, 0xdc);
        avail_cu = (read_vce_register(0, 0,
                                      0x11C) >> 16) - (read_vce_register(0, 0, 0x110) >> 16);
        avail_tu = (read_vce_register(0, 0,
                                      0x11C) & 0xFFFF) - (read_vce_register(0, 0, 0x110) & 0xFFFF);
        avail_tc = (read_vce_register(0, 0,
                                      0x120) >> 16) - (read_vce_register(0, 0, 0x114) >> 16);
        avail_lf = (read_vce_register(0, 0,
                                      0x120) & 0xFFFF) - (read_vce_register(0, 0, 0x114) & 0xFFFF);
        avail_ip = (read_vce_register(0, 0,
                                      0x124) >> 16) - (read_vce_register(0, 0, 0x118) >> 16);
        VLOG(INFO,"       TQ            IP              MC             LF      GDI_EMPTY          ROOM \n");
        VLOG(INFO,"------------------------------------------------------------------------------------------------------------\n");
        VLOG(INFO,"| %d %04d %04d | %d %04d %04d |  %d %04d %04d | %d %04d %04d | 0x%08x | CU(%d) TU(%d) TC(%d) LF(%d) IP(%d)\n",
               (tq >> 22) & 0x07, (tq >> 11) & 0x3ff, tq & 0x3ff,
               (ip >> 22) & 0x07, (ip >> 11) & 0x3ff, ip & 0x3ff,
               (mc >> 22) & 0x07, (mc >> 11) & 0x3ff, mc & 0x3ff,
               (lf >> 22) & 0x07, (lf >> 11) & 0x3ff, lf & 0x3ff,
               vdi_fio_read_register(0, 0x88f4),                      /* GDI empty */
               avail_cu, avail_tu, avail_tc, avail_lf, avail_ip);
        /* CU/TU Queue count */
        reg_val = read_vce_register(0, 0, 0x12C);
        VLOG(INFO,"[DCIDEBUG] QUEUE COUNT: CU(%5d) TU(%5d) ",
               (reg_val >> 16) & 0xffff, reg_val & 0xffff);
        reg_val = read_vce_register(0, 0, 0x1A0);
        VLOG(INFO,"TC(%5d) IP(%5d) ", (reg_val >> 16) & 0xffff,
               reg_val & 0xffff);
        reg_val = read_vce_register(0, 0, 0x1A4);
        VLOG(INFO,"LF(%5d)\n", (reg_val >> 16) & 0xffff);
        VLOG(INFO,"VALID SIGNAL : CU0(%d)  CU1(%d)  CU2(%d) TU(%d) TC(%d) IP(%5d) LF(%5d)\n"
               "               DCI_FALSE_RUN(%d) VCE_RESET(%d) CORE_INIT(%d) SET_RUN_CTU(%d) \n",
               (reg_val >> 6) & 1, (reg_val >> 5) & 1, (reg_val >> 4) & 1,
               (reg_val >> 3) & 1,
               (reg_val >> 2) & 1, (reg_val >> 1) & 1, (reg_val >> 0) & 1,
               (reg_val >> 10) & 1, (reg_val >> 9) & 1, (reg_val >> 8) & 1,
               (reg_val >> 7) & 1);

        VLOG(INFO,"State TQ: 0x%08x IP: 0x%08x MC: 0x%08x LF: 0x%08x\n",
               read_vce_register(0, 0, 0xd0), read_vce_register(0, 0, 0xd4),
               read_vce_register(0, 0, 0xd8), read_vce_register(0, 0, 0xdc));
        VLOG(INFO,"BWB[1]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n",
               read_vce_register(0, 0, 0x194), read_vce_register(0, 0, 0x198));
        VLOG(INFO,"BWB[2]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n",
               read_vce_register(0, 0, 0x194), read_vce_register(0, 0, 0x198));
        VLOG(INFO,"DCI INFO\n");
        VLOG(INFO,"READ_CNT_0 : 0x%08x\n", read_vce_register(0, 0, 0x110));
        VLOG(INFO,"READ_CNT_1 : 0x%08x\n", read_vce_register(0, 0, 0x114));
        VLOG(INFO,"READ_CNT_2 : 0x%08x\n", read_vce_register(0, 0, 0x118));
        VLOG(INFO,"WRITE_CNT_0: 0x%08x\n", read_vce_register(0, 0, 0x11c));
        VLOG(INFO,"WRITE_CNT_1: 0x%08x\n", read_vce_register(0, 0, 0x120));
        VLOG(INFO,"WRITE_CNT_2: 0x%08x\n", read_vce_register(0, 0, 0x124));
        reg_val = read_vce_register(0, 0, 0x128);
        VLOG(INFO,"LF_DEBUG_PT: 0x%08x\n", reg_val & 0xffffffff);
        VLOG(INFO,"cur_main_state %2d, r_lf_pic_deblock_disable %1d, r_lf_pic_sao_disable %1d\n",
               (reg_val >> 16) & 0x1f,
               (reg_val >> 15) & 0x1,
               (reg_val >> 14) & 0x1);
        VLOG(INFO,"para_load_done %1d, i_rdma_ack_wait %1d, i_sao_intl_col_done %1d, i_sao_outbuf_full %1d\n",
               (reg_val >> 13) & 0x1,
               (reg_val >> 12) & 0x1,
               (reg_val >> 11) & 0x1,
               (reg_val >> 10) & 0x1);
        VLOG(INFO,"lf_sub_done %1d, i_wdma_ack_wait %1d, lf_all_sub_done %1d, cur_ycbcr %1d, sub8x8_done %2d\n",
               (reg_val >> 9) & 0x1,
               (reg_val >> 8) & 0x1,
               (reg_val >> 6) & 0x1,
               (reg_val >> 4) & 0x1,
               reg_val & 0xf);
        VLOG(INFO,"[-] VCE REG Dump\n");
        VLOG(INFO,"[-] VCE REG Dump\n");
        /*
                VLOG(INFO, "--------------------> FOR HEVC <--------------------\n");
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000050], IPB WDATA[%08x]\n", ReadRegVCE(coreIdx, 0, 0x00000050));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000024], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000024));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000b0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000b0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000140], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000140));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000c0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000c0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000c4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000c4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000070], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000070));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000074], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000074));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000078], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000078));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000007c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000007c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000080], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000080));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f8], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000f8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000084], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000084));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000fc], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000fc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000088], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000088));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000f4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000f0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000054], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000054));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000058], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000058));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001b0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001b4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b8], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001b8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001bc], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001bc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000090], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000090));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000098], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000098));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000040], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000040));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000060], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000060));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000000c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000000c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000010], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000010));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000094], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000094));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a8], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000a8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000ac], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000ac));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000028], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000028));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000001c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000001c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000020], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000020));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000014], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000014));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000018], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000018));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000a0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000a4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000000], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000000));
                VLOG(INFO, "--------------------> FOR VP9  <--------------------\n");
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000068], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000068));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000064], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000064));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000014c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000014c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000148], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000148));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000154], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000154));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000150], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000150));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000168], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000168));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000164], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000164));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000170], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000170));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000016c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000016c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000000], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000000));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000070], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000070));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000074], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000074));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000078], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000078));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000007c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000007c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000080], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000080));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f8], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000f8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000084], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000084));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000fc], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000fc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000088], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000088));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000f4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000f0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000f0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000054], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000054));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000058], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000058));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001b0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001b4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001b8], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001b8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001bc], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001bc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000024], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000024));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000b0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000b0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000017c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000017c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000140], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000140));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000c0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000c0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000c4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000c4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000090], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000090));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000098], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000098));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000040], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000040));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000004c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000004c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000060], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000060));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000000c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000000c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000010], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000010));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000094], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000094));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000180], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000180));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000184], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000184));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000188], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000188));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001d0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001d0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001d4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001d4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001d8], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001d8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001dc], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001dc));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001e0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001e0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001e4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001e4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001e8], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001e8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001ec], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001ec));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001f0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001f0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001f4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001f4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000001f8], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000001f8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000028], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000028));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[0000001c], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x0000001c));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000020], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000020));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000014], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000014));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000018], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000018));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a8], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000a8));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000ac], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000ac));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a0], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000a0));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[000000a4], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x000000a4));
                VLOG(INFO, "[VCE_ACCESS[W] : IPB ADDR[00000000], IPB WDATA[%08x]\n", ReadRegVCE(0, 0, 0x00000000));

        */
        VLOG(INFO,"-------------------------------------------------------------------------------\n");
#endif
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(product_code)) {
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", product_code);
    }
}

void vdi_make_log(unsigned long core_idx, const char *str, int step)
{
    int val;

    val = VpuReadReg(core_idx, W4_INST_INDEX);
    val &= 0xffff;

    if (step == 1)
        VLOG(INFO, "\n**%s start(%d)\n", str, val);
    else if (step == 2) //
        VLOG(INFO, "\n**%s timeout(%d)\n", str, val);
    else
        VLOG(INFO, "\n**%s end(%d)\n", str, val);
}

void vdi_log(unsigned long core_idx, int cmd, int step)
{
    vdi_info_t *vdi;
    int i;

    // BIT_RUN command
    enum {
        SEQ_INIT = 1,
        SEQ_END = 2,
        PIC_RUN = 3,
        SET_FRAME_BUF = 4,
        ENCODE_HEADER = 5,
        ENC_PARA_SET = 6,
        DEC_PARA_SET = 7,
        DEC_BUF_FLUSH = 8,
        RC_CHANGE_PARAMETER = 9,
        VPU_SLEEP = 10,
        VPU_WAKE = 11,
        ENC_ROI_INIT = 12,
        FIRMWARE_GET = 0xf,
        VPU_RESET = 0x10,
    };

    if (core_idx >= MAX_NUM_VPU_CORE)
        return ;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0)
        return ;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        switch (cmd) {
            case INIT_VPU:
                vdi_make_log(core_idx, "INIT_VPU", step);
                break;

            case DEC_PIC_HDR: //SET_PARAM for ENC
                vdi_make_log(core_idx, "SET_PARAM(ENC), DEC_PIC_HDR(DEC)", step);
                break;

            case FINI_SEQ:
                vdi_make_log(core_idx, "FINI_SEQ", step);
                break;

            case DEC_PIC://ENC_PIC for ENC
                vdi_make_log(core_idx, "DEC_PIC, ENC_PIC", step);
                break;

            case SET_FRAMEBUF:
                vdi_make_log(core_idx, "SET_FRAMEBUF", step);
                break;

            case FLUSH_DECODER:
                vdi_make_log(core_idx, "FLUSH_DECODER", step);
                break;

            case GET_FW_VERSION:
                vdi_make_log(core_idx, "GET_FW_VERSION", step);
                break;

            case QUERY_DECODER:
                vdi_make_log(core_idx, "QUERY_DECODER", step);
                break;

            case SLEEP_VPU:
                vdi_make_log(core_idx, "SLEEP_VPU", step);
                break;

            case CREATE_INSTANCE:
                vdi_make_log(core_idx, "CREATE_INSTANCE", step);
                break;

            case RESET_VPU:
                vdi_make_log(core_idx, "RESET_VPU", step);
                break;

            default:
                vdi_make_log(core_idx, "ANY_CMD", step);
                break;
        }
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        switch (cmd) {
            case SEQ_INIT:
                vdi_make_log(core_idx, "SEQ_INIT", step);
                break;

            case SEQ_END:
                vdi_make_log(core_idx, "SEQ_END", step);
                break;

            case PIC_RUN:
                vdi_make_log(core_idx, "PIC_RUN", step);
                break;

            case SET_FRAME_BUF:
                vdi_make_log(core_idx, "SET_FRAME_BUF", step);
                break;

            case ENCODE_HEADER:
                vdi_make_log(core_idx, "ENCODE_HEADER", step);
                break;

            case RC_CHANGE_PARAMETER:
                vdi_make_log(core_idx, "RC_CHANGE_PARAMETER", step);
                break;

            case DEC_BUF_FLUSH:
                vdi_make_log(core_idx, "DEC_BUF_FLUSH", step);
                break;

            case FIRMWARE_GET:
                vdi_make_log(core_idx, "FIRMWARE_GET", step);
                break;

            case VPU_RESET:
                vdi_make_log(core_idx, "VPU_RESET", step);
                break;

            case ENC_PARA_SET:
                vdi_make_log(core_idx, "ENC_PARA_SET", step);
                break;

            case DEC_PARA_SET:
                vdi_make_log(core_idx, "DEC_PARA_SET", step);
                break;

            default:
                vdi_make_log(core_idx, "ANY_CMD", step);
                break;
        }
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return;
    }

    for (i = 0; i < 0x200; i = i + 16) {
        VLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
             vdi_read_register(core_idx, i), vdi_read_register(core_idx, i + 4),
             vdi_read_register(core_idx, i + 8), vdi_read_register(core_idx,
                     i + 0xc));
    }


    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) { // WAVE4xx
        if (cmd == INIT_VPU || cmd == VPU_RESET || cmd == CREATE_INSTANCE) {
            vdi_print_vpu_status(core_idx);
        }

    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        //if ((cmd == PIC_RUN && step== 0) || cmd == VPU_RESET)
        if (cmd == VPU_RESET) {
            printf_gdi_info(core_idx, 32, 0);

#define VDI_LOG_MBC_BUSY 0x0440
#define VDI_LOG_MC_BASE  0x0C00
#define VDI_LOG_MC_BUSY  0x0C04
#define VDI_LOG_GDI_BUS_STATUS (0x10F4)
#define VDI_LOG_ROT_SRC_IDX  (0x400 + 0x10C)
#define VDI_LOG_ROT_DST_IDX  (0x400 + 0x110)

            VLOG(INFO, "MBC_BUSY = %x\n", vdi_read_register(core_idx,
                    VDI_LOG_MBC_BUSY));
            VLOG(INFO, "MC_BUSY = %x\n", vdi_read_register(core_idx,
                    VDI_LOG_MC_BUSY));
            VLOG(INFO, "MC_MB_XY_DONE=(y:%d, x:%d)\n",
                 (vdi_read_register(core_idx, VDI_LOG_MC_BASE) >> 20) & 0x3F,
                 (vdi_read_register(core_idx, VDI_LOG_MC_BASE) >> 26) & 0x3F);
            VLOG(INFO, "GDI_BUS_STATUS = %x\n", vdi_read_register(core_idx,
                    VDI_LOG_GDI_BUS_STATUS));

            VLOG(INFO, "ROT_SRC_IDX = %x\n", vdi_read_register(core_idx,
                    VDI_LOG_ROT_SRC_IDX));
            VLOG(INFO, "ROT_DST_IDX = %x\n", vdi_read_register(core_idx,
                    VDI_LOG_ROT_DST_IDX));

            VLOG(INFO, "P_MC_PIC_INDEX_0 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x200));
            VLOG(INFO, "P_MC_PIC_INDEX_1 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x20c));
            VLOG(INFO, "P_MC_PIC_INDEX_2 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x218));
            VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x230));
            VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x23C));
            VLOG(INFO, "P_MC_PIC_INDEX_4 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x248));
            VLOG(INFO, "P_MC_PIC_INDEX_5 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x254));
            VLOG(INFO, "P_MC_PIC_INDEX_6 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x260));
            VLOG(INFO, "P_MC_PIC_INDEX_7 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x26C));
            VLOG(INFO, "P_MC_PIC_INDEX_8 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x278));
            VLOG(INFO, "P_MC_PIC_INDEX_9 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x284));
            VLOG(INFO, "P_MC_PIC_INDEX_a = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x290));
            VLOG(INFO, "P_MC_PIC_INDEX_b = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x29C));
            VLOG(INFO, "P_MC_PIC_INDEX_c = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x2A8));
            VLOG(INFO, "P_MC_PIC_INDEX_d = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x2B4));

            VLOG(INFO, "P_MC_PICIDX_0 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x028));
            VLOG(INFO, "P_MC_PICIDX_1 = %x\n", vdi_read_register(core_idx,
                    MC_BASE + 0x02C));
        }
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return;
    }
}

static void byte_swap(unsigned char *data, int len)
{
    register Uint8 temp;
    Int32 i;

    for (i = 0; i < len; i += 2) {
        if(i+1 < len) {
            temp      = data[i];
            data[i]   = data[i + 1];
            data[i + 1] = temp;
        }
        else{
            VLOG(ERR,"Error %s : %d : len = %d i = %d\n", __FUNCTION__, __LINE__, len,i);
        }
    }
}

static void word_swap(unsigned char *data, int len)
{
    register Uint16  temp;
    Uint16 *ptr = (Uint16 *)data;
    Int32   i, size = len / sizeof(Uint16);

    for (i = 0; i < size; i += 2) {
        if(i+1 < size) {
            temp = ptr[i];
            ptr[i] = ptr[i + 1];
            ptr[i + 1] = temp;
        }
        else{
            VLOG(ERR,"Error %s : %d : len = %d , size =%d i = %d\n", __FUNCTION__, __LINE__, len,size,i);
        }
    }
}

static void dword_swap(unsigned char *data, int len)
{
    register Uint32  temp;
    Uint32 *ptr = (Uint32 *)data;
    Int32   i, size = len / sizeof(Uint32);

    for (i = 0; i < size; i += 2) {
        if(i+1 < size){
            temp      = ptr[i];
            ptr[i]   = ptr[i + 1];
            ptr[i + 1] = temp;
        }
        else{
            VLOG(ERR, "Error %s : %d : len = %d , size = %d i = %d\n", __FUNCTION__, __LINE__, len,size,i);
        }
    }
}

static void lword_swap(unsigned char *data, int len)
{
    register Uint64  temp;
    Uint64 *ptr = (Uint64 *)data;
    Int32   i, size = len / sizeof(Uint64);

    for (i = 0; i < size; i += 2) {
        if(i+1 < size) {
            temp      = ptr[i];
            ptr[i]   = ptr[i + 1];
            ptr[i + 1] = temp;
        }
        else{
            VLOG(ERR,"Error  %s : %d : len = %d , size =%d i = %d\n", __FUNCTION__, __LINE__, len,size,i);
        }
    }
}

int vdi_get_system_endian(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        return VDI_128BIT_BUS_SYSTEM_ENDIAN;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        return VDI_SYSTEM_ENDIAN;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }
}

int vdi_convert_endian(unsigned long core_idx, unsigned int endian)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || !vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        switch (endian) {
            case VDI_LITTLE_ENDIAN:
                endian = 0x00;
                break;

            case VDI_BIG_ENDIAN:
                endian = 0x0f;
                break;

            case VDI_32BIT_LITTLE_ENDIAN:
                endian = 0x04;
                break;

            case VDI_32BIT_BIG_ENDIAN:
                endian = 0x03;
                break;
        }
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    return (endian & 0x0f);
}

static Uint32 convert_endian_coda9_to_wave4(Uint32 endian)
{
    Uint32 converted_endian = endian;

    switch (endian) {
        case VDI_LITTLE_ENDIAN:
            converted_endian = 0;
            break;

        case VDI_BIG_ENDIAN:
            converted_endian = 7;
            break;

        case VDI_32BIT_LITTLE_ENDIAN:
            converted_endian = 4;
            break;

        case VDI_32BIT_BIG_ENDIAN:
            converted_endian = 3;
            break;
    }

    return converted_endian;
}


int swap_endian(unsigned long core_idx, unsigned char *data, int len,
                int endian)
{
    vdi_info_t *vdi;
    int changes;
    int sys_endian;
    BOOL byteChange, wordChange, dwordChange, lwordChange;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

//   VLOG(ERR, "ctm>>> %s:%d: product code (0x%x)\n", __FUNCTION__, __LINE__, vdi->product_code);
    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        sys_endian = VDI_128BIT_BUS_SYSTEM_ENDIAN;
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        sys_endian = VDI_SYSTEM_ENDIAN;
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    endian     = vdi_convert_endian(core_idx, endian);
    sys_endian = vdi_convert_endian(core_idx, sys_endian);

    if (endian == sys_endian)
        return 0;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
    }
    else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        endian     = convert_endian_coda9_to_wave4(endian);
        sys_endian = convert_endian_coda9_to_wave4(sys_endian);
    }
    else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    changes     = endian ^ sys_endian;
    byteChange  = changes & 0x01;
    wordChange  = ((changes & 0x02) == 0x02);
    dwordChange = ((changes & 0x04) == 0x04);
    lwordChange = ((changes & 0x08) == 0x08);

    if (byteChange)  byte_swap(data, len);

    if (wordChange)  word_swap(data, len);

    if (dwordChange) dword_swap(data, len);

    if (lwordChange) lword_swap(data, len);

    return 1;
}




#endif  //#if defined(linux) || defined(__linux) || defined(ANDROID)
