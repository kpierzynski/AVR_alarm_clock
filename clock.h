#ifndef __CLOCK_H_
#define __CLOCK_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#include "ds1307.h"
#include "common.h"

void clock_init();
void clock_update_time( time_t * time );
void clock_event();

void register_clock_out_1hz( void (*callback)(void) );
#endif
