#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stddef.h>
#include <string.h>

#include "common.h"
#include "button.h"
#include "clock.h"
#include "soft_timer.h"
#include "display.h"

#include "util/modulo.h"

screen_t main_screen;
screen_t alarm_screen;

#define ALARM_LEN 3
alarm_t alarms[ALARM_LEN];
time_t time;

uint8_t mode;

volatile uint8_t ringing = 0;

void update_main_screen()
{
	memcpy(main_screen.buf, (uint8_t[DISPLAY_LEN]){time.hour / 10, time.hour % 10, time.min / 10, time.min % 10}, DISPLAY_LEN);
}

void update_alarm_screen_from_index(uint8_t i)
{
	memcpy(alarm_screen.buf, (uint8_t[DISPLAY_LEN]){NUM_A, i + 1, NUM_EMPTY, NUM_EMPTY}, DISPLAY_LEN);
}

void update_alarm_screen_from_time(time_t t)
{
	memcpy(alarm_screen.buf, (uint8_t[DISPLAY_LEN]){t.hour / 10, t.hour % 10, t.min / 10, t.min % 10}, DISPLAY_LEN);
}

uint8_t handle_dismiss()
{
	if (ringing)
	{
		alarms[ringing - 1].armed = 0;
		PORTD |= (1 << (5 + ringing - 1));

		display_blink_reset();
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
	uint8_t i;
	for (i = 0; i < ALARM_LEN; i++)
	{
		alarm_t *alarm = &alarms[i];

		if (!alarm->armed)
			continue;

		if (time_compare(alarm->time, time))
		{
			ringing = i + 1;
			display_blink(BLINK_BOTH);
		}
	}
}

void clock_tick()
{
	display_tick();

	clock_update_time(&time);
	update_main_screen();
	if (main_screen.is_enabled)
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
		display_blink_reset();
		update_alarm_screen_from_index(current_alarm_index);
	}
	else
	{
		if (type == 1)
			display_blink(BLINK_HOUR);
		else
			display_blink(BLINK_MIN);

		update_alarm_screen_from_time(alarm->time);
	}
	display_set_screen(&alarm_screen);
	mode = (mode + 1) % (ALARM_LEN * 3);
}

void handle_change_on_alarm(int8_t d)
{
	reset_main();

	if (main_screen.is_enabled)
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

	// SOFT TIMERS
	timer_init();

	// DISPLAY INIT
	display_init(&main_screen);

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

	// BUZZ
	DDRD |= (1 << PD1);
	PORTD &= ~(1 << PD1);

	void reset_screen()
	{
		display_set_screen(&main_screen);
		mode = 0;

		if (!ringing)
			display_blink_reset();
	}

	void ringing_handler()
	{
		if (ringing)
		{
			PORTD ^= (1 << PD1);
		}
	}

	timer_create(2, 7500, reset_screen);
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
