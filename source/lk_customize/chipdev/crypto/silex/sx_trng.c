/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <string.h>

#include <sx_trng.h>
#include <sx_errors.h>
#include <sx_math.h>

#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#define RUN_WITH_NON_INNER_STATUS 1 // trng init run with non inner status

static bool rng_startup_failed = false;
static bool rng_needs_startup_chk = false;

void trng_soft_reset(void)
{
    uint32_t read_value, value;

    read_value = readl(_ioaddr(REG_CE_TRNG_CONTROL));
    value = reg_value(1, read_value, TRNG_SOFTRESET_SHIFT, TRNG_SOFTRESET_MASK);
    LTRACEF("trng_soft_reset val = 1 reg=%x, value=%x.\n", REG_CE_TRNG_CONTROL, value);
    writel(value, _ioaddr(REG_CE_TRNG_CONTROL));

    value = reg_value(0, read_value, TRNG_SOFTRESET_SHIFT, TRNG_SOFTRESET_MASK);
    LTRACEF("trng_soft_reset val = 0 reg=%x, value=%x.\n", REG_CE_TRNG_CONTROL, value);
    writel(value, _ioaddr(REG_CE_TRNG_CONTROL));

    trng_set_startup_chk_flag();
}

uint32_t trng_status_glb(uint32_t bit_mask)
{
    LTRACEF("trng_status_glb REG_CE_TRNG_STATUS value=0x%x.\n", readl(_ioaddr(REG_CE_TRNG_STATUS)));
    return readl(_ioaddr(REG_CE_TRNG_STATUS)) & bit_mask;
}

/**
* @brief Verify the conditioning function of the BA431 against test patterns.
* @return CRYPTOLIB_SUCCESS if successful CRYPTOLIB_CRYPTO_ERR otherwise
*/
static uint32_t trng_conditioning_test(void)
{
    const uint32_t test_data[16] = {0xE1BCC06B, 0x9199452A, 0x1A7434E1, 0x25199E7F,
                                    0x578A2DAE, 0x9CAC031E, 0xAC6FB79E, 0x518EAF45,
                                    0x461CC830, 0x11E45CA3, 0x19C1FBE5, 0xEF520A1A,
                                    0x45249FF6, 0x179B4FDF, 0x7B412BAD, 0x10376CE6
                                   };
    const uint32_t known_answer[4] = {0xA1CAF13F, 0x09AC1F68, 0x30CA0E12, 0xA7E18675};
    const uint32_t test_key[4] = {0x16157E2B, 0xA6D2AE28, 0x8815F7AB, 0x3C4FCF09};

    uint32_t i;
    uint32_t error = 0;
    uint32_t read_value, value;

    /*Soft reset*/
    trng_soft_reset();

    /*Enable test mode*/
    read_value = readl(_ioaddr(REG_CE_TRNG_CONTROL));
    value = reg_value(4, read_value, TRNG_NB128BITBLOCKS_SHIFT, TRNG_NB128BITBLOCKS_MASK);
    value = reg_value(1, value, TRNG_AIS31BYPASS_SHIFT, TRNG_AIS31BYPASS_MASK);
    value = reg_value(1, value, TRNG_HEALTHTESTBYPASS_SHIFT, TRNG_HEALTHTESTBYPASS_MASK);
    value = reg_value(1, value, TRNG_TESTEN_SHIFT, TRNG_TESTEN_MASK);
    writel(value, _ioaddr(REG_CE_TRNG_CONTROL));

    /*Write key*/
    writel(test_key[0], _ioaddr(REG_CE_TRNG_KEY0REG));
    writel(test_key[1], _ioaddr(REG_CE_TRNG_KEY1REG));
    writel(test_key[2], _ioaddr(REG_CE_TRNG_KEY2REG));
    writel(test_key[3], _ioaddr(REG_CE_TRNG_KEY3REG));

    /*Write test data input*/
    for (i = 0; i < sizeof(test_data) / 4; i++) {
        while (trng_status_glb(TRNG_TESTDATABUSY_MASK));

        writel(test_data[i], _ioaddr(REG_CE_TRNG_TESTDATA));
    }

    LTRACEF("write test data finish.\n");

    /*Wait for conditioning test to complete --> wait for return data to appear in FIFO*/
    while (readl(_ioaddr(REG_CE_TRNG_FIFOLEVEL)) < 4);

    /*Clear control register*/
    writel(0, _ioaddr(REG_CE_TRNG_CONTROL));

    /*Compare results to known answer*/
    for (i = 0; i < sizeof(known_answer) / 4; i++) {
        error |= readl(_ioaddr(REG_CE_TRNG_CONTROL + 0x80 + i * 0x4)) ^ known_answer[i];
    }

    if (error) {
        LTRACEF("finish error: %d.\n", error);
        return CRYPTOLIB_CRYPTO_ERR;
    }

    return CRYPTOLIB_SUCCESS;
}

/**
* @brief Poll for the end of the BA431 startup tests and check result.
* @return CRYPTOLIB_SUCCESS if successful CRYPTOLIB_CRYPTO_ERR otherwise
*/
static uint32_t trng_wait_startup(void)
{
    control_fsm_state_t fsm_state;

    /*Wait until startup is finished*/
    do {
        fsm_state = trng_status_glb(TRNG_STATE_MASK);
    }
    while ((fsm_state == FSM_STATE_RESET) || (fsm_state == FSM_STATE_STARTUP));

    /*Check startup test result*/
    if (trng_status_glb(TRNG_STARTUPFAIL_MASK)) {
        return CRYPTOLIB_CRYPTO_ERR;
    }

    return CRYPTOLIB_SUCCESS;
}

int trng_wait_ready(uint32_t vce_id)
{
    int i = 0;
    uint32_t fsm_state;

    do {
        LTRACEF("readl(REG_TRNG_STATUS_CE_(vce_id)) value=0x%x.\n", readl(_ioaddr(REG_TRNG_STATUS_CE_(vce_id))));
        fsm_state = readl(_ioaddr(REG_TRNG_STATUS_CE_(vce_id))) & 0x1;
        i++;

        if ((!fsm_state) && (0 == i % 200)) {
            LTRACEF("check ready times: %d\n", i);
            return -1;
        }
    }
    while (!fsm_state);

    return 0;
}

static int total_time = 0;
uint32_t trng_get_rand(uint32_t vce_id, uint8_t* dst, uint32_t size)
{
    uint32_t rng_value;

    LTRACEF("trng_get_rand vce_id =%d, dst=%p, size=%d.\n", vce_id, dst, size);

    if (size % 4) {
        return CRYPTOLIB_INVALID_PARAM;
    }

#if AIS31_ENABLED

    if (trng_status_glb(BA431_STAT_MASK_PREALM_INT)) {
        CRYPTOLIB_PRINTF("Preliminary noise alarm detected.\n");
        trng_soft_reset();
    }

#endif

    LTRACEF("trng_get_rand rng_needs_startup_chk=%d.\n", rng_needs_startup_chk);

    if (rng_needs_startup_chk) {
        trng_wait_startup();
        rng_needs_startup_chk = false;
    }

    int trng_ready = trng_wait_ready(vce_id);

    if (0 != trng_ready) {
        trng_soft_reset();
        writel(0x1, _ioaddr(REG_CE_TRNG_CTRL));
        thread_sleep(1);
    }

    for (uint32_t i = 0; i < size;) {
        if (i % 8 == 0) {
            trng_ready = trng_wait_ready(vce_id);

            if (0 != trng_ready) {
                LTRACEF("%s, %d\n", __func__, __LINE__);
                trng_soft_reset();
                writel(0x1, _ioaddr(REG_CE_TRNG_CTRL));
                thread_sleep(1);
            }

            LTRACEF("reg%x value = %x.\n", REG_CE_TRNG_CONTROL, readl(_ioaddr(REG_CE_TRNG_CONTROL)));
            LTRACEF("reg(%x) value = %x.\n", REG_CE_TRNG_STATUS, readl(_ioaddr(REG_CE_TRNG_STATUS)));

            if (readl(_ioaddr(REG_CE_TRNG_STATUS)) & 0x100) {
                writel(readl(_ioaddr(REG_CE_TRNG_STATUS)) & 0xFFFFFEFF, _ioaddr(REG_CE_TRNG_STATUS));
            }

            LTRACEF("after readl(%x) value = %x.\n", REG_CE_TRNG_STATUS, readl(_ioaddr(REG_CE_TRNG_STATUS)));
            writel(0x1, _ioaddr(REG_CE_TRNG_CTRL));

            if (!(readl(_ioaddr(REG_TRNG_NUM0_CE_(vce_id) - 4)) & 0x1)) {
                total_time++;
                printf("trng status:0x%x, total time:%d\n", readl(_ioaddr(REG_TRNG_NUM0_CE_(vce_id) - 4)), total_time);
            }
        }

        rng_value = readl(_ioaddr(REG_TRNG_NUM0_CE_(vce_id) + (i % 8)));
        LTRACEF("trng_get_rand rng_value=%x.\n", rng_value);
        memcpy(dst + i, (void*)(&rng_value), 4);
        i = i + 4;
    }

    return CRYPTOLIB_SUCCESS;
}

uint32_t trng_get_rand_blk(uint32_t vce_id, block_t dest)
{
    return trng_get_rand(vce_id, dest.addr, dest.len);
}

uint32_t trng_init(uint32_t cond_test_en)
{
    uint32_t status = 0;
    uint32_t key[4];
    uint32_t value;

    if (cond_test_en) {
        /*Conditioning function test*/
        status = trng_conditioning_test();

        if (status) {
            rng_startup_failed = true;
            return CRYPTOLIB_CRYPTO_ERR;
        }
    }

    /*Soft reset*/
    trng_soft_reset();

    if (RNG_OFF_TIMER_VAL < 0) {
        value = readl(_ioaddr(REG_CE_TRNG_CONTROL));
        value = reg_value(1, value, TRNG_FORCERUN_SHIFT, TRNG_FORCERUN_MASK);
        writel(value, _ioaddr(REG_CE_TRNG_CONTROL));
    }
    else {
        writel(RNG_OFF_TIMER_VAL, _ioaddr(REG_CE_TRNG_SWOFFTMRVAL));
    }

    writel(RNG_CLKDIV, _ioaddr(REG_CE_TRNG_CLKDIV));
    writel(RNG_INIT_WAIT_VAL, _ioaddr(REG_CE_TRNG_INITWAITVAL));

    /*Enable NDRNG*/
    value = readl(_ioaddr(REG_CE_TRNG_CONTROL));
    value = reg_value(RNG_NB_128BIT_BLOCKS, value, TRNG_NB128BITBLOCKS_SHIFT, TRNG_NB128BITBLOCKS_MASK);
    value = reg_value(1, value, TRNG_ENABLE_SHIFT, TRNG_ENABLE_MASK);
    writel(value, _ioaddr(REG_CE_TRNG_CONTROL));

    /*Check startup tests*/
    status = trng_wait_startup();

    if (status) {
        LTRACEF("wait_startup status: 0x%x\n", status);
        rng_startup_failed = true;
        return CRYPTOLIB_CRYPTO_ERR;
    }

    /*Program random key for the conditioning function*/
    key[0] = readl(_ioaddr(REG_CE_TRNG_CONTROL + 0x80));
    key[1] = readl(_ioaddr(REG_CE_TRNG_CONTROL + 0x84));
    key[2] = readl(_ioaddr(REG_CE_TRNG_CONTROL + 0x88));
    key[3] = readl(_ioaddr(REG_CE_TRNG_CONTROL + 0x8c));
    writel(key[0], _ioaddr(REG_CE_TRNG_KEY0REG));
    writel(key[1], _ioaddr(REG_CE_TRNG_KEY1REG));
    writel(key[2], _ioaddr(REG_CE_TRNG_KEY2REG));
    writel(key[3], _ioaddr(REG_CE_TRNG_KEY3REG));

    /*Soft reset to flush FIFO*/
    trng_soft_reset();

    /*Enable interrupts for health tests (repetition and adaptive proportion tests) & AIS31 test failures */
    //ba431_enable_health_test_irq();
    value = readl(_ioaddr(REG_CE_TRNG_CONTROL));
    value = reg_value(1, value, TRNG_ENABLE_SHIFT, TRNG_ENABLE_MASK);
    value = reg_value(0, value, TRNG_LFSREN_SHIFT, TRNG_LFSREN_MASK);
    value = reg_value(0, value, TRNG_TESTEN_SHIFT, TRNG_TESTEN_MASK);
    value = reg_value(0, value, TRNG_CONDBYPASS_SHIFT, TRNG_CONDBYPASS_MASK);
    value = reg_value(0, value, TRNG_INTENREP_SHIFT, TRNG_INTENREP_MASK);
    value = reg_value(1, value, TRNG_INTENPROP_SHIFT, TRNG_INTENPROP_MASK);
    value = reg_value(1, value, TRNG_INTENFULL_SHIFT, TRNG_INTENFULL_MASK);
    value = reg_value(1, value, TRNG_INTENPRE_SHIFT, TRNG_INTENPRE_MASK);
    value = reg_value(1, value, TRNG_INTENALM_SHIFT, TRNG_INTENALM_MASK);
    value = reg_value(0, value, TRNG_FORCERUN_SHIFT, TRNG_FORCERUN_MASK);
    value = reg_value(0, value, TRNG_HEALTHTESTBYPASS_SHIFT, TRNG_HEALTHTESTBYPASS_MASK);
    value = reg_value(0, value, TRNG_AIS31BYPASS_SHIFT, TRNG_AIS31BYPASS_MASK);
    value = reg_value(0, value, TRNG_HEALTHTESTSEL_SHIFT, TRNG_HEALTHTESTSEL_MASK);
    value = reg_value(0, value, TRNG_AIS31TESTSEL_SHIFT, TRNG_AIS31TESTSEL_MASK);
    value = reg_value(0x4, value, TRNG_NB128BITBLOCKS_SHIFT, TRNG_NB128BITBLOCKS_MASK);
    value = reg_value(1, value, TRNG_FIFOWRITESTARTUP_SHIFT, TRNG_FIFOWRITESTARTUP_MASK);
    LTRACEF("writel reg =%x, value = %x.\n", REG_CE_TRNG_CONTROL, value);
    writel(value, _ioaddr(REG_CE_TRNG_CONTROL));

#if AIS31_ENABLED
    //ba431_enable_AIS31_test_irq();
    value = readl(_ioaddr(REG_CE_TRNG_CONTROL));
    value = reg_value(1, value, TRNG_INTENALM_SHIFT, TRNG_INTENALM_MASK);
    LTRACEF("writel reg =%x, value = %x.\n", REG_CE_TRNG_CONTROL, value);
    writel(value, _ioaddr(REG_CE_TRNG_CONTROL));
#endif

    return CRYPTOLIB_SUCCESS;
}

bool trng_startup_failed(void)
{
    return rng_startup_failed;
}

void trng_set_startup_chk_flag(void)
{
    rng_needs_startup_chk = true;
}

bool trng_critical_raised(void)
{
    uint32_t status = trng_status_glb(0xffffffff);
    uint32_t critical_mask = TRNG_PROPFAIL_MASK | TRNG_REPFAIL_MASK;

#if AIS31_ENABLED
    critical_mask |= TRNG_ALMINT_MASK;
#endif

    return status & critical_mask;
}

uint32_t trng_get_word(uint32_t vce_id)
{
    uint32_t rand = 0;
    trng_get_rand(vce_id, (uint8_t*)&rand, sizeof(rand));
    return rand;
}

void trng_fill_blk(uint32_t vce_id, void* param, block_t result)
{
    (void)param;
    trng_get_rand_blk(vce_id, result);
}

bool trng_is_error_detected(void)
{
    return FSM_STATE_ERROR == trng_status_glb(0xffffffff);
}

uint32_t trng_get_hrng(uint32_t vce_id)
{
    return readl(_ioaddr(REG_HRNG_NUM_CE_(vce_id)));
}

uint32_t rng_get_prng(uint32_t vce_id, uint8_t* dst, uint32_t size)
{
    uint32_t ret = 0;
    uint32_t rng_value = 0;
    uint32_t i = 0;
    uint32_t index = 0;
    uint32_t index_max = 0;
    uint32_t cp_left = 0;

    if (dst == NULL) {
        return ret;
    }

    index = 0;
    index_max = size >> 2; //4 byte one cp
    cp_left = size & 0x3;

    for (i = 0; i < index_max; i++) {
        rng_value = readl(_ioaddr(REG_HRNG_NUM_CE_(vce_id)));
        memcpy(dst + index, (void*)(&rng_value), sizeof(rng_value));
        index = index + 4;
        ret = index;
    }

    if (cp_left > 0) {
        rng_value = readl(_ioaddr(REG_HRNG_NUM_CE_(vce_id)));
        memcpy(dst + index, (void*)(&rng_value), cp_left);
        ret = index + cp_left;
    }

    return ret;
}

uint32_t rng_get_trng(uint32_t vce_id, uint8_t* dst, uint32_t size)
{
    uint32_t ret = trng_get_rand(vce_id, dst, size);

    if (ret) {
        return 0;
    }

    return 1;
}

/*
uint32_t trng_get_rand_by_fifo(uint8_t * dst, uint32_t * size)
{
    if (!dst || !size) {
        return CRYPTOLIB_INVALID_PARAM;
    }

#if CE_IN_SAFETY_DOMAIN
    uint32_t read_cnt = 16;
#else
    uint32_t read_cnt = 32;
#endif

    uint32_t rng_value;
    addr_t baddr;
    uint32_t status = readl(_ioaddr(REG_CE_TRNG_STATUS));

    while (0x0 != (status & 0x300)) {
        LTRACEF("trng_get_rand_by_fifo status=0x%x.\n", status);
        trng_soft_reset();
        status = readl(_ioaddr(REG_CE_TRNG_STATUS));
    }

    *size = 0;

    if (0x0 != (status & 0x80)) {
        for (uint32_t i = 0; i < read_cnt; i++) {
            baddr = REG_CE_TRNG_CONTROL + 0x80; //FIFO APB address
            rng_value = readl(_ioaddr(baddr));

            memcpy(dst, (void*)(&rng_value), 4);
            baddr += 4;
            *size += 4;
            dst += 4;
        }

        return CRYPTOLIB_SUCCESS;
    }

    LTRACEF("trng_get_rand_by_fifo fifo is null staus 0x%x.\n", status);

    return CRYPTOLIB_UNSUPPORTED_ERR;
}
*/

uint32_t trng_get_rand_by_fifo(uint8_t* dst, uint32_t size)
{
    if (!dst) {
        return CRYPTOLIB_INVALID_PARAM;
    }

#if CE_IN_SAFETY_DOMAIN
    uint32_t read_cnt = 16;
#else
    uint32_t read_cnt = 32;
#endif

    uint32_t rng_value;
    addr_t baddr;
    uint32_t status;
    uint32_t read_size = 0;

    do {
        status = readl(_ioaddr(REG_CE_TRNG_STATUS));

        while (0x0 != (status & 0x300)) {
            LTRACEF("trng_get_rand_by_fifo status=0x%x.\n", status);
            trng_soft_reset();
            status = readl(_ioaddr(REG_CE_TRNG_STATUS));
        }

        if (0x0 != (status & 0x80)) {
            baddr = REG_CE_TRNG_CONTROL + 0x80; //FIFO APB address

            for (uint32_t i = 0; i < read_cnt; i++) {
                if (size <= read_size) {
                    return CRYPTOLIB_SUCCESS;
                }

                rng_value = readl(_ioaddr(baddr));

                memcpy(dst, (void*)(&rng_value), 4);
                baddr += 4;
                read_size += 4;
                dst += 4;
            }
        }
    }
    while (read_size < size);

    LTRACEF("trng_get_rand_by_fifo fifo is null staus 0x%x.\n", status);

    return CRYPTOLIB_UNSUPPORTED_ERR;
}

uint32_t rng_get_rand_less_ref(uint32_t vce_id, block_t dst, block_t n)
{
    if (dst.len != n.len) {
        return CRYPTOLIB_INVALID_PARAM;
    }

    if (!(n.addr[n.len - 1] & 0x01)) { /*n must be odd*/
        return CRYPTOLIB_INVALID_PARAM;
    }

    bool rnd_invalid;

    /* Check what is the most significant bit of n and compute an index of and a
     * mask for the most significant byte that can be used to remove the leading
     * zeros.
     */
    uint32_t index = 0;
    uint8_t msb_mask = 0xFF;

    /*since n is odd, at minimum the
     *least significant byte should be
     *different from 0
    */
    for (; !n.addr[index]; index++);

    for (; n.addr[index] & msb_mask; msb_mask <<= 1);

    /* Create container for random value from RNG, pointing to the same buffer as
     * dst but referring only to [MSB-1:0] instead of [len-1:0].
     * Force the leading, non-significant bytes to zero.
     */
    memset(dst.addr, 0, index);
    block_t rnd = block_t_convert(dst.addr + index, dst.len - index, EXT_MEM);

    do {
        /* Get a random value */
        //rng_get_trng(vce_id, rnd.addr, rnd.len);
        trng_get_rand_by_fifo(rnd.addr, rnd.len);

        /* Mask off the leading non-significant bits. Keep only the bits that are
         * relevant according to msb_mask. This is done to speed up the process of
         * finding a proper random value.
         * For example:
         * If the highest byte of n is 0x06, the chance that we get a random with
         * a highest byte <= 0x06 is only 7/256 without masking.
         * With the masking process (msb_mask = 0xF8, ~msb_mask = 0x07) we
         * significantly increase the chances of immediately finding a suitable
         * random.
         */
        dst.addr[index] &= ~msb_mask; /*Note that dst.addr[index] = rnd.addr[0]*/

        /* Check if rnd > n-2 (constant time comparison) */
        bool gt = false;
        bool eq = true;
        uint32_t leftop = 0;
        uint32_t rightop = 0;

        for (uint32_t i = 0; i < dst.len; i++) {
            leftop = dst.addr[i];
            rightop = n.addr[i];

            /* We rephrase rnd > n-2 as rnd >= n-1. Since n is odd, n-1 is obtained
             * by masking 1 bit.
             */
            if (i == dst.len - 1) {
                rightop &= 0xFE;
            }

            /* We use a trick to determine whether leftop >= rightop to avoid
             * possible time dependency in the implementations of ">", "<" and "==".
             * If leftop > rightop then (rightop - leftop) will be 0xFFFFFFxx.
             * If leftop <= rightop then (rightop - leftop) will be 0x000000xx.
             * By shifting out the xx, we can determine the relation between left
             * and right.
             *
             * A similar trick is used to determine whether leftop == rightop.
             * If leftop == rightop then (rightop ^ leftop) - 1 will be 0xFFFFFFFF.
             * If leftop != rightop then (rightop ^ leftop) - 1 will be 0x000000xx.
             *
             * By muxing eq with eq, we ensure that eq will be zero from the first
             * different byte onwards.
             * By muxing the leftop >= rightop check with eq, we ensure that it
             * only has an effect when executed on the first most significant byte
             * that is different between the arrays.
             */
            gt |= (bool)((rightop - leftop) >> 8) & eq;
            eq &= (bool)(((rightop ^ leftop) - 1) >> 8);
        }

        rnd_invalid = gt | eq;

    }
    while (rnd_invalid);

    /* Compute k = rnd + 1 (constant time increment) */
    math_array_incr(dst.addr, dst.len, 1);

    return CRYPTOLIB_SUCCESS;
}
