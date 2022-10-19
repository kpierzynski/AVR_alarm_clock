#include "display.h"

screen_t * screen;

volatile uint8_t tick;	  // Ticking 2 vertical dots.

volatile uint8_t blink_mask;	// Mask of blinking digits
volatile uint8_t blink;			// If should blink

const uint8_t nums[ NUM_LEN ] = {
	0b00000010,	   // 0
	0b01011110,	   // 1
	0b10010000,	   // 2
	0b00010100,	   // 3
	0b01001100,	   // 4
	0b00100100,	   // 5
	0b00100000,	   // 6
	0b00011110,	   // 7
	0b00000000,	   // 8
	0b00000100,	   // 9

	0b00001000,	   // A
	0b01100000,	   // B
	0b10100010,	   // C
	0b01010000,	   // D
	0b10100000,	   // E
	0b10101000,	   // F

	0b11111110	  // Empty
};

void display_set_screen( screen_t * s ) {
	screen->is_enabled = 0;
	s->is_enabled = 1;

	screen = s;
}

inline void display_blink( uint8_t mask ) {
	blink_mask = mask;
}

inline void display_tick() {
	tick ^= 1;
}

static void blink_timer() {
	blink ^= 1;
}

void display_init( screen_t * s ) {
	// TIMER FOR MULTIPLEXING
	TCCR0A |= ( 1 << WGM01 );
	TCCR0B |= ( 1 << CS02 ) | ( 1 << CS00 );
	OCR0A = 32;
	TIMSK0 |= ( 1 << OCIE0A );

	spi_init();
	// DOTS
	// DDRD |= (1 << PD5) | (1 << PD6) | (1 << PD7);
	// PORTD |= (1 << PD5) | (1 << PD6) | (1 << PD7);

	// PLEXER TRANSISTOR DIRECTION
	DDRC |= ( 1 << PC0 ) | ( 1 << PC1 ) | ( 1 << PC2 ) | ( 1 << PC3 );

	timer_create( 1, 250, blink_timer );

	screen = s;
}

ISR( TIMER0_COMPA_vect ) {
	static uint8_t digit = 0;

	if ( blink && ( blink_mask & ( 1 << digit ) ) )
		spi_tx( nums[ NUM_EMPTY ] | tick );
	else
		spi_tx( nums[ screen->buf[ digit ] ] | tick );

	PORTC = ( PORTC & 0b11110000 ) | ~( 1 << digit );
	PORTB |= ( 1 << PB2 );
	PORTB &= ~( 1 << PB2 );

	digit = ( digit + 1 ) % DISPLAY_LEN;
}