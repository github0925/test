/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __MEMORY_MAP_H__
#define __MEMORY_MAP_H__

#define ATCM_BASE   0x4C0000
#define BTCM_BASE   0x4B0000
#define B0TCM_BASE  BTCM_BASE
#define B1TCM_BASE  (B0TCM_BASE + 0x8000)

#define TCM_BASE    BTCM_BASE
#define TCM_SZ      0x20000
#define TCM_END     (BTCM_BASE + TCM_SZ - 1)

#define SEC_TCM_SYSTEM_ADDR     0x3B0000
#define SAF_TCM_SYSTEM_ADDR     0x330000

/* For AP, the address is 0x30000000UL which is not relative here */
#define TB_CTRL_BASE (0xF0000000UL)
#define SYS_TB_CTRL_BASE_ADDR (TB_CTRL_BASE + 0x1460000)

#define ROM_BASE    0
#if defined(TGT_safe)
#define ROM_SIZE    0x20000
#define SYS_ICACHE_BASE  R5_SAF_CACHE_BASE
#define SYS_DCACHE_BASE  (SYS_ICACHE_BASE + 0x8000u)
#else
#define ROM_SIZE    0x40000
#define SYS_ICACHE_BASE  R5_SEC_CACHE_BASE
#define SYS_DCACHE_BASE  (SYS_ICACHE_BASE + 0x8000u)
#endif
#define ROM_END (ROM_BASE + ROM_SIZE - 1)

#define RAM_USR_BASE    (RAM_BASE + RAM_SIZE)

#if defined(TGT_safe)
#define OSPI_MEM_BASE       OSPI1_BASE
#else
#define OSPI_MEM_BASE       OSPI2_BASE
#endif
#define OSPI_MEM_SZ         0x4000000
#define OSPI_MEM_END       (OSPI_MEM_BASE + OSPI_MEM_SZ - 1)
#define OSPI_ADDR_WIDTH    26

#define TB_TRIG_CMD_ADDR    0x4CFFC0
#define TB_TRIG_ARG1_ADDR   0x4CFFD0
#define TB_TRIG_ARG2_ADDR   0x4CFFE0
#define CR5_DDR_BASE        0x30000000UL

#endif // __MEMORY_MAP_H__
