#ifndef __ALARM_H_
#define __ALARM_H_

#include "clock.h"
#include "common.h"
#include "util/modulo.h"

#include <stdint.h>

#define ALARM_COUNT 3

inline uint8_t time_compare( time_t a, time_t b ) {
	return ( a.hour == b.hour && a.min == b.min );
}

void alarm_init();

uint8_t alarm_armed( uint8_t index );
void	alarm_arm( uint8_t index );
void	alarm_unarm( uint8_t index );
uint8_t alarm_flip_arm( uint8_t index );

time_t alarm_get_time( uint8_t index );

time_t alarm_update_time( uint8_t i, time_t t );

uint8_t alarm_check( time_t * time );
void	alarm_sync();

#endif