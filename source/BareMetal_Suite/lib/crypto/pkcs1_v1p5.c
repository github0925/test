/********************************************************
 *          Copyright(c) 2019   Spstridrive                 *
 ********************************************************/

#include <common_hdr.h>
#include <atb_crypto.h>

#define ASN1_SEQUENCE 0x30
#define ASN1_OCTET_STRING 0x04
#define ASN1_NULL 0x05
#define ASN1_OID 0x06

static const unsigned char digestinfo_sha1_der[] =   { ASN1_SEQUENCE, 0x0d + 20, ASN1_SEQUENCE, 0x09, ASN1_OID, 0x05, 1 * 40 + 3,  14, 3, 2, 26, ASN1_NULL, 0x00, ASN1_OCTET_STRING, 20};
static const unsigned char digestinfo_sha224_der[] = { ASN1_SEQUENCE, 0x11 + 28, ASN1_SEQUENCE, 0x0d, ASN1_OID, 0x09, 2 * 40 + 16, 0x86, 0x48, 1, 101, 3, 4, 2, 0x04, ASN1_NULL, 0x00, ASN1_OCTET_STRING, 28 };
static const unsigned char digestinfo_sha256_der[] = { ASN1_SEQUENCE, 0x11 + 32, ASN1_SEQUENCE, 0x0d, ASN1_OID, 0x09, 2 * 40 + 16, 0x86, 0x48, 1, 101, 3, 4, 2, 0x01, ASN1_NULL, 0x00, ASN1_OCTET_STRING, 32 };
static const unsigned char digestinfo_sha384_der[] = { ASN1_SEQUENCE, 0x11 + 48, ASN1_SEQUENCE, 0x0d, ASN1_OID, 0x09, 2 * 40 + 16, 0x86, 0x48, 1, 101, 3, 4, 2, 0x02, ASN1_NULL, 0x00, ASN1_OCTET_STRING, 48 };
static const unsigned char digestinfo_sha512_der[] = { ASN1_SEQUENCE, 0x11 + 64, ASN1_SEQUENCE, 0x0d, ASN1_OID, 0x09, 2 * 40 + 16, 0x86, 0x48, 1, 101, 3, 4, 2, 0x03, ASN1_NULL, 0x00, ASN1_OCTET_STRING, 64 };

U32 add_hashinfo(U8 *pstr, U32 pslen,
                   const U8 *str, U32 slen, U8 alg)
{
    U32 der_len = 0U;

    mini_memcpy_s(pstr + pslen - slen, str, slen);
    switch (alg)
    {
    case ALG_HASH_SHA1:
        der_len = 15U;
        mini_memcpy_s(pstr + pslen - slen - der_len, digestinfo_sha1_der, der_len);
        break;
    case ALG_HASH_SHA224:
        der_len = 19U;
        mini_memcpy_s(pstr + pslen - slen - der_len, digestinfo_sha224_der, der_len);
        break;
    case ALG_HASH_SHA256:
        der_len = 19U;
        mini_memcpy_s(pstr + pslen - slen - der_len, digestinfo_sha256_der, der_len);
        break;
    case ALG_HASH_SHA384:
        der_len = 19U;
        mini_memcpy_s(pstr + pslen - slen - der_len, digestinfo_sha384_der, der_len);
        break;
    case ALG_HASH_SHA512:
        der_len = 19U;
        mini_memcpy_s(pstr + pslen - slen - der_len, digestinfo_sha512_der, der_len);
        break;
    default:
        break;
    }
    return slen + der_len;
}

/*
 * refer str rfc3447, section 9.2
 */

/* EM = 0x00 || 0x01 || PS || 0x00 || T */
U32 pkcs1_v1p5_pad(U8 *pstr, U32 pslen,    /* pad string and its len */
                   U32 slen)           /* unpad string and its len*/
{
    if (NULL == pstr) {
        return -1;
    }

    if ((pslen < slen)
        || ((pslen - slen) < 11)) {
        return -2;
    }

    *pstr++ = 0;    /* 0x00 */
    *pstr++ = 1;    /* 0x01*/

    for (int i = 0; i < (pslen - slen - 3); i++, pstr++) {  /* PS */
        *pstr = 0xFFu;
    }

    *pstr = 0;  /* 0x00 */

    return 0;
}

#if defined(CFG_CRYPTO_API_pkcs1_v1p5_unpad)
U32 pkcs1_v1p5_unpad(const U8 *pstr, U32 pslen, /* pad string and its len */
                     U8 *str, U32 *slen)  /* unpad string and its len */
{
    if (NULL == pstr || NULL == str || NULL == slen) {
        return -1;
    }

    if (pstr[0] != 0 || pstr[1] != 0x01 || pstr[2] != 0xffu) {
        return -2;
    }

    const U8 *p = pstr + 2;
    U32 i = 0;

    for (; i < pslen - 2; i++, p++) {
        if (*p != 0xff) {
            break;
        }
    }

    if (i < 8) {
        return -3;
    }

    if (*p != 0x00) {
        return -4;
    }

    p++;

    U32 i_len = pslen  - (p - pstr);

    if ( i_len > *slen) {
        return -6;
    }

    mini_memcpy_s(str, p, i_len);
    *slen = i_len;

    return 0;
}
#endif
