//*****************************************************************************
//
// dma_hal.h - Prototypes for the dma hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __DMA_HAL_H__
#define __DMA_HAL_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
#include "__regs_base.h"
#if ENABLE_SD_DMA
//#include "dw_dma.h"
#endif

#include "chip_res.h"
#include "system_cfg.h"
#include <kernel/mutex.h>
#include <platform/interrupts.h>

#define SDV_DMA_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */
#define MAKE_VERSION(major, minor, bugfix)                                     \
    (((major) << 16) | ((minor) << 8) | (bugfix))
#define DEFAULT_DMA_MAX_NUM 8

typedef struct {
    uint32_t res_glb_idx;
    char res_describe[100];
    paddr_t phy_addr;
    paddr_t mux_phy_addr;
    uint32_t addr_range;
    uint32_t irq_numb;
} dmac_info_t;

/**
 * enum dma_status - DMA transaction status
 */
enum dma_status {
    DMA_COMP = 0,    /* transaction completed */
    DMA_IN_PROGRESS, /* transaction not yet processed */
    DMA_PAUSED,      /* transaction is paused */
    DMA_ERR,         /* transaction failed */
    DMA_PENDING,     /* special requirement, in a time */
};
/**
 * enum dma_transaction_type - DMA transaction status
 */
enum dma_transaction_type {
    DMA_MEMCPY = 0, /* The device is able to do memory to memory copies */
    DMA_MEMSET = 1, /* Async type set memory to 0 or a char value. */
    DMA_SLAVE = 2,  /* The device can handle device to memory transfers. */
    DMA_CYCLIC = 3, /* A cyclic transfer is a transfer where the chunk
                 collection will loop over itself, with the last item pointing
                 to the first. For audio ring buffer. */
    /* last transaction type for creation of the capabilities mask */
    DMA_TYPE_END,
};

/**
 * enum dma_transfer_direction - dma transfer mode and direction indicator
 * @DMA_MEM_TO_MEM: Async/Memcpy mode
 * @DMA_MEM_TO_DEV: Slave mode & From Memory to Device
 * @DMA_DEV_TO_MEM: Slave mode & From Device to Memory
 * @DMA_DEV_TO_DEV: Slave mode & From Device to Device
 */
enum dma_tr_direction {
    DMA_MEM2MEM = 0,
    DMA_MEM2DEV = 1,
    DMA_DEV2MEM = 2,
    DMA_DEV2DEV = 3,
    DMA_TRANS_NONE,
    /* Don't support next four transfer types. */
};
enum dma_chan_tr_type {
    DMA_MEM,
    /* dmac 1 peripheral transfer capabilities. */

    DMA_PERI_CAN1,
    DMA_PERI_CAN2,
    DMA_PERI_CAN3,
    DMA_PERI_CAN4,
    DMA_PERI_ENET1,
    DMA_PERI_I2C1,
    DMA_PERI_I2C2,
    DMA_PERI_I2C3,
    DMA_PERI_I2C4,
    DMA_PERI_I2S_SC1,
    DMA_PERI_I2S_SC2,
    DMA_PERI_OSPI1,
    DMA_PERI_PWM1,
    DMA_PERI_PWM2,
    DMA_PERI_SPI1,
    DMA_PERI_SPI2,
    DMA_PERI_SPI3,
    DMA_PERI_SPI4,
    DMA_PERI_TIMER1,
    DMA_PERI_TIMER2,
    DMA_PERI_UART1,
    DMA_PERI_UART2,
    DMA_PERI_UART3,
    DMA_PERI_UART4,
    DMA_PERI_UART5,
    DMA_PERI_UART6,
    DMA_PERI_UART7,
    DMA_PERI_UART8,
    /* dmac 2~8 peripheral transfer capabilities. */

    DMA_PERI_CAN5,
    DMA_PERI_CAN6,
    DMA_PERI_CAN7,
    DMA_PERI_CAN8,
    DMA_PERI_DC1,
    DMA_PERI_DC2,
    DMA_PERI_DC3,
    DMA_PERI_DC4,
    DMA_PERI_DC5,
    DMA_PERI_DDR_SS,
    DMA_PERI_DP1,
    DMA_PERI_DP2,
    DMA_PERI_DP3,
    DMA_PERI_ENET2,
    DMA_PERI_I2C5,
    DMA_PERI_I2C6,
    DMA_PERI_I2C7,
    DMA_PERI_I2C8,
    DMA_PERI_I2C9,
    DMA_PERI_I2C10,
    DMA_PERI_I2C11,
    DMA_PERI_I2C12,
    DMA_PERI_I2C13,
    DMA_PERI_I2C14,
    DMA_PERI_I2C15,
    DMA_PERI_I2C16,
    DMA_PERI_I2S_MC1,
    DMA_PERI_I2S_MC2,
    DMA_PERI_I2S_SC3,
    DMA_PERI_I2S_SC4,
    DMA_PERI_I2S_SC5,
    DMA_PERI_I2S_SC6,
    DMA_PERI_I2S_SC7,
    DMA_PERI_I2S_SC8,
    DMA_PERI_OSPI2,
    DMA_PERI_PWM3,
    DMA_PERI_PWM4,
    DMA_PERI_PWM5,
    DMA_PERI_PWM6,
    DMA_PERI_PWM7,
    DMA_PERI_PWM8,
    DMA_PERI_SPDIF1,
    DMA_PERI_SPDIF2,
    DMA_PERI_SPDIF3,
    DMA_PERI_SPDIF4,
    DMA_PERI_SPI5,
    DMA_PERI_SPI6,
    DMA_PERI_SPI7,
    DMA_PERI_SPI8,
    DMA_PERI_TIMER3,
    DMA_PERI_TIMER4,
    DMA_PERI_TIMER5,
    DMA_PERI_TIMER6,
    DMA_PERI_TIMER7,
    DMA_PERI_TIMER8,
    DMA_PERI_UART9,
    DMA_PERI_UART10,
    DMA_PERI_UART11,
    DMA_PERI_UART12,
    DMA_PERI_UART13,
    DMA_PERI_UART14,
    DMA_PERI_UART15,
    DMA_PERI_UART16,
    DMA_PERI_ADC,
    DMA_PERI_CAN9,
    DMA_PERI_CAN10,
    DMA_PERI_CAN11,
    DMA_PERI_CAN12,
    DMA_PERI_CAN13,
    DMA_PERI_CAN14,
    DMA_PERI_CAN15,
    DMA_PERI_CAN16,
    DMA_PERI_CAN17,
    DMA_PERI_CAN18,
    DMA_PERI_CAN19,
    DMA_PERI_CAN20,
    DMA_PERI_NUMB,
};
/**
 * enum dma_ctrl_flags - DMA flags to augment operation preparation,
 *  control completion, and communicate status.
 * @DMA_INTERRUPT - trigger an interrupt (callback) upon completion of
 *  this transaction
 * @DMA_PENDING_TIMEOUT - if set, the transaction will wait until timeout.

*/
enum dma_ctrl_flags {
    DMA_INTERRUPT = (1 << 0),
    DMA_PENDING_TIMEOUT = (1 << 1),

};

typedef enum {
    DMA_DEV_BUSWIDTH_UNDEFINED = 0,
    DMA_DEV_BUSWIDTH_1_BYTE = 1,
    DMA_DEV_BUSWIDTH_2_BYTES = 2,
    /*  DMA_DEV_BUSWIDTH_3_BYTES = 3, DW DMA don't support this bus width. */
    DMA_DEV_BUSWIDTH_4_BYTES = 4,
    DMA_DEV_BUSWIDTH_8_BYTES = 8,
    DMA_DEV_BUSWIDTH_16_BYTES = 16,
    DMA_DEV_BUSWIDTH_32_BYTES = 32,
    DMA_DEV_BUSWIDTH_64_BYTES = 64,
} DMA_DEV_BUS_WIDTH;

typedef enum {
    DMA_BURST_TR_1ITEM = 0,
    DMA_BURST_TR_4ITEMS = 1,
    DMA_BURST_TR_8ITEMS = 2,
    DMA_BURST_TR_16ITEMS = 3,
    DMA_BURST_TR_32ITEMS = 4,
    DMA_BURST_TR_64ITEMS = 5,
    DMA_BURST_TR_128ITEMS = 6,
    DMA_BURST_TR_256ITEMS = 7,
    DMA_BURST_TR_512ITEMS = 8,
    DMA_BURST_TR_1024ITEMS = 9,
} DMA_BURST_TR_LEN;
typedef struct dma_lli {
    u64 sar; /* source address */
    u64 dsr; /* dest address */
    u32 block_size;
    u32 reserved1;
    u64 llp; /* linked_list_pointer */
    u64 ctl;
    u32 sstat_write_back;
    u32 dstat_write_back;
    u64 llp_status;
    u64 reserved2;
} dma_lli_t __attribute__((__aligned__(64)));

typedef struct lli_list {
    dma_lli_t lli_item;
    u64 phy_addr;
    struct list_node lli_node; /* lli_node */
    struct list_node dw_list;  /* lli_node header */
    u32 id;
} lli_list_t __attribute__((__aligned__(64)));

/**
 * struct dma_desc - a dma transfer descriptor .
 */
typedef struct dma_desc {

    struct list_node node;
    struct list_node desc_node;

    enum dma_ctrl_flags flags;
    addr_t phys;
    struct dma_chan *chan;
    void (*dmac_irq_evt_handle)(
        enum dma_status status, u32 param,
        void *context);   // dma transfer call back. static void
                          // dmac_irq_evt_handle(status)
    void *callback_param; // dma transfer call back param such as error type.
    void *context;
    struct lli_list *lli_list_ptr;

    /*  A parameter for pending timeout. If timeout without new input char, it
 will trigger a call back. (dmac_irq_evt_handle) with range from
 200(ms)~2000(ms), if not set , the default is 500 ms.
 */
    int pending_timeout;

} dma_desc_t;

/**
 * struct dma_dev_cfg - dma device configuration
 * @direction: enum dma_tr_direction use to detail dma transfer direction.
 * @src_addr: device source physical address
 * @dst_addr: device destination physical address
 * @src_addr_width:  enum DMA_DEV_BUS_WIDTH
 * @dst_addr_width: enum DMA_DEV_BUS_WIDTH
 * @src_maxburst: enum DMA_BURST_TR_LEN  src max burst items.
 * @dst_maxburst: enum DMA_BURST_TR_LEN  dst max burst items.
 */
struct dma_dev_cfg {
    enum dma_tr_direction direction;
    paddr_t src_addr;
    paddr_t dst_addr;
    DMA_DEV_BUS_WIDTH src_addr_width;
    DMA_DEV_BUS_WIDTH dst_addr_width;
    DMA_BURST_TR_LEN src_maxburst;
    DMA_BURST_TR_LEN dst_maxburst;
};

/**
 * struct dma_chan - a virtual dma channel
 * @inst_id: dma instance device id for dma hal instance
 * @chan_id: dma virtual channel id (1~8*dmac number)
 * @chan_status: enum dma_status current channel status.
 */

typedef struct dma_chan {
    spin_lock_t lock; // one dw_dma_chan has one dma_chan
    struct list_node node;
    u32 inst_id;
    u32 chan_id;
    u32 vchan_id;
    u32 dmac_id;
    /*     DMA capabilities */
    u32 chan_cap;
    enum dma_status chan_status;
} dma_chan_t;

/**
 *  A dma init function for dma controller hardware, here regiter all of dma
 * controller.
 */
void hal_dma_init(void);

/**
 *  A dma deinit function for dma controller hardware, here remove all of dma
 * controller.
 */
void hal_dma_deinit(void);

/**
 *  Allocate a DMA slave channel with a transfer capability
 * return a available dma_chan
 */
struct dma_chan *hal_dma_chan_req(enum dma_chan_tr_type ch_type);

/**
 *  Allocate a DMA slave channel with id number
 * return a available dma_chan
 * It's a debug function for testing
 */
struct dma_chan *hal_dma_chan_req_with_ch(u32 id);

/*
dma_dev_config: Set device and controller specific parameters
@dma_chan the channel of dma_chan_req result
@dma_dev_cfg  dma device configuration
//return 0 is success.
*/
void hal_dma_dev_config(struct dma_chan *chan, struct dma_dev_cfg *cfg);

/*
prep_dma_memcpy: Get a descriptor for memory to memory transaction
@dest the destination of memory copy
@src  the source of memory copy
@len  the length of memory copy
@flags Control flags
return a dma_desc
*/
struct dma_desc *hal_prep_dma_memcpy(struct dma_chan *chan, void *dest,
                                     void *src, size_t len,
                                     unsigned long flags);

/*
hal_prep_dma_memset: Get a descriptor for memset function.
@val the value of memset unsigned char
@buf_addr  the target of memory set
@count  the number of memory set
@flags Control flags
return a dma_desc
*/
struct dma_desc *hal_prep_dma_memset(struct dma_chan *chan, u8 val,
                                     void *buf_addr, size_t count,
                                     unsigned long flags);

/*
prep_dma_dev: Get a descriptor for memory to peripheral transaction
@buf_addr the destination of memory
@buf_len  the length of transfer
@flags Control flags
return a dma_desc
*/
struct dma_desc *hal_prep_dma_dev(struct dma_chan *chan, void *buf_addr,
                                  size_t buf_len, unsigned long flags);

/*
prep_dma_cyclic: Get a descriptor for memory to peripheral transaction in
cyclic mode
@buf_addr the destination of memory
@buf_len  the length of transfer
@period_len the length of period, buf_len must be a multiple of period_len.
@flags Control flags
return a dma_desc
*/
struct dma_desc *hal_prep_dma_cyclic(struct dma_chan *chan, void *buf_addr,
                                     size_t buf_len, size_t period_len,
                                     unsigned long flags);

/*
dma_submit: Submit a transfer
@desc the descriptor of a dma transfer
*/
void hal_dma_submit(struct dma_desc *desc);

/*
dma_sync_wait: Wait for dma transfer result.
@desc the descriptor of a dma transfer
@timeout ms if timeout this function will return a error status.
*/
enum dma_status hal_dma_sync_wait(struct dma_desc *desc, int timeout);

/*
dma_terminate: terminate a dma channel.
@chan dma transfer channel.
*/
bool hal_dma_terminate(struct dma_desc *desc);

/*
dma_free_desc: free a dma descriptor.
Must free desc when you don't use it.
@desc dma transfer descriptor.
*/
void hal_dma_free_desc(struct dma_desc *desc);

// internel interrupt handle function Don't use this function.
// enum handler_return dma_interrupt_handle(void *arg);

/*    Next is debug function.---------------- */
void hal_dma_dbg_get_chan_status(void);

/* get current channel status */
enum dma_status hal_dma_get_chan_status(struct dma_chan *chan);

/*wdg driver interface structure */
typedef struct _dma_drv_controller_interface {
    unsigned int chan_cnt;
    struct list_node chan_list; /* List head */
    bool (*dma_chan_req)(struct dma_chan *chan);
    void (*dma_dev_config)(struct dma_chan *chan, struct dma_dev_cfg *cfg);
    struct dma_desc *(*prep_dma_memcpy)(struct dma_chan *chan, void *dest,
                                        void *src, size_t len,
                                        unsigned long flags);
    struct dma_desc *(*prep_dma_memset)(struct dma_chan *chan, u8 val,
                                        void *buf_addr, size_t count,
                                        unsigned long flags);
    struct dma_desc *(*prep_dma_dev)(struct dma_chan *chan, void *buf_addr,
                                     size_t buf_len, unsigned long flags);
    struct dma_desc *(*prep_dma_cyclic)(struct dma_chan *chan, void *buf_addr,
                                        size_t buf_len, size_t period_len,
                                        unsigned long flags);
    void (*dma_submit)(struct dma_desc *desc);
    enum dma_status (*dma_sync_wait)(struct dma_desc *desc, int timeout);
    bool (*dma_terminate)(struct dma_desc *desc);
    void (*dma_free_desc)(struct dma_desc *desc);

} dma_drv_controller_interface_t;

/* internal structure */
typedef struct _dma_instance {

    /*   dma_config_t dma_cfg;  dma config */
    dmac_info_t dma_cfg;

    dma_drv_controller_interface_t controllerTable; /*!< dma driver interface*/
    uint8_t occupied; /*!< 0 - the instance is not occupied; 1 - the instance is
                   occupied*/
    // dma_res_config_t dma_res;
    char dma_magic[20];
    bool dma_inited;
    bool dma_enabled;
    /* instance id 0 ~8*/
    int32_t inst_id;
} dma_instance_t;

/******************************************************************************

 Mark the end of the C bindings section for C++ compilers.

***************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif
