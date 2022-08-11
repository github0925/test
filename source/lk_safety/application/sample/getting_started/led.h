#ifndef _LED_H
#define _LED_H
#include <stdint.h>
void led_configure(void);
uint32_t led_set(uint32_t led);
uint32_t led_clear(uint32_t led);
uint32_t led_toggle(uint32_t led, uint32_t value);
#endif