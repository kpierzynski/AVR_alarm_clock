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
	ALARM_HOUT,
	ALARM_MIN,
	RINGING
} state_t;
state_t state = IDLE;

typedef enum
{
	NONE = 1,
	MODE_BTN,
	UP_BTN,
	DOWN_BTN,
	TIME_UPDATE
} event_t;
event_t event = NONE;

typedef state_t (*callback_t)(void);
typedef struct
{
	state_t state;
	event_t event;
	callback_t callback;
} transition_t;

state_t default_action()
{
	return IDLE;
}

state_t update_time()
{
	clock_update_time(&time);
	return IDLE;
}

#define TRANSITION_COUNT 2
transition_t transitions[TRANSITION_COUNT] = {
	{IDLE, NONE, default_action},
	{IDLE, TIME_UPDATE, update_time},
};

callback_t lookup_transition(state_t s, event_t e)
{
	uint8_t i;
	for (i = 0; i < TRANSITION_COUNT; i++)
	{
		if (transitions[i].state == s && transitions[i].event == e)
			return transitions[i].callback;
	}

	return default_action;
}

void mode_handler()
{
	event = MODE_BTN;
}

void up_handler()
{
	event = UP_BTN;
}

void down_handler()
{
	event = DOWN_BTN;
}

void clock_tick()
{
	event = TIME_UPDATE;
}

int main()
{
	// SOFT TIMERS
	timer_init();

	// DISPLAY INIT
	display_init(&main_screen);

	// BUTTONS
	key_init();
	button_t mode_btn = {&BTN_MODE_PIN, BTN_MODE, 2, mode_handler, NULL};
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
	// DDRD |= (1 << PD1);
	// PORTD &= ~(1 << PD1);

	// timer_create(2, 7500, reset_screen);
	// timer_create(3, 10, ringing_handler);

	sei();
	while (1)
	{
		state = lookup_transition(state, event)();

		key_press(&mode_btn);
		key_press(&up_btn);
		key_press(&down_btn);

		timer_event();

		clock_event();
	}

	return 0;
}
