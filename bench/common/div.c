#include "div.h"
#include <stdbool.h>

static uint32_t abs32(int32_t val)
{
	if (val >= 0)
		return val;

	if (val == INT32_MIN)
		return ((uint32_t)INT32_MAX) + 1;
	
	return -val;
}

struct divu divu32(uint32_t numerator, uint32_t divisor)
{
	if (divisor == 0) {
		/* don't raise a Division-by-zero exception */
		return (struct divu) {UINT32_MAX, numerator};
	}

	uint32_t quot = 0;
	uint32_t rem = 0;
	/* long division */
	for (int i = 31; i >= 0; i--) {
		rem = (rem << 1) | ((numerator >> 31) & 0x01);
		numerator <<= 1;
		quot <<= 1;

		if (rem >= divisor) {
			rem -= divisor;
			quot |= (1 << 0);
		}
	}

	return (struct divu) {quot, rem};
}

struct div div32(int32_t numerator, int32_t divisor)
{
	bool sign_num = numerator < 0;
	bool sign_div = divisor < 0;

	uint32_t num = abs32(numerator);
	uint32_t div = abs32(divisor);
	struct divu result = divu32(num, div);

	int32_t quot = (sign_num ^ sign_div) ? -result.quot : result.quot;
	int32_t rem  = sign_num ? -result.rem : result.rem;

	return (struct div) {quot, rem};
}

