#include "clock.h"

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

	clock_send( CONTROL_ADDR, 0b00010000 );
	clock_send( CH_ADDR, clock_read(CH_ADDR) & 0b01111111 );
}

static uint8_t bcd2dec( uint8_t bcd ) {
	return ((((bcd) >> 4) & 0x0F) * 10 ) + ((bcd) & 0x0F);
}

static uint8_t dec2bcd( uint8_t dec ) {
	return ( (dec/10) << 4 ) | ( dec % 10 );
}

void clock_time( time_t * time ) {
	uint8_t sec = clock_read(SECONDS_ADDR);
	sec = bcd2dec( sec );

	time->min = sec;
}

ISR( INT0_vect ) {
	if( clock_out_1hz_callback ) clock_out_1hz_callback();
}
