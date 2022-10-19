#ifndef __CLOCK_H_
#define __CLOCK_H_

#include "alarm.h"
#include "ds1307.h"

#include <avr/interrupt.h>
#include <avr/io.h>

void clock_init();
void clock_update_time( time_t * time );
void clock_save_time( time_t time );
void clock_event();

void clock_open_alarm( uint8_t index, alarm_t * alarm );
void clock_save_alarm( uint8_t index, alarm_t * alarm );

void register_clock_out_1hz( void ( *callback )( void ) );
#endif
