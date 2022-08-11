/********************************************************
 *	    Copyright(c) 2019	Semidrive  Semiconductor    *
 *	    All rights reserved.                            *
 ********************************************************/

#ifndef __EZUSB_H__
#define __EZUSB_H__

#include <dev/usb.h>
#include "dev/usbc.h"

int ezusb_tx(const U8 *buffer, unsigned int size);
int ezusb_rx(U8 *to, size_t sz);
int ezusb_init(U32 interface_num, ep_t epin, ep_t epout);
bool ezusb_is_online(void);
void cancel_tansfers(ep_t ep,bool dir);

#endif  /* __EZUSB_H__ */
