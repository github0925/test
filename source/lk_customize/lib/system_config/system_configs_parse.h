/*
 * system_configs_parse.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

/*
	*********************
	*    RES_HEADER_T   *
	*********************
	=====================
	=   RES_MODULE_T 0  =
	=       ...         =
	=   RES_MODULE_T n  =
	=====================
	---------------------
	- RES_MODULE_DATA 0 -
	-       ...         -
	- RES_MODULE_DATA n -
	---------------------
*/

#ifndef __SYSTEM_CONFIGS_PARSE_H__
#define __SYSTEM_CONFIGS_PARSE_H__

#include "image_cfg.h"

#if SYS_CFG_VALID
#define CONFIGS_BASE SYS_CFG_MEMBASE
#else
#define CONFIGS_BASE 0
#endif

typedef enum config_module_type
{
    MODULE_RES_INFO = 0,
    MODULE_RID_CFG = 1,
    MODULE_DISPLAY_LINK	= 2,
    MODULE_DISPLAY_SAF_CFG = 3,
    MODULE_PORT_CFG = 4,
    MODULE_PIN_CFG_DELTA = 5,
    MODULE_FIREWALL_CFG =6,
    MODULE_MAX_TYPE
} config_module_type_t;

typedef struct config_header
{
    uint32_t magic;
    uint32_t version;	    // struct version
    uint32_t config_count;  // the number of module_config
    uint32_t config_offset; // the offset from the res file header to the array of RES_HEADER_T struct buffer
    uint8_t reserved[44];   // reserved
    uint32_t crc32;         // CRC32 for all data except nCRC
}config_header_t;

#define CONFIG_HEADER_SIZE sizeof(config_header_t);    // Length = 64

typedef struct module_header
{
    uint32_t  version;                  //config version
    config_module_type_t  module_type;	// config_module_type;
    uint32_t  size;                     // file size
    uint32_t  config_offset;            // data offset
    uint8_t reserved[16];               // reserved
}module_header_t;

typedef struct config_info
{
    unsigned int  version;        // System config binary version
    char  build_time[20];         // build timestamp; "2020-08-27 13:42:52"
    char  username[32];           // build by user
    char  comments[64];           // comments
    unsigned char reserved[136];  // reserved
}config_info_t;

#define MODULE_HEADER_SIZE sizeof(module_header_t);    // Length = 32

/* Get config base address by id
 * @param module_type: module id of configuartion, refer config_module_type_t define
 * @param addr_base: configuration base address in memory
 * @return no error or error value
 */
int get_config_info(config_module_type_t module_type, addr_t * addr_base);

//error type
#define CONFIG_NO_ERROR             0
#define CONFIG_FILE_CHECK_FAIL      1
#define CONFIG_MODULE_NOT_FOUND     2

#endif
