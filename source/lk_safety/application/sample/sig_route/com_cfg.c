#include "com_cfg.h"
#include "Can.h"
#include "Lin.h"

/*==================Tx frames==================*/
frame_desc_t gw_3 = {
    .port_id = PORT_CAN,
    .bus_id = CAN2,
    .prot_id = 0x2B8U,
    .len = 8U,
    .period = 100U
};

frame_desc_t gw_atc_1 = {
    .port_id = PORT_LIN,
    .bus_id = LIN_IFC_SCI2,
    .prot_id = 0x1AU,
    .len = 8U
};

frame_desc_t mcp_1 = {
    .port_id = PORT_CAN,
    .bus_id = CAN2,
    .prot_id = 0x1A4U,
    .len = 8U,
    .period = 100U
};

/*==================Rx frames==================*/
frame_desc_t egsm_1 = {
    .port_id = PORT_CAN,
    .bus_id = CAN1,
    .prot_id = 0x120U,
    .len = 8U,
    .sig_rule_nr = 2U,
    .sig_rule = {
        [0] = {
            /* EGSM_PutLeverInAutoTip */
            .src_sig = {
                .frame = &egsm_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 14U,
                .width = 1U
            },
            .tgt_sig = {
                .frame = &gw_3,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 22U,
                .width = 1U
            }
        },
        [1] = {
            /* EGSM_ErrorStatus */
            .src_sig = {
                .frame = &egsm_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 19U,
                .width = 1U
            },
            .tgt_sig = {
                .frame = &gw_3,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 23U,
                .width = 1U
            }
        }
    }
};

frame_desc_t acm_1 = {
    .port_id = PORT_CAN,
    .bus_id = CAN1,
    .prot_id = 0x122U,
    .len = 8U,
    .sig_rule_nr = 3U,
    .sig_rule = {
        [0] = {
            /* ACM_ShiftTipInfo */
            .src_sig = {
                .frame = &acm_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 12U,
                .width = 4U
            },
            .tgt_sig = {
                .frame = &gw_3,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 11U,
                .width = 4U
            }
        },
        [1] = {
            /* ACM_ErrorStatus */
            .src_sig = {
                .frame = &acm_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 16U,
                .width = 1U
            },
            .tgt_sig = {
                .frame = &gw_3,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 15U,
                .width = 1U
            }
        },
        [2] = {
            /* ACM_LimpOperateTip */
            .src_sig = {
                .frame = &acm_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 28U,
                .width = 4U
            },
            .tgt_sig = {
                .frame = &gw_3,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 16U,
                .width = 4U
            }
        },
    }
};

frame_desc_t tcu_4 = {
    .port_id = PORT_CAN,
    .bus_id = CAN1,
    .prot_id = 0x20AU,
    .len = 8U,
    .sig_rule_nr = 3U,
    .sig_rule = {
        [0] = {
            /* GearInfo */
            .src_sig = {
                .frame = &tcu_4,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 20U,
                .width = 4U
            },
            .tgt_sig = {
                .frame = &gw_3,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 0U,
                .width = 4U
            }
        },
        [1] = {
            /* GearBoxSt */
            .src_sig = {
                .frame = &tcu_4,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 13U,
                .width = 1U
            },
            .tgt_sig = {
                .frame = &gw_3,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 8U,
                .width = 1U
            }
        },
        [2] = {
            /* BuzzerCmd_TCU */
            .src_sig = {
                .frame = &tcu_4,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 24U,
                .width = 2U
            },
            .tgt_sig = {
                .frame = &gw_3,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 9U,
                .width = 2U
            }
        }
    }
};

frame_desc_t tcu_3 = {
    .port_id = PORT_CAN,
    .bus_id = CAN1,
    .prot_id = 0xB4U,
    .len = 8U,
    .sig_rule_nr = 1U,
    .sig_rule = {
        [0] = {
            /* LeverInfo */
            .src_sig = {
                .frame = &tcu_3,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 56U,
                .width = 4U
            },
            .tgt_sig = {
                .frame = &gw_atc_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 12U,
                .width = 4U
            }
        }
    }
};

frame_desc_t lin_mcp_1 = {
    .port_id = PORT_LIN,
    .bus_id = LIN_IFC_SCI2,
    .prot_id = 0x16U,
    .len = 8U,
    .sig_rule_nr = 2U,
    .sig_rule = {
        [0] = {
            /* MCP_APASwitch */
            .src_sig = {
                .frame = &lin_mcp_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 24U,
                .width = 1U
            },
            .tgt_sig = {
                .frame = &mcp_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 24U,
                .width = 1U
            }
        },
        [1] = {
            /* MCP_AVSandFRadarSwitch */
            .src_sig = {
                .frame = &lin_mcp_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 25U,
                .width = 1U
            },
            .tgt_sig = {
                .frame = &mcp_1,
                .order = BYTE_ORDER_INTEL,
                .start_bit = 25U,
                .width = 1U
            }
        }
    }
};

frame_desc_t *g_rx_frame_cfg[] = {
    &egsm_1, &acm_1, &tcu_4, &tcu_3, &lin_mcp_1, NULL
};

frame_desc_t *g_tx_frame_cfg[] = {
    &gw_3, &gw_atc_1, &mcp_1, NULL
};
