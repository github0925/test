#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdm_display.h>
#include <disp_data_type.h>
#include <lib/reg.h>
#if defined (__GNUC__)
  #include <malloc.h>
#elif defined(__ICCARM__)
  #include "heap.h"
#else
  #error Unknown Compiler!
#endif
#include <string.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <kernel/spinlock.h>

//#include <getopt.h>

#include <app.h>
#include <res.h>
#include <trace.h>

#include <chip_res.h>
#include <spi_nor_hal.h>
#include <ext_data.h>
#include <heap.h>

#include "eeprom.h"

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

static int eeprom_write(int argc, const cmd_args *argv)
{
  int ret = 0;
  int index = 0;
  storage_device_t *bdinfo_dev = NULL;

  //eeprom_write [offset] [date_len] [date1] [date2] [date3] ....
  uint64_t offset = atoi(argv[1].str);
  uint64_t data_len = atoi(argv[2].str);
  uint8_t data[256] = {0};

  if((argc - 3) < (int)data_len) {
    dprintf(0, "eeprom test error: input arg  , eeprom_write [offset] [date_len] [date1] [date2] [date3] ....\n");
    return -1;
  }

  for(int i = 0; i < (int)data_len; i++) {
    if(argv[i+3].str == NULL) {
      dprintf(0, "error input data[%d] is null\n", i);
      return -1;
    }
    data[i] = atoi(argv[i+3].str);
  }

  bdinfo_dev = get_eeprom_dev(index);

  if (!bdinfo_dev) {
    return -1;
  }

  ret = bdinfo_dev->init(bdinfo_dev, 0, 0);

  if (ret != 0) {
    bdinfo_dev = NULL;
    return -1;
  }

  if (bdinfo_dev) {
      ret = bdinfo_dev->write(bdinfo_dev, offset, (void *)data, data_len);
      if (ret < 0) {
        dprintf(0, "error eeprom i2c write error\n");
      } else {
        dprintf(0, "write sucseed !\n");
      }
      bdinfo_dev->release(bdinfo_dev);
  }

  return 0;
}

static int eeprom_read(int argc, const cmd_args *argv)
{
  int ret = 0;
  int index = 0;
  storage_device_t *bdinfo_dev = NULL;

  if(argc < 3) {
    dprintf(0, "eeprom test error: input arg < 3 , eeprom_read [offset] [date_len]\n");
    return -1;
  }

  //eeprom_read [offset] [date_len]
  uint64_t offset = atoi(argv[1].str);
  uint64_t data_len = atoi(argv[2].str);
  uint8_t data[256] = {0};

  bdinfo_dev = get_eeprom_dev(index);

  if (!bdinfo_dev) {
    return -1;
  }

  ret = bdinfo_dev->init(bdinfo_dev, 0, 0);

  if (ret != 0) {
    bdinfo_dev = NULL;
    return -1;
  }

  if (bdinfo_dev) {
    ret = bdinfo_dev->read(bdinfo_dev, offset, (void *)data, data_len);
    if (ret < 0) {
      dprintf(0, "error eeprom i2c read error\n");
    } else {
      dprintf(0, "read sucseed !\n");
    }
    bdinfo_dev->release(bdinfo_dev);
  }

  for(int i = 0; i < (int)data_len; i ++) {
    dprintf(0,"read eeprom data[%d] = %d \n", i, data[i]);
  }

  return 0;
}

STATIC_COMMAND_START

STATIC_COMMAND("eeprom_write", "eeprom_write", &eeprom_write)
STATIC_COMMAND("eeprom_read", "eeprom_read", &eeprom_read)
STATIC_COMMAND_END(eeprom_test);

#endif
