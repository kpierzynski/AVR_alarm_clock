#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "button.h"

#define LEN 16
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
};

#define DISPLAY_LEN 4
volatile uint8_t display[DISPLAY_LEN] = {0,0,0,0};
volatile uint8_t tick = 0;

typedef struct {
	uint8_t hour;
	uint8_t min;
} time_t;

time_t time;

void set_time( time_t time ) {
	display[0] = time.hour/10;
	display[1] = time.hour%10;

	display[2] = time.min/10;
	display[3] = time.min%10;
}

ISR( TIMER0_COMPA_vect ) {
	static uint8_t digit = 0;

	PORTC = (1<<digit);
	PORTB = ( nums[ display[digit] ] | tick);

	//PORTD = (PORTD & 0b00011111) | ((PIND & 0b00011100)<<3);

	digit = (digit+1) % DISPLAY_LEN;
}

#define BUTTON_MODE		(1<<PD2);
#define BUTTON_MODE_DDR		DDRD
#define BUTTON_MODE_PORT	PORTD
#define BUTTON_MODE_PIN		PIND
#define BUTTON_MODE_IN		BUTTON_MODE_DDR &= ~BUTTON_MODE
#define BUTTON_MODE_OUT		BUTTON_MODE_DDR |= BUTTON_MODE
#define BUTTON_MODE_HIGH	BUTTON_MODE_PORT |= BUTTON_MODE
#define BUTTON_MODE_LOW		BUTTON_MODE_PORT &= ~BUTTON_MODE
#define BUTTON_MODE_STATE	(BUTTON_MODE_PIN & BUTTON_MODE)

int main() {

	//TIMER FOR MULTIPLEXING
	TCCR0A |= (1<<WGM01);
	TCCR0B |= (1<<CS02)|(1<<CS00);
	OCR0A = 32;
	TIMSK0 |= (1<<OCIE0A);

	key_init();

	//DOTS
	DDRD |= (1<<PD5)|(1<<PD6)|(1<<PD7);

	//BUTTONS
	DDRD &= ~(1<<PD2);
	DDRD &= ~(1<<PD3);
	DDRD &= ~(1<<PD4);
	PORTD |= (1<<PD2)|(1<<PD3)|(1<<PD4);

	BUTTON_MODE_IN;
	BUTTON_MODE_HIGH;

	//NUMBER DATA PORT DIRECTION (use whole port. PB0 for dots)
	DDRB = 0xFF;

	//PLEXER TRANSISTOR DIRECTION
	DDRC |= (1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3);

	time.hour = 12;
	time.min = 39;

	sei();
	while( 1 ) {
		set_time( time );
		time.min++;
		tick = (tick+1)%2;

		_delay_ms(1000);
	}

	return 0;
}
