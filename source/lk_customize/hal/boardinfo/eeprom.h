
#ifndef __EEPROM_H
#define __EEPROM_H
#include <stdint.h>
#include "debug.h"

#include "storage_device.h"

storage_device_t *get_eeprom_dev(int index);
#endif
