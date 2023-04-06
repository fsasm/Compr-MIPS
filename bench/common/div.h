
#include <stdint.h>

struct divu {
	uint32_t quot;
	uint32_t rem;
};

struct div {
	int32_t quot;
	int32_t rem;
};

struct divu divu32(uint32_t numerator, uint32_t divisor);
struct div  div32(int32_t numerator, int32_t divisor);

