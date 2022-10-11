#include "clock.h"

volatile uint8_t call_callback;
void (*clock_out_1hz_callback)(void);

void register_clock_out_1hz( void (*callback)(void) ) {
	clock_out_1hz_callback = callback;
}

static void clock_send( uint8_t addr, uint8_t data ) {
	i2c_start();
	i2c_write( DS1307_ADDR << 1 | 0 );
	i2c_write( addr );
	i2c_write( data );
	i2c_stop();
}

static uint8_t clock_read( uint8_t addr ) {
	register uint8_t data;
	i2c_start();
	i2c_write( DS1307_ADDR << 1 | 0 );
	i2c_write( addr );
	i2c_start();
	i2c_write( DS1307_ADDR << 1 | 1 );
	data = i2c_read( NACK );
	i2c_stop();

	return data;
}

void clock_init() {
	i2c_init( 100 );

	DDRD &= ~(1<<PD2);
	PORTD |= (1<<PD2);

	EICRA |= (1<<ISC01)|(1<<ISC00);		//RISING EDGE
	EIMSK |= (1<<INT0);

	clock_send( CONTROL_ADDR, 0b00010000 );					//SQUARE 1HZ WAVE (check that)
	clock_send( CH_ADDR, clock_read(CH_ADDR) & 0b01111111 );		//START OSCILATOR
	clock_send( HOURS_ADDR, clock_read(HOURS_ADDR) & 0b00111111 );		//SET 24HOUR MODE
}

static uint8_t bcd2dec( uint8_t bcd ) {
	return ((((bcd) >> 4) & 0x0F) * 10 ) + ((bcd) & 0x0F);
}

static uint8_t dec2bcd( uint8_t dec ) {
	return ( (dec/10) << 4 ) | ( dec % 10 );
}

void clock_time( time_t * time ) {
	uint8_t hr = clock_read(HOURS_ADDR);
	hr = bcd2dec( hr );
	uint8_t min = clock_read(MINUTES_ADDR);
	min = bcd2dec( min );

	time->min = min;
	time->hour = hr;
}

void clock_event() {
	if( call_callback ) {
		if( clock_out_1hz_callback ) clock_out_1hz_callback();
		call_callback = 0;
	}
}

ISR( INT0_vect ) {
	call_callback = 1;
}
