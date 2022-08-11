#include <bits.h>
#include <debug.h>
#include <kernel/thread.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "am.h"

#include "am_ctl.h"
#include "am_debug.h"
#include "am_misc.h"

#define AM_DBG_PRT 2
extern am_board_t g_am_board;

bool set_chip_handle(unsigned int chip_id, void *handle, unsigned int addr)
{
    g_am_board.codec_dev[chip_id].dev_handle = handle;
    g_am_board.codec_dev[chip_id].addr = addr;
    return true;
}

bool set_chip_info(unsigned int chip_id, unsigned int protocol,
                   unsigned int codec_type)
{
    g_am_board.codec_dev[chip_id].protocol = protocol;
    g_am_board.codec_dev[chip_id].codec_type = codec_type;
    return false;
}

bool set_chip_writeable_func(unsigned int chip_id, writeable_reg_t func)
{
    g_am_board.codec[chip_id]->writeable_reg = func;
    return true;
}

am_codec_dev_t* get_chip_dev(unsigned int chip_id)
{
    return &g_am_board.codec_dev[chip_id];
}

bool set_chip_readable_func(unsigned int chip_id, readable_reg_t func)
{
    g_am_board.codec[chip_id]->readable_reg = func;
    return true;
}
/**
 * @brief
 *
 * @param chip_id
 * @return true
 * @return false
 */
bool reset(unsigned int chip_id) { return am_reset(chip_id); }
bool burn_fw(unsigned int chip_id, int fw_no)
{
    return am_burn_fw(chip_id, fw_no);
}
bool sync_ctl(unsigned int chip_id) { return am_sync_ctl(chip_id); }
bool read_ctl(unsigned int chip_id, unsigned int ctl_name, unsigned int *val)
{
    return am_read_ctl(chip_id, ctl_name, val);
}

bool write_ctl(unsigned int chip_id, unsigned int ctl_name, unsigned int val)
{
    return am_write_ctl(chip_id, ctl_name, val);
}

bool read_reg(unsigned int chip_id, unsigned int reg_addr, unsigned int *val)
{
    return am_read_reg(chip_id, reg_addr, val);
}

bool write_reg(unsigned int chip_id, unsigned int reg_addr, unsigned int val)
{
    return am_write_reg(chip_id, reg_addr, val);
}

bool write_reg_nocache(unsigned int chip_id, unsigned int reg_addr,
                       unsigned int val)
{
    return am_write_reg_nocache(chip_id, reg_addr, val);
}

bool write_ctl_nocache(unsigned int chip_id, unsigned int ctl_name,
                       unsigned int val)
{
    return am_write_ctl_nocache(chip_id, ctl_name, val);
}
bool make_dirty_reg(unsigned int chip_id, unsigned int reg_addr)
{
    return am_make_dirty_reg(chip_id, reg_addr);
}
/*TODO: No tested*/
bool make_dirty_chip(unsigned int chip_id)
{
    return am_make_dirty_chip(chip_id);
}

bool make_dirty_ctl(unsigned int chip_id, unsigned int ctl_name)
{
    return am_make_dirty_ctl(chip_id, ctl_name);
}
/* debug functions */
bool dump_regs(int chip_id) { return am_dump_regs(chip_id); }
bool delay_ctl(unsigned int val)
{
    thread_sleep(val);
    return true;
}
int get_linear_vol(int vol, int max_val, int min_val)
{
    return am_get_linear_vol(vol, max_val, min_val);
}
