/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <debug.h>
#include <string.h>
#include <app.h>
#include <lib/console.h>
#include <kernel/thread.h>

#include <reg.h>
#include <res.h>
#include <chip_res.h>
#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0
#if PVT_IN_SAFETY_DOMAIN
#define RES_PVT_SENS_ RES_PVT_SENS_PVT_SENS_SAF
#define RES_PVT_SENS_PHY_ADDR_DEFAULT 0xf02c0000
#else
#define RES_PVT_SENS_ RES_PVT_SENS_PVT_SENS_SEC
#define RES_PVT_SENS_PHY_ADDR_DEFAULT 0xf0880000
#endif

#define PVT_REGISTER_DOUT_ADDR_OFFSET 0x4

#define PVT_PRINT_WAIT_TIME 4 // seconds

#define PVT_PRINT_WAIT_TIME_VALUE (PVT_PRINT_WAIT_TIME*1000)
/* pvt ctrl
*____________________________________________________________________
*|------04------|-----03-----|-----02-----|-----01-----|-----00-----|
*|software ctrl |         p mode          |    v mode  | ctrl mode  |
*/
// ctrl mode 0:from fuse, 1 from register
// v mode 0,1 voltage detect mode
// p mode 0,1
// software ctrl sensor enable 0 disable 1 enable
// v mode and p mode all 0 is t mode(temperature mode)

typedef enum pvt_ctrl_offset {
    PVT_CTRL_CTRL_MODE = 0,
    PVT_CTRL_V_MODE,
    PVT_CTRL_P_MODE_0,
    PVT_CTRL_P_MODE_1,
    PVT_CTRL_SOFTWARE_CTRL,
} pvt_ctrl_offset_t;

typedef enum pvt_mode {
    PVT_MODE_TYPE_TEMP = 0,
    PVT_MODE_TYPE_VOL,
    PVT_MODE_TYPE_P,
} pvt_mode_t;

static int pvt_out_print = 1;

//init ctrl and ctrl register
void pvt_ctrl_mode_init(paddr_t phy_addr)
{
    uint32_t data = 0;

    LTRACEF("pvt_ctrl_mode_init phy_addr = 0x%x\n", (uint32_t)phy_addr);

    data = readl(phy_addr);

    LTRACEF("pvt_ctrl_mode_init read phy_addr = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);

    // set pvt ctrl mode: ctrl from register
    data = data | (1 << PVT_CTRL_CTRL_MODE);
    writel(data, phy_addr);

    data = readl(phy_addr);
    LTRACEF("after set pvt ctrl mode read phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);

    // set pvt ctrl register

    data = data | (1 << PVT_CTRL_SOFTWARE_CTRL) ;
    writel(data, phy_addr);

    data = readl(phy_addr);
    LTRACEF("after set pvt ctrl register read phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);
}

//set pvt mode
void pvt_set_mode(paddr_t phy_addr, uint32_t mode_type)
{
    uint32_t data = 0;

    LTRACEF("pvt_ctrl_mode_init phy_addr = 0x%x, mode_type =%d\n", (uint32_t)phy_addr, mode_type);

    data = readl(phy_addr);

    LTRACEF("pvt_set_mode read phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);

    switch (mode_type) {
        case PVT_MODE_TYPE_TEMP:
            data = (data & (~0xE)); //bit1~3 set 0
            break;

        case PVT_MODE_TYPE_VOL:
            data = (data | (0x2)); //bit1 set 1
            break;

        case PVT_MODE_TYPE_P:
            data = ((data & (~0x6)) | (0x4)); //bit2~3 set xxx
            break;

        default:
            data = (data & (~0xE)); //bit1~3 set 0
            break;
    }

    // set pvt mode:
    writel(data, phy_addr);

    if (LOCAL_TRACE) {
        data = readl(phy_addr);
        LTRACEF("after set pvt mode read phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);
    }
}

//read voltage and temperature value
void pvt_read_vt_value(paddr_t phy_addr)
{
    int i = 0;
    uint32_t data = 0;
    float temp_data = 1;
    float final_data = 0;
    uint32_t dout_is_voltage = 0;
    uint32_t dout_is_temperature = 0;
    paddr_t phy_addr_temp = 0;
    float a4_t[5];
    float a2_v[2];

    a4_t[4] = 1.6034E-11;
    a4_t[3] = 1.5608E-08;
    a4_t[2] = -1.5089E-04;
    a4_t[1] = 3.3408E-01;
    a4_t[0] = -6.2861E+01;

    a2_v[1] = 5.9677E-04;
    a2_v[0] = 5.1106E-01;

    if (LOCAL_TRACE) {
        for (i = 0; i < 5; i++) {
            LTRACEF("a4_t%d = %f\n", i, a4_t[i]);
        }

        hexdump((char*)&a4_t[0], 20);
    }

    if (LOCAL_TRACE) {
        LTRACEF("pvt register phy_addr0x%x dump=\n",(uint32_t)phy_addr);
        hexdump((char*)phy_addr, 20);
    }

    phy_addr_temp = phy_addr + PVT_REGISTER_DOUT_ADDR_OFFSET;

    LTRACEF("enter phy_addr_temp = 0x%x\n", (uint32_t)phy_addr_temp);

    data = readl(phy_addr);

    if (data & 0x2) {
        dout_is_voltage = 1;
    }
    else {
        dout_is_voltage = 0;
    }

    if (data & 0xe) {
        dout_is_temperature = 0;
    }
    else {
        dout_is_temperature = 1;
    }

    LTRACEF("dout_is_voltage = %d, dout_is_temperature = %d\n", dout_is_voltage, dout_is_temperature);

    data = readl(phy_addr_temp);
    LTRACEF("read dout phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)phy_addr_temp, data);

    // bit10~1 is dout value
    // bit0 is pvt dout valid indicate 0 is invalid 1 is valid
    data = (data & 0x7fe) >> 1;

    // y = a4*x^4+a3*x^3+a2*x^2+a1*x^1+a0
    if (dout_is_temperature) {
        for (i = 0; i < 5; i++) {
            final_data += a4_t[i] * temp_data;
            temp_data = data * temp_data;
        }

        // LTRACEF("temperature value = %f\n", final_data);
        dprintf(CRITICAL, "temperature value = %f\n", final_data);
    }

    //y =a1*x+a0
    if (dout_is_voltage) {
        final_data = data * a2_v[1] + a2_v[0];
        //LTRACEF("voltage value = %f\n", final_data);
        dprintf(CRITICAL, "voltage value = %f\n", final_data);
    }
}

int pvt_ctrl(void)
{

    int ret = 0;
    int loop_value = 0;
    paddr_t phy_addr = 0;
    int32_t resid = 0;
    int32_t offset_index = 0;

    resid = RES_PVT_SENS_;
    ret = res_get_info_by_id(resid, &phy_addr, &offset_index);

    if (ret == -1) {
        LTRACEF("pvt resouce error, set phy_addr to default\n");
        phy_addr = RES_PVT_SENS_PHY_ADDR_DEFAULT;
        pvt_ctrl_mode_init(phy_addr);
    }
    else {
        LTRACEF("pvt resouce get phy_addr =0x%x\n", (uint32_t)phy_addr);
        pvt_ctrl_mode_init(phy_addr);
    }

    //read pvt value
    while (1) {
        if(pvt_out_print == 1){
            if (loop_value == 0) {
                pvt_set_mode(phy_addr, PVT_MODE_TYPE_TEMP);
                loop_value = 1;
            }
            else {
                pvt_set_mode(phy_addr, PVT_MODE_TYPE_VOL);
                loop_value = 0;
            }

            thread_sleep(PVT_PRINT_WAIT_TIME_VALUE);
            pvt_read_vt_value(phy_addr);
        }else{
            thread_sleep(PVT_PRINT_WAIT_TIME_VALUE);
        }
    }

    return ret;
}

int pvt_test(void)
{
    int ret = 0;

    //pvt setting
    ret = pvt_ctrl();

    if (ret) {
        dprintf(CRITICAL, "pvt_ctrl fail.\n");
        return ret;
    }

    return ret;
}

int pvt_test_outprint(int argc, const cmd_args *argv)
{
    unsigned long is_out;

    if (argc != 2) {
        dprintf(CRITICAL, "input argv error\n");
        return -1;
    }

    is_out = argv[1].u;

    if(is_out == 1){
        pvt_out_print = 1;
    }else if(is_out == 0){
        pvt_out_print = 0;
    }
    return 0;
}
void pvt_entry(const struct app_descriptor * app_desp, void *args)
{
    LTRACEF("pvt_entry start\n");
    //pvt setting
    pvt_ctrl();

}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("pvt_test", "read pvt value", (console_cmd)&pvt_test)
STATIC_COMMAND("pvt_on", "onoff pvt out print,pvt_on 1:out print ,pvt_on 0:close out print,", (console_cmd)&pvt_test_outprint)
STATIC_COMMAND_END(pvt_test);

#endif

APP_START(pvt_entry)
.flags = 0,
.entry = pvt_entry,
APP_END
