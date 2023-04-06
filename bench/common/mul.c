#include "mul.h"
#include <stdbool.h>

uint64_t mulu32(uint32_t left, uint32_t right)
{
	if (left == 0 || right == 0)
		return 0;

	uint64_t a = left;
	uint64_t b = right;

	uint64_t result = 0;
	for (int i = 0; i < 32 && b > 0; i++) {
		if ((b & 1) == 1) {
			result += a;
		}

		a <<= 1;
		b >>= 1;
	}

	return result;
}

static uint32_t abs32(int32_t val)
{
	if (val >= 0)
		return val;

	if (val == INT32_MIN)
		return ((uint32_t)INT32_MAX) + 1;
	
	return -val;
}

int64_t mul32(int32_t left, int32_t right)
{
	bool sign = (left < 0) ^ (right < 0);

	uint32_t a = abs32(left);
	uint32_t b = abs32(right);
	uint64_t result = mulu32(a, b);

	if (sign)
		return -result;
	return result;
}

int64_t mulsu32(int32_t left, uint32_t right)
{
	bool sign = left < 0;
	uint32_t a = abs32(left);
	uint64_t result = mulu32(a, right);

	if (sign)
		return -result;
	return result;
}

