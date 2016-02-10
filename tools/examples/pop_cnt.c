#include <stdint.h>

void
pop_cnt(uint8_t* in, uint8_t** out)
{
	*(out[0]) = 0;

	while (in[0] > 0) {
		if (in[0] & 1)
			(*(out[0]))++;
		in[0] >>= 1;
	}
}

