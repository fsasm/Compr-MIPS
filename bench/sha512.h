/*
 * @file sha512.h
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-04-05
 */
#ifndef SHA512_H
#define SHA512_H

#include <stdint.h>

#define SHA512_BLOCK_SIZE		(128)
#define SHA512_DIGEST_SIZE		(64)
#define SHA512_HASH_SIZE		(64)

void sha512_init_hash(uint64_t hash[8]);
void sha512_hash_to_digest(uint64_t hash[8], uint8_t digest[SHA512_DIGEST_SIZE]);
void sha512_process_block(const uint8_t block[restrict 128], uint64_t hash[restrict 8]);

#endif

