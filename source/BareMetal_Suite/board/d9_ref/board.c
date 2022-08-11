/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include "board.h"
#include "i2c/dw_i2c/include/dw_i2c.h"
#include "srv_timer/srv_timer.h"
#include <error.h>

#if defined(CFG_BRD_DDR_VOL_SETUP)
#define MAX_I2C_NODE_NUM 16

#define PMIC_REG_AP_VOL      0x0au
#define PMIC_REG_GPU_VOL     0x0eu
#define PMIC_REG_CPU_VOL     0x10u
#define PMIC_REG_GPU_CTL_VOL 0x06u

dw_i2c_context  i2c_ctx[MAX_I2C_NODE_NUM];

inline static dw_i2c_context *get_i2c_ctx(module_e m, uint8_t address)
{
    uint32_t i = 0;

    for (; i < MAX_I2C_NODE_NUM; i++) {
        if (i2c_ctx[i].info.io_base == soc_get_module_base(m)
            && i2c_ctx[i].is_configured && i2c_ctx[i].info.slave_addr == address) {
            break;
        }
    }

    if (i != MAX_I2C_NODE_NUM) {
        return &i2c_ctx[i];
    } else {
        return NULL;
    }
}

bool pmic_i2c_init(module_e m, uint8_t address)
{
    addr_t b = soc_get_module_base(m);
    dw_i2c_config_t tca9539_i2c_cfg = {
        .io_base = b,
        .speed = I2C_SPEED_STANDARD,
        .addr_bits = ADDR_7BITS,
        .mode = MASTER_MODE,
        .slave_addr = address,
    };
    uint32_t i = 0;

    for (; i < MAX_I2C_NODE_NUM; i++) {
        if (!i2c_ctx[i].is_configured) {
            break;
        }
    }

    if (i < MAX_I2C_NODE_NUM) {
        return dw_i2c_set_busconfig(&i2c_ctx[i], &tca9539_i2c_cfg);
    } else {
        return false;
    }
}

bool pmic_i2c_write_reg(module_e m, uint8_t address, uint8_t reg, uint8_t data)
{
    dw_i2c_context *ctx = get_i2c_ctx(m, address);
    /* add delay between frame stop to next frame start */
    udelay(200);

    if (NULL == ctx) {
        return false;
    } else {
        return (NO_ERROR == dw_i2c_write_reg_bytes(ctx, ctx->info.slave_addr, reg, &data, 1));
    }
}

bool pmic_i2c_read_reg(module_e m, uint8_t address, uint8_t reg, uint8_t *data)
{
    dw_i2c_context *ctx = get_i2c_ctx(m, address);
    /* add delay between frame stop to next frame start */
    udelay(200);

    if (NULL == ctx) {
        return false;
    } else {
        return (NO_ERROR == dw_i2c_read_reg_bytes(ctx, ctx->info.slave_addr, reg, data, 1));
    }
}

bool pmic_i2c_deinit(module_e m, uint8_t address)
{
    dw_i2c_context *ctx = get_i2c_ctx(m, address);

    if (NULL == ctx) {
        return false;
    } else {
        memset((void *)ctx, 0, sizeof(dw_i2c_context));
        return true;
    }
}

static uint8_t calculate_voltage(int ap_volt_mv)
{
    if (ap_volt_mv >= 600 && ap_volt_mv <= 730) {
        return 0xa + ((ap_volt_mv - 600) + 9) / 10;
    } else if (ap_volt_mv > 730 && ap_volt_mv <= 1400) {
        return 0x18 + ((ap_volt_mv - 735) + 4) / 5;
    } else if (ap_volt_mv > 1400 && ap_volt_mv <= 3360) {
        return 0x9e + ((ap_volt_mv - 1420) + 19) / 20;
    } else {
        assert(0);
    }

    return 0;
}

int32_t change_voltage(uint8_t reg, uint32_t vol, bool need_convert)
{
    static bool first_call = true;
    uint8_t read_data = 0;

    if (first_call) {
        if (!pmic_i2c_init(I2C10, PMIC1_DEV_ID)) {
            DBG("%s: Opps, failed to init pmic i2c\n", __FUNCTION__);
            return -1;
        }

        first_call = false;
    }

    uint8_t val;
    if (need_convert){
        val = calculate_voltage(vol);
    }else{
        val = vol;
    }

    if (!pmic_i2c_write_reg(I2C10, PMIC1_DEV_ID, reg, val)) {
        WARN("%s: Opps, failed to write pmic reg\n", __FUNCTION__);
        return -2;
    }

    if (!pmic_i2c_read_reg(I2C10, PMIC1_DEV_ID, reg, &read_data) || read_data != val) {
        WARN("%s: Opps, failed to write pmic reg, read_data:%u val:%u\n", __FUNCTION__, read_data, val);
        return -3;
    }

    INFO("change voltage sucessfully, val:%u\n", read_data);

    return 0;
}

int setup_ddr_voltage(uint32_t vdd_id, uint32_t mv)
{
    static bool first_call = true;

    if (first_call) {
        if (!pmic_i2c_init(I2C10, PMIC2_DEV_ID)) {
            DBG("%s: Opps, failed to init pmic i2c\n", __FUNCTION__);
            return -1;
        }

        first_call = false;
    }

#define PMIC_DDR_1V1_REG    0x0c
#define PMIC_DDR_0V6_REG    0x0e
#define PMIC_DDR_1V8_REG    0x10

    uint8_t reg = 0;

    if (vdd_id == 1) {
        reg = PMIC_DDR_1V1_REG;
    } else if (2 == vdd_id) {
        reg = PMIC_DDR_0V6_REG;
    } else if (3 == vdd_id) {
        reg = PMIC_DDR_1V8_REG;
    } else {
        return -2;
    }

    vdd_id -= 1;

    uint8_t val = calculate_voltage(mv);

    uint8_t pgood_ctl1 = 0;

    if (!pmic_i2c_read_reg(I2C10, PMIC2_DEV_ID, 0x28, &pgood_ctl1)) {
        DBG("%s: Opps, failed to read pmic reg\n", __FUNCTION__);
        return -3;
    }

    /* Disable PGOOD monitor when change the BULK's voltage, otherwise PGOOD
     * de-asserted and may reset the board */
    uint8_t v = pgood_ctl1 & (~(0x03u << ((vdd_id + 1) * 2)));

    if (!pmic_i2c_write_reg(I2C10, PMIC2_DEV_ID, 0x28, v)) {
        DBG("%s: Opps, failed to write pmic reg\n", __FUNCTION__);
        return -4;
    }

    if (!pmic_i2c_write_reg(I2C10, PMIC2_DEV_ID, reg, val)) {
        DBG("%s: Opps, failed to write pmic reg\n", __FUNCTION__);
        return -5;
    }

    /* wait a while for power to ramp up/down */
    udelay(2000);

    if (!pmic_i2c_write_reg(I2C10, PMIC2_DEV_ID, 0x28, pgood_ctl1)) {
        DBG("%s: Opps, failed to write pmic reg\n", __FUNCTION__);
        return -6;
    }

    return 0;
}
#endif  /* CFG_BRD_DDR_VOL_SETUP */

int32_t board_setup(uint32_t part_rev, uint8_t type, uint8_t maj, uint8_t min)
{
#if defined(CFG_BRD_DDR_VOL_SETUP)
    uint32_t ap_mv = 800;
    module_e m = I2C10;

    soc_pin_cfg(I2C10, NULL);
    soc_config_clk(m, FREQ_DEFAULT);
    if (part_rev != 1)
        ap_mv = 720;

    INFO("%s: to setup vdd_ap to %umv part_rev:%u\n", __FUNCTION__, ap_mv, part_rev);
    change_voltage(PMIC_REG_AP_VOL, ap_mv, true);
    change_voltage(PMIC_REG_GPU_VOL, 850, true);
    change_voltage(PMIC_REG_CPU_VOL, 850, true);
    change_voltage(PMIC_REG_GPU_CTL_VOL, 0x96, false);
#endif
    return 0;
}
