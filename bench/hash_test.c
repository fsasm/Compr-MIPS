/**
 * @file hash_test.h
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-04-04
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef HASH_MD5

#include "md5.h"
#define PROCESS_BLOCK md5_process_block
#define INIT_HASH md5_init_hash
#define TO_DIGEST md5_hash_to_digest
#define DIGEST_SIZE MD5_DIGEST_SIZE
#define BLOCK_SIZE MD5_BLOCK_SIZE
#define LENGTH_SIZE (8)
#define COPY_LENGTH u64_to_u8_le
#define HASH_VAR(name) uint32_t name[4]

#elif HASH_SHA256

#include "sha256.h"
#define PROCESS_BLOCK sha256_process_block
#define INIT_HASH sha256_init_hash
#define TO_DIGEST sha256_hash_to_digest
#define DIGEST_SIZE SHA256_DIGEST_SIZE
#define BLOCK_SIZE SHA256_BLOCK_SIZE
#define LENGTH_SIZE (8)
#define COPY_LENGTH u64_to_u8_be
#define HASH_VAR(name) uint32_t name[8]

#elif HASH_SHA512
#include "sha512.h"
#define PROCESS_BLOCK sha512_process_block
#define INIT_HASH sha512_init_hash
#define TO_DIGEST sha512_hash_to_digest
#define DIGEST_SIZE SHA512_DIGEST_SIZE
#define BLOCK_SIZE SHA512_BLOCK_SIZE
#define LENGTH_SIZE (16)
#define COPY_LENGTH u64_to_u8_be
#define HASH_VAR(name) uint64_t name[8]

#endif

#define LENGTH_INDEX (BLOCK_SIZE - LENGTH_SIZE)

struct test_case {
	const char *str;
	const char *digest;
};

static const char hex_digit[16] = "0123456789ABCDEF";

void u64_to_u8_le (uint64_t from, uint8_t to[8]) {
	for (int i = 0; i < 8; i++) {
		to[i] = (uint8_t)(from & 0xFFu);
		from = from >> 8;
	}
}
static void u64_to_u8_be(uint64_t from, uint8_t to[8])
{
	for (int i = 0; i < 8; i++) {
		to[7 - i] = (uint8_t)(from & 0xFFu);
		from = from >> 8;
	}
}

static void hash(size_t data_size, const uint8_t data[restrict data_size], uint8_t digest[restrict])
{
	HASH_VAR(hashsum);
	INIT_HASH(hashsum);

	/* hash all full blocks */
	size_t num_blocks = data_size / BLOCK_SIZE;
	for (size_t i = 0; i < num_blocks; i++) {
		PROCESS_BLOCK(&data[i * BLOCK_SIZE], hashsum);
	}

	/* add padding and hash the last or last two blocks */
	size_t index = data_size - (num_blocks * BLOCK_SIZE);

	uint8_t end_block[BLOCK_SIZE];
	memcpy(end_block, &data[num_blocks * BLOCK_SIZE], index);
	end_block[index] = 0x80;
	index++;

	if (index == LENGTH_INDEX) {
		uint64_t length = data_size * 8; /* convert to bit */
		COPY_LENGTH(length, &end_block[BLOCK_SIZE - 8]);
		PROCESS_BLOCK(end_block, hashsum);
		TO_DIGEST(hashsum, digest);
	} else if (index < LENGTH_INDEX) {
		memset(&end_block[index], 0, BLOCK_SIZE - 8 - index);
		uint64_t length = data_size * 8; /* convert to bit */
		COPY_LENGTH(length, &end_block[BLOCK_SIZE - 8]);
		PROCESS_BLOCK(end_block, hashsum);
		TO_DIGEST(hashsum, digest);
	} else {
		memset(&end_block[index], 0, BLOCK_SIZE - index);
		PROCESS_BLOCK(end_block, hashsum);
		memset(end_block, 0, BLOCK_SIZE - 8);
		uint64_t length = data_size * 8; /* convert to bit */
		COPY_LENGTH(length, &end_block[BLOCK_SIZE - 8]);
		PROCESS_BLOCK(end_block, hashsum);
		TO_DIGEST(hashsum, digest);
	}
}

static void run_test(struct test_case *test)
{
	uint8_t digest[DIGEST_SIZE];

	hash(strlen(test->str), (const uint8_t *)test->str, digest);

	/*
	for (int i = 0; i < DIGEST_SIZE; i += 2) {
		printf("%02X - %c%c | ", digest[i], test->digest[2 * i], test->digest[2 * i + 1]);
		printf("%02X - %c%c\n", digest[i + 1], test->digest[2 * i + 2], test->digest[2 * i + 3]);
	}*/

	for (int i = 0; i < DIGEST_SIZE; i++) {
		char digit0 = hex_digit[digest[i] >> 4];
		char digit1 = hex_digit[digest[i] & 0x0F];

		if (digit0 != test->digest[2 * i + 0] ||
			digit1 != test->digest[2 * i + 1]) {
			puts("Test failed");
			return;
		} else {
		}
	}
	puts("Test succeeded");
}

int main(void)
{
	#ifdef HASH_MD5
	/* https://www.cosic.esat.kuleuven.be/nessie/testvectors/hash/md5/Md5-128.unverified.test-vectors */
	struct test_case tests[] = {
		{"", "D41D8CD98F00B204E9800998ECF8427E"},
		{"a", "0CC175B9C0F1B6A831C399E269772661"},
		{"abc", "900150983CD24FB0D6963F7D28E17F72"},
		{"message digest", "F96B697D7CB7938D525A2F31AAF161D0"},
		{"abcdefghijklmnopqrstuvwxyz", "C3FCD3D76192E4007DFB496CCA67E13B"},
		{"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "8215EF0796A20BCAAAE116D3876C664A"},
		{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "D174AB98D277D9F5A5611C2C9F419D9F"},
		{"12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57EDF4A22BE3C955AC49DA2E2107B67A"}
	};
	#elif HASH_SHA256
	/* test vectors are from NESSIE https://www.cosic.esat.kuleuven.be/nessie/testvectors/hash/sha/Sha-2-256.unverified.test-vectors */
	struct test_case tests[] = {
		{"", "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855"},
		{"a", "CA978112CA1BBDCAFAC231B39A23DC4DA786EFF8147C4E72B9807785AFEE48BB"},
		{"abc", "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD"},
		{"message digest", "F7846F55CF23E14EEBEAB5B4E1550CAD5B509E3348FBC4EFA3A1413D393CB650"},
		{"abcdefghijklmnopqrstuvwxyz", "71C480DF93D6AE2F1EFAD1447C66C9525E316218CF51FC8D9ED832F2DAF18B73"},
		{"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "248D6A61D20638B8E5C026930C3E6039A33CE45964FF2167F6ECEDD419DB06C1"},
		{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "DB4BFCBD4DA0CD85A60C3C37D3FBD8805C77F15FC6B1FDFE614EE0A7C8FDB4C0"},
		{"12345678901234567890123456789012345678901234567890123456789012345678901234567890", "F371BC4A311F2B009EEF952DD83CA80E2B60026C8E935592D0F9C308453C813E"}
	};
	#elif HASH_SHA512
	/* https://www.cosic.esat.kuleuven.be/nessie/testvectors/hash/sha/Sha-2-512.unverified.test-vectors */
	struct test_case tests[] = {
		{"", "CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE47D0D13C5D85F2B0FF8318D2877EEC2F63B931BD47417A81A538327AF927DA3E"},
		{"a", "1F40FC92DA241694750979EE6CF582F2D5D7D28E18335DE05ABC54D0560E0F5302860C652BF08D560252AA5E74210546F369FBBBCE8C12CFC7957B2652FE9A75"},
		{"abc", "DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A2192992A274FC1A836BA3C23A3FEEBBD454D4423643CE80E2A9AC94FA54CA49F"},
		{"message digest", "107DBF389D9E9F71A3A95F6C055B9251BC5268C2BE16D6C13492EA45B0199F3309E16455AB1E96118E8A905D5597B72038DDB372A89826046DE66687BB420E7C"},
		{"abcdefghijklmnopqrstuvwxyz", "4DBFF86CC2CA1BAE1E16468A05CB9881C97F1753BCE3619034898FAA1AABE429955A1BF8EC483D7421FE3C1646613A59ED5441FB0F321389F77F48A879C7B1F1"},
		{"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "204A8FC6DDA82F0A0CED7BEB8E08A41657C16EF468B228A8279BE331A703C33596FD15C13B1B07F9AA1D3BEA57789CA031AD85C7A71DD70354EC631238CA3445"},
		{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "1E07BE23C26A86EA37EA810C8EC7809352515A970E9253C26F536CFC7A9996C45C8370583E0A78FA4A90041D71A4CEAB7423F19C71B9D5A3E01249F0BEBD5894"},
		{"12345678901234567890123456789012345678901234567890123456789012345678901234567890", "72EC1EF1124A45B047E8B7C75A932195135BB61DE24EC0D1914042246E0AEC3A2354E093D76F3048B456764346900CB130D2A4FD5DD16ABB5E30BCB850DEE843"},
		{"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu", "8E959B75DAE313DA8CF4F72814FC143F8F7779C6EB9F7FA17299AEADB6889018501D289E4900F7E4331B99DEC4B5433AC7D329EEB6DD26545E96E55B874BE909"}
	};
	#endif

	#ifdef HASH_MD5
	puts("MD5 test");
	puts("running 8 test cases");
	#elif HASH_SHA256
	puts("SHA256 test");
	puts("running 8 test cases");
	#elif HASH_SHA512
	puts("SHA512 test");
	puts("running 8 test cases");
	#endif 

	for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		run_test(&tests[i]);
	}
	/*
	uint32_t hash[4];
	md5_init_hash(hash);
	uint8_t zero[64];
	memset(zero, 0, 64);
	md5_process_block(zero, hash);*/
	puts("All tests done");
	return 0;
}

