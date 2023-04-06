/*
 * @file sha256.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-04-04
 */

#include "sha256.h"

static const uint32_t table[64] = {
	0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
	0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5, /*  8 */
	0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
	0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174, /* 16 */
	0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
	0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA, /* 24 */
	0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
	0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967, /* 32 */
	0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
	0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85, /* 40 */
	0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 
	0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070, /* 48 */
	0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
	0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3, /* 56 */
	0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
	0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2  /* 64 */
};

void sha256_init_hash(uint32_t hash[8])
{
	hash[0] = 0x6A09E667;
	hash[1] = 0xBB67AE85;
	hash[2] = 0x3C6EF372;
	hash[3] = 0xA54FF53A;
	hash[4] = 0x510E527F;
	hash[5] = 0x9B05688C;
	hash[6] = 0x1F83D9AB;
	hash[7] = 0x5BE0CD19;
}

static void u32_to_u8_be(uint32_t from, uint8_t to[4])
{
	for (int i = 0; i < 4; i++) {
		to[3 - i] = (uint8_t)(from & 0xFFu);
		from = from >> 8;
	}
}

static uint32_t u8_to_u32_be(const uint8_t from[4])
{
	uint32_t to = 0;
	for (int i = 0; i < 4; i++) {
		to = (to << 8) | from[i];
	}
	return to;
}

static uint32_t ror32(uint32_t value, unsigned int times)
{
	times %= 32;
	if (times == 0)
		return value;
	
	return (value >> times) | (value << (32 - times));
}

void sha256_hash_to_digest(uint32_t hash[8], uint8_t digest[SHA256_DIGEST_SIZE])
{
	for (int i = 0; i < 8; i++) {
		u32_to_u8_be(hash[i], &digest[i * 4]);
	}
}

static uint32_t f1(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) ^ (~x & z);
}

static uint32_t f2(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) ^ (x & z) ^ (y & z);
}

static uint32_t f3(uint32_t x)
{
	return ror32(x, 2) ^ ror32(x, 13) ^ ror32(x, 22);
}

static uint32_t f4(uint32_t x)
{
	return ror32(x, 6) ^ ror32(x, 11) ^ ror32(x, 25);
}

static uint32_t f5(uint32_t x)
{
	return ror32(x, 7) ^ ror32(x, 18) ^ (x >> 3);
}

static uint32_t f6(uint32_t x)
{
	return ror32(x, 17) ^ ror32(x, 19) ^ (x >> 10);
}

#ifndef UNROLLED
void sha256_process_block(const uint8_t block[restrict 64], uint32_t hash[restrict 8])
{
	uint32_t a = hash[0];
	uint32_t b = hash[1];
	uint32_t c = hash[2];
	uint32_t d = hash[3];
	uint32_t e = hash[4];
	uint32_t f = hash[5];
	uint32_t g = hash[6];
	uint32_t h = hash[7];
	
	uint32_t temp1;
	uint32_t temp2;
	uint32_t W[64];
	
	for (uint_fast8_t i = 0; i < 16; i++) {
		W[i] = u8_to_u32_be (&block[i * 4]);
	}
	
	for (uint_fast8_t i = 16; i < 64; i++) {
		W[i] = f6(W[i - 2]) + W[i - 7] + f5(W[i - 15]) + W[i - 16];
	}
	
	for (uint_fast8_t t = 0; t < 64; t++) {
		temp1 = h + f4(e) + f1(e, f, g) + table[t] + W[t];
		temp2 = f3(a) + f2(a, b, c);
		
		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}
	
	hash[0] += a;
	hash[1] += b;
	hash[2] += c;
	hash[3] += d;
	hash[4] += e;
	hash[5] += f;
	hash[6] += g;
	hash[7] += h;
}
#else
void sha256_process_block(const uint8_t block[restrict 64], uint32_t hash[restrict 8])
{
	uint32_t a = hash[0];
	uint32_t b = hash[1];
	uint32_t c = hash[2];
	uint32_t d = hash[3];
	uint32_t e = hash[4];
	uint32_t f = hash[5];
	uint32_t g = hash[6];
	uint32_t h = hash[7];
	
	uint32_t temp1;
	uint32_t temp2;
	uint32_t W[16];

#define CALC_W1(i) \
	W[i] = u8_to_u32_be (&block[i * 4])
#define CALC_W2(i) \
	W[i % 16] = f6(W[(i - 2) % 16]) + W[(i - 7) % 16] + \
	f5(W[(i - 15) % 16]) + W[(i - 16) % 16]

#define ROUND1(t, ra, rb, rc, rd, re, rf, rg, rh) \
	do { \
		CALC_W1(t); \
		temp1 = rh + f4(re) + f1(re, rf, rg) + table[t] + W[t]; \
		temp2 = f3(ra) + f2(ra, rb, rc); \
		rd += temp1; \
		rh = temp1 + temp2; \
	} while(0)
	
#define ROUND2(t, ra, rb, rc, rd, re, rf, rg, rh) \
	do { \
		CALC_W2(t); \
		temp1 = rh + f4(re) + f1(re, rf, rg) + table[t] + W[t % 16]; \
		temp2 = f3(ra) + f2(ra, rb, rc); \
		rd += temp1; \
		rh = temp1 + temp2; \
	} while(0)

	ROUND1( 0, a, b, c, d, e, f, g, h);
	ROUND1( 1, h, a, b, c, d, e, f, g);
	ROUND1( 2, g, h, a, b, c, d, e, f);
	ROUND1( 3, f, g, h, a, b, c, d, e);
	ROUND1( 4, e, f, g, h, a, b, c, d);
	ROUND1( 5, d, e, f, g, h, a, b, c);
	ROUND1( 6, c, d, e, f, g, h, a, b);
	ROUND1( 7, b, c, d, e, f, g, h, a);

	ROUND1( 8, a, b, c, d, e, f, g, h);
	ROUND1( 9, h, a, b, c, d, e, f, g);
	ROUND1(10, g, h, a, b, c, d, e, f);
	ROUND1(11, f, g, h, a, b, c, d, e);
	ROUND1(12, e, f, g, h, a, b, c, d);
	ROUND1(13, d, e, f, g, h, a, b, c);
	ROUND1(14, c, d, e, f, g, h, a, b);
	ROUND1(15, b, c, d, e, f, g, h, a);
	
	ROUND2(16, a, b, c, d, e, f, g, h);
	ROUND2(17, h, a, b, c, d, e, f, g);
	ROUND2(18, g, h, a, b, c, d, e, f);
	ROUND2(19, f, g, h, a, b, c, d, e);
	ROUND2(20, e, f, g, h, a, b, c, d);
	ROUND2(21, d, e, f, g, h, a, b, c);
	ROUND2(22, c, d, e, f, g, h, a, b);
	ROUND2(23, b, c, d, e, f, g, h, a);

	ROUND2(24, a, b, c, d, e, f, g, h);
	ROUND2(25, h, a, b, c, d, e, f, g);
	ROUND2(26, g, h, a, b, c, d, e, f);
	ROUND2(27, f, g, h, a, b, c, d, e);
	ROUND2(28, e, f, g, h, a, b, c, d);
	ROUND2(29, d, e, f, g, h, a, b, c);
	ROUND2(30, c, d, e, f, g, h, a, b);
	ROUND2(31, b, c, d, e, f, g, h, a);

	ROUND2(32, a, b, c, d, e, f, g, h);
	ROUND2(33, h, a, b, c, d, e, f, g);
	ROUND2(34, g, h, a, b, c, d, e, f);
	ROUND2(35, f, g, h, a, b, c, d, e);
	ROUND2(36, e, f, g, h, a, b, c, d);
	ROUND2(37, d, e, f, g, h, a, b, c);
	ROUND2(38, c, d, e, f, g, h, a, b);
	ROUND2(39, b, c, d, e, f, g, h, a);

	ROUND2(40, a, b, c, d, e, f, g, h);
	ROUND2(41, h, a, b, c, d, e, f, g);
	ROUND2(42, g, h, a, b, c, d, e, f);
	ROUND2(43, f, g, h, a, b, c, d, e);
	ROUND2(44, e, f, g, h, a, b, c, d);
	ROUND2(45, d, e, f, g, h, a, b, c);
	ROUND2(46, c, d, e, f, g, h, a, b);
	ROUND2(47, b, c, d, e, f, g, h, a);

	ROUND2(48, a, b, c, d, e, f, g, h);
	ROUND2(49, h, a, b, c, d, e, f, g);
	ROUND2(50, g, h, a, b, c, d, e, f);
	ROUND2(51, f, g, h, a, b, c, d, e);
	ROUND2(52, e, f, g, h, a, b, c, d);
	ROUND2(53, d, e, f, g, h, a, b, c);
	ROUND2(54, c, d, e, f, g, h, a, b);
	ROUND2(55, b, c, d, e, f, g, h, a);

	ROUND2(56, a, b, c, d, e, f, g, h);
	ROUND2(57, h, a, b, c, d, e, f, g);
	ROUND2(58, g, h, a, b, c, d, e, f);
	ROUND2(59, f, g, h, a, b, c, d, e);
	ROUND2(60, e, f, g, h, a, b, c, d);
	ROUND2(61, d, e, f, g, h, a, b, c);
	ROUND2(62, c, d, e, f, g, h, a, b);
	ROUND2(63, b, c, d, e, f, g, h, a);

	hash[0] += a;
	hash[1] += b;
	hash[2] += c;
	hash[3] += d;
	hash[4] += e;
	hash[5] += f;
	hash[6] += g;
	hash[7] += h;
}

#endif

