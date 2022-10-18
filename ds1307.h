#ifndef __DS1307_H_
#define __DS1307_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#include "protocols/i2c.h"

/*
	CONFIG SECTION
*/
#define OUT_FREQ DS1307_1HZ

#define INT_VECT INT0_vect
#define INT INT0
#define INT_RISING (1 << ISC01) | (1 << ISC00)
#define INT_FALLING (1 << ISC01)

#define INT_EDGE_DETECT INT_RISING
#define INT_EDGE_REG EICRA
#define INT_ENABLE_REG EIMSK

#define INT_PIN PD2
#define INT_DDR DDRD
#define INT_PORT PORTD
#define INT_IN INT_DDR &= ~(1 << INT_PIN)
#define INT_PULLUP INT_PORT |= (1 << INT_PIN)
/*
	END CONFIG SECTION
*/

#define DS1307_ADDR 0x68

#define CH_REG 0x00
#define SECONDS_REG 0x00
#define MINUTES_REG 0x01
#define HOURS_REG 0x02
#define MODE_12_24_REG 0x02
#define DAY_REG 0x03
#define DATE_REG 0x04
#define MONTH_REG 0x05
#define YEARS_REG 0x06
#define CONTROL_REG 0x07
#define RAM_REG 0x08

// CH - clock halted in 0x00 register
#define DS1307_CH 7

// 12/24 MODE - 0x02 register
#define DS1307_12_24_MODE 6

// 0x07 register
#define DS1307_RS0 0
#define DS1307_RS1 1
#define DS1307_SQWE 4
#define DS1307_OUT 7

// OUT freq
#define DS1307_1HZ (0 << DS1307_RS1) | (0 << DS1307_RS0)
#define DS1307_4096HZ (0 << DS1307_RS1) | (1 << DS1307_RS0)
#define DS1307_8192HZ (1 << DS1307_RS1) | (0 << DS1307_RS0)
#define DS1307_32768HZ (1 << DS1307_RS1) | (1 << DS1307_RS0)

void ds1307_init();
void ds1307_get_time(uint8_t *hh, uint8_t *mm, uint8_t *ss);
void ds1307_set_time(uint8_t hh, uint8_t mm, uint8_t ss);

void ds1307_read_ram_buf(uint8_t address, uint8_t len, uint8_t *buf);
void ds1307_write_ram_buf(uint8_t address, uint8_t len, uint8_t *buf);

void register_ds1307_out(void (*callback)(void));
#endif