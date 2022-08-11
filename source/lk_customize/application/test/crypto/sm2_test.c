/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <stdio.h>
#include <string.h>

#include <app.h>
#include <lib/console.h>

#include <sd_sm2.h>
#include <trace.h>

#include "ecc_data.h"
#include "ecdsa_data.h"
#include "ce_test.h"

#define LOCAL_TRACE 0 //close local trace 1->0

//for sm2 IP owner pattern test, need modify the following function and parameter

uint32_t sm2_test_fail = 0;

//SM2 Pattern begin
uint8_t  __attribute__((aligned(CACHE_LINE))) sm2_ref_p256_sha256[64] =
    "\x6f\x9f\xe2\xbe\xd9\x4e\xe3\xb1\x90\x52\xfb\x1f\xf8\x66\x2f\xe5"
    "\x51\x41\xb2\x56\x73\x1d\x00\xfd\x2a\x29\xf8\x04\xb9\x33\xfb\x92"  //r
    "\x5b\x66\x63\x13\xe1\x6b\xac\x7c\xca\xba\x81\xc6\x70\xf1\xc3\x28"
    "\x03\x1a\x34\x87\x06\x55\xb4\x06\x97\x03\x69\x66\x77\xce\x08\x86";  //s

uint8_t  __attribute__((aligned(CACHE_LINE))) sm2_ref_p192_sha256[48 + 16] =
    "\x78\xbd\x0f\x23\xd6\x25\x5c\xca\x82\x45\xa2\x5c\x14\xaa\xa6\x15\x72\x3f\x2d\xd8\x87\x6e\x33\x57"  //r
    "\x4b\x57\x81\x89\xc8\xfe\x16\x00\x95\xbb\x90\x56\x18\x72\x94\x97\xaf\x16\x39\x21\x0c\x92\xb4\x82"; //s

uint8_t  __attribute__((aligned(CACHE_LINE))) sm2_ref_p384_sha256[96] =
    "\x30\xea\x51\x4f\xc0\xd3\x8d\x82\x08\x75\x6f\x06\x81\x13\xc7\xcb"
    "\xc4\xbc\x1e\x28\x76\x8b\x3f\x76\x86\x01\xaf\x43\xd8\xfa\x33\xe4"
    "\x46\xa3\xd6\x25\x85\xb5\xc9\xfc\x0d\x2d\xf8\x2a\xf0\x9e\x8a\xd1"  //r
    "\x51\x30\x4c\x5e\x31\x40\x09\x11\xd9\x87\xcc\x98\xb8\x23\x4c\xbe"
    "\xce\xd7\xfb\x76\x7b\x44\x11\xb0\xcf\xf8\x91\x3e\xcb\xbe\xe2\x11"
    "\xb0\x50\x47\x97\x75\xc1\x70\xd5\x04\xed\x52\xe8\x0e\xa8\x1b\x08"; //s

uint8_t  __attribute__((aligned(CACHE_LINE))) sm2_ref_p521_sha256[132 + 28] =
    "\x00\xee\x8d\x07\x64\x7e\xd8\x7c\x26\xed\x31\x4a\x53\x17\x17\x37"
    "\x06\x3e\x21\x0a\xf8\x72\x6b\xf4\xe2\x91\xee\x0a\xe8\xc1\xc3\x59"
    "\xaf\xaf\xf2\x2c\xa5\x75\x7c\x6c\x17\xf3\x75\x05\x81\xe3\xf9\x03"
    "\xaf\x75\xe2\xf8\xda\xf0\xf4\x50\x05\xbf\x0c\x19\xbf\x33\x42\xd6"
    "\xfc\xf4"  //r
    "\x00\x10\xef\x44\xd3\x2f\x15\x3e\x84\xe7\x87\x8a\x0b\xa4\x34\x16"
    "\x54\x9b\x63\x57\x66\x14\xb1\x15\x8b\x18\xfc\xd8\x25\x32\x0a\xe3"
    "\xab\xc5\x2b\x8d\x7a\xbf\x98\x8c\xf2\x1b\x74\xde\x93\x85\xa1\xab"
    "\x92\xb9\x0a\x78\x5d\xfb\x56\x3d\xc6\x5b\x0e\xa3\xe0\xfc\x99\x26"
    "\x92\x4a"; //s

//e521_sha256
uint8_t  __attribute__((aligned(CACHE_LINE))) sm2_ref_e521_sha256[132 + 28] =
    "\x00\x51\xa2\x08\xda\xd3\xcb\x53\x4f\xe5\x05\x7d\x3c\xf8\xe9\x03"
    "\xa1\x8d\x20\x7a\xfc\xa7\x3c\x54\x93\x4c\x41\x92\xcc\x20\x02\x4e"
    "\x5f\x82\x6e\xc6\x96\x0f\xe5\xbc\xed\xf0\x01\x3d\xae\x03\x92\xa2"
    "\xd6\x6d\x3c\xcd\xfa\x56\xab\x15\xe7\x28\x2a\x01\xaa\xd1\xc9\x61"
    "\xc0\x60"  //r
    "\x00\x4f\x54\xbf\x51\x72\x0c\x8a\x7b\x35\xa9\xf7\xaf\xf3\x43\xe7"
    "\x51\x52\x62\xe0\x60\x80\x8f\xf9\x61\x2d\x80\x23\x54\xec\xa2\x9d"
    "\x7e\x0f\x17\xc6\x46\xdb\xa4\x4c\x86\x83\x2a\xf3\xaa\x9b\x02\x45"
    "\x4e\xc7\xb2\xbe\x81\xef\x5b\xfb\xb2\x2d\x80\xa0\xb5\x30\x05\xa9"
    "\x87\xba"; //s

//IP owner's pattern begin
static const uint8_t __attribute__((aligned(CACHE_LINE))) ecc_p256_params_ip[] = {
    0x85, 0x42, 0xd6, 0x9e, 0x4c, 0x04, 0x4f, 0x18, 0xe8, 0xb9, 0x24, 0x35, 0xbf, 0x6f, 0xf7, 0xde, 0x45, 0x72, 0x83, 0x91, 0x5c, 0x45, 0x51, 0x7d, 0x72, 0x2e, 0xdb, 0x8b, 0x08, 0xf1, 0xdf, 0xc3,     //q
    0x85, 0x42, 0xd6, 0x9e, 0x4c, 0x04, 0x4f, 0x18, 0xe8, 0xb9, 0x24, 0x35, 0xbf, 0x6f, 0xf7, 0xdd, 0x29, 0x77, 0x20, 0x63, 0x04, 0x85, 0x62, 0x8d, 0x5a, 0xe7, 0x4e, 0xe7, 0xc3, 0x2e, 0x79, 0xb7,     //n
    0x42, 0x1d, 0xeb, 0xd6, 0x1b, 0x62, 0xea, 0xb6, 0x74, 0x64, 0x34, 0xeb, 0xc3, 0xcc, 0x31, 0x5e, 0x32, 0x22, 0x0b, 0x3b, 0xad, 0xd5, 0x0b, 0xdc, 0x4c, 0x4e, 0x6c, 0x14, 0x7f, 0xed, 0xd4, 0x3d,     //gx
    0x06, 0x80, 0x51, 0x2b, 0xcb, 0xb4, 0x2c, 0x07, 0xd4, 0x73, 0x49, 0xd2, 0x15, 0x3b, 0x70, 0xc4, 0xe5, 0xd7, 0xfd, 0xfc, 0xbf, 0xa3, 0x6e, 0xa1, 0xa8, 0x58, 0x41, 0xb9, 0xe4, 0x6e, 0x09, 0xa2,     //gy
    0x78, 0x79, 0x68, 0xb4, 0xfa, 0x32, 0xc3, 0xfd, 0x24, 0x17, 0x84, 0x2e, 0x73, 0xbb, 0xfe, 0xff, 0x2f, 0x3c, 0x84, 0x8b, 0x68, 0x31, 0xd7, 0xe0, 0xec, 0x65, 0x22, 0x8b, 0x39, 0x37, 0xe4, 0x98,    //a
    0x63, 0xe4, 0xc6, 0xd3, 0xb2, 0x3b, 0x0c, 0x84, 0x9c, 0xf8, 0x42, 0x41, 0x48, 0x4b, 0xfe, 0x48, 0xf6, 0x1d, 0x59, 0xa5, 0xb1, 0x6b, 0xa0, 0x6e, 0x6e, 0x12, 0xd1, 0xda, 0x27, 0xc5, 0x24, 0x9a
};  //b
const sd_ecc_curve_t sx_ecc_curve_p256_ip = {
    .params  = BLOCK_T_CONV(ecc_p256_params_ip, sizeof(ecc_p256_params_ip), HAL_EXT_MEM),
    .pk_flags = BA414EP_CMD_OPFLD(BA414EP_OPFLD_PRIME),
    .bytesize = 32,
};

uint8_t __attribute__((aligned(CACHE_LINE))) sm2_prv_key_ip[32] = "\x12\x8b\x2f\xa8\xbd\x43\x3c\x6c\x06\x8c\x8d\x80\x3d\xff\x79\x79\x2a\x51\x9a\x55\x17\x1b\x1b\x65\x0c\x23\x66\x1d\x15\x89\x72\x63"; //-- d
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_nonce_ip[32] = "\x6c\xb2\x8d\x99\x38\x5c\x17\x5c\x94\xf9\x4e\x93\x48\x17\x66\x3f\xc1\x76\xd9\x25\xdd\x72\xb7\x27\x26\x0d\xba\xae\x1f\xb2\xf9\x6f"; //-- k
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_hash_ip[32] = "\xb5\x24\xf5\x52\xcd\x82\xb8\xb0\x28\x47\x6e\x00\x5c\x37\x7f\xb1\x9a\x87\xe6\xfc\x68\x2d\x48\xbb\x5d\x42\xe3\xd9\xb9\xef\xfe\x76"; //-- h

uint8_t __attribute__((aligned(CACHE_LINE))) sm2_pub_key_ip[64] = "\x0a\xe4\xc7\x79\x8a\xa0\xf1\x19\x47\x1b\xee\x11\x82\x5b\xe4\x62\x02\xbb\x79\xe2\xa5\x84\x44\x95\xe9\x7c\x04\xff\x4d\xf2\x54\x8a" //-- Qx
                             "\x7c\x02\x40\xf8\x8f\x1c\xd4\xe1\x63\x52\xa7\x3c\x17\xb7\xf1\x6f\x07\x35\x3e\x53\xa1\x76\xd6\x84\xa9\xfe\x0c\x6b\xb7\x98\xe8\x57";  //-- Qy
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_sig_ip[64] = "\x75\xe9\xae\x8b\xec\x8d\xbd\x07\xe7\xf3\xdd\x59\x10\x57\xed\x5a\x0d\xeb\xf9\x0a\xdf\x93\x76\xde\x0e\xa8\x4f\xeb\xf9\xf3\x71\xa4"  //-- r
                         "\x4f\x00\xb1\x84\x79\x4a\xcc\xcd\x92\x2d\xd3\xa5\x7e\xb2\xdd\x00\x78\xc5\x5e\x15\x2a\x00\x03\xc4\xe1\xdc\xad\x38\x77\xf9\xeb\x4b"; //-- s

uint8_t __attribute__((aligned(CACHE_LINE))) sm2_prv_key_exc_ip[32] = "\x6f\xcb\xa2\xef\x9a\xe0\xab\x90\x2b\xc3\xbd\xe3\xff\x91\x5d\x44\xba\x4c\xc7\x8f\x88\xe2\xf8\xe7\xf8\x99\x6d\x3b\x8c\xce\xed\xee"; //-- d
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_nonce_exc_ip[32] = "\x83\xa2\xc9\xc8\xb9\x6e\x5a\xf7\x0b\xd4\x80\xb4\x72\x40\x9a\x9a\x32\x72\x57\xf1\xeb\xb7\x3f\x5b\x07\x33\x54\xb2\x48\x66\x85\x63"; //-- k
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_pub_key_exc_ip[64] = "\x24\x54\x93\xd4\x46\xc3\x8d\x8c\xc0\xf1\x18\x37\x46\x90\xe7\xdf\x63\x3a\x8a\x4b\xfb\x33\x29\xb5\xec\xe6\x04\xb2\xb4\xf3\x7f\x43" //-- Qx
                                 "\x53\xc0\x86\x9f\x4b\x9e\x17\x77\x3d\xe6\x8f\xec\x45\xe1\x49\x04\xe0\xde\xa4\x5b\xf6\xce\xcf\x99\x18\xc8\x5e\xa0\x47\xc6\x0a\x4c"; //-- Qy
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_pointb_exc_ip[64] = "\x17\x99\xb2\xa2\xc7\x78\x29\x53\x00\xd9\xa2\x32\x5c\x68\x61\x29\xb8\xf2\xb5\x33\x7b\x3d\xcf\x45\x14\xe8\xbb\xc1\x9d\x90\x0e\xe5" //-- RBx
                                "\x54\xc9\x28\x8c\x82\x73\x3e\xfd\xf7\x80\x8a\xe7\xf2\x7d\x0e\x73\x2f\x7c\x73\xa7\xd9\xac\x98\xb7\xd8\x74\x0a\x91\xd0\xdb\x3c\xf4"; //-- RBy
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_cofactor_exc_ip[32] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"; //-- CoFactor
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_pointa_exc_ip[64] = "\x6c\xb5\x63\x38\x16\xf4\xdd\x56\x0b\x1d\xec\x45\x83\x10\xcb\xcc\x68\x56\xc0\x95\x05\x32\x4a\x6d\x23\x15\x0c\x40\x8f\x16\x2b\xf0" //-- RAx
                                "\x0d\x6f\xcf\x62\xf1\x03\x6c\x0a\x1b\x6d\xac\xcf\x57\x39\x92\x23\xa6\x5f\x7d\x7b\xf2\xd9\x63\x7e\x5b\xbb\xeb\x85\x79\x61\xbf\x1a"; //-- RAy
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_two_w_exc_ip[32] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"; //-- Two_w
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_exc_key_ip[64] = "\x47\xc8\x26\x53\x4d\xc2\xf6\xf1\xfb\xf2\x87\x28\xdd\x65\x8f\x21\xe1\x74\xf4\x81\x79\xac\xef\x29\x00\xf8\xb7\xf5\x66\xe4\x09\x05" //-- Vx
                             "\x2a\xf8\x6e\xfe\x73\x2c\xf1\x2a\xd0\xe0\x9a\x1f\x25\x56\xcc\x65\x0d\x9c\xcc\xe3\xe2\x49\x86\x6b\xbb\x5c\x68\x46\xa4\xc4\xa2\x95"; //-- Vy
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_pointb_exc_ip_wrong[64] = "\x17\x99\xb2\xa2\xc7\x78\x29\x53\x00\xd9\xa2\x32\x5c\x68\x61\x29\xb8\xf2\xb5\x33\x7b\x3d\xcf\x45\x14\xe8\xbb\xc1\x9d\x90\x0e\xe6" //-- RBx
                                      "\x54\xc9\x28\x8c\x82\x73\x3e\xfd\xf7\x80\x8a\xe7\xf2\x7d\x0e\x73\x2f\x7c\x73\xa7\xd9\xac\x98\xb7\xd8\x74\x0a\x91\xd0\xdb\x3c\xf4";  //-- RBy
//IP owner's Pattern end

//sm2 gb pattern start
/*dB：3945208F 7B2144B1 3F36E38A C6D39F95 88939369 2860B51A 42FB81EF 4DF7C5B8*/
uint8_t __attribute__((aligned(CACHE_LINE))) sm2_gb_prv_key[32] = "\x39\x45\x20\x8F\x7B\x21\x44\xB1\x3F\x36\xE3\x8A\xC6\xD3\x9F\x95\x88\x93\x93\x69\x28\x60\xB5\x1A\x42\xFB\x81\xEF\x4D\xF7\xC5\xB8"; //-- d

/*xB：09F9DF31 1E5421A1 50DD7D16 1E4BC5C6 72179FAD 1833FC07 6BB08FF3 56F35020*/
/*yB：CCEA490C E26775A5 2DC6EA71 8CC1AA60 0AED05FB F35E084A 6632F607 2DA9AD13*/

uint8_t __attribute__((aligned(CACHE_LINE))) sm2_gb_pub_key[64] = "\x09\xF9\xDF\x31\x1E\x54\x21\xA1\x50\xDD\x7D\x16\x1E\x4B\xC5\xC6\x72\x17\x9F\xAD\x18\x33\xFC\x07\x6B\xB0\x8F\xF3\x56\xF3\x50\x20" //-- Qx
                             "\xCC\xEA\x49\x0C\xE2\x67\x75\xA5\x2D\xC6\xEA\x71\x8C\xC1\xAA\x60\x0A\xED\x05\xFB\xF3\x5E\x08\x4A\x66\x32\xF6\x07\x2D\xA9\xAD\x13";  //-- Qy

/*message digest:6D65737361676520646967657374*/
uint8_t sm2_gb_sig_msg[14] = "\x6D\x65\x73\x73\x61\x67\x65\x20\x64\x69\x67\x65\x73\x74"; //-- m
/*IDA GB/T1988: 31323334 35363738 31323334 35363738*/
uint8_t sm2_gb_id_test[16] = "\x31\x32\x33\x34\x35\x36\x37\x38\x31\x32\x33\x34\x35\x36\x37\x38";
/*
r：F5A03B06 48D2C463 0EEAC513 E1BB81A1 5944DA38 27D5B741 43AC7EAC EEE720B3
s：B1B6AA29 DF212FD8 763182BC 0D421CA1 BB9038FD 1F7F42D4 840B69C4 85BBC1AA*/

uint8_t __attribute__((aligned(CACHE_LINE))) sm2_gb_ver_msg[64] = "\xF5\xA0\x3B\x06\x48\xD2\xC4\x63\x0E\xEA\xC5\x13\xE1\xBB\x81\xA1\x59\x44\xDA\x38\x27\xD5\xB7\x41\x43\xAC\x7E\xAC\xEE\xE7\x20\xB3"  //-- r
                             "\xB1\xB6\xAA\x29\xDF\x21\x2F\xD8\x76\x31\x82\xBC\x0D\x42\x1C\xA1\xBB\x90\x38\xFD\x1F\x7F\x42\xD4\x84\x0B\x69\xC4\x85\xBB\xC1\xAA"; //-- s

uint8_t __attribute__((aligned(CACHE_LINE))) sm2_gb_m[46];

uint32_t sm2_sigature_gen_single(uint32_t vce_id, sm2_test_type_t type)
{
    uint32_t status;
    uint8_t __attribute__((aligned(CACHE_LINE))) sig[132 + 28];
    sd_ecc_curve_t* ecc_curve;
#if WITH_SIMULATION_PLATFORM
    uint8_t* sig_ref;
#else
    uint8_t* pub_key;
#endif
    uint8_t* prv_key;
    uint32_t size;
    void* crypto_handle;
    int curve_nid;

    TRACEF("sm2 sigature enter! \n");

    switch (type) {
        case SM2_P192:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p192;
#if WITH_SIMULATION_PLATFORM
            sig_ref = sm2_ref_p192_sha256;
#else
            pub_key = (uint8_t*)pub_key_p192;
#endif
            prv_key = (uint8_t*)prv_key_p192;
            curve_nid = NID_X9_62_prime192v1;
            break;

        case SM2_P256:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p256;
#if WITH_SIMULATION_PLATFORM
            sig_ref = sm2_ref_p256_sha256;
#else
            pub_key = (uint8_t*)pub_key_p256;
#endif
            prv_key = (uint8_t*)prv_key_p256;
            curve_nid = NID_X9_62_prime256v1;
            break;

        case SM2_P384:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p384;
#if WITH_SIMULATION_PLATFORM
            sig_ref = sm2_ref_p384_sha256;
#else
            pub_key = (uint8_t*)pub_key_p384;
#endif
            prv_key = (uint8_t*)prv_key_p384;
            curve_nid = NID_secp384r1;
            break;

        case SM2_P521:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p521;
#if WITH_SIMULATION_PLATFORM
            sig_ref = sm2_ref_p521_sha256;
#else
            pub_key = (uint8_t*)pub_key_p521;
#endif
            prv_key = (uint8_t*)prv_key_p521;
            curve_nid = NID_secp521r1;
            break;

        case SM2_E521:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_e521;
#if WITH_SIMULATION_PLATFORM
            sig_ref = sm2_ref_e521_sha256;
#else
            pub_key = (uint8_t*)pub_key_e521;
#endif
            prv_key = (uint8_t*)prv_key_e521;
            curve_nid = NID_sece521r1;
            break;

        default:
            return -1;
    }

    size = ecc_curve->bytesize;

    ec_key_t eckey_test;

    eckey_test.priv_key = block_t_convert(prv_key, size, HAL_EXT_MEM);
    eckey_test.pub_key = block_t_convert(NULL, 0, HAL_EXT_MEM);

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    status = hal_sm2_sign(crypto_handle, curve_nid, ecdsa_msg, 128, sig,
                      2 * size, &eckey_test, type == SM2_P192 ? SD_ALG_SHA1 : SD_ALG_SM3);
    if (status) {
        hal_crypto_delete_handle(crypto_handle);
        LTRACEF("sigature gen result: %d\n", status);
        return status;
    }

#if WITH_SIMULATION_PLATFORM
    hal_crypto_delete_handle(crypto_handle);
    status = memcmp(sig, sig_ref, 2 * size);
    if (status) {
        ce_printf_binary("signature gen result", sig, 2 * size);
        sm2_test_fail++;
    }
    else {
        LTRACEF("sm2 sigature pass! \n");
    }
#else
    eckey_test.priv_key = block_t_convert(NULL, 0, HAL_EXT_MEM);
    eckey_test.pub_key = block_t_convert(pub_key, 2 * size, HAL_EXT_MEM);

    status = hal_sm2_verify(crypto_handle, curve_nid, ecdsa_msg, 128,
                         sig, 2 * size, &eckey_test, type == SM2_P192 ? SD_ALG_SHA1 : SD_ALG_SM3);
    hal_crypto_delete_handle(crypto_handle);
    if (status) {
        LTRACEF("verify after sigature gen result: %d\n", status);
    }
#endif

    return status;
}

uint32_t sm2_sigature_ver_single(uint32_t vce_id, sm2_test_type_t type)
{
    uint32_t status;
    sd_ecc_curve_t* ecc_curve;
    uint8_t* sig_ref;
    uint8_t* pub_key;
    uint32_t size;
    int curve_nid;
    void* crypto_handle;

    TRACEF("sm2 verify enter! \n");

    switch (type) {
        case SM2_P192:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p192;
            sig_ref = sm2_ref_p192_sha256;
            pub_key = (uint8_t*)pub_key_p192;
            curve_nid = NID_X9_62_prime192v1;
            break;

        case SM2_P256:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p256;
            sig_ref = sm2_ref_p256_sha256;
            pub_key = (uint8_t*)pub_key_p256;
            curve_nid = NID_X9_62_prime256v1;
            break;

        case SM2_P384:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p384;
            sig_ref = sm2_ref_p384_sha256;
            pub_key = (uint8_t*)pub_key_p384;
            curve_nid = NID_secp384r1;
            break;

        case SM2_P521:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_p521;
            sig_ref = sm2_ref_p521_sha256;
            pub_key = (uint8_t*)pub_key_p521;
            curve_nid = NID_secp521r1;
            break;

        case SM2_E521:
            ecc_curve = (sd_ecc_curve_t*)&sx_ecc_curve_e521;
            sig_ref = sm2_ref_e521_sha256;
            pub_key = (uint8_t*)pub_key_e521;
            curve_nid = NID_sece521r1;
            break;

        default:
            return -1;
    }

    size = ecc_curve->bytesize;

    ec_key_t eckey_test;

    eckey_test.priv_key = block_t_convert(NULL, 0, HAL_EXT_MEM);
    eckey_test.pub_key = block_t_convert(pub_key, 2 * size, HAL_EXT_MEM);

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    status = hal_sm2_verify(crypto_handle, curve_nid, ecdsa_msg, 128,
                         sig_ref, 2 * size, &eckey_test, type == SM2_P192 ? SD_ALG_SHA1 : SD_ALG_SM3);

    hal_crypto_delete_handle(crypto_handle);

    if (status) {
        LTRACEF("sm2 ver fail size: %d, result: %d\n", size, status);
        sm2_test_fail++;
    }
    else {
        TRACEF("sm2 verify pass! \n");
    }

    return status;
}

uint32_t sm2_signature_gen(uint32_t vce_id)
{
    uint32_t status;
    uint32_t ret;

    for (int i = SM2_P192; i <= SM2_E521; i++) {
        status = sm2_sigature_gen_single(vce_id, i);

        if (status) {
            break;
        }
    }

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    LTRACEF("sm2 signature generation test finish, last staus: %d\n", status);
    return ret;
}

uint32_t  sm2_signature_ver(uint32_t vce_id)
{
    uint32_t status;
    uint32_t ret;

    for (int i = SM2_P192; i <= SM2_E521; i++) {
        status = sm2_sigature_ver_single(vce_id, i);

        if (status) {
            break;
        }
    }

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    LTRACEF("sm2 signature verification test finish, last staus: %d\n", status);
    return ret;
}

void ip_owner_pattern_test(uint32_t vce_id)
{
    uint32_t status;
    uint8_t __attribute__((aligned(CACHE_LINE))) sig[64];
    void* crypto_handle;
    ec_key_t eckey_test;

    eckey_test.priv_key = block_t_convert(sm2_prv_key_ip, 32, HAL_EXT_MEM);
    eckey_test.pub_key = block_t_convert(sm2_pub_key_ip, 64, HAL_EXT_MEM);

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    status = hal_sm2_sign(crypto_handle, NID_sxiptestp256, ecdsa_msg, 128, sig,
                      64, &eckey_test, SD_ALG_SM3);

    if (status) {
        LTRACEF("sig gen for ip owner status: %d\n", status);
    }

#if WITH_SIMULATION_PLATFORM
    status = memcmp(sig, sm2_sig_ip, 64);
    if (status) {
        ce_printf_binary("sig gen for ip owner", sig, 64);
        hal_crypto_delete_handle(crypto_handle);
        sm2_test_fail++;
        return;
    }
    else {
        LTRACEF("sig gen for ip owner pass !\n");
    }


    status =  hal_sm2_verify(crypto_handle, NID_sxiptestp256, ecdsa_msg, 128,
                         sm2_sig_ip, 64, &eckey_test, SD_ALG_SM3);
#else
    status =  hal_sm2_verify(crypto_handle, NID_sxiptestp256, ecdsa_msg, 128,
                         sig, 64, &eckey_test, SD_ALG_SM3);
#endif

    if (status) {
        LTRACEF("sig verify status: %d\n", status);
        sm2_test_fail++;
        return;
    }
    else {
        LTRACEF("SM2_verify verify pass\n");
    }

    eckey_test.priv_key = block_t_convert(sm2_prv_key_exc_ip, 32, HAL_EXT_MEM);
    eckey_test.pub_key = block_t_convert(sm2_pub_key_exc_ip, 64, HAL_EXT_MEM);

    status = hal_sm2_key_exchange(crypto_handle, NID_sxiptestp256, &eckey_test, sm2_pointb_exc_ip, 64,
                              sig, 64);

    hal_crypto_delete_handle(crypto_handle);

    if (status) {
        LTRACEF("key exchange status: %d\n", status);
        return;
    }

#if WITH_SIMULATION_PLATFORM
    status = memcmp(sig, sm2_exc_key_ip, 64);

    if (status) {
        ce_printf_binary("key exchange for ip owner", sig, 64);
        sm2_test_fail++;
        return;
    }
    else {
        LTRACEF("key exchange ip pass ！\n");
    }
#endif

    LTRACEF("P256 of IP owner pattern verification pass!\n");
}

void sm2_gb_pattern_test(uint32_t vce_id)
{
    uint32_t status;
    uint8_t __attribute__((aligned(CACHE_LINE))) sig[64];
    void* crypto_handle;
    ec_key_t eckey_test;

    eckey_test.priv_key = block_t_convert(sm2_gb_prv_key, 32, HAL_EXT_MEM);
    eckey_test.pub_key = block_t_convert(sm2_gb_pub_key, 64, HAL_EXT_MEM);

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    //get z msg ZA= H256(ENTLA||IDA||a||b||xG||yG||xA||yA)
    hal_sm2_compute_id_digest(crypto_handle, NID_X9_62_prime_field, sm2_gb_id_test, 16, sm2_gb_m, 32, &eckey_test, SD_ALG_SM3);

    memcpy(&sm2_gb_m[32], sm2_gb_sig_msg, 14);

    status = hal_sm2_sign(crypto_handle, NID_X9_62_prime_field, sm2_gb_m, 46, sig,
                      64, &eckey_test, SD_ALG_SM3);

    if (status) {
        LTRACEF("sig gen for ip owner status: %d\n", status);
    }

    status = memcmp(sig, sm2_gb_ver_msg, 64);

    if (status) {
        ce_printf_binary("sig gen for ip owner", sig, 64);
        hal_crypto_delete_handle(crypto_handle);
        sm2_test_fail++;
        return;
    }
    else {
        LTRACEF("sig gen for ip owner pass !\n");
    }

    status =  hal_sm2_verify(crypto_handle, NID_X9_62_prime_field, sm2_gb_m, 46,
                         sm2_gb_ver_msg, 64, &eckey_test, SD_ALG_SM3);

    if (status) {
        LTRACEF("sig verify status: %d\n", status);
        sm2_test_fail++;
        hal_crypto_delete_handle(crypto_handle);
        return;
    }
    else {
        LTRACEF("SM2_verify verify pass\n");
    }
/*
    eckey_test.priv_key = block_t_convert(sm2_gb_prv_key, 32, HAL_EXT_MEM);
    eckey_test.pub_key = block_t_convert(sm2_gb_pub_key, 64, HAL_EXT_MEM);

    status = hal_sm2_key_exchange(crypto_handle, NID_X9_62_prime_field, &eckey_test, sm2_pointb_exc_ip, 64,
                              sig, 64);

    hal_crypto_delete_handle(crypto_handle);

    if (status) {
        LTRACEF("key exchange status: %d\n", status);
        return;
    }

    status = memcmp(sig, sm2_exc_key_ip, 64);

    if (status) {
        ce_printf_binary("key exchange for ip owner", sig, 64);
        sm2_test_fail++;
        return;
    }
    else {
        LTRACEF("key exchange ip pass ！\n");
    }
*/
    LTRACEF("P256 of IP owner pattern verification pass!\n");
}

uint32_t sm2_test_slt(void* arg)
{
    uint32_t ret = 0;
    uint32_t result_value;
    ce_test_t* ce_test_s;

    if(arg != NULL){
        ce_test_s = (ce_test_t*)arg;
    }else{
        ret |= (0x1 << CE_TEST_RESULT_OFFSET_UNKOWN_TEST);
        return ret;
    }

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_SM2_SIGN_GEN_TEST;
    result_value = sm2_signature_gen(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_SM2_SIGN_GEN_TEST);

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_SM2_SIGN_VERY_TEST;
    result_value = sm2_signature_ver(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_SM2_SIGN_VERY_TEST);
    //ip_owner_pattern_test(0);

    return ret;
}

uint32_t sm2_test_uart(void)
{
    uint32_t ret = 0;

    ret = sm2_signature_gen(0);
    ret |= sm2_signature_ver(0);

    return ret;
}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("sm2_test", "sm2 generate public key", (console_cmd)&sm2_test_uart)
STATIC_COMMAND("sm2_gb_test", "sm2 gb generate public key", (console_cmd)&sm2_gb_pattern_test)
STATIC_COMMAND("sm2_test_p", "IP Owner pattern test", (console_cmd)&ip_owner_pattern_test)

STATIC_COMMAND_END(sm2_test);

#endif

APP_START(sm2_test)
.flags = 0
         APP_END
