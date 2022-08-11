/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <_assert.h>
#include <debug.h>
#include <compiler.h>
#include <arch.h>
#include <mmc_sdhci.h>

#ifdef DBG
#undef DBG
#define DBG(fmt, args...)
#endif
// NOTE: for test on zone, need define WITH_ON_ZONE macro
//#define WITH_ON_ZONE 1

#define USE_TARGET_HS200_CAPS 1

#define EXT_CSD_BUF_SIZE ROUNDUP(512, CACHE_LINE)

static uint8_t ext_csd_buf[EXT_CSD_BUF_SIZE] __ALIGNED(CACHE_LINE);

struct sdhci_priv_data sdhci_priv;

/* data access time unit in ns */
static const uint32_t taac_unit[] = {1,     10,     100,     1000,
                                     10000, 100000, 1000000, 10000000};

/* data access time value x 10 */
static const uint32_t taac_value[] = {0,  10, 12, 13, 15, 20, 25, 30,
                                      35, 40, 45, 50, 55, 60, 70, 80};

/* data transfer rate in kbit/s */
static const uint32_t xfer_rate_unit[] = {100, 1000, 10000, 100000, 0, 0, 0, 0};

/* data transfer rate value x 10*/
static const uint32_t xfer_rate_value[] = {0,  10, 12, 13, 15, 20, 26, 30,
                                           35, 40, 45, 52, 55, 60, 70, 80};

/*
 * Function: mmc decode and save csd
 * Arg     : Card structure & raw csd
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Decodes CSD response received from the card.
 *           Note that we have defined only few of the CSD elements
 *           in csd structure. We'll only decode those values.
 */
static uint32_t mmc_decode_and_save_csd(struct mmc_card *card)
{
    uint32_t mmc_sizeof = 0;
    uint32_t mmc_unit = 0;
    uint32_t mmc_value = 0;
    uint32_t mmc_temp = 0;
    uint32_t *raw_csd = card->raw_csd;

    struct mmc_csd mmc_csd;

    mmc_sizeof = sizeof(uint32_t) * 8;

    mmc_csd.cmmc_structure = UNPACK_BITS(raw_csd, 126, 2, mmc_sizeof);

    if (MMC_CARD_SD(card)) {
        /* Parse CSD according to SD card spec. */

        /*
         * CSD register is little bit differnet for CSD version 2.0 High
         * Capacity and CSD version 1.0/2.0 Standard memory cards.
         * In Version 2.0 some of the fields have fixed values and it's
         * not necessary for host to refer these fields in CSD sent by
         * card
         */

        if (mmc_csd.cmmc_structure == 1) {
            /* CSD Version 2.0 */
            mmc_csd.card_cmd_class = UNPACK_BITS(raw_csd, 84, 12, mmc_sizeof);
            /* Fixed value is 9 = 2^9 = 512 */
            mmc_csd.write_blk_len = 512;
            /* Fixed value is 9 = 512 */
            mmc_csd.read_blk_len = 512;
            /* Fixed value: 010b */
            mmc_csd.r2w_factor = 0x2;
            /* Not there in version 2.0 */
            mmc_csd.c_size_mult = 0;
            mmc_csd.c_size = UNPACK_BITS(raw_csd, 48, 22, mmc_sizeof);
            mmc_csd.nsac_clk_cycle =
                UNPACK_BITS(raw_csd, 104, 8, mmc_sizeof) * 100;

            mmc_unit = UNPACK_BITS(raw_csd, 112, 3, mmc_sizeof);
            mmc_value = UNPACK_BITS(raw_csd, 115, 4, mmc_sizeof);
            mmc_csd.taac_ns =
                (taac_value[mmc_value] * taac_unit[mmc_unit]) / 10;

            mmc_csd.erase_blk_len = 1;
            mmc_csd.read_blk_misalign = 0;
            mmc_csd.write_blk_misalign = 0;
            mmc_csd.read_blk_partial = 0;
            mmc_csd.write_blk_partial = 0;

            mmc_unit = UNPACK_BITS(raw_csd, 96, 3, mmc_sizeof);
            mmc_value = UNPACK_BITS(raw_csd, 99, 4, mmc_sizeof);
            mmc_csd.tran_speed =
                (xfer_rate_value[mmc_value] * xfer_rate_unit[mmc_unit]) / 10;

            mmc_csd.wp_grp_size = 0x0;
            mmc_csd.wp_grp_enable = 0x0;
            mmc_csd.perm_wp = UNPACK_BITS(raw_csd, 13, 1, mmc_sizeof);
            mmc_csd.temp_wp = UNPACK_BITS(raw_csd, 12, 1, mmc_sizeof);

            /* Calculate the card capcity */
            card->capacity =
                (unsigned long long)(1 + mmc_csd.c_size) * 512 * 1024;
        }
        else {
            /* CSD Version 1.0 */
            mmc_csd.card_cmd_class = UNPACK_BITS(raw_csd, 84, 12, mmc_sizeof);

            mmc_temp = UNPACK_BITS(raw_csd, 22, 4, mmc_sizeof);
            mmc_csd.write_blk_len =
                (mmc_temp > 8 && mmc_temp < 12) ? (1 << mmc_temp) : 512;

            mmc_temp = UNPACK_BITS(raw_csd, 80, 4, mmc_sizeof);
            mmc_csd.read_blk_len =
                (mmc_temp > 8 && mmc_temp < 12) ? (1 << mmc_temp) : 512;

            mmc_unit = UNPACK_BITS(raw_csd, 112, 3, mmc_sizeof);
            mmc_value = UNPACK_BITS(raw_csd, 115, 4, mmc_sizeof);
            mmc_csd.taac_ns =
                (taac_value[mmc_value] * taac_unit[mmc_unit]) / 10;

            mmc_unit = UNPACK_BITS(raw_csd, 96, 3, mmc_sizeof);
            mmc_value = UNPACK_BITS(raw_csd, 99, 4, mmc_sizeof);
            mmc_csd.tran_speed =
                (xfer_rate_value[mmc_value] * xfer_rate_unit[mmc_unit]) / 10;

            mmc_csd.nsac_clk_cycle =
                UNPACK_BITS(raw_csd, 104, 8, mmc_sizeof) * 100;

            mmc_csd.r2w_factor = UNPACK_BITS(raw_csd, 26, 3, mmc_sizeof);
            mmc_csd.sector_size = UNPACK_BITS(raw_csd, 39, 7, mmc_sizeof) + 1;

            mmc_csd.erase_blk_len = UNPACK_BITS(raw_csd, 46, 1, mmc_sizeof);
            mmc_csd.read_blk_misalign = UNPACK_BITS(raw_csd, 77, 1, mmc_sizeof);
            mmc_csd.write_blk_misalign =
                UNPACK_BITS(raw_csd, 78, 1, mmc_sizeof);
            mmc_csd.read_blk_partial = UNPACK_BITS(raw_csd, 79, 1, mmc_sizeof);
            mmc_csd.write_blk_partial = UNPACK_BITS(raw_csd, 21, 1, mmc_sizeof);

            mmc_csd.c_size_mult = UNPACK_BITS(raw_csd, 47, 3, mmc_sizeof);
            mmc_csd.c_size = UNPACK_BITS(raw_csd, 62, 12, mmc_sizeof);
            mmc_csd.wp_grp_size = UNPACK_BITS(raw_csd, 32, 7, mmc_sizeof);
            mmc_csd.wp_grp_enable = UNPACK_BITS(raw_csd, 31, 1, mmc_sizeof);
            mmc_csd.perm_wp = UNPACK_BITS(raw_csd, 13, 1, mmc_sizeof);
            mmc_csd.temp_wp = UNPACK_BITS(raw_csd, 12, 1, mmc_sizeof);

            /* Calculate the card capacity */
            mmc_temp = (1 << (mmc_csd.c_size_mult + 2)) * (mmc_csd.c_size + 1);
            card->capacity =
                (unsigned long long)mmc_temp * mmc_csd.read_blk_len;
        }
    }
    else {
        /* Parse CSD according to MMC card spec. */
        mmc_csd.spec_vers = UNPACK_BITS(raw_csd, 122, 4, mmc_sizeof);
        mmc_csd.card_cmd_class = UNPACK_BITS(raw_csd, 84, 12, mmc_sizeof);
        mmc_csd.write_blk_len = 1 << UNPACK_BITS(raw_csd, 22, 4, mmc_sizeof);
        mmc_csd.read_blk_len = 1 << UNPACK_BITS(raw_csd, 80, 4, mmc_sizeof);
        mmc_csd.r2w_factor = UNPACK_BITS(raw_csd, 26, 3, mmc_sizeof);
        mmc_csd.c_size_mult = UNPACK_BITS(raw_csd, 47, 3, mmc_sizeof);
        mmc_csd.c_size = UNPACK_BITS(raw_csd, 62, 12, mmc_sizeof);
        mmc_csd.nsac_clk_cycle = UNPACK_BITS(raw_csd, 104, 8, mmc_sizeof) * 100;

        mmc_unit = UNPACK_BITS(raw_csd, 112, 3, mmc_sizeof);
        mmc_value = UNPACK_BITS(raw_csd, 115, 4, mmc_sizeof);
        mmc_csd.taac_ns = (taac_value[mmc_value] * taac_unit[mmc_unit]) / 10;

        mmc_csd.read_blk_misalign = UNPACK_BITS(raw_csd, 77, 1, mmc_sizeof);
        mmc_csd.write_blk_misalign = UNPACK_BITS(raw_csd, 78, 1, mmc_sizeof);
        mmc_csd.read_blk_partial = UNPACK_BITS(raw_csd, 79, 1, mmc_sizeof);
        mmc_csd.write_blk_partial = UNPACK_BITS(raw_csd, 21, 1, mmc_sizeof);

        /* Ignore -- no use of this value. */
        mmc_csd.tran_speed = 0x00;

        mmc_csd.erase_grp_size = UNPACK_BITS(raw_csd, 42, 5, mmc_sizeof);
        mmc_csd.erase_grp_mult = UNPACK_BITS(raw_csd, 37, 5, mmc_sizeof);
        mmc_csd.wp_grp_size = UNPACK_BITS(raw_csd, 32, 5, mmc_sizeof);
        mmc_csd.wp_grp_enable = UNPACK_BITS(raw_csd, 31, 1, mmc_sizeof);
        mmc_csd.perm_wp = UNPACK_BITS(raw_csd, 13, 1, mmc_sizeof);
        mmc_csd.temp_wp = UNPACK_BITS(raw_csd, 12, 1, mmc_sizeof);

        /* Calculate the card capcity */
        if (mmc_csd.c_size != 0xFFF) {
            /* For cards less than or equal to 2GB */
            mmc_temp = (1 << (mmc_csd.c_size_mult + 2)) * (mmc_csd.c_size + 1);
            card->capacity =
                (unsigned long long)mmc_temp * mmc_csd.read_blk_len;
        }
        else {
            /* For cards greater than 2GB, Ext CSD register's SEC_COUNT
             * is used to calculate the size.
             */
            uint64_t sec_count;

            sec_count =
                (card->ext_csd[MMC_SEC_COUNT4] << MMC_SEC_COUNT4_SHIFT) |
                (card->ext_csd[MMC_SEC_COUNT3] << MMC_SEC_COUNT3_SHIFT) |
                (card->ext_csd[MMC_SEC_COUNT2] << MMC_SEC_COUNT2_SHIFT) |
                card->ext_csd[MMC_SEC_COUNT1];
            card->capacity = sec_count * MMC_BLK_SZ;
        }
    }

    /* save the information in card structure */
    memcpy(&card->csd, &mmc_csd, sizeof(struct mmc_csd));

    /* Calculate the wp grp size */
    if (MMC_CARD_MMC(card)) {
        if (card->ext_csd[MMC_ERASE_GRP_DEF])
            card->wp_grp_size = MMC_HC_ERASE_MULT *
                                card->ext_csd[MMC_HC_ERASE_GRP_SIZE] /
                                MMC_BLK_SZ;
        else
            card->wp_grp_size = (card->csd.wp_grp_size + 1) *
                                (card->csd.erase_grp_size + 1) *
                                (card->csd.erase_grp_mult + 1);
        card->rpmb_size = RPMB_PART_MIN_SIZE * card->ext_csd[RPMB_SIZE_MULT];
        card->rel_wr_count = card->ext_csd[REL_WR_SEC_C];
    }
    else {
        card->wp_grp_size = (card->csd.wp_grp_size + 1) *
                            (card->csd.erase_grp_size + 1) *
                            (card->csd.erase_grp_mult + 1);
    }

    DBG("Decoded CSD fields:\n");
    DBG("cmmc_structure: %u\n", mmc_csd.cmmc_structure);
    DBG("card_cmd_class: %x\n", mmc_csd.card_cmd_class);
    DBG("write_blk_len: %u\n", mmc_csd.write_blk_len);
    DBG("read_blk_len: %u\n", mmc_csd.read_blk_len);
    DBG("r2w_factor: %u\n", mmc_csd.r2w_factor);
    DBG("sector_size: %u\n", mmc_csd.sector_size);
    DBG("c_size_mult:%u\n", mmc_csd.c_size_mult);
    DBG("c_size: %u\n", mmc_csd.c_size);
    DBG("nsac_clk_cycle: %u\n", mmc_csd.nsac_clk_cycle);
    DBG("taac_ns: %u\n", mmc_csd.taac_ns);
    DBG("tran_speed: %u kbps\n", mmc_csd.tran_speed);
    DBG("erase_blk_len: %u\n", mmc_csd.erase_blk_len);
    DBG("read_blk_misalign: %u\n", mmc_csd.read_blk_misalign);
    DBG("write_blk_misalign: %u\n", mmc_csd.write_blk_misalign);
    DBG("read_blk_partial: %u\n", mmc_csd.read_blk_partial);
    DBG("write_blk_partial: %u\n", mmc_csd.write_blk_partial);
    DBG("wp_grp_size: %u\n", card->wp_grp_size);
    DBG("Card Capacity: %llu Bytes\n", card->capacity);

    return 0;
}

/*
 * Function: mmc decode & save cid
 * Arg     : card structure & raw cid
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Decode CID sent by the card.
 */
static uint32_t mmc_decode_and_save_cid(struct mmc_card *card,
                                        uint32_t *raw_cid)
{
    struct mmc_cid mmc_cid;
    uint32_t mmc_sizeof = 0;
    int i = 0;

    if (!raw_cid) {
        return 1;
    }

    mmc_sizeof = sizeof(uint32_t) * 8;

    if (MMC_CARD_SD(card)) {
        mmc_cid.mid = UNPACK_BITS(raw_cid, 120, 8, mmc_sizeof);
        mmc_cid.oid = UNPACK_BITS(raw_cid, 104, 16, mmc_sizeof);

        for (i = 0; i < 5; i++) {
            mmc_cid.pnm[i] = (uint8_t)UNPACK_BITS(raw_cid, (104 - 8 * (i + 1)),
                                                  8, mmc_sizeof);
        }
        mmc_cid.pnm[5] = 0;
        mmc_cid.pnm[6] = 0;

        mmc_cid.prv = UNPACK_BITS(raw_cid, 56, 8, mmc_sizeof);
        mmc_cid.psn = UNPACK_BITS(raw_cid, 24, 32, mmc_sizeof);
        mmc_cid.month = UNPACK_BITS(raw_cid, 8, 4, mmc_sizeof);
        mmc_cid.year = UNPACK_BITS(raw_cid, 12, 8, mmc_sizeof);
        mmc_cid.year += 2000;
    }
    else {
        mmc_cid.mid = UNPACK_BITS(raw_cid, 120, 8, mmc_sizeof);
        mmc_cid.oid = UNPACK_BITS(raw_cid, 104, 16, mmc_sizeof);

        for (i = 0; i < 6; i++) {
            mmc_cid.pnm[i] = (uint8_t)UNPACK_BITS(raw_cid, (104 - 8 * (i + 1)),
                                                  8, mmc_sizeof);
        }
        mmc_cid.pnm[6] = 0;

        mmc_cid.prv = UNPACK_BITS(raw_cid, 48, 8, mmc_sizeof);
        mmc_cid.psn = UNPACK_BITS(raw_cid, 16, 32, mmc_sizeof);
        mmc_cid.month = UNPACK_BITS(raw_cid, 8, 4, mmc_sizeof);
        mmc_cid.year = UNPACK_BITS(raw_cid, 12, 4, mmc_sizeof);
        mmc_cid.year += 1997;
    }

    /* save it in card database */
    memcpy(&card->cid, &mmc_cid, sizeof(struct mmc_cid));

    DBG("Decoded CID fields:\n");
    DBG("Manufacturer ID: %x\n", mmc_cid.mid);
    DBG("OEM ID: 0x%x\n", mmc_cid.oid);
    DBG("Product Name: %s\n", mmc_cid.pnm);
    DBG("Product revision: %d.%d\n", (mmc_cid.prv >> 4),
            (mmc_cid.prv & 0xF));
    DBG("Product serial number: %X\n", mmc_cid.psn);
    DBG("Manufacturing date: %d %d\n", mmc_cid.month, mmc_cid.year);

    return 0;
}

/*
 * Function: mmc reset cards
 * Arg     : host structure
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Reset all the cards to idle condition (CMD 0)
 */
static uint8_t mmc_reset_card(struct sdhci_host *host)
{
    struct mmc_command cmd;

    memset(&cmd, 0, sizeof(struct mmc_command));

    cmd.cmd_index = CMD0_GO_IDLE_STATE;
#ifdef WITH_ON_ZONE
    /* if on zone, need use argument for go to pre-idel state */
    cmd.argument = 0xF0F0F0F0;
#else
    cmd.argument = 0;
#endif
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_NONE;

    /* send command */
    return sdhci_send_command(host, &cmd);
}

/*
 * Function: mmc operations command
 * Arg     : host & card structure
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Send CMD1 to know whether the card supports host VDD profile or
 * not.
 */
static uint32_t mmc_send_op_cond(struct sdhci_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;
    uint32_t mmc_resp = 0;
    uint32_t mmc_ret = 0;
    uint32_t mmc_retry = 0;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD1 format:
     * [31] Busy bit
     * [30:29] Access mode
     * [28:24] reserved
     * [23:15] 2.7-3.6
     * [14:8]  2.0-2.6
     * [7]     1.7-1.95
     * [6:0]   reserved
     */

    cmd.cmd_index = CMD1_SEND_OP_COND;
    cmd.argument = card->ocr;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R3;

    do {
        mmc_ret = sdhci_send_command(host, &cmd);
        if (mmc_ret)
            return mmc_ret;

        /* Command returned success, now it's time to examine response */
        mmc_resp = cmd.resp[0];

        /* Check the response for busy status */
        if (!(mmc_resp & MMC_OCR_BUSY)) {
            mmc_retry++;
            udelay(1);
            continue;
        }
        else
            break;
    } while (mmc_retry < MMC_MAX_COMMAND_RETRY);

    /* If we reached here after max retries, we failed to get OCR */
    if (mmc_retry == MMC_MAX_COMMAND_RETRY && !(mmc_resp & MMC_OCR_BUSY)) {
        FATAL("Card has busy status set. Init did not complete\n");
        return 1;
    }

    /* Response contains card's ocr. Update card's information */
    card->ocr = mmc_resp;

    if (mmc_resp & MMC_OCR_SEC_MODE)
        card->type = MMC_TYPE_MMCHC;
    else
        card->type = MMC_TYPE_STD_MMC;

    return 0;
}

/*
 * Function: mmc send cid
 * Arg     : host & card structure
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Request any card to send its uniquie card identification
 *           (CID) number (CMD2).
 */
static uint32_t mmc_all_send_cid(struct sdhci_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;
    uint32_t mmc_ret = 0;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD2 Format:
     * [31:0] stuff bits
     */
    cmd.cmd_index = CMD2_ALL_SEND_CID;
    cmd.argument = 0;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R2;

    /* send command */
    mmc_ret = sdhci_send_command(host, &cmd);
    if (mmc_ret) {
        return mmc_ret;
    }

    /* Response contains card's 128 bits CID register */
    mmc_ret = mmc_decode_and_save_cid(card, cmd.resp);
    if (mmc_ret) {
        return mmc_ret;
    }

    return 0;
}

/*
 * Function: mmc send relative address
 * Arg     : host & card structure
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Ask card to send it's relative card address (RCA).
 *           This RCA number is shorter than CID and is used by
 *           the host to address the card in future (CMD3)
 */
static uint32_t mmc_send_relative_address(struct sdhci_host *host,
                                          struct mmc_card *card)
{
    struct mmc_command cmd;
    uint32_t mmc_ret = 0;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD3 Format:
     * [31:0] stuff bits
     */
    if (MMC_CARD_SD(card)) {
        cmd.cmd_index = CMD3_SEND_RELATIVE_ADDR;
        cmd.argument = 0;
        cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
        cmd.resp_type = SDHCI_CMD_RESP_R6;

        /* send command */
        mmc_ret = sdhci_send_command(host, &cmd);
        if (mmc_ret)
            return mmc_ret;

        /* For sD, card will send RCA. Store it */
        card->rca = (cmd.resp[0] >> 16);
    }
    else {
        cmd.cmd_index = CMD3_SEND_RELATIVE_ADDR;
        cmd.argument = (MMC_RCA << 16);
        card->rca = (cmd.argument >> 16);
        cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
        cmd.resp_type = SDHCI_CMD_RESP_R6;

        /* send command */
        mmc_ret = sdhci_send_command(host, &cmd);
        if (mmc_ret)
            return mmc_ret;
    }

    return 0;
}

/*
 * Function: mmc send csd
 * Arg     : host, card structure & o/p arg to store csd
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Requests card to send it's CSD register's contents. (CMD9)
 */
static uint32_t mmc_send_csd(struct sdhci_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;
    uint32_t mmc_arg = 0;
    uint32_t mmc_ret = 0;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD9 Format:
     * [31:16] RCA
     * [15:0] stuff bits
     */
    mmc_arg |= card->rca << 16;

    cmd.cmd_index = CMD9_SEND_CSD;
    cmd.argument = mmc_arg;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R2;

    /* send command */
    mmc_ret = sdhci_send_command(host, &cmd);
    if (mmc_ret)
        return mmc_ret;

    /* response contains the card csd */
    memcpy(card->raw_csd, cmd.resp, sizeof(cmd.resp));

    return 0;
}

/*
 * Function: mmc select card
 * Arg     : host, card structure
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Selects a card by sending CMD7 to the card with its RCA.
 *           If RCA field is set as 0 ( or any other address ),
 *           the card will be de-selected. (CMD7)
 */
static uint32_t mmc_select_card(struct sdhci_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;
    uint32_t mmc_arg = 0;
    uint32_t mmc_ret = 0;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD7 Format:
     * [31:16] RCA
     * [15:0] stuff bits
     */
    mmc_arg |= card->rca << 16;

    cmd.cmd_index = CMD7_SELECT_DESELECT_CARD;
    cmd.argument = mmc_arg;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;

    /* If we are deselecting card, we do not get response */
    if (card->rca) {
        if (MMC_CARD_SD(card))
            cmd.resp_type = SDHCI_CMD_RESP_R1B;
        else
            cmd.resp_type = SDHCI_CMD_RESP_R1;
    }
    else
        cmd.resp_type = SDHCI_CMD_RESP_NONE;

    /* send command */
    mmc_ret = sdhci_send_command(host, &cmd);
    if (mmc_ret)
        return mmc_ret;

    return 0;
}

#if 0
/*
 * Function: mmc set block len
 * Arg     : host, card structure & block length
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Send command to set block length.
 */
static uint32_t mmc_set_block_len(struct sdhci_host *host,
                                  struct mmc_card *card, uint32_t block_len)
{
    struct mmc_command cmd;
    uint32_t mmc_ret = 0;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD16 Format:
     * [31:0] block length
     */

    cmd.cmd_index = CMD16_SET_BLOCKLEN;
    cmd.argument = block_len;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;

    /* send command */
    mmc_ret = sdhci_send_command(host, &cmd);
    if (mmc_ret)
        return mmc_ret;

    /*
     * If blocklength is larger than 512 bytes,
     * the card sets BLOCK_LEN_ERROR bit.
     */
    if (cmd.resp[0] & MMC_R1_BLOCK_LEN_ERR) {
        FATAL("The block length is not supported by the card\n");
        return 1;
    }

    return 0;
}
#endif

/*
 * Function: mmc get card status
 * Arg     : host, card structure & o/p argument card status
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Get the current status of the card
 */
static uint32_t mmc_get_card_status(struct sdhci_host *host,
                                    struct mmc_card *card, uint32_t *status)
{
    struct mmc_command cmd;
    uint32_t mmc_ret = 0;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD13 Format:
     * [31:16] RCA
     * [15:0] stuff bits
     */
    cmd.cmd_index = CMD13_SEND_STATUS;
    cmd.argument = card->rca << 16;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;

    /* send command */
    mmc_ret = sdhci_send_command(host, &cmd);
    if (mmc_ret)
        return mmc_ret;

    /* Checking ADDR_OUT_OF_RANGE error in CMD13 response */
    if ((cmd.resp[0] >> 31) & 0x01)
        return 1;

    *status = cmd.resp[0];
    return 0;
}

/*
 * Function: mmc get ext csd
 * Arg     : host, card structure & array to hold ext attributes
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Send ext csd command & get the card attributes
 */
static uint32_t mmc_get_ext_csd(struct sdhci_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;
    uint32_t mmc_ret = 0;
    void *vptr;

#if WITH_KERNEL_VM
    uint8_t align_pow2 = log2_uint(CACHE_LINE);
    if (vmm_alloc_contiguous(vmm_get_kernel_aspace(), "mmc_ext",
                             EXT_CSD_BUF_SIZE, &vptr, align_pow2, 0,
                             ARCH_MMU_FLAG_CACHED) < 0) {
        printf("Failed to allocate ext csd buf\n");
        return -1;
    }
#else
    vptr = ext_csd_buf;
#endif

    card->ext_csd = vptr;
    assert(card->ext_csd);

    memset(card->ext_csd, 0, EXT_CSD_BUF_SIZE);

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD8 */
    cmd.cmd_index = CMD8_SEND_EXT_CSD;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.data.data_ptr = card->ext_csd;
    cmd.data.num_blocks = 1;
    cmd.data_present = 0x1;
    cmd.trans_mode = SDHCI_MMC_READ;

    /* send command */
    mmc_ret = sdhci_send_command(host, &cmd);
    if (mmc_ret)
        return mmc_ret;

    return mmc_ret;
}

/*
 * Function: mmc switch command
 * Arg     : Host, card structure, access mode, index & value to be set
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Send switch command to the card to set the ext attribute @ index
 */
static uint32_t mmc_switch_cmd(struct sdhci_host *host, struct mmc_card *card,
                               uint32_t access, uint32_t index, uint32_t value)
{

    struct mmc_command cmd;
    uint32_t mmc_ret = 0;
    uint32_t mmc_status;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /*
     * CMD6 Format:
     * [31:26] set to 0
     * [25:24] access
     * [23:16] index
     * [15:8] value
     * [7:3] set to 0
     * [2:0] cmd set
     */
    cmd.cmd_index = CMD6_SWITCH_FUNC;
    cmd.argument |= (access << 24);
    cmd.argument |= (index << 16);
    cmd.argument |= (value << 8);
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1B;

    mmc_ret = sdhci_send_command(host, &cmd);
    if (mmc_ret) {
        FATAL("CMD6 send failed\n");
        return mmc_ret;
    }

    if (index == MMC_EXT_MMC_HS_TIMING && value == SDHCI_EMMC_HISPEED_MODE) {
        /* Set the high speed mode in controller */
        host->ops->set_uhs_mode(host, SDHCI_EMMC_HISPEED_MODE);
        mmc_ret = host->ops->set_clock(host, MMC_CLK_50MHZ);
    }

    /* Check if the card completed the switch command processing */
    mmc_ret = mmc_get_card_status(host, card, &mmc_status);
    if (mmc_ret) {
        FATAL("Get card status failed\n");
        return mmc_ret;
    }

    if (MMC_CARD_STATUS(mmc_status) != MMC_TRAN_STATE) {
        FATAL("Switch cmd failed. Card not in tran state %x\n",
                mmc_status);
        mmc_ret = 1;
    }

    if (mmc_status & MMC_SWITCH_FUNC_ERR_FLAG) {
        FATAL("Switch cmd failed. Switch Error.\n");
        mmc_ret = 1;
    }

    return mmc_ret;
}

bool mmc_set_drv_type(struct sdhci_host *host, struct mmc_card *card,
                      uint8_t drv_type)
{
    uint32_t ret = 0;
    bool drv_type_changed = false;

    uint32_t value = ((drv_type << 4) | MMC_HS200_TIMING);

    if (card->ext_csd[MMC_EXT_MMC_DRV_STRENGTH] & (1 << drv_type))
        ret = mmc_switch_cmd(host, card, MMC_ACCESS_WRITE,
                             MMC_EXT_MMC_HS_TIMING, value);
    if (!ret)
        drv_type_changed = true;

    return drv_type_changed;
}
/*
 * Function: mmc set bus width
 * Arg     : Host, card structure & width
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Send switch command to set bus width
 */
static uint32_t mmc_set_bus_width(struct sdhci_host *host,
                                  struct mmc_card *card, uint32_t width)
{
    uint32_t mmc_ret = 0;

    mmc_ret = mmc_switch_cmd(host, card, MMC_ACCESS_WRITE,
                             MMC_EXT_MMC_BUS_WIDTH, width);

    if (mmc_ret) {
        FATAL("Switch cmd failed\n");
        return mmc_ret;
    }

    return 0;
}

/*
 * Function: mmc card supports hs400 mode
 * Arg     : None
 * Return  : 1 if hs400 mode is supported, 0 otherwise
 * Flow    : Check the ext csd attributes of the card
 */
static uint8_t mmc_card_supports_hs400_mode(struct mmc_card *card)
{
    if (card->ext_csd[MMC_DEVICE_TYPE] & MMC_HS_HS400_MODE)
        return 1;
    else
        return 0;
}

/*
 * Function: mmc card supports hs200 mode
 * Arg     : None
 * Return  : 1 if HS200 mode is supported, 0 otherwise
 * Flow    : Check the ext csd attributes of the card
 */
static uint8_t mmc_card_supports_hs200_mode(struct mmc_card *card)
{
    if (card->ext_csd[MMC_DEVICE_TYPE] & MMC_HS_HS200_MODE)
        return 1;
    else
        return 0;
}

/*
 * Function: mmc card supports ddr mode
 * Arg     : None
 * Return  : 1 if DDR mode is supported, 0 otherwise
 * Flow    : Check the ext csd attributes of the card
 */
static uint8_t mmc_card_supports_ddr_mode(struct mmc_card *card)
{
    if (card->ext_csd[MMC_DEVICE_TYPE] & MMC_HS_DDR_MODE)
        return 1;
    else
        return 0;
}

/*
 * Function : Enable HS200 mode
 * Arg      : Host, card structure and bus width
 * Return   : 0 on Success, 1 on Failure
 * Flow     :
 *           - Set the bus width to 4/8 bit SDR as supported by the target &
 * host
 *           - Set the HS_TIMING on ext_csd 185 for the card
 */
static uint32_t mmc_set_hs200_mode(struct sdhci_host *host,
                                   struct mmc_card *card, uint32_t width)
{
    uint32_t mmc_ret = 0;

    DBG("\n Enabling HS200 Mode Start\n");

    /* Set 4/8 bit SDR bus width */
    mmc_ret = mmc_set_bus_width(host, card, width);
    if (mmc_ret) {
        FATAL("Failure to set wide bus for Card(RCA:%x)\n",
                card->rca);
        return mmc_ret;
    }

    /* Setting HS200 in HS_TIMING using EXT_CSD (CMD6) */
    mmc_ret = mmc_switch_cmd(host, card, MMC_ACCESS_WRITE,
                             MMC_EXT_MMC_HS_TIMING, MMC_HS200_TIMING);

    if (mmc_ret) {
        FATAL("Switch cmd returned failure %d\n", __LINE__);
        return mmc_ret;
    }

    /* Enable HS200 mode in controller */
    MMC_SAVE_TIMING(host, MMC_HS200_TIMING);
    host->ops->set_uhs_mode(host, SDHCI_EMMC_HS200_MODE);
    mmc_ret = host->ops->set_clock(host, MMC_CLK_200MHZ);
    if (mmc_ret)
        return mmc_ret;

    /* Execute Tuning for hs200 mode */
    if ((mmc_ret = sdhci_execute_tuning(host, CMD21_SEND_TUNING_BLOCK, width)))
        FATAL("Tuning for hs200 failed\n");

    DBG("\n Enabling HS200 Mode Done\n");

    return mmc_ret;
}

/*
 * Function: mmc set ddr mode
 * Arg     : Host, card structure and bus width
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Set bus width for ddr mode & set controller in DDR mode
 */
static uint8_t mmc_set_ddr_mode(struct sdhci_host *host, struct mmc_card *card,
                                uint32_t width)
{
    uint8_t mmc_ret = 0;

    DBG("\n Enabling DDR Mode Start\n");

    /* Set 4/8 bit DDR bus width */
    if (width == MMC_DATA_BUS_WIDTH_4BIT)
        mmc_ret = mmc_set_bus_width(host, card, MMC_DATA_DDR_BUS_WIDTH_4BIT);
    else if (width == MMC_DATA_BUS_WIDTH_8BIT)
        mmc_ret = mmc_set_bus_width(host, card, MMC_DATA_DDR_BUS_WIDTH_8BIT);
    else {
        FATAL("DDR mode not suport bus width: %u\n", width);
        return 1;
    }

    if (mmc_ret) {
        FATAL("Failure to set DDR mode for Card(RCA:%x)\n",
                card->rca);
        return mmc_ret;
    }

    /* Save the timing value, before changing the clock */
    MMC_SAVE_TIMING(host, SDHCI_EMMC_DDR52_MODE);

    /* Set the DDR mode in controller */
    host->ops->set_uhs_mode(host, SDHCI_EMMC_DDR52_MODE);
    mmc_ret = host->ops->set_clock(host, MMC_CLK_50MHZ);

    DBG("\n Enabling DDR Mode Done\n");

    return mmc_ret;
}

/*
 * Function: mmc set high speed interface
 * Arg     : Host & card structure
 * Return  : None
 * Flow    : Sets the host uhs mode & clock
 *           Adjust the interface speed to optimal speed
 */
static uint32_t mmc_set_hs_interface(struct sdhci_host *host,
                                     struct mmc_card *card)
{
    uint32_t mmc_ret = 0;

    /* Setting HS_TIMING in EXT_CSD (CMD6) */
    mmc_ret = mmc_switch_cmd(host, card, MMC_ACCESS_WRITE,
                             MMC_EXT_MMC_HS_TIMING, MMC_HS_TIMING);
    if (mmc_ret) {
        FATAL("Switch cmd returned failure %d\n", __LINE__);
        return mmc_ret;
    }

    /* Save the timing value, before changing the clock */
    MMC_SAVE_TIMING(host, SDHCI_EMMC_HISPEED_MODE);

    return mmc_ret;
}

/*
 * Function : Enable HS400 mode
 * Arg      : Host, card structure and bus width
 * Return   : 0 on Success, 1 on Failure
 * Flow     :
 *           - Set the bus width to 8 bit DDR
 *           - Set the HS_TIMING on ext_csd 185 for the card
 */
uint32_t mmc_set_hs400_mode(struct sdhci_host *host, struct mmc_card *card,
                            uint32_t width)
{
    uint32_t mmc_ret = 0;

    /*
     * Emmc 5.0 spec does not allow changing to hs400 mode directly
     * Need to follow the sequence to change to hs400 mode
     * 1. Enable HS200 mode, perform tuning
     * 2. Change to high speed mode
     * 3. Enable DDR mode
     * 4. Enable HS400 mode, note that hs400 tuning is done in enable hs200 step
     */

    DBG("\n Enabling HS400 Mode Start\n");
    /* HS400 mode is supported only in DDR 8-bit */
    if (width != MMC_DATA_BUS_WIDTH_8BIT) {
        FATAL("Bus width is not 8-bit, cannot switch to hs400: %u\n", width);
        return 1;
    }

    /* 1.Enable HS200 mode */
    mmc_ret = mmc_set_hs200_mode(host, card, width);

    if (mmc_ret) {
        FATAL("Failure Setting HS200 mode %s\t%d\n", __func__, __LINE__);
        return mmc_ret;
    }

    /* 2. Enable High speed mode */
    mmc_ret = mmc_set_hs_interface(host, card);
    if (mmc_ret) {
        FATAL("Error adjusting interface speed!:%s\t%d\n", __func__, __LINE__);
        return mmc_ret;
    }

    /*3. Enable DDR mode */
    mmc_ret = mmc_set_ddr_mode(host, card, width);
    if (mmc_ret) {
        FATAL("Failure setting DDR mode:%s\t%d\n", __func__, __LINE__);
        return mmc_ret;
    }

    /*4. Set hs400 timing */
    mmc_ret = mmc_switch_cmd(host, card, MMC_ACCESS_WRITE,
                             MMC_EXT_MMC_HS_TIMING, MMC_HS400_TIMING | (1 << 4));

    if (mmc_ret) {
        FATAL("Switch cmd returned failure %s\t%d\n", __func__, __LINE__);
        return mmc_ret;
    }

    /* Enable HS400 mode in controller */
    /* Save the timing value, before changing the clock */
    MMC_SAVE_TIMING(host, MMC_HS400_TIMING);
    /* Set the clock back to 200 MHZ */
    mmc_ret = host->ops->set_clock(host, MMC_CLK_200MHZ);
    if (mmc_ret)
        return mmc_ret;

    host->ops->set_uhs_mode(host, SDHCI_EMMC_HS400_MODE);

    DBG("\n Enabling HS400 Mode Done\n");

    return mmc_ret;
}

/*
 * Function: mmc_host_init
 * Arg     : mmc device structure
 * Return  : 0 on success, 1 on Failure
 * Flow    : Initialize the host contoller
 *           Set the clock rate to 400 KHZ for init
 */
static uint8_t mmc_host_init(struct mmc_device *dev)
{
    uint8_t mmc_ret = 0;

    struct sdhci_host *host;
    struct mmc_config_data *cfg;
    struct sdhci_priv_data *data;

    host = &dev->host;
    cfg = &dev->config;

    host->base = cfg->sdhc_base;
    host->slot = cfg->slot;
    host->irq = cfg->irq;

    data = &sdhci_priv;

    data->slot = cfg->slot;
    data->use_io_switch = cfg->use_io_switch;
    host->caps.hs200_support = cfg->hs200_support;
    host->caps.hs400_support = cfg->hs400_support;
    host->max_clk_rate = cfg->max_clk_rate;
    host->priv_data = data;
    if (cfg->voltage)
        host->caps.voltage = cfg->voltage;

    /*
     * Initialize the controller, read the host capabilities
     * set power on mode
     */
    sdhci_init(host);

    host->ops->config_pin(host);

    return mmc_ret;
}

/*
 * Function: mmc identify card
 * Arg     : host & card structure
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Performs card identification process:
 *           1. Get card's unique identification number (CID)
 *           2. Get(for sd)/set (for mmc) relative card address (RCA)
 *           3. Select the card to put it in TRAN state
 */
static uint32_t mmc_identify_card(struct sdhci_host *host,
                                  struct mmc_card *card)
{
    uint32_t mmc_return = 0;

    /* Ask card to send its unique card identification (CID) number (CMD2) */
    mmc_return = mmc_all_send_cid(host, card);
    if (mmc_return) {
        FATAL("Failure getting card's CID number!\n");
        return mmc_return;
    }

    /* Ask card to send a relative card address (RCA) (CMD3) */
    mmc_return = mmc_send_relative_address(host, card);
    if (mmc_return) {
        FATAL("Failure getting card's RCA!\n");
        return mmc_return;
    }

    /* Get card's CSD register (CMD9) */
    mmc_return = mmc_send_csd(host, card);
    if (mmc_return) {
        FATAL("Failure getting card's CSD information!\n");
        return mmc_return;
    }

    /* Select the card (CMD7) */
    mmc_return = mmc_select_card(host, card);
    if (mmc_return) {
        FATAL("Failure selecting the Card with RCA: %x\n", card->rca);
        return mmc_return;
    }

    /* Set the card status as active */
    card->status = MMC_STATUS_ACTIVE;

    return 0;
}

/*
 * Function: mmc_reset_card_and_send_op
 * Arg     : Host & Card structure
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Routine to initialize MMC card. It resets a card to idle state,
 *           verify operating voltage and set the card in ready state.
 */
static uint32_t mmc_reset_card_and_send_op(struct sdhci_host *host,
                                           struct mmc_card *card)
{
    uint32_t mmc_return = 0;

    /* 1. Card Reset - CMD0, for card powerup need some delay */
    udelay(200);
    mmc_return = mmc_reset_card(host);
    if (mmc_return) {
        FATAL("Failure resetting MMC cards!\n");
        return mmc_return;
    }

    /* 2. Card Initialization process */
    /*
     * Send CMD1 to identify and reject cards that do not match host's VDD range
     * profile. Cards sends its OCR register in response.
     */
    mmc_return = mmc_send_op_cond(host, card);

    /* OCR is not received, init could not complete */
    if (mmc_return) {
        FATAL("Failure getting OCR response from MMC Card\n");
        return mmc_return;
    }

    return 0;
}

static uint32_t mmc_send_app_cmd(struct sdhci_host *host, struct mmc_card *card)
{
    struct mmc_command cmd = {0};

    cmd.cmd_index = CMD55_APP_CMD;
    cmd.argument = (card->rca << 16);
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;

    if (sdhci_send_command(host, &cmd)) {
        FATAL("Failed Sending CMD55\n");
        return 1;
    }
    return 0;
}

uint32_t mmc_sd_card_init(struct sdhci_host *host, struct mmc_card *card)
{
    uint8_t i;
    struct mmc_command cmd;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* Use the SD card RCA 0x0 during init */
    card->rca = SD_CARD_RCA;

    /* Send CMD8 for voltage check*/
    for (i = 0; i < SD_CMD8_MAX_RETRY; i++) {
        cmd.cmd_index = CMD8_SEND_IF_COND;
        cmd.argument = MMC_SD_HC_VOLT_SUPPLIED;
        cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
        cmd.resp_type = SDHCI_CMD_RESP_R7;

        if (sdhci_send_command(host, &cmd)) {
            FATAL("The response for CMD8 does not match the supplied value\n");
            return 1;
        }
        else {
            /* If the command response echos the voltage back */
            if (cmd.resp[0] == MMC_SD_HC_VOLT_SUPPLIED)
                break;
        }

        udelay(1);
    }

    if (i == SD_CMD8_MAX_RETRY && (cmd.resp[0] != MMC_SD_HC_VOLT_SUPPLIED)) {
        FATAL("Error: CMD8 response timed out\n");
        return 1;
    }

    /* Send ACMD41 for OCR */
    for (i = 0; i < SD_ACMD41_MAX_RETRY; i++) {
        /* Send APP_CMD before ACMD41*/
        if (mmc_send_app_cmd(host, card)) {
            FATAL("Failed sending App command\n");
            return 1;
        }

        /* APP_CMD is successful, send ACMD41 now */
        cmd.cmd_index = ACMD41_SEND_OP_COND;
        cmd.argument = MMC_SD_OCR | MMC_SD_HC_HCS;
        cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
        cmd.resp_type = SDHCI_CMD_RESP_R3;

        if (sdhci_send_command(host, &cmd)) {
            FATAL("Failure sending ACMD41\n");
            return 1;
        }
        else {
            if (cmd.resp[0] & MMC_SD_DEV_READY) {
                if (cmd.resp[0] & (1 << 30))
                    card->type = MMC_CARD_TYPE_SDHC;
                else
                    card->type = MMC_CARD_TYPE_STD_SD;

                break;
            }
        }

        udelay(50);
    }

    if (i == SD_ACMD41_MAX_RETRY && !(cmd.resp[0] & MMC_SD_DEV_READY)) {
        FATAL("Error: ACMD41 response timed out\n");
        return 1;
    }

    return 0;
}

/* Function to read SD card information from SD status */
static uint32_t mmc_sd_get_card_ssr(struct sdhci_host *host,
                                    struct mmc_card *card)
{
    uint8_t raw_sd_status[ROUNDUP(64, CACHE_LINE)] __ALIGNED(CACHE_LINE);
    struct mmc_command cmd = {0};
    uint32_t sd_status[16];
    uint32_t *status = sd_status;
    uint32_t au_size;
    int i;
    int j;

    if (mmc_send_app_cmd(host, card)) {
        FATAL("Failed sending App command\n");
        return 1;
    }

    cmd.cmd_index = ACMD13_SEND_SD_STATUS;
    cmd.argument = 0x0;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.trans_mode = SDHCI_MMC_READ;
    cmd.data_present = 0x1;
    cmd.data.data_ptr = raw_sd_status;
    cmd.data.num_blocks = 0x1;
    cmd.data.blk_sz = 0x40;

    /* send command */
    if (sdhci_send_command(host, &cmd))
        return 1;

    memcpy(sd_status, raw_sd_status, sizeof(sd_status));

    for (i = 15, j = 0; i >= 0; i--, j++)
        sd_status[i] = swap_endian32(sd_status[j]);

    au_size = UNPACK_BITS(status, MMC_SD_AU_SIZE_BIT, MMC_SD_AU_SIZE_LEN, 32);
    /* Card AU size in sectors */
    card->ssr.au_size = 1 << (au_size + 4);
    card->ssr.num_aus =
        UNPACK_BITS(status, MMC_SD_ERASE_SIZE_BIT, MMC_SD_ERASE_SIZE_LEN, 32);
    /* When Erase Time-out Calculation is not supported. set the num_aus 1 */
    card->ssr.num_aus = MAX(card->ssr.num_aus, 1);
    card->ssr.erase_timeout =
        UNPACK_BITS(status, MMC_SD_ERASE_TOUT_BIT, MMC_SD_ERASE_TOUT_LEN, 32);
    card->ssr.erase_offset = UNPACK_BITS(status, MMC_SD_ERASE_OFFSET_BIT,
                                         MMC_SD_ERASE_OFFSET_LEN, 32);

    return 0;
}

/* Function to read the SD CARD configuration register */
static uint32_t mmc_sd_get_card_scr(struct sdhci_host *host,
                                    struct mmc_card *card)
{
    uint8_t scr_resp[ROUNDUP(8, CACHE_LINE)] __ALIGNED(CACHE_LINE);
    struct mmc_command cmd = {0};
    uint32_t raw_scr[2];

    /* Now read the SCR register */
    /* Send APP_CMD before ACMD51*/
    if (mmc_send_app_cmd(host, card)) {
        FATAL("Failed sending App command\n");
        return 1;
    }

    cmd.cmd_index = ACMD51_READ_CARD_SCR;
    cmd.argument = 0x0;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.trans_mode = SDHCI_MMC_READ;
    cmd.data_present = 0x1;
    cmd.data.data_ptr = scr_resp;
    cmd.data.num_blocks = 0x1;
    cmd.data.blk_sz = 0x8;

    /* send command */
    if (sdhci_send_command(host, &cmd))
        return 1;

    memcpy(raw_scr, scr_resp, sizeof(raw_scr));

    card->raw_scr[0] = swap_endian32(raw_scr[0]);
    card->raw_scr[1] = swap_endian32(raw_scr[1]);

    /*
     * Parse & Populate the SCR data as per sdcc spec
     */
    card->scr.bus_widths =
        (card->raw_scr[0] & SD_SCR_BUS_WIDTH_MASK) >> SD_SCR_BUS_WIDTH;
    card->scr.cmd23_support = (card->raw_scr[0] & SD_SCR_CMD23_SUPPORT);
    card->scr.sd_spec =
        (card->raw_scr[0] & SD_SCR_SD_SPEC_MASK) >> SD_SCR_SD_SPEC;
    card->scr.sd3_spec =
        (card->raw_scr[0] & SD_SCR_SD_SPEC3_MASK) >> SD_SCR_SD_SPEC3;

    return 0;
}

/*
 * Function: mmc_set_sd_bus_width
 * Arg     : host, device structure & width
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Set the bus width for the card
 */
uint32_t mmc_sd_set_bus_width(struct sdhci_host *host, struct mmc_card *card,
                              uint8_t width)
{
    struct mmc_command cmd = {0};

    /* Send APP_CMD before ACMD6*/
    if (mmc_send_app_cmd(host, card)) {
        FATAL("Failed sending App command\n");
        return 1;
    }

    cmd.cmd_index = ACMD6_SET_BUS_WIDTH;
    cmd.argument = (width == MMC_DATA_BUS_WIDTH_4BIT) ? (1 << 1) : 0;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;

    /* send command */
    if (sdhci_send_command(host, &cmd))
        return 1;

    return 0;
}

uint32_t mmc_sd_set_hs(struct sdhci_host *host, struct mmc_card *card)
{
    struct mmc_command cmd = {0};
    uint8_t switch_resp[ROUNDUP(64, CACHE_LINE)] __ALIGNED(CACHE_LINE);
    cmd.cmd_index = CMD6_SWITCH_FUNC;
    cmd.argument = MMC_SD_SWITCH_HS;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.trans_mode = SDHCI_MMC_READ;
    cmd.data_present = 0x1;
    cmd.data.data_ptr = switch_resp;
    cmd.data.num_blocks = 0x1;
    cmd.data.blk_sz = 0x40;

    /* send command */
    if (sdhci_send_command(host, &cmd))
        return 1;

    /* Set the SDR25 mode in controller*/
    host->ops->set_uhs_mode(host, SDHCI_UHS_SDR25_MODE);
    if (host->ops->set_clock(host, SDHCI_CLK_50MHZ))
        return 1;

    return 0;
}

/*
 * Function: mmc_init_card
 * Arg     : mmc device structure
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Performs initialization and identification of eMMC cards connected
 *           to the host.
 */

static uint32_t mmc_card_init(struct mmc_device *dev)
{
    uint32_t mmc_return = 0;
    uint8_t bus_width = 0;

    struct sdhci_host *host;
    struct mmc_card *card;
    struct mmc_config_data *cfg;

    host = &dev->host;
    card = &dev->card;
    cfg = &dev->config;

    host->card_type = SDHCI_EMMC_CARD;
    /* set card is emmc true for dwcmshc host */
    host->ops->priv_init(host);

    mmc_return = host->ops->set_clock(host, SDHCI_CLK_400KHZ);
    if (mmc_return) {
        FATAL("Failed to set card clk\n");
        return mmc_return;
    }

    /* Initialize MMC card structure */
    card->status = MMC_STATUS_INACTIVE;

    /* If config 1.8v voltage, set ocr 1.7v-1.9v params, else set 2.7v-3.6v */
    card->ocr = MMC_OCR_27_36 | MMC_OCR_SEC_MODE;
    if (host->caps.voltage == SDHCI_VOL_1_8)
        card->ocr = MMC_OCR_17_19 | MMC_OCR_SEC_MODE;

    /* Initialize the internal MMC */
    mmc_return = mmc_reset_card_and_send_op(host, card);

    if (mmc_return) {
        FATAL("MMC card failed to respond, try for SD card\n");
        host->card_type = SDHCI_SD_CARD;
        /* set card is emmc false for dwcmshc host */
        host->ops->priv_init(host);

        /* need reinit clock */
        mmc_return = host->ops->set_clock(host, SDHCI_CLK_400KHZ);
        if (mmc_return) {
            FATAL("Failed to set card clk\n");
            return mmc_return;
        }

        /* Reset the card & get the OCR */
        mmc_return = mmc_sd_card_init(host, card);
        if (mmc_return) {
            FATAL("Failed to initialize SD card\n");
            return mmc_return;
        }
    }

    /* Identify (CMD2, CMD3 & CMD9) and select the card (CMD7) */
    mmc_return = mmc_identify_card(host, card);
    if (mmc_return)
        return mmc_return;

    /* set interface speed */
    if (MMC_CARD_SD(card)) {
        mmc_return = mmc_sd_set_hs(host, card);
        if (mmc_return) {
            FATAL("Failed to set HS for SD card\n");
            return mmc_return;
        }
    }
    else {
        mmc_return = mmc_set_hs_interface(host, card);
        if (mmc_return) {
            FATAL("Error adjusting interface speed!\n");
            return mmc_return;
        }
    }

    /* Now get the extended CSD for the card */
    if (MMC_CARD_MMC(card)) {
        /* For MMC cards, also get the extended csd */
        mmc_return = mmc_get_ext_csd(host, card);

        if (mmc_return) {
            FATAL("Failure getting card's ExtCSD information!\n");
            return mmc_return;
        }
    }
    else {
        /*Read SCR for sd card */
        if (mmc_sd_get_card_scr(host, card)) {
            FATAL("Failure getting card's SCR register\n");
            return 1;
        }
        /* Read SSR for the SD card */
        if (mmc_sd_get_card_ssr(host, card)) {
            FATAL("Failed to get SSR from the card\n");
            return 1;
        }
    }

    /* Decode and save the CSD register */
    mmc_return = mmc_decode_and_save_csd(card);
    if (mmc_return) {
        FATAL("Failure decoding card's CSD information!\n");
        return mmc_return;
    }

    if (MMC_CARD_MMC(card)) {
        /* Set the bus width based on host, target capbilities */
        if (cfg->bus_width == MMC_BUS_WIDTH_8BIT && host->caps.bus_width_8bit)
            bus_width = MMC_DATA_BUS_WIDTH_8BIT;
        /*
         * Host contoller by default supports 4 bit & 1 bit mode.
         * No need to check for host support here
         */
        else if (cfg->bus_width == MMC_BUS_WIDTH_4BIT)
            bus_width = MMC_DATA_BUS_WIDTH_4BIT;
        else
            bus_width = MMC_DATA_BUS_WIDTH_1BIT;

        /* Set 4/8 bit SDR bus width in controller */
        mmc_return = sdhci_set_bus_width(host, bus_width);

        if (mmc_return) {
            FATAL("Failed to set bus width for host controller\n");
            return 1;
        }

        /*
         * Enable high speed mode in the follwing order:
         * 1. HS400 mode if supported by host & card
         * 1. HS200 mode if supported by host & card
         * 2. DDR mode host, if supported by host & card
         * 3. Use normal speed mode with supported bus width
         */
        if (host->caps.hs400_support && mmc_card_supports_hs400_mode(card)) {
            DBG("SDHC Running in HS400 mode\n");
            mmc_return = mmc_set_hs400_mode(host, card, bus_width);
            if (mmc_return) {
                FATAL("Failure to set HS400 mode for Card(RCA:%x)\n", card->rca);
                return mmc_return;
            }
        }
#if USE_TARGET_HS200_CAPS
        else if (host->caps.hs200_support && host->caps.sdr104_support &&
                 mmc_card_supports_hs200_mode(card))
#else
        else if (host->caps.sdr104_support &&
                 mmc_card_supports_hs200_mode(card))
#endif
        {
            DBG("SDHC Running in HS200 mode\n");
            mmc_return = mmc_set_hs200_mode(host, card, bus_width);

            if (mmc_return) {
                FATAL("Failure to set HS200 mode for Card(RCA:%x)\n", card->rca);
                return mmc_return;
            }
        }
        else if (host->caps.ddr_support && mmc_card_supports_ddr_mode(card)) {
            DBG("SDHC Running in DDR mode\n");
            mmc_return = mmc_set_ddr_mode(host, card, bus_width);

            if (mmc_return) {
                FATAL("Failure to set DDR mode for Card(RCA:%x)\n", card->rca);
                return mmc_return;
            }
        }
        else {
            DBG("SDHC Running in High Speed mode\n");
            /* Set HS_TIMING mode */
            mmc_return = mmc_set_hs_interface(host, card);
            if (mmc_return) {
                FATAL("Failure to enalbe HS mode for Card(RCA:%x)\n", card->rca);
                return mmc_return;
            }
            /* Set wide bus mode */
            mmc_return = mmc_set_bus_width(host, card, bus_width);
            if (mmc_return) {
                FATAL("Failure to set wide bus for Card(RCA:%x)\n", card->rca);
                return mmc_return;
            }
        }
    }
    else {
        /* Check the supported bus width for the card from SCR register */
        if (card->scr.bus_widths & SD_SCR_WIDTH_4BIT)
            bus_width = MMC_DATA_BUS_WIDTH_4BIT;
        else
            bus_width = MMC_DATA_BUS_WIDTH_1BIT;

        mmc_return = mmc_sd_set_bus_width(host, card, bus_width);
        if (mmc_return) {
            FATAL("Failed to set bus width for the card\n");
            return mmc_return;
        }

        /* Set bit SDR bus width in controller */
        mmc_return = sdhci_set_bus_width(host, bus_width);
        if (mmc_return) {
            FATAL("Failed to set bus width for host controller\n");
            return mmc_return;
        }
    }

    card->block_size = MMC_BLK_SZ;
#ifndef WITH_ON_ZONE
    if (MMC_CARD_MMC(card)) {
        /* Enable RST_n_FUNCTION */
        if (!card->ext_csd[MMC_EXT_CSD_RST_N_FUNC]) {
            mmc_return =
                mmc_switch_cmd(host, card, MMC_SET_BIT, MMC_EXT_CSD_RST_N_FUNC,
                               RST_N_FUNC_ENABLE);

            if (mmc_return) {
                FATAL("Failed to enable RST_n_FUNCTION\n");
                return mmc_return;
            }
        }
    }
#endif
    return mmc_return;
}

/*
 * Function: mmc display csd
 * Arg     : None
 * Return  : None
 * Flow    : Displays the csd information
 */
static void mmc_display_csd(struct mmc_card *card)
{
    DBG("erase_grpsize: %d\n", card->csd.erase_grp_size);
    DBG("erase_grpmult: %d\n", card->csd.erase_grp_mult);
    DBG("wp_grpsize: %d\n", card->csd.wp_grp_size);
    DBG("wp_grpen: %d\n", card->csd.wp_grp_enable);
    DBG("perm_wp: %d\n", card->csd.perm_wp);
    DBG("temp_wp: %d\n", card->csd.temp_wp);
}

/*
 * Function: mmc_alloc_dev
 * Arg     : None
 * Return  : Pointer to mmc device
 * Flow    : Alloc mmc device memory
 */
struct mmc_device *mmc_alloc_dev(void)
{
    struct mmc_device *dev;

    dev = (struct mmc_device *)malloc(sizeof(struct mmc_device));

    if (!dev) {
        FATAL("Error allocating mmc device\n");
        return NULL;
    }
    memset(dev, 0, sizeof(struct mmc_device));
    return dev;
}

/*
 * Function: mmc_sdhci_init
 * Arg     : mmc device structure
 *           MMC configuration data
 * Return  : 0 on Success, 1 on Failure
 * Flow    : Entry point to MMC boot process
 *           Initialize the sd host controller
 *           Initialize the mmc card
 *           Set the clock & high speed mode
 */
int mmc_sdhci_init(struct mmc_device *dev)
{
    uint8_t mmc_ret = 0;

    assert(dev);

    memset(&dev->card, 0, sizeof(struct mmc_card));

    /* Initialize the host & clock */
    DBG(" Initializing MMC host data structure and clock!\n");

    mmc_ret = mmc_host_init(dev);
    if (mmc_ret) {
        FATAL("Error Initializing MMC host : %u\n", mmc_ret);
        return mmc_ret;
    }

    /* Initialize and identify cards connected to host */
    mmc_ret = mmc_card_init(dev);
    if (mmc_ret) {
        FATAL("Failed detecting MMC/SDC @ slot%d\n", dev->config.slot);
        return mmc_ret;
    }

    DBG("Done initialization of the card\n");

    mmc_display_csd(&dev->card);

    return 0;
}

static uint32_t mmc_parse_response(uint32_t resp)
{
    /* Trying to write beyond card capacity */
    if (resp & MMC_R1_ADDR_OUT_OF_RANGE) {
        FATAL("Attempting to read or write beyond the Device capacity\n");
        return 1;
    }

    /* Misaligned address not matching block length */
    if (resp & MMC_R1_ADDR_ERR) {
        FATAL("The misaligned address did not match the block length used\n");
        return 1;
    }

    /* Invalid block length */
    if (resp & MMC_R1_BLOCK_LEN_ERR) {
        FATAL("The transferred bytes does not match the block length\n");
        return 1;
    }

    /* Tried to program write protected block */
    if (resp & MMC_R1_WP_VIOLATION) {
        FATAL("Attempt to program a write protected block\n");
        return 1;
    }

    /* card controller error */
    if (resp & MMC_R1_CC_ERROR) {
        FATAL("Device error occurred, which is not related to the host command\n");
        return 1;
    }

    /* Generic error */
    if (resp & MMC_R1_GENERIC_ERR) {
        FATAL("A generic Device error\n");
        return 1;
    }

    /* Finally check for card in TRAN state */
    if (MMC_CARD_STATUS(resp) != MMC_TRAN_STATE) {
        FATAL("MMC card is not in TRAN state\n");
        return 1;
    }

    return 0;
}

static uint32_t mmc_stop_command(struct mmc_device *dev)
{
    struct mmc_command cmd;
    uint32_t mmc_ret = 0;

    memset(&cmd, 0, sizeof(struct mmc_command));
    if (dev->host.card_type == SDHCI_SDIO_CARD)
        cmd.cmd_index = CMD52_SDIO_STOP_TRANSMISSION;
    else
        cmd.cmd_index = CMD12_STOP_TRANSMISSION;

    cmd.argument = (dev->card.rca << 16) | BIT(0);
    cmd.cmd_type = SDHCI_CMD_TYPE_ABORT;
    cmd.resp_type = SDHCI_CMD_RESP_R1;

    mmc_ret = sdhci_send_command(&dev->host, &cmd);
    if (mmc_ret) {
        FATAL("Failed to send stop command\n");
        return mmc_ret;
    }

    /* Response contains 32 bit Card status.
     * Parse the errors & provide relevant information */

    return mmc_parse_response(cmd.resp[0]);
}

/*
 * Function: mmc sdhci read
 * Arg     : mmc device structure, block address, number of blocks & destination
 * Return  : 0 on Success, non zero on success
 * Flow    : Fill in the command structure & send the command
 */
uint32_t mmc_sdhci_read(struct mmc_device *dev, void *dest, uint64_t blk_addr,
                        uint32_t num_blocks)
{
    uint32_t mmc_ret = 0;
    struct mmc_command cmd;
    uint32_t status;
    uint32_t retry = 0;
    struct mmc_card *card = &dev->card;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD17/18 Format:
     * [31:0] Data Address
     */
    if (num_blocks == 1)
        cmd.cmd_index = CMD17_READ_SINGLE_BLOCK;
    else
        cmd.cmd_index = CMD18_READ_MULTIPLE_BLOCK;

    /*
     * Standard emmc cards use byte mode addressing
     * convert the block address to byte address before
     * sending the command
     */
    if (card->type == MMC_TYPE_STD_MMC)
        cmd.argument = blk_addr * card->block_size;
    else
        cmd.argument = blk_addr;

    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.trans_mode = SDHCI_MMC_READ;
    cmd.data_present = 0x1;

    /* Use CMD23 If card supports CMD23:
     * For SD card use the value read from SCR register
     * For emmc by default use CMD23.
     * Also as per SDCC spec always use CMD23 to stop
     * multiblock read/write if UHS (Ultra High Speed) is
     * enabled
     */
    if (MMC_CARD_SD(card))
        cmd.cmd23_support = dev->card.scr.cmd23_support;
    else
        cmd.cmd23_support = 0x1;

    cmd.data.data_ptr = dest;
    cmd.data.num_blocks = num_blocks;

    /* send command */
    mmc_ret = sdhci_send_command(&dev->host, &cmd);

    /* For multi block read failures send stop command */
    if (mmc_ret && num_blocks > 1) {
        return mmc_stop_command(dev);
    }

    /*
     * Response contains 32 bit Card status.
     * Parse the errors & provide relevant information
     */
    if(mmc_parse_response(cmd.resp[0]))
        return 1;

    while (1) {
        if (mmc_get_card_status(&dev->host, &dev->card, &status)) {
            FATAL("Failed to get card status after read\n");
            return 1;
        }

        if ((status & MMC_READY_FOR_DATA) &&
             (MMC_CARD_STATUS(status) == MMC_TRAN_STATE))
            break;

        retry++;
        udelay(1000);
        if (retry == MMC_MAX_CARD_STAT_RETRY) {
            FATAL("Card status check timed out after sending read command\n");
            return 1;
        }
    }

    return 0;
}

/*
 * Function: mmc sdhci write
 * Arg     : mmc device structure, block address, number of blocks & source
 * Return  : 0 on Success, non zero on success
 * Flow    : Fill in the command structure & send the command
 */
uint32_t mmc_sdhci_write(struct mmc_device *dev, void *src, uint64_t blk_addr,
                         uint32_t num_blocks)
{
    uint32_t mmc_ret = 0;
    uint32_t status;
    uint32_t retry = 0;
    struct mmc_command cmd;
    struct mmc_card *card = &dev->card;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* CMD24/25 Format:
     * [31:0] Data Address
     */

    if (num_blocks == 1)
        cmd.cmd_index = CMD24_WRITE_SINGLE_BLOCK;
    else
        cmd.cmd_index = CMD25_WRITE_MULTIPLE_BLOCK;

    /*
     * Standard emmc cards use byte mode addressing
     * convert the block address to byte address before
     * sending the command
     */
    if (card->type == MMC_TYPE_STD_MMC)
        cmd.argument = blk_addr * card->block_size;
    else
        cmd.argument = blk_addr;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.trans_mode = SDHCI_MMC_WRITE;

    /* Use CMD23 If card supports CMD23:
     * For SD card use the value read from SCR register
     * For emmc by default use CMD23.
     * Also as per SDCC spec always use CMD23 to stop
     * multiblock read/write if UHS (Ultra High Speed) is
     * enabled
     */
    if (MMC_CARD_SD(card))
        cmd.cmd23_support = dev->card.scr.cmd23_support;
    else
        cmd.cmd23_support = 0x1;

    cmd.data_present = 0x1;
    cmd.data.data_ptr = src;
    cmd.data.num_blocks = num_blocks;

    /* send command */
    mmc_ret = sdhci_send_command(&dev->host, &cmd);

    /* For multi block write failures send stop command */
    if (mmc_ret && num_blocks > 1) {
        return mmc_stop_command(dev);
    }

    /*
     * Response contains 32 bit Card status.
     * Parse the errors & provide relevant information
     */
    if(mmc_parse_response(cmd.resp[0]))
        return 1;

    while (1) {
        if (mmc_get_card_status(&dev->host, &dev->card, &status)) {
            FATAL("Failed to get card status after write\n");
            return 1;
        }

        if ((status & MMC_READY_FOR_DATA) &&
             (MMC_CARD_STATUS(status) == MMC_TRAN_STATE))
            break;

        retry++;
        udelay(1000);
        if (retry == MMC_MAX_CARD_STAT_RETRY) {
            FATAL("Card status check timed out after sending write command\n");
            return 1;
        }
    }

    return 0;
}

/*
 * Send the erase group start address using CMD35
 */
static uint32_t mmc_send_erase_grp_start(struct mmc_device *dev,
                                         uint32_t erase_start)
{
    struct mmc_command cmd;
    struct mmc_card *card = &dev->card;

    memset(&cmd, 0, sizeof(struct mmc_command));

    if (MMC_CARD_MMC(card))
        cmd.cmd_index = CMD35_ERASE_GROUP_START;
    else
        cmd.cmd_index = CMD32_ERASE_WR_BLK_START;

    /*
     * Standard emmc cards use byte mode addressing
     * convert the block address to byte address before
     * sending the command
     */
    if (card->type == MMC_TYPE_STD_MMC)
        cmd.argument = erase_start * card->block_size;
    else
        cmd.argument = erase_start;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;

    /* send command */
    if (sdhci_send_command(&dev->host, &cmd))
        return 1;

    /*
     * CMD35 on failure returns address out of range error
     */
    if (MMC_ADDR_OUT_OF_RANGE(cmd.resp[0])) {
        FATAL("Address for CMD35 is out of range\n");
        return 1;
    }

    return 0;
}

/*
 * Send the erase group end address using CMD36
 */
static uint32_t mmc_send_erase_grp_end(struct mmc_device *dev,
                                       uint32_t erase_end)
{
    struct mmc_command cmd;
    struct mmc_card *card = &dev->card;

    memset(&cmd, 0, sizeof(struct mmc_command));

    if (MMC_CARD_MMC(card))
        cmd.cmd_index = CMD36_ERASE_GROUP_END;
    else
        cmd.cmd_index = CMD33_ERASE_WR_BLK_END;

    /*
     * Standard emmc cards use byte mode addressing
     * convert the block address to byte address before
     * sending the command
     */
    if (card->type == MMC_TYPE_STD_MMC)
        cmd.argument = erase_end * card->block_size;
    else
        cmd.argument = erase_end;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;

    /* send command */
    if (sdhci_send_command(&dev->host, &cmd))
        return 1;

    /*
     * CMD3 on failure returns address out of range error
     */
    if (MMC_ADDR_OUT_OF_RANGE(cmd.resp[0])) {
        FATAL("Address for CMD36 is out of range\n");
        return 1;
    }

    return 0;
}

/*
 * Send the erase CMD38, to erase the selected erase groups
 */
static uint32_t mmc_send_erase(struct mmc_device *dev, uint64_t erase_timeout)
{
    struct mmc_command cmd;
    uint32_t status;
    uint32_t retry = 0;

    memset(&cmd, 0, sizeof(struct mmc_command));

    cmd.cmd_index = CMD38_ERASE;
    cmd.argument = 0x00000000;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1B;
    cmd.cmd_timeout = erase_timeout;

    /* send command */
    if (sdhci_send_command(&dev->host, &cmd))
        return 1;

    do {
        if (mmc_get_card_status(&dev->host, &dev->card, &status)) {
            FATAL("Failed to get card status after erase\n");
            return 1;
        }
        /* Check if the response of erase command has eras skip status set */
        if (status & MMC_R1_WP_ERASE_SKIP)
            FATAL("Write Protect set for the region, only partial space was erased\n");

        retry++;
        udelay(1000);
        if (retry == MMC_MAX_CARD_STAT_RETRY) {
            FATAL("Card status check timed out after sending erase command\n");
            return 1;
        }
    } while (!(status & MMC_READY_FOR_DATA) ||
             (MMC_CARD_STATUS(status) == MMC_PROG_STATE));

    return 0;
}

/*
 * Function: mmc sdhci erase
 * Arg     : mmc device structure, block address and length
 * Return  : 0 on Success, non zero on failure
 * Flow    : Fill in the command structure & send the command
 */
uint32_t mmc_sdhci_erase(struct mmc_device *dev, uint32_t blk_addr,
                         uint64_t len)
{
    uint32_t erase_unit_sz = 0;
    uint32_t erase_start;
    uint32_t erase_end;
    uint32_t blk_end;
    uint32_t num_erase_grps;
    uint64_t erase_timeout = 0;
    struct mmc_card *card;

    card = &dev->card;

    /*
     * Calculate the erase unit size,
     * 1. Based on emmc 4.5 spec for emmc card
     * 2. Use SD Card Status info for SD cards
     */
    if (MMC_CARD_MMC(card)) {
        /*
         * Calculate the erase unit size as per the emmc specification v4.5
         */
        if (dev->card.ext_csd[MMC_ERASE_GRP_DEF])
            erase_unit_sz =
                (MMC_HC_ERASE_MULT * dev->card.ext_csd[MMC_HC_ERASE_GRP_SIZE]) /
                MMC_BLK_SZ;
        else
            erase_unit_sz = (dev->card.csd.erase_grp_size + 1) *
                            (dev->card.csd.erase_grp_mult + 1);
    }
    else
        erase_unit_sz = dev->card.ssr.au_size * dev->card.ssr.num_aus;

    /* Convert length in blocks */
    len = len / MMC_BLK_SZ;

    if (len < erase_unit_sz) {
        FATAL("Requested length is less than min erase group size\n");
        return 1;
    }

    /* Calculate erase groups based on the length in blocks */
    num_erase_grps = len / erase_unit_sz;

    /* Start address of the erase range */
    erase_start = blk_addr;
    if (MMC_CARD_MMC(card))
        /* Last address of the erase range */
        erase_end = blk_addr + ((num_erase_grps - 1) * erase_unit_sz);
    else
        erase_end = blk_addr + len - 1;

    /* Boundary check for overlap */
    blk_end = blk_addr + len;

    if (erase_end > blk_end) {
        FATAL("The erase group overlaps the max requested for erase\n");
        erase_end -= erase_unit_sz;
    }

    /* Send CMD35 for erase group start */
    if (mmc_send_erase_grp_start(dev, erase_start)) {
        FATAL("Failed to send erase grp start address\n");
        return 1;
    }

    /* Send CMD36 for erase group end */
    if (mmc_send_erase_grp_end(dev, erase_end)) {
        FATAL("Failed to send erase grp end address\n");
        return 1;
    }

    if (MMC_CARD_MMC(card)) {
        /*
         * As per emmc 4.5 spec section 7.4.27, calculate the erase timeout
         * erase_timeout = 300ms * ERASE_TIMEOUT_MULT * num_erase_grps
         */
        erase_timeout = (300 * 1000 * card->ext_csd[MMC_ERASE_TIMEOUT_MULT] *
                         num_erase_grps);

    }
    else {
        /*
         * For sd card, calculate the erase timeout
         * erase_timeout = (earse_timeout * num_erase_grps + erase_offset) * 1s
         */
        erase_timeout =
            card->ssr.erase_timeout * num_erase_grps + card->ssr.erase_offset;
        erase_timeout = MAX(erase_timeout, 1);
        erase_timeout *= 1000 * 1000;
        DBG("\n ssr erase_timeout %d, num_erase_grps %d, ssr erase_offset %d\n",
            card->ssr.erase_timeout, num_erase_grps, card->ssr.erase_offset);
    }

    /* Send CMD38 to perform erase */
    if (mmc_send_erase(dev, erase_timeout)) {
        FATAL("Failed to erase the specified partition\n");
        return 1;
    }

    return 0;
}

/*
 * Function: mmc sdhci cancel
 * Arg     : mmc device structure
 * Return  : 0 on Success, non zero on failure
 * Flow    : call mmc_stop_command
 */
inline uint32_t mmc_sdhci_cancel(struct mmc_device *dev)
{
    return mmc_stop_command(dev);
}

/*
 * Function: mmc get wp status
 * Arg     : mmc device structure, block address and buffer for getting wp
 * status Return  : 0 on Success, 1 on Failure Flow    : Get the WP group status
 * by sending CMD31
 */
uint32_t mmc_get_wp_status(struct mmc_device *dev, uint32_t addr,
                           uint8_t *wp_status)
{
    struct mmc_command cmd;

    memset(&cmd, 0, sizeof(struct mmc_command));

    cmd.cmd_index = CMD31_SEND_WRITE_PROT_TYPE;
    cmd.argument = addr;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.trans_mode = SDHCI_MMC_READ;
    cmd.data_present = 0x1;
    cmd.data.data_ptr = wp_status;
    cmd.data.num_blocks = 0x1;
    cmd.data.blk_sz = 0x8;

    if (sdhci_send_command(&dev->host, &cmd)) {
        FATAL("Failed to get status of write protect bits\n");
        return 1;
    }

    return 0;
}

/*
 * Function: mmc set/clear WP on user area
 * Arg     : mmc device structure, block address,len, & flag to set or clear
 * Return  : 0 on success, 1 on failure
 * Flow    : Function to set/clear power on write protect on user area
 */

uint32_t mmc_set_clr_power_on_wp_user(struct mmc_device *dev, uint32_t addr,
                                      uint64_t len, uint8_t set_clr)
{
    struct mmc_command cmd;
    struct mmc_card *card = &dev->card;
    uint32_t wp_grp_size;
    uint32_t status;
    uint32_t num_wp_grps;
    uint32_t ret;
    uint32_t retry = 0;
    uint32_t i;

    memset(&cmd, 0, sizeof(struct mmc_command));

    /* Convert len into blocks */
    len = len / MMC_BLK_SZ;
    wp_grp_size = dev->card.wp_grp_size;

    /* Disable PERM WP */
    ret = mmc_switch_cmd(&dev->host, &dev->card, MMC_SET_BIT, MMC_USR_WP,
                         MMC_US_PERM_WP_DIS);

    if (ret) {
        FATAL("Failed to Disable PERM WP\n");
        return ret;
    }

    /* Read the default values for user WP */
    ret = mmc_get_ext_csd(&dev->host, &dev->card);

    if (ret) {
        FATAL("Failed to read ext csd for the card\n");
        return ret;
    }

    /* Check if user power on WP is disabled or perm WP is enabled */
    if ((dev->card.ext_csd[MMC_USR_WP] & MMC_US_PWR_WP_DIS) ||
        (dev->card.ext_csd[MMC_USR_WP] & MMC_US_PERM_WP_EN)) {
        FATAL("Power on protection is disabled, cannot be set\n");
        return 1;
    }

    if (len < wp_grp_size) {
        FATAL("Length is less than min WP size, WP was not set\n");
        return 1;
    }

    /* Set power on USER WP */
    ret = mmc_switch_cmd(&dev->host, &dev->card, MMC_SET_BIT, MMC_USR_WP,
                         MMC_US_PWR_WP_EN);

    if (ret) {
        FATAL("Failed to set power on WP for user\n");
        return ret;
    }

    num_wp_grps = ROUNDUP(len, wp_grp_size) / wp_grp_size;

    if (set_clr)
        cmd.cmd_index = CMD28_SET_WRITE_PROTECT;
    else
        cmd.cmd_index = CMD29_CLEAR_WRITE_PROTECT;

    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1B;

    for (i = 0; i < num_wp_grps; i++) {
        /*
         * Standard emmc cards use byte mode addressing
         * convert the block address to byte address before
         * sending the command
         */
        if (card->type == MMC_TYPE_STD_MMC)
            cmd.argument = (addr + (i * wp_grp_size)) * card->block_size;
        else
            cmd.argument = addr + (i * wp_grp_size);

        if (sdhci_send_command(&dev->host, &cmd))
            return 1;

        /* CMD28/CMD29 On failure returns address out of range error */
        if (MMC_ADDR_OUT_OF_RANGE(cmd.resp[0])) {
            FATAL("Address for CMD28/29 is out of range\n");
            return 1;
        }

        /* Check the card status */
        do {
            if (mmc_get_card_status(&dev->host, &dev->card, &status)) {
                FATAL("Failed to get card status afterapplying write protect\n");
                return 1;
            }

            /* Time out for WP command */
            retry++;
            udelay(1000);
            if (retry == MMC_MAX_CARD_STAT_RETRY) {
                FATAL("Card status timed out after sending write protect command\n");
                return 1;
            }
        } while (!(status & MMC_READY_FOR_DATA) ||
                 (MMC_CARD_STATUS(status) == MMC_PROG_STATE));
    }

    return 0;
}

/* Function to put the mmc card to sleep */
void mmc_put_card_to_sleep(struct mmc_device *dev)
{
    struct mmc_command cmd = {0};
    struct mmc_card *card = &dev->card;

    cmd.cmd_index = CMD7_SELECT_DESELECT_CARD;
    cmd.argument = 0x00000000;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_NONE;

    /* send command */
    if (sdhci_send_command(&dev->host, &cmd)) {
        FATAL("card deselect error: %s\n", __func__);
        return;
    }

    cmd.cmd_index = CMD5_SLEEP_AWAKE;
    cmd.argument = (card->rca << MMC_CARD_RCA_BIT) | MMC_CARD_SLEEP;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1B;

    /* send command */
    if (sdhci_send_command(&dev->host, &cmd))
        FATAL("card sleep error: %s\n", __func__);
}

/*
 * Switch the partition access type to rpmb or default
 */
uint32_t mmc_sdhci_switch_part(struct mmc_device *dev, uint32_t type)
{
    uint32_t part_access;
    uint32_t ret;
    struct mmc_card *card;

    card = &dev->card;
    if (!MMC_CARD_MMC(card))
        return 1;

    /* Clear the partition access */
    part_access =
        dev->card.ext_csd[MMC_PARTITION_CONFIG] & ~PARTITION_ACCESS_MASK;
    part_access |= type;

    ret = mmc_switch_cmd(&dev->host, &dev->card, MMC_ACCESS_WRITE,
                         MMC_PARTITION_CONFIG, part_access);

    if (ret) {
        FATAL("Failed to switch partition to type: %u\n", type);
        return 1;
    }

    dev->card.ext_csd[MMC_PARTITION_CONFIG] = part_access;
    return 0;
}

static uint32_t mmc_sdhci_set_blk_cnt(struct mmc_device *dev, uint32_t blk_cnt,
                                      uint32_t rel_write)
{
    struct mmc_command cmd = {0};

    cmd.cmd_index = CMD23_SET_BLOCK_COUNT;
    cmd.argument = blk_cnt & 0x0000ffff;
    cmd.argument |= rel_write;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;

    if (sdhci_send_command(&dev->host, &cmd)) {
        FATAL("Set block count failed: %s\n", __func__);
        return 1;
    }

    return 0;
}

uint32_t mmc_sdhci_rpmb_send(struct mmc_device *dev, struct mmc_command *cmd)
{
    int i;
    uint32_t retry = 5;
    uint32_t status;
    uint32_t rel_write = 0;
    uint32_t ret = 1;

    assert(cmd);

    /* 1. Set the partition type to rpmb */
    if (mmc_sdhci_switch_part(dev, PART_ACCESS_RPMB))
        return 1;

    for (i = 0; i < MAX_RPMB_CMDS; i++) {
        if (!cmd[i].cmd_index)
            break;

        if (cmd[i].write_flag == true)
            rel_write = BIT(31);
        else
            rel_write = 0;

        /* 2. Set the block count using cmd23 */
        if (mmc_sdhci_set_blk_cnt(dev, cmd[i].data.num_blocks, rel_write))
            goto err;

        /* 3. Send the command */
        if (sdhci_send_command(&dev->host, &cmd[i]))
            goto err;
        do {
            /* 4. Poll for card status to ensure rpmb operation completeness */
            if (mmc_get_card_status(&dev->host, &dev->card, &status)) {
                FATAL("Failed to get card status after rpmb operations\n");
                goto err;
            }

            retry--;
            udelay(500);
            if (!retry) {
                FATAL("Card status check timed out after rpmb operations\n");
                goto err;
            }
        } while (!(status & MMC_READY_FOR_DATA) ||
                 (MMC_CARD_STATUS(status) == MMC_PROG_STATE));
    }

    /* If we reach here, that means success */
    ret = 0;

err:
    /* 5. Switch the partition back to default type */
    if (mmc_sdhci_switch_part(dev, PART_ACCESS_DEFAULT))
        ret = 1;

    return ret;
}