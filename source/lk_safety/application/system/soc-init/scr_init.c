/*
 * Copyright (c) 2020 Semidrive Inc.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <app.h>
#include <chip_res.h>
#include <debug.h>
#include <lib/console.h>
#include <reg.h>
#include <res.h>
#include <string.h>
#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0

/* ce key ctrl, use 16 bits, bit value = 1 mean the ce can access the key
*_________________________________________________________________________________________________________
*|-----15-----|-----14-----|-----13-----|-----12-----|-----11-----|-----10-----|-----09-----|-----08 ----|
*|          reserve        |  VLK LOCK  |  VLK LOCK  |            |            | EFUSE GPK3 | EFUSE GPK2 |
*_________________________________________________________________________________________________________
*|-----07-----|-----06-----|-----05-----|-----04-----|-----03-----|-----02-----|-----01-----|-----00-----|
*| EFUSE GPK1 | EFUSE GPK0 | EFUSE SEK2 | EFUSE SEK1 | EFUSE SEK0 |EFUSE ROTPK1|EFUSE ROTPK0|     HUK    |
*/
typedef enum key_enable_offset {
    SAFE_SS_GLUE_OR2_NET_HUK_HLOCK_SE_PROD_Z = 0,
    EFUSEC_ROTPK0_HLOCK_SE,
    EFUSEC_ROTPK1_HLOCK_SE,
    EFUSEC_SEK0_HLOCK_SE,
    EFUSEC_SEK1_HLOCK_SE,
    EFUSEC_SEK2_HLOCK_SE,
    EFUSEC_GP_KEY0_HLOCK_SE,
    EFUSEC_GP_KEY1_HLOCK_SE,
    EFUSEC_GP_KEY2_HLOCK_SE,
    EFUSEC_GP_KEY3_HLOCK_SE,
    SAFE_SS_GLUE_OR2_CE1_KEY_PERCTRL_KEY_HLOCK_10_Z,
    SAFE_SS_GLUE_OR2_CE1_KEY_PERCTRL_KEY_HLOCK_11_Z,
    I_SEC_STORAGE1_VLK_HLOCK_SAFETY_LOCK,
    I_SEC_STORAGE2_VLK_HLOCK_SAFETY_LOCK,
} key_enable_offset_t;

void L16_ce1key_enable_init(paddr_t phy_addr, int32_t offset_index)
{
    unsigned int data = 0;

    LTRACEF("enter phy_addr = 0x%x, offset_index = %d\n", (uint32_t)phy_addr,
            offset_index);
    /* scr_saf, CE1_KEY_PERCTRL_SCR_KEY_ENABLE_15_0, [143:128], scr_num - 4, bits - 16 */
    /* scr_saf[143:128], start_bit - 0, end_bit - 15, offset - 0x2100[15:0] */
    /* huk rotpk1 sek0 can access */
    data = (1 << SAFE_SS_GLUE_OR2_NET_HUK_HLOCK_SE_PROD_Z) |
           (1 << EFUSEC_ROTPK1_HLOCK_SE) | (1 << EFUSEC_SEK0_HLOCK_SE);
    writel(data, phy_addr + (offset_index << 12));

    if (LOCAL_TRACE) {
        data = readl(phy_addr + (offset_index << 12));
        LTRACEF("phy_addr_temp = 0x%x, data = 0x%x\n",
                (uint32_t)(phy_addr + (offset_index << 12)), data);
    }
}

int ce_key_ctrl(void)
{

    int ret = 0;
    paddr_t phy_addr = 0;
    int32_t resid = 0;
    int32_t offset_index = 0;

    resid = RES_SCR_L16_SAF_CE1_KEY_PERCTRL_SCR_KEY_ENABLE;
    ret = res_get_info_by_id(resid, &phy_addr, &offset_index);

    if (ret == -1) {
        LTRACEF("ce1 resouce error\n");
    }
    else {
        L16_ce1key_enable_init(phy_addr, offset_index);
    }

    return ret;
}

/* hardcode scr init by following Z1 configuration when system boot */
#define SCR_SEC_BASE 0xF8200000
void src_init_hc(void)
{
    uint32_t rval;

    /* selects the PHY interface of the ethernet MAC as RGMII */
    rval = readl(SCR_SEC_BASE + (0x614 << 10));
    rval = (rval & (~0x38)) | 0x8;
    writel(rval, SCR_SEC_BASE + (0x614 << 10));

    /* Set i2S SC default mode to full duplex-mode*/
    rval = readl(SCR_SEC_BASE + (0xC << 10));
#if (TARGET_REFERENCE_D9 || TARGET_REFERENCE_D9P)
    rval =  rval  | 0x3D;
#else
    rval =  rval  | 0x3F;
#endif
    writel(rval, SCR_SEC_BASE + (0xC << 10));
}
/* hardcode scr init end */

int scr_init(void)
{
    int ret = 0;

    /* ce key setting */
    ret = ce_key_ctrl();

    if (ret) {
        dprintf(CRITICAL, "ce_key_ctrl fail.\n");
        return ret;
    }

    /* other setting */
    src_init_hc();

    return ret;
}

