#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stddef.h>

#include "common.h"
#include "button.h"
#include "clock.h"
#include "soft_timer.h"

#include "util/modulo.h"

#define NUM_LEN 18
#define NUM_DASH 0x10
#define NUM_EMPTY 0x11

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
	0b11011110, //-			0x10
	0b11111110	// Empty		0x11
};

#define DISPLAY_LEN 4
uint8_t main_screen[DISPLAY_LEN];
uint8_t alarm_screen[DISPLAY_LEN];
volatile uint8_t *display = main_screen;

#define ALARM_LEN 3
alarm_t alarms[ALARM_LEN];
time_t time;

uint8_t mode;

void update_main_screen()
{
	main_screen[0] = time.hour / 10;
	main_screen[1] = time.hour % 10;
	main_screen[2] = time.min / 10;
	main_screen[3] = time.min % 10;
}

void update_alarm_screen_from_index(uint8_t i)
{
	alarm_screen[0] = 0xA;
	alarm_screen[1] = i + 1; //+1 bo niby nie ma czegos takiego jak alarm 0...
	alarm_screen[2] = NUM_EMPTY;
	alarm_screen[3] = NUM_EMPTY;
}

void update_alarm_screen_from_time(time_t t)
{
	alarm_screen[0] = t.hour / 10;
	alarm_screen[1] = t.hour % 10;
	alarm_screen[2] = t.min / 10;
	alarm_screen[3] = t.min % 10;
}

inline uint8_t is_main_visible()
{
	return (display == main_screen);
}

volatile uint8_t ringing = 0;
volatile uint8_t tick = 0;
volatile uint8_t blink_mask;
volatile uint8_t blink;
ISR(TIMER0_COMPA_vect)
{
	static uint8_t digit = 0;

	PORTC = (PORTC & 0b11110000) | (1 << digit);

	if (blink && (blink_mask & (1 << digit)))
		PORTB = (nums[NUM_EMPTY] | tick);
	else
		PORTB = (nums[display[digit]] | tick);

	digit = (digit + 1) % DISPLAY_LEN;
}

uint8_t handle_dismiss()
{
	if (ringing)
	{
		alarms[ringing - 1].armed = 0;
		PORTD |= (1 << (5 + ringing - 1));
		blink_mask = 0b0000;
		ringing = 0;

		return 1;
	}

	return 0;
}

inline uint8_t time_compare(time_t a, time_t b)
{
	return (a.hour == b.hour && a.min == b.min);
}

void check_alarms()
{
	for (uint8_t i = 0; i < ALARM_LEN; i++)
	{
		alarm_t *alarm = &alarms[i];

		if (!alarm->armed)
			continue;

		if (time_compare(alarm->time, time))
		{
			ringing = i + 1;
			blink_mask = 0b1111;
		}
	}
}

void clock_tick()
{
	tick ^= 1;

	clock_update_time(&time);
	update_main_screen();
	if (is_main_visible())
		check_alarms();
}

inline void reset_main()
{
	timer_interval(2, 7500);
}

void next_mode()
{
	reset_main();

	if (handle_dismiss())
		return;

	uint8_t current_alarm_index = mode / ALARM_LEN;
	uint8_t type = mode % 3;

	alarm_t *alarm = &alarms[current_alarm_index];

	if (type == 0)
	{
		blink_mask = 0b0000;
		update_alarm_screen_from_index(current_alarm_index);
	}
	else
	{
		if (type == 1)
			blink_mask = 0b0011;
		else
			blink_mask = 0b1100;

		update_alarm_screen_from_time(alarm->time);
	}
	display = alarm_screen;
	mode = (mode + 1) % (ALARM_LEN * 3);
}

void handle_change_on_alarm(int8_t d)
{
	reset_main();

	if (is_main_visible())
		return;

	uint8_t type = modulo_positive(mode - 1, 3);
	uint8_t current_alarm_index = (modulo_positive(mode - 1, ALARM_LEN * 3)) / 3;

	alarm_t *alarm = &alarms[current_alarm_index];

	switch (type)
	{
	case 0:
	{
		uint8_t armed = (alarm->armed ^= 1);
		if (armed)
			PORTD &= ~(1 << (5 + current_alarm_index));
		else
			PORTD |= (1 << (5 + current_alarm_index));
		return;
	}
	case 1:
		alarm->time.hour = (alarm->time.hour + d) % 24;
		break;
	case 2:
		alarm->time.min = (alarm->time.min + d) % 60;
		break;
	}
	update_alarm_screen_from_time(alarm->time);
}

void up_handler()
{
	handle_change_on_alarm(1);
}

void down_handler()
{
	handle_change_on_alarm(-1);
}

int main()
{
	alarms[0].time.hour = 12;
	alarms[0].time.min = 30;
	alarms[1].time.hour = 7;
	alarms[1].time.min = 15;
	alarms[2].time.hour = 22;
	alarms[2].time.min = 45;

	// TIMER FOR MULTIPLEXING
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS02) | (1 << CS00);
	OCR0A = 32;
	TIMSK0 |= (1 << OCIE0A);

	// SOFT TIMERS
	timer_init();

	// BUTTONS
	key_init();
	button_t mode_btn = {&BTN_MODE_PIN, BTN_MODE, 2, next_mode, NULL};
	button_t up_btn = {&BTN_UP_PIN, BTN_UP, 2, up_handler, NULL};
	button_t down_btn = {&BTN_DOWN_PIN, BTN_DOWN, 2, down_handler, NULL};

	BTN_MODE_IN;
	BTN_MODE_HIGH;

	BTN_UP_IN;
	BTN_UP_HIGH;

	BTN_DOWN_IN;
	BTN_DOWN_HIGH;

	// CLOCK
	clock_init();
	register_clock_out_1hz(clock_tick);

	// DOTS
	DDRD |= (1 << PD5) | (1 << PD6) | (1 << PD7);
	PORTD |= (1 << PD5) | (1 << PD6) | (1 << PD7);

	// NUMBER DATA PORT DIRECTION (use whole port. PB0 for dots)
	DDRB = 0xFF;

	// PLEXER TRANSISTOR DIRECTION
	DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3);

	// BUZZ
	DDRD |= (1 << PD1);
	PORTD &= ~(1 << PD1);

	void blink_handler()
	{
		blink ^= 1;
	}

	void screen2main_handler()
	{
		display = main_screen;
		mode = 0;

		if (!ringing)
			blink_mask = 0b0000;
	}

	void ringing_handler()
	{
		if (ringing)
		{
			PORTD ^= (1 << PD1);
		}
	}

	timer_create(1, 333, blink_handler);
	timer_create(2, 7500, screen2main_handler);
	timer_create(3, 10, ringing_handler);

	sei();
	while (1)
	{
		key_press(&mode_btn);
		key_press(&up_btn);
		key_press(&down_btn);

		timer_event();

		clock_event();
	}

	return 0;
}
