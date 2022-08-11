//*****************************************************************************
//
// pvt_hal.c - the pvt hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <res.h>
#include <pvt_hal.h>
#include <trace.h>

#include <debug.h>
#include <string.h>
#include <kernel/thread.h>

#include <reg.h>
#include <chip_res.h>
#include <irq.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#define RES_PVT_SENS_SAF RES_PVT_SENS_PVT_SENS_SAF
#define RES_PVT_SENS_PHY_ADDR_DEFAULT_SAF 0xf02c0000

#define RES_PVT_SENS_SEC RES_PVT_SENS_PVT_SENS_SEC
#define RES_PVT_SENS_PHY_ADDR_DEFAULT_SEC 0xf0880000


#define PVT_REGISTER_DOUT_ADDR_OFFSET 0x4
#define PVT_REGISTER_HYST_H_OFFSET 0x8
#define PVT_REGISTER_HYST_L_OFFSET 0xC
#define PVT_REGISTER_HYST_R_OFFSET 0x10
#define PVT_REGISTER_HYST_F_OFFSET 0x14
#define PVT_REGISTER_HYST_TIME_OFFSET 0x18
#define PVT_REGISTER_INT_EN_OFFSET 0x1C
#define PVT_REGISTER_INT_STATUS_OFFSET 0x20
#define PVT_REGISTER_INT_CLR_OFFSET 0x24
#define PVT_REGISTER_HYST_H1_OFFSET 0x28
#define PVT_REGISTER_HYST_L1_OFFSET 0x2C
#define PVT_REGISTER_HYST_R1_OFFSET 0x30
#define PVT_REGISTER_HYST_F1_OFFSET 0x34
#define PVT_REGISTER_HYST_TIME1_OFFSET 0x38
#define PVT_REGISTER_INT_EN1_OFFSET 0x3C
#define PVT_REGISTER_INT_STATUS1_OFFSET 0x40
#define PVT_REGISTER_INT_CLR1_OFFSET 0x44

#define PVT_PRINT_WAIT_TIME 3 // seconds

#define PVT_PRINT_WAIT_TIME_VALUE (PVT_PRINT_WAIT_TIME*1000)
/* pvt ctrl
*____________________________________________________________________
*|------04------|-----03-----|-----02-----|-----01-----|-----00-----|
*|software ctrl |         p mode          |    v mode  | ctrl mode  |
*/
// ctrl mode : 0 from fuse; 1 from register
// v mode : 1 voltage detect mode
// p mode : 10 lvt ; 01 ulvt ; 11 svt
// software ctrl sensor enable: 0 disable ; 1 enable
// v mode and p mode all 0 is t mode(temperature mode)

#define PVT_CTRL_CTRL_MODE_SHEFT 0
#define PVT_CTRL_V_MODE_SHEFT 1
#define PVT_CTRL_P_MODE_0_SHEFT 2
#define PVT_CTRL_P_MODE_1_SHEFT 3
#define PVT_CTRL_SOFTWARE_CTRL_SHEFT 4

#define PVT_MODE_TYPE_TEMP 0
#define PVT_MODE_TYPE_VOL 1
#define PVT_MODE_TYPE_P 2

#define PVT_DEVICE_TYPE_ULVT 1
#define PVT_DEVICE_TYPE_LVT 2
#define PVT_DEVICE_TYPE_SVT 3

#define PVT_OUT_TYPE_OFF 0
#define PVT_OUT_TYPE_ON_ALL 1
#define PVT_OUT_TYPE_ON_ONE_TYPE 2

#define MAX_PVT_DEVICE_NUM 2
#define MAX_PVT_ALARM_NUM 2

static pvt_instance_t g_pvtInstance[MAX_PVT_DEVICE_NUM] = {{RES_PVT_SENS_SAF,RES_PVT_SENS_PHY_ADDR_DEFAULT_SAF,0,0,0},{RES_PVT_SENS_SEC,RES_PVT_SENS_PHY_ADDR_DEFAULT_SEC,0,0,0}};

uint32_t pvt_temp_to_dout(float temp_value){

    int i = 0;
    float temp_data = 1;
    float a4_t[5];
    float final_data = 0;
    a4_t[4] = 1.2569E-08;
    a4_t[3] = 2.4759E-05;
    a4_t[2] = 6.6309E-03;
    a4_t[1] = 3.6384E+00;
    a4_t[0] = 2.0705E+02;

    // y = a4*x^4+a3*x^3+a2*x^2+a1*x^1+a0

    for (i = 0; i < 5; i++) {
        final_data += a4_t[i] * temp_data;
        temp_data = temp_value * temp_data;
    }

    LTRACEF("pvt_temp_to_dout value = %f\n", final_data);

    return (uint32_t)final_data;
}

//init ctrl and ctrl register
void pvt_ctrl_mode_init(paddr_t phy_addr)
{
    uint32_t data = 0;

    LTRACEF("pvt_ctrl_mode_init phy_addr = 0x%x\n", (uint32_t)phy_addr);

    data = readl(phy_addr);

    LTRACEF("pvt_ctrl_mode_init read phy_addr = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);

    // set pvt ctrl mode: ctrl from register
    data = data | (1 << PVT_CTRL_CTRL_MODE_SHEFT);
    writel(data, phy_addr);

    data = readl(phy_addr);
    LTRACEF("after set pvt ctrl mode read phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);

    // set pvt ctrl register

    data = data | (1 << PVT_CTRL_SOFTWARE_CTRL_SHEFT) ;
    writel(data, phy_addr);

    data = readl(phy_addr);
    LTRACEF("after set pvt ctrl register read phy_addr_temp = 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);
}

//set pvt mode: return 1, no need wait, return 0, mode change, need wait.
int pvt_set_mode(paddr_t phy_addr, uint32_t mode_type, uint32_t device_type)
{
    uint32_t data = 0;
    uint32_t data_temp = 0;
    LTRACEF("pvt_set_mode phy_addr = 0x%x, mode_type =%d, device_type=%d\n", (uint32_t)phy_addr, mode_type, device_type);

    data = readl(phy_addr);

    LTRACEF("before set pvt mode phy_addr 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);

    switch (mode_type) {
        case PVT_MODE_TYPE_TEMP:
            data_temp = (data & (~0xE)); //bit1~3 set 0
            break;

        case PVT_MODE_TYPE_VOL:
            data_temp = ((data & (~0xE)) | (0x2)); //bit1 set 1
            break;

        case PVT_MODE_TYPE_P:
            data_temp = ((data & (~0xE)) | ((device_type << PVT_CTRL_P_MODE_0_SHEFT)&0xC)); //bit1 set 0,bit2~3 set xxx:1 ulvt,2 lvt, 3 svt
            break;

        default:
            data_temp = (data & (~0xE)); //bit1~3 set 0
            break;
    }

    if(data_temp == data){
        LTRACEF("pvt_set_mode no need write reg\n");
        return 1;
    }

    // set pvt mode:
    writel(data_temp, phy_addr);

    if (LOCAL_TRACE) {
        data = readl(phy_addr);
        LTRACEF("after set pvt mode  phy_addr 0x%x, data = 0x%x\n", (uint32_t)(phy_addr), data);
    }
    //wait 1 ms , 308 microsecond is ok. 1.2mHz, 370
    spin(1000);
    return 0;
}

void pvt_read_pvt_value(paddr_t phy_addr, pvt_out_data_t * out_data)
{
    int i = 0;
    uint32_t data = 0;
    float temp_data = 1;
    float final_data = 0;
    uint32_t dout_is_voltage = 0;
    uint32_t dout_is_temperature = 0;
    uint32_t dout_is_process = 0;
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

    LTRACEF("pvt_read_pvt_value enter phy_addr = 0x%x\n", (uint32_t)phy_addr_temp);

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

    if (data & 0xc) {
        dout_is_process = 1;
    }
    else {
        dout_is_process = 0;
    }

    LTRACEF("dout_is_voltage = %d, dout_is_temperature = %d,dout_is_process = %d\n", dout_is_voltage, dout_is_temperature,dout_is_process);

    data = readl(phy_addr_temp);

    LTRACEF("read dout phy_addr 0x%x, data = 0x%x\n", (uint32_t)phy_addr_temp, data);

    // bit10~1 is dout value
    // bit0 is pvt dout valid indicate 0 is invalid 1 is valid
    data = (data & 0x7fe) >> 1;

    // y = a4*x^4+a3*x^3+a2*x^2+a1*x^1+a0
    if (dout_is_temperature) {
        for (i = 0; i < 5; i++) {
            final_data += a4_t[i] * temp_data;
            temp_data = data * temp_data;
        }

        out_data->temp_data = final_data;

        LTRACEF("temperature value = %f\n", final_data);
    }

    //y =a1*x+a0
    if (dout_is_voltage) {
        final_data = data * a2_v[1] + a2_v[0];

        out_data->voltage_data = final_data;

        LTRACEF("voltage value = %f\n", final_data);
    }

    if(dout_is_process) {

        out_data->process_data = data;

        LTRACEF("process value = 0x%x\n", data);

    }
}

static pvt_instance_t*  pvt_get_instance(pvt_res_id_t res_id)
{

    if(res_id >= MAX_PVT_DEVICE_NUM){
        return NULL;
    }

    if(g_pvtInstance[res_id].occupied == 0){
        g_pvtInstance[res_id].occupied = 1;
        return &g_pvtInstance[res_id];
    }else{
        return NULL;
    }

}

static void  pvt_release_instance(pvt_instance_t* pvt_instance)
{
    if(pvt_instance != NULL){
        pvt_instance->occupied = 0;
    }
}

int pvt_init(pvt_res_id_t res_id)
{

    int ret = 0;
    paddr_t phy_addr = 0;
    int32_t offset_index = 0;

    if(res_id >= MAX_PVT_DEVICE_NUM){
        return ret;
    }

    if(g_pvtInstance[res_id].is_init == 0){

        ret = res_get_info_by_id(g_pvtInstance[res_id].res_id, &phy_addr, &offset_index);

        if (ret == -1) {
            LTRACEF("pvt_init resouce error, set phy_addr to default 0x%x\n",(uint32_t)(g_pvtInstance[res_id].phy_addr));
            pvt_ctrl_mode_init(g_pvtInstance[res_id].phy_addr);
        }
        else {
            LTRACEF("pvt_init resouce get phy_addr =0x%x\n", (uint32_t)phy_addr);
            pvt_ctrl_mode_init(phy_addr);
            g_pvtInstance[res_id].phy_addr = phy_addr;
        }
        g_pvtInstance[res_id].is_init = 1;
        ret = 1;
    }else{
        ret = 2;
    }

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_set_hyst_h.
//!
//! @param res_id input, pvt res id
//! @param hyst_h_thresh_h input, h temp, when the v/t is larger than h, alarm will be generated
//! @param hyst_h_thresh_l input, l temp, alarm will not been cleaned till v/t is smaller than l
//! @param index input, alarm index must be 0 or 1
//! This function is for h temp alarm
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_set_hyst_h(pvt_res_id_t res_id, float hyst_h_thresh_h, float hyst_h_thresh_l, uint32_t index){
    int ret = 0;
    uint32_t dout_h_temp = 0;
    uint32_t dout_l_temp = 0;
    paddr_t phy_addr_temp = 0;
    pvt_instance_t* pvt_inst;
    spin_lock_saved_state_t states;
    uint32_t hyst_h_offset[MAX_PVT_ALARM_NUM] = {PVT_REGISTER_HYST_H_OFFSET,PVT_REGISTER_HYST_H1_OFFSET};

    if((res_id >= MAX_PVT_DEVICE_NUM)||(index>=MAX_PVT_ALARM_NUM)){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){

        dout_h_temp = pvt_temp_to_dout(hyst_h_thresh_h);
        dout_l_temp = pvt_temp_to_dout(hyst_h_thresh_l);

        LTRACEF("hal_pvt_set_hyst_h h= %f, h_out=0x%x, l= %f, l_out=0x%x\n", hyst_h_thresh_h, dout_h_temp, hyst_h_thresh_l , dout_l_temp);

        phy_addr_temp = pvt_inst->phy_addr+hyst_h_offset[index];

        dout_h_temp = (dout_h_temp&0x3ff)|((dout_l_temp&0x3ff)<<10);

        writel(dout_h_temp, phy_addr_temp);

        LTRACEF("hal_pvt_set_hyst_h phy_addr =0x%x, hyst_h=%x\n", (uint32_t)phy_addr_temp, dout_h_temp);

        pvt_release_instance(pvt_inst);
        ret = 1;
    }

    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_set_hyst_l.
//!
//! @param res_id input, pvt res id
//! @param hyst_l_thresh_l input, h temp, when the v/t is small than l, alarm will be generated
//! @param hyst_l_thresh_h input, l temp, alarm will not been cleaned till v/t is larger than h
//! @param index input, alarm index must be 0 or 1
//! This function is for l temp alarm
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_set_hyst_l(pvt_res_id_t res_id, float hyst_l_thresh_l, float hyst_l_thresh_h, uint32_t index){
    int ret = 0;
    uint32_t dout_h_temp = 0;
    uint32_t dout_l_temp = 0;
    paddr_t phy_addr_temp = 0;
    pvt_instance_t* pvt_inst;
    spin_lock_saved_state_t states;
    uint32_t hyst_l_offset[MAX_PVT_ALARM_NUM] = {PVT_REGISTER_HYST_L_OFFSET,PVT_REGISTER_HYST_L1_OFFSET};

    if((res_id >= MAX_PVT_DEVICE_NUM)||(index>=MAX_PVT_ALARM_NUM)){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){

        dout_h_temp = pvt_temp_to_dout(hyst_l_thresh_h);
        dout_l_temp = pvt_temp_to_dout(hyst_l_thresh_l);

        LTRACEF("hal_pvt_set_hyst_h h= %f, h_out=0x%x, l= %f, l_out=0x%x\n", hyst_l_thresh_h, dout_h_temp, hyst_l_thresh_l , dout_l_temp);

        phy_addr_temp = pvt_inst->phy_addr+hyst_l_offset[index];

        dout_h_temp = (dout_h_temp&0x3ff)|((dout_l_temp&0x3ff)<<10);

        writel(dout_h_temp, phy_addr_temp);

        LTRACEF("hal_pvt_set_hyst_h phy_addr =0x%x, hyst_h=%x\n", (uint32_t)phy_addr_temp, dout_h_temp);

        pvt_release_instance(pvt_inst);
        ret = 1;
    }

    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_int_en.
//!
//! @param res_id input, pvt res id
//! @param int_type input, alarm type
//! @param int_en input, 0 disable int, 1 enable int
//! @param index input, alarm index must be 0 or 1
//! This function is for enable or disable alarm
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_int_en(pvt_res_id_t res_id, pvt_int_type_t int_type, uint32_t int_en, uint32_t index){
    int ret = 0;
    uint32_t data = 0;
    paddr_t phy_addr_temp = 0;
    pvt_instance_t* pvt_inst;
    spin_lock_saved_state_t states;
    uint32_t hyst_int_offset[MAX_PVT_ALARM_NUM] = {PVT_REGISTER_INT_EN_OFFSET,PVT_REGISTER_INT_EN1_OFFSET};

    if((res_id >= MAX_PVT_DEVICE_NUM)||(index>=MAX_PVT_ALARM_NUM)){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){

        //enable/disable pvt int
        phy_addr_temp = pvt_inst->phy_addr+hyst_int_offset[index];
        data = readl(phy_addr_temp);

        if(int_en == 0){
            data = data&(~(0x1<<int_type));
        }else{
            data = (data&(~(0x1<<int_type)))|(0x1<<int_type);
        }

        writel(data, phy_addr_temp);
        LTRACEF("hal_pvt_int_en phy_addr =0x%x, int_en=%x\n", (uint32_t)phy_addr_temp, data);

        pvt_release_instance(pvt_inst);
        ret = 1;
    }

    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_get_int_status.
//!
//! @param res_id input, pvt res id
//! @param index input, alarm index must be 0 or 1
//! @param int_status output, int status, bit 0: hyst_high, bit 1: hyst_low, bit 2:hyst_r ,bit 3:hyst_f
//! This function is for get int status,
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_int_status(pvt_res_id_t res_id, uint32_t index, uint32_t * int_status){
    int ret = 0;
    paddr_t phy_addr_temp = 0;
    pvt_instance_t* pvt_inst;
    spin_lock_saved_state_t states;
    uint32_t hyst_status_offset[MAX_PVT_ALARM_NUM] = {PVT_REGISTER_INT_STATUS_OFFSET,PVT_REGISTER_INT_STATUS1_OFFSET};

    if((res_id >= MAX_PVT_DEVICE_NUM)||(index>=MAX_PVT_ALARM_NUM)){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){

        //enable/disable pvt int
        phy_addr_temp = pvt_inst->phy_addr+hyst_status_offset[index];
        *int_status = readl(phy_addr_temp);

        LTRACEF("hal_pvt_get_int_status phy_addr =0x%x, int_en=%x\n", (uint32_t)phy_addr_temp, *int_status);

        pvt_release_instance(pvt_inst);
        ret = 1;
    }

    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_clear_int.
//!
//! @param res_id input, pvt res id
//! @param int_type input, alarm type
//! @param index input, alarm index must be 0 or 1
//! This function is for clear alarm, h alarm, temp must small than h_l, l alarm, temp must larger than l_h
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_clear_int(pvt_res_id_t res_id, pvt_int_type_t int_type, uint32_t index){
    int ret = 0;
    uint32_t data = 0;
    paddr_t phy_addr_temp = 0;
    pvt_instance_t* pvt_inst;
    spin_lock_saved_state_t states;
    uint32_t hyst_clr_offset[MAX_PVT_ALARM_NUM] = {PVT_REGISTER_INT_CLR_OFFSET,PVT_REGISTER_INT_CLR1_OFFSET};

    if((res_id >= MAX_PVT_DEVICE_NUM)||(index>=MAX_PVT_ALARM_NUM)){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){

        //enable/disable pvt int
        phy_addr_temp = pvt_inst->phy_addr+hyst_clr_offset[index];
        data = readl(phy_addr_temp);

        data = (data&(~(0x1<<int_type)))|(0x1<<int_type);

        writel(data, phy_addr_temp);

        LTRACEF("hal_pvt_clear_int phy_addr =0x%x, clear=%x\n", (uint32_t)phy_addr_temp, data);

        pvt_release_instance(pvt_inst);
        ret = 1;
    }

    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_int_register.
//!
//! @param res_id input, pvt res id
//! @param reg_unreg input, 1 register int, 0 unregister int
//! @param call_func input, int call back function
//! @param index input, alarm index must be 0 or 1
//! This function is for register int, or mask interrupt
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_int_register(pvt_res_id_t res_id, uint32_t reg_unreg, int_handler call_func, uint32_t index, void *arg){
    int ret = 0;
    pvt_instance_t* pvt_inst;
    spin_lock_saved_state_t states;
    uint32_t int_num[MAX_PVT_DEVICE_NUM][MAX_PVT_ALARM_NUM] = {{PVT_SNS_SAF_PVT_INT_0_NUM,PVT_SNS_SAF_PVT_INT_1_NUM},{PVT_SNS_SEC_PVT_INT_0_NUM,PVT_SNS_SEC_PVT_INT_1_NUM}};

    if((res_id >= MAX_PVT_DEVICE_NUM)||(index>=MAX_PVT_ALARM_NUM)){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){

        //register int

        LTRACEF("hal_pvt_set_hyst_h int_num=%d, reg_unreg=%d\n", int_num[res_id][index],reg_unreg);
        if(reg_unreg == 0){
            mask_interrupt(int_num[res_id][index]);
        }else{
            if(arg != NULL){
                pvt_inst->arg = arg;
            }
            register_int_handler(int_num[res_id][index],call_func,pvt_inst);
            unmask_interrupt(int_num[res_id][index]);
        }

        pvt_release_instance(pvt_inst);
        ret = 1;
    }

    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_get_pvt.
//!
//! @param res_id input, pvt res id
//! @param pvt_device_type input, pvt device type
//! @param out_data output, pvt value
//! This function is for get pvt value
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_pvt(pvt_res_id_t res_id, pvt_device_type_t pvt_device_type, pvt_out_data_t * out_data){
    int ret = 0;
    uint32_t i = 0;
    pvt_instance_t* pvt_inst;
    spin_lock_saved_state_t states;

    if(res_id >= MAX_PVT_DEVICE_NUM){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){
        for(i = PVT_MODE_TYPE_TEMP; i<=PVT_MODE_TYPE_P; i++){
            pvt_set_mode(pvt_inst->phy_addr, i, pvt_device_type);
            pvt_read_pvt_value(pvt_inst->phy_addr, out_data);
        }
        pvt_release_instance(pvt_inst);
        ret = 1;
    }

    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);

    return ret;
}

//*****************************************************************************
//
//! hal_pvt_get_pvt.
//!
//! @param res_id input, pvt res id
//! @param pvt_device_type input, pvt device type
//! @param out_data output, p value
//! This function is for get pvt value
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_p(pvt_res_id_t res_id, pvt_device_type_t pvt_device_type, uint32_t * out_data){
    int ret = 0;
    pvt_instance_t *pvt_inst;
    pvt_out_data_t out_data_t;

    spin_lock_saved_state_t states;

    if(res_id >= MAX_PVT_DEVICE_NUM){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){

        pvt_set_mode(pvt_inst->phy_addr, PVT_MODE_TYPE_P, pvt_device_type);
        pvt_read_pvt_value(pvt_inst->phy_addr, &out_data_t);
        *out_data = out_data_t.process_data;

        pvt_release_instance(pvt_inst);
        ret =1;
    }

    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);
    return ret;
}

//*****************************************************************************
//
//! hal_pvt_get_pvt.
//!
//! @param res_id input, pvt res id
//! @param out_data output, v value
//! This function is for get pvt value
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_v(pvt_res_id_t res_id, float * out_data){
    int ret = 0;
    pvt_instance_t* pvt_inst;
    pvt_out_data_t out_data_t;
    spin_lock_saved_state_t states;

    if(res_id >= MAX_PVT_DEVICE_NUM){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){

        pvt_set_mode(pvt_inst->phy_addr, PVT_MODE_TYPE_VOL, 0);
        pvt_read_pvt_value(pvt_inst->phy_addr, &out_data_t);
        *out_data = out_data_t.voltage_data;

        pvt_release_instance(pvt_inst);
        ret = 1;
    }
    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);
    return ret;
}
//*****************************************************************************
//
//! hal_pvt_get_pvt.
//!
//! @param res_id input, pvt res id
//! @param out_data output, t value
//! This function is for get pvt value
//!
//! @return  status
//
//*****************************************************************************
int hal_pvt_get_t(pvt_res_id_t res_id, float * out_data){
    int ret = 0;
    pvt_instance_t* pvt_inst;
    pvt_out_data_t out_data_t;
    spin_lock_saved_state_t states;

    if(res_id >= MAX_PVT_DEVICE_NUM){
        return ret;
    }

    spin_lock_irqsave(&(g_pvtInstance[res_id].lock), states);

    pvt_init(res_id);

    pvt_inst = pvt_get_instance(res_id);

    if(pvt_inst != NULL){

        pvt_set_mode(pvt_inst->phy_addr, PVT_MODE_TYPE_TEMP, 0);
        pvt_read_pvt_value(pvt_inst->phy_addr, &out_data_t);
        *out_data = out_data_t.temp_data;

        pvt_release_instance(pvt_inst);
        ret = 1;
    }
    spin_unlock_irqrestore(&(g_pvtInstance[res_id].lock), states);
    return ret;
}