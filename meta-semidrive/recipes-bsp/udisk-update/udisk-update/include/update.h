#ifndef __UPDATE__H_
#define __UPDATE__H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdio_ext.h>
#include <unistd.h>
#include "slots_parse.h"

#define LBA_SIZE 512
#define MAX_CALLOC_SIZE (256*1024)
#define USE_RW
#define OSPI_IMG_COUNT 10
#define EMMC_IMG_COUNT 15

#define PAC_MAGIC       (0x5041434B) // which means "PACK"
#define PAC_VERSION		(0)


#define OSPI_TYPE 0
#define EMMC_TYPE 1

#define SLOT_A 0
#define SLOT_B 1

static const char* EMMC_PAC_ROOT_PATH = "/update/";
static const char* GLOBAL_PAC_NAME = "global.pac";
static const char* OSPI_PAC_NAME = "ospi_safety.pac";

enum
{
    E_PACK_FIRST_DA_FILE    = 0,
    E_PACK_MID_DA_FILE      = 1,
    E_PACK_DLOADER_FILE     = 2,
    E_PACK_SPL_FILE         = 3,
    E_PACK_SAFETY_FILE      = 4,
    E_PACK_MBR_GPT_FILE     = 5,
    E_PACK_IMG_FILE         = 6,
    E_PACK_PAC_FILE         = 7,
};

enum
{
    UPDATE_OSPI_A_TYPE    = 0,
    UPDATE_OSPI_B_TYPE    = 1,
    UPDATE_EMMC_A_TYPE    = 0,
    UPDATE_EMMC_B_TYPE    = 1,
};

int read_img_file_from(const char*name,int from,int len, uint8_t*data);
int do_update(int type);
int dump_update_progress(int type,int add_size);

#endif