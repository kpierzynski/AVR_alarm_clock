#ifndef __CLOCK_H_
#define __CLOCK_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c.h"
#include "common.h"

#define DS1307_ADDR	0x68

#define CH_ADDR		0x00

#define SECONDS_ADDR	0x00
#define MINUTES_ADDR	0x01
#define HOURS_ADDR	0x02

#define DAY_ADDR	0x03
#define DATE_ADDR	0x04
#define MONTH_ADDR	0x05
#define YEARS_ADDR	0x06

#define CONTROL_ADDR	0x07

#define RAM_ADDR	0x08

void register_clock_out_1hz( void (*callback)(void) );
void clock_init();

void clock_time( time_t * time );
void clock_event();

#endif
