#include "ds1307.h"

void ( *ds1307_out_callback )();

void register_ds1307_out( void ( *callback )( void ) ) {
	ds1307_out_callback = callback;
}

static uint8_t bcd2dec( uint8_t bcd ) {
	return ( ( ( ( bcd ) >> 4 ) & 0x0F ) * 10 ) + ( (bcd)&0x0F );
}

static uint8_t dec2bcd( uint8_t dec ) {
	return ( ( dec / 10 ) << 4 ) | ( dec % 10 );
}

static void ds1307_send( uint8_t addr, uint8_t data ) {
	i2c_start();
	i2c_write( DS1307_ADDR << 1 | 0 );
	i2c_write( addr );
	i2c_write( data );
	i2c_stop();
}

static uint8_t ds1307_read( uint8_t addr ) {
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

static void ds1307_read_buf( uint8_t addr, uint8_t len, uint8_t * buf ) {
	i2c_start();
	i2c_write( DS1307_ADDR << 1 | 0 );
	i2c_write( addr );
	i2c_start();
	i2c_write( DS1307_ADDR << 1 | 1 );
	while ( len-- )
		*buf++ = i2c_read( len ? ACK : NACK );
	i2c_stop();
}

static void ds1307_write_buf( uint8_t addr, uint8_t len, uint8_t * buf ) {
	i2c_start();
	i2c_write( DS1307_ADDR << 1 | 0 );
	i2c_write( addr );
	while ( len-- )
		i2c_write( *buf++ );
	i2c_stop();
}

void ds1307_get_time( uint8_t * hh, uint8_t * mm, uint8_t * ss ) {
	*hh = bcd2dec( ds1307_read( HOURS_REG ) );
	*mm = bcd2dec( ds1307_read( MINUTES_REG ) );
	*ss = bcd2dec( ds1307_read( SECONDS_REG ) );
}

void ds1307_set_time( uint8_t hh, uint8_t mm, uint8_t ss ) {
	ds1307_send( HOURS_REG, dec2bcd( hh ) );
	ds1307_send( MINUTES_REG, dec2bcd( mm ) );
	ds1307_send( SECONDS_REG, dec2bcd( ss ) );
}

void ds1307_read_ram_buf( uint8_t address, uint8_t len, uint8_t * buf ) {
	ds1307_read_buf( RAM_REG + address, len, buf );
}

void ds1307_write_ram_buf( uint8_t address, uint8_t len, uint8_t * buf ) {
	ds1307_write_buf( RAM_REG + address, len, buf );
}

void ds1307_init() {
	INT_IN;
	INT_PULLUP;

	INT_EDGE_REG |= INT_EDGE_DETECT;
	INT_ENABLE_REG |= ( 1 << INT );

	ds1307_send( CONTROL_REG, ( 1 << DS1307_SQWE ) | OUT_FREQ );								   // ENABLE SQUARE 1HZ WAVE
	ds1307_send( CH_REG, ds1307_read( CH_REG ) & ~( 1 << DS1307_CH ) );							   // START OSCILATOR
	ds1307_send( MODE_12_24_REG, ds1307_read( MODE_12_24_REG ) & ~( 1 << DS1307_12_24_MODE ) );	   // SET 24HOUR MODE
}

ISR( INT_VECT ) {
	if ( ds1307_out_callback )
		ds1307_out_callback();
}
