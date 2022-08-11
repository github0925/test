/*
 * Copyright (c) 2018  Semidrive
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <assert.h>
#include <platform.h>
#include <err.h>
#include <lib/reg.h>
#include <sys/types.h>
#include <kernel/spinlock.h>
#include <kernel/mutex.h>
#include <__regs_base.h>
#include "target_res.h"
#include "mb_regs.h"
#include "mb_controller.h"
#include "mb_msg.h"

/* reuse masterid table */
#include <dev/sd_ddr_pfmon.h>

/* mbox controller features */
#define CONFIG_STATIC_MBC       (1)

#define MB_MAX_RPROC            (8)
#define MB_MAX_CHANS            (16)
#define MB_MAX_NAMES            (16)
#define MB_MAX_MSGS             (4)
#define MB_BUF_LEN              (4096)
#define MB_BANK_LEN             (MB_BUF_LEN/MB_MAX_MSGS)

#define MB_MSG_REGs_PER_MSG     (3U)
#define MB_MSG_REGs_PER_CPU     (MB_MSG_REGs_PER_MSG * 4)
#define MB_MSGID_INVALID        (0xff)
#define MB_ADDR_ANY             (0xdeadceefU)
#define MB_ADDR_ECHO_TEST       (0x4)
#define MB_TMH_RESET_VAL        0x02U

#define MU_MASTERID_OFF         0x500U
#define MU_TX_BUF_BASE          0x1000U
#define MU_TX_BUF_SIZE          0x1000U
#define MU_RX_BUF_BASE          0x2000U
#define MU_RX_BUF_SIZE          MU_TX_BUF_SIZE
#define MU_RX_BUF_OFF(cpu)      (MU_RX_BUF_SIZE * (cpu))


struct sd_mbox_tx_msg {
    bool used;
    unsigned msg_id;
    unsigned length;
    u32 client;
    unsigned remote;
    addr_t tmh;
    addr_t tmc;
    addr_t tx_buf;
};

struct sd_mbox_device;

struct sd_mbox_chan {
    char chan_name[MB_MAX_NAMES];
    struct sd_mbox_tx_msg *msg;
    unsigned target;
    unsigned actual_size;
    unsigned protocol;
    bool priority;
    bool is_run;
    addr_t rmh;

    struct sd_mbox_device *mbox;
    u32 cl_data;
    u32 dest_addr;
};

typedef struct sd_mbox_device {
    addr_t reg_base;
    int curr_cpu;
    int irq;
    spin_lock_t msg_lock;
    struct sd_mbox_tx_msg tmsg[MB_MAX_MSGS];
    spin_lock_t mlink_lock;
    struct sd_mbox_chan mlink[MB_MAX_CHANS];
} mbox_controller_t;

#if CONFIG_STATIC_MBC
static mbox_controller_t mbc;
#endif
static mbox_controller_t *g_mb_ctl;

static inline u32 mu_get_rx_msg_len(struct sd_mbox_device *mu,
                                    int remote_proc, int msg_id)
{
    addr_t rmh = mu->reg_base + CPU0_MSG0_RMH0_OFF +
                 4 * (remote_proc * MB_MSG_REGs_PER_CPU + msg_id * MB_MSG_REGs_PER_MSG);

    return GFV_CPU0_MSG0_RMH0_CPU0_MSG0_LEN(readl(rmh)) * 2;
}

static inline u32 mu_get_msg_rmh1(struct sd_mbox_device *mu,
                                    int remote_proc, int msg_id)
{
    addr_t rmh = mu->reg_base + CPU0_MSG0_RMH1_OFF +
             4 * (remote_proc * MB_MSG_REGs_PER_CPU + msg_id * MB_MSG_REGs_PER_MSG);

    return readl(rmh);
}

static inline u32 mu_get_msg_rmh2(struct sd_mbox_device *mu,
                                    int remote_proc, int msg_id)
{
    addr_t rmh = mu->reg_base + CPU0_MSG0_RMH2_OFF +
             4 * (remote_proc * MB_MSG_REGs_PER_CPU + msg_id * MB_MSG_REGs_PER_MSG);

    return readl(rmh);
}

static inline bool mu_is_rx_msg_ready(struct sd_mbox_device *mu,
                                      int remote_proc, int msg_id)
{
    u32 shift = remote_proc * 4 + msg_id;

    return ((readl(mu->reg_base + TMS_OFF) & (0x01UL << shift)) != 0);
}

static inline bool mu_is_rx_msg_valid(struct sd_mbox_device *mu,
                                      int remote_proc, int msg_id)
{
    addr_t rmh = mu->reg_base + CPU0_MSG0_RMH0_OFF +
                 4 * (remote_proc * MB_MSG_REGs_PER_CPU + msg_id * MB_MSG_REGs_PER_MSG);

    return (readl(rmh) & BM_CPU0_MSG0_RMH0_CPU0_MSG0_VLD);
}

static int sd_mu_fill_tmh(struct sd_mbox_chan *mlink)
{
    struct sd_mbox_tx_msg *msg = mlink->msg;

    if (msg) {
        u32 tmh0 = FV_TMH0_TXMES_LEN((ALIGN(mlink->actual_size, 2)) / 2)
                   | FV_TMH0_MID(msg->msg_id);
        u32 reset_check;

        tmh0 |= FV_TMH0_MDP(1 << mlink->target);
        tmh0 |= BM_TMH0_TXUSE_MB | FV_TMH0_MBM(1 << msg->msg_id);

        writel(tmh0, msg->tmh);
        /* use tmh1 to identify source address */
        writel(mlink->cl_data, msg->tmh + TMH1_OFF);
        /* use tmh2 to identify destination address */
        writel(mlink->dest_addr, msg->tmh + TMH2_OFF);

        reset_check = readl(msg->tmh);
        if (reset_check != tmh0) {
            dprintf(2, "mu: fill tmh %x != %x\n", tmh0, reset_check);
            return ERR_NOT_READY;
        }

        return 0;
    }

    return ERR_NO_MSG;
}

/* write message to tx buffer */
static u32 sd_mu_write_msg(struct sd_mbox_chan *mlink, void *data)
{
    struct sd_mbox_tx_msg *msg = mlink->msg;

    if (msg) {
        memcpy((void *)msg->tx_buf, data, mlink->actual_size);
    }

    return 0;
}

static int sd_mu_write_rom_msg(struct sd_mbox_chan *mlink, u8 *data)
{
    struct sd_mbox_tx_msg *msg = mlink->msg;
    u64 val = 0ULL;
    int i;
    u8 *ptr;
    u32 len;

    ptr = mb_msg_payload_ptr(data);
    len = mb_msg_payload_size(data);

    if (len > 8) {
        dprintf(ALWAYS, "rom msg len %d larger than expected\n", len);
        return ERR_BAD_LEN;
    }

    for (i = 0; i < len; i++) {
        val |= ((u64) ptr[i] << (i * 8));
    }

    if (msg) {
        u32 tmh0 = FV_TMH0_TXMES_LEN((ALIGN(len, 2)) / 2)
                   | FV_TMH0_MID(msg->msg_id);
        u32 reset_check;

        tmh0 |= FV_TMH0_MDP(1 << mlink->target);
        tmh0 |= BM_TMH0_TXUSE_MB | FV_TMH0_MBM(1 << msg->msg_id);

        writel(tmh0, msg->tmh);
        writel((u32)val, msg->tmh + TMH1_OFF);
        writel((u32)(val >> 32), msg->tmh + TMH2_OFF);

        reset_check = readl(msg->tmh);
        if (reset_check != tmh0) {
            return ERR_NOT_READY;
        }

        /* this is to compatible with default msg,
         * for test only
         */
        memcpy((void *)msg->tx_buf, data, mlink->actual_size);

        return 0;
    }

    return ERR_NO_MSG;
}

/* message read from rx buffer */
void* sd_mu_get_read_ptr(struct sd_mbox_device *mbox, int remote_proc, int msg_id)
{
    addr_t rxb = mbox->reg_base + MU_RX_BUF_BASE;
    return (u8 *) rxb + MU_RX_BUF_OFF(remote_proc) +
        msg_id * MB_BANK_LEN;
}

/* message read from rx buffer */
u32 sd_mu_read_msg(struct sd_mbox_device *mbox, int remote_proc, int msg_id,
                    u8 *data, int len)
{
    addr_t rxb = mbox->reg_base + MU_RX_BUF_BASE;
    memcpy(data, (u8 *) rxb + MU_RX_BUF_OFF(remote_proc) +
        msg_id * MB_BANK_LEN, len);

    return len;
}
/*
 * message acknowledgement, will clear RMH valid bit & TSEND & irq
 */
u32 sd_mu_ack_msg(struct sd_mbox_device *mbox, int remote_proc, int msg_id)
{
    u32 shift = remote_proc * 4 + msg_id;
    writel(readl(mbox->reg_base + RMC_OFF) | (0x01UL << shift),
           mbox->reg_base + RMC_OFF);
    return 0;
}

u32 sd_mu_read_tms(struct sd_mbox_device *mbox)
{
    return readl(mbox->reg_base + TMS_OFF);
}

static void sd_mu_cancel_msg(struct sd_mbox_tx_msg *msg)
{
    writel(readl(msg->tmc) | BM_TMC0_TMC0_MSG_CANCEL, msg->tmc);
}

static int sd_mu_send_msg(struct sd_mbox_tx_msg *msg)
{
    writel(readl(msg->tmc) | BM_TMC0_TMC0_MSG_SEND, msg->tmc);

    return 0;
}

static bool sd_mu_is_msg_sending(struct sd_mbox_tx_msg *msg)
{
    return (!!(readl(msg->tmc) & BM_TMC0_TMC0_MSG_SEND));
}

struct sd_mbox_tx_msg *sd_mu_alloc_msg(struct sd_mbox_device *mbdev,
                                       int prefer, bool priority)
{
    int i;
    struct sd_mbox_tx_msg *msg;

    if (prefer < MB_MAX_MSGS) {
        msg = &mbdev->tmsg[prefer];

        if (!msg->used) {
            msg->used = 1;
            return msg;
        }
    }

    for (i = 0; i < MB_MAX_MSGS; i++) {
        msg = &mbdev->tmsg[i];

        if (!msg->used) {
            msg->used = 1;
            return msg;
        }
    }

    dprintf(0, "mu: all msg used:\n");
    for (i = 0; i < MB_MAX_MSGS; i++) {
        msg = &mbdev->tmsg[i];

        dprintf(0, "msg[%d]used:%d target:%d\n", i, msg->client, msg->remote);
    }

    return NULL;
}

void sd_mu_free_msg(struct sd_mbox_device *mbdev,
                    struct sd_mbox_tx_msg *msg)
{
    msg->used = 0;
    msg->remote = (u32) - 1;
}

int sd_mbox_send_data(struct sd_mbox_chan *mlink, u8 *data)
{
    struct sd_mbox_device *mbdev = mlink->mbox;
    int prefer_msg;
    int ret = 0;
    spin_lock_saved_state_t flags;

    spin_lock_irqsave(&mbdev->msg_lock, flags);

    mlink->actual_size = mb_msg_parse_packet_len(data);
    mlink->priority = mb_msg_parse_packet_prio(data);
    mlink->protocol = mb_msg_parse_packet_proto(data);
    mlink->dest_addr = mb_msg_parse_addr(data);

    /*
     * for safety ROM peer load, we use msg_id 1 according to ROM spec
     */
    prefer_msg = (mlink->protocol == MB_MSG_PROTO_ROM) ? 1 : 0;

    if (!mlink->msg) {
        mlink->msg = sd_mu_alloc_msg(mbdev, prefer_msg, mlink->priority);
        if (!mlink->msg) {
            spin_unlock_irqrestore(&mbdev->msg_lock, flags);
            return ERR_NO_MSG;
        }

        mlink->msg->remote = mlink->target;
        mlink->msg->client = mlink->cl_data;
    }

    dprintf(SPEW, "mu: send_data proto: %d length: %d msg: %d\n",
            mlink->protocol, mlink->actual_size, mlink->msg->msg_id);

    if (MB_MSG_PROTO_ROM == mlink->protocol) {
        /* for ROM peer load */
        ret = sd_mu_write_rom_msg(mlink, data);
    } else if (MB_MSG_PROTO_DSP == mlink->protocol) {
        /* for vdsp fastavm */
        ret = sd_mu_write_rom_msg(mlink, data);
    } else {
        ret = sd_mu_fill_tmh(mlink);
        sd_mu_write_msg(mlink, data);
    }

    if (!ret)
        sd_mu_send_msg(mlink->msg);
    else {
        /* if fail the send anyway, free this msg */
#if !SUPPORT_FAST_BOOT
        dprintf(ALWAYS, "mu: rproc %d unreachable\n", mlink->target);
#endif
        sd_mu_free_msg(mbdev, mlink->msg);
        mlink->msg = NULL;
    }

    spin_unlock_irqrestore(&mbdev->msg_lock, flags);

    return ret;
}

static void sd_mu_check_reset(struct sd_mbox_tx_msg *msg)
{
    if (readl(msg->tmh) == MB_TMH_RESET_VAL) {
        dprintf(ALWAYS, "mu: warning rproc%d is reset\n", msg->remote);
    }
}

int sd_mbox_cancel_lastsend(struct sd_mbox_chan *mlink)
{
    struct sd_mbox_device *mbdev = mlink->mbox;

    if (mlink->msg) {
        sd_mu_cancel_msg(mlink->msg);
        if (sd_mu_is_msg_sending(mlink->msg))
            return ERR_BUSY;

        /* MU hw trigger tmh0-reset after cancel a msg */
        /* use func sd_mu_check_reset(mlink->msg) to test */
        dprintf(INFO, "mu: lastmsg canceled\n");
        sd_mu_free_msg(mbdev, mlink->msg);
        mlink->msg = NULL;
        return 0;
    }

    dprintf(CRITICAL, "mu: sd_mbox_cancel_lastsend not be here\n");
    return ERR_NO_MSG;
}

int sd_mbox_startup(struct sd_mbox_chan *mlink)
{
    mlink->is_run = true;

    return 0;
}

void sd_mbox_shutdown(struct sd_mbox_chan *mlink)
{
    struct sd_mbox_device *mbdev = mlink->mbox;

    if (mlink->msg) {
        if (sd_mu_is_msg_sending(mlink->msg)) {
            sd_mu_cancel_msg(mlink->msg);
        }

        sd_mu_free_msg(mbdev, mlink->msg);
        mlink->msg = NULL;
    }

    mlink->is_run = false;
}

bool sd_mbox_last_tx_done(struct sd_mbox_chan *mlink)
{
    struct sd_mbox_tx_msg *msg = mlink->msg;
    struct sd_mbox_device *mbdev = mlink->mbox;

    if (!sd_mu_is_msg_sending(mlink->msg)) {
        sd_mu_free_msg(mbdev, mlink->msg);
        mlink->msg = NULL;
        return true;
    }

    return false;
}

/*
 * only use the simplest way to allocate channel so far
 */
struct sd_mbox_chan *sd_mbox_request_channel(u8 rproc, u32 cl_addr)
{
    struct sd_mbox_chan *mlink;
    int i;

    if (rproc >= MB_MAX_RPROC)
        return NULL;

    /* TODO: to dynamic allocate channel to remote proc
     * currently use a simple indexing
     */
    spin_lock(&g_mb_ctl->mlink_lock);
    for (i = 0; i < MB_MAX_CHANS; i++) {
        mlink = &g_mb_ctl->mlink[i];
        if (mlink->target == 0xff) {
            mlink->target = rproc;
            mlink->cl_data = cl_addr;
            spin_unlock(&g_mb_ctl->mlink_lock);
            return mlink;
        }
    }
    spin_unlock(&g_mb_ctl->mlink_lock);
    return NULL;
}

int sd_mbox_free_channel(struct sd_mbox_chan *mlink)
{
    spin_lock(&g_mb_ctl->mlink_lock);
    if (mlink->target < MB_MAX_CHANS) {
        mlink->target = 0xff;
        mlink->cl_data = 0;
        spin_unlock(&g_mb_ctl->mlink_lock);
        return 0;
    }
    spin_unlock(&g_mb_ctl->mlink_lock);
    printf("mu: bad mchan %d: %s\n", mlink->target, mlink->chan_name);
    return -1;
}

inline static int ffs(int x)
{
    return __builtin_ffs(x) - 1;
}

/* Only for unit test */
void sd_mu_echo_test(struct sd_mbox_device *mbdev, u32 rproc, u32 msg_id)
{
    struct sd_mbox_tx_msg *msg;
    u32 from = mu_get_msg_rmh1(mbdev, rproc, msg_id);
    u32 to = mu_get_msg_rmh2(mbdev, rproc, msg_id);
    u32 len = mu_get_rx_msg_len(mbdev, rproc, msg_id);
    u32 msg_id_tx = msg_id+1;

    if (msg_id_tx > 3)
        msg_id_tx = 0;

    msg = sd_mu_alloc_msg(mbdev, msg_id_tx, 0);
    if (msg) {
        u32 tmh0 = FV_TMH0_TXMES_LEN((ALIGN(len, 2)) / 2)
                   | FV_TMH0_MID(msg->msg_id);
        msg->remote = rproc;

        tmh0 |= FV_TMH0_MDP(1 << rproc);
        tmh0 |= BM_TMH0_TXUSE_MB | FV_TMH0_MBM(1 << msg->msg_id);

        writel(tmh0, msg->tmh);
        /* use tmh1 to identify source address */
        writel(to, msg->tmh + TMH1_OFF);
        /* use tmh2 to identify destination address */
        writel(from, msg->tmh + TMH2_OFF);
        memcpy((void *)msg->tx_buf, sd_mu_get_read_ptr(mbdev, rproc, msg_id), len);
        sd_mu_send_msg(msg);

        while(sd_mu_is_msg_sending(msg));
        sd_mu_free_msg(mbdev, msg);
    }
    dprintf(INFO, "mu: mbox echo test done\n");
}

static void sd_mbox_process_msg(struct sd_mbox_device *mbdev, int remote_proc, int msg_id, chan_rx_cb_t receive_cb)
{
    sd_msghdr_t *msghdr;
    struct sd_mbox_chan *mlink;

    /* Callback to HAL layer */
    if (receive_cb) {
        u32 from = mu_get_msg_rmh1(mbdev, remote_proc, msg_id);
        u32 to = mu_get_msg_rmh2(mbdev, remote_proc, msg_id);
        u32 len = mu_get_rx_msg_len(mbdev, remote_proc, msg_id);

        if (remote_proc == MASTER_VDSP) {
            /* vdsp use rmh1 & rmh2 to transfer short message */
            dprintf(SPEW, "dsp: rmh1: 0x%lx rmh2: 0x%lx\n", from, to);
            receive_cb(remote_proc, from, (void*)(long)to, len);
            return;
        }

        msghdr = (sd_msghdr_t *) sd_mu_get_read_ptr(mbdev,
                                    remote_proc, msg_id);
        /* protocol default is callback user
         * otherwise is rom code test
         */
        if (MB_MSG_PROTO_ROM != msghdr->protocol) {
        #ifdef CONFIG_FPGA_SELFTEST
            if (to == MB_ADDR_ANY) {
                mlink = &mbdev->mlink[remote_proc];
                to = mlink->cl_data;
            }
        #endif
        } else {
            printf("mu: suppose not ROM msg\n");
            return;
        }

        if (to == MB_ADDR_ECHO_TEST) {
            /* mbox self echo test only */
            sd_mu_echo_test(mbdev, remote_proc, msg_id);
        } else {
            /* len is 2 bytes aligned, use actual len */
            receive_cb(remote_proc, to, msghdr, msghdr->dat_len);
        }
    }
}

enum handler_return sd_mbox_rx_interrupt(int irq, chan_rx_cb_t receive_cb)
{
    struct sd_mbox_device *mbdev = g_mb_ctl;
    unsigned int state, msg_id;
    u32 remote_proc, mmask;
    spin_lock_saved_state_t flags;

    if (!mbdev) {
        return INT_NO_RESCHEDULE;
    }

    spin_lock_irqsave(&mbdev->msg_lock, flags);

    state = sd_mu_read_tms(mbdev);
    if (!state) {
        spin_unlock_irqrestore(&mbdev->msg_lock, flags);
        dprintf(ALWAYS, "%s: spurious interrupt %d\n",
                __func__, irq);
        return INT_NO_RESCHEDULE;
    }

    dprintf(2, "mu: rx intr state: 0x%x\n", state);

    for (remote_proc = 0; remote_proc < MB_MAX_RPROC; remote_proc++) {
        mmask = 0xf & (state >> (4 * remote_proc));

        while (mmask) {
            msg_id = ffs(mmask); /* this bit indicates msg id */
            mmask &= (mmask - 1);

            dprintf(SPEW, "mu: rproc: %d msg: %d\n", remote_proc, msg_id);
            sd_mbox_process_msg(mbdev, remote_proc, msg_id, receive_cb);
            sd_mu_ack_msg(mbdev, remote_proc, msg_id);
            dprintf(SPEW, "mu: ack rproc: %d msg: %d\n", remote_proc, msg_id);
        }
    }
    spin_unlock_irqrestore(&mbdev->msg_lock, flags);

    return INT_RESCHEDULE;
}

/*
 * map master_ids to cpu_id. Pls note that all the four mid for this 'cpu' as
 * same.
 * cpu_id: cpux
 * mid:    masterid, set id0-id3 all the same
 * lock:   lock mid if set
 */
int sd_mbox_config_master(struct sd_mbox_device *mbdev, u8 cpu_id, u8 mid,
                          bool lock)
{
    addr_t cpu_mid_reg = mbdev->reg_base + MU_MASTERID_OFF + 4 * cpu_id;
    u32 val = readl(cpu_mid_reg);
    u32 v;

    if (val & 0x80808080) {
        dprintf(INFO, "Opps, cpu_mid_reg[%d] been locked.\n", cpu_id);
        return -1;
    }

    v = mid & 0x7FU;
    val = v | (v << 8) | (v << 16) | (v << 24);

    if (lock) {
        val |= 0x80808080;
    }

    writel(val, cpu_mid_reg);
    return 0;
}

int sd_mbox_probe(addr_t phyaddr)
{
    struct sd_mbox_device *mbdev;
    int i;

    if (g_mb_ctl) {
        dprintf(ALWAYS, "Semidrive Mailbox already registered\n");
        return 0;
    }

#if CONFIG_STATIC_MBC
    mbdev = &mbc;
#else
    mbdev = malloc(sizeof(*mbdev));
    if (mbdev == 0) {
        dprintf(INFO, "Opps, no memory\n");
        return -1;
    }
#endif

    mbdev->reg_base = (addr_t)_ioaddr(phyaddr);
    /* TODO: get the domain from domain res.h */
    mbdev->curr_cpu = MASTER_SAF_PLATFORM;
//    mbdev->irq = MU_MESSAGE_READY_INT;

    dprintf(INFO, "mu: mapped regbase PA:%p VA:%p\n", phyaddr, mbdev->reg_base);
    spin_lock_init(&mbdev->mlink_lock);
    for (i = 0; i < MB_MAX_CHANS; i++) {
        mbdev->mlink[i].is_run = 0;
        mbdev->mlink[i].msg = NULL;
        mbdev->mlink[i].priority = (i == 0) ? true : false;
        mbdev->mlink[i].protocol = 0;
        mbdev->mlink[i].target = 0xff; /* assigned during request channel */
        mbdev->mlink[i].mbox = mbdev;
        snprintf(mbdev->mlink[i].chan_name, MB_MAX_NAMES, "hwchan%d", i);
    }

    spin_lock_init(&mbdev->msg_lock);

    for (i = 0; i < MB_MAX_MSGS; i++) {
        mbdev->tmsg[i].used = 0;
        mbdev->tmsg[i].msg_id = i;
        mbdev->tmsg[i].client = 0;
        mbdev->tmsg[i].tmh = mbdev->reg_base + TMH0_OFF;
        mbdev->tmsg[i].tmc = mbdev->reg_base + TMC0_OFF + 4 * i;
        mbdev->tmsg[i].tx_buf = mbdev->reg_base + MU_TX_BUF_SIZE +
                                MB_BANK_LEN * i;
    }

    /* request IRQ is OS related, move to HAL */
    /* register IRQ in HAL level */

    /* lock master in rom code, here we don't need do again */
#if CONFIG_LOCK_MASTER
    sd_mbox_config_master(mbdev, MASTER_SAF_PLATFORM, MASTER_SAF_PLATFORM,
                          false); //0,saf, gic1 int232
    sd_mbox_config_master(mbdev, MASTER_SEC_PLATFORM, MASTER_SEC_PLATFORM,
                          false); //1,sec, gic2 int232
    sd_mbox_config_master(mbdev, MASTER_MP_PLATFORM,  MASTER_MP_PLATFORM,
                          false); //2,mp,  gic3 int232
    sd_mbox_config_master(mbdev, MASTER_AP1,  MASTER_AP1,
                          false);                 //3,cpu1,gic4 int232
    sd_mbox_config_master(mbdev, MASTER_AP2,  MASTER_AP2,
                          false);                 //4,cpu2,gic5 int232
    sd_mbox_config_master(mbdev, MASTER_VDSP, MASTER_VDSP,
                          false);                 //5,vsn, int22
    sd_mbox_config_master(mbdev, MASTER_ADSP, MASTER_ADSP,
                          false);                 //6,need to fill all 8 cpus
    sd_mbox_config_master(mbdev, MASTER_TCU,  MASTER_TCU,
                          false);                 //7
#endif

    g_mb_ctl = mbdev;

    dprintf(INFO, "mbc initialized\n");

    return 0;
}

int sd_mbox_remove()
{
    struct sd_mbox_device *mdev = g_mb_ctl;

#if !CONFIG_STATIC_MBC
    if (mdev) {
        free(mdev);
    }
#endif
    g_mb_ctl = NULL;

    return 0;
}

