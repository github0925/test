/**
 * @file am_api.h
 * @author shao yi
 * @brief define audio manager's api
 * @version 0.1
 * @date 2021-01-11
 *
 * @copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */

#ifndef __AM_API_H__
#define __AM_API_H__
/**
 * @brief audio manager support board, user need add enum here and related
 * source
 *
 */
typedef enum {
    BOARD_X9_REFA04 = 0,
    BOARD_X9_REFA03,
    BOARD_X9_MS,
} AM_BOARD;
/**
 * @brief Audio manager Opcode
 *
 */
typedef enum {
    OP_HANDSHAKE = 0,
    OP_START = 1,
    OP_STOP = 2,
    OP_MUTE = 3,
    OP_SETVOL = 4,
    OP_SWITCH = 5,
    OP_RESET = 6,
    OP_RESTORE = 7,
    OP_PLAY_AGENT = 8,
    OP_PCM_PLAYBACK_CTL = 9,
    OP_PCM_CAPTURE_CTL = 10,
    OP_PCM_PLAYBACK_STREAM = 11,
    OP_PCM_CAPTURE_STREAM = 12,
    OP_PCM_LOOPBACK_CTL = 13,
    OP_NUMB
} AM_OPCODE_TYPE;

/**
 * @brief audio manager return value.
 *
 */
typedef enum {
    RET_OK = 0,               ///< return OK
    RET_PATH_NOT_NEED_CHANGE, ///< audio path already in thi status,don't need
                              ///< change
    RET_PATH_NOT_STARTED,     ///< can't not start audio path.
    RET_PATH_VOL_CHANGED,     ///< audio path's volume is changed
    RET_PATH_EXCLUDED,        ///< audio path is excluded.
    RET_PATH_DUCKED,          ///< audio path is ducked.
    RET_ERR_PATH_INVALID_VOLUME, ///< the volume is invalid value
    RET_ERR_INVALID_PATH,        ///< invalid path
    RET_ERR_INIT_FAILED,         ///< init failed
    RET_ERR_SYNC_FAILED,         ///< sync failed
    RET_ERR_SETVOL_FAILED,       ///< set volume failed
    RET_ERR_MUTE_FAILED,         ///< mute failed
    RET_ERR_PATH_INACTIVE,       ///< opcode failed for path inactive
    RET_ERR_CHECK_FAILED,         ///< check failed
    RET_ERR_NULL_FUNC,           ///< null function
    RET_ERR_UNKNOWN
} AM_RESULT;

typedef enum {
    UNMUTE_PATH = 0,
    MUTE_PATH = 1,
} AM_MUTE;

/**
 *@brief audio path enum
 *
 */
typedef enum {
    IDLE_PATH = 0,
    HIFI_PLAYBACK_TO_MAIN_SPK_48K,
    HIFI_CAPTURE_FROM_MAIN_MIC_48K,
    SAFETY_PLAYBACK_TO_MAIN_SPK_48K,
    BT_PLAYBACK_TO_MAIN_SPK_16K,
    BT_CAPTURE_FROM_MAIN_MIC_16K,
    BT_PLAYBACK_TO_MAIN_SPK_8K,
    BT_CAPTURE_FROM_MAIN_MIC_8K,
    CLUSTER_PLAYBACK_TO_MAIN_SPK_48K,
    CLUSTER_CAPTURE_FROM_MAIN_MIC_48K,
    FM_PLAYBACK_TO_MAIN_SPK_48K,
    TBOX_PLAYBACK_TO_MAIN_SPK_48K,
    PATH_TOTAL_NUMB,
} AM_PATH;

#define INVALID_PATH_ID (0x7ff)
#define INVALID_ORDER_INDEX (INVALID_PATH_ID)

/**
 * @brief Audio path active status
 *
 */
typedef enum {
    PATH_INACTIVE = 0,
    PATH_ACTIVE,
} AM_PATH_STATUS;
/* global definition */
#define VOLUME_STEPS 100
/**
 * @brief init audio manager befor use, it should for special board type.
 *
 * @param board_type enum AM_BOARD
 * @return AM_RESULT
 */
AM_RESULT audio_manager_init(int board_type);
/**
 * @brief release audio manager
 *
 * @return AM_RESULT
 */
AM_RESULT audio_manager_release(void);
/**
 * @brief audio manager deamon check.
 *
 * @return AM_RESULT
 */
AM_RESULT audio_manager_deamon(void);
/**
 * @brief audio_manager_reset
 *
 * @return AM_RESULT
 */
AM_RESULT audio_manager_reset(void);
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
                                  int param_size, unsigned char *param);

/**
 * @brief start audio path
 *
 * @param path_id enum AM_PATH
 * @param vol 0 ~ 100
 * @return AM_RESULT
 */
AM_RESULT start_path(unsigned int path_id, int vol);
/**
 * @brief stop audio path
 *
 * @param path_id enum AM_PATH
 * @return AM_RESULT
 */
AM_RESULT stop_path(unsigned int path_id);
/**
 * @brief switch from one path to another.
 *
 *
 * @param from_path_id
 * @param to_path_id
 * @return AM_RESULT
 */
AM_RESULT switch_path(unsigned int from_path_id, unsigned int to_path_id);
/**
 * @brief mute path
 *
 * @param path_id
 * @param val 1: mute 0: un-mute
 * @return AM_RESULT
 */
AM_RESULT mute_path(unsigned int path_id, int val);
/**
 * @brief Set the path vol object
 *
 * @param path_id
 * @param volume 0~100
 * @return AM_RESULT
 */
AM_RESULT set_path_vol(unsigned int path_id, int volume);


/**
 *@brief Get the path state function
 *
 *@param path_id AM_PATH
 *@return int AM_PATH_STATUS
 */
int get_path_stat(unsigned int path_id);
/**
 *@brief Get the path vol object
 *
 *@param path_id AM_PATH
 *@return int volume of path
 */
int get_path_vol(unsigned int path_id);
/**
 *@brief Get the path mute state
 *
 *@param path_id
 *@return int
 */
int get_path_mute_stat(unsigned int path_id);

#endif
