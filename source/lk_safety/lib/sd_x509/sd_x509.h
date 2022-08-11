#ifndef _SD_X509_H_
#define _SD_X509_H_

#ifdef __cplusplus
extern "C" {
#endif

#define  NID_RSAENCRYPTION           {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01}
#define  NID_SHA256WITHRSAENCRYPTION {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b}
#define  NID_SHA512WITHRSAENCRYPTION {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0d}
#define  NID_RSASSAPSS               {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0a}
#define  NID_SHA256                  {0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01}
#define  NID_SHA512                  {0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03}
#define  NID_ECDSAWITHSHA256         {0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02}
#define  NID_ECDSAWITHSHA512         {0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x04}

#define  NID_ECDSAPUBKEY             {0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01}
#define  NID_ECDSA_SECP521R1         {0x2b, 0x81, 0x04, 0x00, 0x23}
#define  NID_ECDSA_SECP384R1         {0x2b, 0x81, 0x04, 0x00, 0x22}
#define  NID_ECDSA_P256V1            {0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07}

#define  NID_ECDSA_PRIME             {0x2a, 0x86, 0x48, 0xce, 0x3d, 0x01, 0x01}

typedef enum sign_typ
{
    SIGN_SHA256_RSA_PKCS1,
    SIGN_SHA512_RSA_PKCS1,
    SIGN_SHA256_RSASSA_PSS,
    SIGN_SHA512_RSASSA_PSS,
    SIGN_SHA256_ECCDSA,
    SIGN_SHA512_ECCDSA,
    SIGN_UNKNOWN_TYPE
}X509_SIGN_T;

typedef enum public_key_type
{
    PUBKEY_RSA = 0,
    PUBKEY_EC_P256V1,
    PUBKEY_EC_SECP384R1,
    PUBKEY_EC_SECP521R1,
    PUBKEY_EC_UNKNOWN,
    PUBKEY_UNKNOWN_TYPE,
} X509_PUBKEY_TYPE;

typedef struct {
    X509_SIGN_T sign_type;
    X509_PUBKEY_TYPE pubkey_type;
    const unsigned char *n;
    unsigned short n_len;
    const unsigned char *e;
    unsigned short e_len;
    const unsigned char *issuer;
    unsigned short issuer_len;
    const unsigned char *serial;
    unsigned short serial_len;
    const unsigned char *subject;
    unsigned short subject_len;
    const unsigned char *alg;
    unsigned short alg_len;
    const unsigned char *sign;
    unsigned short sign_len;
    const unsigned char *tbs;
    unsigned short tbs_len;
    unsigned short cert_len;
} X509_CERT;

int X509CertParse(const unsigned char cert_buf[],
                        unsigned int cert_len,
                        X509_CERT* cert_chain );

int X509CertChainParse( const unsigned char cert_buf[],
                        unsigned int cert_len,
                        X509_CERT** cert_chain );

int X509CertChainFree(X509_CERT* cert_chain, unsigned int count);

X509_SIGN_T X509_sign_type(const unsigned char* sign_alg, unsigned int len);

X509_PUBKEY_TYPE X509ECPubkeyTypeBylen(unsigned short len);
#ifdef __cplusplus
};
#endif

#endif

