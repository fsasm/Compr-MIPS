#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "fix16.h"

/* ungetchar is only available if compiled with the own standard library. */
#ifndef __mips__
#define ungetchar(c) ungetc(c, stdin)
#endif

fix16_t fix_mul(fix16_t left, fix16_t right)
{
	int64_t result = (int64_t)left * (int64_t)right;
	result += (1 << 15); /* round up if fraction is >= 0.5 */
	result /= (1 << 16);
	/* check for overflows and underflows */
	if (result > FIX_MAX) {
		return FIX_MAX;
	}
	if (result < FIX_MIN) {
		return FIX_MIN;
	}

	return (fix16_t)result;
}

static uint32_t abs32(int32_t val)
{
	if (val >= 0)
		return val;

	if (val == INT32_MIN)
		return ((uint32_t)INT32_MAX) + 1;
	
	return -val;
}

static uint32_t fix_divu(uint32_t numerator, uint32_t divisor)
{
	uint64_t quot = 0;
	uint32_t rem = 0;

	uint64_t num = ((uint64_t)numerator) << 16;

	/* long division */
	for (int i = 47; i >= 0; i--) {
		rem = (rem << 1) | ((num >> 47) & 0x01);
		num <<= 1;
		quot <<= 1;

		if (rem >= divisor) {
			rem -= divisor;
			quot |= (1 << 0);
		}
	}

	if (quot >= INT32_MAX)
		quot = INT32_MAX;

	return quot;
}

fix16_t fix_div(fix16_t left, fix16_t right)
{
	if (right == 0)
		return FIX_MAX;

	bool sign_num = left < 0;
	bool sign_div = right < 0;

	uint32_t num = abs32(left);
	uint32_t div = abs32(right);
	fix16_t result = fix_divu(num, div);

	int32_t quot = (sign_num ^ sign_div) ? -result : result;

	return quot;
}

fix16_t fix_add(fix16_t left, fix16_t right)
{
	int64_t a = left;
	int64_t b = right;
	int64_t result = a + b;
	if (result > FIX_MAX) {
		return FIX_MAX;
	}
	if (result < FIX_MIN) {
		return FIX_MIN;
	}
	return (fix16_t)result;
}

fix16_t fix_sub(fix16_t left, fix16_t right)
{
	int64_t a = left;
	int64_t b = right;
	int64_t result = a - b;
	if (result > FIX_MAX) {
		return FIX_MAX;
	}
	if (result < FIX_MIN) {
		return FIX_MIN;
	}
	return (fix16_t)result;
}

fix16_t fix_parse(void)
{
	fix16_t result = 0;
	bool negative = false;

	char c = getchar();

	if (c == '+') {
		negative = false;
		c = getchar();
	} else if (c == '-') {
		negative = true;
		c = getchar();
	}  

	if ('0' <= c && c <= '9') {
		/* integer part */
		result = c - '0';
		do {
			c = getchar();
			if (c < '0' || c > '9') {
				break;
			}
			result = (result * 10) + (c - '0');
		} while (true);
		result <<= 16;
	}

	if (c == '.') {
		/* fractional part */
		uint32_t scale = 1u;
		uint32_t frac = 0;
		do {
			c = getchar();
			if (c < '0' || c > '9') {
				break;
			}
			frac = (frac * 10u) + (c - '0');
			scale = (scale * 10u);
		} while (true);
		ungetchar(c);
		result += (frac << 16) / scale;
	} else {
		ungetchar(c);
	}

	if (negative)
		return -result;
	return result;
}

void fix_print(fix16_t f)
{
	if (f < 0) {
		putchar('-');
		f = -f;
	}

	/* integer part */
	int i = f >> 16;

	char digits[10];
	int num_digits = 0;

	while (num_digits < 10) {
		digits[num_digits] = '0' + (i % 10);
		num_digits++;	

		i /= 10;

		if (i == 0)
			break;
	}
	
	for (int j = num_digits; j > 0; j--) {
		putchar(digits[j - 1]);
	}
	
	/* fractional part */
	putchar('.');
	uint32_t frac = f & 0x0000FFFF;

	do {
		frac *= 10;
		putchar('0' + (frac >> 16));
		frac -= frac & 0xFFFF0000;
	} while(frac != 0);
}

