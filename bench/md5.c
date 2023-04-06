/*
 * md5.c
 * Author: Fabjan Sukalia <fsukalia@gmail.com>
 * Date: 2016-07-13
 */

#include "md5.h"

static const uint32_t table[64] = {
	0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
	0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501, /*  8 */
	0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
	0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821, /* 16 */
	0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
	0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8, /* 24 */
	0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
	0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A, /* 32 */
	0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
	0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70, /* 40 */
	0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
	0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665, /* 48 */
	0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
	0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1, /* 56 */
	0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
	0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391  /* 64 */
};

static const uint_fast8_t md5_shift[16] = {
	7, 12, 17, 22,
	5,  9, 14, 20,
	4, 11, 16, 23,
	6, 10, 15, 21
};

static void u32_to_u8_le (uint32_t from, uint8_t to[4]) {
	for (int i = 0; i < 4; i++) {
		to[i] = (uint8_t)(from & 0xFFu);
		from = from >> 8;
	}
}

static uint32_t u8_to_u32_le (const uint8_t from[4]) {
	uint32_t to = 0;
	for (int i = 0; i < 4; i++) {
		to = (to << 8) | from[3 - i];
	}
	return to;
}

static uint32_t rol32(uint32_t value, unsigned int times)
{
	times %= 32;
	if (times == 0)
		return value;
	
	return (value << times) | (value >> (32 - times));
}

void md5_init_hash(uint32_t hash[4]) {
	hash[0] = 0x67452301;
	hash[1] = 0xEFCDAB89;
	hash[2] = 0x98BADCFE;
	hash[3] = 0x10325476;
}

void md5_hash_to_digest(uint32_t hash[4], uint8_t digest[MD5_DIGEST_SIZE]) {
	for (int i = 0; i < 4; i++) {
		u32_to_u8_le(hash[i], &digest[i * 4]);
	}
}

#ifndef UNROLLED
void md5_process_block(const uint8_t block[64], uint32_t hash[4]) {
	uint32_t a = hash[0];
	uint32_t b = hash[1];
	uint32_t c = hash[2];
	uint32_t d = hash[3];

	for (int i = 0; i < 16; i++) {
		uint32_t f = ((c ^ d) & b) ^ d;
		a += f + table[i] + u8_to_u32_le(&block[i * 4]);
		uint_fast8_t shift = md5_shift[i & 0x03];
		a = b + rol32(a, shift);

		uint32_t temp = a;
		a = d;
		d = c;
		c = b;
		b = temp;
	}

	for (int i = 16; i < 32; i++) {
		uint_fast8_t block_index = (i * 5 + 1) & 0x0F;
		uint32_t f = (d & b) | (~d & c);
		a += f + table[i] + u8_to_u32_le(&block[block_index * 4]);
		uint_fast8_t shift = md5_shift[(i & 0x03) + 4];
		a = b + rol32(a, shift);

		uint32_t temp = a;
		a = d;
		d = c;
		c = b;
		b = temp;
	}

	for (int i = 32; i < 48; i++) {
		uint_fast8_t block_index = (i * 3 + 5) & 0x0F;
		uint32_t f = b ^ c ^ d;
		a += f + table[i] + u8_to_u32_le(&block[block_index * 4]);
		uint_fast8_t shift = md5_shift[(i & 0x03) + 8];
		a = b + rol32(a, shift);

		uint32_t temp = a;
		a = d;
		d = c;
		c = b;
		b = temp;
	}

	for (int i = 48; i < 64; i++) {
		uint_fast8_t block_index = (i * 7) & 0x0F;
		uint32_t f = c ^ (b | ~d);
		a += f + table[i] + u8_to_u32_le(&block[block_index * 4]);
		uint_fast8_t shift = md5_shift[(i & 0x03) + 12];
		a = b + rol32(a, shift);

		uint32_t temp = a;
		a = d;
		d = c;
		c = b;
		b = temp;
	}

	hash[0] += a;
	hash[1] += b;
	hash[2] += c;
	hash[3] += d;
}
#else
void md5_process_block(const uint8_t block[64], uint32_t hash[4]) {
	uint32_t a = hash[0];
	uint32_t b = hash[1];
	uint32_t c = hash[2];
	uint32_t d = hash[3];

	uint32_t f; 
	uint_fast8_t shift;

#define ROUND1(r, ra, rb, rc, rd) \
	do { \
		f = ((rc ^ rd) & rb) ^ rd; \
		ra += f + table[r] + u8_to_u32_le(&block[r * 4]); \
		shift = md5_shift[r & 0x03]; \
		ra = rb + rol32(ra, shift); \
	} while(0)

#define ROUND2(r, ra, rb, rc, rd) \
	do { \
		f = (rd & rb) | (~rd & rc); \
		ra += f + table[r] + u8_to_u32_le(&block[((r * 5 + 1) & 0x0F) * 4]); \
		shift = md5_shift[(r & 0x03) + 4]; \
		ra = rb + rol32(ra, shift); \
	} while(0)

#define ROUND3(r, ra, rb, rc, rd) \
	do { \
		f = rb ^ rc ^ rd; \
		ra += f + table[r] + u8_to_u32_le(&block[((r * 3 + 5) & 0x0F) * 4]); \
		shift = md5_shift[(r & 0x03) + 8]; \
		ra = rb + rol32(ra, shift); \
	} while(0)

#define ROUND4(r, ra, rb, rc, rd) \
	do { \
		f = rc ^ (rb | ~rd); \
		ra += f + table[r] + u8_to_u32_le(&block[((r * 7) & 0x0F) * 4]); \
		shift = md5_shift[(r & 0x03) + 12]; \
		ra = rb + rol32(ra, shift); \
	} while(0)

	ROUND1( 0, a, b, c, d);
	ROUND1( 1, d, a, b, c);
	ROUND1( 2, c, d, a, b);
	ROUND1( 3, b, c, d, a);
	ROUND1( 4, a, b, c, d);
	ROUND1( 5, d, a, b, c);
	ROUND1( 6, c, d, a, b);
	ROUND1( 7, b, c, d, a);
	ROUND1( 8, a, b, c, d);
	ROUND1( 9, d, a, b, c);
	ROUND1(10, c, d, a, b);
	ROUND1(11, b, c, d, a);
	ROUND1(12, a, b, c, d);
	ROUND1(13, d, a, b, c);
	ROUND1(14, c, d, a, b);
	ROUND1(15, b, c, d, a);

	ROUND2(16, a, b, c, d);
	ROUND2(17, d, a, b, c);
	ROUND2(18, c, d, a, b);
	ROUND2(19, b, c, d, a);
	ROUND2(20, a, b, c, d);
	ROUND2(21, d, a, b, c);
	ROUND2(22, c, d, a, b);
	ROUND2(23, b, c, d, a);
	ROUND2(24, a, b, c, d);
	ROUND2(25, d, a, b, c);
	ROUND2(26, c, d, a, b);
	ROUND2(27, b, c, d, a);
	ROUND2(28, a, b, c, d);
	ROUND2(29, d, a, b, c);
	ROUND2(30, c, d, a, b);
	ROUND2(31, b, c, d, a);

	ROUND3(32, a, b, c, d);
	ROUND3(33, d, a, b, c);
	ROUND3(34, c, d, a, b);
	ROUND3(35, b, c, d, a);
	ROUND3(36, a, b, c, d);
	ROUND3(37, d, a, b, c);
	ROUND3(38, c, d, a, b);
	ROUND3(39, b, c, d, a);
	ROUND3(40, a, b, c, d);
	ROUND3(41, d, a, b, c);
	ROUND3(42, c, d, a, b);
	ROUND3(43, b, c, d, a);
	ROUND3(44, a, b, c, d);
	ROUND3(45, d, a, b, c);
	ROUND3(46, c, d, a, b);
	ROUND3(47, b, c, d, a);

	ROUND4(48, a, b, c, d);
	ROUND4(49, d, a, b, c);
	ROUND4(50, c, d, a, b);
	ROUND4(51, b, c, d, a);
	ROUND4(52, a, b, c, d);
	ROUND4(53, d, a, b, c);
	ROUND4(54, c, d, a, b);
	ROUND4(55, b, c, d, a);
	ROUND4(56, a, b, c, d);
	ROUND4(57, d, a, b, c);
	ROUND4(58, c, d, a, b);
	ROUND4(59, b, c, d, a);
	ROUND4(60, a, b, c, d);
	ROUND4(61, d, a, b, c);
	ROUND4(62, c, d, a, b);
	ROUND4(63, b, c, d, a);

	hash[0] += a;
	hash[1] += b;
	hash[2] += c;
	hash[3] += d;
}

#endif

