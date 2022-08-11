/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <err.h>
#include <string.h>

#include <app.h>
#include <lib/console.h>
#include <platform.h>
#include <reg.h>
#include <trace.h>
#include <debug.h>
#include <lib/reg.h>
#include "ce_test.h"
#include "cipher_test.h"
#include "dsa_test.h"
#include "ecc_keygen_test.h"
#include "ecdsa_test.h"
#include "hash_test.h"
#include "rsa_test.h"
#include "sm2_test.h"

#define LOCAL_TRACE 0 //close local trace 1->0

void enable_vce_key_interface(void)
{
#if CE_IN_SAFETY_DOMAIN
    addr_t src_saf_base = 0xfc200000;
    writel(0xffff, _ioaddr((src_saf_base + (0x210 << 10))));
#else
    addr_t src_sec_base = 0xf8280000;

    for (int i = 0; i < 8; i++) {
        writel(0xffff, _ioaddr(src_sec_base + i * 0x1000));
    }
#endif
}

void ce_printf_binary(const char* info, const void* content, uint32_t content_len)
{
    dprintf(CRITICAL, "%s: \n", info);
    uint32_t i = 0;

    for (; i < content_len; i++) {
        dprintf(CRITICAL, "0x%02x ", *((uint8_t*)(content) + i));

        if (0 != i && 0 == (i + 1) % 16) {
            dprintf(CRITICAL, "\n");
        }
    }

    if (!(i % 16)) {
        dprintf(CRITICAL, "\n");
    }
}

static enum handler_return ce_test_timer_cb(struct timer* t, lk_time_t now, void* arg)
{
    ce_test_t* temp = (ce_test_t*)arg;

    if(temp->test_timer_init == 1){
        timer_cancel(&(temp->test_timer));
        temp->test_timer_init = 0;
    }

    switch (temp->current_index)
    {
        case CE_TEST_ITEM_INDEX_CIPHER_PATH_TEST:
            strcpy(temp->result_string, "ce cipher path test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_CIPHER_CONTEXT_TEST:
            strcpy(temp->result_string, "ce cipher context test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_DSA_KEY_GEN_TEST:
            strcpy(temp->result_string, "ce dsa keygen test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_DSA_SIGN_GEN_TEST:
            strcpy(temp->result_string, "ce dsa signgen test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_DSA_SIGN_VERY_TEST:
            strcpy(temp->result_string, "ce dsa signvery test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_ECC_KEY_GEN_TEST:
            strcpy(temp->result_string, "ce ecc keygen test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_ECDSA_SIG_GEN_TEST:
            strcpy(temp->result_string, "ce ecdsa signgen test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_ECDSA_SIGN_VERY_TEST:
            strcpy(temp->result_string, "ce ecdsa signvery test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_HASH_PATH_TEST:
            strcpy(temp->result_string, "ce hash path test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_HASH_MULT_TEST:
            strcpy(temp->result_string, "ce hash mult test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_RSA_ENC_TEST:
            strcpy(temp->result_string, "ce rsa enc test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_RSA_DEC_TEST:
            strcpy(temp->result_string, "ce rsa dec test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_RSA_SIGN_GEN_TEST:
            strcpy(temp->result_string, "ce rsa signgen test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_RSA_SIGN_VERY_TEST:
            strcpy(temp->result_string, "ce rsa signvery test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_RSA_KEY_GEN_TEST:
            strcpy(temp->result_string, "ce rsa kengen test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_SM2_SIGN_GEN_TEST:
            strcpy(temp->result_string, "ce sm2 signgen test fail, timeout");
            break;
        case CE_TEST_ITEM_INDEX_SM2_SIGN_VERY_TEST:
            strcpy(temp->result_string, "ce sm2 signvery test fail, timeout");
            break;
        default:
            strcpy(temp->result_string, "ce unkown test index fail, timeout");
            break;
    }

    return 0;
}

ce_test_t ce_test_context;

int slt_module_test_ce_test(uint times, uint timeout, char* result_string){
    int ret = 0;
    ce_test_t* ce_test_s;

    LTRACEF(" entry, times= %d, timeout =%d\n", times, timeout);
    ce_test_s = &ce_test_context;

    timer_initialize(&(ce_test_s->test_timer));
    timer_set_oneshot(&(ce_test_s->test_timer), CE_TEST_TIMEOUT_FOR_SLT_DEFAULT_VALUE, ce_test_timer_cb, (void*)ce_test_s);
    ce_test_s->test_timer_init = 1;
    ce_test_s->result_string = result_string;

    ret |= cipher_test_slt(ce_test_s);//include cmac test
    ret |= dsa_test_slt(ce_test_s);
    ret |= ecc_keygen_test_slt(ce_test_s);
    ret |= ecdsa_test_slt(ce_test_s);
    ret |= hash_test_slt(ce_test_s);//include hmac test
    ret |= rsa_test_slt(ce_test_s);
    ret |= sm2_test_slt(ce_test_s);

    ce_test_s->test_timer_init = 0;
    timer_cancel(&(ce_test_s->test_timer));

    if (result_string != NULL) {
        if(ret  != 0){
            strcpy(result_string, "ce test fail, detail cause see error code");
        }else{
            strcpy(result_string, "ce test pass");
        }
    }

    LTRACEF(" end, ret= %d\n", ret);
    return ret;
}

uint32_t ce_test(void)
{
    uint32_t ret = 0;

    ret |= cipher_test_uart();//include cmac test
    ret |= dsa_test_uart();
    ret |= ecc_keygen_test_uart();
    ret |= ecdsa_test_uart();
    ret |= hash_test_uart();//include hmac test
    ret |= rsa_test_uart();
    ret |= sm2_test_uart();

    return ret;
}

int ce_fw_writel(uint32_t val, addr_t reg)
{
    int ret = ce_writel_(val, reg);
    LTRACEF("ce_fw_writel(0x%x, 0x%08x), r(0x%08x)\n", (uint32_t)reg, val, ce_readl_(reg));
    return ret;
}

int ce_fw_readl(addr_t reg)
{
	LTRACEF("ce_fw_readl(0x%08x)\n", ce_readl_(reg));
	return ce_readl_(reg);
}

static void ce_display_mem(int argc, const cmd_args *argv)
{
    addr_t address;

	LTRACEF("ce_display_mem enter\n");

    if (strcmp(argv[0].str, "rc") == 0) {
        address = argv[1].u;
        ce_fw_readl(address);
    }
}

static void ce_modify_mem(int argc, const cmd_args *argv)
{
    addr_t address;
    uint32_t value;

    LTRACEF("ce_modify_mem enter\n");

    if (strcmp(argv[0].str, "wc") == 0) {
        address = argv[1].u;
        value = argv[2].u;
        ce_fw_writel(value, _vaddr(address));
    }
}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("ce_test", "crypto engine all alg test", (console_cmd)&ce_test)
STATIC_COMMAND("rc", "check read word ", (console_cmd)&ce_display_mem)
STATIC_COMMAND("wc", "check write word", (console_cmd)&ce_modify_mem)
STATIC_COMMAND_END(ce_test);

#endif

SLT_MODULE_TEST_HOOK(ce_test,slt_module_test_ce_test);

APP_START(virt_test)
.flags = 0
APP_END
