#include "alarm.h"

alarm_t alarms[ALARM_COUNT];

inline uint8_t alarm_armed(uint8_t index)
{
	return alarms[index].armed;
}

inline void alarm_arm(uint8_t index)
{
	alarms[index].armed = 1;
}

inline void alarm_unarm(uint8_t index)
{
	alarms[index].armed = 0;
}

uint8_t alarm_flip_arm(uint8_t index)
{
	alarms[index].armed ^= 1;
	return alarms[index].armed;
}

inline time_t alarm_get_time(uint8_t index)
{
	return alarms[index].time;
}

inline void alarm_update_time(uint8_t i, time_t t)
{
	alarms[i].time.hour = modulo_positive(alarms[i].time.hour + t.hour, 24);
	alarms[i].time.min = modulo_positive(alarms[i].time.min + t.min, 60);
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

void alarm_init()
{
	uint8_t i;
	for (i = 0; i < ALARM_COUNT; i++)
	{
		clock_open_alarm(i, &alarms[i]);

		if (!alarm_verify(alarms + i))
		{
			alarm_default(alarms + i);
		}

		if (alarms[i].armed)
			PORTD &= ~(1 << (5 + i));
		else
			PORTD |= (1 << (5 + i));
	}
}