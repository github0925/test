/**
 *@file am.h
 *@author yi shao (yi.shao@semidrive.com)
 *@brief
 *@version 0.1
 *@date 2021-05-11
 *
 *@copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#ifndef __AM_H__
#define __AM_H__
#include <stdio.h>
#include <kernel/mutex.h>
#include "am_api.h"
#include "am_debug.h"
/**
 * @brief Audio Control type.
 *
 */
typedef enum {
    CTL_NORMAL_TYPE = 0, ///< normal control type
    CTL_GAIN_TYPE = 1,   ///< gain type
    CTL_COEFF_TYPE = 2,  ///< coeff
    CTL_GPIO_TYPE = 3,   ///< gpio
} AM_CTL_TYPE;

/**
 * @brief Audio Path direction
 *
 */
typedef enum {
    PATH_IN,  // From mic
    PATH_OUT, // To speaker
} AM_PATH_DIRECTION;
/**
 * @brief Audio Path attribute
 *
 */
typedef enum {
    PATH_EXCLUSIVE =
        0, ///< High priority, another audio path need quit or mute.
    PATH_SUPPRESS_EXTRA, ///< Another audio path need duck, such as reduce
                         ///< audio volume to 20%, and other audio paths
                         ///< will restore after this audio path disabled.
    PATH_SUPPRESS,       ///< Another audio path need duck, such as reduce audio
                         ///< volume to 50%, and other audio paths will restore
                         ///< after this audio path disabled.
    PATH_NORMAL          ///<
} AM_PATH_ATTRIBUTE;
/**
 *@brief register cache types
 *
 */
typedef enum {
    CACHE_FREE = 0,          ///< register cache be not used, need sync.
    CACHE_CLEAN = 1,         ///< register cache is in clean state
    CACHE_DIRTY = 2,         ///< register cache is dirty
} AM_REG_CACHE_STATUS;

/**
 *@brief am board status
 *
 */
typedef enum {
    AM_BOARD_UNINITED = 0, ///<
    AM_BOARD_INITED = 1,   ///<
    AM_BOARD_RESETING = 2, ///<
    AM_BOARD_RESTORING = 3, ///<

} AM_BOARD_STATUS;
/**
 * @brief Audio path status
 *
 */
typedef struct {
    unsigned int active;
    unsigned int order;
    unsigned int volume;
    unsigned int mute;
    unsigned int attribute;
} sdrv_am_path_status_t;
/**
 * @brief Audio control status.
 *
 */
typedef struct {
    unsigned int ctl_id;
    unsigned int ctl_reg;
    unsigned int ctl_mask;
    unsigned int ctl_ref;
    unsigned int ctl_type;
} sdrv_am_ctl_t;

/*set to last item for static sdrv_am_reg_t ak7738_reg[] */
#define ERR_REG_ADDR 0xFFFF
typedef struct {
    unsigned int reg_addr;
    unsigned int value;
    unsigned int status;
   // unsigned int type;
} sdrv_am_reg_t;

typedef enum {
    CODEC_TYPE_AK7738 = 0,
    CODEC_TYPE_TCA9539,
    CODEC_TYPE_TAS6424,
    CODEC_TYPE_XF6020,
    CODEC_TYPE_TAS5404,
} codec_type_t;

typedef enum {
    PROTOCOL_TYPE_UNKNOWN = 0,
    PROTOCOL_TYPE_I2C,
    PROTOCOL_TYPE_SPI,
    PROTOCOL_TYPE_GPIO,
    PROTOCOL_TYPE_NUMB,
} codec_protocol_type_t;
/*this structure define the audio codec resource*/

typedef struct {
    int chip_id;
    codec_type_t codec_type; ///< codec
    void *dev_handle;        ///< i2c instance/GPIO instance/spi instance
    uint8_t addr;
    uint8_t protocol;
} am_codec_dev_t;
typedef bool (*writeable_reg_t)(am_codec_dev_t *dev, unsigned int reg);
typedef bool (*readable_reg_t)(am_codec_dev_t *dev, unsigned int reg);

/** codec control driver interface
 *  TODO: user am_reg save data only support single instance.
 */
typedef struct am_ctl_interface {
    bool (*initialize)(am_codec_dev_t *dev);
    bool (*read_ctl)(am_codec_dev_t *dev, unsigned int ctl, unsigned int *val);
    bool (*write_ctl)(am_codec_dev_t *dev, unsigned int ctl, unsigned int val);
    bool (*read_reg)(am_codec_dev_t *dev, unsigned int reg, unsigned int *val);
    bool (*write_reg)(am_codec_dev_t *dev, unsigned int reg, unsigned int val);
    bool (*reset)(am_codec_dev_t *dev);
    bool (*burn_fw)(am_codec_dev_t *dev, int fw_no);
    bool (*user_func)(am_codec_dev_t *dev, int mode, int param_size, unsigned char *param);
    bool (*writeable_reg)(am_codec_dev_t *dev, unsigned int reg);
    bool (*readable_reg)(am_codec_dev_t *dev, unsigned int reg);
    sdrv_am_reg_t *am_regs;
    sdrv_am_ctl_t *am_ctls;
    unsigned int am_regs_numb;
} am_ctl_interface_t;

#define MAX_CODEC_NUMB 8
/**
 * @brief Audio Manager board information
 *
 */
typedef struct {
    am_ctl_interface_t *codec[MAX_CODEC_NUMB];
    am_codec_dev_t codec_dev[MAX_CODEC_NUMB];
    sdrv_am_path_status_t am_path_status[PATH_TOTAL_NUMB];
    int codec_numb;
    AM_BOARD_STATUS board_status;
    mutex_t am_path_mutex;
    bool deamon_on;
} am_board_t;

/*wdg driver interface structure */
typedef struct am_board_interface {

    bool (*chip_initialize)(int chip_id);
    bool (*chip_release)(int chip_id);
    bool (*reset)(void);
    bool (*path_op)(unsigned int path_id, unsigned int op_code,
                    unsigned int param);
    bool (*sync)(void);
    bool (*exception_check)(void);
    bool (*user_func)(unsigned int path_id, int mode, int param_size,
                      unsigned char *param);

} am_board_interface_t;

/* internal structure */
typedef struct _am_instance {
    am_board_interface_t am_board; /*!< dma driver interface*/
    char am_magic[20];
    bool initialized;
    /* instance id 0 ~8*/
    int32_t inst_id;
} am_instance_t;
bool set_chip_handle(unsigned int chip_id, void * handle,  unsigned int addr);
bool set_chip_info(unsigned int chip_id, unsigned int protocol, unsigned int codec_type);
bool set_chip_writeable_func(unsigned int chip_id,  writeable_reg_t  func);
bool set_chip_readable_func(unsigned int chip_id,  readable_reg_t  func);
am_codec_dev_t *get_chip_dev(unsigned int chip_id);
/**
 *@brief sync the controls of chip by chip_id
 *
 *@param chip_id
 *@return true
 *@return false
 */
bool sync_ctl(unsigned int chip_id);
/**
 *@brief read ctl's value by chip id
 *
 *@param chip_id
 *@param ctl_name
 *@param val
 *@return true
 *@return false
 */
bool read_ctl(unsigned int chip_id, unsigned int ctl_name, unsigned int *val);
/**
 *@brief write value to ctl by chip id.
 *
 *@param chip_id
 *@param ctl_name
 *@param val
 *@return true
 *@return false
 */
bool write_ctl(unsigned int chip_id, unsigned int ctl_name, unsigned int val);
/**
 *@brief write value to ctl directly.
 *
 *@param chip_id
 *@param ctl_name
 *@param val
 *@return true
 *@return false
 */
bool write_ctl_nocache(unsigned int chip_id, unsigned int ctl_name,
                       unsigned int val);


bool read_reg(unsigned int chip_id, unsigned int reg_addr, unsigned int *val);
bool write_reg(unsigned int chip_id, unsigned int reg_addr, unsigned int val);
bool write_reg_nocache(unsigned int chip_id, unsigned int reg_addr, unsigned int val);
/**
 *@brief reset control by chip_id
 *
 *@param chip_id
 *@return true
 *@return false
 */
bool reset(unsigned int chip_id);
/**
 *@brief burn firmware by chip_id.
 *
 *@param chip_id
 *@param fw_no
 *@return true
 *@return false
 */
bool burn_fw(unsigned int chip_id, int fw_no);
/**
 * @brief delay unsigned int ms
 *
 * @param val ms
 * @return bool
 */
bool delay_ctl(unsigned int val);

/**
 *@brief Get the linear vol object, this function is used to
 *       calculate volume index by linear mapping.
 *
 *@param vol input vol index , from 0 ~100
 *@param max_val  max volume index
 *@param min_val  min volume index
 *@return int     volume index result for vol.
 */
int get_linear_vol(int vol, int max_val, int min_val);
/**
 *@brief dump register's value by chip id.
 *
 *@param chip_id
 *@return true
 *@return false
 */
bool dump_regs(int chip_id);

/**
 * @brief make reg cache dirty. then will read from reg
 *
 * @param chip_id
 * @param reg_addr
 * @return true
 * @return false
 */
bool make_dirty_reg(unsigned int chip_id, unsigned int reg_addr);
/*TODO: No tested*/
bool make_dirty_chip(unsigned int chip_id);
bool make_dirty_ctl(unsigned int chip_id, unsigned int ctl_name);


#endif