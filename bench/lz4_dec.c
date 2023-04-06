/**
 * @file lz4_dec.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-08-14
 * Decompresses a data block with lz4. Based on example programs from the lz4 
 * distribution.
 */

#include "lz4.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define BLOCK_SIZE (1 * 1024)

static int read_int()
{
	int i = 0;
	for (int j = 0; j < 4; j++) {
		int c = getchar();
		if (c == EOF)
			return 0;
		i |= c << (j * 8);
	}

	return i;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	LZ4_streamDecode_t stream;
	LZ4_setStreamDecode(&stream, NULL, 0);

	uint8_t blocks[2][BLOCK_SIZE];
	uint8_t comp_buffer[LZ4_COMPRESSBOUND(BLOCK_SIZE)];

	unsigned int index = 0;
	bool run = true;

	do {
		int comp_size = read_int();
		if (comp_size <= 0)
			break;

		for (int i = 0; i < comp_size; i++) {
			int c = getchar();
			if (c == EOF) {
				run = false;
				break;
			}

			comp_buffer[i] = c;
		}

		int dec_size = LZ4_decompress_safe_continue(&stream, (const char *)comp_buffer, 
				(char *)blocks[index], comp_size, BLOCK_SIZE);

		if (dec_size <= 0)
			break;

		for (int s = 0; s < dec_size; s++) {
			putchar(blocks[index][s]);
		}

		index ^= 1;

	} while(run);

	return 0;
}

