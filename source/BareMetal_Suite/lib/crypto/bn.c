/********************************************************
 *      Copyright(c) 2020   Semidrive                   *
 *      All rights reserved.                            *
 ********************************************************/

#include <common_hdr.h>

bool bn_is_zero(uint8_t *bn, uint32_t sz)
{
    bool non_zero = false;
    for (uint32_t i = 0; i < sz; i++, bn++) {
        non_zero |= (0u != *bn);
    }
    return !non_zero;
}

int32_t bn_cmp(uint8_t *A, uint8_t *B, uint32_t sz)
{
    bool done = false;
    int32_t ret = 0;

    for (uint32_t i = 0; i < sz; i++, A++, B++) {
        if ((*A > *B) && !done) {
            done = true;
            ret = 1;
        } else if ((*A < *B) && !done) {
            done = true;
            ret = -1;
        }
    }
    return ret;
}

#if defined(BN_TEST)
void bn_test(void)
{
    uint8_t A[] = {0x1, 0x2, 0x3};
    uint8_t B[] = {0x2, 0x3, 0x4};

    int32_t flg = bn_cmp(A, B, sizeof(A));
    DBG("A %s B\n", 0 == flg ? "equal" :
                    1 == flg ? "bigger" :
                    "smaller");
    flg = bn_cmp(B, A, sizeof(A));
    DBG("B %s A\n", 0 == flg ? "equal" :
                    1 == flg ? "bigger" :
                    "smaller");

    memcpy(A, B, sizeof(A));
    flg = bn_cmp(A, B, sizeof(A));
    DBG("A %s B\n", 0 == flg ? "equal" :
                    1 == flg ? "bigger" :
                    "smaller");
    if(!bn_is_zero(A, sizeof(A))) {
        DBG("A is not zero\n");
    }

    memset(A, 0, sizeof(A));
    if(bn_is_zero(A, sizeof(A))) {
        DBG("A is zero\n");
    }
}
#endif
