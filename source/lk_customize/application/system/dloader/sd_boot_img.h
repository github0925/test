/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _SD_BOOT_IMG_H
#define _SD_BOOT_IMG_H

#define SFS_TAG  (0x53465301)
#define SFS_TAG_OFFSET         (0x0)
#define SFS_INIT_ACT_OFFSET    (0x4)
#define SFS_XFER_CONFIG_OFFSET (0x40)
#define SFS_IP_SETTINGS_OFFSET (0x50)
#define SFS_FREQ_OFFSET        (0x60)
#define SFS_TP_OFFSET          (0x68)
#define SFS_NIA_OFFSET         (0x70)
#define SFS_BIA_OFFSET         (0x74)
#define SFS_CRC32_OFFSET       (0x7C)

#define SFS_SIZE 128
#define SFS_INIT_ACT_SIZE      (0x3C)
#define SFS_XFER_CONFIG_SIZE   (0x10)
#define SFS_IP_SETTINGS_SIZE   (0x10)
#define SFS_TP_SIZE            (0x8)

#define BPT_TAG  (0x42505401)
#define BPT_TAG_OFFSET     (0x0)
#define BPT_SIZE_OFFSET    (0x6)
#define BPT_SEC_VER_OFFSET (0x8)
#define BPT_MDA_OFFSET     (0xC)
#define BPT_IIB_OFFSET     (0x20)
#define BPT_RCP_OFFSET     (0x9C)
#define BPT_SIG_OFFSET     (0x480)
#define BPT_CRC32_OFFSET   (0x7FC)

#define BPT_SIZE     (0x800)
#define BPT_RCP_SIZE (0x414)
#define BPT_SIG_SIZE (0x200)

#define IIB_TAG  (0xEAE1)
#define IIB_TAG_OFFSET       (0x0)
#define IIB_SIZE_OFFSET      (0x2)
#define IIB_DCC_OFFSET       (0x8)
#define IIB_DID_OFFSET       (0x10)
#define IIB_IMG_SZ_OFFSET    (0x28)
#define IIB_LOAD_BASE_OFFSET (0x2C)
#define IIB_EP_OFFSET        (0x34)
#define IIB_HASH_OFFSET      (0x3C)

#define IIB_DCC_SIZE      (0x8)
#define IIB_DID_SIZE      (0x8)
#define IIB_HASH_SIZE     (0x40)

#define RCP_TAG         (0xEAF0)
#define RCP_TAG_OFFSET  (0x0)
#define RCP_SIZE_OFFSET (0x2)
#define RCP_ID_OFFSET   (0x4)
#define RCP_PKT_OFFSET  (0x5)
#define RCP_PK_OFFSET   (0x10)

#define RCP_PK_SIZE (0x404)
#define RCP_SIZE    (0x414)


struct sfs {
    uint32_t tag;
    uint8_t init_act[SFS_INIT_ACT_SIZE];
    uint8_t xfer_config[SFS_XFER_CONFIG_SIZE];
    uint8_t ospi_settings[SFS_IP_SETTINGS_SIZE];
    uint8_t freq;
    uint8_t training_pattern[SFS_TP_SIZE];
    uint32_t normal_img_base;
    uint32_t backup_img_base;
    uint32_t crc32;
};

struct iib {
    uint16_t tag;
    uint16_t size;
    uint8_t dcc[IIB_DCC_SIZE];
    uint8_t did[IIB_DID_SIZE];
    uint32_t image_size;
    uint32_t load_base;
    uint32_t entry_point;
    uint8_t hash[IIB_HASH_SIZE];
};

struct rcp {
    uint16_t tag;
    uint16_t size;
    uint8_t id;
    uint8_t pk_type;
    uint8_t pk[RCP_PK_SIZE];
};

struct bpt {
    uint32_t tag;
    uint16_t size;
    uint32_t sec_version;
    uint8_t hash_alg;
    struct iib iib;
    struct rcp rcp;
    uint8_t signature[BPT_SIG_SIZE];
    uint32_t crc32;
};

int32_t get_sfs_info(struct sfs *sfs, uint8_t *buffer, uint32_t len);
int32_t get_bpt_info(struct bpt *bpt, uint8_t *buffer, uint32_t len);
uint32_t sfs_crc32(uint32_t crc, uint8_t *buffer, uint32_t len);

#endif
