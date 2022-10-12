#ifndef __BUTTON_H_
#define __BUTTON_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#include "soft_timer.h"

typedef struct {
	volatile uint8_t *KPIN;
	uint8_t key_mask;
	uint8_t wait_time_s;
	void (*kfun1)(void);
	void (*kfun2)(void);
	uint8_t klock;
	uint8_t flag;
} button_t;

void key_press( button_t * btn );
void key_init();

#endif
