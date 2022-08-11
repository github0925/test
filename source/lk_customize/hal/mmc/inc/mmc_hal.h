/*
 * mmc_hal.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SD/eMMC hal header.
 *
 * Revision History:
 * -----------------
 * 0.1, 11/10/2019 init version
 */

#ifndef __MMC_HAL_H__
#define __MMC_HAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "chip_res.h"

#define DEFAULT_MMC_MAX_NUM 4

enum mmc_opt {
    MMC_OPT_READ = 1,
    MMC_OPT_WRITE,
    MMC_OPT_ERASE,
    MMC_OPT_MAX,
};

struct mmc_priv_res {
    uint8_t slot;
    uint32_t res_glb_idx;
    uint32_t int_irq;
    uint32_t wake_irq;
    uint32_t rst_idx;
    uint32_t clk_ip_idx;
    uint32_t clk_ip_gate_idx;
    uint64_t ddr_mode_scr_signal;
};

typedef uint64_t mmc_address_t;

typedef uint64_t mmc_length_t;

enum mmc_opt_result {
    MMC_OPT_COMPLETE = 0,
    MMC_OPT_FAILED,
    MMC_OPT_PENDING,
    MMC_OPT_INCONSISTENT,
    MMC_OPT_INVALID,
    MMC_OPT_RESULET_MAX,
};

typedef void (*mmc_notification)(enum mmc_opt type, enum mmc_opt_result result);

struct mmc_cfg {
    /* Clock rates */
#define MMC_CLK_400KHZ 400000
#define MMC_CLK_25MHZ 25000000
#define MMC_CLK_50MHZ 50000000
#define MMC_CLK_100MHZ 100000000
#define MMC_CLK_200MHZ 200000000
#define MMC_CLK_400MHZ 400000000
    uint32_t max_clk_rate; /* Max clock rate supported */

#define MMC_BUS_WIDTH_1BIT 0
#define MMC_BUS_WIDTH_4BIT 1
#define MMC_BUS_WIDTH_8BIT 2
    uint16_t bus_width; /* Bus width used */

#define MMC_VOL_1_8 5
#define MMC_VOL_3_0 6
#define MMC_VOL_3_3 7
    uint32_t voltage; /* bus voltage */

    uint8_t hs200_support; /* SDHC HS200 mode supported or not */
    uint8_t hs400_support; /* SDHC HS400 mode supported or not */
};

struct mmc_handle {
    addr_t apb_base;
    struct mmc_cfg *config;
    struct mmc_priv_res *priv_res;
    /*block size, read write api para need aligned with it */
    uint32_t block_size;
    /*erase group size, erase api length para need aligned with it */
    uint32_t erase_grp_size;

    /*
     * if the async_mode is ture,
     * the erase/read/write api is asynchronous functions,
     * the user need register the notification event handle,
     * ctrl the mmc complete or error event.
     */
    bool async_mode;
    enum mmc_opt opt_type;
    enum mmc_opt_result opt_result;
    mmc_notification event_handle;

    void *priv_data;
};

/*
 * Function: mmc creat handle api
 * Arg     : mmc device handle, mmc id
 * Return  : true on Success, false on failed
 */
bool hal_mmc_creat_handle(void **handle, uint32_t mmc_res_glb_idx);

/*
 * Function: mmc release handle api
 * Arg     : mmc device handle
 * Return  : true on Success, false on failed
 */
bool hal_mmc_release_handle(void **handle);

/*
 * Function: mmc init api, alloc device struct memory
 * Arg     : mmc device handle
 * Return  : 0 on Success, non zero on failure
 */
int hal_mmc_init(void *handle);

/*
 * Function: mmc read api
 * Arg     : mmc device handle, src address, dst buffer ptr, read length
 * Return  : 0 on Success, non zero on failure
 */
int hal_mmc_read(void *handle, mmc_address_t src, uint8_t *dst,
                 mmc_length_t length);

/*
 * Function: mmc write api
 * Arg     : mmc device handle, dst address, src buffer ptr, write length
 * Return  : 0 on Success, non zero on failure
 */
int hal_mmc_write(void *handle, mmc_address_t dst, const uint8_t *src_buf,
                  mmc_length_t length);

/*
 * Function: mmc erase api
 * Arg     : mmc device handle, dst address, erase length
 * Return  : 0 on Success, non zero on failure
 */
int hal_mmc_erase(void *handle, mmc_address_t dst, mmc_length_t length);

/*
 * Function: mmc asynchronous cancel api
 * Arg     : mmc device handle
 * Return  : 0 on Success, non zero on failure
 */
int hal_mmc_cancel(void *handle);

/*
 * Function: mmc get device capacity api
 * Arg     : mmc device handle
 * Return  : device memory capacity count by byte
 */
mmc_length_t hal_mmc_get_capacity(void *handle);

/*
 * Function: mmc get device single block size
 *           read/write api para need aligned with erase group size.
 * Arg     : mmc device handle
 * Return  : device block size count by byte
 */
uint32_t hal_mmc_get_block_size(void *handle);

/*
 * Function: mmc get device erase group size,
 *           erase api length para need aligned with erase group size.
 * Arg     : mmc device handle
 * Return  : device erase group size count by byte
 */
uint32_t hal_mmc_get_erase_grp_size(void *handle);

enum part_access_type {
    PART_ACCESS_DEFAULT = 0x0,
    PART_ACCESS_BOOT1,
    PART_ACCESS_BOOT2,
    PART_ACCESS_RPMB,
    PART_ACCESS_GP1,
    PART_ACCESS_GP2,
    PART_ACCESS_GP3,
    PART_ACCESS_GP4
};
/*
 * Function: mmc switch partition api
 * Arg     : mmc device handle, partition type
 * Return  : 0 on Success, non zero on failure
 */
int hal_mmc_switch_part(void *handle, enum part_access_type);

#ifdef __cplusplus
}
#endif
#endif // __MMC_HAL_H__
