/********************************************************
 *	        Copyright(c) 2018	Semidrive 		        *
 *******************************************************/

#include <common_hdr.h>
#include <soc.h>
#include <arch.h>
#include <atb_test/atb_test.h>

#define MPS     512

static void buf_dump(U8 *b, size_t sz)
{
#if defined(VTEST_USB)
    int c = 16, dump = 0;
    while (sz  > c) {
        DBG("@%08x: ", dump);
        for (int i = 0; i < c; i++) {
            io_printf("%02x ", b[i]);
        }
        b += c;
        sz -= c;
        dump += c;
        io_printf("\n");
    }
    if (sz) {
        DBG("@%08x: ", dump);
        for (int i = 0; i < sz; i++) {
            io_printf("%02x ", b[i]);
        }
        io_printf("\n");
    }
#endif
}

extern U32 ezusb_open(bt_dev_id_e dev_id, void *para);
extern U32 ezusb_read(void *dev, U32 from, U8 *to, U32 sz);
extern U32 ezusb_write(void *dev, U8 *from, U32 to, U32 sz);
extern int ezusb_rx(U8 *to, size_t sz);

unsigned int usbc_test_basic(unsigned char en, unsigned int para)
{
    U32 res = -1;

    DBG("%s: USB test running...\n", __FUNCTION__);
    do {
        if(NULL == bt_dev_open(BT_DEV_USB, NULL)) {
            break;
        }
        DBG("%s: USB device been opened.\n", __FUNCTION__);
        U8 rx[MPS*2];
        int i = 0;
        memset(rx, 0, sizeof(rx));
        clean_cache_range(rx, sizeof(rx));
        DBG("%s: Suppose Testbench sends one mps in one xfer\n", __FUNCTION__);
        int32_t bytes = ezusb_rx(rx, MPS);
        DBG("%s: %d bytes received.\n", __FUNCTION__, bytes);
        if(0 > bytes) {
            DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
            break;
        }
        for (i = 0; i < MPS; i++) {
            if(rx[i] != (U8)i) {
                DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
                break;
            }
        }
        if (i != MPS) break;

        DBG("%s: Suppose Testbench sends 2 mps in one xfer*\n", __FUNCTION__);
        memset(rx, 0, sizeof(rx));
        DBG("%s: to rd 1 byte\n", __FUNCTION__);
        if (0 > ezusb_rx(rx, 1)) {
            DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
            break;
        } else {
            DBG("%s: the one byte is 0x%02x\n", __FUNCTION__, rx[0]);
        }
        DBG("%s: to rd 7 byte\n", __FUNCTION__);
        if (0 > ezusb_rx(rx + 1, 7)) {
            DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
            break;
        } else {
            DBG("%s: the 7 bytes are 0x%02x...0x%02x", __FUNCTION__, rx[1], rx[7]);
        }
        DBG("%s: to rd %d byte\n", __FUNCTION__, MPS - 8);
        if (0 > ezusb_rx(rx + 8, MPS - 8)) {
            DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
            break;
        }
        DBG("%s: to dump rx-ed bytes...\n", __FUNCTION__);
        buf_dump(rx, MPS);
        DBG("%s: to rd 512 bytes\n", __FUNCTION__);
        if (0 > ezusb_rx(rx + MPS, MPS)) {
            DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
            break;
        }
        DBG("%s: to dump rx-ed bytes...\n", __FUNCTION__);
        buf_dump(rx + MPS, MPS);
        for (i = 0; i < 2 * MPS; i++) {
            if(rx[i] != (U8)i) {
                DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
                break;
            }
        }
        if (i != 2 * MPS) break;

        DBG("%s: Suppose Testbench sends 0.5 mps in one xfer*\n", __FUNCTION__);
        memset(rx, 0, sizeof(rx));
        int n = ezusb_rx(rx, MPS/2);
        if (n != MPS / 2) {
            DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
            break;
        }
        for (i = 0; i < n; i++) {
            if(rx[i] != (U8)i) {
                DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
                break;
            }
        }
        if ( i != n) {
            break;
        }

        DBG("%s: Suppose TB sends 0.5 mps, then 0.5 msp\n", __FUNCTION__);
        memset(rx, 0, sizeof(rx));
        n = ezusb_rx(rx, MPS);
        if (n != MPS) {
            DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
            break;
        }
        for (i = 0; i < MPS; i++) {
            if(rx[i] != (U8)i) {
                DBG("%s: failed at line %d\n", __FUNCTION__, __LINE__);
                break;
            }
        }
        if (i != MPS) break;

        U8 tx[MPS*2];
        for (i = 0; i < sizeof(tx); i++) {
            tx[i] = ~i;
        }
        DBG("%s: send two mps data to host...\n", __FUNCTION__);
        ezusb_write(NULL, tx, 0, sizeof(tx));

        res = 0;
    } while (0);

    return res;
}

DCLR_ATB_TEST("usb basic test", FALSE,
              EN_BIT_ut_usb_dev_hs_bulk_in_test, usbc_test_basic)
