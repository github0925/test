/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __SOC_HAL_H__
#define __SOC_HAL_H__

#include <types_def.h>
#include <soc_def.h>
#if !defined(SOC_host)
#include "lnk_sym.h"
#endif

typedef enum {
    LC_ATE = 0,
    LC_DEV,
    LC_PROD,
    LC_FAIL,
} life_cycle_e;

typedef struct {
    U32 hdr;    /* Tag: bit31-24, 0xE7; bit23-16, size; bit7:0, ver */
    U32 rom_ver;
    U8 dev_id;  /* bt_dev_id_e */
    U8 image;   /* bt_image_e */
    U8 rsvd[18];
    U32 log_addr;
} bt_infor_t, *bt_infor_p;

typedef enum {
    RST_CYCLE_PERSIST,
    PWR_CYCLE_PERSIST,
} storage_reg_type_e;

void soc_early_init(void);
void soc_init(void);
void soc_clock_init(void);

uintptr_t soc_get_module_base(module_e m);
U32 soc_read_fuse(U32 id);
U32 soc_sense_fuse(U32 id);
U32 soc_get_bt_pin(void);
void soc_set_bt_pin(U32);
life_cycle_e soc_get_lc(void);
void soc_config_clk(module_e m, clk_freq_e freq);
void soc_pin_cfg(module_e m, void *para);
uintptr_t soc_to_dma_address(uintptr_t cpu_addr);
U32 soc_get_core_id(void);
void soc_read_uuid(U8 *id, U32 sz);
void soc_read_dev_id(U8 *id, U32 sz);
void soc_deassert_reset(module_e m);
void soc_assert_reset(module_e m);
U32 soc_get_reset_source(void);
U32 soc_rd_storage_reg(U8 type, U32 id);
void soc_wr_storage_reg(U8 type, U32 id, U32 v);
void system_reset(void);
void soc_wdog_reset_en(void);
void soc_en_isolation(module_e m);
void soc_dis_isolation(module_e m);
U32 soc_lock_memory(uintptr_t start, size_t sz);
void soc_assert_security_violation(void);
void soc_kick_cpu(module_e m);
void soc_start_cpu_core(module_e m, uint32_t core, addr_t rvba);
void soc_stop_secondary_core(module_e m, uint32_t core);
void soc_get_rand(uint8_t *rng, size_t sz);

#define vaddr_to_paddr(a)  soc_to_dma_address((uintptr_t)(a))

#endif  /* __SOC_HAL_H__ */
