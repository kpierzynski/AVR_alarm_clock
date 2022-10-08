#include "button.h"

void key_press( TBUTTON * btn ) {
	register uint8_t key_press = (*btn->KPIN & btn->key_mask);

	if( !btn->klock && !key_press ) {
		btn->klock=1;
		// reakcja na PRESS krótkie wcinięcie klawisza
		if(btn->kfun1) btn->kfun1();
		btn->flag=1;
		Timer1 = (btn->wait_time_s*1000)/10;
	} else if( btn->klock && key_press ) {
		(btn->klock)++;
		if( !btn->klock ) {
			Timer1=0;
			btn->flag=0;
		}
	} else if( btn->flag && !Timer1 ) {
		// reakcja na dłuższe wcinięcie klawisza
		if(btn->kfun2) btn->kfun2();
		btn->flag=0;
	}
}
