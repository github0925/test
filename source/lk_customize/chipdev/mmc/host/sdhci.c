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
#include <arch/ops.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <errno.h>
#include <kernel/vm.h>
#include <platform.h>
#include <platform/interrupts.h>
#include <platform/timer.h>
#include <target.h>

#include <lib/reg.h>
#include <mmc_sdhci.h>
#include <res.h>

#include "sdhci.h"

#define SDHCI_TOUT_TIMER_CLK (1000000)
#define MAX_TUNING_LOOP 140
#define udelay(x) spin(x)
#define lower_32_bits(n) ((u32)(n))
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#define IS_CACHE_LINE_ALIGNED(addr) !((addr_t)(addr) & (CACHE_LINE - 1))

#define SDHCI_SUPPORT_INTERRUPT
#define INT_STACK_SZ 4096

#define ADMA_BOUNDARY_SIZE (0x8000000)
#define CALC_BOUNDARY(addr, boundary) ((addr + boundary) & ~(boundary - 1))

static void sdhci_dumpregs(struct sdhci_host *host)
{
    dprintf(CRITICAL,
            "*************** SDHC REG DUMP START *****************\n");
    dprintf(CRITICAL, "Version:      0x%08x\n",
            REG_READ16(host, SDHCI_SPEC_VERSION_REG));
    dprintf(CRITICAL, "Arg2:         0x%08x\t Blk Cnt:      0x%08x\n",
            REG_READ32(host, SDHCI_SDMASA_BLKCNT_REG),
            REG_READ16(host, SDHCI_BLK_CNT_REG));
    dprintf(CRITICAL, "Arg1:         0x%08x\t Blk Sz :      0x%08x\n",
            REG_READ32(host, SDHCI_ARGUMENT_REG),
            REG_READ16(host, SDHCI_BLKSZ_REG));
    dprintf(CRITICAL, "Command:      0x%08x\t Trans mode:   0x%08x\n",
            REG_READ16(host, SDHCI_CMD_REG),
            REG_READ16(host, SDHCI_TRANS_MODE_REG));
    dprintf(CRITICAL, "Resp0:        0x%08x\t Resp1:        0x%08x\n",
            REG_READ32(host, SDHCI_RESP_REG),
            REG_READ32(host, SDHCI_RESP_REG + 0x4));
    dprintf(CRITICAL, "Resp2:        0x%08x\t Resp3:        0x%08x\n",
            REG_READ32(host, SDHCI_RESP_REG + 0x8),
            REG_READ32(host, SDHCI_RESP_REG + 0xC));
    dprintf(CRITICAL, "Prsnt State:  0x%08x\t Host Ctrl1:   0x%08x\n",
            REG_READ32(host, SDHCI_PRESENT_STATE_REG),
            REG_READ8(host, SDHCI_HOST_CTRL1_REG));
    dprintf(CRITICAL, "Timeout ctrl: 0x%08x\t Power Ctrl:   0x%08x\n",
            REG_READ8(host, SDHCI_TIMEOUT_REG),
            REG_READ8(host, SDHCI_PWR_CTRL_REG));
    dprintf(CRITICAL, "Error stat:   0x%08x\t Int Status:   0x%08x\n",
            REG_READ16(host, SDHCI_ERR_INT_STS_REG),
            REG_READ16(host, SDHCI_NRML_INT_STS_REG));
    dprintf(CRITICAL, "Host Ctrl2:   0x%08x\t Clock ctrl:   0x%08x\n",
            REG_READ16(host, SDHCI_HOST_CTRL2_REG),
            REG_READ16(host, SDHCI_CLK_CTRL_REG));
    dprintf(CRITICAL, "Caps1:        0x%08x\t Caps2:        0x%08x\n",
            REG_READ32(host, SDHCI_CAPS_REG1),
            REG_READ32(host, SDHCI_CAPS_REG2));
    dprintf(CRITICAL, "Adma Err:     0x%08x\t Auto Cmd err: 0x%08x\n",
            REG_READ8(host, SDHCI_ADM_ERR_REG),
            REG_READ16(host, SDHCI_AUTO_CMD_ERR));
    dprintf(CRITICAL, "Adma addr1:   0x%08x\t Adma addr2:   0x%08x\n",
            REG_READ32(host, SDHCI_ADM_ADDR_REG),
            REG_READ32(host, SDHCI_ADM_ADDR_REG + 0x4));
}

/*
 * Function: sdhci reset
 * Arg     : Host structure & mask to write to reset register
 * Return  : None
 * Flow:   : Reset the host controller
 */
void sdhci_reset(struct sdhci_host *host, uint8_t mask)
{
    uint32_t reg;
    uint32_t timeout = SDHCI_RESET_MAX_TIMEOUT;
    REG_WRITE8(host, mask, SDHCI_RESET_REG);

    /* Wait for the reset to complete */
    do {
        reg = REG_READ8(host, SDHCI_RESET_REG);
        reg &= mask;

        if (!reg) {
            break;
        }

        if (!timeout) {
            dprintf(CRITICAL, "Error: sdhci reset failed for: %x\n", mask);
            break;
        }

        timeout--;
        thread_sleep(1);
    } while (1);
}

/*
 * Function: sdhci error status enable
 * Arg     : Host structure
 * Return  : None
 * Flow:   : Enable command error status
 */
static void sdhci_error_status_enable(struct sdhci_host *host)
{
    /* Enable all interrupt status */
    REG_WRITE16(host, SDHCI_NRML_INT_STS_EN, SDHCI_NRML_INT_STS_EN_REG);
    REG_WRITE16(host, SDHCI_ERR_INT_STS_EN, SDHCI_ERR_INT_STS_EN_REG);
    /* Enable all interrupt signal */
    REG_WRITE16(host, SDHCI_NRML_INT_SIG_EN, SDHCI_NRML_INT_SIG_EN_REG);
    REG_WRITE16(host, SDHCI_ERR_INT_SIG_EN, SDHCI_ERR_INT_SIG_EN_REG);
}

/*
 * Function: sdhci clock supply
 * Arg     : Host structure
 * Return  : 0 on Success, 1 on Failure
 * Flow:   : 1. Calculate the clock divider
 *           2. Set the clock divider
 *           3. Check if clock stable
 *           4. Enable Clock
 */
uint32_t sdhci_clk_supply(struct sdhci_host *host, uint32_t clk)
{
    uint32_t ret = 0;
    uint16_t clk_ctrl;
    uint32_t div = 0;
    uint32_t freq = 0;
    uint16_t clk_val = 0;
    /* Disable the clock */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    if (clk >= host->caps.base_clk_rate) {
        goto clk_ctrl;
    }

    /* As per the sd spec div should be a multiplier of 2 */
    for (div = 2; div < SDHCI_CLK_MAX_DIV; div += 2) {
        freq = host->caps.base_clk_rate / div;

        if (freq <= clk) {
            break;
        }
    }

    div >>= 1;
clk_ctrl:
    /* As per the sdhci spec 3.0, bits 6-7 of the clock
     * control registers will be mapped to bit 8-9, to
     * support a 10 bit divider value.
     * This is needed when the divider value overflows
     * the 8 bit range.
     */
    clk_val = ((div & SDHCI_SDCLK_FREQ_MASK) << SDHCI_SDCLK_FREQ_SEL);
    clk_val |= ((div & SDHC_SDCLK_UP_BIT_MASK) >> SDHCI_SDCLK_FREQ_SEL)
               << SDHCI_SDCLK_UP_BIT_SEL;
    clk_val |= SDHCI_INT_CLK_EN;
    REG_WRITE16(host, clk_val, SDHCI_CLK_CTRL_REG);
    /* Check for clock stable, timeout 150ms */
    ret =
        sdhci_wait_for_bit(host, SDHCI_CLK_CTRL_REG, SDHCI_CLK_STABLE, 0, 150);

    if (ret) {
        dprintf(CRITICAL, "Error: sdhci clock wait stable timeout!");
        return ret;
    }

    /* Now clock is stable, enable it */
    clk_val = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_val |= SDHCI_CLK_EN;
    REG_WRITE16(host, clk_val, SDHCI_CLK_CTRL_REG);
    host->cur_clk_rate = clk;
    DBG("\n %s: clock_rate: %d clock_div:0x%08x\n", __func__, clk, div);
    return ret;
}

/*
 * Function: sdhci stop sdcc clock
 * Arg     : Host structure
 * Return  : 0 on Success, 1 on Failure
 * Flow:   : 1. Stop the clock
 */
static uint32_t sdhci_stop_sdcc_clk(struct sdhci_host *host)
{
    uint32_t reg;
    reg = REG_READ32(host, SDHCI_PRESENT_STATE_REG);

    if (reg & (SDHCI_CMD_ACT | SDHCI_DAT_ACT)) {
        dprintf(CRITICAL, "Error: SDCC command & data line are active\n");
        return 1;
    }

    REG_WRITE16(host, SDHCI_CLK_DIS, SDHCI_CLK_CTRL_REG);
    return 0;
}

/*
 * Function: sdhci change frequency
 * Arg     : Host structure & clock value
 * Return  : 0 on Success, 1 on Failure
 * Flow:   : 1. Stop the clock
 *           2. Star the clock with new frequency
 */
static uint32_t sdhci_change_freq_clk(struct sdhci_host *host, uint32_t clk)
{
    if (sdhci_stop_sdcc_clk(host)) {
        dprintf(CRITICAL, "Error: Card is busy, cannot change frequency\n");
        return 1;
    }

    if (sdhci_clk_supply(host, clk)) {
        dprintf(CRITICAL, "Error: cannot change frequency\n");
        return 1;
    }

    return 0;
}

/*
 * Function: sdhci set bus power
 * Arg     : Host structure
 * Return  : None
 * Flow:   : 1. Set the voltage
 *           2. Set the sd power control register
 */
static void sdhci_set_bus_power_on(struct sdhci_host *host)
{
    uint16_t ctrl;
    uint8_t voltage;
    voltage = host->caps.voltage;
    voltage <<= SDHCI_BUS_VOL_SEL;
    REG_WRITE8(host, voltage, SDHCI_PWR_CTRL_REG);
    voltage |= SDHCI_BUS_PWR_EN;
    REG_WRITE8(host, voltage, SDHCI_PWR_CTRL_REG);
    /* switch signal voltage by host ctrl2 register */
    ctrl = REG_READ16(host, SDHCI_HOST_CTRL2_REG);

    if (host->caps.voltage < SDHCI_VOL_3_3) {
        ctrl |= SDHCI_1_8_VOL_SET;
    }
    else {
        ctrl &= ~SDHCI_1_8_VOL_SET;
    }

    REG_WRITE16(host, ctrl, SDHCI_HOST_CTRL2_REG);
    DBG("\n %s: voltage: 0x%02x; host ctrl2: 0x%04x\n", __func__, voltage,
        ctrl);
}

/*
 * Function: sdhci set uhs mode, needs to be used together with set clock api
 * Arg     : Host structure, UHS mode
 * Return  : None
 * Flow:   : 1. Disable the clock
 *           2. Enable UHS mode
 *           3. Enable the clock
 * Details :
 */
void sdhci_set_uhs_mode(struct sdhci_host *host, uint32_t mode)
{
    uint8_t ctrl = 0;
    uint16_t ctrl2 = 0;
    uint16_t clk = 0;

    /* Disable the card clock */
    clk = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk, SDHCI_CLK_CTRL_REG);

    ctrl = REG_READ8(host, SDHCI_HOST_CTRL1_REG);
    if (mode > SDHCI_UHS_SDR12_MODE)
        ctrl |= SDHCI_HIGH_SPEED_EN;
    else
        ctrl &= ~SDHCI_HIGH_SPEED_EN;
    REG_WRITE8(host, ctrl, SDHCI_HOST_CTRL1_REG);

    /* sets the uhs mode */
    ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
    ctrl2 &= ~SDHCI_UHS_MODE_MASK;
    ctrl2 |= mode & SDHCI_UHS_MODE_MASK;
    REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
}

/*
 * Function: sdhci set adma mode
 * Arg     : Host structure
 * Return  : None
 * Flow:   : Set adma mode
 */
static void sdhci_set_adma_mode(struct sdhci_host *host)
{
    uint8_t ctrl1;
    uint16_t ctrl2;
    ctrl1 = REG_READ8(host, SDHCI_HOST_CTRL1_REG);

    if (host->caps.spec_version >= SDHCI_SPEC_VER4_NUM) {
        ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
        ctrl2 |= SDHCI_VER4_ENABLE;

        if (host->caps.addr_64bit_v4) {
            ctrl2 |= SDHCI_ADMA_64BIT_V4;
        }
        else {
            ctrl2 &= ~SDHCI_ADMA_64BIT_V4;
        }

        ctrl2 |= SDHCI_ADMA2_26BIT_LEN_MODE;
        REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
        /* Select ADMA2 type */
        ctrl1 |= SDHCI_ADMA2_SEL;
    }
    else {
        /* Select ADMA2 type, 64bit address */
        ctrl1 |= SDHCI_ADMA_64BIT;
    }

    REG_WRITE8(host, ctrl1, SDHCI_HOST_CTRL1_REG);
}

/*
 * Function: sdhci set bus width
 * Arg     : Host & width
 * Return  : 0 on Sucess, 1 on Failure
 * Flow:   : Set the bus width for controller
 */
uint8_t sdhci_set_bus_width(struct sdhci_host *host, uint16_t width)
{
    uint16_t reg = 0;
    reg = REG_READ8(host, SDHCI_HOST_CTRL1_REG);

    switch (width) {
        case MMC_DATA_BUS_WIDTH_8BIT:
            width = SDHCI_BUS_WITDH_8BIT;
            break;
        case MMC_DATA_BUS_WIDTH_4BIT:
            width = SDHCI_BUS_WITDH_4BIT;
            break;
        case MMC_DATA_BUS_WIDTH_1BIT:
            width = SDHCI_BUS_WITDH_1BIT;
            break;

        default:
            dprintf(CRITICAL, "Bus width is invalid: %u\n", width);
            return 1;
    }

    DBG("\n %s: bus width:0x%04x\n", __func__, width);
    REG_WRITE8(host, (reg | width), SDHCI_HOST_CTRL1_REG);
    return 0;
}

/*
 * Function: sdhci command err status
 * Arg     : Host structure
 * Return  : 0 on Sucess, 1 on Failure
 * Flow:   : Look for error status
 */
static uint8_t sdhci_cmd_err_status(struct sdhci_host *host)
{
    uint32_t err;
    err = REG_READ16(host, SDHCI_ERR_INT_STS_REG);

    if (err & SDHCI_CMD_TIMEOUT_MASK) {
        dprintf(CRITICAL, "Error: Command timeout error\n");
        return 1;
    }
    else if (err & SDHCI_CMD_CRC_MASK) {
        dprintf(CRITICAL, "Error: Command CRC error\n");
        return 1;
    }
    else if (err & SDHCI_CMD_END_BIT_MASK) {
        dprintf(CRITICAL, "Error: CMD end bit error\n");
        return 1;
    }
    else if (err & SDHCI_CMD_IDX_MASK) {
        dprintf(CRITICAL, "Error: Command Index error\n");
        return 1;
    }
    else if (err & SDHCI_DAT_TIMEOUT_MASK) {
        dprintf(CRITICAL, "Error: DATA time out error\n");
        return 1;
    }
    else if (err & SDHCI_DAT_CRC_MASK) {
        dprintf(CRITICAL, "Error: DATA CRC error\n");
        return 1;
    }
    else if (err & SDHCI_DAT_END_BIT_MASK) {
        dprintf(CRITICAL, "Error: DATA end bit error\n");
        return 1;
    }
    else if (err & SDHCI_CUR_LIM_MASK) {
        dprintf(CRITICAL, "Error: Current limit error\n");
        return 1;
    }
    else if (err & SDHCI_AUTO_CMD12_MASK) {
        dprintf(CRITICAL, "Error: Auto CMD12 error\n");
        return 1;
    }
    else if (err & SDHCI_ADMA_MASK) {
        dprintf(CRITICAL, "Error: ADMA error\n");
        return 1;
    }

    return 0;
}

/*
 * Function: sdhci cmd abort for error recovery
 * Arg     : Host & command structure
 * Return  : None
 * Flow:   : 1. Send abort command, the cmd type must be abort type
 */
static void sdhci_cmd_abort(struct sdhci_host *host)
{
    struct mmc_command cmd;
    memset(&cmd, 0, sizeof(struct mmc_command));
    /* Send CMD12 for abort transfer*/
    cmd.cmd_index = CMD12_STOP_TRANSMISSION;
    cmd.argument = SD_CARD_RCA << 16;
    cmd.cmd_type = SDHCI_CMD_TYPE_ABORT;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    sdhci_send_command(host, &cmd);
    return;
}

/*
 * Function: sdhci command complete
 * Arg     : Host & command structure
 * Return  : 0 on Sucess, 1 on Failure
 * Flow:   : 1. Check for command complete
 *           2. Check for transfer complete
 *           3. Get the command response
 *           4. Check for errors & error recovery
 */
static uint8_t sdhci_cmd_complete(struct sdhci_host *host,
                                  struct mmc_command *cmd)
{
    uint8_t i;
    uint8_t ret = 0;
    uint8_t need_reset = 0;
    uint64_t retry = 0;
    uint32_t int_status;
    uint32_t trans_complete = 0;
    uint32_t err_status;
    uint64_t max_trans_retry =
        (cmd->cmd_timeout ? cmd->cmd_timeout : SDHCI_MAX_TRANS_RETRY);

    do {
        int_status = REG_READ16(host, SDHCI_NRML_INT_STS_REG);

        if (int_status & SDHCI_INT_STS_CMD_COMPLETE) {
            break;
        }
        else if (int_status & SDHCI_ERR_INT_STAT_MASK &&
                 !host->tuning_in_progress) {
            goto err;
        }

        /*
         * If Tuning is in progress ignore cmd crc, cmd timeout & cmd end bit
         * errors
         */
        if (host->tuning_in_progress) {
            err_status = REG_READ16(host, SDHCI_ERR_INT_STS_REG);

            if ((err_status & SDHCI_CMD_CRC_MASK) ||
                (err_status & SDHCI_CMD_END_BIT_MASK) ||
                err_status & SDHCI_CMD_TIMEOUT_MASK) {
                sdhci_reset(host, (SOFT_RESET_CMD | SOFT_RESET_DATA));
                return 0;
            }
            /*
             * According sdhci v4.2 spec,
             * the tuning cmd is not need wait for cmd complete.
             */
            return 0;
        }

        retry++;
        udelay(1);

        if (retry == SDHCI_MAX_CMD_RETRY) {
            dprintf(CRITICAL, "Error: Command never completed\n");
            ret = 1;
            goto err;
        }
    } while (1);

    /* Command is complete, clear the interrupt bit */
    REG_WRITE16(host, SDHCI_INT_STS_CMD_COMPLETE, SDHCI_NRML_INT_STS_REG);

    /* Copy the command response,
     * The valid bits for R2 response are 0-119, & but the actual response
     * is stored in bits 8-128. We need to move 8 bits of MSB of each
     * response to register 8 bits of LSB of next response register.
     * As:
     * MSB 8 bits of RESP0 --> LSB 8 bits of RESP1
     * MSB 8 bits of RESP1 --> LSB 8 bits of RESP2
     * MSB 8 bits of RESP2 --> LSB 8 bits of RESP3
     */
    if (cmd->resp_type == SDHCI_CMD_RESP_R2) {
        for (i = 0; i < 4; i++) {
            cmd->resp[i] = REG_READ32(host, SDHCI_RESP_REG + (i * 4));
            cmd->resp[i] <<= SDHCI_RESP_LSHIFT;

            if (i != 0)
                cmd->resp[i] |=
                    (REG_READ32(host, SDHCI_RESP_REG + ((i - 1) * 4)) >>
                     SDHCI_RESP_RSHIFT);
        }
    }
    else {
        cmd->resp[0] = REG_READ32(host, SDHCI_RESP_REG);
    }

    retry = 0;

    /*
     * Clear the transfer complete interrupt
     */
    if (cmd->data_present || cmd->resp_type == SDHCI_CMD_RESP_R1B) {
        do {
            int_status = REG_READ16(host, SDHCI_NRML_INT_STS_REG);

            if (int_status & SDHCI_INT_STS_TRANS_COMPLETE) {
                /* here will failed*/
                trans_complete = 1;
                break;
            }
            /*
             * Some controllers set the data timout first on issuing an erase &
             * take time to set data complete interrupt. We need to wait hoping
             * the controller would set data complete
             */
            else if (int_status & SDHCI_ERR_INT_STAT_MASK &&
                     !host->tuning_in_progress &&
                     !((REG_READ16(host, SDHCI_ERR_INT_STS_REG) &
                        SDHCI_DAT_TIMEOUT_MASK))) {
                goto err;
            }

            /*
             * If we are in tuning then we need to wait until Data timeout ,
             * Data end or Data CRC error
             */
            if (host->tuning_in_progress) {
                err_status = REG_READ16(host, SDHCI_ERR_INT_STS_REG);

                if ((err_status & SDHCI_DAT_TIMEOUT_MASK) ||
                    (err_status & SDHCI_DAT_CRC_MASK)) {
                    sdhci_reset(host, (SOFT_RESET_CMD | SOFT_RESET_DATA));
                    return 0;
                }
            }

            retry++;
            udelay(1);

            if (retry == max_trans_retry) {
                dprintf(CRITICAL, "Error: Transfer never completed\n");
                sdhci_dumpregs(host);
                ret = 1;
                goto err;
            }
        } while (1);

        /* Transfer is complete, clear the interrupt bit */
        REG_WRITE16(host, SDHCI_INT_STS_TRANS_COMPLETE, SDHCI_NRML_INT_STS_REG);
    }

err:
    /* Look for errors */
    int_status = REG_READ16(host, SDHCI_NRML_INT_STS_REG);
    err_status = REG_READ16(host, SDHCI_ERR_INT_STS_REG);

    if (int_status & SDHCI_ERR_INT_STAT_MASK) {
        /* Reset Command & Dat lines on error */
        need_reset = 1;

        /*
         * As per SDHC spec transfer complete has higher priority than data
         * timeout If both transfer complete & data timeout are set then we
         * should ignore data timeout error.
         * --------------------------------------------------------------------
         * | Transfer complete | Data timeout | Meaning of the Status          |
         * |-------------------------------------------------------------------|
         * |      0            |       0      | Interrupted by another factor  |
         * |-------------------------------------------------------------------|
         * |      0            |       1      | Timeout occured during transfer|
         * |-------------------------------------------------------------------|
         * |      1            |  Don't Care  | Command execution complete     |
         *  -------------------------------------------------------------------
         */
        if ((err_status & SDHCI_DAT_TIMEOUT_MASK) && trans_complete) {
            ret = 0;
        }
        else if (cmd->cmd_type == SDHCI_CMD_TYPE_ABORT) {
            need_reset = 0;
            REG_WRITE16(host, err_status, SDHCI_ERR_INT_STS_REG);
        }
        else if (sdhci_cmd_err_status(host)) {
            ret = 1;
            /* Dump sdhc registers on error */
            sdhci_dumpregs(host);
        }
    }

    /* Reset data & command line */
    if (need_reset) {
        /*
         * Reference synopsys's dwcmsch user guide doc 1.70a:
         * need issue DAT line reset when issuing CMD line reset.
         */
        sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
        REG_WRITE16(host, err_status, SDHCI_ERR_INT_STS_REG);

        /* Send abort cmd only when data present */
        if (cmd->data_present) {
            sdhci_cmd_abort(host);
        }
    }

    return ret;
}

static void _sdhci_adma_write_desc(struct sdhci_host *host, void **desc,
                                   addr_t addr, int len, unsigned int attr)
{
    struct desc_entry_a64 *dma_desc = *desc;

    dma_desc->addr_l = lower_32_bits((addr_t)addr);
    dma_desc->addr_h = upper_32_bits((addr_t)addr);
    dma_desc->tran_att = attr | FV_ADMA2_ATTR_LEN(len);

    *desc += sizeof(struct desc_entry_a64);
}

static void sdhci_adma_write_desc(struct sdhci_host *host, void **desc,
                                  addr_t addr, int len, unsigned int attr)
{
    int offset;
    addr_t boundary_addr;

    boundary_addr = CALC_BOUNDARY(addr, ADMA_BOUNDARY_SIZE);

    if ((addr + len) > boundary_addr) {
        offset = boundary_addr - addr;
        _sdhci_adma_write_desc(host, desc, addr, offset, attr);
        addr += offset;
        len -= offset;
    }

    _sdhci_adma_write_desc(host, desc, addr, len, attr);
}

/*
 * Function: sdhci prep desc table
 * Arg     : Pointer data & length
 * Return  : Pointer to desc table
 * Flow:   : Prepare the adma table as per the sd spec v 4.0
 */
static struct desc_entry_a64 *sdhci_prep_desc_table(struct sdhci_host *host,
                                                    paddr_t data, uint32_t len)
{
    uint32_t sg_len = 0;
    uint32_t remain = 0;
    uint32_t i;
    uint32_t table_len = 0;
    uint32_t desc_len_max = 0;
    uint32_t tran_len = 0;
    uint32_t adma_attr = 0;
    struct desc_entry_a64 *sg_list = NULL;
    struct desc_entry_a64 *desc = NULL;

    if (host->caps.addr_64bit_v4) {
        desc_len_max = SDHCI_ADMA2_DESC_LINE_SZ_V4;
    }
    else {
        desc_len_max = SDHCI_ADMA_DESC_LINE_SZ;
    }

    /* Calculate the number of entries in desc table */
    sg_len = len / desc_len_max;
    remain = len - (sg_len * desc_len_max);

    /* Allocate sg_len + 1 entries if there are remaining bytes at the end
        */
    if (remain) {
        sg_len++;
    }

    /* For workaround the snps adma boundary, alloc one more desc entry */
    table_len = ((sg_len + 1) * sizeof(struct desc_entry_a64));
    sg_list = memalign(CACHE_LINE, ROUNDUP(table_len, CACHE_LINE));

    if (!sg_list) {
        dprintf(CRITICAL, "Error allocating memory\n");
        ASSERT(0);
    }

    memset(sg_list, 0, table_len);
    desc = sg_list;

    for (i = 0; i < sg_len; i++) {
        tran_len = MIN(len, desc_len_max);
        adma_attr = SDHCI_ADMA_DATA_VALID;
        sdhci_adma_write_desc(host, (void **)&desc, data, tran_len, adma_attr);
        data += tran_len;
        len -= tran_len;
    }

    /* Add the last entry - nop, end, valid */
    adma_attr = SDHCI_ADMA_NOP_END_VALID;
    sdhci_adma_write_desc(host, (void **)&desc, 0, 0, adma_attr);

    arch_clean_invalidate_cache_range((addr_t)sg_list,
                                      ROUNDUP(table_len, CACHE_LINE));

    for (i = 0; i < sg_len + 1; i++) {
        DBG("\n %s: sg_list: addr_l: 0x%x addr_h: 0x%x attr: 0x%x\n", __func__,
            sg_list[i].addr_l, sg_list[i].addr_h, sg_list[i].tran_att);
    }

    return sg_list;
}

/*
 * Function: sdhci adma transfer
 * Arg     : Host structure & command stucture
 * Return  : Pointer to desc table
 * Flow    : 1. Prepare descriptor table
 *           2. Write adma register
 *           3. Write block size & block count register
 */
static struct desc_entry_a64 *sdhci_adma_transfer(struct sdhci_host *host,
                                                  struct mmc_command *cmd)
{
    uint32_t num_blks = 0;
    uint32_t sz;
    paddr_t data;
    struct desc_entry_a64 *adma_desc;
    paddr_t adma_desc_paddr;
    num_blks = cmd->data.num_blocks;
    data = p2ap((paddr_t)_paddr(cmd->data.data_ptr));

    /*
     * Some commands send data on DAT lines which is less
     * than SDHCI_MMC_BLK_SZ, in that case trying to read
     * more than the data sent by the card results in data
     * CRC errors. To avoid such errors allow data to pass
     * the required block size, if the block size is not
     * passed use the default value
     */
    if (cmd->data.blk_sz) {
        sz = num_blks * cmd->data.blk_sz;
    }
    else {
        sz = num_blks * SDHCI_MMC_BLK_SZ;
    }

    /* Prepare adma descriptor table */
    adma_desc = sdhci_prep_desc_table(host, data, sz);
    adma_desc_paddr = p2ap((paddr_t)_paddr(adma_desc));
    /* Write adma address to adma register */
    REG_WRITE32(host, (uint32_t)adma_desc_paddr, SDHCI_ADM_ADDR_REG);
    REG_WRITE32(host, upper_32_bits(adma_desc_paddr), SDHCI_ADM_ADDR_REG + 0x4);

    /* Write the block size */
    if (cmd->data.blk_sz) {
        REG_WRITE16(host, cmd->data.blk_sz, SDHCI_BLKSZ_REG);
    }
    else {
        REG_WRITE16(host, SDHCI_MMC_BLK_SZ, SDHCI_BLKSZ_REG);
    }

    /*
     * Set block count in block count register
     */
    if (host->caps.spec_version >= SDHCI_SPEC_VER4_NUM) {
        REG_WRITE32(host, num_blks, SDHCI_SDMASA_BLKCNT_REG);
    }
    else {
        REG_WRITE16(host, num_blks, SDHCI_BLK_CNT_REG);
    }

    return adma_desc;
}

static void sdhci_pre_xfer(struct sdhci_host *host)
{
    REG_WRITE16(host, 0xFFFF, SDHCI_NRML_INT_STS_REG);
    REG_WRITE16(host, 0xFFFF, SDHCI_ERR_INT_STS_REG);
}

static inline bool sdhci_data_line_cmd(struct mmc_command *cmd)
{
    return cmd->data_present || cmd->resp_type & SDHCI_CMD_RESP_R1B;
}

static inline uint16_t sdhci_cale_timeout(uint64_t us, uint32_t freq)
{
    uint16_t i = 0;
    uint64_t t;

    for (i = 0; i < 0xF; i++) {
        t = 1 << (13 + i);

        if (t / freq * 1000 * 1000 > us) {
            break;
        }
    }

    return MIN(i, 0xE);
}

/*
 * Function: sdhci send command
 * Arg     : Host structure & command stucture
 * Return  : 0 on Success, 1 on Failure
 * Flow:   : 1. Prepare the command register
 *           2. If data is present, prepare adma table
 *           3. Run the command
 *           4. Check for command results & take action
 */
uint32_t sdhci_send_command(struct sdhci_host *host, struct mmc_command *cmd)
{
    uint32_t ret = 0;
    uint8_t retry = 0;
    uint32_t resp_type = 0;
    uint16_t trans_mode = 0;
    uint16_t present_state;
    uint32_t flags;
    uint16_t mask;
    uint8_t timeout;
    DBG("\n %s: START: cmd:%04d, arg:0x%08x, resp_type:0x%04x, "
        "data_present:%d\n",
        __func__, cmd->cmd_index, cmd->argument, cmd->resp_type,
        cmd->data_present);
    cmd->error = 0;

    if (cmd->data_present) {
        ASSERT(cmd->data.data_ptr);
        ASSERT(IS_CACHE_LINE_ALIGNED(cmd->data.data_ptr));
    }

    do {
        mask = SDHCI_STATE_CMD_MASK;

        if (sdhci_data_line_cmd(cmd)) {
            mask |= SDHCI_STATE_DAT_MASK;
        }

        present_state = REG_READ32(host, SDHCI_PRESENT_STATE_REG);
        /* check if CMD & DAT lines are free */
        present_state &= mask;

        if (!present_state) {
            break;
        }

        thread_sleep(1);
        retry++;

        if (retry == 1000) {
            dprintf(CRITICAL, "Error: CMD or DAT lines were never freed\n");
            return 1;
        }
    } while (1);

    event_init(&host->cmd_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    host->cmd = cmd;

    sdhci_pre_xfer(host);

    switch (cmd->resp_type) {
        case SDHCI_CMD_RESP_R1:
        case SDHCI_CMD_RESP_R3:
        case SDHCI_CMD_RESP_R6:
        case SDHCI_CMD_RESP_R7:
            /* Response of length 48 have 32 bits
             * of response data stored in RESP0[0:31]
             */
            resp_type = SDHCI_CMD_RESP_48;
            break;

        case SDHCI_CMD_RESP_R2:
            /* Response of length 136 have 120 bits
             * of response data stored in RESP0[0:119]
             */
            resp_type = SDHCI_CMD_RESP_136;
            break;

        case SDHCI_CMD_RESP_R1B:
            /* Response of length 48 have 32 bits
             * of response data stored in RESP0[0:31]
             * & set CARD_BUSY status if card is busy
             */
            resp_type = SDHCI_CMD_RESP_48_BUSY;
            break;

        case SDHCI_CMD_RESP_NONE:
            resp_type = SDHCI_CMD_RESP_NONE;
            break;

        default:
            dprintf(CRITICAL, "Invalid response type for the command\n");
            return 1;
    };

    flags = (resp_type << SDHCI_CMD_RESP_TYPE_SEL_BIT);
    if (cmd->data_present || cmd->cmd_index == CMD21_SEND_TUNING_BLOCK)
        flags |= (1 << SDHCI_CMD_DATA_PRESENT_BIT);
    flags |= (cmd->cmd_type << SDHCI_CMD_CMD_TYPE_BIT);

    /* Enable Command CRC & Index check for commands with response
     * R1, R6, R7 & R1B. Also only CRC check for R2 response
     */
    switch (cmd->resp_type) {
        case SDHCI_CMD_RESP_R1:
        case SDHCI_CMD_RESP_R6:
        case SDHCI_CMD_RESP_R7:
        case SDHCI_CMD_RESP_R1B:
            flags |=
                (1 << SDHCI_CMD_CRC_CHECK_BIT) | (1 << SDHCI_CMD_IDX_CHECK_BIT);
            break;

        case SDHCI_CMD_RESP_R2:
            flags |= (1 << SDHCI_CMD_CRC_CHECK_BIT);
            break;

        default:
            break;
    };

    /* Set the timeout value */
#ifdef SDHCI_SUPPORT_INTERRUPT
    if (cmd->cmd_timeout) {
        timeout = sdhci_cale_timeout(cmd->cmd_timeout, SDHCI_TOUT_TIMER_CLK);
    }
    else {
        timeout = SDHCI_CMD_TIMEOUT;
    }

#else
    timeout = 0xE;
#endif
    REG_WRITE8(host, timeout, SDHCI_TIMEOUT_REG);

    /* Check if data needs to be processed */
    if (cmd->data_present) {
        /*
         * clean any stale cache lines ensure cpu data buffer flush in memory,
         * and forbid cpu write back cache on unpredictable time.
         */
        arch_clean_invalidate_cache_range(
            (addr_t)cmd->data.data_ptr,
            (cmd->data.blk_sz) ? (cmd->data.num_blocks * cmd->data.blk_sz)
                               : (cmd->data.num_blocks * SDHCI_MMC_BLK_SZ));
        cmd->data.sg_list = sdhci_adma_transfer(host, cmd);
        memcpy(&host->data_cmd, cmd, sizeof(struct mmc_command));
    }

    /* Write the argument 1 */
    REG_WRITE32(host, cmd->argument, SDHCI_ARGUMENT_REG);

    if (cmd->trans_mode == SDHCI_MMC_READ)
        trans_mode |= SDHCI_READ_MODE;

    /* Set the Transfer mode */
    if (cmd->data_present) {
        /* Enable DMA */
        trans_mode |= SDHCI_DMA_EN;

        /*
         * Enable auto cmd23 or cmd12 for multi block transfer
         * based on what command card supports
         */
        if (cmd->data.num_blocks > 1) {
            if (cmd->cmd23_support) {
                trans_mode |=
                    SDHCI_TRANS_MULTI | SDHCI_AUTO_CMD23_EN | SDHCI_BLK_CNT_EN;
                REG_WRITE32(host, cmd->data.num_blocks,
                            SDHCI_SDMASA_BLKCNT_REG);
            }
            else
                trans_mode |=
                    SDHCI_TRANS_MULTI | SDHCI_AUTO_CMD12_EN | SDHCI_BLK_CNT_EN;
        }
    }

    /* Write to transfer mode register */
    REG_WRITE16(host, trans_mode, SDHCI_TRANS_MODE_REG);
    /* Write the command register */
    REG_WRITE16(host, SDHCI_PREP_CMD(cmd->cmd_index, flags), SDHCI_CMD_REG);
#ifdef SDHCI_SUPPORT_INTERRUPT
    if (!host->tuning_in_progress)
        event_wait(&host->cmd_event);

    ret = host->cmd->error;

    if (ret) {
        sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);

        if (cmd->resp_type == SDHCI_CMD_RESP_R1B) {
            goto cmd_out;
        }
    }

    /*
     * If has data and sync mode, or cmd has busy status bit,
     * need wait for data complete signal.
     */
    if ((host->data_cmd.data_present && !host->async_mode) ||
        cmd->resp_type == SDHCI_CMD_RESP_R1B) {
        if (!host->tuning_in_progress)
            event_wait(&host->data_complete_event);

        if (host->data_cmd.data_present) {
            ret = host->data_cmd.error;
            host->data_cmd.data_present = 0;
        }
        else {
            ret = host->cmd->error;
        }
    }

#else

    /* Command complete sequence */
    if (sdhci_cmd_complete(host, cmd)) {
        ret = 1;
        goto err;
    }

    /* Invalidate the cache only for read operations */
    if (cmd->trans_mode == SDHCI_MMC_READ) {
        /* Read can be performed on block size < SDHCI_MMC_BLK_SZ, make sure to
         * flush the data only for the read size instead
         */
        arch_invalidate_cache_range(
            (addr_t)cmd->data.data_ptr,
            (cmd->data.blk_sz) ? (cmd->data.num_blocks * cmd->data.blk_sz)
                               : (cmd->data.num_blocks * SDHCI_MMC_BLK_SZ));
    }

err:

    /* Free the scatter/gather list */
    if (cmd->data.sg_list) {
        free(cmd->data.sg_list);
    }

#endif
cmd_out:
    host->cmd = NULL;
    DBG("\n %s: END: host:0x%lx cmd:%04d, arg:0x%08x, resp:0x%08x 0x%08x "
        "0x%08x 0x%08x\n",
        __func__, host->base, cmd->cmd_index, cmd->argument, cmd->resp[0],
        cmd->resp[1], cmd->resp[2], cmd->resp[3]);
    return ret;
}

static void sdhci_start_tuning(struct sdhci_host *host)
{
    uint16_t ctrl2;
    ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
    ctrl2 |= SDHCI_EXEC_TUNING;
    REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
}

static void sdhci_abort_tuning(struct sdhci_host *host)
{
    uint16_t ctrl2;
    ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
    ctrl2 &= ~SDHCI_EXEC_TUNING;
    ctrl2 &= ~SDHCI_SAMPLE_CLK_SEL;
    REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
    sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
}

static uint32_t sdhci_tuning_sequence(struct sdhci_host *host, uint32_t opcode,
                                      uint32_t bus_width)
{
    uint32_t ret;
    uint32_t reg;
    struct mmc_command cmd;
    memset(&cmd, 0, sizeof(struct mmc_command));
    cmd.cmd_index = opcode;
    cmd.argument = 0x0;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.trans_mode = SDHCI_MMC_READ;
    cmd.data_present = 0x0;
    cmd.data.data_ptr = NULL;
    cmd.data.num_blocks = 0x1;

    if (cmd.cmd_index == CMD21_SEND_TUNING_BLOCK &&
        bus_width == MMC_DATA_BUS_WIDTH_8BIT)
        cmd.data.blk_sz = 128;
    else
        cmd.data.blk_sz = 64;

    REG_WRITE16(host, cmd.data.blk_sz, SDHCI_BLKSZ_REG);

    if (host->caps.spec_version >= SDHCI_SPEC_VER4_NUM)
        REG_WRITE32(host, cmd.data.num_blocks, SDHCI_SDMASA_BLKCNT_REG);
    else
        REG_WRITE16(host, cmd.data.num_blocks, SDHCI_BLK_CNT_REG);

    /* Write to transfer mode register */
    REG_WRITE16(host, SDHCI_READ_MODE, SDHCI_TRANS_MODE_REG);

    for (int i = 0; i < MAX_TUNING_LOOP; i++) {
        sdhci_send_command(host, &cmd);
        /* wait for read buf ready, timeout 50ms */
        ret = sdhci_wait_for_bit(host, SDHCI_NRML_INT_STS_REG,
                                 SDHCI_NRML_INT_BUF_RD_READY, 0, 50);

        if (ret) {
            dprintf(CRITICAL, "sdhci tuning timeout!\n");
            sdhci_dumpregs(host);
            return ret;
        }

        reg = REG_READ16(host, SDHCI_HOST_CTRL2_REG);

        if (!(reg & SDHCI_EXEC_TUNING)) {
            if (reg & SDHCI_SAMPLE_CLK_SEL) {
                dprintf(INFO, "sdhci: tuning cycle count = %d\n", i);
                if (i < 40)
                    dprintf(CRITICAL, "SDHC: tuning cycle abnormal, num is %d!\n", i);

                return 0;
            }
            break;
        }
    }

    return 1;
}

static inline void enable_tune_clk_stop(struct sdhci_host *host)
{
    u32 reg;
    u16 vender_base;

    /* read verder base register address */
    vender_base = REG_READ16(host, SDHCI_VENDOR_BASE_REG) & 0xFFF;

    reg = REG_READ32(host, vender_base + SDHCI_VENDER_AT_CTRL_REG);
    reg &= ~(SDHCI_TUNE_SWIN_TH_VAL_MASK << SDHCI_TUNE_SWIN_TH_VAL_LSB);
    reg |= (0xF << SDHCI_TUNE_SWIN_TH_VAL_LSB);
    reg |= SDHCI_TUNE_CLK_STOP_EN_MASK;
    REG_WRITE32(host, reg, vender_base + SDHCI_VENDER_AT_CTRL_REG);
}

/*
 * Function: sdhci execute tuning
 * Arg     : Host structure & tuning opcode & bus width
 * Return  : 0 on Success, 1 on Failure
 * Flow:   : 1. IF has platform tuning api, execute it
 *           2. start sdhci tuning
 *           3. execute sdhci tuning sequence
 */
uint32_t sdhci_execute_tuning(struct sdhci_host *host, uint32_t opcode,
                              uint32_t bus_width)
{
    uint32_t ret = 0;
    uint16_t ctrl2 = 0;
    uint16_t clk_ctrl = 0;

    /* reset the tuning circuit */
    ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
    ctrl2 &= ~SDHCI_EXEC_TUNING;
    ctrl2 &= ~SDHCI_SAMPLE_CLK_SEL;
    REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
    enable_tune_clk_stop(host);
    sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);

    if (host->ops->platform_execute_tuning) {
        return host->ops->platform_execute_tuning(host, opcode, bus_width);
    }

    /*
     * For avoid giltches, need disable the card clock before set
     * EXEC_TUNING bit.
     */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);
    spin(1);

    sdhci_start_tuning(host);
    host->tuning_in_progress = 1;
    spin(1);

    /* enable the card clock */
    clk_ctrl |= SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);
    spin(1);

    if (sdhci_tuning_sequence(host, opcode, bus_width)) {
        dprintf(CRITICAL, "sdhci tuning failed, abort tuning state!\n");
        sdhci_abort_tuning(host);
        ret = 1;
    }

    host->tuning_in_progress = 0;
    return ret;
}

enum handler_return sdhci_irq_handle(void *arg)
{
    u32 int_status;
    u32 mask;
    struct sdhci_host *host = arg;
    int max_loops = 16;
    struct mmc_command *cmd = host->cmd;
    struct mmc_command *data_cmd = &host->data_cmd;
    spin_lock(&host->spin_lock);
    int_status = REG_READ32(host, SDHCI_NRML_INT_STS_REG);

    if (!int_status) {
        spin_unlock(&host->spin_lock);
        return INT_NO_RESCHEDULE;
    }

    do {
        DBG("sdhci: irq status 0x%08x\n", int_status);
        /* clear the interrupts */
        mask = int_status &
               (SDHCI_INT_CMD_MASK | SDHCI_INT_DATA_MASK | SDHCI_INT_BUS_POWER);
        REG_WRITE32(host, mask, SDHCI_NRML_INT_STS_REG);

        if (int_status & SDHCI_INT_CMD_MASK) {
            if (int_status &
                (SDHCI_INT_TIMEOUT | SDHCI_INT_CRC | SDHCI_INT_END_BIT |
                 SDHCI_INT_INDEX | SDHCI_INT_ACMD12ERR)) {
                cmd->error = (int_status & SDHCI_INT_CMD_MASK) >> 16;
                dprintf(CRITICAL, "sdhci cmd error! reg = 0x%08x\n",
                        int_status);
                sdhci_dumpregs(host);
            }

            if (cmd->resp_type == SDHCI_CMD_RESP_R2) {
                for (int i = 0; i < 4; i++) {
                    cmd->resp[i] = REG_READ32(host, SDHCI_RESP_REG + (i * 4));
                    cmd->resp[i] <<= SDHCI_RESP_LSHIFT;

                    if (i != 0)
                        cmd->resp[i] |=
                            (REG_READ32(host, SDHCI_RESP_REG + ((i - 1) * 4)) >>
                             SDHCI_RESP_RSHIFT);
                }
            }
            else {
                cmd->resp[0] = REG_READ32(host, SDHCI_RESP_REG);
            }

            event_signal(&host->cmd_event, false);
        }

        /* clear the data complete flag */
        if (int_status & SDHCI_INT_DATA_MASK) {
            if (data_cmd->data_present) {
                data_cmd->error = (int_status & SDHCI_INT_DATA_MASK) >> 16;

                if (int_status & SDHCI_INT_DATA_END || data_cmd->error) {
                    event_signal(&host->data_event, false);
                }
            }
            else {
                cmd->error |= (int_status & SDHCI_INT_DATA_MASK) >> 16;

                if (int_status & SDHCI_INT_DATA_END || cmd->error) {
                    event_signal(&host->data_event, false);
                }
            }
        }

        int_status = REG_READ32(host, SDHCI_NRML_INT_STS_REG);
    } while (int_status & --max_loops);

    spin_unlock(&host->spin_lock);
    return INT_NO_RESCHEDULE;
}

static int sdhci_data_thread(void *arg)
{
    struct sdhci_host *host = arg;
    struct mmc_command *data_cmd;
    struct mmc_handle *handle = NULL;

    for (;;) {
        event_wait(&host->data_event);
        DBG("\n %s:sdhci data thread in!\n", __func__);

        if (host->parent) {
            handle = host->parent;
        }

        data_cmd = &host->data_cmd;

        if (!host->data_cmd.data_present) {
            /* If no data cmd, may be RESP_R1B type cmd. */
            if (host->cmd->error) {
                dprintf(CRITICAL, "sdhci busy error! 0x%08x\n", host->cmd->error);
                sdhci_dumpregs(host);
                sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
            }

            event_signal(&host->data_complete_event, false);
            continue;
        }
        else {
            if (data_cmd->error) {
                dprintf(CRITICAL, "sdhci data error! 0x%08x\n", data_cmd->error);
                sdhci_dumpregs(host);
                sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
                /* Send abort cmd only when data present */
                sdhci_cmd_abort(host);
            }
        }

        if (data_cmd->trans_mode == SDHCI_MMC_READ) {
            /*
             * Read can be performed on block size < SDHCI_MMC_BLK_SZ,
             * make sure to flush the data only for the read size instead.
             */
            arch_invalidate_cache_range(
                (addr_t)data_cmd->data.data_ptr,
                (data_cmd->data.blk_sz)
                    ? (data_cmd->data.num_blocks * data_cmd->data.blk_sz)
                    : (data_cmd->data.num_blocks * SDHCI_MMC_BLK_SZ));
        }

        /* Free the scatter/gather list */
        if (data_cmd->data.sg_list) {
            free(data_cmd->data.sg_list);
            data_cmd->data.sg_list = NULL;
        }

        if (handle && handle->event_handle) {
            data_cmd->data_present = 0;
            handle->opt_result = data_cmd->error;
            handle->event_handle(handle->opt_type, handle->opt_result);
        }
        else {
            DBG("\n %s:signal complete event!\n", __func__);
            event_signal(&host->data_complete_event, true);
        }
    }

    return 0;
}

/*
 * Function: sdhci init
 * Arg     : Host structure
 * Return  : None
 * Flow:   : 1. Reset the controller
 *           2. Read the capabilities register & populate the host
 *           controller capabilities for use by other functions
 *           3. Enable the power control
 *           4. Set initial bus width
 *           5. Set Adma mode
 *           6. Enable the error status
 */
void sdhci_init(struct sdhci_host *host)
{
    uint32_t caps[2];
    uint16_t spec_version;

    host->spin_lock = SPIN_LOCK_INITIAL_VALUE;
    event_init(&host->cmd_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&host->data_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&host->data_complete_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    sdhci_reset(host, SDHCI_SOFT_RESET);

    spec_version = REG_READ16(host, SDHCI_SPEC_VERSION_REG);
    host->caps.spec_version = (uint8_t)(spec_version & 0xff);
    /* Read the capabilities register & store the info */
    caps[0] = REG_READ32(host, SDHCI_CAPS_REG1);
    caps[1] = REG_READ32(host, SDHCI_CAPS_REG2);
    DBG("\n %s: Host version: %d capability: cap1:0x%08x, cap2: 0x%08x\n",
        __func__, host->caps.spec_version, caps[0], caps[1]);

    host->caps.base_clk_rate =
        (caps[0] & SDHCI_CLK_RATE_MASK) >> SDHCI_CLK_RATE_BIT;
    host->caps.base_clk_rate *= 1000000;
    /* Get the max block length for mmc */
    host->caps.max_blk_len =
        (caps[0] & SDHCI_BLK_LEN_MASK) >> SDHCI_BLK_LEN_BIT;

    /* 8 bit Bus width */
    if (caps[0] & SDHCI_8BIT_WIDTH_MASK) {
        host->caps.bus_width_8bit = 1;
    }

    /* Adma2 support */
    if (caps[0] & SDHCI_BLK_ADMA_MASK) {
        host->caps.adma2_support = 1;
    }

    /* V4 64bit address support */
    if (caps[0] & SDHCI_CAP_ADDR_64BIT_V4) {
        host->caps.addr_64bit_v4 = 1;
    }

    /* V3 64bit address support */
    if (caps[0] & SDHCI_CAP_ADDR_64BIT_V3) {
        host->caps.addr_64bit_v3 = 1;
    }

    /* Supported voltage, do nothing when it has been set */
    if (host->caps.voltage) {
        ;
    }
    else if (caps[0] & SDHCI_3_3_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_3_3;
    }
    else if (caps[0] & SDHCI_3_0_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_3_0;
    }
    else if (caps[0] & SDHCI_1_8_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_1_8;
    }

    /* DDR mode support */
    host->caps.ddr_support = (caps[1] & SDHCI_DDR50_MODE_MASK) ? 1 : 0;
    /* SDR50 mode support */
    host->caps.sdr50_support = (caps[1] & SDHCI_SDR50_MODE_MASK) ? 1 : 0;
    /* SDR104 mode support */
    host->caps.sdr104_support = (caps[1] & SDHCI_SDR104_MODE_MASK) ? 1 : 0;
    sdhci_reset(host, SDHCI_SOFT_RESET);

    /* Set bus power on */
    if (!host->ops->set_power) {
        host->ops->set_power = sdhci_set_bus_power_on;
    }

    host->ops->set_power(host);
    /* Set bus width */
    sdhci_set_bus_width(host, SDHCI_BUS_WITDH_1BIT);
    /* Set Adma mode */
    sdhci_set_adma_mode(host);
    /* Enable error status */
    sdhci_error_status_enable(host);
#ifdef SDHCI_SUPPORT_INTERRUPT
    thread_t *thread =
        thread_create("sdhci_thread", sdhci_data_thread, (void *)host,
                      DEFAULT_PRIORITY, INT_STACK_SZ);
    thread_resume(thread);
    DBG("\n Host irq: %d \n", host->irq);
    register_int_handler(host->irq, &sdhci_irq_handle, (void *)host);
    unmask_interrupt(host->irq);
#endif
}
