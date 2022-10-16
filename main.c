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
#include "alarm.h"

#include "util/modulo.h"

screen_t main_screen;
screen_t alarm_screen;

time_t time;

uint8_t mode;

uint8_t ringing = 0;

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
		alarm_unarm(ringing - 1);

		PORTD |= (1 << (5 + ringing - 1));

		display_blink(BLINK_NONE);
		ringing = 0;

		alarm_sync();

		return 1;
	}

	return 0;
}

void check_alarms()
{
	uint8_t i;
	for (i = 0; i < ALARM_COUNT; i++)
	{
		if (!alarm_armed(i))
			continue;

		if (time_compare(alarm_get_time(i), time))
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

	uint8_t current_alarm_index = mode / ALARM_COUNT;
	uint8_t type = mode % 3;

	if (type == 0)
	{
		display_blink(BLINK_NONE);
		update_alarm_screen_from_index(current_alarm_index);
	}
	else
	{
		if (type == 1)
			display_blink(BLINK_HOUR);
		else
			display_blink(BLINK_MIN);

		update_alarm_screen_from_time(alarm_get_time(current_alarm_index));
	}
	display_set_screen(&alarm_screen);
	mode = (mode + 1) % (ALARM_COUNT * 3);
}

void handle_change_on_alarm(int8_t d)
{
	reset_main();

	if (main_screen.is_enabled)
		return;

	uint8_t type = modulo_positive(mode - 1, 3);
	uint8_t current_alarm_index = (modulo_positive(mode - 1, ALARM_COUNT * 3)) / 3;

	switch (type)
	{
	case 0:
	{
		uint8_t armed = alarm_flip_arm(current_alarm_index);
		if (armed)
			PORTD &= ~(1 << (5 + current_alarm_index));
		else
			PORTD |= (1 << (5 + current_alarm_index));
		return;
	}
	case 1:
		alarm_update_time(current_alarm_index, (time_t){d, 0});
		break;
	case 2:
		alarm_update_time(current_alarm_index, (time_t){0, d});
		break;
	}
	update_alarm_screen_from_time(alarm_get_time(current_alarm_index));
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

	// ALARM
	alarm_init();

	// BUZZ
	DDRD |= (1 << PD1);
	PORTD &= ~(1 << PD1);

	void reset_screen()
	{
		if (main_screen.is_enabled)
			return;

		alarm_sync();
		display_set_screen(&main_screen);
		mode = 0;

		if (!ringing)
			display_blink(BLINK_NONE);
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
