#include <Os.h>
#include <Com.h>
#include <debug.h>
#include <__regs_base.h>

ALARM(McalTest)
{
    OsActivateTask(McalTest);
}

TASK(McalTest)
{
    uint8 sigVal = 0x33;
    dprintf(INFO, "CAN1 MCR = 0x%x\n", *((uint32*)APB_CAN1_BASE));
    dprintf(INFO, "CAN1 mb data:\n");
    hexdump((const void*)(APB_CAN1_BASE + 0x80U), 32U);
    dprintf(ALWAYS, "Send signal <COM_SID_LoopBack_Tx>\n");
    Com_SendSignal(COM_SID_LoopBack_Tx, &sigVal);
    dprintf(INFO, "CAN1 IFLAG1 = 0x%x\n", *((uint32*)(APB_CAN1_BASE + 0x30U)));
    Com_ReceiveSignal(COM_SID_LoopBack_Rx, &sigVal);
    dprintf(ALWAYS, "The value of received signal <COM_SID_LoopBack_Rx> is 0x%x\n", sigVal);
    OsTerminateTask(McalTest);
}