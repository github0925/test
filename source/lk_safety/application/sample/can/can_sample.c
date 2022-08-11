#include <Can.h>
#include <can_cfg.h>
#include <stdio.h>
#include <lib/console.h>

void CanIf_RxIndication(uint16_t ControllerId, Can_IdType CanId,
                               uint8_t CanDlc, const uint8_t *CanSduPtr)
{
    printf("can_sample: CanIf_RxIndication\n");
    printf("ControllerId=%d CanId=0x%x CanDlc=%d\n", ControllerId, CanId, CanDlc);
    for(int i = 0; i < CanDlc; i++) {
        printf("data[%d]=%x ", i, CanSduPtr[i]);
    }
    printf("\n");
    return;
}

void CanIf_TxComfirmation(PduIdType CanTxPduId)
{
    printf("can_sample: CanIf_TxComfirmation\n");
    printf("CanTxPduId=%d\n", CanTxPduId);
    return;
}

int can_sample(void)
{
    Can_Init(&gCan_Config);

    printf("can channel 2: Can_SetControllerMode\n");
    Can_SetControllerMode(1, CAN_CS_STARTED);

    uint8_t Data[64] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                        20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                        30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                        40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                        50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                        60, 61, 62, 63
                        };
    uint16_t hth;
    Can_PduType PduInfo;
    uint16_t swPduHandle = 0;
    Std_ReturnType ret;
    uint32_t id;
    uint8_t length;

    /* transmit canfd data by can ch2 */
    hth = 13;
    id = MAKE_STANDARD_CAN_FD_ID(0x327);
    length = 64U;

    PduInfo.swPduHandle = swPduHandle;
    PduInfo.length = length;
    PduInfo.id = id;
    PduInfo.sdu = Data;

    ret = Can_Write(hth, &PduInfo);
    printf("%s() Result: %d, hth=%d\n", __func__, ret, hth);

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("can_sample", "can sample once", (console_cmd)&can_sample)
STATIC_COMMAND_END(can);
#endif