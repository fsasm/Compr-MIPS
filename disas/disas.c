/**
 * @file disas.c
 * @author Fabjan Sukalia <fsukalia@gmail.com>
 * @date 2016-03-25
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../common/instr.h"
#include "../common/v2_instr.h"
#include "../common/print_instr.h"

static char *program_name = "disas";

static void usage(void)
{
	fprintf(stderr, "Usage: %s [-c] [-p] IN-FILE \n", program_name);
	fprintf(stderr, "\t-c\tcompressed instruction format\n");
	fprintf(stderr, "\t-p\tconvert to pseudo instructions if possible\n");
	exit(EXIT_FAILURE);
}

static uint32_t to_instr4(uint8_t bytes[4])
{
	uint32_t instr = bytes[0];
	instr = (instr << 8) | bytes[1];
	instr = (instr << 8) | bytes[2];
	instr = (instr << 8) | bytes[3];
	return instr;
}

static uint16_t to_instr2(uint8_t bytes[2])
{
	uint16_t instr = bytes[0];
	instr = (instr << 8) | bytes[1];
	return instr;
}

int main(int argc, char *argv[])
{
	if (argc > 0)
		program_name = argv[0];

	bool v2 = false;
	bool conv = false;

	int opt = 0;

	while ((opt = getopt(argc, argv, "cp")) != -1) {
		switch(opt) {
		case 'c':
			v2 = true;
			break;

		case 'p':
			conv = true;
			break;

		case '?':
		default:
			usage();
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "input file required\n");
		usage();
	}

	FILE *file = fopen(argv[optind], "rb");

	if (file == NULL) {
		fprintf(stderr, "Couldn't open file '%s'\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	uint8_t bytes[4];
	struct instr instr;

	uint32_t addr = 0;

	if (!v2) {
		while(fread(bytes, sizeof(bytes), 1, file) == 1) {
			uint32_t code = to_instr4(bytes);
			memset(&instr, 0, sizeof(instr));
	
			parse_instr(code, &instr);
	
			if (instr.op == INVALID_OP) {
				fprintf(stderr, "Invalid instruction at %u\n", addr);
				exit(EXIT_FAILURE);
			}
	
			if (conv) {
				conv_to_pseudo(&instr);
			}
	
			printf("%8.8x\t", addr);
			print_instr(&instr);
	
			addr += 4;
		}
	} else {
		while(fread(bytes, 2, 1, file) == 1) {
			uint32_t code = to_instr2(bytes);
			code <<= 16;
	
			if (code < 0x80000000) {
				/* long instruction */
				fread(bytes, 2, 1, file);
				code |= to_instr2(bytes);
			}
			memset(&instr, 0, sizeof(instr));
	
			parse_instr_v2(code, &instr);
	
			if (instr.op == INVALID_OP) {
				fprintf(stderr, "Invalid instruction at %u\n", addr);
				exit(EXIT_FAILURE);
			}
	
			if (conv) {
				conv_to_pseudo(&instr);
			}
	
			printf("%8.8X\t", addr);
			print_instr(&instr);
	
			if (code < 0x80000000) {
				addr += 4;
			} else {
				addr += 2;
			}
		}
	}

	fclose(file);

	return 0;
}

