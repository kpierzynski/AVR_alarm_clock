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

inline time_t alarm_get_time(uint8_t index)
{
	return alarms[index].time;
}

void alarm_sync()
{
	uint8_t i;
	for (i = 0; i < ALARM_COUNT; i++)
		clock_save_alarm(i, alarms + i);
}

void alarm_init()
{
	uint8_t i;
	for (i = 0; i < ALARM_COUNT; i++)
		clock_open_alarm(i, alarms + i);
}