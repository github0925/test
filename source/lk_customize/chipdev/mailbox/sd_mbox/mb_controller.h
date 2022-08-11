/*
 * Copyright (c) 2018  Semidrive
 */
#include <sys/types.h>

struct sd_mbox_chan;
struct sd_mbox_device;

int sd_mbox_probe(addr_t phyaddr);
int sd_mbox_remove(void);
int sd_mbox_config_master(struct sd_mbox_device *mbdev, u8 cpu_id, u8 mid,
                          bool lock);
struct sd_mbox_chan *sd_mbox_request_channel(u8 rproc, u32 data);
int sd_mbox_free_channel(struct sd_mbox_chan *mlink);
int sd_mbox_send_data(struct sd_mbox_chan *mlink, u8 *data);
int sd_mbox_cancel_lastsend(struct sd_mbox_chan *mlink);
int sd_mbox_startup(struct sd_mbox_chan *mlink);
void sd_mbox_shutdown(struct sd_mbox_chan *mlink);
bool sd_mbox_last_tx_done(struct sd_mbox_chan *mlink);

static inline void sd_mbox_wait_tx_done(struct sd_mbox_chan *mlink)
{
    while (!sd_mbox_last_tx_done(mlink));
}

typedef void (*chan_rx_cb_t)(u32 src, u32 dst, void *mssg, u16 len);

enum handler_return sd_mbox_rx_interrupt(int irq, chan_rx_cb_t cb);

