#include <stdint.h>

uint8_t
max(uint8_t a, uint8_t b)
{
	return (a > b ? a : b);
}

void
max3(uint8_t* in, uint8_t** out)
{
	*(out[0]) = max(max(in[0], in[1]), in[2]);
}

