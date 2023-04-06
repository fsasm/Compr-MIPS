/*
 * @file sha256.h
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-04-04
 */

#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>

#define SHA256_BLOCK_SIZE	(64)
#define SHA256_DIGEST_SIZE	(32)
#define SHA256_HASH_SIZE	(32)

void sha256_init_hash(uint32_t hash[8]);
void sha256_hash_to_digest(uint32_t hash[8], uint8_t digest[SHA256_DIGEST_SIZE]);
void sha256_process_block(const uint8_t block[restrict 64], uint32_t hash[restrict 8]);

#endif
