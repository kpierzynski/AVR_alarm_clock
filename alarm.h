#ifndef __ALARM_H_
#define __ALARM_H_

#include <stdint.h>

#include "common.h"
#include "clock.h"

#include "util/modulo.h"

#define ALARM_COUNT 3

inline uint8_t time_compare(time_t a, time_t b)
{
	return (a.hour == b.hour && a.min == b.min);
}

void alarm_init();

uint8_t alarm_armed(uint8_t index);
void alarm_arm(uint8_t index);
void alarm_unarm(uint8_t index);
uint8_t alarm_flip_arm(uint8_t index);

time_t alarm_get_time(uint8_t index);

void alarm_update_time(uint8_t i, time_t t);

void alarm_sync();

#endif