#ifndef __INDEV_LOCAL_HEAD__
#define __INDEV_LOCAL_HEAD__

#include <stdlib.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>
#include <event.h>

int indev_local_init(int x_off, int y_off, int x_max, int y_max);
int indev_local_process_touch_report(u8 *data, u16 len, int screen_id);
bool indev_local_is_pressed(int screen_id);
void indev_local_get_xy(int *x, int *y, int screen_id);

#endif //__INDEV_LOCAL_HEAD__