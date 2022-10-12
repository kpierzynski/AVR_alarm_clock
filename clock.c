#include "clock.h"

volatile uint8_t call_callback;
void (*clock_out_1hz_callback)(void);

void register_clock_out_1hz(void (*callback)(void))
{
	clock_out_1hz_callback = callback;
}

static void tick()
{
	call_callback = 1;
}

void clock_update_time(time_t *time)
{
	uint8_t ss;
	ds1307_get_time(&time->hour, &time->min, &ss);
}

void clock_init()
{
	i2c_init(100);
	ds1307_init();
	register_ds1307_out(tick);
}

void clock_event()
{
	if (call_callback)
	{
		if (clock_out_1hz_callback)
			clock_out_1hz_callback();
		call_callback = 0;
	}
}
