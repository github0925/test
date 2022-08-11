
#include <app.h>
#include <arch.h>
#include <debug.h>
#include <heap.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <platform.h>
#include <platform/debug.h>
#include <res_loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "am_api.h"
#include "am.h"
#include "animation_config.h"
#include "chip_res.h"
#include "dma_hal.h"
#include "i2s_hal.h"
#if ENABLE_SD_DMA
#include "dma_hal.h"
#endif
#include "res.h"

static void *i2s_handle;
static lk_time_t start_ts;
static event_t e_record_end;
static event_t e_playback_end;
static void *audio_pcm_ptr = NULL;
static int audio_pcm_size = 0;
static struct dma_desc *desc_tx, *desc_rx;

int load_pcm_from_path(const char *path, void **buf, int *size)
{

    int ret = 0;
    *size = ROUNDUP(res_size(path), 32);
    *buf = memalign(32, *size);

    if (!(*buf)) {
        printf("Create audio pcm fail.\n");
        return -1;
    }
    ret = res_load(path, *buf, *size, 0);
    if (ret < 0) {
        printf("can't load audio res!\n");
        free(buf);
        buf = NULL;
    }
    printf("size 0x%x %p.\n", *size, *buf);
    return ret;
}

static void playback_irq_evt_handle(enum dma_status status, u32 err,
                                    void *context)
{
    printf("dma irq evt: status(%d) err(0x%x) context(%p) \n", status, err,
           context);

    if (DMA_COMP != status) {
        /* terminate this channel after error. */
        hal_dma_terminate(desc_tx);
        event_signal(&e_playback_end, false);
    }
}

void i2s_sc_play(void *buf, int size)
{

    pcm_params_t pcm_info;
    /* 	if(i2s_handle != NULL)
            hal_sd_i2s_sc_release_handle(i2s_handle); */
    if (!hal_sd_i2s_sc_create_handle(&i2s_handle, RES_I2S_SC_I2S_SC1)) {
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

    hal_sd_i2s_sc_start_up(i2s_handle, pcm_info);
    hal_sd_i2s_sc_set_format(i2s_handle);
    hal_sd_i2s_sc_set_hw_params(i2s_handle);
    start_ts = current_time();
    printf("pcm res 0x%x %p \n", size, buf);
    event_init(&e_playback_end, false, EVENT_FLAG_AUTOUNSIGNAL);
    desc_tx = hal_i2s_trigger_dma_tr_start(i2s_handle, DMA_PERI_I2S_SC1, buf,
                                           size, playback_irq_evt_handle,
                                           SD_AUDIO_PCM_TRIGGER_PLAYBACK_START);
    if (desc_tx == NULL)
        return;
    event_wait_timeout(&e_playback_end, 2000);

    hal_dma_sync_wait(desc_tx, 3000);

    hal_sd_i2s_sc_trigger(i2s_handle, SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);

    hal_dma_free_desc(desc_tx);
    hal_sd_i2s_sc_shutdown(i2s_handle);
    hal_sd_i2s_sc_release_handle(i2s_handle);
    printf("%s : %d %d %d\n", __FUNCTION__, __LINE__, current_time() - start_ts,
           size);
}

static int am_test(int argc, const cmd_args *argv)
{
    int vol = 80;
    int mute = 0;
    dprintf(0, "free heap memory:%d bytes.\n", xPortGetFreeHeapSize());
    if (!strcmp(argv[1].str, "help")) {
        dprintf(0, "am start/stop/setvol/mute/switch "
                   "hifip/hific/safetyp/btp/btc/clusterp/fmp/tboxp \n");
        dprintf(0, "am play\n");
        return 0;
    } else if (!strcmp(argv[1].str, "play")) {
        dprintf(0, "am play sc\n");
        i2s_sc_play(audio_pcm_ptr, audio_pcm_size);
        return 0;
    } else if (!strcmp(argv[1].str, "init")) {
        dprintf(0, "am init :audio manager init ->> %d\n", BOARD_X9_REFA04);

        audio_manager_init(BOARD_X9_REFA04);
        start_path(IDLE_PATH, 80);
        load_pcm_from_path(FA_PATH, &audio_pcm_ptr, &audio_pcm_size);
        printf("pcm res 0x%x %p \n", audio_pcm_size, audio_pcm_ptr);
        return 0;
    } else if (!strcmp(argv[1].str, "initpcm")) {
        load_pcm_from_path(FA_PATH, &audio_pcm_ptr, &audio_pcm_size);
        printf("pcm res 0x%x %p \n", audio_pcm_size, audio_pcm_ptr);
        return 0;
    } else if (!strcmp(argv[1].str, "start")) {
        dprintf(0, "am start ");
        if (!strcmp(argv[2].str, "hifip")) {
            dprintf(0, "HIFI_PLAYBACK_TO_MAIN_SPK_48K \n");
            start_path(HIFI_PLAYBACK_TO_MAIN_SPK_48K, 80);
        } else if (!strcmp(argv[2].str, "hific")) {
            dprintf(0, "HIFI_CAPTURE_FROM_MAIN_MIC_48K \n");
            start_path(HIFI_CAPTURE_FROM_MAIN_MIC_48K, 80);
        } else if (!strcmp(argv[2].str, "safetyp")) {
            dprintf(0, "SAFETY_PLAYBACK_TO_MAIN_SPK_48K\n");
            start_path(SAFETY_PLAYBACK_TO_MAIN_SPK_48K, 80);
        } else if (!strcmp(argv[2].str, "btp")) {
            dprintf(0, "BT_PLAYBACK_TO_MAIN_SPK_16K\n");
            start_path(BT_PLAYBACK_TO_MAIN_SPK_16K, 80);
        } else if (!strcmp(argv[2].str, "btc")) {
            dprintf(0, "BT_CAPTURE_FROM_MAIN_MIC_16K\n");
            start_path(BT_CAPTURE_FROM_MAIN_MIC_16K, 80);
        }else if (!strcmp(argv[2].str, "clusterp")) {
            dprintf(0, "CLUSTER_PLAYBACK_TO_MAIN_SPK_48K\n");
            start_path(CLUSTER_PLAYBACK_TO_MAIN_SPK_48K, 80);
        }else if (!strcmp(argv[2].str, "fmp")) {
            dprintf(0, "FM_PLAYBACK_TO_MAIN_SPK_48K\n");
            start_path(FM_PLAYBACK_TO_MAIN_SPK_48K, 80);
        }else if (!strcmp(argv[2].str, "tboxp")) {
            dprintf(0, "TBOX_PLAYBACK_TO_MAIN_SPK_48K\n");
            start_path(TBOX_PLAYBACK_TO_MAIN_SPK_48K, 80);
        }
        return 0;
    } else if (!strcmp(argv[1].str, "stop")) {
        dprintf(0, "am stop ");
        if (!strcmp(argv[2].str, "hifip")) {
            dprintf(0, "HIFI_PLAYBACK_TO_MAIN_SPK_48K \n");
            stop_path(HIFI_PLAYBACK_TO_MAIN_SPK_48K);

        } else if (!strcmp(argv[2].str, "hific")) {
            dprintf(0, "HIFI_CAPTURE_FROM_MAIN_MIC_48K \n");
            stop_path(HIFI_CAPTURE_FROM_MAIN_MIC_48K);

        } else if (!strcmp(argv[2].str, "safetyp")) {
            dprintf(0, "SAFETY_PLAYBACK_TO_MAIN_SPK_48K\n");
            stop_path(SAFETY_PLAYBACK_TO_MAIN_SPK_48K);

        } else if (!strcmp(argv[2].str, "btp")) {
            dprintf(0, "BT_PLAYBACK_TO_MAIN_SPK_16K\n");
            stop_path(BT_PLAYBACK_TO_MAIN_SPK_16K);

        } else if (!strcmp(argv[2].str, "btc")) {
            dprintf(0, "BT_CAPTURE_FROM_MAIN_MIC_16K\n");
            stop_path(BT_CAPTURE_FROM_MAIN_MIC_16K);
        }
        else if (!strcmp(argv[2].str, "clusterp")) {
            dprintf(0, "CLUSTER_PLAYBACK_TO_MAIN_SPK_48K\n");
            stop_path(CLUSTER_PLAYBACK_TO_MAIN_SPK_48K);
        }else if (!strcmp(argv[2].str, "fmp")) {
            dprintf(0, "FM_PLAYBACK_TO_MAIN_SPK_48K\n");
            stop_path(FM_PLAYBACK_TO_MAIN_SPK_48K);
        }else if (!strcmp(argv[2].str, "tboxp")) {
            dprintf(0, "TBOX_PLAYBACK_TO_MAIN_SPK_48K\n");
            stop_path(TBOX_PLAYBACK_TO_MAIN_SPK_48K);
        }
        return 0;
    } else if (!strcmp(argv[1].str, "setvol")) {
        dprintf(0, "am setvol ");
        if (!strcmp(argv[2].str, "btp")) {
            vol = argv[3].i;
            dprintf(0, "BT_PLAYBACK_TO_MAIN_SPK_16K to %d\n", vol);
            set_path_vol(BT_PLAYBACK_TO_MAIN_SPK_16K, vol);
        }
        return 0;
    } else if (!strcmp(argv[1].str, "mute")) {
        dprintf(0, "am mute\n");
        if (!strcmp(argv[2].str, "btc")) {
            mute = argv[3].i;
            dprintf(0, "BT_CAPTURE_FROM_MAIN_MIC_16K to %d\n", mute);
            mute_path(BT_CAPTURE_FROM_MAIN_MIC_16K, mute);
        } else if (!strcmp(argv[2].str, "hific")) {
            mute = argv[3].i;
            dprintf(0, "HIFI_CAPTURE_FROM_MAIN_MIC_48K to %d\n", mute);
            mute_path(HIFI_CAPTURE_FROM_MAIN_MIC_48K, mute);
        }
        return 0;
    } else if (!strcmp(argv[1].str, "switch")) {
        dprintf(0, "am switch\n");
        return 0;
    } else if (!strcmp(argv[1].str, "dump")) {
        dprintf(0, "am dump\n");
        dump_regs(0);
        dump_regs(1);
        dump_regs(2);
        dump_regs(3);

        return 0;
    } else if (!strcmp(argv[1].str, "reset")) {
        audio_manager_reset();
        start_path(IDLE_PATH, 80);
    }

    return 0;
}
#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("am", "am start/stop/setvol/mute/switch pathname",
               (console_cmd)&am_test)
STATIC_COMMAND_END(am_test);
#endif
APP_START(i2s_test_sample).flags = 0, APP_END