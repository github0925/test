/*
 * ap_safety_mail.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include "string.h"
#include "remote_test.h"
#include "func_can.h"
#include "board_start.h"
/*
* safety send mail to ap,due to resource not in safety domain.
*/
static ap_safety_mail_t ap_safety_mail;
/*creat a road for communication between safety core and ap core, we call this We call this road a remote call */
bool remote_test_queue_creat(void)
{
    bool ret = false;

    ap_safety_mail.lock = SPIN_LOCK_INITIAL_VALUE;

    if ((ap_safety_mail.xMutex = xSemaphoreCreateMutex()) == NULL)
        return ret;

    if ((ap_safety_mail.MsgQueue = xQueueCreate(sizeof(ap_safety_mail.MsgBuf),
                                   2)) == false)
        return ret;

    set_para_value(ret, true);

    return ret;
}
/*initialize remote road flag*/
void remote_test_init(void)
{
    set_para_value(ap_safety_mail.xmutex_flg, true);
}
/*safety core sends a remote request to ap core*/
bool remote_test_send_req(can_cmd_t *cmdx)
{
    canx_opt_t canx_opt;
    set_para_value(canx_opt.can_send, (can_send_t *)canx_opt.pay_load);
    set_para_value(canx_opt.resp_chn_id, CAN2);
    memcpy(canx_opt.can_send, cmdx, sizeof(canx_opt.pay_load));

    return can_channel_to_write(&canx_opt, SINGLE_CMD);
}
/*safety core waitting ap core response*/
bool remote_test_wait_resp(uint32_t time_out, board_test_exec_t *exec)
{
    bool ret = false;

    set_para_value(ap_safety_mail.ap_resp_state, _RECV);
    set_para_value(ap_safety_mail.xqueue_flg, true);

    if (ap_safety_mail.xmutex_flg == false)
        return ret;

    set_para_value(ap_safety_mail.xmutex_flg, false);

    if ((ret = xQueueReceive(ap_safety_mail.MsgQueue,
                             (void *)&ap_safety_mail.MsgBuf, time_out)) != true) {
        set_para_value(ap_safety_mail.ap_resp_state, _ERR);
        set_para_value(ap_safety_mail.xqueue_flg, false);
        set_para_value(ap_safety_mail.xmutex_flg, true);
        xQueueReset(ap_safety_mail.MsgQueue);
        return ret;
    }

    memcpy(exec->resp, &ap_safety_mail.MsgBuf, sizeof(ap_safety_mail.MsgBuf));

    return ret;
}
/*safety core processes ap core response*/
bool remote_test_resp_cb(const uint8_t *CanSduPtr)
{
    bool ret = false;
    BaseType_t pxHigherPriorityTaskWoken = true;

    if (ap_safety_mail.ap_resp_state != _RECV)
        return ret;

    memcpy(&ap_safety_mail.MsgBuf, CanSduPtr, sizeof(ap_safety_mail.MsgBuf));

    if (ap_safety_mail.xqueue_flg != false) {
        if ((ret = xQueueSendFromISR(ap_safety_mail.MsgQueue,
                                     (void *)&ap_safety_mail.MsgBuf, NULL)) != true) {
            set_para_value(ap_safety_mail.ap_resp_state, _ERR);
            return ret;
        }
    }

    set_para_value(ap_safety_mail.xmutex_flg, true);
    set_para_value(ap_safety_mail.ap_resp_state, _IDLE);

    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

    return ret;
}
