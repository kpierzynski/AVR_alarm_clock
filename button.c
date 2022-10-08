#include "button.h"

volatile uint16_t Timer1;

void key_init() {
	//TIMER 10ms
	TCCR2A |= (1<<WGM21);				//CTC
	TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20);	//1024 preskaler
	OCR2A = 77;
	TIMSK2 |= (1<<OCIE2A);
}

ISR( TIMER2_COMPA_vect ) {
	uint16_t n;

	n = Timer1;
	if( n ) Timer1 = --n;
}

void key_press( button_t * btn ) {
	register uint8_t key_press = (*btn->KPIN & btn->key_mask);

	if( !btn->klock && !key_press ) {
		btn->klock = 1;
		// reakcja na PRESS krótkie wcinięcie klawisza
		if(btn->kfun1) btn->kfun1();
		btn->flag = 1;
		Timer1 = (btn->wait_time_s*1000)/10;
	} else if( btn->klock && key_press ) {
		(btn->klock)++;
		if( !btn->klock ) {
			Timer1 = 0;
			btn->flag = 0;
		}
	} else if( btn->flag && !Timer1 ) {
		// reakcja na dłuższe wcinięcie klawisza
		if(btn->kfun2) btn->kfun2();
		btn->flag = 0;
	}
}
