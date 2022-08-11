#ifndef  _RADIO_H
#define  _RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <lk_wrapper.h>

#define FM_NUM 5

void init_fm(void);
bool get_fav(uint8_t sel);
void set_fav(bool bfav,uint8_t sel);
char* get_lable(uint8_t sel);
uint8_t getsel(void);
void setsel(uint8_t sel);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif