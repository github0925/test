#ifndef __H_VIRT_COM_H__
#define __H_VIRT_COM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "target_res.h"
#include "virCom.h"
#include "virComCbk_server.h"
#include "virComCbk.h"

#define VIRCOM_DEBUG INFO

#define VIRCOM_CLIENT_ADDR          0x0A01
#define VIRCOM_SERVER_ADDR          0x0A02

#define VIRCOM_CBK_CLIENT_ADDR      0x0B01
#define VIRCOM_CBK_SERVER_ADDR      0x0B02

#define VIRCOM_SHM_BASE             (void *)(IRAM4_BASE + 0x3C000)
#define VIRCOM_SHM_SIZE             0x1000

#define VIRCOM_CBK_SHM_BASE         (void *)((uint32_t)VIRCOM_SHM_BASE + VIRCOM_SHM_SIZE)
#define VIRCOM_CBK_SHM_SIZE         0x1000

#define VIRCAN_NUM_OF_CTRL_MAX      20
#define VIRCAN_NUM_OF_HRHS_MAX      512
#define VIRCAN_NUM_OF_HTHS_MAX      512
#define VIRCAN_NUM_OF_RXFIFOS_MAX   512

bool vircom_init(void);

#ifdef __cplusplus
}
#endif

#endif
