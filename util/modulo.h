#include <avr/io.h>

inline uint8_t modulo_positive(int8_t i, uint8_t n)
{
	return ((i % n) + n) % n;
}