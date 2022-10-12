#include "clock.h"

volatile uint8_t call_callback;
void (*clock_out_1hz_callback)(void);

void register_clock_out_1hz( void (*callback)(void) ) {
	clock_out_1hz_callback = callback;
}

void clock_update_time( time_t * time ) {
	uint8_t ss;
	ds1307_get_time( &time->hour, &time->min, &ss);
}

void clock_event() {
	if( call_callback ) {
		if( clock_out_1hz_callback ) clock_out_1hz_callback();
		call_callback = 0;
	}
}
