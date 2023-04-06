/*
 * @file sha512.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-04-05
 */

#include "sha512.h"

static const uint64_t sha512_table[80] = {
	0x428A2f98D728AE22, 0x7137449123EF65CD, 0xB5C0FBCFEC4D3B2F, 0xE9B5DBA58189DBBC,
	0x3956C25BF348B538, 0x59F111F1B605D019, 0x923F82A4AF194F9B, 0xAB1C5ED5DA6D8118, /*  8 */
	0xD807AA98A3030242, 0x12835B0145706FBE, 0x243185BE4EE4B28C, 0x550C7DC3D5FFB4E2,
	0x72BE5D74F27B896F, 0x80DEB1FE3B1696B1, 0x9BDC06A725C71235, 0xC19BF174CF692694, /* 16 */
	0xE49B69C19EF14AD2, 0xEFBE4786384F25E3, 0x0FC19DC68B8CD5B5, 0x240CA1CC77AC9C65,
	0x2DE92C6F592B0275, 0x4A7484AA6EA6E483, 0x5CB0A9DCBD41FBD4, 0x76F988DA831153B5, /* 24 */
	0x983E5152EE66DFAB, 0xA831C66D2DB43210, 0xB00327C898FB213F, 0xBF597FC7BEEF0EE4,
	0xC6E00BF33DA88FC2, 0xD5A79147930AA725, 0x06CA6351E003826F, 0x142929670A0E6E70, /* 32 */
	0x27B70A8546D22FFC, 0x2E1B21385C26C926, 0x4D2C6DFC5AC42AED, 0x53380D139D95B3DF,
	0x650A73548BAF63DE, 0x766A0ABB3C77B2A8, 0x81C2C92E47EDAEE6, 0x92722C851482353B, /* 40 */
	0xA2BFE8A14CF10364, 0xA81A664BBC423001, 0xC24B8B70D0F89791, 0xC76C51A30654BE30,
	0xD192E819D6EF5218, 0xD69906245565A910, 0xF40E35855771202A, 0x106AA07032BBD1B8, /* 48 */
	0x19A4C116B8D2D0C8, 0x1E376C085141AB53, 0x2748774CDF8EEB99, 0x34B0BCB5E19B48A8,
	0x391C0CB3C5C95A63, 0x4ED8AA4AE3418ACB, 0x5B9CCA4F7763E373, 0x682E6FF3D6B2B8A3, /* 56 */
	0x748F82EE5DEFB2FC, 0x78A5636F43172F60, 0x84C87814A1F0AB72, 0x8CC702081A6439EC,
	0x90BEFFFA23631E28, 0xA4506CEBDE82BDE9, 0xBEF9A3F7B2C67915, 0xC67178F2E372532B, /* 64 */
	0xCA273ECEEA26619C, 0xD186B8C721C0C207, 0xEADA7DD6CDE0EB1E, 0xF57D4F7FEE6ED178,
	0x06F067AA72176FBA, 0x0A637DC5A2C898A6, 0x113F9804BEF90DAE, 0x1B710B35131C471B, /* 72 */
	0x28DB77F523047D84, 0x32CAAB7B40C72493, 0x3C9EBE0A15C9BEBC, 0x431D67C49C100D4C,
	0x4CC5D4BECB3E42B6, 0x597F299CFC657E2A, 0x5FCB6FAB3AD6FAEC, 0x6C44198C4A475817  /* 80 */
};

void sha512_init_hash(uint64_t hash[8]) {
	hash[0] = 0x6A09E667F3BCC908;
	hash[1] = 0xBB67AE8584CAA73B;
	hash[2] = 0x3C6EF372FE94F82B;
	hash[3] = 0xA54FF53A5F1D36F1;
	hash[4] = 0x510E527FADE682D1;
	hash[5] = 0x9B05688C2B3E6C1F;
	hash[6] = 0x1F83D9ABFB41BD6B;
	hash[7] = 0x5BE0CD19137E2179;
}

static void u64_to_u8_be (uint64_t from, uint8_t to[8]) {
	for (int i = 0; i < 8; i++) {
		to[7 - i] = (uint8_t)(from & 0xFFu);
		from = from >> 8;
	}
}

static uint64_t u8_to_u64_be (const uint8_t from[8]) {
	uint64_t to = 0;
	for (int i = 0; i < 8; i++) {
		to = (to << 8) | from[i];
	}
	return to;
}

static uint64_t ror64(uint64_t value, unsigned int times)
{
	times %= 64;
	if (times == 0)
		return value;
	
	return (value >> times) | (value << (64 - times));
}

void sha512_hash_to_digest(uint64_t hash[8], uint8_t digest[SHA512_DIGEST_SIZE]) {
	for (int i = 0; i < 8; i++) {
		u64_to_u8_be(hash[i], &digest[i * 8]);
	}
}

/* ch */
static uint64_t f1(uint64_t x, uint64_t y, uint64_t z) {
	return (x & y) ^ (~x & z);
}

/* maj */
static uint64_t f2(uint64_t x, uint64_t y, uint64_t z) {
	return (x & y) ^ (x & z) ^ (y & z);
}

/* e_0 */
static uint64_t f3(uint64_t x) {
	return ror64(x, 28) ^ ror64(x, 34) ^ ror64(x, 39);
}

/* e_1 */
static uint64_t f4(uint64_t x) {
	return ror64(x, 14) ^ ror64(x, 18) ^ ror64(x, 41);
}

static uint64_t f5(uint64_t x) {
	return ror64(x, 1) ^ ror64(x, 8) ^ (x >> 7);
}

static uint64_t f6(uint64_t x) {
	return ror64(x, 19) ^ ror64(x, 61) ^ (x >> 6);
}

#ifndef UNROLLED
void sha512_process_block(const uint8_t block[restrict 128], uint64_t hash[restrict 8]) {
	uint64_t a = hash[0];
	uint64_t b = hash[1];
	uint64_t c = hash[2];
	uint64_t d = hash[3];
	uint64_t e = hash[4];
	uint64_t f = hash[5];
	uint64_t g = hash[6];
	uint64_t h = hash[7];
	
	uint64_t temp1;
	uint64_t temp2;
	uint64_t W[80];
	
	for(uint_fast8_t i = 0; i < 16; i++) {
		W[i] = u8_to_u64_be(&block[i * 8]);
	}
	
	for(uint_fast8_t i = 16; i < 80; i++) {
		W[i] = f6(W[i - 2]) + W[i - 7] + f5(W[i - 15]) + W[i - 16];
	}
	
	for(uint_fast8_t t = 0; t < 80; t++) {
		temp1 = h + f4(e) + f1(e, f, g) + sha512_table[t] + W[t];
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
void sha512_process_block(const uint8_t block[restrict 128], uint64_t hash[restrict 8]) {
	uint64_t a = hash[0];
	uint64_t b = hash[1];
	uint64_t c = hash[2];
	uint64_t d = hash[3];
	uint64_t e = hash[4];
	uint64_t f = hash[5];
	uint64_t g = hash[6];
	uint64_t h = hash[7];
	
	uint64_t temp1;
	uint64_t temp2;
	uint64_t W[16];

#define CALC_W1(i) \
	W[i] = u8_to_u64_be(&block[i * 8])
#define CALC_W2(i) \
	W[i % 16] = f6(W[(i - 2) % 16]) + W[(i - 7) % 16] + \
	f5(W[(i - 15) % 16]) + W[(i - 16) % 16]

#define ROUND1(t, ra, rb, rc, rd, re, rf, rg, rh) \
	do { \
		CALC_W1(t); \
		temp1 = rh + f4(re) + f1(re, rf, rg) + sha512_table[t] + W[t]; \
		temp2 = f3(ra) + f2(ra, rb, rc); \
		rd += temp1; \
		rh = temp1 + temp2; \
	} while(0)
	
#define ROUND2(t, ra, rb, rc, rd, re, rf, rg, rh) \
	do { \
		CALC_W2(t); \
		temp1 = rh + f4(re) + f1(re, rf, rg) + sha512_table[t] + W[t % 16]; \
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
	
	ROUND2(64, a, b, c, d, e, f, g, h);
	ROUND2(65, h, a, b, c, d, e, f, g);
	ROUND2(66, g, h, a, b, c, d, e, f);
	ROUND2(67, f, g, h, a, b, c, d, e);
	ROUND2(68, e, f, g, h, a, b, c, d);
	ROUND2(69, d, e, f, g, h, a, b, c);
	ROUND2(70, c, d, e, f, g, h, a, b);
	ROUND2(71, b, c, d, e, f, g, h, a);
	
	ROUND2(72, a, b, c, d, e, f, g, h);
	ROUND2(73, h, a, b, c, d, e, f, g);
	ROUND2(74, g, h, a, b, c, d, e, f);
	ROUND2(75, f, g, h, a, b, c, d, e);
	ROUND2(76, e, f, g, h, a, b, c, d);
	ROUND2(77, d, e, f, g, h, a, b, c);
	ROUND2(78, c, d, e, f, g, h, a, b);
	ROUND2(79, b, c, d, e, f, g, h, a);
	
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

