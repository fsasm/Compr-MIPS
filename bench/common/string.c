
#include <string.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n)
{
	uint8_t *d = dest;
	const uint8_t *s = src;

	for (size_t i = 0; i < n; i++) {
		*(d + i) = *(s + i);	
	}
	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
	uint8_t *d = dest;
	const uint8_t *s = src;

	if (s < d && (s + n) > d) {
		/* the upper part of the source region overlaps with the destination region */
		for (size_t i = 0; i < n; i++) {
			size_t index = n - i - 1;
			*(d + index) = *(s + index);
		}
	} else {
		for (size_t i = 0; i < n; i++) {
			*(d + i) = *(s + i);
		}
	}

	return dest;
}

size_t strlen(const char *s) 
{
	size_t i = 0;
	while (*s++ != '\0') {
		i++;
	}
	return i;
}

int strcmp(const char *s1, const char *s2) {
	for (;;) {
		char a = *s1++;
		char b = *s2++;
		if (a == '\0' && b == '\0') {
			return 0;
		}
		int d = a - b;
		if (d != 0) {
			return d;
		}
	}
}

void *memset(void *s, int c, size_t n)
{
	uint8_t *ptr = s;
	for (size_t i = 0; i < n; i++) {
		ptr[i] = (uint8_t)(c & 0xFF);
	}
	return s;
}

