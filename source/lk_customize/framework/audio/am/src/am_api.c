/**
 * @file am_api.c
 * @author shao yi
 * @brief
 * @version 0.1
 * @date 2021-05-08
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <debug.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread.h>
#include "am_api.h"

#include "ak7738.h"
#include "am.h"
#include "am_debug.h"
#include "am_path.h"
#include "am_tas5404.h"
#include "am_tca9539.h"
#include "refa04_path.h"
#include "refa03_path.h"
#include "tas6424.h"
#include "xf6020.h"

#define AM_API_DBG_PRT 2
#define AM_DEAMON_ERR_CHECK_MS 200
#define AM_DEAMON_ACTIVE_CHECK_MS 1000
#define AM_DEAMON_IDLE_CHECK_MS 5000

/**
 * @brief Same as AM_BOARD enum
 *
 */
const char *board_name_str[] = {
    "BOARD_X9_REFA04",
    "BOARD_X9_REFA03",
    "BOARD_X9_MS",
};
/**
 *@brief this part should be same ad path enum.
 *
 */
const char *path_name_str[] = {
    "IDLE_PATH",
    "HIFI_PLAYBACK_TO_MAIN_SPK_48K",
    "HIFI_CAPTURE_FROM_MAIN_MIC_48K",
    "SAFETY_PLAYBACK_TO_MAIN_SPK_48K",
    "BT_PLAYBACK_TO_MAIN_SPK_16K",
    "BT_CAPTURE_FROM_MAIN_MIC_16K",
    "BT_PLAYBACK_TO_MAIN_SPK_8K",
    "BT_CAPTURE_FROM_MAIN_MIC_8K",
    "CLUSTER_PLAYBACK_TO_MAIN_SPK_48K",
    "CLUSTER_CAPTURE_FROM_MAIN_MIC_48K",
    "FM_PLAYBACK_TO_MAIN_SPK_48K",
    "TBOX_PLAYBACK_TO_MAIN_SPK_48K",
};
/* global audio data */

am_board_t g_am_board;
const am_board_interface_t *g_am_board_interface;

/*global definition*/
/**
 * @brief
 *
 * @param board_type
 * @return AM_RESULT
 */
AM_RESULT audio_manager_init(int board_type)
{
    bool ret = false;
    /* Load am_data to global table */

    dprintf(AM_API_DBG_PRT, "audio_manager_init in %s.\n",
            board_name_str[board_type]);
    /*init path manager*/
    /*init board */
    g_am_board.codec_numb = 0;
    g_am_board.deamon_on = true;
    g_am_board.board_status = AM_BOARD_UNINITED;
    switch (board_type) {
    case BOARD_X9_REFA04:
        /* 1. get board interface,Don't change the sequence and set codec
         * number*/
        g_am_board_interface = get_refa04_board_interface();
        g_am_board.codec_numb = REFA04_CHIP_NUMB;
        /*2. get codec interface CHIP_AK7738*/
        g_am_board.codec[REFA04_CHIP_AK7738] = sdrv_ak7738_get_ctl_interface();
        /*3. init codec interface CHIP_AK7738*/
        ret = g_am_board_interface->chip_initialize(REFA04_CHIP_AK7738);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }

        g_am_board.codec[REFA04_CHIP_TCA9539] =
            sdrv_tca9539_get_ctl_interface();
        ret = g_am_board_interface->chip_initialize(REFA04_CHIP_TCA9539);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }

        g_am_board.codec[REFA04_CHIP_TAS6424] =
            sdrv_tas6424_get_ctl_interface();
        ret = g_am_board_interface->chip_initialize(REFA04_CHIP_TAS6424);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }

        g_am_board.codec[REFA04_CHIP_XF6020] = sdrv_xf6020_get_ctl_interface();
        ret = g_am_board_interface->chip_initialize(REFA04_CHIP_XF6020);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }
        g_am_board.codec[REFA04_CHIP_TAS5404] =
            sdrv_tas5404_get_ctl_interface();
        ret = g_am_board_interface->chip_initialize(REFA04_CHIP_TAS5404);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }
        g_am_board.codec_numb = REFA04_CHIP_NUMB;
        break;
    case BOARD_X9_REFA03:
        g_am_board_interface = get_refa03_board_interface();
        g_am_board.codec_numb = REFA03_CHIP_NUMB;
        /*get ak7738 codec interface CHIP_AK7738*/
        g_am_board.codec[REFA03_CHIP_AK7738] = sdrv_ak7738_get_ctl_interface();
        ret = g_am_board_interface->chip_initialize(REFA03_CHIP_AK7738);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }

        g_am_board.codec[REFA03_CHIP_TCA9539] =
            sdrv_tca9539_get_ctl_interface();

        ret = g_am_board_interface->chip_initialize(REFA03_CHIP_TCA9539);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }

        g_am_board.codec[REFA03_CHIP_TAS6424] =
            sdrv_tas6424_get_ctl_interface();
        ret = g_am_board_interface->chip_initialize(REFA03_CHIP_TAS6424);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }

        g_am_board.codec[REFA03_CHIP_XF6020] = sdrv_xf6020_get_ctl_interface();
        ret = g_am_board_interface->chip_initialize(REFA03_CHIP_XF6020);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }
        g_am_board.codec[REFA03_CHIP_TAS5404] =
            sdrv_tas5404_get_ctl_interface();
        ret = g_am_board_interface->chip_initialize(REFA03_CHIP_TAS5404);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            return RET_ERR_INIT_FAILED;
        }
        break;
    case BOARD_X9_MS:
        break;
    }
    /* Reset board */
    ret = g_am_board_interface->reset();
    if (ret == false) {
        _ERR_FUNC_LINE_PRT_
        return RET_ERR_INIT_FAILED;
    }
    for (int i = 0; i < g_am_board.codec_numb; i++) {
        if (g_am_board.codec[i]->initialize) {
            g_am_board.codec[i]->initialize(&g_am_board.codec_dev[i]);
        } else {
            _ERR_FUNC_LINE_PRT_
        }
    }
    /*init path status*/
    init_path_status_tbl(PATH_TOTAL_NUMB);
    g_am_board.board_status = AM_BOARD_INITED;
    mutex_init(&g_am_board.am_path_mutex);
    if (g_am_board.deamon_on == true) {
        audio_manager_deamon();
    }
    return RET_OK;
}

static void am_deamon_task(void)
{
    int res;
    while (g_am_board.deamon_on == true) {
        mutex_acquire(&g_am_board.am_path_mutex);
        res = am_check_task();
        mutex_release(&g_am_board.am_path_mutex);
        if (res != RET_OK) {
            printf("am_deamon_task status:%d \n", res);
            thread_sleep(AM_DEAMON_ERR_CHECK_MS);
        } else {

            if (get_active_path_number() > 1) {
                // printf("am_deamon_task path numb:%d
                // \n",get_active_path_number());
                thread_sleep(AM_DEAMON_ACTIVE_CHECK_MS);
            } else {
                thread_sleep(AM_DEAMON_IDLE_CHECK_MS);
            }
        }
    }
}

/**
 * @brief start a thread check audio manager status. Need start after audio
 * manager first init
 *
 * @return AM_RESULT
 */

AM_RESULT audio_manager_deamon(void)
{

    thread_t *au_deamon_thread =
        thread_create("am_deamon", (thread_start_routine)am_deamon_task, 0,
                      DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(au_deamon_thread);
    return RET_OK;
}

/**
 * @brief audio_manager_reset
 *
 * @return AM_RESULT
 */
AM_RESULT audio_manager_reset(void)
{
    bool ret = false;
    if (g_am_board.board_status != AM_BOARD_INITED) {
        _ERR_FUNC_LINE_PRT_;
        return RET_ERR_INIT_FAILED;
    }
    mutex_acquire(&g_am_board.am_path_mutex);
    g_am_board.board_status = AM_BOARD_RESETING;
    ret = g_am_board_interface->reset();
    mutex_release(&g_am_board.am_path_mutex);
    if (ret == false) {
        _ERR_FUNC_LINE_PRT_
        return RET_ERR_INIT_FAILED;
    }

    init_path_status_tbl(PATH_TOTAL_NUMB);
    g_am_board.board_status = AM_BOARD_INITED;
    return RET_OK;
}
/**
 * @brief audio_manager_reset_codec
 *
 * @param codec_id
 * @return AM_RESULT
 */
/* AM_RESULT audio_manager_reset_codec(int codec_id)
{
    _NO_COMP_PRT_;
    return RET_OK;
}
 */
/**
 * @brief Audio Manager release
 *
 * @return AM_RESULT
 */
AM_RESULT audio_manager_release(void)
{
    g_am_board.deamon_on = false;
    return RET_OK;
}
/**
 * @brief A user function to execute custom special functions.
 *
 * @param path_id this function will check path id, to execute the function.
 * @param mode  user define the mode of this function.
 * @param param_size next param_size.
 * @param param  param buffer.
 * @return AM_RESULT
 */
AM_RESULT audio_manager_user_func(unsigned int path_id, int mode,
                                  int param_size, unsigned char *param)
{
    bool ret = false;

    if (g_am_board.board_status != AM_BOARD_INITED) {
        _ERR_FUNC_LINE_PRT_;
        return RET_ERR_INIT_FAILED;
    }
    /*path already isn't enabled*/
    if (am_get_path_active_status(path_id) == 0) {
        _ERR_FUNC_LINE_PRT_;
        return RET_ERR_PATH_INACTIVE;
    }
    /* invoke board function*/
    if (g_am_board_interface->user_func) {
        mutex_acquire(&g_am_board.am_path_mutex);
        ret = g_am_board_interface->user_func(path_id, mode, param_size, param);
        if (ret == false) {
            _ERR_FUNC_LINE_PRT_
            g_am_board.board_status = AM_BOARD_RESETING;
            mutex_release(&g_am_board.am_path_mutex);
            return RET_ERR_UNKNOWN;
        }
        mutex_release(&g_am_board.am_path_mutex);
    } else {
        dprintf(AM_API_DBG_PRT, "board user_func is NULL.\n");
    }
    return RET_OK;
}

/**
 * @brief start an audio path
 *
 * @param path_name
 * @param vol
 * @return AM_RESULT
 */
AM_RESULT start_path(unsigned int path_id, int vol)
{
    bool ret = false;

    if (g_am_board.board_status != AM_BOARD_INITED) {
        _ERR_FUNC_LINE_PRT_;
        return RET_ERR_INIT_FAILED;
    }
    if (path_id >= get_path_number()) {
        dprintf(AM_API_DBG_PRT, "Can't find path id by name.\n");
        return RET_ERR_INVALID_PATH;
    }
    printf("path name: %s %d\n", path_name_str[path_id], path_id);
    /* set path status */
    /*path already enabled*/
    if (am_get_path_active_status(path_id) != 0) {
        /*path volume changed*/
        if (am_get_path_vol_changed(path_id, vol) == 0) {
            return RET_PATH_NOT_NEED_CHANGE;
        } else {
            /* TODO:change path volume  */
            _NO_COMP_PRT_;
            return RET_PATH_VOL_CHANGED;
        }
    }
    mutex_acquire(&g_am_board.am_path_mutex);
    add_path_by_id(path_id, vol);
    /* config path */

    ret = g_am_board_interface->sync();
    if (ret == false) {
        _ERR_FUNC_LINE_PRT_
        g_am_board.board_status = AM_BOARD_RESETING;
        mutex_release(&g_am_board.am_path_mutex);
        return RET_ERR_SYNC_FAILED;
    }
    mutex_release(&g_am_board.am_path_mutex);
    return RET_OK;
}
/**
 * @brief stop an audio path
 *
 * @param path_name
 * @return AM_RESULT
 */
AM_RESULT stop_path(unsigned int path_id)
{

    bool ret = false;

    if (g_am_board.board_status != AM_BOARD_INITED) {
        _ERR_FUNC_LINE_PRT_;
        return RET_ERR_INIT_FAILED;
    }
    if (path_id >= get_path_number()) {
        dprintf(AM_API_DBG_PRT, "Can't find path id by name.\n");
        return RET_ERR_INVALID_PATH;
    }
    mutex_acquire(&g_am_board.am_path_mutex);
    remove_path_by_id(path_id);
    ret = g_am_board_interface->sync();
    if (ret == false) {
        _ERR_FUNC_LINE_PRT_
        g_am_board.board_status = AM_BOARD_RESETING;
        mutex_release(&g_am_board.am_path_mutex);
        return RET_ERR_SYNC_FAILED;
    }
    mutex_release(&g_am_board.am_path_mutex);
    return RET_OK;
}
/**
 * @brief switch an audio path
 *
 * @param from_path_name
 * @param to_path_name
 * @return AM_RESULT
 */
AM_RESULT switch_path(unsigned int from_path_id, unsigned int to_path_id)
{

    int ret = RET_OK;
    int mute, vol, status;
    if (g_am_board.board_status != AM_BOARD_INITED) {
        _ERR_FUNC_LINE_PRT_;
        return RET_ERR_INIT_FAILED;
    }
    if (from_path_id >= get_path_number()) {
        dprintf(AM_API_DBG_PRT, "Can't find from path id by name.\n");
        return RET_ERR_INVALID_PATH;
    }
    if (to_path_id >= get_path_number()) {
        dprintf(AM_API_DBG_PRT, "Can't find to path id by name.\n");
        return RET_ERR_INVALID_PATH;
    }
    /**Check path status */
    status = am_get_path_active_status(from_path_id);
    if (status == PATH_INACTIVE) {
        dprintf(0, "This path(%d) already is inactive .\n", from_path_id);
    }
    status = am_get_path_active_status(from_path_id);
    if (status == PATH_ACTIVE) {
        dprintf(0, "This path(%d) already is active .\n", to_path_id);
    }
    /** get path vol and mute */
    mute = am_get_path_mute(from_path_id);
    vol = am_get_path_vol(from_path_id);
    mutex_acquire(&g_am_board.am_path_mutex);
    ret = add_path_by_id(to_path_id, vol);
    if (ret != RET_OK) {
        _ERR_FUNC_LINE_PRT_
        goto out;
    }
    ret = remove_path_by_id(from_path_id);
    if (ret != RET_OK) {
        _ERR_FUNC_LINE_PRT_
        goto out;
    }
    ret = am_set_path_mute(to_path_id, mute);
    if (ret != RET_OK) {
        _ERR_FUNC_LINE_PRT_
        goto out;
    }

    if (g_am_board_interface->sync() == false) {
        _ERR_FUNC_LINE_PRT_
        g_am_board.board_status = AM_BOARD_RESETING;
        ret = RET_ERR_SYNC_FAILED;
        goto out;
    }

out:
    mutex_release(&g_am_board.am_path_mutex);
    return ret;
}
/**
 * @brief mute one audio path.
 *
 * @param path_name
 * @param val
 * @return AM_RESULT
 */
AM_RESULT mute_path(unsigned int path_id, int val)
{
    int ret;
    if (g_am_board.board_status != AM_BOARD_INITED) {
        _ERR_FUNC_LINE_PRT_;
        return RET_ERR_INIT_FAILED;
    }
    if (path_id >= get_path_number()) {
        dprintf(AM_API_DBG_PRT, "Can't find path id by name.\n");
        return RET_ERR_INVALID_PATH;
    }
    ret = am_set_path_mute(path_id, val);
    if (ret != RET_OK) {
        return ret;
    }
    mutex_acquire(&g_am_board.am_path_mutex);
    if (g_am_board_interface->sync() == false) {
        _ERR_FUNC_LINE_PRT_
        g_am_board.board_status = AM_BOARD_RESETING;
        mutex_release(&g_am_board.am_path_mutex);
        return RET_ERR_SYNC_FAILED;
    }
    mutex_release(&g_am_board.am_path_mutex);
    return RET_OK;
}

/**
 * @brief Set the Path Vol object
 *
 * @param path_name
 * @param volume
 * @return AM_RESULT
 */
AM_RESULT set_path_vol(unsigned int path_id, int volume)
{
    int ret;
    if (g_am_board.board_status != AM_BOARD_INITED) {
        _ERR_FUNC_LINE_PRT_;
        return RET_ERR_INIT_FAILED;
    }
    if (path_id >= get_path_number()) {
        dprintf(AM_API_DBG_PRT, "Can't find path id by name.\n");
        return RET_ERR_INVALID_PATH;
    }
    ret = am_set_path_vol(path_id, volume);
    if (ret != RET_OK) {
        return ret;
    }
    mutex_acquire(&g_am_board.am_path_mutex);
    if (g_am_board_interface->sync() == false) {
        _ERR_FUNC_LINE_PRT_
        g_am_board.board_status = AM_BOARD_RESETING;
        mutex_release(&g_am_board.am_path_mutex);
        return RET_ERR_SYNC_FAILED;
    }
    mutex_release(&g_am_board.am_path_mutex);
    return RET_OK;
}
/**
 * @brief Get the Path Stat object
 *
 * @param path_id
 * @return int path state
 */
int get_path_stat(unsigned int path_id)
{
    if (path_id >= get_path_number()) {
        dprintf(AM_API_DBG_PRT, "Can't find path id by name.\n");
        return RET_ERR_INVALID_PATH;
    }
    return am_get_path_active_status(path_id);
}
/**
 * @brief Get the Path Vol object
 *
 * @param path_name
 * @return int
 */
int get_path_vol(unsigned int path_id)
{
    if (path_id >= get_path_number()) {
        dprintf(AM_API_DBG_PRT, "Can't find path id by name.\n");
        return RET_ERR_INVALID_PATH;
    }
    return am_get_path_vol(path_id);
}
/**
 * @brief Get the Path Mute Stat object
 *
 * @param path_name
 * @return int
 */
int get_path_mute_stat(unsigned int path_id)
{
    if (path_id >= get_path_number()) {
        dprintf(AM_API_DBG_PRT, "Can't find path id by name.\n");
        return RET_ERR_INVALID_PATH;
    }
    return am_get_path_mute(path_id);
}
