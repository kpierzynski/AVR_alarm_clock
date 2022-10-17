#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <avr/io.h>

#include "protocols/spi.h"
#include "soft_timer.h"

#define DISPLAY_LEN 4

#define NUM_LEN 17

#define NUM_0 0x00
#define NUM_1 0x01
#define NUM_2 0x02
#define NUM_3 0x03
#define NUM_4 0x04
#define NUM_5 0x05
#define NUM_6 0x06
#define NUM_7 0x07
#define NUM_8 0x08
#define NUM_9 0x09
#define NUM_A 0x0A
#define NUM_B 0x0B
#define NUM_C 0x0C
#define NUM_D 0x0D
#define NUM_E 0x0E
#define NUM_F 0x0F
#define NUM_DASH 0x10
#define NUM_EMPTY 0x11

#define BLINK_NONE 0b0000
#define BLINK_HOUR 0b0011
#define BLINK_MIN 0b1100
#define BLINK_BOTH 0b1111

typedef struct
{
	uint8_t buf[DISPLAY_LEN];
	uint8_t is_enabled;
} screen_t;

void display_init(screen_t *s);

void display_set_screen(screen_t *screen);

void display_blink(uint8_t mask);

void display_tick();

#endif