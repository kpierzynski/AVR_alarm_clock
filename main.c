#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stddef.h>

#include "common.h"
#include "button.h"
#include "clock.h"

#define LEN 18
uint8_t nums[LEN] = {	0b00100000,	//0
			0b10111010,	//1
			0b01001000,	//2
			0b00011000,	//3
			0b10010010,	//4
			0b00010100,	//5
			0b00000100,	//6
			0b10111000,	//7
			0b00000000,	//8
			0b00010000,	//9
			0b10000000,	//A
			0b00000110,	//B
			0b01100100,	//C
			0b00001010,	//D
			0b01000100,	//E
			0b11000100,	//F
			0b11011110,	//-		0x10
			0b11111110	//Empty		0x11
};

#define DISPLAY_LEN	4
volatile uint8_t display[DISPLAY_LEN];

#define ALARM_LEN	3
alarm_t alarms[ALARM_LEN];
time_t time;

volatile uint8_t mode;

volatile uint8_t tick = 0;
void clock_tick() {
	tick ^= 1;

	clock_time( &time );
	display[0] = time.hour/10;
	display[1] = time.hour%10;
	display[2] = time.min/10;
	display[3] = time.min%10;

}

volatile uint8_t blink_mask;
volatile uint8_t blink;
ISR( TIMER0_COMPA_vect ) {
	static uint8_t digit = 0;

	PORTC = (PORTC & 0b11110000) | (1<<digit);

	if( blink && (blink_mask & (1<<digit)) )
		PORTB = ( nums[ 0x11 ] | tick );
	else PORTB = ( nums[ display[digit] ] | tick );

	digit = (digit+1) % DISPLAY_LEN;
}

void next_mode() {
}

void up_tick() {
}

void down_tick() {
}

int main() {

	//TIMER FOR MULTIPLEXING
	TCCR0A |= (1<<WGM01);
	TCCR0B |= (1<<CS02)|(1<<CS00);
	OCR0A = 32;
	TIMSK0 |= (1<<OCIE0A);

	//BUTTONS
	key_init();
	button_t mode = { &BTN_MODE_PIN, BTN_MODE, 2, next_mode, next_mode };
	button_t up = { &BTN_UP_PIN, BTN_UP, 2, up_tick, up_tick };
	button_t down = { &BTN_DOWN_PIN, BTN_DOWN, 2, down_tick, down_tick };

	BTN_MODE_IN;
	BTN_MODE_HIGH;

	BTN_UP_IN;
	BTN_UP_HIGH;

	BTN_DOWN_IN;
	BTN_DOWN_HIGH;

	//CLOCK
	clock_init();
	register_clock_out_1hz( clock_tick );

	//DOTS
	DDRD |= (1<<PD5)|(1<<PD6)|(1<<PD7);
	PORTD |= (1<<PD5)|(1<<PD6)|(1<<PD7);

	//NUMBER DATA PORT DIRECTION (use whole port. PB0 for dots)
	DDRB = 0xFF;

	//PLEXER TRANSISTOR DIRECTION
	DDRC |= (1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3);

	sei();
	while( 1 ) {
		key_press(&mode);
		key_press(&up);
		key_press(&down);

		if( !Timer2 ) {
			blink ^= 1;
			Timer2 = 33;
		}
	}

	return 0;
}

