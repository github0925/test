/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef __GPT_H__
#define __GPT_H__

/* OSPI BASE ADDR */
#define BASE 0x4000000UL

/* OSPI First Partition */
#define SFS_SIZE 0x2000

/* OPSI GPT Partion */
#define GPT_MBR_SIZE 0x200
#define GPT_HEADER_SIZE 0x200

/* GPT Partion Info */
#define GPT_ENTRY_NAME_OFFSET 0x38
#define GPT_ENTRY_START_ADDR_OFFSET 0x20
#define GPT_ENTRY_START_ADDR_SIZE   0x8
#define GPT_ENTRY_END_ADDR_OFFSET 0x28
#define GPT_ENTRY_END_ADDR_SIZE   0x8

#define GPT_ENTRY_START_ADDR (BASE+SFS_SIZE+GPT_MBR_SIZE+GPT_HEADER_SIZE)
#define GPT_ENTRY_SIZE 0x80

#define GPT_ENTRY_NAME_ADDR(cnt) (GPT_ENTRY_START_ADDR+(cnt)*GPT_ENTRY_SIZE+GPT_ENTRY_NAME_OFFSET)
#define GPT_ENTRY_FIRST_LBA_ADDR(cnt) (*(uint32_t *)(GPT_ENTRY_START_ADDR+(cnt)*GPT_ENTRY_SIZE+GPT_ENTRY_START_ADDR_OFFSET))
#define GPT_ENTRY_LAST_LBA_ADDR(cnt) (*(uint32_t *)(GPT_ENTRY_START_ADDR+(cnt)*GPT_ENTRY_SIZE+GPT_ENTRY_END_ADDR_OFFSET))

#define GPT_PARTITION_ADDR(cnt) (BASE + SFS_SIZE + GPT_ENTRY_FIRST_LBA_ADDR(cnt)*SECTOR_SIZE)
#define GPT_PARTITION_SIZE(cnt) ((GPT_ENTRY_LAST_LBA_ADDR(cnt) - GPT_ENTRY_FIRST_LBA_ADDR(cnt) + 1)*SECTOR_SIZE)

#define GPT_PARTITION_NUM 128

/* logic size */
#define SECTOR_SIZE 0x200

#define PT_DDR_FW  "ddr_fw"
#define PT_DDR_INIT_SEQ "ddr_init_seq"

#endif  /* __GPT_H__ */
