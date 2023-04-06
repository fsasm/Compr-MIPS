/*
 * @file md5.h
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-07-13
 */

#ifndef MD5_H
#define MD5_H

#include <stdint.h>

#define MD5_BLOCK_SIZE	(64)
#define MD5_DIGEST_SIZE	(16)
#define MD5_HASH_SIZE	(16)

void md5_init_hash(uint32_t hash[4]);
void md5_hash_to_digest(uint32_t hash[4], uint8_t digest[MD5_DIGEST_SIZE]);
void md5_process_block(const uint8_t block[restrict 64], uint32_t hash[restrict 4]);

#endif
