#ifndef __SD_MULTI_SFS_DEF__H_
#define __SD_MULTI_SFS_DEF__H_

/*
    ***************************
    *    MULTI_SFS_HEADER_T   *
    *************************
    ===========================
    =   FLASH_SEQ_INFO_T 0    =  ---
    =       ...               =    |
    =   FLASH_SEQ_INFO_T n    =    |
    ===========================    |
    ---------------------------    |
    -   FLASH_SEQ_DATA 0      - <--|
    -       ...               -
    -   FLASH_SEQ_DATA n      -
    ---------------------------
*/

#define MSFS_MAGIC        (0x5346534D) // which means "MSFS"
#define MULTI_SFS_MAX_CNT  (8)
#define FLASH_ID_BITS_LEN  (16)

typedef struct msfs {
    uint32_t magic;
    uint32_t version;
    uint32_t count;        // the number of flash sequence
    uint32_t offset;       // the offset from the msfs file header to the array of flash sequence info struct buffer
    uint8_t  reserved[16]; // reserved

}__PACKED msfs_t;

typedef struct flash_seq_info {
    uint32_t version;   // struct version
    uint32_t offset;    // data offset
    uint32_t size;      // data size;
    uint64_t flash_id;  // flash id Manufacturer ID (1 Byte)<<8 | Memory Type (1 Byte)
    // e.g. "gd25q32/16" -> 0xC840
    uint8_t reserved[12];   // reserved
}__PACKED flash_seq_info_t;

#endif // __SD_MULTI_SFS_DEF__H_
