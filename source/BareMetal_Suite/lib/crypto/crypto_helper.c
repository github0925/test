/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <atb_crypto.h>
#include <atb_crypto.h>

#if defined(CORE_sec)
#include <RegBase.h>
#include <Mcu_Rstgen.h>
#include <Mcu_Scr.h>
#include <Mcu_ScrBits.h>

extern uint32_t sec_volation_flag;

void soc_assert_security_violation(void)
{
    /** TODO: disable interrupt for clear key in secure volation.*/
#define SECURE_CTRL_OFF     0x8000U
    uint32_t v = readl(CE2_REG_BASE + SECURE_CTRL_OFF);
    if ((v & (0x01u << 7)) == 0) {
        v |= (0x01u << 7);    /* i_scr_sec_sw_vio */
        writel(v, CE2_REG_BASE + SECURE_CTRL_OFF);
    }
    /* this scr signal also connects to storage2/ce1/ce2 */
    if (0 == Mcu_ScrGetBit(APB_SCR_SEC_BASE, RW,
                    SCR_SEC_CE2_SECURITY_VIOLATION_7_RW_WIDTH)) {
        Mcu_ScrSetBit(APB_SCR_SEC_BASE, RW,
                    SCR_SEC_CE2_SECURITY_VIOLATION_7_RW_WIDTH);
    }

    sec_volation_flag = 0xA5A5A5A5UL;

    ///* reset sec core*/
    //rg_glb_reset_en(APB_RSTGEN_SEC_BASE, 1);    // self sw reset enable
    //rg_glb_self_reset(APB_RSTGEN_SEC_BASE, 1);
}
#endif

uint32_t crypto_get_aead_hdr_sz(cipher_mode_e mode, const uint8_t *in, uint32_t i_sz,
                                uint32_t *hdr_sz)
{
    uint32_t sz = 0;
    uint32_t aad_sz = 0;

    if (MODE_CCM == mode) {
        if (i_sz < 32) {
            return -1;
        }

        sz +=  16;

        if (in[0] & (0x01u << 6)) {
            aad_sz = ((uint32_t)in[16] << 8) | (uint32_t)in[17];

            if (aad_sz == 0xfeff) {
                aad_sz = (uint32_t)in[18] << 24 | (uint32_t)in[19] << 16
                         | (uint32_t)in[20] << 8 | (uint32_t)in[21];
                aad_sz += 6;
            } else if (aad_sz == 0xffff) {
                DBG("%s: Line %d: TODO\n", __FUNCTION__, __LINE__);
            } else {
                aad_sz += 2;
            }

            sz += ROUNDUP(aad_sz, 16);
        }

        *hdr_sz = sz;
    } else if (MODE_GCM == mode) {
        sz = (uint32_t)in[4] << 24 | (uint32_t)in[5] << 16
             | (uint32_t)in[6] << 8 | (uint32_t)in[7];
        aad_sz = (uint32_t)in[12] << 24 | (uint32_t)in[13] << 16
                 | (uint32_t)in[14] << 8 | (uint32_t)in[15];

        if ( ((uint32_t) -1 - sz < aad_sz) || (sz + aad_sz) > i_sz) {
            return -2;
        }

        *hdr_sz = sz;
    } else {
        *hdr_sz = 0;
    }

    return 0;
}

/*
 *!!!NOTE!!!
 * The routines below are for test purpose only, and not been fully tested.
 * Do not use them in a production release.
 */
static int c_2_v(char c, unsigned char *val)
{
    unsigned char v = 0;

    if (c >= '0' && c <= '9') {
        v = c - '0';
    } else if (c >= 'a' && c <= 'f') {
        v = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        v = c - 'A' + 10;
    } else {
        return -1;
    }

    *val = v;
    return 0;
}

int str2bn(const char *str, unsigned char *bn, unsigned int *bn_sz)
{
    if (NULL == str || NULL == bn || NULL == bn_sz) {
        return -1;
    }

    unsigned int slen = strlen(str);
    unsigned int olen = (slen + 1) / 2;

    if (*bn_sz < olen) {
        return -2;
    }

    unsigned char v = 0;

    if (slen % 2) {
        if (0 != c_2_v(*str, &v)) {
            return -5;
        }

        *bn = v;
        bn++;
        str++;
        slen--;
    }

    for (int i = 0; i < slen; i++, str++) {
        unsigned char tmp = 0;

        if (0 != c_2_v(*str, &tmp)) {
            DBG("!!! Opps, c_2_v failed\n");
            return -10;
        }

        if (0 == (i % 2)) {
            v = (tmp << 4);
        } else {
            v |= tmp;
            *bn = v;
            bn++;
        }
    }

    *bn_sz = olen;

    return 0;
}

/* https://tools.ietf.org/html/rfc3610, Coutner with CMC-MAC (CCM) */

int ccm_input_init(uint8_t *hdr, uint32_t M, uint32_t L, bool with_aad)
{
    /*
        Name  Description                               Size    Encoding
        ----  ----------------------------------------  ------  --------
        M     Number of octets in authentication field  3 bits  (M-2)/2
        L     Number of octets in length field          3 bits  L-1
     */
    /* Flag */
    hdr[0] = ((uint8_t)(L - 1) & 7) | (uint8_t)(((M - 2) / 2) & 7) << 3;

    if (with_aad) {
        hdr[0] |= (0x01u << 6u);
    }

    return 0;
}

int ccm_input_setiv(uint8_t *hdr, const uint8_t *nonce, size_t nlen, size_t mlen)
{
    unsigned int L = hdr[0] & 7; /* the L parameter */

    if (sizeof(mlen) == 8 && L >= 3) {
        hdr[8] = (uint8_t)(mlen >> (56 % (sizeof(mlen) * 8)));
        hdr[9] = (uint8_t)(mlen >> (48 % (sizeof(mlen) * 8)));
        hdr[10] = (uint8_t)(mlen >> (40 % (sizeof(mlen) * 8)));
        hdr[11] = (uint8_t)(mlen >> (32 % (sizeof(mlen) * 8)));
    } else {
        hdr[8] = 0;
        hdr[9] = 0;
        hdr[10] = 0;
        hdr[11] = 0;
    }

    hdr[12] = (uint8_t)(mlen >> 24);
    hdr[13] = (uint8_t)(mlen >> 16);
    hdr[14] = (uint8_t)(mlen >> 8);
    hdr[15] = (uint8_t)mlen;

    memcpy(&hdr[1], nonce, 14 - L);

    return 0;
}

void ccm_input_aad(const uint8_t *aad, size_t alen, uint8_t *out, size_t *olen)
{
    unsigned int i;

    if (alen == 0 || NULL == aad
        || NULL == out || NULL == olen)
        return;

    if (*olen < ROUNDUP(alen, 16) + 16)
        return;

    memset(out, 0, 16);

    if (alen < (0x10000 - 0x100)) {
        out[0] ^= (u8)(alen >> 8);
        out[1] ^= (u8)alen;
        i = 2;
    } else if (sizeof(alen) == 8
               && alen >= (size_t)1 << (32 % (sizeof(alen) * 8))) {
        out[0] ^= 0xFF;
        out[1] ^= 0xFF;
        out[2] ^= (u8)(alen >> (56 % (sizeof(alen) * 8)));
        out[3] ^= (u8)(alen >> (48 % (sizeof(alen) * 8)));
        out[4] ^= (u8)(alen >> (40 % (sizeof(alen) * 8)));
        out[5] ^= (u8)(alen >> (32 % (sizeof(alen) * 8)));
        out[6] ^= (u8)(alen >> 24);
        out[7] ^= (u8)(alen >> 16);
        out[8] ^= (u8)(alen >> 8);
        out[9] ^= (u8)alen;
        i = 10;
    } else {
        out[0] ^= 0xFF;
        out[1] ^= 0xFE;
        out[2] ^= (u8)(alen >> 24);
        out[3] ^= (u8)(alen >> 16);
        out[4] ^= (u8)(alen >> 8);
        out[5] ^= (u8)alen;
        i = 6;
    }

    memcpy(&out[i], aad, alen);

    i += alen;

    memset(&out[i], 0, 16 - i % 16);

    *olen = ROUNDUP(i, 16);
}
