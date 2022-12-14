#ifndef __UTIL_MODULE_H_
#define __UTIL_MODULE_H_

#include <avr/io.h>

// Make sure that modulo result is positive
inline uint8_t modulo_positive( int8_t i, uint8_t n ) {
	return ( ( i % n ) + n ) % n;
}

#endif