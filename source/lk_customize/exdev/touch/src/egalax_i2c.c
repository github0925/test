#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <thread.h>
#include <event.h>
#include <i2c_hal.h>
#include <hal_port.h>
#include <safe_ts.h>

#include "touch_driver.h"
#include "boardinfo_hwid_usr.h"
#include "hal_dio.h"
#include "gpioirq.h"
#include "serdes_9xx.h"

#define MAX_EVENTS		600
#define MAX_I2C_LEN		64
#define FIFO_SIZE		8192 //(PAGE_SIZE*2)
#define MAX_SUPPORT_POINT	16
#define REPORTID_MOUSE		0x01
#define REPORTID_VENDOR		0x03
#define REPORTID_MTOUCH		0x06
#define MAX_RESOLUTION		4095
#define REPORTID_MTOUCH2	0x18

struct tagMTContacts {
	unsigned char ID;
	signed char Status;
	unsigned short X;
	unsigned short Y;
};

/* Define vendor spcific touchscreen data */
struct egalax_ts_data {
	u16 id;
	u16 version;
	u16 vendor;
	u16 instance;
	u16 product;
	int cfg_len;
	const char *cfg_data;
	unsigned int dev_irq;
	void *i2c_handle;
	event_t event;
	const char *name;
	const struct ts_board_config *conf;
	struct safe_ts_device *safe_ts_dev;
};

static int egalax_dbg_flag = 0;
static unsigned char input_report_buf[MAX_I2C_LEN+2];
static struct tagMTContacts pContactBuf[MAX_SUPPORT_POINT];

static int egalax_i2c_read(struct egalax_ts_data *egalax, u8 *buf, int len)
{
	int ret = 0;
	ret = hal_i2c_read_reg_data(egalax->i2c_handle, egalax->conf->i2c_addr,
	                    buf, 2, buf + 2, len - 2);

	if (ret < 0) {
		dprintf(CRITICAL, "%s: read error, ret=%d.\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int egalax_i2c_write(struct egalax_ts_data *egalax, u8 *buf, int len)
{
	int ret = 0;
	ret = hal_i2c_write_reg_data(egalax->i2c_handle, egalax->conf->i2c_addr,
	                         buf, 2, buf + 2, len - 2);

	if (ret < 0) {
		dprintf(CRITICAL, "%s: transmit error, ret=%d.\n", __func__, ret);
		return ret;
	}

	return ret;
}

static int egalax_read_version(struct egalax_ts_data *egalax)
{
	egalax->name = "eGalax_Touch_Screen";
	egalax->vendor = 0x0EEF;
	egalax->product = 0x0020;
	egalax->version = 0x0001;

	return 0;
}

static int egalax_reset_device(struct egalax_ts_data *egalax)
{
	if (egalax->conf->serdes_config.enable) {
		uint16_t ser_addr = egalax->conf->serdes_config.ser_addr;
		uint16_t des_addr = egalax->conf->serdes_config.des_addr;
		uint16_t irq_channel = egalax->conf->serdes_config.irq_channel;
		uint16_t reset_channel = egalax->conf->serdes_config.reset_channel;

		if (du90ub948_gpio_output(egalax->i2c_handle, des_addr, reset_channel, 1))
		{
			dprintf(ALWAYS, "du90ub948_gpio_output failed\n");
			return -1;
		}

		thread_sleep(10);
		du90ub948_gpio_output(egalax->i2c_handle, des_addr, reset_channel, 0);
		thread_sleep(10);
		du90ub948_gpio_output(egalax->i2c_handle, des_addr, reset_channel, 1);
		thread_sleep(100);
		du90ub948_gpio_input(egalax->i2c_handle, des_addr, irq_channel);
		du90ub941_or_947_gpio_input(egalax->i2c_handle, ser_addr, irq_channel);
	}

	return 0;
}

#define MAX_POINT_PER_PACKET	5
#define POINT_STRUCT_SIZE	10
static int TotalPtsCnt=0, RecvPtsCnt=0;
static void ProcessReport( struct egalax_ts_data *egalax, unsigned char *buf)
{
	int len;
	unsigned char i, j, index=0, cnt_down=0, cnt_up=0, shift=0;
	unsigned char status=0;
	unsigned short contactID=0, x=0, y=0;
	struct touch_report_data report_data;

	if(TotalPtsCnt<=0)
	{
		if(buf[1]==0 || buf[1]>MAX_SUPPORT_POINT)
		{
			dprintf(CRITICAL, " NumsofContacts mismatch, skip packet\n");
			return;
		}

		TotalPtsCnt = buf[1];

		if(TotalPtsCnt > 10)
			TotalPtsCnt = 10;

		RecvPtsCnt = 0;
	}
	else if(buf[1] > 0)
	{
		TotalPtsCnt = RecvPtsCnt = 0;
		dprintf(CRITICAL, " NumsofContacts mismatch, skip packet\n");
		return;
	}

	while(index < MAX_POINT_PER_PACKET)
	{
		shift = index * POINT_STRUCT_SIZE + 2;
		status = buf[shift] & 0x01;
		contactID = buf[shift+1];
		x = ((buf[shift+3]<<8) + buf[shift+2]);
		y = ((buf[shift+5]<<8) + buf[shift+4]);

		if( buf[0]==REPORTID_MTOUCH2 )
		{
			x >>= 2;
			y >>= 2;
		}

		if( contactID>=MAX_SUPPORT_POINT )
		{
			TotalPtsCnt = RecvPtsCnt = 0;
			dprintf(CRITICAL, " Get error ContactID.\n");
			return;
		}

		if (egalax_dbg_flag)
			dprintf(ALWAYS, " Get Point[%d] Update: Status=%d X=%d Y=%d, TotalPtsCnt:%d\n", contactID, status, x, y, TotalPtsCnt);

	#ifdef _SWITCH_XY
		short tmp = x;
		x = y;
		y = tmp;
	#endif
	#ifdef _CONVERT_X
		x = MAX_RESOLUTION-x;
	#endif

	#ifdef _CONVERT_Y
		y = MAX_RESOLUTION-y;
	#endif

		pContactBuf[RecvPtsCnt].ID = contactID;
		pContactBuf[RecvPtsCnt].Status = status;
		pContactBuf[RecvPtsCnt].X = x;
		pContactBuf[RecvPtsCnt].Y = y;

		RecvPtsCnt++;
		index++;

		if(RecvPtsCnt == TotalPtsCnt)
		{
			report_data.key_value = 0;
			report_data.touch_num = RecvPtsCnt;
			j = 0;

			for(i=0; i<RecvPtsCnt; i++)
			{
				if(pContactBuf[i].Status){
					report_data.coord_data[j].id = pContactBuf[i].ID;
					report_data.coord_data[j].x = pContactBuf[i].X;
					report_data.coord_data[j].y = pContactBuf[i].Y;
					report_data.coord_data[j].w = 0;
					report_data.coord_data[j].x =
						report_data.coord_data[j].x * egalax->conf->coord_config.x_max / 4096;
					report_data.coord_data[j].y =
						report_data.coord_data[j].y * egalax->conf->coord_config.y_max / 4096;
					j++;
					cnt_down++;
				}else{
					report_data.touch_num-- ;
					cnt_up++;
					continue;
				}
			}

			if (egalax_dbg_flag)
			{
				for(j=0; j<report_data.touch_num; j++)
					dprintf(ALWAYS, "ID:%d, x:%d, Y:%d\n", report_data.coord_data[j].id, report_data.coord_data[j].x, report_data.coord_data[j].y);
				dprintf(ALWAYS, "RecvPtsCnt:%d, touch_num:%d\n", RecvPtsCnt, report_data.touch_num);
			}

			len = 2 + report_data.touch_num * TS_COORD_METADATA_SIZE;
			safe_ts_report_data(egalax->safe_ts_dev, &report_data, len);

			TotalPtsCnt = RecvPtsCnt = 0;

			return;
		}
	}
}

static int egalax_ts_read_input_report(struct egalax_ts_data *egalax, u8 *data)
{
	int ret = 0;
	int frameLen = 0;
	unsigned char* pdata = NULL;

	if( egalax_i2c_read(egalax, input_report_buf, MAX_I2C_LEN+2) < 0)
	{
		dprintf(ALWAYS, " I2C read input report fail!\n");
		ret = -1;
		return ret;
	}

	if(egalax_dbg_flag)
	{
		int i;
		for(i=1; i<=MAX_I2C_LEN+2; i++)
		{
			dprintf(ALWAYS, "%d ", input_report_buf[i-1]);
			if(i%10 == 0)
				dprintf(ALWAYS,"\n");
		}
		dprintf(ALWAYS,"\n");
	}

	pdata = input_report_buf + 2;

	frameLen = pdata[0] + (pdata[1]<<8);
	if (egalax_dbg_flag)
	{
		dprintf(ALWAYS, " I2C read data with Len=%d\n", frameLen);
	}

	if(frameLen == 0)
	{
		dprintf(ALWAYS, "Device reset\n");
		return -1;
	}

	switch(pdata[2])
	{
		case REPORTID_MTOUCH:
		case REPORTID_MTOUCH2:
			ProcessReport(egalax, pdata+2);
			ret = 0;
			break;
		default:
			dprintf(ALWAYS, " I2C read error data with hedaer=%d\n", pdata[2]);
			ret = -1;
			break;
	}

    return ret;
}

static int egalax_ts_work_func(void *arg)
{
	struct egalax_ts_data *egalax = arg;
	u8 pdata[MAX_I2C_LEN+2] = {0,};

	while (1)
	{
		event_wait(&egalax->event);
		egalax_ts_read_input_report(egalax, pdata);

		unmask_gpio_interrupt(egalax->dev_irq);
	}

	return 0;
}

static enum handler_return egalax_irq_handler(void *arg)
{
	struct egalax_ts_data *egalax = arg;

	mask_gpio_interrupt(egalax->dev_irq);
	event_signal(&egalax->event, false);

	return INT_RESCHEDULE;
}

static int egalax_config_device(struct egalax_ts_data *egalax)
{
	event_init(&egalax->event, false, EVENT_FLAG_AUTOUNSIGNAL);
	thread_t *tp_thread = thread_create("egalax_thread", egalax_ts_work_func,
	                                    egalax, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
	thread_detach_and_resume(tp_thread);
	register_gpio_int_handler(egalax->dev_irq, IRQ_TYPE_LEVEL_LOW, egalax_irq_handler, egalax);
	unmask_gpio_interrupt(egalax->dev_irq);

	return 0;
}

static int egalax_set_inited(void *arg)
{
	return 0;
}

static int egalax_probe_device(const struct ts_board_config *conf)
{
	int ret;
	struct egalax_ts_data *egalax = NULL;

	egalax = (struct egalax_ts_data *)malloc(sizeof(struct egalax_ts_data));
	if (!egalax) {
		dprintf(CRITICAL, "%s, alloc mem fail\n", __func__);
		return -1;
	}

	memset(egalax, 0, sizeof(struct egalax_ts_data));
	egalax->conf = conf;
	egalax->instance = egalax->conf->ts_domain_support;
	egalax->dev_irq = egalax->conf->irq_pin.pin_num;

	if (!hal_i2c_creat_handle(&egalax->i2c_handle, egalax->conf->res_id)) {
		dprintf(CRITICAL, "%s, i2c handle fail,instance=%#x\n", __func__,
		        egalax->instance);
		goto err;
	}

	if (egalax_dbg_flag)
	{
		dprintf(ALWAYS, "serdes_config, Enable:%d, ser_addr:0x%x, des_addr:0x%x\n",
			egalax->conf->serdes_config.enable, egalax->conf->serdes_config.ser_addr, egalax->conf->serdes_config.des_addr);
	}

	if (egalax->conf->serdes_config.enable) {
		ret = du90ub941_or_947_enable_port(egalax->i2c_handle,
		egalax->conf->serdes_config.ser_addr, egalax->conf->serdes_config.serdes_type);
		if (ret) {
			dprintf(ALWAYS, "%s, enable port fail,instance=%#x\n", __func__,
			    egalax->instance);
			goto err1;
		}

		ret = du90ub941_or_947_enable_i2c_passthrough(egalax->i2c_handle,
		egalax->conf->serdes_config.ser_addr);
		if (ret) {
			dprintf(ALWAYS, "%s, enable i2c pass fail,instance=%#x\n", __func__,
			    egalax->instance);
			goto err1;
		}
	}

	int count = 0;

	do {
		ret = egalax_reset_device(egalax);

		if (ret > 0 && !egalax_read_version(egalax))
			break;
	} while (++count < 3);

	if (count >= 3) {
		dprintf(ALWAYS, "%s: read egalax fail, instance=%#x\n", __func__,
		egalax->instance);
		goto err1;
	}

	ret = egalax_config_device(egalax);
	if (ret) {
		dprintf(ALWAYS, "%s:config device fail, instance=%#x\n", __func__,
		egalax->instance);
		goto err1;
	}

	egalax->safe_ts_dev = safe_ts_alloc_device();
	if (!egalax->safe_ts_dev) {
		dprintf(ALWAYS, "%s: safe_ts_alloc_device fail, instance=%#x\n",
		__func__, egalax->instance);
		goto err;
	}

	egalax->safe_ts_dev->instance = egalax->conf->ts_domain_support;
	egalax->safe_ts_dev->screen_id = egalax->conf->screen_id;
	egalax->safe_ts_dev->vinfo.id = egalax->id;
	egalax->safe_ts_dev->vinfo.version = egalax->version;
	egalax->safe_ts_dev->vinfo.vendor = egalax->vendor;
	egalax->safe_ts_dev->vinfo.name = egalax->conf->device_name;
	egalax->safe_ts_dev->cinfo.max_touch_num = egalax->conf->coord_config.max_touch_num;
	egalax->safe_ts_dev->cinfo.swapped_x_y = egalax->conf->coord_config.swapped_x_y;
	egalax->safe_ts_dev->cinfo.inverted_x = egalax->conf->coord_config.inverted_x;
	egalax->safe_ts_dev->cinfo.inverted_y = egalax->conf->coord_config.inverted_y;
	egalax->safe_ts_dev->set_inited = egalax_set_inited;
	egalax->safe_ts_dev->vendor_priv = egalax;

	ret = safe_ts_register_device(egalax->safe_ts_dev);
	if (ret) {
		safe_ts_delete_device(egalax->safe_ts_dev);
		dprintf(ALWAYS, "%s: safe_ts_register_device fail, instance=%#x\n",
		__func__, egalax->instance);
		goto err;
	}

	return 0;

err1:
	hal_i2c_release_handle(egalax->i2c_handle);
err:
	free(egalax);
	return -1;
}


static struct touch_driver egalax_driver = {
	"egalax_i2c",
	egalax_probe_device,
};

register_touch_driver(egalax_driver);

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
static int egalax_dbg_en(int argc, const cmd_args *argv)
{
	egalax_dbg_flag = atoui(argv[1].str);
	dprintf(ALWAYS, "egalax_dbg_flag=%d\n", egalax_dbg_flag);
	return 0;
}

STATIC_COMMAND_START STATIC_COMMAND("egalax_dbg_en", "egalax_dbg_en [0/1]",
                                    (console_cmd)&egalax_dbg_en)
STATIC_COMMAND_END(egalax_i2c_touch);
#endif

