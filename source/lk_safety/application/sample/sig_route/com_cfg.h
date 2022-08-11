#include "com.h"

#ifndef COM_CFG_H__
#define COM_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EGSM_1_EGSM_PutLeverInAutoTip_handle   (&egsm_1.sig_rule[0].src_sig)

extern frame_desc_t egsm_1;
extern frame_desc_t acm_1;
extern frame_desc_t tcu_4;
extern frame_desc_t lin_mcp_1;
extern frame_desc_t gw_atc_1;

extern frame_desc_t *g_rx_frame_cfg[];
extern frame_desc_t *g_tx_frame_cfg[];

#ifdef __cplusplus
}
#endif

#endif
