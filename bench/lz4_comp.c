/**
 * @file lz4_comp.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-08-14
 * Compresses a data block with lz4. Based on example programs from the lz4 
 * distribution.
 */

#include "lz4.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define BLOCK_SIZE (1 * 1024)

static void write_int(int i)
{
	for (int j = 0; j < 4; j++) {
		putchar(i & 0xFF);
		i >>= 8;
	}
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	LZ4_stream_t stream;
	LZ4_resetStream(&stream);

	uint8_t blocks[2][BLOCK_SIZE];
	uint8_t out_buffer[LZ4_COMPRESSBOUND(BLOCK_SIZE)];

	unsigned int index = 0;
	bool run = true;

	do {
		size_t i = 0;
		/* read from stdin until block is full or an EOF */
		for (; i < BLOCK_SIZE; i++) {
			int c = getchar();
			if (c == EOF) {
				run = false;
				break;
			}

			blocks[index][i] = (uint8_t)c;
		}

		if (i == 0)
			break;

		int comp_size = LZ4_compress_fast_continue(&stream, (const char *)blocks[index], 
				(char *)out_buffer, i, LZ4_COMPRESSBOUND(BLOCK_SIZE), 1);

		if (comp_size <= 0)
			break;

		write_int(comp_size);

		for (int s = 0; s < comp_size; s++) {
			putchar(out_buffer[s]);
		}

		index ^= 1;

	} while(run);

	return 0;
}

