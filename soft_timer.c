#include "soft_timer.h"

volatile uint16_t TimerButton;
timer_t			  timers[ MAX_TIMER_COUNT ];

static void timer_process( timer_t * t ) {
	if ( !t->cnt ) {
		if ( t->callback )
			t->callback();

		t->cnt = t->interval / 10;
	}
}

void timer_create( uint8_t index, uint16_t interval, void ( *callback )() ) {
	timers[ index ] = ( timer_t ){ interval / 10, interval, 1, callback };
}

void timer_interval( uint8_t index, uint16_t interval ) {
	register uint8_t sreg = SREG;
	cli();
	timers[ index ].interval = interval;
	timers[ index ].cnt = interval / 10;
	SREG = sreg;
}

void timer_event() {
	for ( uint8_t i = 0; i < MAX_TIMER_COUNT; i++ ) {
		timer_t * timer = &timers[ i ];

		if ( !timer->enabled )
			continue;

		timer_process( timer );
	}
}

void timer_init() {
	// TIMER 10ms
	TCCR2A |= ( 1 << WGM21 );									// CTC
	TCCR2B |= ( 1 << CS22 ) | ( 1 << CS21 ) | ( 1 << CS20 );	// 1024 preskaler
	OCR2A = 77;													// 10ms
	TIMSK2 |= ( 1 << OCIE2A );									// Enable interrupt
}

ISR( TIMER2_COMPA_vect ) {
	uint16_t n;

	n = TimerButton;
	if ( n )
		TimerButton = --n;

	for ( uint8_t i = 0; i < MAX_TIMER_COUNT; i++ ) {
		n = timers[ i ].cnt;
		if ( n )
			timers[ i ].cnt = --n;
	}
}