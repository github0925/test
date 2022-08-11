/*
 * spi_nor_hal.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: spi norflash hal header.
 *
 * Revision History:
 * -----------------
 * 0.1, 11/10/2019 init version
 */

#ifndef __SPI_NOR_HAL_H__
#define __SPI_NOR_HAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "res.h"

#define DEFAULT_SPI_NOR_MAX_NUM 2

enum spi_nor_opt {
    SPI_NOR_OPT_READ = 1,
    SPI_NOR_OPT_WRITE,
    SPI_NOR_OPT_ERASE,
    SPI_NOR_OPT_MAX,
};

struct spi_nor_priv_res {
    uint32_t slot;
    uint32_t irq;
    uint32_t res_reg_glb_idx;
    uint32_t res_glb_idx;
    uint32_t rst_res_glb_idx;
    uint32_t ckgen_res_glb_idx;
};

typedef uint64_t spi_nor_address_t;

typedef uint64_t spi_nor_length_t;

enum spi_nor_opt_result {
    SPI_NOR_OPT_COMPLETE = 0,
    SPI_NOR_OPT_FAILED,
    SPI_NOR_OPT_PENDING,
    SPI_NOR_OPT_INCONSISTENT,
    SPI_NOR_OPT_INVALID,
    MAX_OPT_RESULET_MAX,
};

typedef void (*spi_nor_notification)(enum spi_nor_opt type,
                                     enum spi_nor_opt_result result);

struct spi_nor_cfg {
#define SPI_NOR_CS0 (0)
#define SPI_NOR_CS1 (1)
    uint8_t cs;

    /* Clock rates */
#define SPI_NOR_CLK_25MHZ 25000000
#define SPI_NOR_CLK_50MHZ 50000000
#define SPI_NOR_CLK_100MHZ 100000000
#define SPI_NOR_CLK_200MHZ 200000000
#define SPI_NOR_CLK_400MHZ 400000000
    uint32_t bus_clk; /* Max clock rate supported */

    /*
     * if dtr_enable true,
     * all opcode, address and data bytes will be transfer by 8-8-8 ddr mode.
     */
    bool octal_ddr_en;
};

struct spi_nor_handle {
    uint32_t id;
    uint32_t irq;
    addr_t apb_base;
    addr_t ahb_base;
    struct spi_nor_cfg *config;
    /*block size, write protect and erase para need aligned with it */
    uint32_t block_size;

    /*
     * if the async_mode is ture,
     * the erase/read/write api is asynchronous functions,
     * the user need register the notification event handle,
     * ctrl the spi_nor complete or error event.
     */
    bool async_mode;
    enum spi_nor_opt opt_type;
    enum spi_nor_opt_result opt_result;
    spi_nor_notification event_handle;

    void *priv_data;
};

/*
 * Function: spi_nor creat handle api
 * Arg     : spi_nor device handle, spi_nor id
 * Return  : true on Success, false on failed
 */
bool hal_spi_nor_creat_handle(void **handle, uint32_t spi_nor_res_glb_idx);

/*
 * Function: spi_nor release handle api
 * Arg     : spi_nor device handle
 * Return  : true on Success, false on failed
 */
bool hal_spi_nor_release_handle(void **handle);

/*
 * Function: spi_nor init api, alloc device struct memory
 * Arg     : spi_nor device handle
 * Return  : 0 on Success, non zero on failure
 */
int hal_spi_nor_init(void *handle);

#if 0
/*
 * Function: spi_nor init api, release device struct memory
 * Arg     : spi_nor device handle
 * Return  : 0 on Success, non zero on failure
 */
int hal_spi_nor_deinit(void *handle);
#endif

/*
 * Function: spi_nor read api
 * Arg     : spi_nor device handle, src address, dst buffer ptr, read length
 * Return  : 0 on Success, non zero on failure
 */
int hal_spi_nor_read(void *handle, spi_nor_address_t src, uint8_t *dst,
                     spi_nor_length_t length);

/*
 * Function: spi_nor write api
 * Arg     : spi_nor device handle, dst address, src buffer ptr, write length
 * Return  : 0 on Success, non zero on failure
 */
int hal_spi_nor_write(void *handle, spi_nor_address_t dst,
                      const uint8_t *src_buf, spi_nor_length_t length);

/*
 * Function: spi_nor erase api
 * Arg     : spi_nor device handle, dst address, erase length
 * Return  : 0 on Success, non zero on failure
 */
int hal_spi_nor_erase(void *handle, spi_nor_address_t dst,
                      spi_nor_length_t length);

/*
 * Function: spi_nor asynchronous cancel api
 * Arg     : spi_nor device handle
 * Return  : 0 on Success, non zero on failure
 */
int hal_spi_nor_cancel(void *handle);

/*
 * Function: spi_nor get device capacity api
 * Arg     : spi_nor device handle
 * Return  : device memory capacity count by byte
 */
spi_nor_length_t hal_spi_nor_get_capacity(void *handle);

/*
 * Function: spi_nor get device id api
 * Arg     : spi_nor device handle
 * Return  : flash id
 */
spi_nor_length_t hal_spi_nor_get_flash_id(void *handle);


/*
 * Function: spi_nor reset slave
 * Arg     : spi_nor device handle
 * Return  : None
 */
void hal_spi_nor_reset_slave(void *handle);

#ifdef __cplusplus
}
#endif
#endif // __SPI_NOR_HAL_H__
