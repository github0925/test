/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <error.h>
#include <soc.h>
#include "board.h"
#include "debug.h"
#include "fuse_ctrl.h"
#include "i2c/dw_i2c/include/dw_i2c.h"

#define EEPROM_SLV_ADDR     0x50
#define EEPROM_HWID_REG_OFF 0x00

#if defined (BOARD_INFO)
int board_info(void* buf, uint32_t len)
{
    bool ret;
    status_t err;
    module_e m = I2C4;
    //soc_deassert_reset(m);
    soc_pin_cfg(I2C4, NULL);
    soc_config_clk(m, FREQ_DEFAULT);
    addr_t b = soc_get_module_base(m);
    dw_i2c_context i2c_ctx;

    dw_i2c_config_t eeprom_i2c_cfg = {
        .io_base = b,
        .speed = I2C_SPEED_STANDARD,
        .addr_bits = ADDR_7BITS,
        .mode = MASTER_MODE,
        .slave_addr = EEPROM_SLV_ADDR,
    };

    if (!buf || !len)
    {
        INFO("buf error!\n");
        return ERR_NOT_ENOUGH_BUFFER;
    }

    ret = dw_i2c_set_busconfig(&i2c_ctx, &eeprom_i2c_cfg);
    if (!ret)
    {
        INFO("fail to config i2c!\n");
        return ERR_BAD_STATE;
    }

    err = dw_i2c_read_reg_bytes(&i2c_ctx, i2c_ctx.info.slave_addr, EEPROM_HWID_REG_OFF, buf, len);
    if(NO_ERROR != err)
    {
        INFO("fail to read hwid, err:%u!\n", err);
    }

    return err;
}
#endif
