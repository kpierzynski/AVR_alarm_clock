#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stddef.h>

#include "common.h"
#include "button.h"
#include "clock.h"

#define NUM_LEN 18
#define NUM_DASH	0x10
#define NUM_EMPTY	0x11
const uint8_t nums[NUM_LEN] = {
	0b00100000,	//0
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
uint8_t main_screen[DISPLAY_LEN];
uint8_t alarm_screen[DISPLAY_LEN];
volatile uint8_t * display = main_screen;

#define ALARM_LEN	3
alarm_t alarms[ALARM_LEN];
time_t time;

uint8_t mode;

void update_main_screen() {
	main_screen[0] = time.hour/10;
	main_screen[1] = time.hour%10;
	main_screen[2] = time.min/10;
	main_screen[3] = time.min%10;
}

void update_alarm_screen_from_index( uint8_t i ) {
	alarm_screen[0] = 0xA;
	alarm_screen[1] = i;
	alarm_screen[2] = NUM_EMPTY;
	alarm_screen[3] = NUM_EMPTY;
}

void update_alarm_screen_from_time( time_t t ) {
	alarm_screen[0] = time.hour/10;
	alarm_screen[1] = time.hour%10;
	alarm_screen[2] = time.min/10;
	alarm_screen[3] = time.min%10;
}

volatile uint8_t tick = 0;
void clock_tick() {
	tick ^= 1;

	clock_time( &time );
}

volatile uint8_t blink_mask;
volatile uint8_t blink;
ISR( TIMER0_COMPA_vect ) {
	static uint8_t digit = 0;

	PORTC = (PORTC & 0b11110000) | (1<<digit);

	if( blink && (blink_mask & (1<<digit)) )
		PORTB = ( nums[ NUM_EMPTY ] | tick );
	else PORTB = ( nums[ display[digit] ] | tick );

	digit = (digit+1) % DISPLAY_LEN;
}

void next_mode() {
	Timer3 = 1000;

	uint8_t current_alarm_index = mode / ALARM_LEN;
	uint8_t type = mode % 3;

	alarm_t * alarm = &alarms[current_alarm_index];

	if( type == 0 ) {
		blink_mask = 0b0000;
		update_alarm_screen_from_index( current_alarm_index );
	} else {
		if( type == 1 )
			blink_mask = 0b0011;
		else blink_mask = 0b1100;

		update_alarm_screen_from_time( alarm->time );
	}
	display = alarm_screen;
	mode = (mode+1)%(ALARM_LEN*3);
}

void handle_change_on_alarm(uint8_t d) {
	Timer3 = 1000;

	if( display == main_screen ) return;

	uint8_t type = (mode-1) % 3;
	uint8_t current_alarm_index = (mode-1) / 3;

	alarm_t * alarm = &alarms[current_alarm_index];

	switch( type ) {
		case 0:
		{
			uint8_t armed = (alarm->armed ^= 1);
			if( armed ) PORTD &= ~(1<<(5+current_alarm_index));
			else PORTD |= (1<<(5+current_alarm_index));
			return;
		}
		case 1:
			alarm->time.hour += d;
			break;
		case 2:
			alarm->time.min += d;
			break;
	}
	update_alarm_screen_from_time( alarm->time );
}

void up_handler() {
	handle_change_on_alarm(1);
}

void down_handler() {
	handle_change_on_alarm(-1);
}

uint8_t time_compare(time_t a, time_t b) {
	return ( a.hour == b.hour && a.min == b.min );
}

void check_alarms() {
	for( uint8_t i = 0; i < ALARM_LEN; i++ ) {
		alarm_t * alarm = &alarms[i];

		if( !alarm->armed ) continue;

		if( 
	}
}

int main() {

	alarms[0].time.hour = 12;
	alarms[0].time.min = 30;
	alarms[1].time.hour = 7;
	alarms[1].time.min = 15;
	alarms[2].time.hour = 22;
	alarms[2].time.min = 45;

	//TIMER FOR MULTIPLEXING
	TCCR0A |= (1<<WGM01);
	TCCR0B |= (1<<CS02)|(1<<CS00);
	OCR0A = 32;
	TIMSK0 |= (1<<OCIE0A);

	//BUTTONS
	key_init();
	button_t mode_btn = { &BTN_MODE_PIN, BTN_MODE, 2, next_mode, NULL };
	button_t up_btn = { &BTN_UP_PIN, BTN_UP, 2, up_handler, NULL };
	button_t down_btn = { &BTN_DOWN_PIN, BTN_DOWN, 2, down_handler, NULL };

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
		key_press(&mode_btn);
		key_press(&up_btn);
		key_press(&down_btn);

		if( !Timer2 ) {
			blink ^= 1;
			Timer2 = 33;
		}

		if( !Timer3 ) {
			display = main_screen;
			blink_mask = 0b0000;
			mode = 0;
			Timer3 = 1000;
		}
	}

	return 0;
}
