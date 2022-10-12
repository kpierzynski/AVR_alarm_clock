#include "button.h"

static uint8_t flag;
static void flag_handler() {
	flag = 1;
}

void key_init() {
	timer_create( 0, 0, flag_handler );
}

void key_press( button_t * btn ) {
	register uint8_t key_press = (*btn->KPIN & btn->key_mask);

	if( !btn->klock && !key_press ) {
		btn->klock = 1;
		// reakcja na PRESS krótkie wcinięcie klawisza
		if(btn->kfun1) btn->kfun1();
		btn->flag = 1;
		timer_interval( 0,  (btn->wait_time_s*1000)/10 );
	} else if( btn->klock && key_press ) {
		(btn->klock)++;
		if( !btn->klock ) {
			timer_interval(0, 0);
			btn->flag = 0;
		}
	} else if( btn->flag && flag ) {
		// reakcja na dłuższe wcinięcie klawisza
		if(btn->kfun2) btn->kfun2();
		btn->flag = 0;
		flag = 0;
	}
}
