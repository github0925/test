#include <arch/defines.h>
#include <debug.h>
#include <malloc.h>
#include <string.h>
#include "sd_x509.h"

#define DEBUG_ON 0

#define SEARCH_LEN_MAX 10000
#define BOOL_TAG    0x01
#define INT_TAG     0x02
#define BS_TAG      0x03
#define OC_TAG      0x04
#define NUL_TAG     0x05
#define OID_TAG     0x06
#define PS_TAG      0x13
#define UT_TAG      0x17
#define GT_TAG      0x17
#define SEQ_TAG     0x30
#define SET_TAG     0x31
#define SEQ_CTX_TAG 0xA0

#define EC_P521R1_LEN 66
#define EC_P384R1_LEN 48
#define EC_P256V1_LEN 32

#define ERROR(format, args...) \
        dprintf(CRITICAL, "%s %d "format"\n", __func__, __LINE__, ##args);

#if DEBUG_ON
#define DEBUG_DUMP(ptr, size, format, args...) \
    do{ \
        dprintf(CRITICAL, "%s %d "format"\n", __func__, __LINE__, ##args); \
        hexdump8(ptr, size); \
    }while(0);

#else
#define DEBUG_DUMP(ptr, size, format, args...)
#endif

enum {
    X509_TRUE,
    X509_FALSE
};

enum {
    X509_T_NULL,
    X509_T_UNIV,
    X509_T_APPL,
    X509_T_CONS,
    X509_T_PRIV
};
typedef enum {
    HASH_SHA256,
    HASH_SHA512,
    HASH_UNKNOWN
} HASH_TYPE;

typedef struct X509Item {
    const unsigned char *addr;
    const unsigned char *cont;
    unsigned short len;
    unsigned char lenBytes;
    unsigned char exist;
    unsigned char taglen;
    unsigned short total_len;
} X509Item;

struct pubKeyInfo {
    X509Item self;
    X509Item n;
    X509Item e;
};

struct subjectPublicKey {
    X509Item self;
    struct pubKeyInfo key;
};

struct SubjectPublicKeyInfo {
    X509Item self;
    X509Item algorithm;
    X509_PUBKEY_TYPE type;
    struct subjectPublicKey pubkey;
};

struct theCertificate {
    X509Item self;
    X509Item version;
    X509Item serialNumber;
    X509Item signature;
    X509Item issuer;
    X509Item validity;
    X509Item subject;
    struct SubjectPublicKeyInfo subjectPublicKeyInfo;
    X509Item issuerUniqueID;
    X509Item subjectUniqueID;
    X509Item extension;
};

struct signatureAlgorithm {
    X509Item self;
    X509_SIGN_T type;
};

struct signatureValue {
    X509Item self;
};

struct Certificate {
    X509Item self;
    struct theCertificate cer;
    struct signatureAlgorithm sAlg;
    struct signatureValue sV;
};

const uint8_t NID_rsa_id[]   = NID_RSAENCRYPTION;
const uint8_t NID_ecdsa_id[] = NID_ECDSAPUBKEY;
const uint8_t NID_ecdsa_prime[] = NID_ECDSA_PRIME;
const uint8_t NID_ecdsa_p256[] =  NID_ECDSA_P256V1;
const uint8_t NID_ecdsa_p384[] =  NID_ECDSA_SECP384R1;
const uint8_t NID_ecdsa_p521[] = NID_ECDSA_SECP521R1;
const uint8_t NID_ecdsawithsha256[] = NID_ECDSAWITHSHA256;
const uint8_t NID_ecdsawithsha512[] = NID_ECDSAWITHSHA512;
const uint8_t NID_sha256rsa[] = NID_SHA256WITHRSAENCRYPTION;
const uint8_t NID_sha512rsa[] = NID_SHA512WITHRSAENCRYPTION;
const uint8_t NID_rsassapss[] = NID_RSASSAPSS;
const uint8_t NID_sha256[] = NID_SHA256;
const uint8_t NID_sha512[] = NID_SHA512;

/*static unsigned char
TagTrans( unsigned char tag )
{
    unsigned char tmptag = tag;
    switch ( tmptag >> 6 ) {
    case 0x00:
        return X509_T_UNIV;
    case 0x01:
        return X509_T_APPL;
    case 0x10:
        return X509_T_CONS;
    case 0x11:
        return X509_T_PRIV;
    default:
        return X509_T_NULL;
    }
}*/

static unsigned char
LenTrans( const unsigned char  *buf,
          unsigned short *len,
          unsigned char  *lenBytes )
{
    if ( *buf < 0x80 ) {
        *len = *buf;
        *lenBytes = 1;
    }
    else if ( *buf > 0x80 ) {
        unsigned char i, j;
        i = *buf & 0x7F;
        *len = 0;

        for ( j = 0; j < i; j++ ) {
            *len = *len * 0x100 + buf[j + 1];
        }

        *lenBytes = i + 1;
    }
    else if ( *buf == 0x80 ) {
        unsigned short index;

        for ( index = 0; index < SEARCH_LEN_MAX; index++ ) {
            if ( memcmp( &buf[index + 1], "\x00\x00", 2 ) == 0 )
                break;
        }

        if ( index != 10000 ) {
            *len = index + 2;
            *lenBytes = 1;
        }
        else {
            return X509_FALSE;
        }
    }

    return X509_TRUE;
}

static unsigned char
TLVTrans( const unsigned char *buf,
          unsigned int buflen,
          X509Item *item )
{
    unsigned char status;
    unsigned short index;
    unsigned short valuelen;
    unsigned char lenBytes;

    item->addr = buf;
    item->taglen = 1;

    index = 0;

    while (buf[index] == 0x00) {
        index++;
        item->taglen++;
    }

    index++;

    status = LenTrans(&buf[index], &valuelen, &lenBytes);

    if (X509_TRUE != status) {
        ERROR("")
        return status;
    }

    if ((valuelen + lenBytes) > buflen) {
        ERROR("valuelen:0x%0x lenBytes:0x%0x buflen:0x%0x\n",
              valuelen, lenBytes, buflen);
        return X509_FALSE;
    }

    item->cont = &buf[lenBytes + item->taglen];
    item->len = valuelen;
    item->lenBytes = lenBytes;
    item->total_len = lenBytes + item->taglen + valuelen;
    item->exist = 1;
    return X509_TRUE;
}

static X509_PUBKEY_TYPE
X509PublickeyType(X509Item *pubkeyAlg,
                  struct theCertificate *certTBS)
{
    X509Item itemTemp;
    unsigned char status;

    DEBUG_DUMP(pubkeyAlg->cont, pubkeyAlg->len, "alg content:\n");
    status = TLVTrans(pubkeyAlg->cont, pubkeyAlg->len, &itemTemp);

    if (X509_TRUE != status) {
        ERROR("");
        return PUBKEY_UNKNOWN_TYPE;
    }

    DEBUG_DUMP(itemTemp.cont, itemTemp.len, "oid:\n");

    /* check public key alg, ec or rsa */
    if (!memcmp(itemTemp.cont, NID_ecdsa_id, sizeof(NID_ecdsa_id))) {

        /* There is no param in public key alg */
        if (*(pubkeyAlg->cont + itemTemp.total_len) == OID_TAG) {
            status = TLVTrans(&pubkeyAlg->cont[itemTemp.total_len],
                              pubkeyAlg->len - itemTemp.total_len,
                              &itemTemp);

            if (status != X509_TRUE) {
                return  PUBKEY_UNKNOWN_TYPE;
            }

            DEBUG_DUMP(itemTemp.cont, itemTemp.len, "ec public key param oid:\n");

            if (!memcmp(itemTemp.cont, NID_ecdsa_p521, sizeof(NID_ecdsa_p521))) {
                return  PUBKEY_EC_SECP521R1;
            }
            else if (!memcmp(itemTemp.cont, NID_ecdsa_p384, sizeof(NID_ecdsa_p384))) {
                return  PUBKEY_EC_SECP384R1;
            }
            else if (!memcmp(itemTemp.cont, NID_ecdsa_p256, sizeof(NID_ecdsa_p256))) {
                return  PUBKEY_EC_P256V1;
            }
        }

        return  PUBKEY_EC_UNKNOWN;
    }
    else if (!memcmp(itemTemp.cont, NID_rsa_id, sizeof(NID_rsa_id))) {
        return  PUBKEY_RSA;
    }
    else {
        return  PUBKEY_UNKNOWN_TYPE;
    }
}

static int
X509RSAPubkeyParse(X509Item *pubkey,
                   struct theCertificate *certTBS)
{
    X509Item *pitem;
    unsigned char status;
    unsigned short index = 0;
    unsigned short buflen = 0;

    /* here, public key has no tag, len, val len in bit string*/
    DEBUG_DUMP(pubkey->cont, pubkey->len, "pubkey:\n");
    index = 0;
    buflen = pubkey->len - index;

    pitem = &certTBS->subjectPublicKeyInfo.pubkey.key.self;
    status = TLVTrans(pubkey->cont, pubkey->len, pitem);

    if (X509_TRUE != status) {
        ERROR("");
        return -1;
    }

    DEBUG_DUMP(pitem->cont, pitem->len, "key seq:\n");
    /* n */
    index = 0;
    buflen = pitem->len - index;

    pitem = &certTBS->subjectPublicKeyInfo.pubkey.key.n;
    status = TLVTrans(
                 &certTBS->subjectPublicKeyInfo.pubkey.key.self.cont[index], buflen, pitem);

    if (X509_TRUE != status) {
        ERROR("");
        return -1;
    }

    DEBUG_DUMP(pitem->cont, pitem->len, "n:\n");
    /* e */
    index += (pitem->taglen + pitem->lenBytes + pitem->len);
    buflen -= index;

    pitem = &certTBS->subjectPublicKeyInfo.pubkey.key.e;
    status = TLVTrans(
                 &certTBS->subjectPublicKeyInfo.pubkey.key.self.cont[index], buflen, pitem);

    if (X509_TRUE != status) {
        ERROR("");
        return -1;
    }

    DEBUG_DUMP(pitem->cont, pitem->len, "e:\n");
    return 0;
}

X509_PUBKEY_TYPE
X509ECPubkeyTypeBylen(unsigned short len)
{
    if (len == EC_P521R1_LEN) {
        return PUBKEY_EC_SECP521R1;
    }
    else if (len == EC_P384R1_LEN) {
        return PUBKEY_EC_SECP384R1;
    }
    else if (len == EC_P256V1_LEN) {
        return PUBKEY_EC_P256V1;
    }
    else {
        ERROR("ec public key len:%d", len);
        return PUBKEY_EC_UNKNOWN;
    }
}

static X509_PUBKEY_TYPE
X509ECPubkeyTypeByParam(const X509Item *alg)
{
    X509Item primeSeq;
    X509Item primeVal;
    X509Item ecParams;
    X509Item skipItem;
    unsigned short status;
    unsigned short index = 0;

    /* public key type, rsa or ec */
    status = TLVTrans(alg->cont, alg->total_len, &skipItem);

    if (X509_TRUE != status) {
        ERROR("");
        return PUBKEY_EC_UNKNOWN;
    }

    status = TLVTrans(alg->cont + skipItem.total_len,
                      alg->total_len - skipItem.total_len,
                      &ecParams);

    if (X509_TRUE != status) {
        ERROR("");
        return PUBKEY_EC_UNKNOWN;
    }

    DEBUG_DUMP(ecParams.cont, ecParams.len, "ec params:\n");

    status = TLVTrans(ecParams.cont, ecParams.total_len, &skipItem);

    if (X509_TRUE != status) {
        ERROR("");
        return PUBKEY_EC_UNKNOWN;
    }

    index = skipItem.total_len;

    status = TLVTrans(&ecParams.cont[skipItem.total_len],
                      ecParams.total_len - skipItem.total_len,
                      &primeSeq);

    if (X509_TRUE != status) {
        ERROR("");
        return PUBKEY_EC_UNKNOWN;
    }

    DEBUG_DUMP(primeSeq.cont, primeSeq.len, "prime:\n");

    index = 0;
    status = TLVTrans(primeSeq.cont,
                      primeSeq.total_len,
                      &skipItem);

    if (X509_TRUE != status) {
        ERROR("");
        return PUBKEY_EC_UNKNOWN;
    }

    DEBUG_DUMP(skipItem.cont, skipItem.len, "prime field oid:\n");

    if (memcmp(skipItem.cont, NID_ecdsa_prime, sizeof(NID_ecdsa_prime))) {
        DEBUG_DUMP(skipItem.cont, skipItem.len, "");
        return PUBKEY_EC_UNKNOWN;
    }

    index = skipItem.total_len;
    status = TLVTrans(primeSeq.cont + index,
                      primeSeq.total_len - index,
                      &primeVal);


    if (X509_TRUE != status) {
        ERROR("");
        return PUBKEY_EC_UNKNOWN;
    }

    while (*(primeVal.cont) == 0x00) {
        (primeVal.len)--;
        (primeVal.cont)++;
    }

    DEBUG_DUMP(primeVal.cont, primeVal.len, "prime:\n");
    return X509ECPubkeyTypeBylen(primeVal.len);
}

static int
X509ECPubkeyParse(const X509Item *pubkey,
                  struct theCertificate *certTBS)
{
    X509Item *pitemN;
    X509Item *pitemKey;
    X509Item *pitemAlg = &certTBS->subjectPublicKeyInfo.algorithm;
    X509_PUBKEY_TYPE *pubkeyType =  &certTBS->subjectPublicKeyInfo.type;

    if (*pubkeyType == PUBKEY_EC_UNKNOWN) {
        *pubkeyType = X509ECPubkeyTypeByParam(pitemAlg);
    }

    if (*pubkeyType >= PUBKEY_EC_UNKNOWN ||
            *pubkeyType < PUBKEY_EC_P256V1) {
        ERROR("cann't get ec key type");
        return -1;
    }

    pitemKey = &certTBS->subjectPublicKeyInfo.pubkey.key.self;
    pitemN = &certTBS->subjectPublicKeyInfo.pubkey.key.n;

    /* For ec public, key and n are the same */
    memcpy(pitemN, pubkey, sizeof(X509Item));
    memcpy(pitemKey, pubkey, sizeof(X509Item));

    DEBUG_DUMP(pitemN->cont, pitemN->len, "ec public key:\n");
    return 0;
}

static int
X509TBSCertResolve( const unsigned char *cer,
                    unsigned short cerlen,
                    struct theCertificate *certTBS )
{
    X509Item *pitem;
    unsigned short index;
    unsigned short buflen;
    unsigned char status;
    X509_PUBKEY_TYPE pubkeyType = PUBKEY_UNKNOWN_TYPE;

    /* subject */
    index = 0;
    buflen = cerlen;
    status = TLVTrans(&cer[index], buflen, &certTBS->self);

    if (X509_TRUE != status)
        return -1;

    /* version */
    index = 0;
    buflen = certTBS->self.len;
    pitem = &certTBS->version;

    if (certTBS->self.cont[0] == 0x02) { /* no version */
        pitem->addr = 0x00;
        pitem->cont = 0x00;
        pitem->len = 0x00;
        pitem->lenBytes = 0x00;
        pitem->exist = 0x00;
        pitem->taglen = 0x00;
    }
    else {
        status = TLVTrans(certTBS->self.cont, buflen, pitem);

        if (X509_TRUE != status) {
            ERROR("");
            return -1;
        }
    }

    /* serial */
    index += (pitem->taglen + pitem->lenBytes + pitem->len);
    buflen = certTBS->self.len - index;

    pitem = &certTBS->serialNumber;
    status = TLVTrans(&certTBS->self.cont[index], buflen, pitem);

    if (X509_TRUE != status) {
        ERROR("");
        return -1;
    }

    /* signature alg */
    index += (pitem->taglen + pitem->lenBytes + pitem->len);
    buflen = certTBS->self.len - index;

    pitem = &certTBS->signature;
    status = TLVTrans(&certTBS->self.cont[index], buflen, pitem);

    if (X509_TRUE != status) {
        ERROR("");
        return -1;
    }

    /* issuer */
    index += (pitem->taglen + pitem->lenBytes + pitem->len);
    buflen = certTBS->self.len - index;

    pitem = &certTBS->issuer;
    status = TLVTrans(&certTBS->self.cont[index], buflen, pitem);

    if (X509_TRUE != status) {
        ERROR("");
        return -1;
    }

    /* validate time */
    index += (pitem->taglen + pitem->lenBytes + pitem->len);
    buflen = certTBS->self.len - index;

    pitem = &certTBS->validity;
    status = TLVTrans(&certTBS->self.cont[index], buflen, pitem);

    if (X509_TRUE != status) {
        ERROR("");
        return -1;
    }

    /* subject name */
    index += (pitem->taglen + pitem->lenBytes + pitem->len);
    buflen = certTBS->self.len - index;

    pitem = &certTBS->subject;
    status = TLVTrans(&certTBS->self.cont[index], buflen, pitem);

    if (X509_TRUE != status) {
        ERROR("");
        return -1;
    }

    /* pubkey info */
    index += (pitem->taglen + pitem->lenBytes + pitem->len);
    buflen = certTBS->self.len - index;

    pitem = &certTBS->subjectPublicKeyInfo.self;
    status = TLVTrans(&certTBS->self.cont[index], buflen, pitem);

    if (X509_TRUE != status) {
        ERROR("");
        return -1;
    }

    /* pubkey alg */
    pitem = &certTBS->subjectPublicKeyInfo.self;
    index = 0;
    buflen = certTBS->subjectPublicKeyInfo.self.len;

    pitem = &certTBS->subjectPublicKeyInfo.algorithm;
    status = TLVTrans(&certTBS->subjectPublicKeyInfo.self.cont[index], buflen,
                      pitem);

    if (X509_TRUE != status) {
        ERROR("")
        return -1;
    }

    /* pubkey */
    index += (pitem->taglen + pitem->lenBytes + pitem->len);
    buflen -= (pitem->taglen + pitem->lenBytes + pitem->len);

    pitem = &certTBS->subjectPublicKeyInfo.pubkey.self;
    status = TLVTrans(&certTBS->subjectPublicKeyInfo.self.cont[index], buflen,
                      pitem);

    if (X509_TRUE != status) {
        ERROR("")
        return -1;
    }

    pubkeyType = X509PublickeyType(&certTBS->subjectPublicKeyInfo.algorithm,
                                   certTBS);
    certTBS->subjectPublicKeyInfo.type = pubkeyType;

    if (pubkeyType == PUBKEY_RSA) {
        return X509RSAPubkeyParse(&certTBS->subjectPublicKeyInfo.pubkey.self,
                                  certTBS);
    }
    else if (pubkeyType <= PUBKEY_EC_UNKNOWN
             && pubkeyType >= PUBKEY_EC_P256V1) {
        return X509ECPubkeyParse(&certTBS->subjectPublicKeyInfo.pubkey.self,
                                 certTBS);
    }
    else {
        dprintf(CRITICAL, "%s %d public key type unknown\n", __func__, __LINE__);
        return -1;
    }
}

static int X509CertRSASignVal(struct Certificate *cert)
{
    unsigned short index;
    unsigned short buflen;
    unsigned char status;
    X509Item *signVal;

    index = cert->cer.self.total_len +  cert->sAlg.self.total_len;
    buflen = cert->self.len - index;
    signVal = &cert->sV.self;

    status = TLVTrans(&cert->self.cont[index], buflen, signVal);

    if (X509_TRUE != status) {
        ERROR("")
        return -1;
    }

    return 0;
}

static int X509CertRmZeroHeader(const unsigned char **buf,
                                uint16_t *len)
{
    if (!buf || !(*buf) || !len || !(*len))
        return -1;

    while (*len > 0) {
        if ( *(*buf) == 0x00) {
            *buf += 1;
            *len -= 1;
        }
        else {
            break;
        }
    }

    return 0;
}

static int X509CertECDSASignVal(struct Certificate *cert)
{
    X509Item signValSeq;
    X509Item *signVal;
    unsigned short index;
    unsigned short signLen;
    unsigned short buflen;
    unsigned char status;
    X509Item signR = {0};
    X509Item signS = {0};
    unsigned short Roff = 0;
    unsigned short Soff = 0;
    uint8_t *signVBuf = NULL;

    index = cert->cer.self.total_len +  cert->sAlg.self.total_len;
    buflen = cert->self.len - index;
    signVal = &cert->sV.self;

    status = TLVTrans(&cert->self.cont[index], buflen, signVal);

    if (X509_TRUE != status) {
        ERROR("")
        return -1;
    }

    status = TLVTrans(signVal->cont, signVal->len, &signValSeq);

    if (X509_TRUE != status) {
        ERROR("")
        return -1;
    }

    DEBUG_DUMP(signValSeq.cont, signValSeq.len, "signValSeq:");
    /* get signR */
    status = TLVTrans(signValSeq.cont, signValSeq.len, &signR);

    DEBUG_DUMP(signR.cont, signR.len, "signR:");

    if (X509_TRUE != status) {
        ERROR("")
        return -1;
    }

    index = signR.total_len;
    buflen = signValSeq.len - index;

    /* get signS */
    status = TLVTrans(signValSeq.cont + index, buflen, &signS);
    DEBUG_DUMP(signS.cont, signS.len, "signS:");

    if (X509_TRUE != status) {
        ERROR("")
        return -1;
    }

    /* merge signR and signS into final signVal */
    if (signR.cont[0] == 0x00 && ((signR.cont[1] & 0x80) == 0x80)
            && (signR.len & 0x01))
        X509CertRmZeroHeader(&(signR.cont), &(signR.len));

    if (signS.cont[0] == 0x00 && ((signS.cont[1] & 0x80) == 0x80)
            && (signS.len & 0x01))
        X509CertRmZeroHeader(&(signS.cont), &(signS.len));

    if (signS.len > signR.len) {
        Roff = signS.len - signR.len;
        signLen = 2 * signS.len;
    }
    else if (signS.len < signR.len) {
        Soff = signR.len - signS.len;
        signLen = 2 * signR.len;
    }
    else {
        if (signS.len & 0x01) {
            Roff++;
            Soff++;
        }

        signLen = 2 * signR.len + Roff + Soff;
    }

    signVBuf = memalign(CACHE_LINE, signLen);

    if (!signVBuf) {
        ERROR("no enough memory!");
        return -1;
    }

    DEBUG_DUMP(signR.cont, signR.len, "signR:");
    DEBUG_DUMP(signS.cont, signS.len, "signS:");

    memset(signVBuf, 0x0, signLen);
    memcpy(signVBuf + Roff, signR.cont, signR.len);
    memcpy(signVBuf + Roff + signR.len + Soff, signS.cont, signS.len);
    signVal->len = signR.len + signS.len + Roff + Soff;
    signVal->cont = signVBuf;

    DEBUG_DUMP(signVal->cont, signVal->len, "signVal:");
    return 0;
}

static int
X509CertResolve( const unsigned char *cer,
                 unsigned int cerlen,
                 struct Certificate *cert )
{
    X509Item *pitem;
    const unsigned char *p;
    unsigned short index;
    unsigned int  buflen;
    unsigned char status;
    X509_SIGN_T signType;

    p = cer;

    /* cert self */
    index = 0;
    buflen = cerlen;
    status = TLVTrans(&p[index], buflen, &cert->self);

    if (X509_TRUE != status) {
        ERROR("")
        return -1;
    }

    /* cert subject */
    if (0 != X509TBSCertResolve(cert->self.cont, cert->self.len, &cert->cer)) {
        ERROR("")
        return -1;
    }

    /* alg */
    index = 1 + cert->cer.self.lenBytes + cert->cer.self.len;
    buflen = cert->self.len - index;
    pitem = &cert->sAlg.self;
    status = TLVTrans(&cert->self.cont[index], buflen, pitem);

    if (X509_TRUE != status) {
        ERROR("")
        return -1;
    }

    DEBUG_DUMP(pitem->cont, pitem->len, "alg:");
    signType = X509_sign_type(pitem->cont, pitem->len);
    cert->sAlg.type = signType;

    /* signature value */
    if (signType == SIGN_SHA256_ECCDSA
            || signType == SIGN_SHA512_ECCDSA) {

        return X509CertECDSASignVal(cert);
    }
    else {
        return X509CertRSASignVal(cert);
    }
}

int
X509CertParse( const unsigned char  cert_buf[],
               unsigned int cert_len,
               X509_CERT *x509_cert )
{
    struct Certificate cert = {0};
    X509_SIGN_T sign_type;

    if ( 0 != X509CertResolve( cert_buf, cert_len, &cert ) ) {
        ERROR("")
        return -1;
    }

    x509_cert->pubkey_type = cert.cer.subjectPublicKeyInfo.type;
    x509_cert->n = cert.cer.subjectPublicKeyInfo.pubkey.key.n.cont;
    x509_cert->n_len = cert.cer.subjectPublicKeyInfo.pubkey.key.n.len;
    X509CertRmZeroHeader(&(x509_cert->n), &(x509_cert->n_len));

    if (x509_cert->pubkey_type >= PUBKEY_EC_P256V1
            && x509_cert->pubkey_type <= PUBKEY_EC_SECP521R1) {
        if ( *x509_cert->n == 0x04  ) {/* compressed flag */
            x509_cert->n++;
            x509_cert->n_len--;
        }
    }
    else {
        x509_cert->e = cert.cer.subjectPublicKeyInfo.pubkey.key.e.cont;
        x509_cert->e_len = cert.cer.subjectPublicKeyInfo.pubkey.key.e.len;
        X509CertRmZeroHeader(&(x509_cert->e), &(x509_cert->e_len));
    }

    x509_cert->issuer = cert.cer.issuer.cont;
    x509_cert->issuer_len = cert.cer.issuer.len;
    x509_cert->serial = cert.cer.serialNumber.cont;
    x509_cert->serial_len = cert.cer.serialNumber.len;
    x509_cert->subject = cert.cer.self.addr;
    x509_cert->subject_len = cert.cer.self.len + cert.cer.self.lenBytes + 1;
    x509_cert->alg = cert.sAlg.self.cont;
    x509_cert->alg_len = cert.sAlg.self.len;
    x509_cert->sign_type = cert.sAlg.type;
    x509_cert->sign = cert.sV.self.cont;
    x509_cert->sign_len = cert.sV.self.len;
    x509_cert->tbs = cert.cer.self.addr;
    x509_cert->tbs_len = cert.cer.self.total_len;

    sign_type = cert.sAlg.type;

    if (!(sign_type == SIGN_SHA256_ECCDSA || sign_type == SIGN_SHA512_ECCDSA))
        X509CertRmZeroHeader(&(x509_cert->sign), &(x509_cert->sign_len));

    x509_cert->cert_len = cert.self.total_len;
    return 1;
}

int X509CertChainParse( const unsigned char  cert_buf[],
                        unsigned int cert_len,
                        X509_CERT **cert_chain )
{
    int cert_count = 0;
    unsigned int len = cert_len;
    X509_CERT  cert = {0};
    X509_CERT  *cert_p = NULL;
    X509_CERT  *cert_temp = NULL;
    const unsigned char *buf = cert_buf;

    while (len > 0 && X509CertParse(buf, len, &cert) > 0) {
        cert_count++;
        buf += cert.cert_len;
        len -= cert.cert_len;

        cert_temp = malloc(cert_count * sizeof(X509_CERT));

        if (!cert_temp) {
            goto fail;
        }

        if (!cert_p) {
            memcpy(cert_temp + cert_count - 1, &cert, sizeof(X509_CERT));
            cert_p = cert_temp;
        }
        else {
            memcpy(cert_temp, cert_p, (cert_count - 1) * sizeof(X509_CERT));
            memcpy(cert_temp + cert_count - 1, &cert, sizeof(X509_CERT));
            free(cert_p);
            cert_p = cert_temp;
        }
    }

    *cert_chain = cert_p;
    return cert_count;

fail:

    return -1;
}

int X509CertChainFree(X509_CERT *cert_chain, unsigned int count)
{
    X509_CERT *cert = cert_chain;

    while (count > 0 ) {
        switch (cert->pubkey_type) {
            case PUBKEY_EC_P256V1:
            case PUBKEY_EC_SECP384R1:
            case PUBKEY_EC_SECP521R1: {
                if (cert->sign) {
                    free((void *)(addr_t)(cert->sign));
                }

                break;
            }

            default:
                break;
        }

        cert++;
        count--;
    }

    if (count > 0)
        free(cert_chain);

    return 0;
}

static HASH_TYPE
X509_sign_pss_hash(const unsigned char *sign_para,
                   unsigned int len)
{
    unsigned short val_len;
    unsigned char lenBytes;
    const unsigned char *hash_nid = NULL;

    if (!sign_para || !len || sign_para[0] != SEQ_TAG)
        return HASH_UNKNOWN;

    for (unsigned int i = 0; i < 4; i++) {
        if (LenTrans(++sign_para, &val_len, &lenBytes) != X509_TRUE)
            return HASH_UNKNOWN;

        sign_para += lenBytes;
    }

    hash_nid = sign_para;

    if (!memcmp(hash_nid, NID_sha256, val_len)) {
        return HASH_SHA256;
    }
    else if (!memcmp(hash_nid, NID_sha512, val_len)) {
        return HASH_SHA512;
    }
    else {
        DEBUG_DUMP(hash_nid, val_len,  "unsupport alg\n");
        return HASH_UNKNOWN;
    }

}

X509_SIGN_T X509_sign_type(const unsigned char *sign_alg, unsigned int len)
{
    HASH_TYPE hash_type;
    X509Item sAlg;
    unsigned short val_len;
    const unsigned char *oid_start;

    if (!sign_alg || !len || sign_alg[0] != OID_TAG)
        return SIGN_UNKNOWN_TYPE;

    if (TLVTrans(sign_alg, len, &sAlg) != X509_TRUE)
        return SIGN_UNKNOWN_TYPE;

    oid_start = sAlg.cont;
    val_len = sAlg.len;

    if (!memcmp(oid_start, NID_sha256rsa, val_len)) {
        return SIGN_SHA256_RSA_PKCS1;
    }
    else if (!memcmp(oid_start, NID_sha512rsa, val_len)) {
        return SIGN_SHA512_RSA_PKCS1;
    }
    else if (!memcmp(oid_start, NID_rsassapss, val_len)) {

        if (TLVTrans(sign_alg + sAlg.total_len, len - sAlg.total_len,
                     &sAlg) != X509_TRUE)
            return SIGN_UNKNOWN_TYPE;

        hash_type = X509_sign_pss_hash(sAlg.cont, sAlg.len);

        if (hash_type == HASH_SHA256)
            return SIGN_SHA256_RSASSA_PSS;
        else if (hash_type == HASH_SHA512)
            return SIGN_SHA512_RSASSA_PSS;
        else
            return SIGN_UNKNOWN_TYPE;
    }
    else if (!memcmp(oid_start, NID_ecdsawithsha256, val_len)) {
        return SIGN_SHA256_ECCDSA;
    }
    else if (!memcmp(oid_start, NID_ecdsawithsha512, val_len)) {
        return SIGN_SHA512_ECCDSA;
    }
    else {
        DEBUG_DUMP(oid_start, val_len, "unsupport alg\n");
        return SIGN_UNKNOWN_TYPE;
    }
}

