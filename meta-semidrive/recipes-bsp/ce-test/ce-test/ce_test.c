/*
 * ce_test.c
 *
 *  Created on: 2021
 *      Author:
 */

/*
 * Test application that data integraty of inter processor
 * communication from linux userspace to a remote software
 * context. The application sends chunks of data to the
 * remote processor. The remote side echoes the data back
 * to application which then validates the data returned.
 */

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>
#include "rsa_data.h"

#define MAX_OUTPUT_NUM                  5

#define SM2_TEST_CMD                    0
#define RSA_TEST_CMD                    1


#define RSA_ENC_TRAV_TEST               0
#define RSA_DEC_TRAV_TEST               1
#define RSA_CRT_DEC_TRAV_TEST           2
#define RSA_SIG_GEN_TRAV_TEST           3
#define RSA_SIG_VERIFY_TRAV_TEST        4
#define RSA_PRIV_KEY_GEN_TRAV_TEST      5
#define RSA_CRT_KEY_GEN_TRAV_TEST       6
#define RSA_KEY_GEN_TRAV_TEST           7
#define RSA_OAEP_ENC_TRAV_TEST          8
#define RSA_PSS_SIG_GEN_TRAV_TEST       9
#define RSA_OAEP_CRT_ENC_TRAV_TEST      10

#define SEMIDRIVE_SM2_VERIFY_MSG        _IO(0xaa, 3)
#define SEMIDRIVE_SM2_VERIFY_NO_DIGEST  _IO(0xaa, 1)
#define SEMIDRIVE_SM2_VERIFY_DIGEST     _IO(0xaa, 2)
#define SEMIDRIVE_SM2_SET_PUBKEY        _IO(0xaa, 4)

#define SEMIDRIVE_RSA_ENC               _IO(0xab, 1)
#define SEMIDRIVE_RSA_CRT_DEC           _IO(0xab, 3)
#define SEMIDRIVE_RSA_DEC               _IO(0xab, 2)
#define SEMIDRIVE_RSA_SIG_GEN           _IO(0xab, 4)
#define SEMIDRIVE_RSA_SIG_VERIFY        _IO(0xab, 5)
#define SEMIDRIVE_RSA_PRIV_KEY_GEN      _IO(0xab, 6)
#define SEMIDRIVE_RSA_CRT_KEY_GEN       _IO(0xab, 7)
#define SEMIDRIVE_RSA_KEY_GEN           _IO(0xab, 8)

#define SEMIDRIVE_CE_GET_PAGE           _IO(0xaf, 1)
#define SEMIDRIVE_CE_FREE_PAGE          _IO(0xaf, 2)
#define SEMIDRIVE_CE_CLEAR_PAGE         _IO(0xaf, 3)

#define SEMIDRIVE_SM2_SET_TIME_STAMP    _IO(0xaa, 9)
#define SEMIDRIVE_SM2_GET_TIME_STAMP    _IO(0xaa, 0xa)

#define SEMIDRIVE_RSA_SET_TIME_STAMP    _IO(0xab, 9)
#define SEMIDRIVE_RSA_GET_TIME_STAMP    _IO(0xab, 0xa)

#define DEFINE_V2X_PATTERN                  1
#define DEFINE_GB_PATTERN                   1
typedef enum buff_addr_type {
    HAL_SRAM_PUB = 0,
    HAL_SRAM_SEC = 1,
    HAL_KEY_INT = 2,
    HAL_EXT_MEM = 3,
    HAL_PKE_INTERNAL = 4
} buff_addr_type_t;

struct sm2_verify_msg {
    const uint8_t* msg;
    size_t msg_len;
    const uint8_t* sig;
    size_t sig_len;
    const uint8_t* key;
    size_t key_len;
    uint32_t* ret;
} __attribute__((packed));

typedef enum ce_rsa_pad_types_e {
    RSA_PADDING_NONE      = 0x0,/**< No padding */
    RSA_PADDING_OAEP      = 0x1,/**< Optimal Asymmetric Encryption Padding */
    RSA_PADDING_EME_PKCS  = 0x2,/**< EME-PKCS padding */
    RSA_PADDING_EMSA_PKCS = 0x3,/**< EMSA-PKCS padding */
    RSA_PADDING_PSS       = 0x4 /**< EMSA-PSS padding */
} ce_rsa_pad_types_t;

typedef enum  {
    ALG_MD5     = 0x0,
    ALG_SHA1    = 0x1,
    ALG_SHA224  = 0x2,
    ALG_SHA256  = 0x3,
    ALG_SHA384  = 0x4,
    ALG_SHA512  = 0x5,
    ALG_SM3     = 0x6
} ce_hash_alg_t;

typedef struct {
    uint8_t* n;       /* Modulus */
    uint8_t* e;       /* Public exponent */
    uint8_t* d;       /* Private exponent */
    uint32_t  n_len;
    uint32_t  e_len;
    uint32_t  d_len;
} rsa_keypair_t;

struct rsa_all_information {
    const uint8_t* msg;
    size_t msg_len;
    const uint8_t* cipher;
    size_t cipher_len;
    const uint8_t* sig;
    size_t sig_len;
    rsa_keypair_t rsa_keypair;
    ce_rsa_pad_types_t ce_rsa_pad_types_temp;
    buff_addr_type_t addr_type;
    ce_hash_alg_t ce_rsa_hashType;
    uint8_t* final_result[MAX_OUTPUT_NUM];
    size_t final_result_len[MAX_OUTPUT_NUM];
    uint32_t* ret;
} __attribute__((packed));


struct rsa_crt_decrypt_information {
    const uint8_t* msg;
    size_t msg_len;
    const uint8_t* cipher;
    size_t cipher_len;
    const uint8_t* p;
    size_t p_len;
    const uint8_t* q;
    size_t q_len;
    const uint8_t* dP;
    size_t dP_len;
    const uint8_t* dQ;
    size_t dQ_len;
    const uint8_t* qInv;
    size_t qInv_len;
    ce_rsa_pad_types_t ce_rsa_pad_types_temp;
    buff_addr_type_t addr_type;
    ce_hash_alg_t ce_rsa_hashType;
    uint8_t* final_result[MAX_OUTPUT_NUM];
    size_t final_result_len[MAX_OUTPUT_NUM];
    uint32_t* ret;
} __attribute__((packed));

struct rsa_priv_key_gen_information {
    const uint8_t* p;
    size_t p_len;
    const uint8_t* q;
    size_t q_len;
    const uint8_t* n;
    size_t n_len;
    const uint8_t* e;
    size_t e_len;
    const uint8_t* d;
    size_t d_len;
    uint8_t* final_result[MAX_OUTPUT_NUM];
    size_t final_result_len[MAX_OUTPUT_NUM];
    uint32_t* ret;
} __attribute__((packed));

struct rsa_crt_key_gen_information {
    const uint8_t* p;
    size_t p_len;
    const uint8_t* q;
    size_t q_len;
    const uint8_t* d;
    size_t d_len;
    const uint8_t* dP;
    size_t dP_len;
    const uint8_t* dQ;
    size_t dQ_len;
    const uint8_t* qInv;
    size_t inv_len;
    uint8_t* final_result[MAX_OUTPUT_NUM];
    size_t final_result_len[MAX_OUTPUT_NUM];
    uint32_t* ret;
} __attribute__((packed));

struct rsa_key_gen_information {
    size_t  keysize;
    const uint8_t* e;
    size_t e_len;
    const uint8_t* p;
    size_t p_len;
    const uint8_t* q;
    size_t q_len;
    const uint8_t* n;
    size_t n_len;
    const uint8_t* d;
    size_t d_len;
    uint8_t* final_result[MAX_OUTPUT_NUM];
    size_t final_result_len[MAX_OUTPUT_NUM];
    uint32_t* ret;
} __attribute__((packed));



//sm2 gb pattern start
/*dB：3945208F 7B2144B1 3F36E38A C6D39F95 88939369 2860B51A 42FB81EF 4DF7C5B8*/
uint8_t sm2_gb_prv_key[32] = "\x39\x45\x20\x8F\x7B\x21\x44\xB1\x3F\x36\xE3\x8A\xC6\xD3\x9F\x95\x88\x93\x93\x69\x28\x60\xB5\x1A\x42\xFB\x81\xEF\x4D\xF7\xC5\xB8"; //-- d

/*xB：09F9DF31 1E5421A1 50DD7D16 1E4BC5C6 72179FAD 1833FC07 6BB08FF3 56F35020*/
/*yB：CCEA490C E26775A5 2DC6EA71 8CC1AA60 0AED05FB F35E084A 6632F607 2DA9AD13*/

uint8_t sm2_gb_pub_key[64] = "\x09\xF9\xDF\x31\x1E\x54\x21\xA1\x50\xDD\x7D\x16\x1E\x4B\xC5\xC6\x72\x17\x9F\xAD\x18\x33\xFC\x07\x6B\xB0\x8F\xF3\x56\xF3\x50\x20" //-- Qx
                             "\xCC\xEA\x49\x0C\xE2\x67\x75\xA5\x2D\xC6\xEA\x71\x8C\xC1\xAA\x60\x0A\xED\x05\xFB\xF3\x5E\x08\x4A\x66\x32\xF6\x07\x2D\xA9\xAD\x13";  //-- Qy

/*message digest:6D65737361676520646967657374*/
uint8_t sm2_gb_sig_msg[14] = "\x6D\x65\x73\x73\x61\x67\x65\x20\x64\x69\x67\x65\x73\x74"; //-- m

/*IDA GB/T1988: 31323334 35363738 31323334 35363738*/
uint8_t sm2_gb_id_test[16] = "\x31\x32\x33\x34\x35\x36\x37\x38\x31\x32\x33\x34\x35\x36\x37\x38";

/*
r：F5A03B06 48D2C463 0EEAC513 E1BB81A1 5944DA38 27D5B741 43AC7EAC EEE720B3
s：B1B6AA29 DF212FD8 763182BC 0D421CA1 BB9038FD 1F7F42D4 840B69C4 85BBC1AA*/

uint8_t sm2_gb_ver_msg[64] = "\xF5\xA0\x3B\x06\x48\xD2\xC4\x63\x0E\xEA\xC5\x13\xE1\xBB\x81\xA1\x59\x44\xDA\x38\x27\xD5\xB7\x41\x43\xAC\x7E\xAC\xEE\xE7\x20\xB3"  //-- r
                             "\xB1\xB6\xAA\x29\xDF\x21\x2F\xD8\x76\x31\x82\xBC\x0D\x42\x1C\xA1\xBB\x90\x38\xFD\x1F\x7F\x42\xD4\x84\x0B\x69\xC4\x85\xBB\xC1\xAA"; //-- s \xAA


//prv=D26AEC2F7A863978977424908BBDDCF555F903B3044D11BC9DEF868D0DBC89E0
//pub=CE3F31F9066B0B05E9F0C400553765837539297DA9FB6944B0505AED06CD06C78C83975E2D8F90B33FCB0B2AABA1F0D37772350C46E882D36A7D17754A1863E6
uint8_t sm2_v2x_pub_key[64] = "\xCE\x3F\x31\xF9\x06\x6B\x0B\x05\xE9\xF0\xC4\x00\x55\x37\x65\x83\x75\x39\x29\x7D\xA9\xFB\x69\x44\xB0\x50\x5A\xED\x06\xCD\x06\xC7"
                              "\x8C\x83\x97\x5E\x2D\x8F\x90\xB3\x3F\xCB\x0B\x2A\xAB\xA1\xF0\xD3\x77\x72\x35\x0C\x46\xE8\x82\xD3\x6A\x7D\x17\x75\x4A\x18\x63\xE6";
//hash=9E8B6D53EE60191ADB5D47829B07463238AB0F9FE39DDE61D8EE4936329E31FB
uint8_t sm2_v2x_hash_msg[32] = "\x9E\x8B\x6D\x53\xEE\x60\x19\x1A\xDB\x5D\x47\x82\x9B\x07\x46\x32\x38\xAB\x0F\x9F\xE3\x9D\xDE\x61\xD8\xEE\x49\x36\x32\x9E\x31\xFB";
//sig=44995BE0CEC61E79D908E88DB5FA0C7ECCBF3E82961FB2FEBC41018C0837E00B064498F7CEFE8086105BE377E8231D94F6276A373CEEF94825BE9A3A6A4A17C7
uint8_t sm2_v2x_ver_msg[64] = "\x44\x99\x5B\xE0\xCE\xC6\x1E\x79\xD9\x08\xE8\x8D\xB5\xFA\x0C\x7E\xCC\xBF\x3E\x82\x96\x1F\xB2\xFE\xBC\x41\x01\x8C\x08\x37\xE0\x0B"
                              "\x06\x44\x98\xF7\xCE\xFE\x80\x86\x10\x5B\xE3\x77\xE8\x23\x1D\x94\xF6\x27\x6A\x37\x3C\xEE\xF9\x48\x25\xBE\x9A\x3A\x6A\x4A\x17\xC7";
void sm2_test_main(void)
{
    uint32_t ret;
    int fd;
    struct sm2_verify_msg verify_msg;
    int i;
    int pass_num;
    int d = 3;

    ret = 0xff;
    pass_num = 0;
    printf("123666\r\n");
    printf("\r\n ce test start ret =%d\r\n", ret);

    while (d--) {
        printf("123\r\n");
    }

    fd = open("/dev/semidrive-ce2", O_RDWR);

    if (fd < 0) {
        printf("open semidrive-ce2 fair!\n");
        exit(-1);
    }

#if DEFINE_GB_PATTERN
    verify_msg.msg = sm2_gb_sig_msg;
    verify_msg.msg_len = 14;
    verify_msg.sig = sm2_gb_ver_msg;
    verify_msg.sig_len = 64;
    verify_msg.key = sm2_gb_pub_key;
    verify_msg.key_len = 64;
    verify_msg.ret = &ret;

    printf("\r\n ce gb test start ret =%d, verify_msg=%p\r\n", ret, &verify_msg);
    ioctl(fd, SEMIDRIVE_SM2_GET_TIME_STAMP, &i);
    printf("\r\n ce gb msg test start time_stamp =%d,\r\n", i);
    i = 1008;
    ioctl(fd, SEMIDRIVE_SM2_SET_TIME_STAMP, &i);

    for (i = 0; i < 2000; i++) {
        ret = 0xff;
        ioctl(fd, SEMIDRIVE_SM2_VERIFY_MSG, &verify_msg);

        if (ret == 0) {
            pass_num++;
        }
    }

    ioctl(fd, SEMIDRIVE_SM2_SET_TIME_STAMP, &i);
    ioctl(fd, SEMIDRIVE_SM2_GET_TIME_STAMP, &i);
    printf("\r\n ce gb msg test end time_stamp =%d,\r\n", i);
    printf("\r\n ce gb test end ret =%d pass_num=%d\r\n", ret, pass_num);

    ret = 0xff;
#endif

#if DEFINE_V2X_PATTERN

    verify_msg.msg = NULL;
    verify_msg.msg_len = 0;
    verify_msg.sig = NULL;
    verify_msg.sig_len = 0;
    verify_msg.key = &sm2_v2x_pub_key;
    verify_msg.key_len = 64;
    verify_msg.ret = &ret;

    ret = 0xff;
    printf("\r\n ce hash msg test start set key ret =%d,\r\n", ret);
    ioctl(fd, SEMIDRIVE_SM2_SET_PUBKEY, &verify_msg);
    //printf("\r\n ce hash msg test end set key ret =%d,\r\n", ret);

    verify_msg.msg = &sm2_v2x_hash_msg;
    verify_msg.msg_len = 32;
    verify_msg.sig = &sm2_v2x_ver_msg;
    verify_msg.sig_len = 64;
    verify_msg.key = NULL;
    verify_msg.key_len = 0;
    verify_msg.ret = &ret;

    printf("\r\n ce verf msg test start ret =%d,\r\n", ret);
    ioctl(fd, SEMIDRIVE_SM2_GET_TIME_STAMP, &i);
    printf("\r\n ce verf msg test start time_stamp =%d,\r\n", i);
    i = 1009;
    ioctl(fd, SEMIDRIVE_SM2_SET_TIME_STAMP, &i);

    for (i = 0; i < 2000; i++) {
        ret = 0xff;
        ioctl(fd, SEMIDRIVE_SM2_VERIFY_DIGEST, &verify_msg);

        if (ret == 0) {
            pass_num++;
        }
    }

    ioctl(fd, SEMIDRIVE_SM2_SET_TIME_STAMP, &i);
    ioctl(fd, SEMIDRIVE_SM2_GET_TIME_STAMP, &i);
    printf("\r\n ce verf msg test end time_stamp =%d,\r\n", i);
    printf("\r\n ce verf test end ret =%d pass_num=%d\r\n", ret, pass_num);
#endif
    close(fd);

}

/** @brief printf string and Large number buff by %x 16 per line
 *  @param arrey string to display
 *  @param my_buff Large number of addresses to be displayed
 *  @param len len of Large number
 */

void my_printf(char* arrey, uint8_t* my_buff, int len)
{
    int i = 0;
    printf("%s\n", arrey);

    for (i = 0; i < len; i++) {
        if (i % 16 == 0) {
            printf("\n");
        }

        printf("0x%x ", *(my_buff + i));

    }

    printf("\n");
}

uint32_t mem_rev(uint8_t* src, uint32_t len)
{
    uint32_t status;
    uint32_t i = 0;
    uint8_t tmp = 0;

    if (len > RSA_4096_LEN) {
        printf("len > RSA_4096_LEN, len is %d\n", len);
        return 0;
    }

    for (i = 0; i < len / 2; i++) {
        src[i] = src[len - i - 1];
        tmp = src[i];
        src[len - i - 1] = tmp;

    }
}

int memcmp_reverse(uint8_t* src, const uint8_t* dst, uint32_t len)
{
    const unsigned char* su1, *su2;
    int res = 0;

    //for (su1 = src + len - 1, su2 = dst; 0 < len; --su1, ++su2, len--) {
    for (su1 = src, su2 = dst; 0 < len; ++su1, ++su2, len--) {
        //    printf("* su1 is %x , *su2 is %x,src is  %x\n",* su1,*su2);
        if ((res = *su1 - *su2) != 0) {
            printf("memcmp_reverse: error at:%d byte, src=%d ,dst=%d ,src at %p!\n", len, *su1, *su2, src);
            break;
        }
    }

    return res;
}



uint32_t rsa_enc_test(int  filp, uint32_t keysize, buff_addr_type_t addr_type,
                      uint8_t* n, uint8_t* pub_expo, uint8_t* msg, uint8_t* except_s, uint32_t runtime)
{
    uint32_t res;
    rsa_keypair_t rsa_keypair_temp;
    uint8_t*  result32;
    uint32_t result_len = keysize;
    uint64_t cur_time;
    struct rsa_all_information verify_msg;
    memset(result_4096, 0, RSA_4096_LEN);
    rsa_keypair_temp.n = n;
    rsa_keypair_temp.n_len = keysize;
    rsa_keypair_temp.e = pub_expo;
    rsa_keypair_temp.e_len = keysize;
    rsa_keypair_temp.d = NULL;
    rsa_keypair_temp.d_len = keysize;
    verify_msg.rsa_keypair = rsa_keypair_temp;
    verify_msg.msg = msg;
    verify_msg.msg_len = keysize;
    verify_msg.cipher = except_s;
    verify_msg.cipher_len = keysize;
    verify_msg.sig = NULL;
    verify_msg.sig_len = keysize;
    verify_msg.ce_rsa_pad_types_temp = RSA_PADDING_NONE;
    verify_msg.addr_type = addr_type;
    verify_msg.ce_rsa_hashType = ALG_SHA256;
    verify_msg.ret = &res;
    verify_msg.final_result[0] = result_4096;
    verify_msg.final_result_len[0] = RSA_4096_LEN;

    result32 = verify_msg.final_result[0];
    printf("rsa encryption enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_ENC, &verify_msg);
    }

    printf("-------------------------------------------------\n");
    printf("\nres = %d\n", res);

    my_printf("verify_msg.final_result[0] is ", verify_msg.final_result[0], keysize);


    res = memcmp_reverse(/*(void *)(sram_base + RSA_SRAM_DST_OFFSET)*/verify_msg.final_result[0], except_s, keysize);

    if (res) {
//        printf("result is\n %s",verify_msg.final_result[0]);
        printf("\n excepted_result is\n%s", except_s);
        printf("final_result[0] addr is %x\n", verify_msg.final_result[0]);
        printf("result_4096 addr is %x\n", result_4096);
        printf("rsa_encrypt_blk compare result: %d\n\n", res);
    }
    else {
        printf("rsa encryption pass \n");
//        my_printf("result is\n",verify_msg.final_result[0],keysize);
    }

    return res;
}

uint32_t rsa_enc_trav_test(int filp, int cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status = 0;
    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa enc DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");

    if (cmd & 1 << 0) {
        status = rsa_enc_test(filp, RSA_1024_LEN, HAL_EXT_MEM, n_1024,
                              public_expo_1024, msg_1024, cipher_1024, runtime);
    }

    if (cmd & 1 << 1) {
        status |= rsa_enc_test(filp, RSA_2048_LEN, HAL_EXT_MEM, n_2048,
                               public_expo_2048, msg_2048, cipher_2048, runtime);
    }

    if (cmd & 1 << 2) {
        status |= rsa_enc_test(filp, RSA_3072_LEN, HAL_EXT_MEM, n_3072,
                               public_expo_3072, msg_3072, cipher_3072, runtime);
    }

    if (cmd & 1 << 3) {
        status |= rsa_enc_test(filp, RSA_4096_LEN, HAL_EXT_MEM, n_4096,
                               public_expo_4096, msg_4096, cipher_4096, runtime);
    }

    if (status) {
        ret = 1;
        printf("enc is wrong\n");
    }
    else {
        printf("enc is finished\n");
        ret = 0;
    }

    return ret;
}

uint32_t rsa_oaep_enc_test(int  filp, uint32_t keysize, buff_addr_type_t addr_type,
                           uint8_t* n, uint8_t* pub_expo, uint8_t* priv_key,  uint8_t* msg, uint32_t msg_len, uint32_t runtime)
{
    uint32_t res;
    rsa_keypair_t rsa_keypair_temp;
    uint8_t*  result32;
    uint32_t result_len = msg_len;
    uint64_t cur_time;
    struct rsa_all_information verify_msg, verify_cipher;
    memset(result_4096, 0, RSA_4096_LEN);
    memset(result_4096_1, 0, RSA_4096_LEN);
    memset(result_4096_2, 0, RSA_4096_LEN);
    rsa_keypair_temp.n = n;
    rsa_keypair_temp.n_len = keysize;
    rsa_keypair_temp.e = pub_expo;
    rsa_keypair_temp.e_len = keysize;
    rsa_keypair_temp.d = priv_key;
    rsa_keypair_temp.d_len = keysize;
    verify_msg.rsa_keypair = rsa_keypair_temp;
    verify_msg.msg = msg;
    verify_msg.msg_len = msg_len;
    verify_msg.cipher = NULL;
    verify_msg.cipher_len = keysize;
    verify_msg.sig = NULL;
    verify_msg.sig_len = keysize;
    verify_msg.ce_rsa_pad_types_temp = RSA_PADDING_OAEP;
    verify_msg.addr_type = addr_type;
    verify_msg.ce_rsa_hashType = ALG_SHA256;
    verify_msg.ret = &res;
    verify_msg.final_result[0] = result_4096;
    verify_msg.final_result_len[0] = RSA_4096_LEN;
    verify_msg.final_result[1] = result_4096_1;
    verify_msg.final_result_len[1] = RSA_4096_LEN;
    verify_msg.final_result[2] = result_4096_2;
    verify_msg.final_result_len[2] = RSA_4096_LEN;
    uint32_t i = 0;
    printf("rsa oaep encryption enter \n");

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_ENC, &verify_msg);
    }

//    mem_rev(result_4096,keysize);
    printf("rsa oaep encryption over \n");
    verify_cipher.rsa_keypair = rsa_keypair_temp;
    verify_cipher.msg = NULL;
    verify_cipher.msg_len = msg_len;
    verify_cipher.cipher = result_4096;
    verify_cipher.cipher_len = keysize;
    verify_cipher.sig = NULL;
    verify_cipher.sig_len = keysize;
    verify_cipher.ce_rsa_pad_types_temp = RSA_PADDING_OAEP;
    verify_cipher.ce_rsa_hashType = ALG_SHA256;
    verify_cipher.ret = &res;
    verify_cipher.final_result[0] = result_4096_1;
    verify_cipher.final_result_len[0] = RSA_4096_LEN;
    printf("rsa oaep decryption enter \n");

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_DEC, &verify_cipher);
    }

    printf("rsa oaep decryption over------------------------------\n");
    printf("\nres = %d\n", res);

    my_printf("verify_msg.final_result[0] is ", verify_msg.final_result[0], keysize);
    my_printf("verify_cipher.final_result[0] is ", verify_cipher.final_result[0], keysize);

    res = memcmp_reverse(/*(void *)(sram_base + RSA_SRAM_DST_OFFSET)*/ msg, verify_cipher.final_result[0], msg_len);

    if (res) {
//        printf("result is\n %s",verify_msg.final_result[0]);
        printf("\n excepted_result is\n%s", msg);
        printf("final_result[0] addr is %x\n", verify_msg.final_result[0]);
        printf("result_4096 addr is %x\n", result_4096_1);
        printf("rsa_encrypt_blk compare result: %d\n\n", res);
    }
    else {
        printf("rsa oaep encryption pass \n");
//        my_printf("result is\n",verify_msg.final_result[0],keysize);
    }

    return res;
}

uint32_t rsa_oaep_enc_trav_test(int filp, int* cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status = 0;
    uint32_t reduce_len = cmd[1];
    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa oaep enc DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");

    if (cmd[0] & 1 << 0) {
        status = rsa_oaep_enc_test(filp, RSA_1024_LEN, HAL_EXT_MEM, n_1024,
                                   public_expo_1024, private_key_1024 + RSA_1024_LEN, msg_1024 + reduce_len,
                                   RSA_1024_LEN - reduce_len, runtime);
    }

    if (cmd[0] & 1 << 1) {
        status |= rsa_oaep_enc_test(filp, RSA_2048_LEN, HAL_EXT_MEM, n_2048,
                                    public_expo_2048, private_key_2048, msg_2048 + reduce_len,
                                    RSA_2048_LEN - reduce_len, runtime);
    }

    if (cmd[0] & 1 << 2) {
        status |= rsa_oaep_enc_test(filp, RSA_3072_LEN, HAL_EXT_MEM, n_3072,
                                    public_expo_3072, private_key_3072, msg_3072 + reduce_len,
                                    RSA_3072_LEN - reduce_len, runtime);
    }

    if (cmd[0] & 1 << 3) {
        status |= rsa_oaep_enc_test(filp, RSA_4096_LEN, HAL_EXT_MEM, n_4096,
                                    public_expo_4096, private_key_4096, msg_4096 + reduce_len,
                                    RSA_4096_LEN - reduce_len, runtime);
    }

    if (status) {
        ret = 1;
        printf("oaep enc is wrong\n");
    }
    else {
        printf("oaep enc is finished\n");
        ret = 0;
    }

    return ret;
}

uint32_t rsa_oaep_crt_enc_test(int  filp, uint32_t keysize, buff_addr_type_t addr_type, uint8_t* n,
                               uint8_t* pub_expo, uint8_t* p, uint8_t* q, uint8_t* dP, uint8_t* dQ,
                               uint8_t* qInv, uint8_t* msg, uint32_t msg_len, uint32_t runtime)
{
    uint32_t res;
    rsa_keypair_t rsa_keypair_temp;
    uint8_t*  result32;
    uint32_t result_len = msg_len;
    struct rsa_all_information verify_msg;
    memset(result_4096, 0, RSA_4096_LEN);
    memset(result_4096_1, 0, RSA_4096_LEN);
    memset(result_4096_2, 0, RSA_4096_LEN);
    rsa_keypair_temp.n = n;
    rsa_keypair_temp.n_len = keysize;
    rsa_keypair_temp.e = pub_expo;
    rsa_keypair_temp.e_len = keysize;
    rsa_keypair_temp.d = NULL;
    rsa_keypair_temp.d_len = 0;
    verify_msg.rsa_keypair = rsa_keypair_temp;
    verify_msg.msg = msg;
    verify_msg.msg_len = msg_len;
    verify_msg.cipher = NULL;
    verify_msg.cipher_len = keysize;
    verify_msg.sig = NULL;
    verify_msg.sig_len = 0;
    verify_msg.ce_rsa_pad_types_temp = RSA_PADDING_OAEP;
    verify_msg.addr_type = addr_type;
    verify_msg.ce_rsa_hashType = ALG_SHA256;
    verify_msg.ret = &res;
    verify_msg.final_result[0] = result_4096;
    verify_msg.final_result_len[0] = RSA_4096_LEN;
    printf("rsa crt oaep encryption enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_ENC, &verify_msg);
    }

    my_printf("verify_msg.final_result[0] is ", verify_msg.final_result[0], keysize);
    printf("rsa crt oaep encryption over \n");
    struct rsa_crt_decrypt_information verify_cipher;
    verify_cipher.msg = NULL;
    verify_cipher.msg_len = msg_len;
    verify_cipher.cipher = result_4096;
    verify_cipher.cipher_len = keysize;
    verify_cipher.p = p;
    verify_cipher.p_len = keysize;
    verify_cipher.q = q;
    verify_cipher.q_len = keysize;
    verify_cipher.dP = dP;
    verify_cipher.dP_len = keysize;
    verify_cipher.dQ = dQ;
    verify_cipher.dQ_len = keysize;
    verify_cipher.qInv = qInv;
    verify_cipher.qInv_len = keysize;
    verify_cipher.ce_rsa_pad_types_temp = RSA_PADDING_OAEP;
    verify_cipher.ce_rsa_hashType = ALG_SHA256;
    verify_cipher.addr_type = addr_type;
    verify_cipher.ret = &res;
    verify_cipher.final_result[0] = result_4096_1;
    verify_cipher.final_result_len[0] = RSA_4096_LEN;
    result32 = verify_cipher.final_result[0];
    printf("rsa crt oaep decryption enter \n");

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_CRT_DEC, &verify_cipher);
    }

    printf("-------------------------------------------------\n");
    //my_printf("verify_msg.final_result[0] is ",verify_msg.final_result[0],keysize);
    my_printf("verify_cipher.final_result[0] is ", verify_cipher.final_result[0], keysize);

    res = memcmp_reverse(/*(void *)(sram_base + RSA_SRAM_DST_OFFSET)*/ msg, verify_cipher.final_result[0], msg_len);

    if (res) {
//        printf("result is\n %s",verify_msg.final_result[0]);
        printf("\n excepted_result is\n%s", msg);
        printf("final_result[0] addr is %x\n", verify_msg.final_result[0]);
        printf("result_4096 addr is %x\n", result_4096_1);
        printf("rsa_encrypt_blk compare result: %d\n\n", res);
    }
    else {
        printf("rsa oaep encryption pass \n");
//        my_printf("result is\n",verify_msg.final_result[0],keysize);
    }

    return res;


}

uint32_t rsa_oaep_crt_enc_trav_test(int filp, int* cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status = 0;
    uint32_t reduce_len = cmd[1];
    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa oaep enc DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");

    if (cmd[0] & 1 << 0) {
        status = rsa_oaep_crt_enc_test(filp, RSA_1024_LEN, HAL_EXT_MEM, n_1024, public_expo_1024,
                                       p_1024, q_1024, dP_1024, dQ_1024, qInv_1024,
                                       msg_1024 + reduce_len, RSA_1024_LEN - reduce_len, runtime);
    }

    if (status) {
        ret = 1;
        printf("oaep enc is wrong\n");
    }
    else {
        printf("oaep enc is finished\n");
        ret = 0;
    }

    return ret;
}


uint32_t rsa_dec_test(int  filp, uint32_t keysize, buff_addr_type_t addr_type,
                      uint8_t* n, uint8_t* priv_key, uint8_t* cipher, uint8_t* except_s, uint32_t runtime)
{
    uint32_t res;
    rsa_keypair_t rsa_keypair_temp;
    uint8_t*  result32;
    uint32_t result_len = keysize;
    uint64_t cur_time;
    struct rsa_all_information verify_msg;
    memset(result_4096, 0, RSA_4096_LEN);
    rsa_keypair_temp.n = n;
    rsa_keypair_temp.n_len = keysize;
    rsa_keypair_temp.e = NULL;
    rsa_keypair_temp.e_len = keysize;
    rsa_keypair_temp.d = priv_key;
    rsa_keypair_temp.d_len = keysize;
    verify_msg.rsa_keypair = rsa_keypair_temp;
    verify_msg.msg = except_s;
    verify_msg.msg_len = keysize;
    verify_msg.cipher = cipher;
    verify_msg.cipher_len = keysize;
    verify_msg.sig = NULL;
    verify_msg.sig_len = 0;
    verify_msg.ce_rsa_pad_types_temp = RSA_PADDING_NONE;
    verify_msg.addr_type = addr_type;
    verify_msg.ce_rsa_hashType = ALG_SHA256;
    verify_msg.ret = &res;
    verify_msg.final_result[0] = result_4096;
    verify_msg.final_result_len[0] = RSA_4096_LEN;
    result32 = verify_msg.final_result[0];
    printf("rsa decryption enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_DEC, &verify_msg);
    }

    printf("-------------------------------------------------\n");
    printf("\nres = %d\n", res);
    my_printf("verify_msg.final_result[0] is ", verify_msg.final_result[0], keysize);
    res = memcmp_reverse(result32, except_s, keysize);

    if (res) {
        printf("result is\n %s", result32);
        printf("\n excepted_result is\n%s", except_s);
        printf("final_result[0] addr is %x\n", result32);
        printf("result_4096 addr is %x\n", result_4096);
        printf("rsa_decrypt_blk compare result: %d\n\n", res);
    }
    else {
        printf("rsa decryption pass \n");
//        my_printf("result is\n",verify_msg.final_result[0],keysize);
    }

    printf("--------------------------------------------------------------------------------------\n");
    return res;

}
uint32_t rsa_dec_trav_test(int filp, int cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status;

    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa dec DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");

    if (cmd & 1 << 0) {
        status = rsa_dec_test(filp, RSA_1024_LEN, HAL_EXT_MEM, n_1024,
                              private_key_1024 + RSA_1024_LEN, cipher_1024, msg_1024, runtime);
    }

    if (cmd & 1 << 1) {
        status |= rsa_dec_test(filp, RSA_2048_LEN, HAL_EXT_MEM, n_2048,
                               private_key_2048, cipher_2048, msg_2048, runtime);
    }

    if (cmd & 1 << 2) {
        status |= rsa_dec_test(filp, RSA_3072_LEN, HAL_EXT_MEM, n_3072,
                               private_key_3072, cipher_3072, msg_3072, runtime);
    }

    if (cmd & 1 << 3) {
        status |= rsa_dec_test(filp, RSA_4096_LEN, HAL_EXT_MEM, n_4096,
                               private_key_4096, cipher_4096, msg_4096, runtime);
    }

    if (status) {
        ret = 1;
        printf("dec is wrong\n");
    }
    else {
        ret = 0;
        printf("dec is right\n");
    }

    return ret;
}

uint32_t rsa_crt_dec_test(int  filp, uint32_t keysize, buff_addr_type_t addr_type,
                          uint8_t* cipher, uint8_t* p, uint8_t* q, uint8_t* dP, uint8_t* dQ,
                          uint8_t* qInv, uint8_t* except_s, uint32_t runtime)
{
    uint32_t res;

    uint8_t*  result32;
    uint32_t result_len = keysize;
    struct rsa_crt_decrypt_information verify_msg;
    memset(result_4096, 0, RSA_4096_LEN);
    verify_msg.msg = except_s;
    verify_msg.msg_len = keysize;
    verify_msg.cipher = cipher;
    verify_msg.cipher_len = keysize;
    verify_msg.p = p;
    verify_msg.p_len = keysize;
    verify_msg.q = q;
    verify_msg.q_len = keysize;
    verify_msg.dP = dP;
    verify_msg.dP_len = keysize;
    verify_msg.dQ = dQ;
    verify_msg.dQ_len = keysize;
    verify_msg.qInv = qInv;
    verify_msg.qInv_len = keysize;
    verify_msg.ce_rsa_pad_types_temp = RSA_PADDING_NONE;
    verify_msg.ce_rsa_hashType = ALG_SHA256;
    verify_msg.addr_type = addr_type;


    verify_msg.ret = &res;
    verify_msg.final_result[0] = result_4096;
    verify_msg.final_result_len[0] = RSA_4096_LEN;
    result32 = verify_msg.final_result[0];
    printf("rsa decryption enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_CRT_DEC, &verify_msg);
    }

    printf("-------------------------------------------------\n");
    printf("\nres = %d\n", res);
    my_printf("verify_msg.final_result[0] is ", verify_msg.final_result[0], keysize);
    res = memcmp_reverse(result32, except_s, keysize);

    if (res) {
        printf("result is\n %s", result32);
        printf("\n excepted_result is\n%s", except_s);
        printf("final_result[0] addr is %x\n", result32);
        printf("result_4096 addr is %x\n", result_4096);
        printf("rsa_crt_decrypt_blk compare result: %d\n\n", res);
    }
    else {
        printf("rsa crt decryption pass \n");
//        my_printf("result is\n",verify_msg.final_result[0],keysize);
    }

    printf("--------------------------------------------------------------------------------------\n");
    return res;
}

uint32_t rsa_dec_crt_trav_test(int filp, int cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status;
    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa crt dec DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");

    if (cmd & 1 << 0) {
        status = rsa_crt_dec_test(filp, RSA_1024_LEN, HAL_EXT_MEM, cipher_1024, p_1024, q_1024,
                                  dP_1024, dQ_1024, qInv_1024, msg_1024, runtime);
    }

    if (status) {
        ret = 1;
        printf("crt dec is wrong\n");
    }
    else {
        ret = 0;
        printf("crt dec is right\n");
    }

    return ret;
}


uint32_t rsa_sig_gen_test(int filp, uint32_t keysize, buff_addr_type_t addr_type,
                          uint8_t* n, uint8_t* priv_key, uint8_t* msg, uint8_t* except_s, uint32_t runtime)
{
    uint32_t res;
    rsa_keypair_t rsa_keypair_temp;
    uint8_t*  result32;
    uint32_t result_len = keysize;
    uint64_t cur_time;
    struct rsa_all_information verify_msg;
    memset(result_4096, 0, RSA_4096_LEN);
    rsa_keypair_temp.n = n;
    rsa_keypair_temp.n_len = keysize;
    rsa_keypair_temp.e = NULL;
    rsa_keypair_temp.e_len = keysize;
    rsa_keypair_temp.d = priv_key;
    rsa_keypair_temp.d_len = keysize;
    verify_msg.rsa_keypair = rsa_keypair_temp;
    verify_msg.msg = msg;
    verify_msg.msg_len = keysize;
    verify_msg.cipher = NULL;
    verify_msg.cipher_len = keysize;
    verify_msg.sig = except_s;
    verify_msg.sig_len = keysize;
    verify_msg.ce_rsa_pad_types_temp = RSA_PADDING_NONE;
    verify_msg.addr_type = addr_type;
    verify_msg.ce_rsa_hashType = ALG_SHA256;
    verify_msg.ret = &res;
    verify_msg.final_result[0] = result_4096;
    verify_msg.final_result_len[0] = RSA_4096_LEN;
    result32 = verify_msg.final_result[0];
    printf("rsa sig gen enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_SIG_GEN, &verify_msg);
    }

    printf("-------------------------------------------------\n");
    printf("\nres = %d\n", res);
    my_printf("verify_msg.final_result[0] is ", verify_msg.final_result[0], keysize);
    res = memcmp_reverse(result32, except_s, keysize);

    if (res) {
        printf("result is\n %s", result32);
        printf("\n excepted_result is\n%s", except_s);
        printf("final_result[0] addr is %x\n", result32);
        printf("result_4096 addr is %x\n", result_4096);
        printf("rsa_rsa_sig_gen_blk compare result: %d\n\n", res);
    }
    else {
        printf("rsa sign gen pass \n");
    }

    printf("--------------------------------------------------------------------------------------\n");
    return res;
}

uint32_t rsa_sig_gen_trav_test(int filp, int cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status;
    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa sig DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");

    if (cmd & 1 << 0) {
        status =  rsa_sig_gen_test(filp, RSA_1024_LEN, HAL_EXT_MEM, n_1024,
                                   private_key_1024 + RSA_1024_LEN, msg_1024, sig_1024, runtime);
    }

    if (cmd & 1 << 1) {
        status |= rsa_sig_gen_test(filp, RSA_2048_LEN, HAL_EXT_MEM, n_2048,
                                   private_key_2048, msg_2048, sig_2048, runtime);
    }

    if (cmd & 1 << 2) {
        status |= rsa_sig_gen_test(filp, RSA_3072_LEN, HAL_EXT_MEM, n_3072,
                                   private_key_3072, msg_3072, sig_3072, runtime);
    }

    if (cmd & 1 << 3) {
        status |= rsa_sig_gen_test(filp, RSA_4096_LEN, HAL_EXT_MEM, n_4096,
                                   private_key_4096, msg_4096, sig_4096, runtime);
    }

    if (status) {
        ret = 1;
        printf("sig gen is wrong\n");
    }
    else {
        ret = 0;
        printf("sin gen is right\n");
    }

    return ret;
}

uint32_t rsa_pss_sig_gen_test(int filp, uint32_t keysize, buff_addr_type_t addr_type,
                              uint8_t* n, uint8_t* pub_expo, uint8_t* priv_key, uint8_t* msg, uint32_t msg_len, uint32_t runtime)
{
    uint32_t res;
    rsa_keypair_t rsa_keypair_temp;
    uint8_t*  result32;
    uint32_t result_len = keysize;
    uint64_t cur_time;
    struct rsa_all_information verify_msg, verify_sig;
    memset(result_4096, 0, RSA_4096_LEN);
    rsa_keypair_temp.n = n;
    rsa_keypair_temp.n_len = keysize;
    rsa_keypair_temp.e = pub_expo;
    rsa_keypair_temp.e_len = keysize;
    rsa_keypair_temp.d = priv_key;
    rsa_keypair_temp.d_len = keysize;
    verify_msg.rsa_keypair = rsa_keypair_temp;
    verify_msg.msg = msg;
    verify_msg.msg_len = msg_len;
    verify_msg.cipher = NULL;
    verify_msg.cipher_len = keysize;
    verify_msg.sig = NULL;
    verify_msg.sig_len = keysize;
    verify_msg.ce_rsa_pad_types_temp = RSA_PADDING_PSS;
    verify_msg.addr_type = addr_type;
    verify_msg.ce_rsa_hashType = ALG_SHA256;
    verify_msg.ret = &res;
    verify_msg.final_result[0] = result_4096;
    verify_msg.final_result_len[0] = RSA_4096_LEN;
    result32 = verify_msg.final_result[0];
    printf("rsa pss sig gen enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_SIG_GEN, &verify_msg);
    }

    printf("-------------------------------------------------\n");
    verify_sig.rsa_keypair = rsa_keypair_temp;
    verify_sig.msg = msg;
    verify_sig.msg_len = msg_len;
    verify_sig.cipher = NULL;
    verify_sig.cipher_len = keysize;
    verify_sig.sig = result_4096;
    verify_sig.sig_len = keysize;
    verify_sig.ce_rsa_pad_types_temp = RSA_PADDING_PSS;
    verify_sig.addr_type = addr_type;
    verify_sig.ce_rsa_hashType = ALG_SHA256;
    verify_sig.ret = &res;
    verify_sig.final_result[0] = result_4096_1;
    verify_sig.final_result_len[0] = RSA_4096_LEN;
    printf("rsa pss sig verify enter \n");

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_SIG_VERIFY, &verify_sig);
    }

    printf("-------------------------------------------------\n");
    printf("\nres = %d\n", res);

    if (res) {

        printf("rsa_rsa_sig_verify_blk compare result: %d\n\n", res);
    }
    else {
        printf("rsa sign verify pass \n");
    }

    return res;
}


uint32_t rsa_pss_sig_gen_trav_test(int filp, int* cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status = 0;
    uint32_t reduce_len = cmd[1];
    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa oaep enc DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");

    if (cmd[0] & 1 << 0) {
        status = rsa_pss_sig_gen_test(filp, RSA_1024_LEN, HAL_EXT_MEM,
                                      n_1024, public_expo_1024, private_key_1024 + RSA_1024_LEN,
                                      msg_1024 + reduce_len, RSA_1024_LEN - reduce_len, runtime);
    }

    if (cmd[0] & 1 << 1) {
        status |= rsa_pss_sig_gen_test(filp, RSA_2048_LEN, HAL_EXT_MEM,
                                       n_2048, public_expo_2048, private_key_2048, msg_2048 + reduce_len,
                                       RSA_2048_LEN - reduce_len, runtime);
    }

    if (cmd[0] & 1 << 2) {
        status |= rsa_pss_sig_gen_test(filp, RSA_3072_LEN, HAL_EXT_MEM,
                                       n_3072, public_expo_3072, private_key_3072, msg_3072 + reduce_len,
                                       RSA_3072_LEN - reduce_len, runtime);
    }

    if (cmd[0] & 1 << 3) {
        status |= rsa_pss_sig_gen_test(filp, RSA_4096_LEN, HAL_EXT_MEM,
                                       n_4096, public_expo_4096, private_key_4096, msg_4096 + reduce_len,
                                       RSA_4096_LEN - reduce_len, runtime);
    }

    if (status) {
        ret = 1;
        printf("oaep enc is wrong\n");
    }
    else {
        printf("oaep enc is finished\n");
        ret = 0;
    }

    return ret;
}

uint32_t rsa_sig_verify_test(int filp, uint32_t keysize, buff_addr_type_t addr_type,
                             uint8_t* n, uint8_t* pub_expo, uint8_t* msg, uint8_t* sig, uint32_t runtime)
{
    uint32_t res;
    rsa_keypair_t rsa_keypair_temp;
    uint8_t*  result32;
    uint32_t result_len = keysize;
    uint64_t cur_time;
    struct rsa_all_information verify_msg;
    memset(result_4096, 0, RSA_4096_LEN);
    rsa_keypair_temp.n = n;
    rsa_keypair_temp.n_len = keysize;
    rsa_keypair_temp.e = pub_expo;
    rsa_keypair_temp.e_len = keysize;
    rsa_keypair_temp.d = NULL;
    rsa_keypair_temp.d_len = keysize;
    verify_msg.rsa_keypair = rsa_keypair_temp;
    verify_msg.msg = msg;
    verify_msg.msg_len = keysize;
    verify_msg.cipher = NULL;
    verify_msg.cipher_len = 0;
    verify_msg.sig = sig;
    verify_msg.sig_len = keysize;
    verify_msg.ce_rsa_pad_types_temp = RSA_PADDING_NONE;
    verify_msg.addr_type = addr_type;
    verify_msg.ce_rsa_hashType = ALG_SHA256;
    verify_msg.ret = &res;
    verify_msg.final_result[0] = result_4096;
    verify_msg.final_result_len[0] = RSA_4096_LEN;
    result32 = verify_msg.final_result[0];
    printf("rsa sig verify enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_SIG_VERIFY, &verify_msg);
    }

    printf("-------------------------------------------------\n");
    printf("\nres = %d\n", res);

    if (res) {

        printf("rsa_rsa_sig_verify_blk compare result: %d\n\n", res);
    }
    else {
        printf("rsa sign verify pass \n");
    }

    printf("--------------------------------------------------------------------------------------\n");
    return res;
}

uint32_t rsa_sig_verify_trav_test(int filp, int cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status;
    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa sig DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");

    if (cmd & 1 << 0) {
        status =  rsa_sig_verify_test(filp, RSA_1024_LEN, HAL_EXT_MEM,
                                      n_1024, public_expo_1024, msg_1024, sig_1024, runtime);
    }

    if (cmd & 1 << 1) {
        status =  rsa_sig_verify_test(filp, RSA_2048_LEN, HAL_EXT_MEM,
                                      n_2048, public_expo_2048, msg_2048, sig_2048, runtime);
    }

    if (cmd & 1 << 2) {
        status =  rsa_sig_verify_test(filp, RSA_3072_LEN, HAL_EXT_MEM,
                                      n_3072, public_expo_3072, msg_3072, sig_3072, runtime);
    }

    if (cmd & 1 << 3) {
        status =  rsa_sig_verify_test(filp, RSA_4096_LEN, HAL_EXT_MEM,
                                      n_4096, public_expo_4096, msg_4096, sig_4096, runtime);
    }

    if (status) {
        ret = 1;
        printf("sig verify is wrong\n");
    }
    else {
        ret = 0;
        printf("sig verify is right\n");
    }

    return ret;

}

uint32_t rsa_priv_key_gen_test(int filp, uint32_t keysize, buff_addr_type_t add_type,
                               uint8_t* pub_expo, uint8_t* p, uint8_t* q, uint8_t* except_n,
                               uint8_t* except_priv_key, uint32_t runtime)
{
    uint32_t res;
    uint8_t*  result32;
    uint32_t result_len = keysize;
    uint64_t cur_time;
    struct rsa_priv_key_gen_information verify_key;
    memset(result_4096, 0, RSA_4096_LEN);
    memset(result_4096_1, 0, RSA_4096_LEN);
    memset(result_4096_2, 0, RSA_4096_LEN);
    memset(result_4096_3, 0, RSA_4096_LEN);
    verify_key.p = p;
    verify_key.p_len = keysize;
    verify_key.q = q;
    verify_key.q_len = keysize;
    verify_key.n = NULL;
    verify_key.n_len = keysize;
    verify_key.e = pub_expo;
    verify_key.e_len = keysize;
    verify_key.d = NULL;
    verify_key.d_len = keysize;
    verify_key.final_result[0] = result_4096;
    verify_key.final_result[1] = result_4096_1;
    verify_key.final_result[2] = result_4096_2;
    verify_key.final_result[3] = result_4096_3;
    verify_key.final_result_len[0] = RSA_4096_LEN;
    verify_key.final_result_len[1] = RSA_4096_LEN;
    verify_key.final_result_len[2] = RSA_4096_LEN;
    verify_key.final_result_len[3] = RSA_4096_LEN;
    verify_key.ret = &res;
    printf("rsa priv key gen enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_PRIV_KEY_GEN, &verify_key);
    }

    printf("-------------------------------------------------\n");
    printf("\nres = %d\n", res);
    my_printf("verify_key.final_result[0] is ", verify_key.final_result[0], keysize);
    my_printf("verify_key.final_result[0] is ", verify_key.final_result[1], keysize);
    res = memcmp_reverse(except_n, verify_key.final_result[0], keysize);

    if (res) {
        printf("result is\n %s", verify_key.final_result[0]);
        printf("\n excepted_result is\n%s", except_n);
        printf("final_result[0] addr is %x\n", verify_key.final_result[0]);
        printf("rsa_rsa_priv_key_gen_blk compare result: %d\n\n", res);
    }

    res = memcmp_reverse(except_priv_key, verify_key.final_result[1], keysize);

    if (res) {
        res = 1;
        printf("result is\n %s", verify_key.final_result[1]);
        printf("\n excepted_result is\n%s", except_priv_key);
        printf("final_result[1] addr is %x\n", verify_key.final_result[1]);
        printf("rsa_rsa_priv_key_gen_blk compare result: %d\n\n", res);
    }
    else {
        res = 0;
        printf("rsa key priv key gen pass \n");
    }

    printf("--------------------------------------------------------------------------------------\n");

    return res;
}


uint32_t rsa_priv_key_gen_trav_test(int filp, int cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status;
    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa priv key gem DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");
    status =  rsa_priv_key_gen_test(filp, RSA_1024_LEN, HAL_EXT_MEM, public_expo_1024,
                                    p_1024, q_1024, n_1024, private_key_1024 + RSA_1024_LEN, runtime);

    if (status) {
        ret = 1;
        printf("priv key gen is wrong\n");
    }
    else {
        ret = 0;
        printf("priv key gen is right\n");
    }

    return ret;
}

int32_t rsa_crt_key_gen_test(int filp, uint32_t keysize, buff_addr_type_t add_type,
                             uint8_t* p, uint8_t* q, uint8_t* priv_key, uint8_t* except_dP,
                             uint8_t* except_dQ, uint8_t* except_qInv, uint32_t runtime)
{
    uint32_t res;
    uint8_t*  result32;
    uint32_t result_len = keysize;
    uint64_t cur_time;
    struct rsa_crt_key_gen_information verify_key;
    memset(result_4096, 0, RSA_4096_LEN);
    memset(result_4096_1, 0, RSA_4096_LEN);
    memset(result_4096_2, 0, RSA_4096_LEN);
    memset(result_4096_3, 0, RSA_4096_LEN);
    verify_key.p = p;
    verify_key.p_len = keysize;
    verify_key.q = q;
    verify_key.q_len = keysize;
    verify_key.d = priv_key;
    verify_key.d_len = keysize;
    verify_key.dP = except_dP;
    verify_key.dP_len = keysize;
    verify_key.dQ = except_dQ;
    verify_key.dQ_len = keysize;
    verify_key.qInv = except_qInv;
    verify_key.inv_len = keysize;

    verify_key.final_result[0] = result_4096;
    verify_key.final_result[1] = result_4096_1;
    verify_key.final_result[2] = result_4096_2;
    verify_key.final_result_len[0] = RSA_4096_LEN;
    verify_key.final_result_len[1] = RSA_4096_LEN;
    verify_key.final_result_len[2] = RSA_4096_LEN;
    verify_key.ret = &res;

    printf("rsa crt key gen enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_CRT_KEY_GEN, &verify_key);
    }

    printf("-------------------------------------------------\n");
    printf("\nres = %d\n", res);
    my_printf("verify_key.final_result[0] is ", verify_key.final_result[0], keysize);
    my_printf("verify_key.final_result[0] is ", verify_key.final_result[1], keysize);
    my_printf("verify_key.final_result[0] is ", verify_key.final_result[2], keysize);
    my_printf("dP_1024 is ", dP_1024, keysize);
    my_printf("dQ_1024 is ", dQ_1024, keysize);
    my_printf("qInv_1024 is ", qInv_1024, keysize);
    my_printf("except_dP is ", except_dP, keysize);
    my_printf("except_dQ is ", except_dQ, keysize);
    my_printf("except_qInv is ", except_qInv, keysize);

    res = memcmp_reverse(except_dP, verify_key.final_result[0], keysize);

    if (res) {
        printf("result is\n %s", verify_key.final_result[0]);
        printf("\n excepted_result is\n%s", except_dP);
        printf("final_result[0] addr is %x\n", verify_key.final_result[0]);
        printf("rsa_rsa_priv_key_gen_blk compare result: %d\n\n", res);
    }

    res = memcmp_reverse(except_dQ, verify_key.final_result[1], keysize);

    if (res) {
        printf("result is\n %s", verify_key.final_result[1]);
        printf("\n excepted_result is\n%s", except_dQ);
        printf("final_result[1] addr is %x\n", verify_key.final_result[1]);
        printf("rsa_rsa_priv_key_gen_blk compare result: %d\n\n", res);
    }

    res = memcmp_reverse(except_qInv, verify_key.final_result[2], keysize);

    if (res) {
        printf("result is\n %s", verify_key.final_result[2]);
        printf("\n excepted_result is\n%s", except_qInv);
        printf("final_result[1] addr is %x\n", verify_key.final_result[2]);
        printf("rsa_rsa_priv_key_gen_blk compare result: %d\n\n", res);
    }
    else {
        printf("rsa key priv key gen pass \n");
    }

    printf("--------------------------------------------------------------------------------------\n");

    return res;

}


uint32_t  rsa_crt_key_gen_trav_test(int filp, int cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status;
    printf("☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆____________rsa crt key gem DDR base test begin________________☆☆☆☆☆☆☆☆☆☆☆☆☆☆\n");
    status =  rsa_crt_key_gen_test(filp, RSA_1024_LEN, HAL_EXT_MEM, p_1024, q_1024,
                                   private_key_1024 + RSA_1024_LEN, dP_1024, dQ_1024, qInv_1024, runtime);

    if (status) {
        ret = 1;
        printf("crt key gen is wrong\n");
    }
    else {
        ret = 0;
        printf("crt key gen is right\n");
    }

    return ret;
}

uint32_t rsa_key_gen_test(int filp, uint32_t keysize, buff_addr_type_t add_type,
                          uint8_t* e, uint32_t runtime)
{
    uint32_t res;
    uint8_t*  result32;
    uint32_t result_len = keysize;
    uint64_t cur_time;
    struct rsa_key_gen_information verify_key;
    memset(result_4096, 0, RSA_4096_LEN);
    memset(result_4096_1, 0, RSA_4096_LEN);
    memset(result_4096_2, 0, RSA_4096_LEN);
    memset(result_4096_3, 0, RSA_4096_LEN);
    verify_key.keysize = keysize;
    verify_key.e = e;
    verify_key.e_len = keysize;
    verify_key.p = NULL;
    verify_key.p_len = keysize;
    verify_key.q = NULL;
    verify_key.q_len = keysize;
    verify_key.n = NULL;
    verify_key.n_len = keysize;
    verify_key.d = NULL;
    verify_key.d_len = keysize;
    verify_key.final_result[0] = result_4096;
    verify_key.final_result[1] = result_4096_1;
    verify_key.final_result[2] = result_4096_2;
    verify_key.final_result[3] = result_4096_3;
    verify_key.final_result_len[0] = RSA_4096_LEN;
    verify_key.final_result_len[1] = RSA_4096_LEN;
    verify_key.final_result_len[2] = RSA_4096_LEN;
    verify_key.final_result_len[3] = RSA_4096_LEN;
    verify_key.ret = &res;
    printf("rsa  key gen enter \n");
    uint32_t i = 0;

    for (i = 0; i < runtime; i ++) {
        ioctl(filp, SEMIDRIVE_RSA_KEY_GEN, &verify_key);
    }

    printf("-------------------------------------------------\n");
    printf("\nres = %d\n", res);
    my_printf("verify_key.final_result[0] n is ", verify_key.final_result[0], keysize); //n
    my_printf("verify_key.final_result[1] p is ", verify_key.final_result[1], keysize); //p
    my_printf("verify_key.final_result[2] q is ", verify_key.final_result[2], keysize); //q
    my_printf("verify_key.final_result[3] d is ", verify_key.final_result[3], keysize); //d
    return res;
}

uint32_t rsa_key_gen_trav_test(int filp, int cmd, uint32_t runtime)
{
    uint32_t ret;
    uint32_t status;
    uint32_t* public_expo_buff;
    uint32_t keysize = cmd;

    switch (keysize) {
        case RSA_1024_LEN:
            public_expo_buff = public_expo_1024;
            break;

        case RSA_2048_LEN:
            public_expo_buff = public_expo_2048;
            break;

        case RSA_3072_LEN:
            public_expo_buff = public_expo_3072;
            break;

        case RSA_4096_LEN:
            public_expo_buff = public_expo_4096;
            break;

        default:
            ret = 1;
            printf("key gen is wrong\n");
            return ret;

    }

    printf("using keysize\n");
    status = rsa_key_gen_test(filp, keysize, HAL_EXT_MEM, public_expo_buff, runtime);

    if (status) {
        ret = 1;
        printf("key gen is wrong\n");
    }
    else {
        ret = 0;
        printf("key gen is right\n");
    }

    return ret;

}


uint32_t rsa_test_main(int* cmd_num, int parament_num)
{
    int ret = 0;
    int fd;
    struct rsa_all_information verify_msg;
    int i;
    ret = 0xff;
    printf("rsa_enc_test\n");
    fd = open("/dev/semidrive-ce2", O_RDWR);

    if (fd < 0) {
        printf("open semidrive-ce2 fair\n");
        exit(-1);
    }

    if (cmd_num[1] & 1 << RSA_ENC_TRAV_TEST) {                  //1
        ret = rsa_enc_trav_test(fd, cmd_num[2], cmd_num[3]);
    }

    if (cmd_num[1] & 1 << RSA_DEC_TRAV_TEST) {                  //2
        ret |= rsa_dec_trav_test(fd, cmd_num[2], cmd_num[3]);
    }

    if (cmd_num[1] & 1 << RSA_CRT_DEC_TRAV_TEST) {              //4
        ret |= rsa_dec_crt_trav_test(fd, cmd_num[2], cmd_num[3]);
    }

    if (cmd_num[1] & 1 << RSA_SIG_GEN_TRAV_TEST) {              //8
        ret |= rsa_sig_gen_trav_test(fd, cmd_num[2], cmd_num[3]);
    }

    if (cmd_num[1] & 1 << RSA_SIG_VERIFY_TRAV_TEST) {           //16
        ret |= rsa_sig_verify_trav_test(fd, cmd_num[2], cmd_num[3]);
    }

    if (cmd_num[1] & 1 << RSA_PRIV_KEY_GEN_TRAV_TEST) {         //32
        ret |= rsa_priv_key_gen_trav_test(fd, cmd_num[2], cmd_num[3]);
    }

    if (cmd_num[1] & 1 << RSA_CRT_KEY_GEN_TRAV_TEST) {          //64
        ret |= rsa_crt_key_gen_trav_test(fd, cmd_num[2], cmd_num[3]);
    }

    if (cmd_num[1] & 1 << RSA_KEY_GEN_TRAV_TEST) {              //128
        ret |= rsa_key_gen_trav_test(fd, cmd_num[2], cmd_num[3]);
    }

    if (cmd_num[1] & 1 << RSA_OAEP_ENC_TRAV_TEST) {
        ret |= rsa_oaep_enc_trav_test(fd, cmd_num + 2, cmd_num[4]);
    }

    if (cmd_num[1] & 1 << RSA_PSS_SIG_GEN_TRAV_TEST) {
        ret |= rsa_pss_sig_gen_trav_test(fd, cmd_num + 2, cmd_num[4]);
    }

    if (cmd_num[1] & 1 << RSA_OAEP_CRT_ENC_TRAV_TEST) {
        ret |= rsa_oaep_crt_enc_trav_test(fd, cmd_num + 2, cmd_num[4]);
    }

}

void ce_page_operate(int* cmd_num)
{
    int fd;
    fd = open("/dev/semidrive-ce2", O_RDWR);

    if (fd < 0) {
        printf("open semidrive-ce2 fair\n");
        exit(-1);
    }

    if (cmd_num[0] == 1) {
        ioctl(fd, SEMIDRIVE_CE_GET_PAGE, &cmd_num[0]);
        printf("SEMIDRIVE_CE_GET_PAGE\n");

    }
    else if (cmd_num[0] == 2) {
        ioctl(fd, SEMIDRIVE_CE_FREE_PAGE, &cmd_num[0]);
        printf("SEMIDRIVE_CE_FREE_PAGE\n");
    }
    else if (cmd_num[0] == 3) {
        ioctl(fd, SEMIDRIVE_CE_CLEAR_PAGE, &cmd_num[0]);
        printf("SEMIDRIVE_CE_CLEAR_PAGE\n");
    }
    else {
        printf("wrong cmd\n");
    }
}

//入参都是数字
//第一位是选择加密种类，rsa、sm2等等
//第二位是选择操作方式，如加密、解密、签名、验签等
//第三位选择要操作的长度，RSA_1024_LEN等等
//第四位选择要操作的次数。
//如果选择使用1次rsa的1024位加密功能，在linux操作界面输入
//ce-test 2 1 1 1
//ce-test 2 128 128 1 代表生成1次128字节的RSA钥匙
//ce-test 2 256 1 80 1 代表n为128字节长度，明文是以msg_1024+80为起地址，结尾为msg_1024+127的数组，进行RSA的OAEP填充加密。
int main(int argc, char* argv[])
{
    int cmd_num[10] = {0};
    int i;

    if (argc < 4) {
        printf("input parameters is insufficient \n");
    }

    for (i = 0; i < argc - 1 ; i++) {
        cmd_num[i] = atoi(argv[i + 1]);
    }

    if (cmd_num[0] & 1 << SM2_TEST_CMD) {
        sm2_test_main();
    }

    if (cmd_num[0] & 1 << RSA_TEST_CMD) {
        rsa_test_main(cmd_num, i);
    }

    if (cmd_num[0] == 4) {
        ce_page_operate(cmd_num);
    }

    return 0;
}
