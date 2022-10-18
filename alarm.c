#include "alarm.h"

alarm_t alarms[ALARM_COUNT];
uint8_t dots[ALARM_COUNT] = {(1 << PB6), (1 << PB7), (1 << PB0)};

static void alarm_dot(uint8_t index)
{
	if ((alarms + index)->armed)
		PORTB &= ~dots[index];
	else
		PORTB |= dots[index];
}

inline uint8_t alarm_armed(uint8_t index)
{
	return alarms[index].armed;
}

inline void alarm_arm(uint8_t index)
{
	alarms[index].armed = 1;
	alarm_dot(index);
}

inline void alarm_unarm(uint8_t index)
{
	alarms[index].armed = 0;
	alarm_dot(index);
}

uint8_t alarm_flip_arm(uint8_t index)
{
	alarms[index].armed ^= 1;
	alarm_dot(index);
	return alarms[index].armed;
}

inline time_t alarm_get_time(uint8_t index)
{
	return alarms[index].time;
}

time_t alarm_update_time(uint8_t i, time_t t)
{
	alarms[i].time.hour = modulo_positive(alarms[i].time.hour + t.hour, 24);
	alarms[i].time.min = modulo_positive(alarms[i].time.min + t.min, 60);

	return alarms[i].time;
}

void alarm_sync()
{
	uint8_t i;
	for (i = 0; i < ALARM_COUNT; i++)
		clock_save_alarm(i, &alarms[i]);
}

static uint8_t alarm_verify(alarm_t *alarm)
{
	if (alarm->armed != 0 && alarm->armed != 1)
		return 0;
	if (alarm->time.hour >= 24)
		return 0;
	if (alarm->time.min >= 60)
		return 0;

	return 1;
}

static void alarm_default(alarm_t *alarm)
{
	alarm->armed = 0;
	alarm->time.hour = 0;
	alarm->time.min = 0;
}

uint8_t alarm_check(time_t *time)
{
	uint8_t i;
	for (i = 0; i < ALARM_COUNT; i++)
	{
		alarm_t *alarm = alarms + i;

		if (!alarm->armed)
			continue;

		if (alarm->time.hour == time->hour && alarm->time.min == time->min)
			return i + 1;
	}
	return 0;
}

void alarm_init()
{
	// DOTS
	DDRB |= (1 << PB0) | (1 << PB6) | (1 << PB7);

	uint8_t i;
	for (i = 0; i < ALARM_COUNT; i++)
	{
		clock_open_alarm(i, &alarms[i]);

		if (!alarm_verify(alarms + i))
			alarm_default(alarms + i);

		alarm_dot(i);
	}
}