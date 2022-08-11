#include <irq.h>
#include <__regs_base.h>
#include <Can.h>
#include "can_cfg.h"

struct Can_ControllerConfig gCan_CtrllerConfig[CAN_CTRL_CONFIG_CNT] = {
    {
        .controllerId = CAN1,
        .baseAddr = (uint32)APB_CAN1_BASE,
        .irq_num = CANFD1_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN2,
        .baseAddr = (uint32)APB_CAN2_BASE,
        .irq_num = CANFD2_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN3,
        .baseAddr = (uint32)APB_CAN3_BASE,
        .irq_num = CANFD3_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN4,
        .baseAddr = (uint32)APB_CAN4_BASE,
        .irq_num = CANFD4_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

#if MAX_FLEXCAN_CH > 4
    {
        .controllerId = CAN5,
        .baseAddr = (uint32)APB_CAN5_BASE,
        .irq_num = CANFD5_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN6,
        .baseAddr = (uint32)APB_CAN6_BASE,
        .irq_num = CANFD6_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN7,
        .baseAddr = (uint32)APB_CAN7_BASE,
        .irq_num = CANFD7_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN8,
        .baseAddr = (uint32)APB_CAN8_BASE,
        .irq_num = CANFD8_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

#if MAX_FLEXCAN_CH > 8
    {
        .controllerId = CAN9,
        .baseAddr = (uint32)APB_CAN9_BASE,
        .irq_num = CANFD9_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN10,
        .baseAddr = (uint32)APB_CAN10_BASE,
        .irq_num = CANFD10_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN11,
        .baseAddr = (uint32)APB_CAN11_BASE,
        .irq_num = CANFD11_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN12,
        .baseAddr = (uint32)APB_CAN12_BASE,
        .irq_num = CANFD12_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN13,
        .baseAddr = (uint32)APB_CAN13_BASE,
        .irq_num = CANFD13_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN14,
        .baseAddr = (uint32)APB_CAN14_BASE,
        .irq_num = CANFD14_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN15,
        .baseAddr = (uint32)APB_CAN15_BASE,
        .irq_num = CANFD15_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN16,
        .baseAddr = (uint32)APB_CAN16_BASE,
        .irq_num = CANFD16_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN17,
        .baseAddr = (uint32)APB_CAN17_BASE,
        .irq_num = CANFD17_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN18,
        .baseAddr = (uint32)APB_CAN18_BASE,
        .irq_num = CANFD18_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN19,
        .baseAddr = (uint32)APB_CAN19_BASE,
        .irq_num = CANFD19_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },

    {
        .controllerId = CAN20,
        .baseAddr = (uint32)APB_CAN20_BASE,
        .irq_num = CANFD20_IPI_INT_MBOR_NUM,
        .flexcanCfg = {
            .clkSrc = FLEXCAN_ClkSrcPeri,   /* 80MHz */
            .maxMbNum = 14U,
            .enableSelfWakeup = true,
            .enableIndividMask = true,
            .enableCANFD = true,
            BAUDRATE_1M_5M,
            .can_fd_cfg = {
                .enableISOCANFD = true,
                .enableBRS = true,
                .enableTDC = true,
                .TDCOffset = 8U,
                .r0_mb_data_size = CAN_FD_64BYTES_PER_MB,
                .r1_mb_data_size = CAN_FD_64BYTES_PER_MB
            }
        }
    },
#endif
#endif
};

/**
 * @note Mail boxes of the same controller MUST BE
 *       CONSECUTIVE in the configuration structure.
 */
struct Can_MBConfig gCan_RxMBCfg[NUM_OF_HRHS] = {
    /**
     * Format: Hrh, CAN ID, Mail box ID, Polling flag, frame ID, mask.
     *
     * If one MB rx any frame, frmae ID is not matter and
     * mask should be 0.
     */

    /* CAN1 */
    GEN_ETD_RX_MB_CFG	(	0,	1,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	1,	1,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	2,	1,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	3,	1,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	4,	1,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	5,	1,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	6,	1,	6,	false, 0,	0	),

    /* CAN2 */
    GEN_ETD_RX_MB_CFG	(	7,	2,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	8,	2,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	9,	2,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	10,	2,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	11,	2,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	12,	2,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	13,	2,	6,	false, 0,	0	),

    /* CAN3 */
    GEN_ETD_RX_MB_CFG	(	14,	3,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	15,	3,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	16,	3,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	17,	3,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	18,	3,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	19,	3,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	20,	3,	6,	false, 0,	0	),

    /* CAN4 */
    GEN_ETD_RX_MB_CFG	(	21,	4,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	22,	4,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	23,	4,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	24,	4,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	25,	4,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	26,	4,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	27,	4,	6,	false, 0,	0	),

#if MAX_FLEXCAN_CH > 4
    /* CAN5 */
    GEN_ETD_RX_MB_CFG	(	28,	5,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	29,	5,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	30,	5,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	31,	5,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	32,	5,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	33,	5,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	34,	5,	6,	false, 0,	0	),


    /* CAN6 */
    GEN_ETD_RX_MB_CFG	(	35,	6,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	36,	6,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	37,	6,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	38,	6,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	39,	6,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	40,	6,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	41,	6,	6,	false, 0,	0	),

    /* CAN7 */
    GEN_ETD_RX_MB_CFG	(	42,	7,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	43,	7,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	44,	7,	2,	false,0,	0	),
    GEN_ETD_RX_MB_CFG	(	45,	7,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	46,	7,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	47,	7,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	48,	7,	6,	false, 0,	0	),

    /* CAN8 */
    GEN_ETD_RX_MB_CFG	(	49,	8,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	50,	8,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	51,	8,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	52,	8,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	53,	8,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	54,	8,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	55,	8,	6,	false, 0,	0	),

#if MAX_FLEXCAN_CH > 8
    /* CAN9 */
    GEN_ETD_RX_MB_CFG	(	56,	9,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	57,	9,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	58,	9,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	59,	9,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	60,	9,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	61,	9,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	62,	9,	6,	false, 0,	0	),

    /* CAN10 */
    GEN_ETD_RX_MB_CFG	(	63,	10,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	64,	10,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	65,	10,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	66,	10,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	67,	10,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	68,	10,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	69,	10,	6,	false, 0,	0	),

    /* CAN11 */
    GEN_ETD_RX_MB_CFG	(	70,	11,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	71,	11,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	72,	11,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	73,	11,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	74,	11,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	75,	11,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	76,	11,	6,	false, 0,	0	),

    /* CAN12 */
    GEN_ETD_RX_MB_CFG	(	77,	12,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	78,	12,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	79,	12,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	80,	12,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	81,	12,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	82,	12,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	83,	12,	6,	false, 0,	0	),

    /* CAN13 */
    GEN_ETD_RX_MB_CFG	(	84,	13,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	85,	13,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	86,	13,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	87,	13,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	88,	13,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	89,	13,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	90,	13,	6,	false, 0,	0	),

    /* CAN14 */
    GEN_ETD_RX_MB_CFG	(	91,	14,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	92,	14,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	93,	14,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	94,	14,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	95,	14,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	96,	14,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	97,	14,	6,	false, 0,	0	),

    /* CAN15 */
    GEN_ETD_RX_MB_CFG	(	98,	15,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	99,	15,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	100,	15,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	101,	15,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	102,	15,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	103,	15,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	104,	15,	6,	false, 0,	0	),

    /* CAN16 */
    GEN_ETD_RX_MB_CFG	(	105,	16,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	106,	16,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	107,	16,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	108,	16,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	109,	16,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	110,	16,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	111,	16,	6,	false, 0,	0	),

    /* CAN17 */
    GEN_ETD_RX_MB_CFG	(	112,	17,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	113,	17,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	114,	17,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	115,	17,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	116,	17,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	117,	17,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	118,	17,	6,	false, 0,	0	),

    /* CAN18 */
    GEN_ETD_RX_MB_CFG	(	119,	18,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	120,	18,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	121,	18,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	122,	18,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	123,	18,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	124,	18,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	125,	18,	6,	false, 0,	0	),

    /* CAN19 */
    GEN_ETD_RX_MB_CFG	(	126,	19,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	127,	19,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	128,	19,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	129,	19,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	130,	19,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	131,	19,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	132,	19,	6,	false, 0,	0	),

    /* CAN20 */
    GEN_ETD_RX_MB_CFG	(	133,	20,	0,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	134,	20,	1,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	135,	20,	2,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	136,	20,	3,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	137,	20,	4,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	138,	20,	5,	false, 0,	0	),
    GEN_ETD_RX_MB_CFG	(	139,	20,	6,	false, 0,	0	),
#endif
#endif
};

/**
 * @note Mail boxes of the same controller MUST BE
 *       CONSECUTIVE in the configuration structure.
 */
struct Can_MBConfig gCan_TxMBCfg[NUM_OF_HTHS] = {
    /**
     * Format: Hth, CAN ID, Mail box ID, Polling flag, padding value.
     *
     * Padding value is used in CAN FD mode.
     */

    /* CAN1 */
    GEN_TX_MB_CFG	(	1,	1,	7,	false, 0	),
    GEN_TX_MB_CFG	(	3,	1,	8,	false, 0	),
    GEN_TX_MB_CFG	(	0,	1,	9,	false, 0	),
    GEN_TX_MB_CFG	(	2,	1,	10,	false, 0	),
    GEN_TX_MB_CFG	(	6,	1,	11,	false, 0	),
    GEN_TX_MB_CFG	(	5,	1,	12,	false, 0	),
    GEN_TX_MB_CFG	(	4,	1,	13,	false, 0	),

    /* CAN2 */
    GEN_TX_MB_CFG	(	7,	2,	7,	false, 0	),
    GEN_TX_MB_CFG	(	8,	2,	8,	false, 0	),
    GEN_TX_MB_CFG	(	9,	2,	9,	false, 0	),
    GEN_TX_MB_CFG	(	10,	2,	10,	false, 0	),
    GEN_TX_MB_CFG	(	11,	2,	11,	false, 0	),
    GEN_TX_MB_CFG	(	12,	2,	12,	false, 0	),
    GEN_TX_MB_CFG	(	13,	2,	13,	false, 0	),

    /* CAN3 */
    GEN_TX_MB_CFG	(	14,	3,	7,	false, 0	),
    GEN_TX_MB_CFG	(	15,	3,	8,	false, 0	),
    GEN_TX_MB_CFG	(	16,	3,	9,	false, 0	),
    GEN_TX_MB_CFG	(	17,	3,	10,	false, 0	),
    GEN_TX_MB_CFG	(	18,	3,	11,	false, 0	),
    GEN_TX_MB_CFG	(	19,	3,	12,	false, 0	),
    GEN_TX_MB_CFG	(	20,	3,	13,	false, 0	),

    /* CAN4 */
    GEN_TX_MB_CFG	(	21,	4,	7,	false, 0	),
    GEN_TX_MB_CFG	(	22,	4,	8,	false, 0	),
    GEN_TX_MB_CFG	(	23,	4,	9,	false, 0	),
    GEN_TX_MB_CFG	(	24,	4,	10,	false, 0	),
    GEN_TX_MB_CFG	(	25,	4,	11,	false, 0	),
    GEN_TX_MB_CFG	(	26,	4,	12,	false, 0	),
    GEN_TX_MB_CFG	(	27,	4,	13,	false, 0	),
#if MAX_FLEXCAN_CH > 4
    /* CAN5 */
    GEN_TX_MB_CFG	(	28,	5,	7,	false, 0	),
    GEN_TX_MB_CFG	(	29,	5,	8,	false, 0	),
    GEN_TX_MB_CFG	(	30,	5,	9,	false, 0	),
    GEN_TX_MB_CFG	(	31,	5,	10,	false, 0	),
    GEN_TX_MB_CFG	(	32,	5,	11,	false, 0	),
    GEN_TX_MB_CFG	(	33,	5,	12,	false, 0	),
    GEN_TX_MB_CFG	(	34,	5,	13,	false, 0	),

    /* CAN6 */
    GEN_TX_MB_CFG	(	35,	6,	7,	false, 0	),
    GEN_TX_MB_CFG	(	36,	6,	8,	false, 0	),
    GEN_TX_MB_CFG	(	37,	6,	9,	false, 0	),
    GEN_TX_MB_CFG	(	38,	6,	10,	false, 0	),
    GEN_TX_MB_CFG	(	39,	6,	11,	false, 0	),
    GEN_TX_MB_CFG	(	40,	6,	12,	false, 0	),
    GEN_TX_MB_CFG	(	41,	6,	13,	false, 0	),

    /* CAN7 */
    GEN_TX_MB_CFG	(	42,	7,	7,	false, 0	),
    GEN_TX_MB_CFG	(	43,	7,	8,	false, 0	),
    GEN_TX_MB_CFG	(	44,	7,	9,	false, 0	),
    GEN_TX_MB_CFG	(	45,	7,	10,	false, 0	),
    GEN_TX_MB_CFG	(	46,	7,	11,	false, 0	),
    GEN_TX_MB_CFG	(	47,	7,	12,	false, 0	),
    GEN_TX_MB_CFG	(	48,	7,	13,	false, 0	),

    /* CAN8 */
    GEN_TX_MB_CFG	(	49,	8,	7,	false, 0	),
    GEN_TX_MB_CFG	(	50,	8,	8,	false, 0	),
    GEN_TX_MB_CFG	(	51,	8,	9,	false, 0	),
    GEN_TX_MB_CFG	(	52,	8,	10,	false, 0	),
    GEN_TX_MB_CFG	(	53,	8,	11,	false, 0	),
    GEN_TX_MB_CFG	(	54,	8,	12,	false, 0	),
    GEN_TX_MB_CFG	(	55,	8,	13,	false, 0	),

#if MAX_FLEXCAN_CH > 8
    /* CAN9 */
    GEN_TX_MB_CFG	(	56,	9,	7,	false, 0	),
    GEN_TX_MB_CFG	(	57,	9,	8,	false, 0	),
    GEN_TX_MB_CFG	(	58,	9,	9,	false, 0	),
    GEN_TX_MB_CFG	(	59,	9,	10,	false, 0	),
    GEN_TX_MB_CFG	(	60,	9,	11,	false, 0	),
    GEN_TX_MB_CFG	(	61,	9,	12,	false, 0	),
    GEN_TX_MB_CFG	(	62,	9,	13,	false, 0	),

    /* CAN10 */
    GEN_TX_MB_CFG	(	63,	10,	7,	false, 0	),
    GEN_TX_MB_CFG	(	64,	10,	8,	false, 0	),
    GEN_TX_MB_CFG	(	65,	10,	9,	false, 0	),
    GEN_TX_MB_CFG	(	66,	10,	10,	false, 0	),
    GEN_TX_MB_CFG	(	67,	10,	11,	false, 0	),
    GEN_TX_MB_CFG	(	68,	10,	12,	false, 0	),
    GEN_TX_MB_CFG	(	69,	10,	13,	false, 0	),

    /* CAN11 */
    GEN_TX_MB_CFG	(	70,	11,	7,	false, 0	),
    GEN_TX_MB_CFG	(	71,	11,	8,	false, 0	),
    GEN_TX_MB_CFG	(	72,	11,	9,	false, 0	),
    GEN_TX_MB_CFG	(	73,	11,	10,	false, 0	),
    GEN_TX_MB_CFG	(	74,	11,	11,	false, 0	),
    GEN_TX_MB_CFG	(	75,	11,	12,	false, 0	),
    GEN_TX_MB_CFG	(	76,	11,	13,	false, 0	),

    /* CAN12 */
    GEN_TX_MB_CFG	(	77,	12,	7,	false, 0	),
    GEN_TX_MB_CFG	(	78,	12,	8,	false, 0	),
    GEN_TX_MB_CFG	(	79,	12,	9,	false, 0	),
    GEN_TX_MB_CFG	(	80,	12,	10,	false, 0	),
    GEN_TX_MB_CFG	(	81,	12,	11,	false, 0	),
    GEN_TX_MB_CFG	(	82,	12,	12,	false, 0	),
    GEN_TX_MB_CFG	(	83,	12,	13,	false, 0	),

    /* CAN13 */
    GEN_TX_MB_CFG	(	84,	13,	7,	false, 0	),
    GEN_TX_MB_CFG	(	85,	13,	8,	false, 0	),
    GEN_TX_MB_CFG	(	86,	13,	9,	false, 0	),
    GEN_TX_MB_CFG	(	87,	13,	10,	false, 0	),
    GEN_TX_MB_CFG	(	88,	13,	11,	false, 0	),
    GEN_TX_MB_CFG	(	89,	13,	12,	false, 0	),
    GEN_TX_MB_CFG	(	90,	13,	13,	false, 0	),

    /* CAN14 */
    GEN_TX_MB_CFG	(	91,	14,	7,	false, 0	),
    GEN_TX_MB_CFG	(	92,	14,	8,	false, 0	),
    GEN_TX_MB_CFG	(	93,	14,	9,	false, 0	),
    GEN_TX_MB_CFG	(	94,	14,	10,	false, 0	),
    GEN_TX_MB_CFG	(	95,	14,	11,	false, 0	),
    GEN_TX_MB_CFG	(	96,	14,	12,	false, 0	),
    GEN_TX_MB_CFG	(	97,	14,	13,	false, 0	),

    /* CAN15 */
    GEN_TX_MB_CFG	(	98,	15,	7,	false, 0	),
    GEN_TX_MB_CFG	(	99,	15,	8,	false, 0	),
    GEN_TX_MB_CFG	(	100,	15,	9,	false, 0	),
    GEN_TX_MB_CFG	(	101,	15,	10,	false, 0	),
    GEN_TX_MB_CFG	(	102,	15,	11,	false, 0	),
    GEN_TX_MB_CFG	(	103,	15,	12,	false, 0	),
    GEN_TX_MB_CFG	(	104,	15,	13,	false, 0	),

    /* CAN16 */
    GEN_TX_MB_CFG	(	105,	16,	7,	false, 0	),
    GEN_TX_MB_CFG	(	106,	16,	8,	false, 0	),
    GEN_TX_MB_CFG	(	107,	16,	9,	false, 0	),
    GEN_TX_MB_CFG	(	108,	16,	10,	false, 0	),
    GEN_TX_MB_CFG	(	109,	16,	11,	false, 0	),
    GEN_TX_MB_CFG	(	110,	16,	12,	false, 0	),
    GEN_TX_MB_CFG	(	111,	16,	13,	false, 0	),

    /* CAN17 */
    GEN_TX_MB_CFG	(	112,	17,	7,	false, 0	),
    GEN_TX_MB_CFG	(	113,	17,	8,	false, 0	),
    GEN_TX_MB_CFG	(	114,	17,	9,	false, 0	),
    GEN_TX_MB_CFG	(	115,	17,	10,	false, 0	),
    GEN_TX_MB_CFG	(	116,	17,	11,	false, 0	),
    GEN_TX_MB_CFG	(	117,	17,	12,	false, 0	),
    GEN_TX_MB_CFG	(	118,	17,	13,	false, 0	),

    /* CAN18 */
    GEN_TX_MB_CFG	(	119,	18,	7,	false, 0	),
    GEN_TX_MB_CFG	(	120,	18,	8,	false, 0	),
    GEN_TX_MB_CFG	(	121,	18,	9,	false, 0	),
    GEN_TX_MB_CFG	(	122,	18,	10,	false, 0	),
    GEN_TX_MB_CFG	(	123,	18,	11,	false, 0	),
    GEN_TX_MB_CFG	(	124,	18,	12,	false, 0	),
    GEN_TX_MB_CFG	(	125,	18,	13,	false, 0	),

    /* CAN19 */
    GEN_TX_MB_CFG	(	126,	19,	7,	false, 0	),
    GEN_TX_MB_CFG	(	127,	19,	8,	false, 0	),
    GEN_TX_MB_CFG	(	128,	19,	9,	false, 0	),
    GEN_TX_MB_CFG	(	129,	19,	10,	false, 0	),
    GEN_TX_MB_CFG	(	130,	19,	11,	false, 0	),
    GEN_TX_MB_CFG	(	131,	19,	12,	false, 0	),
    GEN_TX_MB_CFG	(	132,	19,	13,	false, 0	),

    /* CAN20 */
    GEN_TX_MB_CFG	(	133,	20,	7,	false, 0	),
    GEN_TX_MB_CFG	(	134,	20,	8,	false, 0	),
    GEN_TX_MB_CFG	(	135,	20,	9,	false, 0	),
    GEN_TX_MB_CFG	(	136,	20,	10,	false, 0	),
    GEN_TX_MB_CFG	(	137,	20,	11,	false, 0	),
    GEN_TX_MB_CFG	(	138,	20,	12,	false, 0	),
    GEN_TX_MB_CFG	(	139,	20,	13,	false, 0	),
#endif
#endif
};

const Can_ConfigType gCan_Config = {
    .controllerCount = CAN_CTRL_CONFIG_CNT,
    .rxCount = NUM_OF_HRHS,
    .txCount = NUM_OF_HTHS,
    .ctrllerCfg = gCan_CtrllerConfig,
    .rxMBCfg = gCan_RxMBCfg,
    .txMBCfg = gCan_TxMBCfg
};
