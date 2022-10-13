#include "display.h"

screen_t *screen;

volatile uint8_t tick; // Ticking 2 vertical dots.

volatile uint8_t blink_mask; // Mask of blinking digits
volatile uint8_t blink;		 // If should blink

const uint8_t nums[NUM_LEN] = {
	0b00100000, // 0
	0b10111010, // 1
	0b01001000, // 2
	0b00011000, // 3
	0b10010010, // 4
	0b00010100, // 5
	0b00000100, // 6
	0b10111000, // 7
	0b00000000, // 8
	0b00010000, // 9
	0b10000000, // A
	0b00000110, // B
	0b01100100, // C
	0b00001010, // D
	0b01000100, // E
	0b11000100, // F
	0b11011110, // -
	0b11111110	// Empty
};

void display_set_screen(screen_t *s)
{
	screen->is_enabled = 0;
	s->is_enabled = 1;

	screen = s;
}

inline void display_blink(uint8_t mask)
{
	blink_mask = mask;
}

inline void display_blink_reset()
{
	blink_mask = BLINK_NONE;
}

inline void display_tick()
{
	tick ^= 1;
}

static void display_blink_tick()
{
	blink ^= 1;
}

void display_init(screen_t *s)
{
	// TIMER FOR MULTIPLEXING
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS02) | (1 << CS00);
	OCR0A = 32;
	TIMSK0 |= (1 << OCIE0A);

	// DOTS
	DDRD |= (1 << PD5) | (1 << PD6) | (1 << PD7);
	PORTD |= (1 << PD5) | (1 << PD6) | (1 << PD7);

	// DIGIT DATA PORT DIRECTION (use whole port. PB0 for vertical dots)
	DDRB = 0xFF;

	// PLEXER TRANSISTOR DIRECTION
	DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3);

	timer_create(1, 250, display_blink_tick);

	screen = s;
}

ISR(TIMER0_COMPA_vect)
{
	static uint8_t digit = 0;

	PORTC = (PORTC & 0b11110000) | (1 << digit);

	if (blink && (blink_mask & (1 << digit)))
		PORTB = (nums[NUM_EMPTY] | tick);
	else
		PORTB = (nums[screen->buf[digit]] | tick);

	digit = (digit + 1) % DISPLAY_LEN;
}