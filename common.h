#ifndef __COMMON_H_
#define __COMMON_H_

#include <avr/io.h>

typedef struct
{
	uint8_t hour;
	uint8_t min;
} time_t;

typedef struct
{
	time_t	time;
	uint8_t armed;
} alarm_t;

#define BTN_MODE	   ( 1 << PD5 )
#define BTN_MODE_DDR   DDRD
#define BTN_MODE_PORT  PORTD
#define BTN_MODE_PIN   PIND
#define BTN_MODE_IN	   BTN_MODE_DDR &= ~BTN_MODE
#define BTN_MODE_OUT   BTN_MODE_DDR |= BTN_MODE
#define BTN_MODE_HIGH  BTN_MODE_PORT |= BTN_MODE
#define BTN_MODE_LOW   BTN_MODE_PORT &= ~BTN_MODE
#define BTN_MODE_STATE ( BTN_MODE_PIN & BTN_MODE )

#define BTN_UP		 ( 1 << PD6 )
#define BTN_UP_DDR	 DDRD
#define BTN_UP_PORT	 PORTD
#define BTN_UP_PIN	 PIND
#define BTN_UP_IN	 BTN_UP_DDR &= ~BTN_UP
#define BTN_UP_OUT	 BTN_UP_DDR |= BTN_UP
#define BTN_UP_HIGH	 BTN_UP_PORT |= BTN_UP
#define BTN_UP_LOW	 BTN_UP_PORT &= ~BTN_UP
#define BTN_UP_STATE ( BTN_UP_PIN & BTN_UP )

#define BTN_DOWN	   ( 1 << PD7 )
#define BTN_DOWN_DDR   DDRD
#define BTN_DOWN_PORT  PORTD
#define BTN_DOWN_PIN   PIND
#define BTN_DOWN_IN	   BTN_DOWN_DDR &= ~BTN_DOWN
#define BTN_DOWN_OUT   BTN_DOWN_DDR |= BTN_DOWN
#define BTN_DOWN_HIGH  BTN_DOWN_PORT |= BTN_DOWN
#define BTN_DOWN_LOW   BTN_DOWN_PORT &= ~BTN_DOWN
#define BTN_DOWN_STATE ( BTN_DOWN_PIN & BTN_DOWN )

#endif
