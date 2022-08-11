
/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#if defined(CFG_CRYPTO_MODULE)

#include <common_hdr.h>
#include <atb_crypto.h>

#if defined(CFG_CRYPTO_HW_ENG)
extern crypto_eng_t eng_hw_ce;
#endif
#if defined(CFG_CRYPTO_SW_ENG)
extern crypto_eng_t crypto_sw_eng;
#endif

crypto_eng_t *g_crypto_eng_list[] = {
#if defined(CFG_CRYPTO_HW_ENG)
    &eng_hw_ce,
#endif
#if defined(CFG_CRYPTO_SW_ENG)
    &crypto_sw_eng,
#endif
    NULL
};
#endif  /* CFG_CRYPTO_MODULE */
