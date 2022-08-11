/*
test i2s sc play and record on evb board.
*/

#include "akm_ak7738.h"
#include "animation_config.h"
#include "chip_res.h"
#include "dma_hal.h"
#include "early_app_cfg.h"
#include "i2c_hal.h"
#include "i2s_hal.h"
#include "ti_tas6424.h"
#if ENABLE_AUDIO_MANAGER
#include "am_api.h"
#endif

#include <app.h>
#include <container.h>
#include <debug.h>
#include <heap.h>
#include <lib/console.h>
#include <platform.h>
#include <platform/debug.h>
#include <res_loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boardinfo_hwid_usr.h"
#include "audio_agent.h"
enum {
    FA_NONE = 0, // Fast audio no
    FA_CODEC_REFA03,
    FA_CODEC_REFA04,
    FA_CODEC_MS,
    FA_CODEC_CFG0,
    FA_AM_REFA03,
    FA_AM_REFA04,
    FA_AM_MS,
    FA_AM_CFG0,
};
static int fast_audio_flag = FA_NONE;

#define PERIOD_SIZE (2048)
#define TEST_DURATION (10000)

#define I2S_RES RES_I2S_SC_I2S_SC1
#define I2C_RES RES_I2C_I2C4
#define I2S_DMA_CHAN DMA_PERI_I2S_SC1
static lk_time_t start_ts;
static event_t e_record_end;

static void *i2s_handle;
static struct dma_desc *desc_tx, *desc_rx;
static struct audio_codec_dev *ak7738;
const static struct au_codec_dev_ctrl_interface *ak7738_drv;
static struct audio_codec_dev *tas6424;
const static struct au_codec_dev_ctrl_interface *tas6424_drv;
/*  */
uint8_t *fast_audio_pcm = NULL;
int fast_audio_size = 0;

static void init_ref_codecs(pcm_params_t pcm)
{
    pcm_params_t pcm_tas6424;
#if FAST_AUDIO_CFG0 == 1
    ak7738 = sdrv_ak7738_get_dev(2);
#else
    if (get_part_id(PART_BOARD_ID_MIN) <= 3) {
        ak7738 = sdrv_ak7738_get_dev(1);
    } else {
        ak7738 = sdrv_ak7738_get_dev(2);
    }
#endif
    ak7738_drv = sdrv_ak7738_get_controller_interface();
    hal_i2c_creat_handle(&ak7738->i2c_handle, ak7738->i2c_res_num);
    if (&ak7738->i2c_handle != NULL) {
        i2c_app_config_t i2c_conf = hal_i2c_get_busconfig(ak7738->i2c_handle);
        printf("i2c conf poll is %d\n", i2c_conf.poll);
        i2c_conf.poll = 1;
        hal_i2c_set_busconfig(ak7738->i2c_handle, &i2c_conf);
    }
    ak7738_drv->initialize(ak7738);
    ak7738_drv->set_format(*ak7738, pcm);
    ak7738_drv->set_hw_params(*ak7738, pcm);
    ak7738_drv->start_up(ak7738);
    tas6424 = sdrv_tas6424_get_dev(1);
    tas6424_drv = sdrv_tas6424_get_controller_interface();
    /* Overwite pcm parameters */
    memcpy(&pcm_tas6424, &pcm, sizeof(pcm_params_t));
    pcm_tas6424.mode = SD_AUDIO_PCM_MODE_SET(
        pcm.mode, SD_AUDIO_TRANSFER_MODE_ENABLE, SD_AUDIO_TRANSFER_CODEC_SLAVE);
    pcm_tas6424.standard = SD_AUDIO_I2S_DSP_A;
    hal_i2c_creat_handle(&tas6424->i2c_handle, tas6424->i2c_res_num);
    if (&tas6424->i2c_handle != NULL) {
        i2c_app_config_t i2c_conf = hal_i2c_get_busconfig(tas6424->i2c_handle);
        printf("i2c conf poll is %d\n", i2c_conf.poll);
        i2c_conf.poll = 1;
        hal_i2c_set_busconfig(tas6424->i2c_handle, &i2c_conf);
    }
    tas6424_drv->initialize(tas6424);
    tas6424_drv->set_format(*tas6424, pcm_tas6424);
    tas6424_drv->set_hw_params(*tas6424, pcm_tas6424);
    tas6424_drv->set_volume(tas6424, 60, AUDIO_VOL_LINEOUT);
    tas6424_drv->start_up(tas6424);
}

static void trigger_ref_codecs(int cmd)
{
    tas6424_drv->trigger(*tas6424, cmd);
    ak7738_drv->trigger(*ak7738, cmd);
}

static void disable_ref_codecs(void)
{
    tas6424_drv->shutdown(*tas6424);
    ak7738_drv->shutdown(*ak7738);
}

static void playback_irq_evt_handle(enum dma_status status, u32 err,
                                    void *context)
{
    printf("dma irq evt: status(%d) err(0x%x) context(%p) \n", status, err,
           context);
    if (DMA_COMP != status) {
        /* terminate this channel after error. */
        hal_dma_terminate(desc_tx);
        if (fast_audio_flag <= FA_CODEC_CFG0) {
            trigger_ref_codecs(SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
        }
    }
}

static void record_cyclic_irq_evt_handle(enum dma_status status, u32 err,
                                         void *context)
{
    // printf("dma irq evt: status(%d) err(%d) context(%p) \n", status, err,
    // context);
    if ((current_time() - start_ts) > TEST_DURATION) {
        printf("sc record stopped ts(%d)\n", current_time());
        hal_dma_terminate(desc_rx);
        printf("sc record stopped ts1(%d)\n", current_time());
        trigger_ref_codecs(SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP);
        event_signal(&e_record_end, false);
        printf("sc record stopped ts2(%d)\n", current_time());
    }
}

void sc_play(void)
{
    pcm_params_t pcm_info;
    if (!hal_sd_i2s_sc_create_handle(&i2s_handle, I2S_RES)) {
        printf("error:i2s resource get failed.\n");
        return;
    }
    pcm_info.mode = SD_AUDIO_TRANSFER_CODEC_MASTER |
                    SD_AUDIO_DIR_MODE_TRANSMIT |
                    SD_AUDIO_CHANNEL_NUM_TX_STEREO | SD_AUDIO_TRANSFER_WITH_DMA;
    pcm_info.sample_rate = SD_AUDIO_SR_48000;
    pcm_info.resolution = SD_AUDIO_SAMPLE_WIDTH_16BITS;
    pcm_info.standard = SD_AUDIO_I2S_STANDARD_PHILLIPS;
    pcm_info.slot_width = SD_AUDIO_SLOT_WIDTH_32BITS;
#if ENABLE_AUDIO_MANAGER
    if (fast_audio_flag <= FA_CODEC_CFG0) {
        init_ref_codecs(pcm_info);
    } else {
        start_path(SAFETY_PLAYBACK_TO_MAIN_SPK_48K, 80);
    }
#else
    init_ref_codecs(pcm_info);
#endif
    hal_sd_i2s_sc_start_up(i2s_handle, pcm_info);
    hal_sd_i2s_sc_set_format(i2s_handle);
    hal_sd_i2s_sc_set_hw_params(i2s_handle);
    start_ts = current_time();
    printf("pcm res 0x%x %p \n", fast_audio_size, (void *)fast_audio_pcm);
    // event_init(&e_record_end, false, EVENT_FLAG_AUTOUNSIGNAL);
    if (fast_audio_flag <= FA_CODEC_CFG0) {
        trigger_ref_codecs(SD_AUDIO_PCM_TRIGGER_PLAYBACK_START);
    }
    desc_tx = hal_i2s_trigger_dma_tr_start(
        i2s_handle, I2S_DMA_CHAN, fast_audio_pcm, fast_audio_size,
        playback_irq_evt_handle, SD_AUDIO_PCM_TRIGGER_PLAYBACK_START);
    if (desc_tx == NULL)
        return;
    hal_dma_sync_wait(desc_tx, -1);
    hal_sd_i2s_sc_trigger(i2s_handle, SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
    hal_dma_free_desc(desc_tx);
    hal_sd_i2s_sc_shutdown(i2s_handle);
    hal_sd_i2s_sc_release_handle(i2s_handle);
    printf("%s : %d %d %d\n", __FUNCTION__, __LINE__, current_time() - start_ts,
           fast_audio_size);
#if ENABLE_AUDIO_MANAGER
    if (fast_audio_flag <= FA_CODEC_CFG0) {
        disable_ref_codecs();
    } else {
        stop_path(SAFETY_PLAYBACK_TO_MAIN_SPK_48K);
    }
#else
    disable_ref_codecs();
#endif
}

int load_pcm(void)
{
    int ret = 0;
    fast_audio_size = ROUNDUP(res_size(FA_PATH), 32);
    fast_audio_pcm = (uint8_t *)memalign(32, fast_audio_size);
    if (!fast_audio_pcm) {
        USDBG("Create audio pcm fail.\n");
        return -1;
    }
    ret = res_load(FA_PATH, fast_audio_pcm, fast_audio_size, 0);
    if (ret < 0) {
        printf("can't load audio res!\n");
        free(fast_audio_pcm);
        fast_audio_pcm = NULL;
    }
    return ret;
}

void audio_task(token_handle_t token)
{
    int ret = 0;
    // struct sd_hwid_usr hwid = get_fullhw_id();
    if (get_part_id(PART_BOARD_TYPE) == BOARD_TYPE_REF) {
        if (get_part_id(PART_BOARD_ID_MAJ) == 1) {
            if (get_part_id(PART_BOARD_ID_MIN) == 4) {
#if ENABLE_AUDIO_MANAGER
                fast_audio_flag = FA_AM_REFA04;
                if (RET_OK == audio_manager_init(BOARD_X9_REFA04)) {
                    printf("Audio Manager Init success.\n");
                    start_path(IDLE_PATH, 80);
                } else {
                    fast_audio_flag = FA_CODEC_REFA04;
                }
#else
                fast_audio_flag = FA_CODEC_REFA04;
#endif
            } else if (get_part_id(PART_BOARD_ID_MIN) == 3) {
#if ENABLE_AUDIO_MANAGER
                fast_audio_flag = FA_AM_REFA03;
                if (RET_OK == audio_manager_init(BOARD_X9_REFA03)) {
                    printf("Audio Manager Init success.\n");
                    start_path(IDLE_PATH, 80);
                } else
                {
                    fast_audio_flag = FA_CODEC_REFA03;
                }

#else
                fast_audio_flag = FA_CODEC_REFA03;
#endif
            }
        }
    }
#if FAST_AUDIO_CFG0 == 1
    fast_audio_flag = FA_CODEC_CFG0;
#endif
    printf("Fast Audio flag %-2d\n", fast_audio_flag);
    if (fast_audio_flag > FA_NONE) {
        ret = load_pcm();
        if (ret < 0) {
            printf("Load PCM failed.\n");
            return;
        }
        sc_play();
        free(fast_audio_pcm);
        fast_audio_pcm = NULL;
        // after play stop init agent.
        if (fast_audio_flag >= FA_AM_REFA03) {
#if ENABLE_AUDIO_MANAGER
            au_agent_init();
#endif
        }
    }
}
