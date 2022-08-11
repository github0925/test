/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef __BOOT_FUN_H__
#define __BOOT_FUN_H__

#include <stdint.h>
#include <__regs_base.h>

#define FUSE_BT_CGF_INDEX0    173
#define FUSE_BT_CGF_INDEX1    (FUSE_BT_CGF_INDEX1 + 1)
#define FUSE_BT_CGF_INDEX2    (FUSE_BT_CGF_INDEX2 + 1)

#define PIN_ERROR -1U

//Boot Information Block
#define BOOT_INFO_MAGIC        0xe7002001
#define EMMC1_BOOT_FROM_BLK    1
#define EMMC2_BOOT_FROM_BLK    2
#define OSPI1_BOOT_FROM_BLK  0x10
#define OSPI2_BOOT_FROM_BLK  0x11
#define USB_BOOT_FROM_BLK    0x30
#define PEER_BOOT_FROM_BLK   0x40

#define BOOT_PIN_0   0U    /* SEC wait for peer load, SAF boot from OSPI1 */
#define BOOT_PIN_1   1U    /* SEC boot from emmc1,    SAF boot from OPSI1 */
#define BOOT_PIN_2   2U    /* SEC boot from emmc2, SAF boot from OSPI1 */
#define BOOT_PIN_3   3U    /* SEC boot from SD, SAF boot from OSPI1 */
#define BOOT_PIN_4   4U    /* SEC boot from OSPI2, SAF boot from OSPI1 */
#define BOOT_PIN_5   5U    /* SEC boot from USB device, SAF boot from OSPI1 */
#define BOOT_PIN_6   6U    /* Reserved boot pin */
#define BOOT_PIN_7   7U    /* Reserved boot pin */
#define BOOT_PIN_8   8U    /* SEC boot from USB device, SAF wait for peer load */
#define BOOT_PIN_9   9U    /* SEC boot from emmc1, SAF handover then fail safe mode */
#define BOOT_PIN_10 10U    /* SEC boot from emmc2, SAF handover then fail safe mode */
#define BOOT_PIN_11 11U    /* SEC boot from SD, SAF handover then fail safe mode */
#define BOOT_PIN_12 12U    /* SEC boot from OSPI2, SAF handover then fail safe mode */
#define BOOT_PIN_13 13U    /* SEC boot from USB device, SAF handover then fail safe mode */
#define BOOT_PIN_14 14U    /* Reserved boot pin */
#define BOOT_PIN_15 15U    /* Debug mode for both SEC and SAF */


#define BOOT_INFO_FROM_GPIO_SHIFT                    0
#define BOOT_INFO_FROM_GPIO_MASK                     (1<<BOOT_INFO_FROM_GPIO_SHIFT)

#define BOOT_INFO_FROM_FUSE_SHIFT                    1
#define BOOT_INFO_FROM_FUSE_MASK                     (1<<BOOT_INFO_FROM_FUSE_SHIFT)

#define BOOT_INFO_FROM_SCR_SHIFT                     2
#define BOOT_INFO_FROM_SCR_MASK                      (1<<BOOT_INFO_FROM_SCR_SHIFT)

#define BOOT_INFO_USB_PROVISION_DIS_SHIFT             3
#define BOOT_INFO_USB_PROVISION_DIS_MASK              (1<<BOOT_INFO_USB_PROVISION_DIS_SHIFT)

#define BOOT_INFO_SAFETY_HANDOVER_DIS_SHIFT           4
#define BOOT_INFO_SAFETY_HANDOVER_DIS_MASK            (1<<BOOT_INFO_SAFETY_HANDOVER_DIS_SHIFT)

#define BOOT_INFO_PEER_LOAD_ON_SAF_FAILURE_DIS_SHIFT     5
#define BOOT_INFO_PEER_LOAD_ON_SAF_FAILURE_DIS_MASK      (1<<BOOT_INFO_PEER_LOAD_ON_SAF_FAILURE_DIS_SHIFT)

#define BOOT_INFO_BOOT_FROM_PEER_LOAD_SHIFT          6
#define BOOT_INFO_BOOT_FROM_PEER_LOAD_MASK           (1<<BOOT_INFO_BOOT_FROM_PEER_LOAD_SHIFT)

#define BOOT_INFO_BOOT_PIN_UPDATED_SHIFT             31
#define BOOT_INFO_BOOT_PIN_UPDATE_MASK               (1<<BOOT_INFO_BOOT_PIN_UPDATED_SHIFT)

typedef struct boot_info {
        uint32_t boot_ops;
        uint32_t boot_pin;
} boot_info_t;

/* Boot info block */
#define BT_BLOCK_INFO_LOCATION (R5_SEC_TCMB_BASE + 64*2*0x400 - 0x80)

struct boot_info_block {
    uint32_t tag;  /* Should be 0xe7002001 */
    uint32_t rom_revision;
    uint8_t boot_device;
    uint8_t boot_image;
    unsigned char reserved[6];
    unsigned char misc[16];
};

/* Overwrite boot pin from saf, then reset sec */
uint32_t overwrite_pin_by_scr(uint32_t pin);

/* Read scr/fuse/gpio to get boot options */

uint32_t boot_get_pin(void);

bool is_boot_from_peer_load(void);

bool need_peer_load_on_saf_failure(void);

bool is_usb_provision(void);

#endif

