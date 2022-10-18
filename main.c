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

uint8_t ringing;

typedef enum
{
	IDLE = 1,
	ALARM,
	ALARM_HOUR,
	ALARM_MIN,
	RINGING,
	TIME_HOUR,
	TIME_MIN
} state_t;
state_t state = IDLE;

typedef enum
{
	NONE = 1,
	MODE_BTN,
	UP_BTN,
	DOWN_BTN,
	TIME_UPDATE,
	TIME_CHANGE,
	TIMEOUT,
} event_t;
event_t event = NONE;

typedef state_t (*callback_t)();
typedef struct
{
	state_t state;
	event_t event;
	callback_t callback;
} transition_t;

void update_buf(uint8_t *buf, time_t t)
{
	buf[0] = t.hour / 10;
	buf[1] = t.hour % 10;
	buf[2] = t.min / 10;
	buf[3] = t.min % 10;
}

state_t default_action()
{
	return state;
}

state_t update_time()
{
	clock_update_time(&time);
	ringing = alarm_check(&time);
	update_buf(main_screen.buf, time);
	display_tick();

	if (ringing)
	{
		display_blink(BLINK_BOTH);
		return RINGING;
	}
	return state;
}

uint8_t alarm_index;

state_t show_alarm()
{
	alarm_screen.buf[0] = NUM_A;
	alarm_screen.buf[1] = alarm_index + 1; // + 1 Because there is no alarm number 0 according to humans...
	alarm_screen.buf[2] = NUM_EMPTY;
	alarm_screen.buf[3] = NUM_EMPTY;

	display_set_screen(&alarm_screen);
	return ALARM;
}

state_t loop()
{
	alarm_index = (alarm_index + 1) % ALARM_COUNT;
	return show_alarm();
}

state_t edit_hour()
{
	time_t t = alarm_get_time(alarm_index);
	update_buf(alarm_screen.buf, t);
	display_blink(BLINK_HOUR);

	return ALARM_HOUR;
}

state_t hour_change()
{
	time_t t = {(event == UP_BTN) ? 1 : -1, 0};
	t = alarm_update_time(alarm_index, t);
	update_buf(alarm_screen.buf, t);
	return ALARM_HOUR;
}

state_t edit_min()
{
	time_t t = alarm_get_time(alarm_index);
	update_buf(alarm_screen.buf, t);
	display_blink(BLINK_MIN);

	return ALARM_MIN;
}

state_t min_change()
{
	time_t t = {0, (event == UP_BTN) ? 1 : -1};
	t = alarm_update_time(alarm_index, t);
	update_buf(alarm_screen.buf, t);
	return ALARM_MIN;
}

state_t default_screen()
{
	if (main_screen.is_enabled)
		return state;

	alarm_index = 0;
	display_blink(BLINK_NONE);
	display_set_screen(&main_screen);
	alarm_sync();

	return IDLE;
}

state_t dismiss()
{
	alarm_unarm(ringing - 1);
	ringing = 0;
	display_blink(BLINK_NONE);
	return IDLE;
}

state_t arm()
{
	alarm_flip_arm(alarm_index);
	return ALARM;
}

state_t set_time_hour()
{
	display_blink(BLINK_HOUR);
	return TIME_HOUR;
}

state_t set_time_min() {
	display_blink(BLINK_MIN);
	return TIME_MIN;
}

state_t change_time_hour() {
	time.hour += (event == UP_BTN) ? 1 : -1;
	time.hour = time.hour % 24;
	return TIME_HOUR;
}

state_t change_time_min() {
	time.min += (event == UP_BTN) ? 1 : -1;
	time.min = time.min % 60;
	return TIME_MIN;
}

state_t save_time() {
	display_blink(BLINK_NONE);
	clock_save_time(time);
	return IDLE;
}

#define TRANSITION_COUNT 24
transition_t transitions[TRANSITION_COUNT] = {
	{IDLE, NONE, default_action},
	{IDLE, TIME_UPDATE, update_time},
	{IDLE, MODE_BTN, show_alarm},

	{ALARM, MODE_BTN, edit_hour},
	{ALARM, UP_BTN, arm},
	{ALARM, DOWN_BTN, arm},

	{ALARM_HOUR, UP_BTN, hour_change},
	{ALARM_HOUR, DOWN_BTN, hour_change},
	{ALARM_HOUR, MODE_BTN, edit_min},

	{ALARM_MIN, UP_BTN, min_change},
	{ALARM_MIN, DOWN_BTN, min_change},
	{ALARM_MIN, MODE_BTN, loop},

	{ALARM_MIN, TIMEOUT, default_screen},
	{ALARM_HOUR, TIMEOUT, default_screen},
	{ALARM, TIMEOUT, default_screen},

	{RINGING, MODE_BTN, dismiss},
	{RINGING, TIME_UPDATE, update_time},

	{ALARM, TIME_CHANGE, set_time_hour},
	{TIME_HOUR, UP_BTN, change_time_hour},
	{TIME_HOUR, DOWN_BTN, change_time_hour},

	{TIME_HOUR, MODE_BTN, set_time_min },
	{TIME_MIN, UP_BTN, change_time_min},
	{TIME_MIN, DOWN_BTN, change_time_min},

	{TIME_MIN, MODE_BTN, save_time}
	
};

void lookup_transition(state_t s, event_t e)
{
	uint8_t i;
	for (i = 0; i < TRANSITION_COUNT; i++)
	{
		if (transitions[i].state == s && transitions[i].event == e)
		{
			if (transitions[i].callback)
				state = transitions[i].callback();
			event = NONE;
		}
	}
}

void ringer()
{
	if (!ringing)
	{
		PORTB &= ~(1 << PB1);
		return;
	}

	PORTB ^= (1 << PB1);
}

void timeout()
{
	event = TIMEOUT;
}

void mode_handler()
{
	timer_interval(2, 7500);
	event = MODE_BTN;
}

void up_handler()
{
	timer_interval(2, 7500);
	event = UP_BTN;
}

void down_handler()
{
	timer_interval(2, 7500);
	event = DOWN_BTN;
}

void clock_tick()
{
	event = TIME_UPDATE;
}

void time_edit()
{
	event = TIME_CHANGE;
}

int main()
{
	// SOFT TIMERS
	timer_init();

	// DISPLAY INIT
	display_init(&main_screen);

	// BUTTONS
	key_init();
	button_t mode_btn = {&BTN_MODE_PIN, BTN_MODE, 2, mode_handler, time_edit};
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
	DDRB |= (1 << PB1);
	PORTB &= ~(1 << PB1);

	timer_create(2, 7500, timeout);
	timer_create(3, 10, ringer);

	sei();
	while (1)
	{
		lookup_transition(state, event);

		key_press(&mode_btn);
		key_press(&up_btn);
		key_press(&down_btn);

		timer_event();

		clock_event();
	}

	return 0;
}
