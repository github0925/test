#ifndef CAN_CFG_H_
#define CAN_CFG_H_

#define CAN_CTRL_CONFIG_CNT MAX_FLEXCAN_CH

#define BAUDRATE_250K_500K  .nominalBitTiming = {   /* 250kbps, sample point 75% */ \
                                .preDivider = 16U, \
                                .rJumpwidth = 2U, \
                                .propSeg = 6U, \
                                .phaseSeg1 = 8U, \
                                .phaseSeg2 = 5U \
                            }, \
                            .dataBitTiming = {  /* 500kbps, sample point 80% */ \
                                .preDivider = 16U, /* Should be the same as nominalBitTiming.preDivider. */ \
                                .rJumpwidth = 1U, \
                                .propSeg = 3U, \
                                .phaseSeg1 = 4U, \
                                .phaseSeg2 = 2U \
                            }

#define BAUDRATE_500K_1M    .nominalBitTiming = {   /* 500kbps, sample point 75% */ \
                                .preDivider = 8U, \
                                .rJumpwidth = 2U, \
                                .propSeg = 6U, \
                                .phaseSeg1 = 8U, \
                                .phaseSeg2 = 5U \
                            }, \
                            .dataBitTiming = {  /* 1Mbps, sample point 80% */ \
                                .preDivider = 8U, /* Should be the same as nominalBitTiming.preDivider. */ \
                                .rJumpwidth = 1U, \
                                .propSeg = 3U, \
                                .phaseSeg1 = 4U, \
                                .phaseSeg2 = 2U \
                            }

#define BAUDRATE_500K_2M    .nominalBitTiming = {   /* 500kbps, sample point 75% */ \
                                .preDivider = 4U, \
                                .rJumpwidth = 4U, \
                                .propSeg = 10U, \
                                .phaseSeg1 = 19U, \
                                .phaseSeg2 = 10U \
                            }, \
                            .dataBitTiming = {  /* 2Mbps, sample point 80% */ \
                                .preDivider = 4U, /* Should be the same as nominalBitTiming.preDivider. */ \
                                .rJumpwidth = 1U, \
                                .propSeg = 3U, \
                                .phaseSeg1 = 4U, \
                                .phaseSeg2 = 2U \
                            }

#define BAUDRATE_1M_2M  .nominalBitTiming = {   /* 1Mbps, sample point 80% */ \
                            .preDivider = 2U, \
                            .rJumpwidth = 5U, \
                            .propSeg = 10U, \
                            .phaseSeg1 = 21U, \
                            .phaseSeg2 = 8U \
                        }, \
                        .dataBitTiming = {  /* 2Mbps, sample point 75% */ \
                            .preDivider = 2U, /* Should be the same as nominalBitTiming.preDivider. */ \
                            .rJumpwidth = 3U, \
                            .propSeg = 5U, \
                            .phaseSeg1 = 9U, \
                            .phaseSeg2 = 5U \
                        }

#define BAUDRATE_1M_5M  .nominalBitTiming = {   /* 1Mbps, sample point 80% */ \
                            .preDivider = 2U, \
                            .rJumpwidth = 5U, \
                            .propSeg = 10U, \
                            .phaseSeg1 = 21U, \
                            .phaseSeg2 = 8U \
                        }, \
                        .dataBitTiming = {  /* 5Mbps, sample point 75% */ \
                            .preDivider = 2U, /* Should be the same as nominalBitTiming.preDivider. */ \
                            .rJumpwidth = 1U, \
                            .propSeg = 2U, \
                            .phaseSeg1 = 3U, \
                            .phaseSeg2 = 2U \
                        }

#define GEN_STD_RX_MB_CFG(hrh_id, can_id, mb_id, poll, frame_id, mask)    { \
                                                                           .hwObjId = RX##hrh_id, \
                                                                           .controllerId = CAN##can_id, \
                                                                           .mbId = mb_id##U, \
                                                                           .polling = poll, \
                                                                           .rx = { \
                                                                                  .frameId = frame_id##U, \
                                                                                  .rxIDFilterMask = ( mask##U << 18 ) | 0x3FFFFU, \
                                                                                  .frameType = FLEXCAN_FrameTypeData, \
                                                                                  .frameFormat = FLEXCAN_STANDARD_FRAME \
                                                                                 } \
                                                                          }
#define GEN_ETD_RX_MB_CFG(hrh_id, can_id, mb_id, poll, frame_id, mask)    { \
                                                                           .hwObjId = RX##hrh_id, \
                                                                           .controllerId = CAN##can_id, \
                                                                           .mbId = mb_id##U, \
                                                                           .polling = poll, \
                                                                           .rx = { \
                                                                               .frameId = frame_id##U, \
                                                                               .rxIDFilterMask = mask##U, \
                                                                               .frameType = FLEXCAN_FrameTypeData, \
                                                                               .frameFormat = FLEXCAN_EXTEND_FRAME \
                                                                               } \
                                                                          }

#define GEN_TX_MB_CFG(hth_id, can_id, mb_id, poll, padding)   { \
                                                               .hwObjId = TX##hth_id, \
                                                               .controllerId = CAN##can_id, \
                                                               .mbId = mb_id##U, \
                                                               .polling = poll, \
                                                               .CanHwObjectCount = 1U, \
                                                               .tx = { \
                                                                   .paddingVal = padding##U, \
                                                                   } \
                                                              }

enum {
    RX0,
    RX1,
    RX2,
    RX3,
    RX4,
    RX5,
    RX6,
    RX7,
    RX8,
    RX9,
    RX10,
    RX11,
    RX12,
    RX13,
    RX14,
    RX15,
    RX16,
    RX17,
    RX18,
    RX19,
    RX20,
    RX21,
    RX22,
    RX23,
    RX24,
    RX25,
    RX26,
    RX27,
#if MAX_FLEXCAN_CH > 4
    RX28,
    RX29,
    RX30,
    RX31,
    RX32,
    RX33,
    RX34,
    RX35,
    RX36,
    RX37,
    RX38,
    RX39,
    RX40,
    RX41,
    RX42,
    RX43,
    RX44,
    RX45,
    RX46,
    RX47,
    RX48,
    RX49,
    RX50,
    RX51,
    RX52,
    RX53,
    RX54,
    RX55,
#if MAX_FLEXCAN_CH > 8
    RX56,
    RX57,
    RX58,
    RX59,
    RX60,
    RX61,
    RX62,
    RX63,
    RX64,
    RX65,
    RX66,
    RX67,
    RX68,
    RX69,
    RX70,
    RX71,
    RX72,
    RX73,
    RX74,
    RX75,
    RX76,
    RX77,
    RX78,
    RX79,
    RX80,
    RX81,
    RX82,
    RX83,
    RX84,
    RX85,
    RX86,
    RX87,
    RX88,
    RX89,
    RX90,
    RX91,
    RX92,
    RX93,
    RX94,
    RX95,
    RX96,
    RX97,
    RX98,
    RX99,
    RX100,
    RX101,
    RX102,
    RX103,
    RX104,
    RX105,
    RX106,
    RX107,
    RX108,
    RX109,
    RX110,
    RX111,
    RX112,
    RX113,
    RX114,
    RX115,
    RX116,
    RX117,
    RX118,
    RX119,
    RX120,
    RX121,
    RX122,
    RX123,
    RX124,
    RX125,
    RX126,
    RX127,
    RX128,
    RX129,
    RX130,
    RX131,
    RX132,
    RX133,
    RX134,
    RX135,
    RX136,
    RX137,
    RX138,
    RX139,
#endif
#endif

    NUM_OF_HRHS,

    TX0 = 0,
    TX1,
    TX2,
    TX3,
    TX4,
    TX5,
    TX6,
    TX7,
    TX8,
    TX9,
    TX10,
    TX11,
    TX12,
    TX13,
    TX14,
    TX15,
    TX16,
    TX17,
    TX18,
    TX19,
    TX20,
    TX21,
    TX22,
    TX23,
    TX24,
    TX25,
    TX26,
    TX27,
#if MAX_FLEXCAN_CH > 4
    TX28,
    TX29,
    TX30,
    TX31,
    TX32,
    TX33,
    TX34,
    TX35,
    TX36,
    TX37,
    TX38,
    TX39,
    TX40,
    TX41,
    TX42,
    TX43,
    TX44,
    TX45,
    TX46,
    TX47,
    TX48,
    TX49,
    TX50,
    TX51,
    TX52,
    TX53,
    TX54,
    TX55,
#if MAX_FLEXCAN_CH > 8
    TX56,
    TX57,
    TX58,
    TX59,
    TX60,
    TX61,
    TX62,
    TX63,
    TX64,
    TX65,
    TX66,
    TX67,
    TX68,
    TX69,
    TX70,
    TX71,
    TX72,
    TX73,
    TX74,
    TX75,
    TX76,
    TX77,
    TX78,
    TX79,
    TX80,
    TX81,
    TX82,
    TX83,
    TX84,
    TX85,
    TX86,
    TX87,
    TX88,
    TX89,
    TX90,
    TX91,
    TX92,
    TX93,
    TX94,
    TX95,
    TX96,
    TX97,
    TX98,
    TX99,
    TX100,
    TX101,
    TX102,
    TX103,
    TX104,
    TX105,
    TX106,
    TX107,
    TX108,
    TX109,
    TX110,
    TX111,
    TX112,
    TX113,
    TX114,
    TX115,
    TX116,
    TX117,
    TX118,
    TX119,
    TX120,
    TX121,
    TX122,
    TX123,
    TX124,
    TX125,
    TX126,
    TX127,
    TX128,
    TX129,
    TX130,
    TX131,
    TX132,
    TX133,
    TX134,
    TX135,
    TX136,
    TX137,
    TX138,
    TX139,
#endif
#endif

    NUM_OF_HTHS
};

extern const Can_ConfigType gCan_Config;

#endif