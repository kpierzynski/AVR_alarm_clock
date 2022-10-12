#ifndef __SOFT_TIMER_H_
#define __SOFT_TIMER_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define MAX_TIMER_COUNT 4

typedef struct
{
	volatile uint16_t cnt;
	uint16_t interval;
	uint8_t enabled;
	void (*callback)();
} timer_t;

void timer_init();
void timer_create(uint8_t index, uint16_t interval, void (*callback)());
void timer_event();
void timer_interval(uint8_t index, uint16_t interval);

#endif