
#include "serdes_9xx.h"
#include <i2c_hal.h>

//#define SERDES_DEBUG

int du90ub941_or_947_enable_port(void *i2c_handle, uint8_t addr,
    enum ts_serdes_type port_flag)
{
	int ret;
    u8 sreg = 0x1e, sval = 0;

    if (port_flag == TI941_SINGLE || port_flag == TI947_SINGLE)
        sval = 0x1;
    else if (port_flag == TI941_SECOND || port_flag == TI947_SECOND)
        sval = 0x2;
    else if (port_flag == TI941_DUAL || port_flag == TI947_DUAL)
		sval = 0x4;
	else{
		dprintf(ALWAYS, "%s, port_flag=%d invalid, addr=%#x\n",
			__func__, port_flag, addr);
		return -1;
	}

    ret = hal_i2c_write_reg_data(i2c_handle, addr, &sreg, 1, &sval, 1);
	if (ret) {
        dprintf(ALWAYS, "%s: port_flag=%d, addr=%#x, i2c write failed\n",
			__func__, port_flag, addr);
		return -1;
	}

#ifdef SERDES_DEBUG
	int ret1;
	sval = 0;

    ret1 = hal_i2c_read_reg_data(i2c_handle, addr, &sreg, 1, &sval, 1);
    if (ret1) {
		dprintf(ALWAYS, "%s: port_flag=%d, addr=%#x, i2c read failed\n",
			__func__, port_flag, addr);
    }

	dprintf(ALWAYS, "%s: port_flag=%d, addr=%#x, read=%#x\n",
		__func__, port_flag, addr, sval);
#endif
	return ret;
}

int du90ub941_or_947_enable_i2c_passthrough(void *i2c_handle, uint8_t addr)
{
	int ret;
	u8 sreg = 0x17, sval = 0x9e;

	ret = hal_i2c_write_reg_data(i2c_handle, addr, &sreg, 1, &sval, 1);
	if (ret) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c write failed\n", __func__, addr);
		return -1;
	}

#ifdef SERDES_DEBUG
	int ret1;
	sval = 0;

	ret1 = hal_i2c_read_reg_data(i2c_handle, addr, &sreg, 1, &sval, 1);
	if (ret1) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c read failed\n", __func__, addr);
	}

	dprintf(ALWAYS, "%s: addr=%#x, read=%#x\n", __func__, addr, sval);
#endif

	return ret;
}

int du90ub941_or_947_enable_int(void *i2c_handle, uint8_t addr)
{
	int ret;
	u8 sreg = 0xc6, sval = 0x21;

	ret = hal_i2c_write_reg_data(i2c_handle, addr, &sreg, 1, &sval, 1);
	if (ret) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c write failed\n", __func__, addr);
		return -1;
	}

#ifdef SERDES_DEBUG
	int ret1;
	sval = 0;

	ret1 = hal_i2c_read_reg_data(i2c_handle, addr, &sreg, 1, &sval, 1);
	if (ret1) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c read failed\n", __func__, addr);
	}

	dprintf(ALWAYS, "%s: addr=%#x, read=%#x\n", __func__, addr, sval);
#endif

	return ret;
}

int du90ub948_gpio_output(void *i2c_handle, uint8_t addr, int gpio, int val)
{
	int ret;
    u8 dreg = 0, dval = 0;

	switch (gpio) {
	case 0:
		dreg = 0x1d;
		break;
	case 1:
	case 2:
		dreg = 0x1e;
		break;
	case 3:
		dreg = 0x1f;
		break;
	case 5:
	case 6:
		dreg = 0x20;
		break;
	case 7:
	case 8:
		dreg = 0x21;
		break;
	default:
		dprintf(ALWAYS, "%s, 948(0) gpio num %d invalid\n", __func__, gpio);
		return -1;
    }

	ret = hal_i2c_read_reg_data(i2c_handle, addr, &dreg, 1, &dval, 1);
	if (ret) {
		dprintf(ALWAYS, "%s: addr=%#x, dreg=%#x, i2c read failed\n",
			__func__, addr, dreg);
	}

	switch (gpio) {
	case 0:
	case 1:
	case 3:
	case 5:
	case 7:
		dval &= 0xf0;
		if (val == 1)
			dval |= 0x09;
		else
			dval |= 0x01;
		break;
	case 2:
	case 6:
	case 8:
		dval &= 0x0f;
		if (val == 1)
			dval |= 0x90;
		else
			dval |= 0x10;
		break;
	default:
		dprintf(ALWAYS, "%s, 948(1) gpio num %d invalid\n", __func__, gpio);
		return -1;
    }

	ret = hal_i2c_write_reg_data(i2c_handle, addr, &dreg, 1, &dval, 1);
	if (ret) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c write failed\n", __func__, addr);
		return -1;
	}

#ifdef SERDES_DEBUG
	int ret1;
	dval = 0;

	ret1 = hal_i2c_read_reg_data(i2c_handle, addr, &dreg, 1, &dval, 1);
	if (ret1) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c read failed\n", __func__, addr);
	}

	dprintf(ALWAYS, "%s: addr=%#x, read=%#x\n", __func__, addr, dval);
#endif

    return ret;
}

int du90ub948_gpio_input(void *i2c_handle, uint8_t addr, int gpio)
{
	int ret;
    u8 dreg = 0, dval = 0;

	switch (gpio) {
	case 0:
		dreg = 0x1d;
		break;
	case 1:
	case 2:
		dreg = 0x1e;
		break;
	case 3:
		dreg = 0x1f;
		break;
	case 5:
	case 6:
		dreg = 0x20;
		break;
	case 7:
	case 8:
		dreg = 0x21;
		break;
	default:
		dprintf(ALWAYS, "%s, 948(0) gpio num %d invalid\n", __func__, gpio);
		return -1;
    }

	ret = hal_i2c_read_reg_data(i2c_handle, addr, &dreg, 1, &dval, 1);
	if (ret) {
		dprintf(ALWAYS, "%s: addr=%#x, dreg=%#x, i2c read failed\n",
			__func__, addr, dreg);
	}

	switch (gpio) {
	case 0:
	case 1:
	case 3:
	case 5:
	case 7:
		dval &= 0xf0;
		dval |= 0x03;
		break;
	case 2:
	case 6:
	case 8:
		dval &= 0x0f;
		dval |= 0x30;
		break;
	default:
		dprintf(ALWAYS, "%s, 948(1) gpio num %d invalid\n", __func__, gpio);
		return -1;
    }

	ret = hal_i2c_write_reg_data(i2c_handle, addr, &dreg, 1, &dval, 1);
	if (ret) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c write failed\n", __func__, addr);
		return -1;
	}

#ifdef SERDES_DEBUG
	int ret1;
	dval = 0;

	ret1 = hal_i2c_read_reg_data(i2c_handle, addr, &dreg, 1, &dval, 1);
	if (ret1) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c read failed\n", __func__, addr);
	}

	dprintf(ALWAYS, "%s: addr=%#x, read=%#x\n", __func__, addr, dval);
#endif

    return ret;
}

int du90ub941_or_947_gpio_input(void *i2c_handle, uint8_t addr, int gpio)
{
	int ret;
    u8 sreg = 0, sval = 0;

	switch (gpio) {
	case 0:
		sreg = 0x0d;
		break;
	case 1:
	case 2:
		sreg = 0x0e;
		break;
	case 3:
		sreg = 0x0f;
		break;
	case 5:
	case 6:
		sreg = 0x10;
		break;
	case 7:
	case 8:
		sreg = 0x11;
		break;
	default:
		dprintf(ALWAYS, "%s, --gpio num %d invalid\n", __func__, gpio);
		return -1;
    }

	ret = hal_i2c_read_reg_data(i2c_handle, addr, &sreg, 1, &sval, 1);
	if (ret) {
		dprintf(ALWAYS, "%s: addr=%#x, sreg=%#x, i2c read failed\n",
			__func__, addr, sreg);
	}

	switch (gpio) {
	case 0:
	case 1:
	case 3:
		sval &= 0xf0;
		sval |= 0x05;
		break;
	case 2:
		sval &= 0x0f;
		sval |= 0x50;
		break;
	case 5:
	case 7:
		sval &= 0xf0;
		sval |= 0x03;
		break;
	case 6:
	case 8:
		sval &= 0x0f;
		sval |= 0x30;
		break;
	default:
		dprintf(ALWAYS, "%s, ++gpio num %d invalid\n", __func__, gpio);
		return -1;
    }

	ret = hal_i2c_write_reg_data(i2c_handle, addr, &sreg, 1, &sval, 1);
	if (ret) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c write failed\n", __func__, addr);
		return -1;
	}

#ifdef SERDES_DEBUG
	int ret1;
	sval = 0;

	ret1 = hal_i2c_read_reg_data(i2c_handle, addr, &sreg, 1, &sval, 1);
	if (ret1) {
		dprintf(ALWAYS, "%s: addr=%#x, i2c read failed\n", __func__, addr);
	}

	dprintf(ALWAYS, "%s: addr=%#x, read=%#x\n", __func__, addr, sval);
#endif

    return ret;
}

