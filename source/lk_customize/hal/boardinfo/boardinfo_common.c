#include <__regs_base.h>
#include <reg.h>
#include <lib/reg.h>
#include "boardinfo_common.h"
#include "eeprom.h"
#include "assert.h"
#include "debug.h"
#ifdef BOARDINFO_GPIO
#include "board_info_gpio.h"
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) ((int)(sizeof(A)/sizeof((A)[0])))
#endif

#define BOARDINFO_FUSE_UUID_OFFSET    0x1020
#define BOARDINFO_APB_EFUSEC_BASE    APB_EFUSEC_BASE

struct stor_item_t {
    enum stor_item_e item;
    uint64_t offset;
    int len;
};

#define STOR_ITEM(_item, _offset, _len) \
    {   .item = STOR_ITEM_ ##_item, \
        .offset = _offset, \
        .len = _len, \
    }

struct stor_item_t g_stor_item[] = {
    STOR_ITEM(HWID, 0, 4),
};

static storage_device_t *boardinfo_init(int index)
{
    int ret = 0;
    storage_device_t *bdinfo_dev = NULL;
#ifdef BOARDINFO_EEPROM
    bdinfo_dev = get_eeprom_dev(index);
#elif defined BOARDINFO_GPIO
    bdinfo_dev = get_board_info_pin_dev(index);
#endif

    if (!bdinfo_dev) {
        return NULL;
    }

    ret = bdinfo_dev->init(bdinfo_dev, 0, 0);

    if (ret != 0)
        bdinfo_dev = NULL;

    return bdinfo_dev;
}
static struct stor_item_t *get_item(enum stor_item_e item)
{
    int i;

    for (i = 0; i < ARRAYSIZE(g_stor_item); i++) {
        if (g_stor_item[i].item == item)
            return &g_stor_item[i];
    }

    return NULL;
}
static int internal_boardinfo_write(int devindex, enum stor_item_e item,
                                    const void *data, int len)
{
    struct stor_item_t *p = get_item(item);
    storage_device_t *bdinfo_dev = NULL;
    int ret = -1;

    if (!p)
        return -1;

    ASSERT(p->len == len);
    //program eeprom
    bdinfo_dev = boardinfo_init(devindex);

    if (bdinfo_dev) {
        ret = bdinfo_dev->write(bdinfo_dev, p->offset, (void *)data, len);
        bdinfo_dev->release(bdinfo_dev);
    }

    return ret;
}
static int internal_boardinfo_read(int devindex, enum stor_item_e item,
                                   void *data, int len)
{
    struct stor_item_t *p = get_item(item);
    storage_device_t *bdinfo_dev = NULL;
    int ret = -1;

    if (!p)
        return -1;

    ASSERT(p->len == len);
    bdinfo_dev = boardinfo_init(devindex);

    if (bdinfo_dev) {
        ret = bdinfo_dev->read(bdinfo_dev, p->offset, (void *)data, len);
        bdinfo_dev->release(bdinfo_dev);
    }

    return ret;
}
int boardinfo_write(enum stor_item_e item, const void *data, int len)
{
    return internal_boardinfo_write(0, item, data, len);
}
int boardinfo_read(enum stor_item_e item, void *data, int len)
{
    return internal_boardinfo_read(0, item, data, len);
}
int boardinfo_write_part2(enum stor_item_e item, const void *data, int len)
{
    return internal_boardinfo_write(1, item, data, len);
}
int boardinfo_read_part2(enum stor_item_e item, void *data, int len)
{
    return internal_boardinfo_read(1, item, data, len);
}

int boardinfo_get_serialno(uint32_t *uuid)
{
    int ret;
    addr_t base;

    if(uuid == NULL){
        return -1;
    }

    base = _ioaddr(BOARDINFO_APB_EFUSEC_BASE);

    *uuid = readl(base + BOARDINFO_FUSE_UUID_OFFSET);
    *(uuid + 1) = readl(base + BOARDINFO_FUSE_UUID_OFFSET + 4);

    if(*uuid == 0){
        ret = 1;
    }else{
        ret = 0;
    }

    return ret;
}