/*
 * slt_com_uart.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <assert.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lib/console.h>
#include <slt_message.h>
#include <slt_main.h>
#include <pvt_hal.h>

#define MAX_RESULT_INFO_LEN 1024

static int start_test(void)
{
    int ret;
    slt_app_context_t* pcontext;
    slt_msg_node_t msg_node;

    msg_node.msg_item.msg_type = SLT_MESSAGE_FROM_PC_TEST;
    msg_node.msg_item.msg_len = 0;
    msg_node.msg_item.msg_value = NULL;

    pcontext = slt_get_gcontext();
    ret = slt_putmsginqueue(pcontext, &msg_node);

    return ret;
}

int slt_send_msg2host(const uint8_t * buf, uint32_t count)
{
    for(uint32_t i = 0; i < count; i++)
    {
        platform_dputc(buf[i]);
    }
    return count;
}

int notify_host_test_result(slt_app_context_t* pcontext)
{
    char *result_info = calloc(1, MAX_RESULT_INFO_LEN);
    int pass_ret = 0;
    uint32_t print_len;
    char* temp;

    pvt_out_data_t out_data = {0};

    //init pvt value
    //#define PVT_DEVICE_TYPE_ULVT 1
    hal_pvt_get_pvt(PVT_RES_ID_SAF, 1, &out_data);

    if (!pcontext || !result_info)
        return -1;

    temp = result_info;
    for(uint32_t j = 0; j< SLT_DOMAIN_TYPE_END;j++){
        if(pcontext->domain_test_enable[j] == 0){
            continue;
        }
        pass_ret |= pcontext->slt_test_result.test_result_info[j].test_result;
    }

    if (pass_ret)
    {
        print_len = sprintf(temp, "<<ErrCode:Fail,Temperature:%f,RawData:ChipID=0x%08x%08x,,SV=%d,Cnum=%d,Time=%dms", out_data.temp_data,
                pcontext->slt_test_result.chip_id[1], pcontext->slt_test_result.chip_id[0], pcontext->software_version, pcontext->slt_test_result.all_test_num, pcontext->slt_test_result.all_test_usetime);
        temp += (print_len > 0) ? print_len : 0;
    }
    else
    {
        print_len = sprintf(temp, "<<ErrCode:Pass,Temperature:%f,RawData:ChipID=0x%08x%08x,SV=%d,Cnum=%d,Time=%dms>>", out_data.temp_data,
                pcontext->slt_test_result.chip_id[1], pcontext->slt_test_result.chip_id[0], pcontext->software_version, pcontext->slt_test_result.all_test_num, pcontext->slt_test_result.all_test_usetime);
        temp += (print_len > 0) ? print_len : 0;
        goto end;
    }

    for(uint32_t j = 0; j< SLT_DOMAIN_TYPE_END;j++){

        if(pcontext->domain_test_enable[j] == 0){
            continue;
        }

        if (pcontext->slt_test_result.test_result_info[j].test_result == 0)
            continue;

        print_len = sprintf(temp, "domain%d:Fail,", j);
        temp += (print_len > 0) ? print_len : 0;

        for (uint32_t i = 0; i < pcontext->slt_test_result.test_result_info[j].test_num; i++) {
            if (pcontext->slt_test_result.test_result_info[j].test_case_info[i].result_value)
            {
                print_len = sprintf(temp, "%s:fail,value:0x%x,reason:%s",
                        pcontext->slt_test_result.test_result_info[j].test_case_info[i].test_name,
                        pcontext->slt_test_result.test_result_info[j].test_case_info[i].result_value,
                        pcontext->slt_test_result.test_result_info[j].test_case_info[i].result_string);

                temp += (print_len > 0) ? print_len : 0;
            }
        }
    }

    print_len = sprintf(temp, ">>\n");
    temp += (print_len > 0) ? print_len : 0;
end:
    slt_send_msg2host((uint8_t*)result_info, (uint32_t)(temp - result_info));
    if(result_info)
        free(result_info);

    return 0;
}

int notify_host_ready(void)
{
    slt_send_msg2host((uint8_t*)"DOSOK\n", 6);
    return 0;
}

int tw_cmd(int argc, const cmd_args* argv)
{
    if (slt_all_domain_init_done())
        start_test();

    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("#NEXT", "start slt test cmd", (console_cmd)&tw_cmd)
STATIC_COMMAND_END(tw_test);
