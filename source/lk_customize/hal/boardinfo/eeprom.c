#include "eeprom.h"
#include "i2c_hal.h"
#include "hal_port.h"
struct eeprom_data {
    uint32_t i2c_bus_resid;
    uint8_t slave_addr;
    void *i2c_handle;
};
extern const domain_res_t g_iomuxc_res;
const Port_PinModeType MODE_GPIO_E10_M1_I2C4_SCL = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT3),
};

const Port_PinModeType MODE_GPIO_E11_M1_I2C4_SDA = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT3),
};

void i2c4_port_config(void)
{
    static void *i2c_port_handle;
    bool ioret;
    // Port setup
    ioret = hal_port_creat_handle(&i2c_port_handle, g_iomuxc_res.res_id[0]);

    if (!ioret) {
        return;
    }

    ioret = hal_port_set_pin_mode(i2c_port_handle, PortConf_PIN_RGMII1_RXD3,
                                  MODE_GPIO_E10_M1_I2C4_SCL);
    ioret = hal_port_set_pin_mode(i2c_port_handle, PortConf_PIN_RGMII1_RX_CTL,
                                  MODE_GPIO_E11_M1_I2C4_SDA);
    hal_port_release_handle(&i2c_port_handle);
}

static int init(storage_device_t *storage_dev, uint32_t res_idx,
                void *config)

{
    void *i2c_handle = NULL;
    struct eeprom_data *data = config ? config : storage_dev->priv;
    i2c_app_config_t i2c_conf;
    hal_i2c_creat_handle(&i2c_handle, data->i2c_bus_resid);

    if (i2c_handle == NULL) {
        return -1;
    }

    data->i2c_handle = i2c_handle;
    storage_dev->priv = data;
    i2c_conf = hal_i2c_get_busconfig(i2c_handle);
    i2c_conf.poll = 1;
    hal_i2c_set_busconfig(i2c_handle, &i2c_conf);
    return 0;
}
static int release(storage_device_t *storage_dev)
{
    struct eeprom_data *data = storage_dev->priv;
    hal_i2c_release_handle(data->i2c_handle);
    data->i2c_handle = NULL;
    return 0;
}

static int write(storage_device_t *storage_dev, uint64_t offset,
                 const uint8_t *buf, uint64_t data_len)
{
    int ret = 1;
    struct eeprom_data *data = storage_dev->priv;
    hal_i2c_write_reg_data(data->i2c_handle, data->slave_addr,
                           (void *)&offset, 1, (void *)buf, data_len);
    ret = 0;
    return ret;
}
static int read(storage_device_t *storage_dev, uint64_t src,
                uint8_t *dst, uint64_t data_len)
{
    int ret = 1;
    struct eeprom_data *data = storage_dev->priv;
    hal_i2c_read_reg_data(data->i2c_handle, data->slave_addr,
                          (void *)&src, 1, (void *)dst, data_len);
    ret = 0;
    return ret;
}

struct eeprom_data eeprom_dev_data[2] = {
    {RES_I2C_I2C4, 0x50, NULL},
    {RES_I2C_I2C4, 0x51, NULL},
};

static storage_device_t eeprom_dev[2] = {
    {
        .priv = &eeprom_dev_data[0],
        .init = init,
        .read = read,
        .write = write,
        .erase = NULL,
        .release = release,
    },
    {
        .priv = &eeprom_dev_data[1],
        .init = init,
        .read = read,
        .write = write,
        .erase = NULL,
        .release = release,
    },
};

storage_device_t *get_eeprom_dev(int index)
{
    if (index >= 2)
        return NULL;

    i2c4_port_config();
    return &eeprom_dev[index];
}


