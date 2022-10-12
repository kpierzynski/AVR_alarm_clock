#ifndef __SOFT_TIMER_H_
#define __SOFT_TIMER_H_

#include <avr/io.h>

#define MAX_TIMER_COUNT		4

typedef struct {
	uint16_t interval;
	uint8_t enabled;
} timer_t;

#endif