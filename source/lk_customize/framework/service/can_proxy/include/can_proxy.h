#ifndef __CAN_PROXY_H__
#define __CAN_PROXY_H__

#include <stdio.h>
#include <stdint.h>

#include "can_proxy_buffer.h"
#include "can_proxy_bundle.h"

#define CPDBG_EN    0

#if CPDBG_EN
#define CPDBG(...) printf(__VA_ARGS__)
#else
#define CPDBG(...)
#endif

/* port declaration */

/* can */

/* CAN Frame Max Data Length */
#define CP_CAN_MAX_LEN          (64U)
/* CAN Frame Type(0:DATA or 1:REMOTE). */
#define CP_CAN_FLAG_TYPE        (1 << 29)
/* CAN FD or classic frame? 0: classic or 1: canfd */
#define CP_CAN_FLAG_CANFD       (1 << 30)
/* CAN Frame Identifier(0: STD or 1: EXT format). */
#define CP_CAN_FLAG_FMT         (1 << 31)
struct cp_can_frame {
    u32 can_id;  /* 32 bit CAN_ID + FORMAT/CANFD/TYPE flags */
    u8    len;     /* frame payload length in byte */
    u8    data[CP_CAN_MAX_LEN];
}__attribute__((packed));

#define CP_CAN_MTU	(sizeof(struct cp_can_frame))

int can_if_tx(struct cp_if_t* cpif,void* from, int len);
int can_if_init(struct cp_if_t* cpif);
int can_if_rx_cb(struct cp_if_t* cpif, void* data, int len);

/* rpmsg */

int rpmsg_if_init(struct cp_if_t* cpif);
int rpmsg_if_rxcb(struct cp_if_t* cpif, void* data, int len);
int rpmsg_if_tx(struct cp_if_t* cpif,void* from, int len);

/* Bundle Rule generation */

//bundle_id = bus_id.if_id
// | bus 3 | bus 2 | ... |FIXED_POINT|if 3 | if 2 | ... | if 0|
// | bit 31| bit 30|... |FIXED_POINT |bit 3| bit 2| ... | bit 0|
#define GEN_BUNDLE_ID(bus_bitmap,if_bitmap) (bus_bitmap | if_bitmap)
#define REV_BUNDLE_ID(bundle_id,bus_bitmap,if_bitmap) do {\
    bus_bitmap = bundle_id & (0xffffffff << BITMAP_FIXED_POINT); \
    if_bitmap = bundle_id & (0xffffffff >> (31 - BITMAP_FIXED_POINT) ); \
}while(0)

/* bundles definition */
/* A bundle that send to can x interface. Usually comes from rpmsg. */
#define TARGET_CAN1_BUNDLE   GEN_BUNDLE_ID(BUS_BITMAP(cpifBusCan),INTERFACE_ID_BITMAP(0))
#define TARGET_CAN2_BUNDLE   GEN_BUNDLE_ID(BUS_BITMAP(cpifBusCan),INTERFACE_ID_BITMAP(1))
#define TARGET_CAN3_BUNDLE   GEN_BUNDLE_ID(BUS_BITMAP(cpifBusCan),INTERFACE_ID_BITMAP(2))
#define TARGET_CAN4_BUNDLE   GEN_BUNDLE_ID(BUS_BITMAP(cpifBusCan),INTERFACE_ID_BITMAP(3))
/* A bundle that broadcast to can interface. */
#define TARGET_CAN_BUNDLE  ( (TARGET_CAN1_BUNDLE) | (TARGET_CAN2_BUNDLE) | (TARGET_CAN3_BUNDLE) | (TARGET_CAN4_BUNDLE) )

/* A bundle that send to ivi interface. Usually comes from can/spi. */
#define TARGET_IVI_BUNDLE   GEN_BUNDLE_ID(BUS_BITMAP(cpifBusRPMSG),INTERFACE_ID_BITMAP(0))
/* A bundle that send to cluster interface. Usually comes from can/spi. */
#define TARGET_CLU_BUNDLE   GEN_BUNDLE_ID(BUS_BITMAP(cpifBusRPMSG),INTERFACE_ID_BITMAP(2))
/* A bundle that send to vm cluster interface. Usually comes from can/spi. */
#define TARGET_VCLU_BUNDLE   GEN_BUNDLE_ID(BUS_BITMAP(cpifBusRPMSG),INTERFACE_ID_BITMAP(1))

#define TARGET_AP_BUNDLE ((TARGET_IVI_BUNDLE) | (TARGET_CLU_BUNDLE) | (TARGET_VCLU_BUNDLE))
#define TARGET_VM_BUNDLE ((TARGET_IVI_BUNDLE) | (TARGET_VCLU_BUNDLE))

extern const uint32_t bundle_rules[];

#endif